#include "Cpu.h"
#include "CanTP.h"
#include "UDS.h"
#include "flash_c55_driver.h"
#include "Bootloader.h"
#include "stdlib.h"

#define BUFFER_SIZE_BYTE                (254U)
#define COMPRESSED_SIZE_BYTE			(14U)
#define BLOCK_START_ADDRS               (0x01500000U)
#define BLOCK_END_ADDRS                 (0x01245000U)
//#define BOOTLOADER_FLASH_WORDSIZE 		4U

volatile int exit_code = 0;

uint8_t sendConsec = 0;
volatile uint8_t send7mada = 1;
uint8_t seq_n = 1;


uint32_t dest=BLOCK_START_ADDRS;


const uint8_t compressed[COMPRESSED_SIZE_BYTE] ={
		0x4f, 0xaa, 0xbb, 0xcc, 0xdd, 0x04, 0x00, 0xe2, 0x50, 0xbb, 0xcc, 0xdd, 0xaa, 0xbb
};;
uint8_t buffer [BUFFER_SIZE_BYTE];
int32_t size = BUFFER_SIZE_BYTE;
int32_t sizeCompressed = COMPRESSED_SIZE_BYTE;
uint32_t idx = 0;
//volatile uint32_t calculatedCRC32;

void UDS_StubTask(void* params){
	can_buff_config_t buffConf = {false, false, 0xAA, CAN_MSG_ID_STD, false};
	CAN_ConfigTxBuff(&can_pal1_instance, 1, &buffConf);

	// PROG SESSION
	// status, msgID     FType-data size\	,     SID ,    Subfun ID,  parameters,  pad,   Nbytes = 8
	while(1){
		switch(send7mada){
		case 0: ;
		can_message_t message = {0,      0x55, { 0x02, 0x10,0x02, 0xAA,0xAA,0xAA,0xAA,0xAA},   8};
		CAN_Send(&can_pal1_instance, 1, &message);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		break;

		case 1: ;
		//generate random seed
		srand(*((uint32_t *) (0x40040010)));
		volatile uint32_t seed = rand() % 0x10000;
		// Level-specific masks
		uint32_t level_masks[] = {
				0x00000000, // Level 0 (unused)
				0x5A5A5A5A, // Level 1
				0xA5A5A5A5, // Level 2
				0x12345678, // Level 3 (programming)
				0x87654321  // Level 4+
		};

		uint32_t key = seed;

		// 1. Bit rotation based on level
		uint8_t rotation = (1 * 3) % 32;
		key = (key << rotation) | (key >> (32 - rotation));

		// 2. XOR with level-specific mask
		key ^= level_masks[1];

		// 3. Simple scrambling
		key = ((key & 0xAAAAAAAA) >> 1) | ((key & 0x55555555) << 1);

		// 4. Final XOR with inverted seed
		key ^= ~seed;

		//		volatile uint32_t v1 = PIT_DRV_GetCurrentTimerCount(INST_PIT1, 3);
		can_message_t message0 = {0, 0x55, {0x04,0x00,0x55,0x27,0x01,0xAA,0xAA,0xAA},8};
		CAN_Send(&can_pal1_instance, 1, &message0);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		can_message_t message0_2 = {0, 0x55, {0x10,0x08,0x00,0x55,0x27,0x02,(key >> 24) & 0xFF,(key >> 16) & 0xFF},8};
		CAN_Send(&can_pal1_instance, 1, &message0_2);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		can_message_t message0_3 = {0, 0x55, {0x21,(key >> 8)& 0xFF,key & 0xFF,0xAA,0xAA,0xAA,0xAA,0xAA},8};
		CAN_Send(&can_pal1_instance, 1, &message0_3);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 2;
		break;

		case 2: ;
		volatile uint32_t v1 = PIT_DRV_GetCurrentTimerCount(INST_PIT1, 3);
		//		v1 = 30;
		//REQ DOWNLOAD
		// mem address(start address) = 4 bytes .. mem size = 1 byte
		can_message_t message1 = {0, 0x55, {0x10, 0x0A, 0x00, 0x55, 0x34, 0x10, 0x14, 0x01}, 8};
		CAN_Send(&can_pal1_instance, 1, &message1);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		can_message_t message1_2 = {0, 0x55, {0x21,0x20, 0x00,0x00, 0x0E, 0xAA, 0xAA,0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message1_2);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 3;
		break;

		case 3: ;
		can_message_t message2 = {0, 0x55, {0x10,0x0C, 0x00, 0x55, 0x31, 0x01, 0xff, 0x00}, 8};
		CAN_Send(&can_pal1_instance, 1, &message2);
		vTaskDelay(pdMS_TO_TICKS(20));
		can_message_t message2_2 = {0, 0x55, {0x21,0x14, 0x01,0x20,0x00,0x00,0x0E, 0xAA, 0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message2_2);
		vTaskDelay(pdMS_TO_TICKS(20));
		send7mada = 4;
		break;
		//				ERASE memory
		//		status_t  returnStatus = BootloaderFlash_Unlock();
		//		returnStatus = BootloaderFlash_Erase(0x1500000, 4000);
		//		send7mada = 4;
		//		break;

		case 4: ;

		can_message_t message3 = {0, 0x55, {0x10,0x12,0X00,0X55,0x36,0x01,compressed[0],compressed[1]}, 8};
		sizeCompressed = sizeCompressed - 2;
		CAN_Send(&can_pal1_instance, 1, &message3);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		int i = 2;
		while(sizeCompressed>0)
		{
			message3.data[0] = 0x20 + seq_n;
			seq_n = (seq_n + 1) % 16;
			for(int j = 0 ; j <7 ; j++){
				message3.data[j+1]= compressed[i+j];
			}
			CAN_Send(&can_pal1_instance, 1, &message3);
			sizeCompressed = sizeCompressed - 7;
			i = i+7;
			vTaskDelay(pdMS_TO_TICKS( 20 ));
		}
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 5;
		break;

		case 5: ;
		//Req Transfer Exit
		can_message_t message4 = {0, 0x55, {0x03,0x00,0x55,0x37,0xAA,0xAA,0xAA,0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message4);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 6;
		break;

		case 6: ;
		// START check memory
		can_message_t message5 = {0, 0x55, {0x10, 0x0B,0X00,0X55, 0x31, 0x01, 0xFF, 0x01}, 8};
		CAN_Send(&can_pal1_instance, 1, &message5);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		can_message_t message5_2 = {0, 0x55, {0x21,0x02, 0xC5, 0xAD, 0x19, 0x2E, 0xAA, 0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message5_2);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 7;
		break;

		case 7: ;
		//REQ DOWNLOAD 2
		can_message_t message6 = {0, 0x55, {0x10,0x0A,0x00, 0x55, 0x34, 0x00, 0x14, 0x01}, 8};
		CAN_Send(&can_pal1_instance, 1, &message6);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		can_message_t message6_2 = {0, 0x55, {0x21,0x20, 0x01 ,0x00, 0xFE, 0xAA, 0xAA, 0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message6_2);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 8;
		break;


		case 8:;
		seq_n = 1;
		int32_t size2 = 254;
		can_message_t message7 = {0, 0x55, {0x11,0x02,0X00,0X55,0x36,0x01,buffer[0],buffer[1]}, 8};
		size2 = size2 - 2;
		CAN_Send(&can_pal1_instance, 1, &message7);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		int ii = 2;
		while(size2>0)
		{
			message7.data[0] = 0x20 + seq_n;
			seq_n = (seq_n + 1) % 16;
			for(int j = 0 ; j <7 ; j++){
				message7.data[j+1]= buffer[ii+j];
			}
			CAN_Send(&can_pal1_instance, 1, &message7);
			size2 = size2 - 7;
			ii = ii+7;
			vTaskDelay(pdMS_TO_TICKS( 20 ));
		}
		send7mada = 9;
		break;

		case 9:;
		//Req Transfer Exit
		can_message_t message8 = {0, 0x55, {0x03,0x00,0x55,0x37,0xAA,0xAA,0xAA,0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message8);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 10;
		break;

		case 10:;
		// START check memory
		can_message_t message9 = {0, 0x55, {0x10, 0x0B,0X00,0X55, 0x31, 0x01, 0xFF, 0x01}, 8};
		CAN_Send(&can_pal1_instance, 1, &message9);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		can_message_t message9_2 = {0, 0x55, {0x21,0x02, 0xC5, 0xAD, 0x19, 0x2E, 0xAA, 0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message9_2);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 11;
		break;


		case 11:; // Finalize programming
		//		volatile uint32_t v2 = xTaskGetTickCount();
		uint8_t k = 6;
		uint8_t local_size = 58;
		uint8_t local_seq_n = 2;
		// Signature (64 bytes):
		uint8_t signature[64] = {0xa3, 0x3b, 0x06, 0x79, 0xed, 0xfc, 0xda, 0x28,
				0x2a, 0x5b, 0x70, 0xff, 0xb8, 0x05, 0xc3, 0xe5,
				0xac, 0x71, 0xbc, 0x25, 0x8e, 0x5f, 0x37, 0xe0,
				0x61, 0xba, 0xce, 0x14, 0x84, 0x23, 0x47, 0x40,
				0x45, 0x54, 0x16, 0xac, 0x03, 0x09, 0x54, 0x83,
				0x58, 0xfc, 0xe6, 0x2c, 0x24, 0xc3, 0xba, 0x55,
				0xb1, 0x31, 0x00, 0xd3, 0xe8, 0x19, 0x9d, 0x23,
				0xa2, 0xae, 0xca, 0x2f, 0x44, 0x83, 0xbf, 0xed};
		//start frame + total size of can bus frames including headers = 0x050
		can_message_t message10 = {0, 0x55, {0x10,0x47,0x00, 0x55, 0x31, 0x01, 0xff, 0x02}, 8};
		// can_message_t message10 = {0, 0x55, {0x02, 0x11, 0x01, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message10);
		vTaskDelay(pdMS_TO_TICKS(20));
		can_message_t message10_2 = {0, 0x55, {0x21, 0x01, signature[0], signature[1], signature[2], signature[3], signature[4], signature[5]}, 8};
		CAN_Send(&can_pal1_instance, 1, &message10_2);
		vTaskDelay(pdMS_TO_TICKS(20));
		while (local_size > 0)
		{
			message10.data[0] = 0x20 + local_seq_n;
			local_seq_n = (local_seq_n + 1) % 16;
			for (int j = 0; j < 7 && local_size > 0 ; j++)
			{
				message10.data[j + 1] = signature[k + j];
				local_size--;
			}
			CAN_Send(&can_pal1_instance, 1, &message10);
			// local_size = local_size - 7;
			k = k + 7;
			vTaskDelay(pdMS_TO_TICKS(20));
		}
		vTaskDelay(pdMS_TO_TICKS(100));

		send7mada = 12;
		break;

		case 12: ;
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
		//		case 10: ;
		//		//ECU Reset (Hard Reset)
		//		can_message_t message10 = {0, 0x55, {0x02,0x11,0x01,0xAA,0xAA,0xAA,0xAA,0xAA}, 8};
		//		CAN_Send(&can_pal1_instance, 1, &message10);
		//		vTaskDelay(pdMS_TO_TICKS( 20 ));
		//		break;
		//
	}
}
void sendFromUDS2(void * pv){
	while(1){
		sendFromUDS(pv);
		vTaskDelay(pdMS_TO_TICKS( 10 ));
	}
}
void recieve2(void * pv){
	while(1){
		recieve(pv);
		UDS_Receive();
		vTaskDelay(pdMS_TO_TICKS( 10 ));
	}
}

void loopBackTask(void* params){

	can_message_t message = {0, 0x55, {0x02,0x10,0x02,0xAA,0xAA,0xAA,0xAA,0xAA}, 8};

	can_buff_config_t buffConf = {false, false, 0xAA, CAN_MSG_ID_STD, false};
	CAN_ConfigTxBuff(&can_pal1_instance, 1, &buffConf);
	CAN_SendBlocking(&can_pal1_instance, 1, &message, 2000);

	//while(sendConsec == 0);
	can_message_t message1 = {0, 0x55, {0x04,0x34,0x00,0x31,0x12,0x34,0x56,0x58}, 8};
	CAN_SendBlocking(&can_pal1_instance, 1, &message1, 2000);

	for(int i = 0;i<2; i = (i + 1) % 16){
		message.data[0] = 0x20 + i;
		//for(int j = 1 ; j < 7 ; j++)
		CAN_SendBlocking(&can_pal1_instance, 1, &message, 2000);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
	}

	while(1);
}

void DisableResetEscalation (void){
	/*Reset Escalation Registers*/
	uint8_t * const FRETRegister = (uint8_t *)0xFFFA8604;
	uint8_t * const DRETRegister = (uint8_t *)0xFFFA8608;

	/*Clearing those registers; see datasheet p2809, 2810*/
	*FRETRegister = 0U;
	*DRETRegister = 0U;
}

void blink_led(void* params){
	while(1){
		//PINS_DRV_TogglePins(GREEN_PORT, (1 << GREEN_LED));
		PINS_DRV_TogglePins(PTH, (1 << 5));

		vTaskDelay(pdMS_TO_TICKS(2000));
	}
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

extern dataFrame requestFrame;
extern dataFrame responseFrame;


//uint32_t dest=BLOCK_START_ADDRS;
//uint8_t buffer [BUFFER_SIZE_BYTE];
//uint32_t size = BUFFER_SIZE_BYTE;
//uint32_t idx = 0;
//volatile uint32_t calculatedCRC32;

BL_Functions a_BLHandlersConfig = {.BL_TransferDataHandler = Bootloader_Program,
		.BL_Check_Memory = Bootloader_CheckMemory,
		.BL_Erase_Memory = Bootloader_Erase_Memory,
		.BL_Finalize_Programming = Bootloader_Finalize_Programming};

// volatile int a = 1;
int main(void)
{
	/* Write your local variable definition here */

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
#ifdef PEX_RTOS_INIT
	PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of Processor Expert internal initialization.                    ***/

	/* Write your code here */
	/* For example: for(;;) { } */

	/* Initialize clock gate*/
	CLOCK_SYS_Init(g_clockManConfigsArr,   CLOCK_MANAGER_CONFIG_CNT,
			g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
	PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);
	//Init CAN Stack
	Can_init(&can_pal1_instance, &can_pal1_Config0);
	CanTP_init(&responseFrame, &requestFrame);
	UDS_Init(&a_BLHandlersConfig);
	Bootloader_Init(&a_BLHandlersConfig);



	//BOARD RECOVERY
	//		status_t  returnStatus = BootloaderFlash_Unlock();
	//	    returnStatus = BootloaderFlash_Erase(0x1400000, 4000);
	//	    returnStatus = BootloaderFlash_VerifyBlank(0x1400000,4000);
	//	    if (returnStatus == STATUS_SUCCESS)
	//	    {
	//	    	while(1);
	//	    }
	/*
		status_t returnStatus = BootloaderFlash_Unlock();
	    returnStatus = BootloaderFlash_Erase(0x1400000, 4000);
	 */

	// Test Code
	//	for(idx=0;idx<size - 2;idx = idx+4){
	//		buffer[idx]=0xAA; /*Initialize Buffer*/
	//		buffer[idx+1]=0XBB;
	//		buffer[idx+2]=0XCC;
	//		buffer[idx+3]=0XDD;
	//	}
	//	buffer[252] = 0xAA;
	//	buffer[253] = 0xBB;

	//	BLOCK_END_ADDRS + 1 - BLOCK_START_ADDRS;
	//	size = BUFFER_SIZE_BYTE;/*Size in bytes for the program API*/

	//	xTaskCreate(UDS_StubTask,
	//			"UDS_send example",
	//			800,
	//			(void *) 0,
	//			5,
	//			NULL);


	xTaskCreate(recieve2,
			"TpReceieve",
			2048,
			(void *) 0,
			6,
			NULL);

	xTaskCreate(sendFromUDS2,
			"TPSend",
			configMINIMAL_STACK_SIZE,
			(void *) 0,
			6,
			NULL);

	xTaskCreate(blink_led,
			"LED",
			configMINIMAL_STACK_SIZE,
			(void *) 0,
			6,
			NULL);
	vTaskStartScheduler();

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
  return exit_code;}

  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
