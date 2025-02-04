/*
 * Bootmanager.h
 *
 *  Created on: Feb 3, 2025
 *      Author: ideapad
 */

#ifndef BOOTMANAGER_H_
#define BOOTMANAGER_H_

#define EEPROM_FLAG_ADDRESS  0x00F90000  // Address from the datasheet
#define BOOTLOADER_FLAG_VALUE  0xA5      // Predefined flag value for bootloader mode
#define BOOTLOADER_START_ADDRESS  0x00FA4010  // Start address for bootloader
#define APPLICATION_START_ADDRESS  0x00FA0010// Start address for application

//typedef void (*reset_handler_t)(void);
typedef void (*void_fn)(void);
// Function prototypes
uint8_t read_eeprom_flag(uint32_t address);
void jump_to_address(uint32_t address);
void bootloader_function();
void application_function();


#endif /* BOOTMANAGER_H_ */
