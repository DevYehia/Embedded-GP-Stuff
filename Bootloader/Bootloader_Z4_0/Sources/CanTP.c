#include "CanTP.h"



can_instance_t* can_instance;
typedef enum CanTP_States {WAIT_FOR_FIRST, SEND_FLOW_CTL, WAIT_FOR_CONSECUTIVE} CANTP_States;
typedef enum CANTP_Frame_Types {SINGLE, FIRST,CONSECUTIVE ,FLOW} CANTP_Frame_Types;
uint32_t curr_buff_idx = 0;
uint16_t  dataSize = 0;



uint8_t ready = 0;

uint8_t prevblock = 0;

dataFrame* UDSFrame ;
uint8_t timeout = 0;
uint8_t timerStarted = 0;

dataFrame* sendingUDS;

can_buff_config_t buffConf = {false, false, 0xAA, CAN_MSG_ID_STD, false};
xSemaphoreHandle xBinarySemaphore;
can_message_t recvMessage;
void (* currState) ();

// handles canTP flow control frame sending
void send_flow_control(char type,uint32_t buffIdx)
{
	can_message_t message;
	message.id = 0x33;
	message.length = 8;
	if(type == 'E')
	{
		message.data[0] = 0x32;
	}
	if(type == 'A')
	{
		message.data[0] = 0x30;
	}
	message.data[1]= 0x00; // block size
	message.data[2] = 0x14; //st min
	for(int i = 3;i<8;i++)
	{
		message.data[i] = 0xAA;
	}
	CAN_Send(can_instance,buffIdx,&message);
}

// handles canTP single frame sending
void send_single_frame(uint8_t *payload,uint32_t buffIdx){
	can_message_t message;
	message.id = 0x33;      
	message.length = 8;
	message.data[0] = 0x0F & sendingUDS->dataSize;
	for(int i = 1;i<sendingUDS->dataSize + 1;i++)
	{
		message.data[i] = payload[i-1];
	}
	for(int i = sendingUDS->dataSize + 1;i<8;i++)
	{
		message.data[i] = 0xAA;
	}


	CAN_ConfigTxBuff(&can_pal1_instance, 1, &buffConf);
	CAN_Send(&can_pal1_instance,buffIdx,&message);
	sendingUDS->ready = 0;
}

// handles canTP consecutive frame sending
void send_consecutive_frame(uint8_t *payload,uint32_t buffIdx)
{
	can_message_t message;
	message.id = 0x33;      
	message.length = 8;
	uint8_t seq_n = 1;
	int32_t size = sendingUDS->dataSize;

	message.data[0] = 0x10 | sendingUDS->dataSize >>8;
	message.data[1] = sendingUDS->dataSize;
	message.data[2] = payload[0];
	message.data[3] = payload[1];
	message.data[4] = payload[2];
	message.data[5] = payload[3];
	message.data[6] = payload[4];
	message.data[7] = payload[5];

	size = sendingUDS->dataSize - 6;
	CAN_Send(&can_pal1_instance, 1, &message);

	BaseType_t takeStatus = xSemaphoreTake(xBinarySemaphore,(pdMS_TO_TICKS(5000)));

	if(takeStatus == pdPASS)
	{
		int i = 6;
		while(size>0)
		{
			message.data[0] = 0x20 + seq_n;
			seq_n = (seq_n + 1) % 16;
			int frame_message_size = 7;

			if(size < 7){
				frame_message_size = size;

			}
			for(int j = 0 ; j < frame_message_size ; j++){
				message.data[j+1]= payload[i+j];
			}
			if(size<7)
			{
				for(int i = frame_message_size + 1;i<8;i++)
				{
					message.data[i] = 0xAA;
				}
			}

			CAN_Send(&can_pal1_instance, 1, &message);
			size = size - 7;
			i = i+7;
			vTaskDelay(pdMS_TO_TICKS( 20 ));
		}
		UDSFrame->ready = 2;
	}
}

// reads canTP frame type
CANTP_Frame_Types get_type(can_message_t message)
{
	return message.data[0]>>4;
}

// reads canTP payload size 
uint16_t get_size(can_message_t message)
{

	uint16_t size = message.data[0] & 0xF;
	size = size<<8 | message.data[1];
	return size;
}

// reads canTP payload size 
uint8_t get_payload_size(uint8_t *payload){
	uint8_t size = 0;
	for(int i=0; i<7 ; i++){
		if(payload[i] != 0xAA){
			size++;
		}
	}
	return size;
}

// interrupt callback that is called when can interrupt happens
void interrupt_callback(uint32_t instance, can_event_t eventType, uint32_t buffIdx, void *driverState){
	if(eventType == CAN_EVENT_RX_COMPLETE){
		ready = 1;
		CAN_Receive(&can_pal1_instance, RX_BUFF_NUM, &recvMessage);
	}
	else if(eventType == CAN_EVENT_TX_COMPLETE){
		if(currState == handleFlowCtl){
			currState();
		}
	}
}




// main sending function that takes values from uds and sends it to can
void sendFromUDS(void * pv)
{
	if(sendingUDS->ready == 1)
	{
		if(sendingUDS->dataSize<8)
		{
			send_single_frame(sendingUDS->dataBuffer,TX_BUFF_NUM);
			sendingUDS->ready = 0;
		}
		if(sendingUDS->dataSize>=8)
		{
			send_consecutive_frame(sendingUDS->dataBuffer,TX_BUFF_NUM);
			sendingUDS->ready = 0;
		}
	}

}

// main recieve function that changes the states 
void recieve(void * pv)
{

	if(ready == 1)
	{
		timeout = 0;
		if(get_type(recvMessage) == SINGLE){
			handleSingleFrame();
		}
		else if(get_type(recvMessage)== FIRST){
			currState();
		}
		else if (get_type(recvMessage) == CONSECUTIVE)
		{
			currState();
		}
		else if (get_type(recvMessage) == FLOW)
		{
			if(recvMessage.data[0] == 0x30)
			{
				xSemaphoreGive(xBinarySemaphore);
			}
			if(recvMessage.data[0] == 0x32)
			{

			}
		}
		ready = 0;
	}

	if(timerStarted == 1)
	{
		timeout++;
		if(timeout >= 100)
		{
			timeOutHandle();
		}
	}
	//    }
}
//handles consecutive frame data reading and check for seq errors
void handleConsecutiveFrame(){
	CANTP_Frame_Types type = get_type(recvMessage);
	if(type == CONSECUTIVE){
		if((prevblock + 1 )%16 == (recvMessage.data[0] & 0X0F) )
		{
			prevblock= (prevblock + 1)%16;
			readCanTPPayload(consecutiveFrameSize,startConsecutive);
		}

		else if((prevblock + 1 )%16 != (recvMessage.data[0] & 0X0F) )
		{
			dataSize = 0;
			resetCanTP();
			UDSFrame->ready = 0;
			for(int i = 0; i<MAX_TP_SIZE;i++)
			{
				UDSFrame->dataBuffer[i] = 0;
			}
			send_flow_control('E',TX_BUFF_NUM);
			return;
		}

		//copy 7 bytes from received message to message buffer
		//stop when dataSize = 0 or 7 bytes are read
	}
	if(curr_buff_idx > UDSFrame->dataSize)
	{
		dataSize = 0;

		resetCanTP();
		UDSFrame->ready = 0;
		for(int i = 0; i<MAX_TP_SIZE;i++)
		{
			UDSFrame->dataBuffer[i] = 0;
		}
		send_flow_control('E',TX_BUFF_NUM);
		return;
	}
	if(dataSize == 0)
	{
		resetCanTP();
		UDSFrame->ready = 1;
	}
}
// resets canTP values
void resetCanTP(){
	currState = handleFirstFrame;
	curr_buff_idx = 0;
	timerStarted = 0;
	timeout = 0;
	prevblock = 0;
}

//handle single frames messages recieved
void handleSingleFrame(){
	uint8_t payload[7];
	uint8_t size = recvMessage.data[0];
	UDSFrame->dataSize = size;
	for(int i=0; i<size ; i++){
		UDSFrame->dataBuffer[i]  = recvMessage.data[i+1];
	}
	UDSFrame->ready = 1;

}

//changes the state to consecutive frame after sending flow control
void handleFlowCtl(){
	currState = handleConsecutiveFrame;
}

//Handles first frame recieved and sends flow control frame
void handleFirstFrame(){
	CANTP_Frame_Types type = get_type(recvMessage);
	if(type == FIRST){
		dataSize = get_size(recvMessage);
		UDSFrame->dataSize = dataSize;
		currState = handleFlowCtl;
		readCanTPPayload(firstFrameSize,startFirst);
		timerStarted = 1;
		send_flow_control('A',TX_BUFF_NUM);
	}
}


// resets canTP values and sends an error message to the sender
void timeOutHandle()
{
	dataSize = 0;
	resetCanTP();
	UDSFrame->ready = 0;
	UDSFrame->dataSize = 0;
	for(int i = 0; i<MAX_TP_SIZE;i++)
	{
		UDSFrame->dataBuffer[i] = 0;
	}
	send_flow_control('E',TX_BUFF_NUM);


}

//save payload
void readCanTPPayload(uint8_t size,uint8_t start)
{
	for(int i = 0 ;  i < size && dataSize > 0 ; i++){
		UDSFrame->dataBuffer[curr_buff_idx ++] = recvMessage.data[i + start];
		dataSize--;
	}
}

// CanTP init start with start state Handle first frame , init sending and recieve buffer for CanTP and UDS communication  
void CanTP_init(dataFrame* sendingBuffer, dataFrame* recieveBuffer)
{
	sendingUDS = sendingBuffer;
	UDSFrame = recieveBuffer;
	currState = handleFirstFrame;
	xBinarySemaphore = xSemaphoreCreateBinary();
}


// Can init for can instance , RX buff id and interrupt callback function
void Can_init(can_instance_t* can_pal_instance, can_user_config_t* can_pal_Config)
{
	can_instance = can_pal_instance;
	CAN_Init(&can_pal1_instance, &can_pal1_Config0);
	CAN_InstallEventCallback(&can_pal1_instance, interrupt_callback, NULL);
	CAN_ConfigRxBuff(&can_pal1_instance, RX_BUFF_NUM, &buffConf, 0x55);
	CAN_Receive(&can_pal1_instance, RX_BUFF_NUM, &recvMessage);
	CAN_ConfigTxBuff(&can_pal1_instance, 1, &buffConf);

}

