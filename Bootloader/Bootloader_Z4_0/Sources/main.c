#include "Cpu.h"
#include "CanTP.h"
#include "UDS.h"
#include "flash_c55_driver.h"
#include "Bootloader.h"

#define BUFFER_SIZE_BYTE                (252U)
#define COMPRESSED_SIZE_BYTE			(14U)
#define BLOCK_START_ADDRS               (0x01500000U)
#define BLOCK_END_ADDRS                 (0x01245000U)
//#define BOOTLOADER_FLASH_WORDSIZE 		4U

volatile int exit_code = 0;

uint8_t sendConsec = 0;
volatile uint8_t send7mada = 2;
uint8_t seq_n = 1;


uint32_t dest=BLOCK_START_ADDRS;
const uint8_t compressed[COMPRESSED_SIZE_BYTE] ={
		0x4f, 0xaa, 0xbb, 0xcc, 0xdd, 0x04, 0x00, 0xe0, 0x50, 0xdd, 0xaa, 0xbb, 0xcc, 0xdd
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
	//                      status, msgID     FType-data size\	,     SID ,    Subfun ID,  parameters,  pad,                         Nbytes = 8
	while(1){
		switch(send7mada){
		case 1: ;
		can_message_t message = {0,      0x55, {   0x02,                0x10,    0x02,                    0xAA,0xAA,0xAA,0xAA,0xAA},   8};
		CAN_Send(&can_pal1_instance, 1, &message);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		break;

		case 2: ;
		//REQ DOWNLOAD
		// mem address(start address) = 4 bytes .. mem size = 1 byte
		can_message_t message1 = {0, 0x55, {0x10, 0x08, 0x34, 0x10, 0x14, 0x01, 0x50, 0x00}, 8};
		CAN_Send(&can_pal1_instance, 1, &message1);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		can_message_t message11 = {0, 0x55, {0x21, 0x00, 0x0E, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message11);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 3;
		break;

		case 3: ;
		//ERASE memory
		status_t  returnStatus = BootloaderFlash_Unlock();
		returnStatus = BootloaderFlash_Erase(0x1500000, 4000);
		send7mada = 5;
		break;

		//		case 4: ;
		//		//Stop erase memory
		//		can_message_t message3 = {0, 0x55, {0x05,0x31,0x02,0xff,0x00,0x00,0xAA,0xAA}, 8};
		//		CAN_Send(&can_pal1_instance, 1, &message3);
		//		vTaskDelay(pdMS_TO_TICKS( 20 ));
		//		break;

		case 5: ;

		//		can_message_t message1 = {0, 0x55, {0x10,0x08,0x34,0x00,0x14,0x01,0x50,0x00}, 8};
		//		CAN_Send(&can_pal1_instance, 1, &message1);
		//		vTaskDelay(pdMS_TO_TICKS( 20 ));
		//		can_message_t message11 = {0, 0x55, {0x21,0x00,0xFF, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 8};
		//		CAN_Send(&can_pal1_instance, 1, &message11);

		can_message_t message4 = {0, 0x55, {0x10,0x10,0x36,0x01,compressed[0],compressed[1],compressed[2],compressed[3]}, 8};
		sizeCompressed = sizeCompressed - 6;
		CAN_Send(&can_pal1_instance, 1, &message4);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		int i = 4;
		while(sizeCompressed>0)
		{
			message4.data[0] = 0x20 + seq_n;
			seq_n = (seq_n + 1) % 16;
			for(int j = 0 ; j <7 ; j++){
				message4.data[j+1]= compressed[i+j];
			}
			CAN_Send(&can_pal1_instance, 1, &message4);
			sizeCompressed = sizeCompressed - 7;
			i = i+7;
			vTaskDelay(pdMS_TO_TICKS( 20 ));
		}
		//TRANSFER DATA                             seq_n=1
		//		can_message_t message4 = {0, 0x55, {0x07,0x36,seq_n,buffer[],0x44,0x44,0x44,0x44}, 8};
		//		CAN_Send(&can_pal1_instance, 1, &message4);
		//		seq_n++;
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 9;
		break;

		case 6: ;
		//TRANSFER DATA 2                           seq_n=2
		can_message_t message5 = {0, 0x55, {0x07,0x36,0x02,0x55,0x66,0x77,0x88,0x99}, 8};
		CAN_Send(&can_pal1_instance, 1, &message5);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		break;

		case 7: ;

		// START check memory
		can_message_t message6 = {0, 0x55, {0x10, 0x09, 0x31, 0x01, 0xFF, 0x01, 0x02, 0x47}, 8};
		CAN_Send(&can_pal1_instance, 1, &message6);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		can_message_t message66 = {0, 0x55, {0x21, 0x56, 0xEE, 0x79, 0xAA, 0xAA, 0xAA, 0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message66);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 13;
		break;
		case 8: ;
		// STOP check memory
		can_message_t message7 = {0, 0x55, {0x05,0x31,0x02,0xfe,0x00,0x34,0x56,0x58}, 8};
		CAN_Send(&can_pal1_instance, 1, &message7);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		break;

		case 9: ;
		//Req Transfer Exit
		can_message_t message8 = {0, 0x55, {0x01,0x37,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message8);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 7;
		break;

		case 10: ;
		//ECU Reset (Hard Reset)
		can_message_t message9 = {0, 0x55, {0x02,0x11,0x01,0xAA,0xAA,0xAA,0xAA,0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message9);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		break;

		case 11: ;
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

		case 12:; // Finalize programming
		uint8_t k = 1;
		uint8_t local_size = 63;
		uint8_t local_seq_n = 1;
		// Signature (64 bytes):
		// Signature (64 bytes):
		uint8_t signature[64] = {0xa8, 0xe3, 0xbb, 0x09, 0x4d, 0x46, 0xa9, 0x1f,
				0xd2, 0x8e, 0xe3, 0x88, 0xcc, 0x01, 0x2e, 0x22,
				0xc3, 0x92, 0xd2, 0x70, 0x89, 0x72, 0xd6, 0xba,
				0x87, 0xc9, 0x08, 0x61, 0x2b, 0xa8, 0xae, 0xa3,
				0xe9, 0xe1, 0x78, 0xc9, 0xaf, 0x3c, 0x74, 0x56,
				0x1c, 0xfc, 0xb3, 0x04, 0x72, 0x52, 0x11, 0xdf,
				0xaf, 0x6f, 0x2d, 0x75, 0x25, 0x75, 0xf4, 0xf5,
				0x42, 0x89, 0xbe, 0xc2, 0x11, 0xbd, 0xfd, 0x68 };
		//start frame + total size of can bus frames including headers = 0x050
		can_message_t message10 = {0, 0x55, {0x10, 0x45, 0x31, 0x01, 0xff, 0x02, 0x01, signature[0]}, 8};
		// can_message_t message10 = {0, 0x55, {0x02, 0x11, 0x01, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message10);
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
		break;
		case 13: ;

		//REQ DOWNLOAD
		// mem address(start address) = 4 bytes .. mem size = 1 byte
		can_message_t message22 = {0, 0x55, {0x10, 0x08, 0x34, 0x00, 0x14, 0x01, 0x50, 0x01}, 8};
		CAN_Send(&can_pal1_instance, 1, &message22);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		can_message_t message222 = {0, 0x55, {0x21, 0x00, 0xFC, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 8};
		CAN_Send(&can_pal1_instance, 1, &message222);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		send7mada = 14;
		break;

		case 14: ;
		seq_n = 1;
		int32_t size2 = 252;
		can_message_t message44 = {0, 0x55, {0x10,0xfe,0x36,0x01,buffer[0],buffer[1],buffer[2],buffer[3]}, 8};
		size2 = size2 - 6;
		CAN_Send(&can_pal1_instance, 1, &message44);
		vTaskDelay(pdMS_TO_TICKS( 20 ));
		int ii = 4;
		while(size2>0)
		{
			message44.data[0] = 0x20 + seq_n;
			seq_n = (seq_n + 1) % 16;
			for(int j = 0 ; j <7 ; j++){
				message44.data[j+1]= buffer[ii+j];
			}
			CAN_Send(&can_pal1_instance, 1, &message44);
			size2 = size2 - 7;
			ii = ii+7;
			vTaskDelay(pdMS_TO_TICKS( 20 ));
		}

		send7mada = 9;
		break;


		}
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
	PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of Processor Expert internal initialization.
	 *                     ***/
	/* Initialize clock gate*/
	CLOCK_SYS_Init(g_clockManConfigsArr,   CLOCK_MANAGER_CONFIG_CNT,
			g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
	PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);

	//BOARD RECOVERY

	//Init CAN Stack


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


	    int x = 0xAABBCCDD;
	    int y[] = {1,2,3,4,5,6,7,8,1,2,3,4,5,6,7,8};
	    int dest = 0x1409ff8;
	    for (int i = 0 ; i < 16 ; i+=2){


	    returnStatus = BootloaderFlash_Program(dest, 8, y+i);
	    dest+=8;
	    }*/
	//returnStatus = BootloaderFlash_Erase(0x1400000, 4000);


	Can_init(&can_pal1_instance, &can_pal1_Config0);

	CanTP_init(&responseFrame, &requestFrame);

	// for(idx=0;idx<size;idx = idx+4){
	// 	buffer[idx]=0xAA; /*Initialize Buffer*/
	// 	buffer[idx+1]=0XBB;
	// 	buffer[idx+2]=0XCC;
	// 	buffer[idx+3]=0XDD;
	// }

	//	BLOCK_END_ADDRS + 1 - BLOCK_START_ADDRS;
	//	size = BUFFER_SIZE_BYTE;/*Size in bytes for the program API*/

	//TODO fix passed parameter
	UDS_Init(&a_BLHandlersConfig);

	Bootloader_Init(&a_BLHandlersConfig);
//
//	*((volatile uint32_t *)0x40040008) = 0x11111111;

	//	status_t returnStatus = BootloaderFlash_Unlock();
	//
	//
	//    returnStatus = BootloaderFlash_Erase(0x1500000, 4000);
	//
	//
	//	returnStatus = BootloaderFlash_Program(0x1500000, 252, buffer);
	//
	//	returnStatus = BootloaderFlash_ProgramVerify(0x1500000, 252, buffer);
	//	while (returnStatus == STATUS_FLASH_INPROGRESS)
	//	{
	//		returnStatus = BootloaderFlash_ProgramVerify(0x1500000, 252, buffer);
	//	}

	// xTaskCreate(UDS_StubTask,
	// 		"UDS_send example",
	// 		800,
	// 		(void *) 0,
	// 		5,
	// 		NULL);


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
  return exit_code;
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
}
