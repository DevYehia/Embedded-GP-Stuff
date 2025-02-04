/*
 * main implementation: use this 'C' sample to create your own application
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include "MPC5748G.h"
#include "Bootmanager.h"

int main() {

   // *(volatile uint8_t *)EEPROM_FLAG_ADDRESS = BOOTLOADER_FLAG_VALUE;


   // uint8_t boot_flag = read_eeprom_flag(EEPROM_FLAG_ADDRESS);


   // if (boot_flag == BOOTLOADER_FLAG_VALUE) {
    if (1) {
        bootloader_function();
    } else {
        application_function();
    }

    while (1);
}

uint8_t read_eeprom_flag(uint32_t address) {
    volatile uint8_t *eeprom_ptr = (volatile uint8_t *)address;
    return *eeprom_ptr;
}

void jump_to_address(uint32_t address) {

//	uint32_t* reset_vector_entry = (uint32_t*) (address);
//	void_fn jump_fn = (void_fn)reset_vector_entry;
//	INT_SYS_DisableIRQGlobal();
//	jump_fn();

		INT_SYS_DisableIRQGlobal();
		__asm__("e_lis %r12,0x00FA");
		__asm__("e_or2i %r12,0x4010");
		__asm__("e_lwz %r0,0(%r12) ");
		__asm__("mtlr %r0");
		__asm__("se_blrl");
}

void bootloader_function() {
    jump_to_address(BOOTLOADER_START_ADDRESS);
}

void application_function() {
    jump_to_address(APPLICATION_START_ADDRESS);
}
