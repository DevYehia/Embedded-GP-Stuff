#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "can_pal1.h"
#include <stdint.h>
#include "status.h"
#include "Bootloader_Flash/BootloaderFlash.h" /* Lower-level flash operations */
#include "UDS.h"
#include "bootloader_utils/bl_signature/bl_hash/sha256.h"
#include "bootloader_utils/bl_signature/bl_signature.h"
#include "bootloader_utils/bl_decompress/bl_decompress.h"

/* Maximum block number for data transfer operations (UDS limit) */
#define MAX_BLOCK_NUMBER 4095U

/* Platform-specific flash definitions */
#define FLASH_FMC PFLASH_BASE
#define FLASH_PFCR1 0x000000000U
#define FLASH_PFCR2 0x000000004U
#define FLASH_FMC_BFEN_MASK 0x000000001U

/**
 * @brief Initialize the bootloader module.
 *
 * This function initializes the bootloader by:
 *   - Validating and storing the provided function handlers.
 *   - Disabling flash controller caches.
 *   - Retrieving bootloader data via UDS.
 *   - Initializing the lower-level flash module.
 *
 * @param a_pBLHandlersConfig Pointer to the structure containing application-specific bootloader function handlers.
 * @return status_t STATUS_SUCCESS if initialization is successful; otherwise, an error status.
 */
status_t Bootloader_Init(BL_Functions *a_pBLHandlersConfig);

/**
 * @brief Erase flash memory.
 *
 * This function performs the following steps:
 *   - Unlocks flash memory.
 *   - Erases the flash area specified in the global UDS bootloader data.
 *   - Verifies that the erased area is blank.
 *
 * @return status_t STATUS_SUCCESS if the memory is successfully erased and verified; otherwise, an error status.
 */
status_t Bootloader_Erase_Memory(void);

/**
 * @brief Program flash memory.
 *
 * This function programs flash memory based on the global UDS bootloader data and then
 * verifies that the programming operation is complete.
 *
 * @return status_t STATUS_SUCCESS if programming and verification are successful; otherwise, an error status.
 */
status_t Bootloader_Program(void);

/**
 * @brief Verify programmed flash memory.
 *
 * This function computes a CRC32 over the programmed flash area and compares it with the expected
 * value provided in the global UDS bootloader data.
 *
 * @return status_t STATUS_SUCCESS if the CRC check is successful; otherwise, an error status.
 */
status_t Bootloader_CheckMemory(void);


/* Handler to finish the hash and verify the signature.
 * Expects `signature` to point to the raw ECDSA signature bytes.
 */
status_t Bootloader_Finalize_Programming(void);

#endif /* BOOTLOADER_H_ */
