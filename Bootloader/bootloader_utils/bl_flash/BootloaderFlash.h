/***************************************************************************************
 * Module Name  : Bootloader Flash                                                     *
 * File Name    : BootloaderFlash.h                                                    *
 * Author       : Ouda                                                                 *
 * Purpose      : This module is a wrapper layer driver for bootloader flashing        *
 *                functionalities.                                                     *
 ***************************************************************************************/

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

/****************************************************************************************
 *                                    -=[Includes]=-                                    *
 ****************************************************************************************/
#include "flash_c55_driver.h"
#include "crc_driver.h"
#include "status.h"

/****************************************************************************************
 *                                  -=[Definitions]=-                                   *
 ****************************************************************************************/
/* Flash Configuration */
#define BOOTLOADER_FLASH_WORDSIZE           (C55_WORD_SIZE)
#define BOOTLOADER_FLASH_NUM_256KB_BLOCKS   (22U)

/* Lock States */
#define UNLOCK_FIRST256_BLOCKS              (0x00000000U)
#define UNLOCK_SECOND256_BLOCKS             (0x00000000U)

/* Verification Parameters */
#define NUMBER_OF_WORD_BLANK_CHECK          (0x90)
#define NUMBER_OF_WORD_PGM_VERIFY           (0x80)
#define NUMBER_OF_WORD_CHECK_SUM            (0x120)

/* Module Instances */
#define BOOTLOADERFLASH_CRC_INSTANCE        (0U)

/* Platform-Specific Flash Definitions */
#define FLASH_FMC                           (PFLASH_BASE)
#define FLASH_PFCR1                         (0x000000000U)
#define FLASH_PFCR2                         (0x000000004U)
#define FLASH_FMC_BFEN_MASK                 (0x000000001U)

/****************************************************************************************
 *                              -=[Function Prototypes]=-                               *
 ****************************************************************************************/

/****************************************************************
 * Function Name: BlockFlags_ClearAll [HELPER]                  *
 * Inputs       : None                                          *
 * Outputs      : None                                          *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Clears all block erase flags.                 *
 ****************************************************************/
void BootloaderFlash_ClearErasedFlags(void);

/****************************************************************
 * Function Name: BootloaderFlash_DisableCache                  *
 * Inputs       : None                                          *
 * Outputs      : None                                          *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Disables flash controller cache for reliable  *
 *                flash operations.                             *
 ****************************************************************/
void BootloaderFlash_DisableCache(void);

/****************************************************************
 * Function Name: BootloaderFlash_RestoreCache                  *
 * Inputs       : None                                          *
 * Outputs      : None                                          *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Restores original flash controller cache      *
 *                configuration.                                *
 ****************************************************************/
void BootloaderFlash_RestoreCache(void);

/****************************************************************
 * Function Name: BootloaderFlash_Init                          *
 * Inputs       : None                                          *
 * Outputs      : status_t - Initialization status              *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Initializes the Bootloader Flash module.      *
 ****************************************************************/
status_t BootloaderFlash_Init(void);

/****************************************************************
 * Function Name: BootloaderFlash_Unlock                        *
 * Inputs       : None                                          *
 * Outputs      : status_t - Unlock operation status            *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Unlocks the first 256K blocks of the flash.   *
 ****************************************************************/
status_t BootloaderFlash_Unlock(void);

/****************************************************************
 * Function Name: BootloaderFlash_Erase                         *
 * Inputs       : a_Dest - Starting address of flash to erase   *
 *                a_Size - Number of bytes to erase             *
 * Outputs      : status_t - Erase operation status             *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Erases specified flash range in main space    *
 *                starting at given address for given size.     *
 ****************************************************************/
status_t BootloaderFlash_Erase(uint32_t a_Dest, uint32_t a_Size);

/****************************************************************
 * Function Name: BootloaderFlash_VerifyBlank                   *
 * Inputs       : a_Dest - Starting address to verify           *
 *                a_Size - Number of bytes to verify            *
 * Outputs      : status_t - Verification status                *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Verifies if the specified flash block is blank*
 ****************************************************************/
status_t BootloaderFlash_VerifyBlank(uint32_t a_Dest, uint32_t a_Size);

/****************************************************************
 * Function Name: BootloaderFlash_Program                       *
 * Inputs       : a_dest  - Destination address in flash        *
 *                a_size  - Size of data to program (bytes)     *
 *                a_source- Source address of data              *
 * Outputs      : status_t - Programming status                 *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Asynchronous                                  *
 * Description  : Programs data to the specified flash address. *
 ****************************************************************/
status_t BootloaderFlash_Program(uint32_t a_dest, uint32_t a_size, uint32_t a_source);

/****************************************************************
 * Function Name: BootloaderFlash_ProgramVerify                 *
 * Inputs       : a_dest  - Destination address in flash        *
 *                a_size  - Size of data verified (bytes)       *
 *                a_source- Source address of data              *
 * Outputs      : status_t - Verification status                *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Verifies the programmed data in flash.        *
 ****************************************************************/
status_t BootloaderFlash_ProgramVerify(uint32_t a_dest, uint32_t a_size, uint32_t a_source);

/****************************************************************
 * Function Name: BootloaderFlash_Read                          *
 * Inputs       : a_dest  - Source address in flash to read from*
 *                a_size  - Size of data to read (bytes)        *
 *                a_pBuffer- Pointer to buffer for storing data *
 * Outputs      : status_t - Read operation status              *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Reads data from specified flash address into  *
 *                the provided buffer.                          *
 ****************************************************************/
status_t BootloaderFlash_Read(uint32_t a_dest, uint32_t a_size, uint32_t *a_pBuffer);

/****************************************************************
 * Function Name: BootloaderFlash_InitCRC                       *
 * Inputs       : None                                          *
 * Outputs      : status_t - Initialization status              *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Initializes CRC module using predefined       *
 *                instance and configuration.                   *
 ****************************************************************/
status_t BootloaderFlash_InitCRC(void);

/****************************************************************
 * Function Name: BootloaderFlash_CalculateCRC32                *
 * Inputs       : a_dest  - Source address in flash to read from*
 *                a_size  - Size of data to calculate CRC for   *
 *                          (bytes)                             *
 * Outputs      : uint32_t - Calculated CRC32                   *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Calculates CRC32 for flashed data.            *
 ****************************************************************/
uint32_t BootloaderFlash_CalculateCRC32(uint32_t a_dest, uint32_t a_size);

#endif /* BOOTLOADER_H_ */