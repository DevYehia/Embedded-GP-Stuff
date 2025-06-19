#include "Bootloader.h"

#define DECOMPRESSED_BUFFER_SIZE 5000

#define FLASHED_ARRAY ((volatile uint32_t*)0x40040010)

#define SIGNATURE_ARRAY ((volatile uint32_t*)0x40040108)

#define BOOTMANAGER_START_ADDRESS 0x01000000

#define BOOTMANAGER_END_ADDRESS 0x0103FFFF

#define BOOTLOADER_START_ADDRESS 0x01200000

#define BOOTLOADER_END_ADDRESS 0x0123FFFF


/* Global pointer to bootloader data received from UDS */
static BL_Data *gBootloaderData = NULL;

/* External function to receive bootloader data via UDS */
extern BL_Data *UDS_BL_Receive(void);

/* Tracks the most recent status of flash operations */
static status_t lastOperationStatus;

/* Cumulative size of data programmed into flash memory */
uint32_t totalProgramSize = 0;

/* Cumulative size of data programmed before padding into flash memory */
uint32_t totalActualSize = 0;

///* Indicates whether SHA-256 hash context has been initialized */
//static bool isHashInitialized = false;

/* Indicates that programming should target the bootloader-update region */
static bool gEnterBootloaderUpdateMode = false;

/* Holds the start address for Bootloader_Program (remapped if update mode is active) */
static uint32_t programStartAddress;

// size of the data block to program (raw or decompressed),
static uint32_t blockSize = 0;


static volatile uint32_t* const flashedArray = FLASHED_ARRAY;


static uint8_t index = 0;

/* Global structure to store application-specific bootloader function handlers */
static BL_Functions gBootloaderHandlers = {
		.BL_TransferDataHandler = NULL,
		.BL_Check_Memory = NULL,
		.BL_Erase_Memory = NULL,
		.BL_Finalize_Programming = NULL};


// Initialize the bootloader with application-provided handlers and disable flash caches
status_t Bootloader_Init(BL_Functions *a_pBLHandlersConfig)
{
	/* Validate input handler configuration */
	if (a_pBLHandlersConfig == NULL ||
			a_pBLHandlersConfig->BL_TransferDataHandler == NULL ||
			a_pBLHandlersConfig->BL_Check_Memory == NULL ||
			a_pBLHandlersConfig->BL_Erase_Memory == NULL ||
			a_pBLHandlersConfig->BL_Finalize_Programming == NULL)
	{
		return STATUS_ERROR;
	}
	/* Store the provided bootloader function handlers for later use */
	gBootloaderHandlers = *a_pBLHandlersConfig;

	/* Disable flash controller caches to ensure safe flash operations */
	BootloaderFlash_DisableCache();

	/* Retrieve bootloader data via UDS and store it globally */
	gBootloaderData = UDS_BL_Receive();
	if (gBootloaderData == NULL)
	{
		return STATUS_ERROR;
	}

	/* Initialize the lower-level flash operations module */
	return BootloaderFlash_Init();
}

// Erase the flash region specified by UDS bootloader data and verify blank state
status_t Bootloader_Erase_Memory(void)
{
	/* Ensure that bootloader data is available */
	if (gBootloaderData == NULL)
	{
		return STATUS_ERROR;
	}

	/* Unlock flash memory for erase operation */
	lastOperationStatus = BootloaderFlash_Unlock();
	if (lastOperationStatus != STATUS_SUCCESS)
	{
		return lastOperationStatus;
	}

	/* Determine start address for erase, possibly remapped for update region */
	uint32_t eraseStartAddress = gBootloaderData->ers_mem_start_address;
	if ((eraseStartAddress >= BOOTMANAGER_START_ADDRESS && eraseStartAddress <= BOOTMANAGER_END_ADDRESS) ||
			(eraseStartAddress >= BOOTLOADER_START_ADDRESS && eraseStartAddress <= BOOTLOADER_END_ADDRESS))
	{
		eraseStartAddress = RemapToUpdateRegion(eraseStartAddress);
		gEnterBootloaderUpdateMode = true; /* Enable update-mode remapping for subsequent operations */
		*((volatile uint32_t *)0x4004000C) = 0xBBBBBBBB;
	}

	else
	{
		*((volatile uint32_t *)0x40040008)= 0x11111111;
	}

	/* Erase flash memory starting at the specified address for the given size */
	lastOperationStatus = BootloaderFlash_Erase(eraseStartAddress, gBootloaderData->ers_total_size);
	if (lastOperationStatus != STATUS_SUCCESS)
	{
		return lastOperationStatus;
	}

	/* Verify that the erased flash memory is blank */
	return BootloaderFlash_VerifyBlank(eraseStartAddress, gBootloaderData->ers_total_size);
}

// Program the flash memory with data blocks from UDS, decompress if the compression flag is set, then verify
status_t Bootloader_Program(void)
{
	/* Ensure that bootloader data is available */
	if (gBootloaderData == NULL)
	{
		return STATUS_ERROR;
	}

	if (gBootloaderData->request_flag == 1)
	{
		totalProgramSize = 0;
		totalActualSize = 0;
	}

	uint32_t *programDataBuffer;

	if (gBootloaderData->compression_flag == 1)
	{
		char decompressedBuffer[DECOMPRESSED_BUFFER_SIZE] = {0};
		volatile uint32_t decompressedBytesCount = BLDecomp_Decompress(
				(const uint8_t *)gBootloaderData->data,
				(uint8_t *)decompressedBuffer,
				gBootloaderData->data_block_size);

		blockSize = decompressedBytesCount;
		totalActualSize += blockSize;
		uint8_t remainder = 0;
		remainder = blockSize % 4;
		if(remainder){
			uint8_t idx = blockSize;
			blockSize+= (4 - remainder);
			while(remainder--){
				decompressedBuffer[idx] = 0xFF;
				idx++;
			}
		}
		programDataBuffer = (uint32_t)decompressedBuffer;
	}
	else
	{
		/* No compression: program directly from received data buffer */
		programDataBuffer = (uint32_t *)gBootloaderData->data;
		blockSize = gBootloaderData->data_block_size;
		totalActualSize += blockSize;
		uint8_t remainder = 0;
		remainder = blockSize % 4;
		if(remainder)
			blockSize+= (4 - remainder);

	}

	totalProgramSize += blockSize;
	/* Calculate address for programming, remap if update mode is active */
	programStartAddress = gBootloaderData->mem_start_address;
	if (gEnterBootloaderUpdateMode)
	{
		programStartAddress = RemapToUpdateRegion(programStartAddress);
		flashedArray[index] = programStartAddress;
		flashedArray[index + 1] = blockSize;
		index += 2;
	}

	/* Program flash memory with the provided data */
	lastOperationStatus = BootloaderFlash_Program(programStartAddress, blockSize, programDataBuffer);
	if (lastOperationStatus != STATUS_SUCCESS)
	{
		return lastOperationStatus;
	}

	/* Verify that flash programming was successful */
	lastOperationStatus = BootloaderFlash_ProgramVerify(programStartAddress, blockSize, programDataBuffer);
	/* Wait until program verification is no longer in progress */
	while (lastOperationStatus == STATUS_FLASH_INPROGRESS)
	{
		lastOperationStatus = BootloaderFlash_ProgramVerify(programStartAddress, blockSize, programDataBuffer);
	}

	return lastOperationStatus;
}

// Verify programmed flash memory using CRC32 and update running SHA-256 hash
status_t Bootloader_CheckMemory(void)
{
	/* Ensure that bootloader data is available */
	if (gBootloaderData == NULL)
	{
		return STATUS_ERROR;
	}

	if(*((volatile uint32_t *)0x40040004) != 0xFFFFBBBB)
		Core_Boot();
	*((volatile uint32_t *)0x40040004) = 0xFFFFBBBB;

	/* Initialize CRC calculation module */
	BootloaderFlash_InitCRC();

	/* Compute CRC32 over the flash memory area defined by UDS bootloader data */
	uint32_t calculatedCrc = BootloaderFlash_CalculateCRC32(programStartAddress, totalProgramSize);

	/* Restore flash controller cache settings after flash operations */
	BootloaderFlash_RestoreCache();
	while(SIGNATURE_ARRAY[0] != 0x00000000 && SIGNATURE_ARRAY[1] != 0x00000000);
	SIGNATURE_ARRAY[0] = programStartAddress;
	SIGNATURE_ARRAY[1] = (size_t)totalActualSize;
	*((uint32_t *)0x40040100) = 0xAAAAAAAA;


	//	status_t hashStatus;
	//
	//	/* Initialize SHA-256 hash context if first pass */
	//	if (!isHashInitialized)
	//	{
	//		status_t hashStatus = BLSig_InitHash();
	//		isHashInitialized = true;
	//	}
	//
	//	if (!(isHashInitialized && hashStatus == STATUS_ERROR))
	//	{
	//		/* now update the running hash with this chunk */
	//		hashStatus = BLSig_UpdateHash((uint8_t *)(programStartAddress), (size_t)totalActualSize);
	//	}

	/* Compare the computed CRC with the expected CRC value from bootloader data */
	return (calculatedCrc == gBootloaderData->CRC_Field) ? STATUS_SUCCESS : STATUS_ERROR;
}

// Finalize programming by verifying ECDSA signature against computed hash
status_t Bootloader_Finalize_Programming(void)
{
	status_t signatureStatus = STATUS_ERROR;
	for (int i = 0; i < 64; i += 4) {
	    uint32_t word = ((uint32_t)gBootloaderData->signature[i]) |
	                    ((uint32_t)gBootloaderData->signature[i + 1] << 8) |
	                    ((uint32_t)gBootloaderData->signature[i + 2] << 16) |
	                    ((uint32_t)gBootloaderData->signature[i + 3] << 24);
	    SIGNATURE_ARRAY[i / 4 + 2] = word;
	}

	*((uint32_t *)0x40040100) = 0xFFFFFFFF;
	//	uint8_t hashDigest[32];
	//
	//	/* Finalize SHA-256 hash into local buffer */
	//	status_t signatureStatus = BLSig_FinalizeHash(hashDigest);
	//	if (signatureStatus != STATUS_SUCCESS)
	//	{
	//		return signatureStatus;
	//	}
	//
	//	/* Verify ECDSA signature provided in bootloader data */
	//	signatureStatus = BLSig_VerifySignature(hashDigest,
	//			sizeof(hashDigest),
	//			gBootloaderData->signature);

//	uint32_t signatureCompleted = *(volatile uint32_t *)0x40040100;
	while(*(volatile uint32_t *)0x40040100 != 0x00000000);
	if(*((uint32_t *)0x40040104) = 0x11111111)
	{
		*((uint32_t *)0x40040104) = 0x00000000;
		signatureStatus = STATUS_SUCCESS;
		*((volatile uint32_t *)0x40040008) = 0x00000000;
	}

	//	if (signatureStatus == STATUS_SUCCESS)
	//	{
	//		/* Write valid flag to a known memory location */
	//		*((volatile uint32_t *)0x40040008) = 0x00000000;
	//	}
	return signatureStatus;
}
