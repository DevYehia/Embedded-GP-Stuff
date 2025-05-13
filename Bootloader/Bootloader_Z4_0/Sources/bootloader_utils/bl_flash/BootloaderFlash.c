/***************************************************************************************
 * Module Name  : Bootloader Flash                                                     *
 * File Name    : BootloaderFlash.c                                                    *
 * Author       : Ouda                                                                 *
 * Purpose      : This module is a rubber layer driver for bootloader flashing         *
 *                functionalities.                                                     *
 ***************************************************************************************/

/****************************************************************************************
 *                                    -=[Includes]=-                                    *
 ****************************************************************************************/
#include "BootloaderFlash.h"


/*******************************************************************************
 *                             Global Variables                                *
 *******************************************************************************/

/****************************************************************
 * Global Variable Name : g_BootLoaderFlashModuleInit           *
 *--------------------------------------------------------------*
 * Description          : Flag for module initialization.       *
 ****************************************************************/
static uint32_t volatile g_BootLoaderFlashModuleInit = false;

/****************************************************************
 * Global Variable Name : g_BootLoaderFlash_pCtxData            *
 *--------------------------------------------------------------*
 * Description          : Structure to store the context of     *
 *                        flash operations.                     *
 *--------------------------------------------------------------*
 * Fields Description   : - dest   : Destination address of the *
 *                                    program operation.        *
 *                        - pMisr  : Context MISR values for    *
 *                                   user test mode.            *
 *                        - size   : Size of the program data.  *
 *                        - source : Source address of the      *
 *                                   program data.              *
 ****************************************************************/
static flash_context_data_t g_BootLoaderFlash_pCtxData;

/****************************************************************
 * Global Variable Name : g_BootLoaderFlash_failedAddress       *
 *--------------------------------------------------------------*
 * Description          : Holds the address where flash         *
 *                        operation failed.                     *
 ****************************************************************/
static uint32_t g_BootLoaderFlash_failedAddress = 0;

/****************************************************************
 * Global Variable Name : g_BlocksStartAddresses                *
 *--------------------------------------------------------------*
 * Description          : Array containing starting addresses   *
 *                        of 256KB flash blocks for bootloader  *
 *                        operations.                           *
 ****************************************************************/
static const uint32_t g_BlocksStartAddresses[BOOTLOADER_FLASH_NUM_256KB_BLOCKS] = {
    0x01000000, /* Start Address of 256KB Code Flash Block 0 */
    0x01040000, /* Start Address of 256KB Code Flash Block 1 */
    0x01080000, /* Start Address of 256KB Code Flash Block 2 */
    0x010C0000, /* Start Address of 256KB Code Flash Block 3 */
    0x01100000, /* Start Address of 256KB Code Flash Block 4 */
    0x01140000, /* Start Address of 256KB Code Flash Block 5 */
    0x01180000, /* Start Address of 256KB Code Flash Block 6 */
    0x011C0000, /* Start Address of 256KB Code Flash Block 7 */
    0x01200000, /* Start Address of 256KB Code Flash Block 8 */
    0x01240000, /* Start Address of 256KB Code Flash Block 9 */
    0x01280000, /* Start Address of 256KB Code Flash Block 10 */
    0x012C0000, /* Start Address of 256KB Code Flash Block 11 */
    0x01300000, /* Start Address of 256KB Code Flash Block 12 */
    0x01340000, /* Start Address of 256KB Code Flash Block 13 */
    0x01380000, /* Start Address of 256KB Code Flash Block 14 */
    0x013C0000, /* Start Address of 256KB Code Flash Block 15 */
    0x01400000, /* Start Address of 256KB Code Flash Block 16 */
    0x01440000, /* Start Address of 256KB Code Flash Block 17 */
    0x01480000, /* Start Address of 256KB Code Flash Block 18 */
    0x014C0000, /* Start Address of 256KB Code Flash Block 19 */
    0x01500000, /* Start Address of 256KB Code Flash Block 20 */
    0x01540000  /* Start Address of 256KB Code Flash Block 21 */
};

/****************************************************************
 * Global Variable Name : g_BootloaderFLashCRC_InitConfig       *
 *--------------------------------------------------------------*
 * Description          : Structure with CRC Module configs.    *
 ****************************************************************/
extern const crc_user_config_t g_BootloaderFLashCRC_InitConfig;

/****************************************************************
 * Global Variable Name : g_blockEraseFlags                     *
 *--------------------------------------------------------------*
 * Description          : Holds flags indicating whether        *
 *                        erase operation has occurred for      *
 *                        each flash block.                     *
 ****************************************************************/
static uint32_t g_blockEraseFlags = 0;

/****************************************************************
 * Global Variable Name : g_pflash_pfcr1                        *
 *--------------------------------------------------------------*
 * Description          : Stores the original configuration of  *
 *                        the primary flash controller register *
 *                        PFCR1 before disabling the cache.     *
 ****************************************************************/
static volatile uint32_t g_pflash_pfcr1 = 0;

/****************************************************************
 * Global Variable Name : g_pflash_pfcr2                        *
 *--------------------------------------------------------------*
 * Description          : Stores the original configuration of  *
 *                        the primary flash controller register *
 *                        PFCR2 before disabling the cache.     *
 ****************************************************************/
static volatile uint32_t g_pflash_pfcr2 = 0;

/*******************************************************************************
 *                                Functions                                    *
 *******************************************************************************/

 /****************************************************************
 * Function Name: DisableFlashControllerCache [HELPER]          *
 * Inputs       : flashConfigReg - Flash configuration register *
 *                disableVal - Value to disable cache           *
 *                origin_pflash_pfcr - Pointer to store original*
 *                                      register value          *
 * Outputs      : None                                          *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Disables the flash controller cache by        *
 *                modifying the specified configuration register*
 *                and saves the original value.                 *
 ****************************************************************/
static void DisableFlashControllerCache(uint32_t flashConfigReg,
	uint32_t disableVal,
	uint32_t *origin_pflash_pfcr)
{
/* Read the values of the register of flash configuration */
*origin_pflash_pfcr = REG_READ32(FLASH_FMC + flashConfigReg);
/* Disable Caches */
REG_BIT_CLEAR32(FLASH_FMC + flashConfigReg, disableVal);
}

/****************************************************************
* Function Name: RestoreFlashControllerCache [HELPER]          *
* Inputs       : flashConfigReg - Flash configuration register *
*                pflash_pfcr - Original register value to      *
* 				  restore                                       *
* Outputs      : None                                          *
* Reentrancy   : Non-Reentrant                                 *
* Synchronous  : Synchronous                                   *
* Description  : Restores the original configuration to the    *
*                specified flash controller cache register.    *
****************************************************************/
static void RestoreFlashControllerCache(uint32_t flashConfigReg,
	uint32_t pflash_pfcr)
{
REG_WRITE32(FLASH_FMC + flashConfigReg, pflash_pfcr);
}


/****************************************************************
* Function Name: BlockFlags_SetErased [HELPER]                 *
* Inputs       : block - Index of the block to mark as erased  *
* Outputs      : None                                          *
* Reentrancy   : Non-Reentrant                                 *
* Synchronous  : Synchronous                                   *
* Description  : Sets the flag indicating the specified block  *
*                has been erased.                              *
****************************************************************/
static void BlockFlags_SetErased(uint32_t block)
{
if (block < BOOTLOADER_FLASH_NUM_256KB_BLOCKS)
{
g_blockEraseFlags |= (1U << block);
}
}

/****************************************************************
* Function Name: BlockFlags_IsErased [HELPER]                  *
* Inputs       : block - Index of the block to check           *
* Outputs      : bool - True if block is marked as erased      *
* Reentrancy   : Non-Reentrant                                 *
* Synchronous  : Synchronous                                   *
* Description  : Checks if the specified block is marked as    *
*                erased.                                       *
****************************************************************/
static bool BlockFlags_IsErased(uint32_t block)
{
if (block < BOOTLOADER_FLASH_NUM_256KB_BLOCKS)
{
return (g_blockEraseFlags & (1U << block)) != 0;
}
return false;
}
/****************************************************************
* Function Name: BlockFlags_ClearAll [HELPER]                  *
* Inputs       : None                                          *
* Outputs      : None                                          *
* Reentrancy   : Non-Reentrant                                 *
* Synchronous  : Synchronous                                   *
* Description  : Clears all block erase flags.                 *
****************************************************************/
void BootloaderFlash_ClearErasedFlags(void)
{
g_blockEraseFlags = 0;
}

/****************************************************************
* Function Name: BootloaderFlash_DisableCache                  *
* Inputs       : None                                          *
* Outputs      : None                                          *
* Reentrancy   : Non-Reentrant                                 *
* Synchronous  : Synchronous                                   *
* Description  : Disables the flash controller cache to ensure *
*                reliable flash operations by saving the       *
*                original cache configurations.                *
****************************************************************/
void BootloaderFlash_DisableCache(void){
DisableFlashControllerCache(FLASH_PFCR1, FLASH_FMC_BFEN_MASK, &g_pflash_pfcr1);
DisableFlashControllerCache(FLASH_PFCR2, FLASH_FMC_BFEN_MASK, &g_pflash_pfcr2);
}

/****************************************************************
* Function Name: BootloaderFlash_RestoreCache                  *
* Inputs       : None                                          *
* Outputs      : None                                          *
* Reentrancy   : Non-Reentrant                                 *
* Synchronous  : Synchronous                                   *
* Description  : Restores the original flash controller cache  *
*                configurations after flash operations.        *
****************************************************************/
void BootloaderFlash_RestoreCache(void){
/* Restore flash controller cache settings after flash operations */
RestoreFlashControllerCache(FLASH_PFCR1, g_pflash_pfcr1);
RestoreFlashControllerCache(FLASH_PFCR2, g_pflash_pfcr2);
}

/****************************************************************
 * Function Name: BootloaderFlash_Init                          *
 * Inputs       : None                                          *
 * Outputs      : status_t - Initialization status              *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Initializes the Bootloader Flash module.      *
 ****************************************************************/
status_t BootloaderFlash_Init(void)
{
	status_t returnCode = STATUS_SUCCESS;
	if (g_BootLoaderFlashModuleInit == false)
	{
		returnCode = FLASH_DRV_Init();
		if (returnCode == STATUS_SUCCESS)
		{
			g_BootLoaderFlashModuleInit = true;
		}
		else
		{
			/*Do Nothing*/
		}
	}
	else
	{
		/*Do Nothing*/
	}
	return returnCode;
}

/****************************************************************
 * Function Name: BootloaderFlash_Unlock                        *
 * Inputs       : None                                          *
 * Outputs      : status_t - Unlock operation status            *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Unlocks the first 256K blocks of the flash.   *
 ****************************************************************/
status_t BootloaderFlash_Unlock(void)
{
	status_t returnCode = STATUS_SUCCESS;
	returnCode = FLASH_DRV_SetLock(C55_BLOCK_256K_FIRST, UNLOCK_FIRST256_BLOCKS);
	return returnCode;
}

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
status_t BootloaderFlash_Erase(uint32_t a_Dest, uint32_t a_Size)
{
	status_t returnCode = STATUS_SUCCESS;
	uint32_t blkLockState; /* block lock status to be retrieved */
	uint32_t startAddress = a_Dest;
	uint32_t endAddress = startAddress + a_Size;
	uint32_t idx = 0U;
	uint32_t blocksToErase = 0U;
	flash_state_t opResult;
	flash_block_select_t blockSelect;

	/**************************************************************************/
	/* Find blocks to be erased corresponding to that range                   */
	/**************************************************************************/
	for (idx = 0U; idx < BOOTLOADER_FLASH_NUM_256KB_BLOCKS; idx++)
	{
		if ((g_BlocksStartAddresses[idx] <= startAddress) &&
			(g_BlocksStartAddresses[idx + 1] > startAddress))
		{
			blocksToErase |= (1 << idx);
		}
		else if ((startAddress < g_BlocksStartAddresses[idx]) &&
				 (endAddress >= g_BlocksStartAddresses[idx]))
		{
			blocksToErase |= (1 << idx);
		}
		else if (endAddress < g_BlocksStartAddresses[idx])
		{
			break;
		}
		else
		{
			/*Do Nothing*/
		}
	}

	if (blocksToErase == 0)
	{
		returnCode = STATUS_ERROR; /* Invalid Range */
		return returnCode;
	}

	/* Remove blocks that are already marked as erased */
	for (idx = 0U; idx < BOOTLOADER_FLASH_NUM_256KB_BLOCKS; idx++)
	{
		if ((blocksToErase & (1U << idx)) && BlockFlags_IsErased(idx))
		{
			/* Skip erasing this block as it's already erased */
			blocksToErase &= ~(1U << idx);
		}
	}

	if (blocksToErase == 0)
	{
		returnCode = STATUS_SUCCESS; /* Block was erased BEFORE*/
		return returnCode;
	}
	else
	{
		/* Do Nothing */
	}

	/**************************************************************************/
	/* Lock to protect UTest address space                                    */
	/**************************************************************************/
	returnCode = FLASH_DRV_GetLock(C55_BLOCK_UTEST, &blkLockState);
	if (!(blkLockState & 0x00000001U))
	{
		returnCode = FLASH_DRV_SetLock(C55_BLOCK_UTEST, 0x1U);
		if (STATUS_SUCCESS != returnCode)
		{
			while (1)
				;
		}
	}

	/**************************************************************************/
	/* Erase the selected blocks                                    		  */
	/**************************************************************************/
	/* Each bit corresponds to a block in the respective partitions:
	 *  - Low Blocks exist in partitions 0 and 1.
	 *  - Mid Blocks exist in partitions 2 and 3.
	 *  - HIGH Blocks exist in partitions 4 and 5.
	 *  - 256KB selects generally 256KB Blocks which are in partitions 6,7,8 & 9
	 *  */
	blockSelect.lowBlockSelect = 0x0U;				  /*No Blocks Selected from P0, P1*/
	blockSelect.midBlockSelect = 0x0U;				  /*No Blocks Selected from P2, P3*/
	blockSelect.highBlockSelect = 0x0U;				  /*No Blocks Selected from P4, P5*/
	blockSelect.first256KBlockSelect = blocksToErase; /*Selected from 256KB*/
	blockSelect.second256KBlockSelect = 0x0U;		  /*Reserved : MPC5748G has only 22 256KB blocks*/
	returnCode = FLASH_DRV_Erase(ERS_OPT_MAIN_SPACE, &blockSelect);
	do
	{
		/* Stay in loop till erase operation is complete */
		returnCode = FLASH_DRV_CheckEraseStatus(&opResult);
	} while (returnCode == STATUS_FLASH_INPROGRESS);

	/* If the erase was successful, mark the blocks as erased */
	if (returnCode == STATUS_SUCCESS)
	{
		for (idx = 0U; idx < BOOTLOADER_FLASH_NUM_256KB_BLOCKS; idx++)
		{
			if (blocksToErase & (1U << idx))
			{
				BlockFlags_SetErased(idx);
			}
		}
	}

	return returnCode;
}

/****************************************************************
 * Function Name: BootloaderFlash_VerifyBlank                   *
 * Inputs       : None                                          *
 * Outputs      : status_t - Verification status                *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Verifies if the specified flash block is blank*
 ****************************************************************/
status_t BootloaderFlash_VerifyBlank(uint32_t a_Dest, uint32_t a_Size)
{
	status_t returnCode = STATUS_SUCCESS;
	uint32_t numOfWordCycle = NUMBER_OF_WORD_BLANK_CHECK;
	uint32_t failedAddress;
	/* Blank check */
	returnCode = FLASH_DRV_BlankCheck(a_Dest,
									  a_Size,
									  numOfWordCycle,
									  &failedAddress,
									  NULL_CALLBACK);
	return returnCode;
}

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
status_t BootloaderFlash_Program(uint32_t a_dest, uint32_t a_size, uint32_t a_source)
{
	status_t returnCode = STATUS_SUCCESS;
	flash_state_t opResult; /* store the state of flash */
	do
	{
		/* Wait if there's a programming operation going on */
		returnCode = FLASH_DRV_CheckProgramStatus(&g_BootLoaderFlash_pCtxData, &opResult);
	} while (returnCode == STATUS_FLASH_INPROGRESS);

	returnCode = FLASH_DRV_Program(&g_BootLoaderFlash_pCtxData, a_dest, a_size, a_source);
	return returnCode;
}

/****************************************************************
 * Function Name: BootloaderFlash_ProgramVerify                 *
 * Inputs       : a_dest  - Destination address in flash        *
 *                a_size  - Size of data verified               *
 *                a_source- Source address of data              *
 * Outputs      : status_t - Verification status                *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Verifies the programmed data in flash.        *
 ****************************************************************/
status_t BootloaderFlash_ProgramVerify(uint32_t a_dest, uint32_t a_size, uint32_t a_source)
{
	status_t returnCode = STATUS_SUCCESS;
	flash_state_t opResult;	 /* store the state of flash */
	uint32_t numOfWordCycle; /* number of words per verification cycle */
	returnCode = FLASH_DRV_CheckProgramStatus(&g_BootLoaderFlash_pCtxData, &opResult);
	if (returnCode == STATUS_FLASH_INPROGRESS)
	{
		/*Do Nothing*/
	}
	else
	{
		/*Verify the programming*/
		numOfWordCycle = NUMBER_OF_WORD_PGM_VERIFY;
		/* Program verify */
		returnCode = FLASH_DRV_ProgramVerify(a_dest,
											 a_size,
											 a_source,
											 numOfWordCycle,
											 &g_BootLoaderFlash_failedAddress,
											 NULL_CALLBACK);
	}

	return returnCode;
}

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
status_t BootloaderFlash_Read(uint32_t a_dest, uint32_t a_size, uint32_t *a_pBuffer)
{
	status_t returnCode = STATUS_SUCCESS;		  /* return code */
	uint32_t destIndex = 0U;					  /* destination address index */
	uint32_t iterations = a_size / C55_WORD_SIZE; /* Number of words needs to be read */
	if (a_pBuffer == NULL)
	{
		returnCode = STATUS_ERROR;
	}

	/* Word by word verify */
	for (destIndex = 0U; destIndex < iterations; destIndex++)
	{
		a_pBuffer[destIndex] = *(volatile uint32_t *)a_dest;

		a_dest += C55_WORD_SIZE;
		a_size -= C55_WORD_SIZE;
	}
	return returnCode;
}

/****************************************************************
 * Function Name: BootloaderFlash_InitCRC                       *
 * Inputs       : None                                          *
 * Outputs      : status_t - Initialization status              *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Initializes CRC module using predefined       *
 *                instance and configuration.                   *
 ****************************************************************/
status_t BootloaderFlash_InitCRC(void)
{
	status_t returnCode = STATUS_SUCCESS; /* return code */
	returnCode = CRC_DRV_Init(BOOTLOADERFLASH_CRC_INSTANCE, &g_BootloaderFLashCRC_InitConfig);
	return returnCode;
}

/****************************************************************
 * Function Name: BootloaderFlash_CalculateCRC32                *
 * Inputs       : a_dest  - Source address in flash to read from*
 *                a_size  - Size of data to calculate CRC for   *
 *                          (in bytes)                          *
 * Outputs      : uint32_t - Calculated CRC32                   *
 * Reentrancy   : Non-Reentrant                                 *
 * Synchronous  : Synchronous                                   *
 * Description  : Calculates CRC32 for flashed data.            *
 ****************************************************************/
uint32_t BootloaderFlash_CalculateCRC32(uint32_t a_dest, uint32_t a_size)
{
	uint32_t destIndex = 0U;					  /* destination address index */
	uint32_t iterations = a_size / C55_WORD_SIZE; /* Number of words needs to be read */
	uint32_t flashWord;							  /* Word read from flash*/
	uint32_t resultCRC;
	/* Word by word verify */
	CRC_DRV_WriteData(BOOTLOADERFLASH_CRC_INSTANCE, a_dest, a_size);
	/*	for (destIndex = 0U; destIndex < iterations; destIndex++)
		{
			flashWord = *(volatile uint32_t *)a_dest;

			a_dest += C55_WORD_SIZE;
			a_size -= C55_WORD_SIZE;
		}*/
	resultCRC = CRC_DRV_GetCrcResult(BOOTLOADERFLASH_CRC_INSTANCE);

	return (resultCRC ^ 0xFFFFFFFF);
}
