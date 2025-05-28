#include <stdint.h>
#include "MPC5748G.h"
#include "BootLoaderFLash.h"
#include "clockMan1.h"

#define FLASHED_ARRAY ((volatile uint32_t*)0x40040010)

static volatile uint32_t* const flashedArray = FLASHED_ARRAY;

static inline uint32_t RemapToUpdateRegion(uint32_t addr)
{
	return (addr & ~0x00F00000U) /* clear bits 20-23 */
			| (2U << 20);         /* set hex-digit-3 to 2 */
}


int main(void)
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  #ifdef PEX_RTOS_INIT
    PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of Processor Expert internal initialization.                    ***/

	status_t lastOperationStatus;

	BootloaderFlash_Init();

	/* Disable flash controller caches to ensure safe flash operations */
	BootloaderFlash_DisableCache();

	BootloaderFlash_Unlock();

	uint32_t eraseStartAddress = RemapToUpdateRegion(flashedArray[0]);

	BootloaderFlash_Erase(eraseStartAddress,flashedArray[1]);

	BootloaderFlash_VerifyBlank(eraseStartAddress,flashedArray[1]);

	int idx = 0;

	uint32_t programStartAddress;

	while(flashedArray[idx]!= 0x00000000)
	{
		programStartAddress = RemapToUpdateRegion(flashedArray[idx]);
		lastOperationStatus = BootloaderFlash_Program(programStartAddress, flashedArray[idx + 1], flashedArray[idx]);

		/* Verify that flash programming was successful */
		lastOperationStatus = BootloaderFlash_ProgramVerify(programStartAddress, flashedArray[idx + 1], flashedArray[idx]);

		while (lastOperationStatus == STATUS_FLASH_INPROGRESS)
		{
			lastOperationStatus = BootloaderFlash_ProgramVerify(programStartAddress, flashedArray[idx + 1], flashedArray[idx]);
		}

		/* Restore flash controller cache settings after flash operations */
		BootloaderFlash_RestoreCache();

		idx+=2;
	}

	*((volatile uint32_t *)0x4004000C) = 0x00000000;

	__asm__("e_lis %r12,0x00F9");
	__asm__("e_or2i %r12,0x8010");
	__asm__("e_lwz %r0,0(%r12) ");
	__asm__("mtlr %r0");
	__asm__("se_blrl");
}
