#include "Cpu.h"
#include "CanTP.h"
#include "UDS.h"
#include "flash_c55_driver.h"
#include "Bootloader.h"

#define BUFFER_SIZE_BYTE (252U)
#define BLOCK_START_ADDRS (0x01500000U)
#define BLOCK_END_ADDRS (0x01245000U)
// #define BOOTLOADER_FLASH_WORDSIZE 		4U

volatile int exit_code = 0;

uint8_t sendConsec = 0;
uint8_t send7mada = 2;
uint8_t seq_n = 1;

uint32_t dest = BLOCK_START_ADDRS;
uint8_t buffer[BUFFER_SIZE_BYTE];
int32_t size = BUFFER_SIZE_BYTE;
uint32_t idx = 0;
// volatile uint32_t calculatedCRC32;

void UDS_StubTask(void *params)
{
	can_buff_config_t buffConf = {false, false, 0xAA, CAN_MSG_ID_STD, false};
	CAN_ConfigTxBuff(&can_pal1_instance, 1, &buffConf);

	// PROG SESSION
	//                      status, msgID     FType-data size\	,     SID ,    Subfun ID,  parameters,  pad,                         Nbytes = 8
	while (1)
	{
		switch (send7mada)
		{
		case 1:;
			can_message_t message = {0, 0x33, {0x02, 0x10, 0x02, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 8};
			CAN_Send(&can_pal1_instance, 1, &message);
			vTaskDelay(pdMS_TO_TICKS(20));
			break;

		case 2:;
			// REQ DOWNLOAD
			//  mem address(start address) = 4 bytes .. mem size = 1 byte
			can_message_t message1 = {0, 0x33, {0x10, 0x08, 0x34, 0x00, 0x14, 0x01, 0x50, 0x00}, 8};
			CAN_Send(&can_pal1_instance, 1, &message1);
			vTaskDelay(pdMS_TO_TICKS(20));
			can_message_t message11 = {0, 0x33, {0x21, 0x00, 0xFC, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 8};
			CAN_Send(&can_pal1_instance, 1, &message11);
			vTaskDelay(pdMS_TO_TICKS(20));
			send7mada = 3;
			break;

		case 3:;
			// ERASE memory
			can_message_t message2 = {0, 0x33, {0x05, 0x31, 0x01, 0xff, 0x00, 0x00, 0xAA, 0xAA}, 8};
			CAN_Send(&can_pal1_instance, 1, &message2);
			vTaskDelay(pdMS_TO_TICKS(20));
			send7mada = 5;
			break;

		case 4:;
			// Stop erase memory
			can_message_t message3 = {0, 0x33, {0x05, 0x31, 0x02, 0xff, 0x00, 0x00, 0xAA, 0xAA}, 8};
			CAN_Send(&can_pal1_instance, 1, &message3);
			vTaskDelay(pdMS_TO_TICKS(20));
			break;

		case 5:;

			//		can_message_t message1 = {0, 0x33, {0x10,0x08,0x34,0x00,0x14,0x01,0x50,0x00}, 8};
			//		CAN_Send(&can_pal1_instance, 1, &message1);
			//		vTaskDelay(pdMS_TO_TICKS( 20 ));
			//		can_message_t message11 = {0, 0x33, {0x21,0x00,0xFF, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 8};
			//		CAN_Send(&can_pal1_instance, 1, &message11);

			can_message_t message4 = {0, 0x33, {0x11, 0x02, 0x36, 0x01, buffer[0], buffer[1], buffer[2], buffer[3]}, 8};
			size = size - 6;
			CAN_Send(&can_pal1_instance, 1, &message4);
			vTaskDelay(pdMS_TO_TICKS(20));
			int i = 4;
			while (size > 0)
			{
				message4.data[0] = 0x20 + seq_n;
				seq_n = (seq_n + 1) % 16;
				for (int j = 0; j < 7; j++)
				{
					message4.data[j + 1] = buffer[i + j];
				}
				CAN_Send(&can_pal1_instance, 1, &message4);
				size = size - 7;
				i = i + 7;
				vTaskDelay(pdMS_TO_TICKS(20));
			}
			// TRANSFER DATA                             seq_n=1
			//		can_message_t message4 = {0, 0x33, {0x07,0x36,seq_n,buffer[],0x44,0x44,0x44,0x44}, 8};
			//		CAN_Send(&can_pal1_instance, 1, &message4);
			//		seq_n++;
			vTaskDelay(pdMS_TO_TICKS(20));
			send7mada = 0;
			break;

		case 6:;
			// TRANSFER DATA 2                           seq_n=2
			can_message_t message5 = {0, 0x33, {0x07, 0x36, 0x02, 0x55, 0x66, 0x77, 0x88, 0x99}, 8};
			CAN_Send(&can_pal1_instance, 1, &message5);
			vTaskDelay(pdMS_TO_TICKS(20));
			break;

		case 7:;
			// START check memory
			can_message_t message6 = {0, 0x33, {0x05, 0x31, 0x01, 0xfe, 0x00, 0x00, 0xAA, 0xAA}, 8};
			CAN_Send(&can_pal1_instance, 1, &message6);
			vTaskDelay(pdMS_TO_TICKS(20));
			break;
		case 8:;
			// STOP check memory
			can_message_t message7 = {0, 0x33, {0x05, 0x31, 0x02, 0xfe, 0x00, 0x34, 0x56, 0x58}, 8};
			CAN_Send(&can_pal1_instance, 1, &message7);
			vTaskDelay(pdMS_TO_TICKS(20));
			break;

		case 9:;
			// Req Transfer Exit
			can_message_t message8 = {0, 0x33, {0x01, 0x37, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 8};
			CAN_Send(&can_pal1_instance, 1, &message8);
			vTaskDelay(pdMS_TO_TICKS(20));
			break;

		case 10:;
			// ECU Reset (Hard Reset)
			can_message_t message9 = {0, 0x33, {0x02, 0x11, 0x01, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 8};
			CAN_Send(&can_pal1_instance, 1, &message9);
			vTaskDelay(pdMS_TO_TICKS(20));
			break;

		case 11:;
			*(uint32_t *)(0x40040000) = 0x0;
			CAN_Deinit(&can_pal1_instance);
			PIT_DRV_Deinit(INST_PIT1);
			CRC_DRV_Deinit(INST_CRC1);
			__asm__("e_lis %r12,0x00F9");
			__asm__("e_or2i %r12,0x8010");
			__asm__("e_lwz %r0,0(%r12) ");
			__asm__("mtlr %r0");
			__asm__("se_blrl");
			break;
		}
	}
}

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
		PINS_DRV_TogglePins(PTH, (1 << 5));

		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

extern dataFrame requestFrame;
extern dataFrame responseFrame;

// uint32_t dest=BLOCK_START_ADDRS;
// uint8_t buffer [BUFFER_SIZE_BYTE];
// uint32_t size = BUFFER_SIZE_BYTE;
// uint32_t idx = 0;
// volatile uint32_t calculatedCRC32;

BL_Functions a_BLHandlersConfig = {.BL_TransferDataHandler = Bootloader_Program,
								   .BL_Check_Memory = Bootloader_CheckMemory,
								   .BL_Erase_Memory = Bootloader_Erase_Memory};

// volatile int a = 1;
int main(void)
{

	//	while (a);

	/* Write your local variable definition here */
	//	uint32_t dest=BLOCK_START_ADDRS;
	//	uint32_t buffer [BUFFER_SIZE_BYTE/BOOTLOADER_FLASH_WORDSIZE];
	//	uint32_t size = BUFFER_SIZE_BYTE/BOOTLOADER_FLASH_WORDSIZE;
	//	uint32_t idx = 0;
	//	uint32_t pflash_pfcr1, pflash_pfcr2;
	//	volatile uint32_t calculatedCRC32;
	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
#ifdef PEX_RTOS_INIT
	PEX_RTOS_INIT(); /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of Processor Expert internal initialization.
	 *                     ***/
	/* Initialize clock gate*/
	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
				   g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
	PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);

	// BOARD RECOVERY

	// Init CAN Stack

	/*
			status_t returnStatus = BootloaderFlash_Unlock();


			returnStatus = BootloaderFlash_Erase(0x1400000, 4000);


			int x = 0xAABBCCDD;
			int y[] = {1,2,3,4,5,6,7,8,1,2,3,4,5,6,7,8};
			int dest = 0x1409ff8;
			for (int i = 0 ; i < 16 ; i+=2){


			returnStatus = BootloaderFlash_Program(dest, 8, y+i);
			dest+=8;
			}*/
	// returnStatus = BootloaderFlash_Erase(0x1400000, 4000);

	Can_init(&can_pal1_instance, &can_pal1_Config0);

	CanTP_init(&responseFrame, &requestFrame);

	for (idx = 0; idx < size; idx = idx + 4)
	{
		buffer[idx] = 0xAA; /*Initialize Buffer*/
		buffer[idx + 1] = 0XBB;
		buffer[idx + 2] = 0XCC;
		buffer[idx + 3] = 0XDD;
	}

	//	BLOCK_END_ADDRS + 1 - BLOCK_START_ADDRS;
	//	size = BUFFER_SIZE_BYTE;/*Size in bytes for the program API*/

	// TODO fix passed parameter
	UDS_Init(&a_BLHandlersConfig);

	Bootloader_Init(&a_BLHandlersConfig);

	// xTaskCreate(UDS_StubTask,
	// 		"UDS_send example",
	// 		configMINIMAL_STACK_SIZE,
	// 		(void *) 0,
	// 		5,
	// 		NULL);

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
				"LED",
				configMINIMAL_STACK_SIZE,
				(void *)0,
				6,
				NULL);
	vTaskStartScheduler();

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
}
