#include <stdint.h>
#include "MPC5748G.h"
#include "Bootmanager.h"
#include "clockMan1.h"

// __attribute__((section(".noinit")))
// volatile uint32_t initVAR;

/**
 * @brief Disable any reset escalation sources according to hardware datasheet.
 *
 * Clears the FRET and DRET escalation registers to prevent continuous reset loops.
 */
void DisableResetEscalation(void)
{
	/*Reset Escalation Registers*/
	uint8_t *const FRETRegister = (uint8_t *)0xFFFA8604;
	uint8_t *const DRETRegister = (uint8_t *)0xFFFA8608;

	/*Clearing those registers; see datasheet p2809, 2810*/
	*FRETRegister = 0U;
	*DRETRegister = 0U;
}

uint32_t readNoInitFlag(uint32_t address)
{
	volatile uint32_t *flagPtr = (volatile uint32_t *)address;
	return *flagPtr;
}

void jumpToAddress(void *address)
{

	// Disable interrupts globally before jumping
	INT_SYS_DisableIRQGlobal();

	__asm__ volatile(
		"or   %%r12, %0, %0    \n\t" // Move the passed-in address into r12
		"mtlr %%r12        \n\t"	 // Move the value from r12 into the link register
		"se_blrl           \n\t"	 // Branch to the address in the link register
		:							 // no outputs
		: "r"(address)				 // input: the address to load into r12
		: "r0", "r12", "lr"			 // clobbered registers
	);
}

void bootloaderHandler()
{
	/**
	 * @brief Jump to the bootloader image start address.
	 */
	jumpToAddress((void *)BOOTLOADER_START_ADDRESS);
}

void applicationHandler()
{
	/**
	 * @brief Jump to the main application image start address.
	 */
	jumpToAddress((void *)APPLICATION_START_ADDRESS);
}

void bluHandler()
{
	/**
	 * @brief Jump to the main blu image start address.
	 */
	jumpToAddress((void *)BLU_START_ADDRESS);
}


void Core_Boot(void)

{

	/* Enable e200z4b and e200z2 cores in RUN0-RUN3, DRUN and SAFE modes */
	uint32_t mctl = MC_ME->MCTL;

	MC_ME->CCTL2 = 0x00FEU;    /* e200z4b is active */

	/* Set start address for e200z4b and e200z2 cores */

	MC_ME->CADDR2 = 0x01040000 | 0x1U; /* e200z4b boot address + RMC bit */

	/* Mode change - re-enter the DRUN mode to start cores */


	MC_ME->MCTL = 0x30005AF0;         /* Mode & Key */

	MC_ME->MCTL = 0x3000A50F;        /* Mode & Key inverted */

	//	MC_ME->MCTL = (mctl & 0xffff0000ul) | KEY_VALUE1;
	//	MC_ME->MCTL =  mctl;

	while((MC_ME->GS & (1 << 27)) == 1);   /* Wait for mode entry complete */

	while (((MC_ME->GS >> 28) & 0xF) != 0x3); /* Check DRUN mode entered */

	//

}//Core_Boot
/**
 * @brief Entry point of the boot manager.
 *
 * Reads initialization flags, disables reset escalation, and
 * dispatches execution to either the bootloader or the main application.
 */
int main(void)
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  #ifdef PEX_RTOS_INIT
    PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of Processor Expert internal initialization.                    ***/
	uint32_t boot_flag = readNoInitFlag(NOINIT_FLAG_ADDRESS);
	uint32_t validity_flag = readNoInitFlag(NOINIT_FLAG_ADDRESS + 8);
	uint32_t blu_flag = readNoInitFlag(NOINIT_FLAG_ADDRESS + 12);

	DisableResetEscalation();

//	if(*((volatile uint32_t *)0x40040004) != 0xFFFFBBBB)
//		Core_Boot();
//	*((volatile uint32_t *)0x40040004) = 0xFFFFBBBB;
	//Core_Boot();
//	*((volatile uint32_t *)0x40040004) = 0xFFFFBBBB;

	if(blu_flag == BLU_FLAG_VALUE)
		bluHandler();
	if (boot_flag == BOOTLOADER_FLAG_VALUE || validity_flag != VALIDITY_FLAG_VALUE)
	{
		bootloaderHandler();
	}
	else
	{
		applicationHandler();
	}
	while (1)
		;
}
