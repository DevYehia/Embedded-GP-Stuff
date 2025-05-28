/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
/* ###################################################################
 **     Filename    : main.c
 **     Processor   : MPC574xG
 **     Abstract    :
 **         Main module.
 **         This module contains user's application code.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
/*!
 ** @file main.c
 ** @version 01.00
 ** @brief
 **         Main module.
 **         This module contains user's application code.
 */
/*!
 **  @addtogroup main_module main module documentation
 **  @{
 */
/* MODULE main */


/* Including necessary module. Cpu.h contains other modules needed for compiling.*/
#include "Cpu.h"

volatile int exit_code = 0;
/* User includes (#include below this line is not maintained by Processor Expert) */

/*! 
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
 */

//#define PORT	PTH
//#define LED		5
//void delay(volatile int cycles)
//{
//    /* Delay function - do nothing for a number of cycles */
//    while(cycles--);
//}

static bool isHashInitialized = false;

#define SIGNATURE_ARRAY ((volatile uint32_t*)0x40040108)

int main(void)
{
	/* Write your local variable definition here */

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
#ifdef PEX_RTOS_INIT
	PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of Processor Expert internal initialization.                    ***/

	//	  PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);
	//	  while(1)
	//	  {
	/* Insert a small delay to make the blinking visible */
	//	      delay(720000);
	/* Toggle output value LED1 */
	//	      PINS_DRV_TogglePins(PORT, (1 << LED));
	//	  }
	while(1)
	{
		uint32_t hashOP = *(volatile uint32_t *)0x40040100;
		if(hashOP == 0xAAAAAAAA && SIGNATURE_ARRAY[0] != 0x00000000 && SIGNATURE_ARRAY[1] != 0x00000000)
		{
			status_t hashStatus;
			uint32_t programStartAddress = SIGNATURE_ARRAY[0];
			size_t totalActualSize = SIGNATURE_ARRAY[1];
			SIGNATURE_ARRAY[0] = 0x00000000;
			SIGNATURE_ARRAY[1] = 0x00000000;
			/* Initialize SHA-256 hash context if first pass */
			if (!isHashInitialized)
			{
				status_t hashStatus = BLSig_InitHash();
				isHashInitialized = true;
			}

			if (!(isHashInitialized && hashStatus == STATUS_ERROR))
			{
				/* now update the running hash with this chunk */
				hashStatus = BLSig_UpdateHash((uint8_t *)(programStartAddress), totalActualSize);
			}
		}
		if (hashOP == 0xFFFFFFFF && SIGNATURE_ARRAY[0] == 0x00000000 && SIGNATURE_ARRAY[1] == 0x00000000)
		{
			uint8_t hashDigest[32];

			/* Finalize SHA-256 hash into local buffer */
			status_t signatureStatus = BLSig_FinalizeHash(hashDigest);
			uint32_t high = SIGNATURE_ARRAY[3];
			uint32_t low  = SIGNATURE_ARRAY[2];

			uint64_t combined = ((uint64_t)high << 32) | low;

			uint8_t signature[64] = {0};
			for (int i = 0; i < 16; ++i) {
			    uint32_t word = SIGNATURE_ARRAY[i + 2];  // Read from index 2 to 11

			    signature[i * 4 + 0] = (uint8_t)(word & 0xFF);
			    signature[i * 4 + 1] = (uint8_t)((word >> 8) & 0xFF);
			    signature[i * 4 + 2] = (uint8_t)((word >> 16) & 0xFF);
			    signature[i * 4 + 3] = (uint8_t)((word >> 24) & 0xFF);
			}
			/* Verify ECDSA signature provided in bootloader data */
			signatureStatus = BLSig_VerifySignature(hashDigest,
					sizeof(hashDigest),
					signature);
			if(signatureStatus == STATUS_SUCCESS)
				*((uint32_t *)0x40040104) = 0x11111111;
			*(volatile uint32_t *)0x40040100 = 0x00000000;
		}
	}

	/* Write your code here */
	/* For example: for(;;) { } */

	/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;) {
    if(exit_code != 0) {
      break;
    }
  }
  return exit_code;
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
 ** @}
 */
/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.1 [05.21]
 **     for the NXP C55 series of microcontrollers.
 **
 ** ###################################################################
 */
