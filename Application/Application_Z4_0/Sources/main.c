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
#include "UDS.h"
#include "CanTP.h"
#include "FreeRTOS.h"
#include "pin_mux.h"

#define GREEN_PORT PTA
#define GREEN_LED 7
#define ORANGE_PORT PTJ
#define ORANGE_LED 4

volatile int exit_code = 0;
/* User includes (#include below this line is not maintained by Processor Expert) */

/*!
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
*/

uint8_t sendConsec = 0;

void sendFromUDS2(void *pv)
{
	while (1)
	{
		sendFromUDS(pv);
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
void recieve2(void *pv)
{
	while (1)
	{
		recieve(pv);
		UDS_Receive();
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void loopBackTask(void *params)
{

	can_message_t message = {0, 0x33, {0x02, 0x10, 0x02, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 8};

	can_buff_config_t buffConf = {false, false, 0xAA, CAN_MSG_ID_STD, false};
	CAN_ConfigTxBuff(&can_pal1_instance, 1, &buffConf);
	CAN_SendBlocking(&can_pal1_instance, 1, &message, 2000);

	// while(sendConsec == 0);
	can_message_t message1 = {0, 0x33, {0x04, 0x34, 0x00, 0x31, 0x12, 0x34, 0x56, 0x58}, 8};
	CAN_SendBlocking(&can_pal1_instance, 1, &message1, 2000);

	for (int i = 0; i < 2; i = (i + 1) % 16)
	{
		message.data[0] = 0x20 + i;
		// for(int j = 1 ; j < 7 ; j++)
		CAN_SendBlocking(&can_pal1_instance, 1, &message, 2000);
		vTaskDelay(pdMS_TO_TICKS(20));
	}

	while (1)
		;
}

void testSendConsec()
{
	uint8_t payload[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE};
	send_consecutive_frame(payload, 1);
}

void DisableResetEscalation(void)
{
	/*Reset Escalation Registers*/
	uint8_t *const FRETRegister = (uint8_t *)0xFFFA8604;
	uint8_t *const DRETRegister = (uint8_t *)0xFFFA8608;

	/*Clearing those registers; see datasheet p2809, 2810*/
	*FRETRegister = 0U;
	*DRETRegister = 0U;
}

void blink_led(void *params)
{
	while (1)
	{
		// PINS_DRV_TogglePins(GREEN_PORT, (1 << GREEN_LED));
		PINS_DRV_TogglePins(PTA, (1 << 4));

		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

extern dataFrame requestFrame;
extern dataFrame responseFrame;
volatile int a = 1;
int main(void)
{
	// volatile int a= 1;
	//	while(a);

	//	__asm__("e_lis %r12,0x00F9");
	//	__asm__("e_or2i %r12,0x8010");
	//	__asm__("e_lwz %r0,0(%r12) ");
	//	__asm__("mtlr %r0");
	//	__asm__("se_blrl");
/* Write your local variable definition here */

/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
#ifdef PEX_RTOS_INIT
	PEX_RTOS_INIT(); /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of Processor Expert internal initialization.                    ***/

	DisableResetEscalation();
	/* Initialize clocks */
	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
				   g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

	/* Write your code here */
	/* For example: for(;;) { } */

	//   CLOCK_DRV_Init(&clockMan1_InitConfig0);
	//   while(*((volatile uint32_t*) 0xffec0000) == 0);
	// UDS_Init(&can_pal1_instance, &can_pal1_Config0);
	PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);

	Can_init(&can_pal1_instance, &can_pal1_Config0);
	//    Can_init(&can_pal1_instance, &can_pal1_Config0);
	CanTP_init(&responseFrame, &requestFrame);

	//    UDS_Init();

	// xTaskCreate(UDS_StubTask,
	//     "UDS_send example",
	// 	configMINIMAL_STACK_SIZE,
	//     (void *) 0,
	//     5,
	//     NULL);

	/*xTaskCreate(testSendConsec,
		"green",
		configMINIMAL_STACK_SIZE,
		(void *) 0,
		5,
		NULL);*/

	xTaskCreate(recieve2,
				"TpReceieve",
				configMINIMAL_STACK_SIZE,
				(void *)0,
				6,
				NULL);

	xTaskCreate(sendFromUDS2,
				"TPSend",
				configMINIMAL_STACK_SIZE,
				(void *)0,
				6,
				NULL);

	xTaskCreate(blink_led,
				"LED_Blink",
				configMINIMAL_STACK_SIZE,
				(void *)0,
				6,
				NULL);
	//    xTaskCreate(UDS_Receive,
	//    		"UDSReceive",
	//    		configMINIMAL_STACK_SIZE,
	//        (void *) 0,
	//        2,
	//        NULL
	//    		);

	vTaskStartScheduler();

	// CAN_Init(&can_pal1_instance, &can_pal1_Config0);

/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
/*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
#ifdef PEX_RTOS_START
	PEX_RTOS_START(); /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of RTOS startup code.  ***/
	/*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
	for (;;)
	{
		if (exit_code != 0)
		{
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
