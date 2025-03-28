#ifndef BOOTMANAGER_H_
#define BOOTMANAGER_H_

#define NOINIT_FLAG_ADDRESS  0x40040000  // Address containing the programming session flag
#define BOOTLOADER_FLAG_VALUE  0xFFFFAAAA    // Predefined flag value for bootloader mode
#define BOOTLOADER_START_ADDRESS  0x01200000//0x00FA4010  // Start address for bootloader
#define APPLICATION_START_ADDRESS  0x01400000//0x00FA0010 // Start address for application



typedef void (*void_fn)(void);
// Function prototypes
uint32_t read_noinit_flag(uint32_t address);
void jump_to_address(void *address);
void bootloader_function();
void application_function();


#endif /* BOOTMANAGER_H_ */
