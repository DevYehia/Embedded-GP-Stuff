#ifndef BOOTMANAGER_H_
#define BOOTMANAGER_H_

#include <stdint.h>

// Address for storing the programming session flag (no-init section).
#define NOINIT_FLAG_ADDRESS 0x40040000

// Predefined flag value indicating bootloader mode.
#define BOOTLOADER_FLAG_VALUE 0xFFFFAAAA

// Flag value that indicates a valid application state.
#define VALIDITY_FLAG_VALUE 0x00000000

#define BLU_FLAG_VALUE 0xBBBBBBBB

// Start address for the bootloader image.
#define BOOTLOADER_START_ADDRESS 0x01200000

// Start address for the main application image.
#define APPLICATION_START_ADDRESS 0x01400000

// Start address for the main blu image.
#define BLU_START_ADDRESS 0x01540000

// Type definition for a function pointer with no parameters and no return.
typedef void (*VoidFunction)(void);

/**
 * @brief Reads a 32-bit flag from the no-init memory region.
 * @param address Memory address to read the flag from.
 * @return The 32-bit flag value stored at the specified address.
 */
uint32_t readNoInitFlag(uint32_t address);

/**
 * @brief Jumps execution to the specified code address.
 * @param address Pointer to the start of the code region.
 */
void jumpToAddress(void *address);

/**
 * @brief Handler that transfers execution to the bootloader.
 */
void bootloaderHandler(void);

/**
 * @brief Handler that transfers execution to the main application.
 */
void applicationHandler(void);

#endif /* BOOTMANAGER_H_ */
