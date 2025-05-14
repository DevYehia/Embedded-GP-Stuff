#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include <stdint.h>
#include "can_pal1.h"
#include "UDS.h"
#include "status.h"
#include "bootloader_utils/bl_flash/BootloaderFlash.h"    /* Lower-level flash operations */
#include "bootloader_utils/bl_signature/bl_hash/sha256.h" /* SHA-256 hashing routines used for firmware integrity checks */
#include "bootloader_utils/bl_signature/bl_signature.h"   /* ECDSA signature generation and verification */
#include "bootloader_utils/bl_decompress/bl_decompress.h" /* Decompression routines for compressed bootloader data blocks */

/* brief Maximum UDS data block number (protocol limit). */
#define MAX_BLOCK_NUMBER 4095U

/**
 * @brief Remap a firmware address into the bootloader-update region.
 *
 * When updating the resident bootloader image, certain flash sectors must
 * be redirected to an update zone. This inline function forces bits 20–23
 * of the address to '0101' (hex digit 5) to perform the remapping.
 *
 * @param addr  Original flash address in application or bootloader space.
 * @return      Address remapped into the bootloader-update flash region.
 */
static inline uint32_t RemapToUpdateRegion(uint32_t addr)
{
    return (addr & ~0x00F00000U) /* clear bits 20-23 */
           | (5U << 20);         /* set hex-digit-3 to 5 */
}

/**
 * @brief Initialize the bootloader module and disable flash caches.
 *
 * Validates and stores application callback handlers, disables flash
 * caches to ensure safe flash operations, retrieves bootloader metadata
 * over UDS, and initializes the low-level flash driver.
 *
 * @param a_pBLHandlersConfig  Pointer to user-supplied callback handlers
 *                              for data transfer, memory check/erase,
 *                              and finalization operations.
 * @return                     STATUS_SUCCESS on success, or a specific
 *                             status_t error code on failure.
 */
status_t Bootloader_Init(BL_Functions *a_pBLHandlersConfig);

/**
 * @brief Erase the target flash memory region defined in UDS metadata.
 *
 * Steps:
 *  1. Unlock the flash controller.
 *  2. Remap the start address if update-mode is required.
 *  3. Erase the specified sectors.
 *  4. Verify erased region reads back as all 0xFF.
 *
 * @return STATUS_SUCCESS if erase and blank verification succeed;
 *         otherwise returns a failure status_t code.
 */
status_t Bootloader_Erase_Memory(void);

/**
 * @brief Program a block of flash memory from the UDS data buffer.
 *
 * On each call, this function:
 *  - Decompresses the block if compression_flag is set.
 *  - Calculates the write address (with remapping if needed).
 *  - Writes the data into flash.
 *  - Verifies the written data by reading back.
 *  - Updates a running total of bytes programmed.
 *
 * @return STATUS_SUCCESS if programming and verification succeed;
 *         otherwise returns a failure status_t code.
 */
status_t Bootloader_Program(void);

/**
 * @brief Verify the integrity of the programmed flash region.
 *
 * Performs:
 *  - CRC32 calculation over all programmed bytes.
 *  - Comparison against the CRC field from UDS metadata.
 *  - SHA-256 hash update for ECDSA verification stage.
 *  - Restores flash cache settings after CRC.
 *
 * @return STATUS_SUCCESS if CRC matches expected value;
 *         otherwise returns STATUS_ERROR.
 */
status_t Bootloader_CheckMemory(void);

/**
 * @brief Finalize the bootloader update by verifying the ECDSA signature.
 *
 * This step:
 *  - Finalizes the SHA-256 hash into a digest buffer.
 *  - Verifies the digest against the signature attached in UDS metadata.
 *  - If valid, writes a "valid application" flag to a known RAM register.
 *
 * @return STATUS_SUCCESS if signature is valid; otherwise an error code.
 */
status_t Bootloader_Finalize_Programming(void);

#endif /* BOOTLOADER_H_ */
