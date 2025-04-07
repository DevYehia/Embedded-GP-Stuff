/***************************************************************************************
 * Module Name  : Bootloader Flash                                                     *
 * File Name    : BootloaderFlash.h                                                    *
 * Author       : Ouda                                                                 *
 * Purpose      : This module is a rubber layer driver for bootloader flashing	       *
 * 				  functionalities.										               *
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
#define BOOTLOADER_FLASH_WORDSIZE (C55_WORD_SIZE)

/* Lock State */
#define UNLOCK_FIRST256_BLOCKS (0x00000000U)
#define UNLOCK_SECOND256_BLOCKS (0x00000000U)

#define NUMBER_OF_WORD_BLANK_CHECK (0x90)
#define NUMBER_OF_WORD_PGM_VERIFY (0x80)
#define NUMBER_OF_WORD_CHECK_SUM (0x120)

#define BOOTLOADERFLASH_CRC_INSTANCE (0U)

#define BOOTLOADER_FLASH_NUM_256KB_BLOCKS (22U)

/* Platform Flash */
#define FLASH_FMC PFLASH_BASE
#define FLASH_PFCR1 0x000000000U
#define FLASH_PFCR2 0x000000004U
#define FLASH_FMC_BFEN_MASK 0x000000001U

/****************************************************************************************
 *                                  -=[Function IDs]=-                                  *
 ****************************************************************************************/

/****************************************************************************************
 *                              -=[Function Prototypes]=-                               *
 ****************************************************************************************/

/****************************************************************
 * Function Name: BootloaderFlash_Init                          *
 * Inputs       : None                                          *
 * Outputs      : status_t - Initialization status              *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                    *
 * Description  : Initializes the Bootloader Flash module.      *
 ****************************************************************/
status_t BootloaderFlash_Init(void);

/****************************************************************
 * Function Name: BootloaderFlash_Unlock                        *
 * Inputs       : None                                          *
 * Outputs      : status_t - Unlock operation status            *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                    *
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
 * Inputs       : None                                          *
 * Outputs      : status_t - Verification status                *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                    *
 * Description  : Verifies if the specified flash block is blank*
 ****************************************************************/
status_t BootloaderFlash_VerifyBlank(uint32_t a_Dest, uint32_t a_Size);

/****************************************************************
 * Function Name: BootloaderFlash_Program                       *
 * Inputs       : a_dest  - Destination address in flash        *
 *                a_size  - Size of data to program             *
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
 *                a_size  - Size of data verified               *
 *                a_source- Source address of data              *
 * Outputs      : status_t - Verification status                *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                    *
 * Description  : Verifies the programmed data in flash.        *
 ****************************************************************/
status_t BootloaderFlash_ProgramVerify(uint32_t a_dest, uint32_t a_size, uint32_t a_source);

/****************************************************************
 * Function Name: BootloaderFlash_Read                          *
 * Inputs       : a_dest  - Source address in flash to read from*
 *                a_size  - Size of data to read (in bytes)     *
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
 * Description  : Initializes CRC module using predefined        *
 *                instance and configuration.                   *
 ****************************************************************/
status_t BootloaderFlash_InitCRC(void);

/****************************************************************
 * Function Name: BootloaderFlash_Read                          *
 * Inputs       : a_dest  - Source address in flash to read from*
 *                a_size  - Size of data to calculate CRC for 	*
 *                (in bytes)     								*
 * Outputs      : uint32_t - Caclulated CRC32              		*
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Calculates CRC32 for flashed data				*
 ****************************************************************/
uint32_t BootloaderFlash_CalculateCRC32(uint32_t a_dest, uint32_t a_size);

/****************************************************************************************
 *                          -=[Static Functions Prototypes]=-                           *
 ****************************************************************************************/

/**************************************************************
 *                          Disable Flash Cache                *
 ***************************************************************/
void DisableFlashControllerCache(uint32_t flashConfigReg,
                                 uint32_t disableVal,
                                 uint32_t *origin_pflash_pfcr);

/*****************************************************************
 *               Restore configuration register of FCM            *
 ******************************************************************/
void RestoreFlashControllerCache(uint32_t flashConfigReg,
                                 uint32_t pflash_pfcr);

/****************************************************************************************
 *                          -Mark the specified block as erased                         *
 ****************************************************************************************/

void BlockFlags_SetErased(uint32_t block);

/****************************************************************************************
 *                          Clear the erased flags for all blocks                       *
 ****************************************************************************************/

void BlockFlags_ClearAll(void);

/****************************************************************************************
 *                          Check if the specified block is marked as erased            *
 ****************************************************************************************/

bool BlockFlags_IsErased(uint32_t block);

#endif
