#include <stdint.h>
#include <stdbool.h>
#include "MPC5748G.h"
#include "Bootmanager.h"
#include "clockMan1.h"



__attribute__((section(".noinit")))
volatile uint32_t initVAR;
void DisableResetEscalation (void){
	/*Reset Escalation Registers*/
	uint8_t * const FRETRegister = (uint8_t *)0xFFFA8604;
	uint8_t * const DRETRegister = (uint8_t *)0xFFFA8608;

	/*Clearing those registers; see datasheet p2809, 2810*/
	*FRETRegister = 0U;
	*DRETRegister = 0U;
}


int main(void) {

	//volatile uint32_t test = 1;
	//while(test){};
//	 *(volatile uint8_t *)NOINIT_FLAG_ADDRESS = BOOTLOADER_FLAG_VALUE;

	uint32_t boot_flag = read_noinit_flag(NOINIT_FLAG_ADDRESS);
//	uint32_t *p = &initVAR;
//	initVAR = 0xFFFFAAAA;

	//	__attribute__((section(".noinit")))


	DisableResetEscalation();
	*((volatile uint32_t *)0x40040004) = 0xFFFFBBBB;

	 if (boot_flag == BOOTLOADER_FLAG_VALUE) {
		bootloader_function();
	} else {
		application_function();
	}
	while (1);
}

uint32_t read_noinit_flag(uint32_t address) {
	volatile uint32_t *noinit_ptr = (volatile uint32_t *)address;
	return *noinit_ptr;
}

void jump_to_address(void  *address) {

//	uint32_t* reset_vector_entry = (uint32_t*) (address);
//	void_fn jump_fn = (void_fn)reset_vector_entry;
//	INT_SYS_DisableIRQGlobal();
//	jump_fn();

	INT_SYS_DisableIRQGlobal();
//	__asm__("e_lis %r12,0x00FA");
//	__asm__("e_or2i %r12,0x4010");
//	__asm__("e_lwz %r0,0(%r12) ");
//	__asm__("mtlr %r0");
//	__asm__("se_blrl");
//	 	MC_ME_ResetClockSourcesConfiguration(MC_ME);
//	    MC_ME_ResetPeripheralsConfiguration(MC_ME, (uint32_t) MC_ME_PERIPH_CONFIG_COUNT);
//
//	    for(uint8_t i = 0; i<8; i++)
//	    {
//	    	MC_ME->RUN_PC[i] = (uint32_t) 0x0;
//	    }
//	    for(uint8_t i = 0; i<8; i++)
//	    {
//	    	MC_ME->LP_PC[i] = (uint32_t) 0x0;
//	    }
//	    MC_ME->DRUN_MC = 0x00130000;
//	    MC_ME->MCTL = 0x30005AF0;
//	    MC_ME->MCTL = 0x3000A50F;
//	    uint32_t tmp = (MC_ME->GS & 0x08000000);
//	    while(tmp == 0x08000000)
//	    {
//	    tmp = (MC_ME->GS & 0x08000000);
//	    MC_ME->DMTS;
//	    }

	__asm__ volatile(
	    "or   %%r12, %0, %0    \n\t"  // Move the passed-in address into r12
	    "mtlr %%r12        \n\t"   // Move the value from r12 into the link register
	    "se_blrl           \n\t"   // Branch to the address in the link register
	    : // no outputs
	    : "r" (address) // input: the address to load into r12
	    : "r0", "r12", "lr" // clobbered registers
	);
}/*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

void bootloader_function() {
	jump_to_address((void *)BOOTLOADER_START_ADDRESS);
}

void application_function() {
	jump_to_address((void *)APPLICATION_START_ADDRESS);
}
