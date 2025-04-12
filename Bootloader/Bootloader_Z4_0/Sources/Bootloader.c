#include "Bootloader.h"

/* Global pointer to bootloader data received from UDS */
static BL_Data *g_BLData = NULL;

/* External function to receive bootloader data via UDS */
extern BL_Data *UDS_BL_Receive(void);

/* Variables to store original flash controller cache configuration */
static volatile uint32_t pflash_pfcr1 = 0;
static volatile uint32_t pflash_pfcr2 = 0;

/* Global variable to track the return status of bootloader operations */
static status_t returnStatus;

// volatile uint32_t calculatedCRC32;

/* Global structure to store application-specific bootloader function handlers */
static BL_Functions g_UDSBootloaderHandler = {
	.BL_TransferDataHandler = NULL,
	.BL_Check_Memory = NULL,
	.BL_Erase_Memory = NULL};

status_t Bootloader_Init(BL_Functions *a_pBLHandlersConfig)
{
	/* Validate input handler configuration */
	if (a_pBLHandlersConfig == NULL ||
		a_pBLHandlersConfig->BL_TransferDataHandler == NULL ||
		a_pBLHandlersConfig->BL_Check_Memory == NULL ||
		a_pBLHandlersConfig->BL_Erase_Memory == NULL)
	{
		return STATUS_ERROR;
	}
	/* Store the provided bootloader function handlers for later use */
	g_UDSBootloaderHandler = *a_pBLHandlersConfig;

	/* Disable flash controller caches to ensure safe flash operations */
	DisableFlashControllerCache(FLASH_PFCR1, FLASH_FMC_BFEN_MASK, &pflash_pfcr1);
	DisableFlashControllerCache(FLASH_PFCR2, FLASH_FMC_BFEN_MASK, &pflash_pfcr2);

	/* Retrieve bootloader data via UDS and store it globally */
	g_BLData = UDS_BL_Receive();
	if (g_BLData == NULL)
	{
		return STATUS_ERROR;
	}

	/* Initialize the lower-level flash operations module */
	return BootloaderFlash_Init();
}

status_t Bootloader_Erase_Memory(void)
{
	/* Ensure that bootloader data is available */
	if (g_BLData == NULL)
	{
		return STATUS_ERROR;
	}

	/* Unlock flash memory for erase operation */
	returnStatus = BootloaderFlash_Unlock();
	if (returnStatus != STATUS_SUCCESS)
	{
		return returnStatus;
	}

	/* Erase flash memory starting at the specified address for the given size */
	returnStatus = BootloaderFlash_Erase(g_BLData->ers_mem_start_address, g_BLData->ers_total_size);
	if (returnStatus != STATUS_SUCCESS)
	{
		return returnStatus;
	}

	/* Verify that the erased flash memory is blank */
	return BootloaderFlash_VerifyBlank(g_BLData->ers_mem_start_address, g_BLData->ers_total_size);
}

status_t Bootloader_Program(void)
{
	/* Ensure that bootloader data is available */
	if (g_BLData == NULL)
	{
		return STATUS_ERROR;
	}

	/* Retrieve the pointer to the program data from the bootloader data structure */
	uint32_t *programData = g_BLData->data;

	/* Program flash memory with the provided data */
	returnStatus = BootloaderFlash_Program(g_BLData->mem_start_address, g_BLData->data_block_size, programData);
	if (returnStatus != STATUS_SUCCESS)
	{
		return returnStatus;
	}

	/* Verify that flash programming was successful */
	returnStatus = BootloaderFlash_ProgramVerify(g_BLData->mem_start_address, g_BLData->data_block_size, programData);
	while (returnStatus == STATUS_FLASH_INPROGRESS)
	{
		returnStatus = BootloaderFlash_ProgramVerify(g_BLData->mem_start_address, g_BLData->data_block_size, programData);
	}

	return returnStatus;
}

status_t Bootloader_CheckMemory(void)
{
	/* Ensure that bootloader data is available */
	if (g_BLData == NULL)
	{
		return STATUS_ERROR;
	}

	uint32_t prev_start_address = g_BLData->mem_start_address - g_BLData->total_size;

	/* Initialize CRC calculation module */
	BootloaderFlash_InitCRC();

	/* Compute CRC32 over the flash memory area defined by UDS bootloader data */
	uint32_t computed_crc = BootloaderFlash_CalculateCRC32(prev_start_address, g_BLData->total_size);

	/* Restore flash controller cache settings after flash operations */
	RestoreFlashControllerCache(FLASH_PFCR1, pflash_pfcr1);
	RestoreFlashControllerCache(FLASH_PFCR2, pflash_pfcr2);

	/* Compare the computed CRC with the expected CRC value from bootloader data */
	if (computed_crc == g_BLData->CRC_Field)
	{
		return STATUS_SUCCESS;
	}
	else
	{
		return STATUS_ERROR;
	}
}
