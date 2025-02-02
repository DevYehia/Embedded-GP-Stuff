#include "CanTP.h"


extern uint8_t sendConsec;

can_instance_t* can_instance;
typedef enum CanTP_States {WAIT_FOR_FIRST, SEND_FLOW_CTL, WAIT_FOR_CONSECUTIVE} CANTP_States;
typedef enum CANTP_Frame_Types {SINGLE, FIRST,CONSECUTIVE ,FLOW} CANTP_Frame_Types;
uint32_t curr_buff_idx = 0;
uint16_t  dataSize = 0;

// uint8_t dataBuffer[MAX_TP_SIZE];

uint8_t ready = 0;

uint8_t prevblock = 0;

struct dataFrame UDSFrame ;
uint8_t timeout = 0;

struct dataFrame sendingUDS;

can_message_t recvMessage;
void (* currState) ();
void (* UDS_Callback)();

void send_flow_control(char type,uint32_t buffIdx)
{
    can_message_t message;
    message.id = 0x000;      //set later with tooling for HMI or application
    message.length = 8;
    if(type == 'E')
    {
        message.data[0] = 0x32;
    }
    if(type == 'A')
    {
        message.data[0] = 0x30;
    }
    message.data[1]= 0x04; // block size
    message.data[2] = 0x05;
    for(int i = 3;i<8;i++)
    {
        message.data[i] = 0xAA;
    }
    CAN_Send(can_instance,buffIdx,&message);
}

void send_single_frame(uint8_t *payload){
	can_message_t message;
	message.id = 0x000;      //set later with tooling for HMI or application
	message.length = 8;
	message.data[0] = get_payload_size(payload);
    for(int i = 1;i<8;i++)
    {
        message.data[i] = payload[i];
    }
}

CANTP_Frame_Types get_type(can_message_t message)
{
    return message.data[0]>>4;
}

uint16_t get_size(can_message_t message)
{
   // uint8_t testSize = message.data[0];
    uint16_t size = message.data[0];
    size = size<<12 | message.data[1];
    return size;
}

uint8_t get_payload_size(uint8_t *payload){
	uint8_t size = 0;
	for(int i=0; i<7 ; i++){
		if(payload[i] != 0xAA){
			size++;
		}
	}
	return size;
}

void interrupt_callback(uint32_t instance, can_event_t eventType, uint32_t buffIdx, void *driverState){
    if(eventType == CAN_EVENT_RX_COMPLETE){
        ready = 1;
        // if(get_type(recvMessage) == SINGLE){
        // 	handleSingleFrame();
        // }
        // else{
        // 	currState();
        // }
        // for(int i =0 ;i<64;i++)
        // {
        //     canTp.dataBuffer[i] = recvMessage.data[i];
        // }
	}
    // else if(eventType == CAN_EVENT_TX_COMPLETE){
    //     if(currState == handleFlowCtl){
    //         currState();
    //     }
    // }
}





void sendFromUDS(void * pv)
{
    if(sendingUDS.ready == 1)
    {
        send_single_frame(sendingUDS.dataBuffer);
    }
}


void recieve(void * pv)
{
    if(ready == 1)
    {
    if(get_type(recvMessage) == SINGLE){
        	handleSingleFrame();
        }
    else if(get_type(recvMessage)!= SINGLE){
        currState();
        }
        ready = 0;
    }
    //delay here 10ms
    timeout++;
    if(timeout >= 100)
    {
        timeOutHandle();
    }

}

void handleConsecutiveFrame(){
    CANTP_Frame_Types type = get_type(recvMessage);
    if(type == CONSECUTIVE){
         if((prevblock + 1 ) == (recvMessage.data[0] & 0X2F) )
         {
            readCanTPPayload(consecutiveFrameSize,startConsecutive);
         }

         else if((prevblock + 1 ) != (recvMessage.data[0] & 0X0F) )
         {
            dataSize = 0;
            currState = handleFirstFrame;
            curr_buff_idx = 0;
            UDSFrame.ready = 0;
            for(int i = 0; i<MAX_TP_SIZE;i++)
            {
            UDSFrame.dataBuffer[i] = 0;
            }
            send_flow_control('E',TX_BUFF_NUM);
         }
        
        /*if(dataSize >= 7)
        {
            
            dataSize = dataSize-7;
        }
        else
        {
            dataSize = 0;
        }*/

    //copy 7 bytes from received message to message buffer
    //stop when dataSize = 0 or 7 bytes are read
    }
    if(curr_buff_idx > dataSize)
    {
            dataSize = 0;
            currState = handleFirstFrame;
            curr_buff_idx = 0;
            UDSFrame.ready = 0;
            for(int i = 0; i<MAX_TP_SIZE;i++)
            {
            UDSFrame.dataBuffer[i] = 0;
            }
            send_flow_control('E',TX_BUFF_NUM);
    }
    if(dataSize == 0)
    {
        currState = handleFirstFrame;
        curr_buff_idx = 0;
        UDSFrame.ready = 1;
    }
    CAN_Receive(&can_pal1_instance, RX_BUFF_NUM, &recvMessage);
}

void handleSingleFrame(){
	uint8_t payload[7];
	uint8_t size = recvMessage.data[0];
	for(int i=0; i<size ; i++){
		UDSFrame.dataBuffer[i]  = recvMessage.data[i+1];
	}
    //UDS_Callback(payload);
    UDSFrame.ready = 1;
	CAN_Receive(&can_pal1_instance, RX_BUFF_NUM, &recvMessage);

}

void handleFlowCtl(){
    CAN_Receive(&can_pal1_instance, RX_BUFF_NUM, &recvMessage);
    currState = handleConsecutiveFrame;
    sendConsec = 1;
}

void handleFirstFrame(){
    CANTP_Frame_Types type = get_type(recvMessage);
    if(type == FIRST){
        dataSize = get_size(recvMessage);
        currState = handleFlowCtl;
        readCanTPPayload(firstFrameSize,startFirst);
        send_flow_control('A',TX_BUFF_NUM);
        }
}



void timeOutHandle()
{
   
            dataSize = 0;
            currState = handleFirstFrame;
            curr_buff_idx = 0;
            UDSFrame.ready = 0;
            for(int i = 0; i<MAX_TP_SIZE;i++)
            {
            UDSFrame.dataBuffer[i] = 0;
            }
            send_flow_control('E',TX_BUFF_NUM);
            timeout = 0x00;
            //delete self
}

//save payload
void readCanTPPayload(uint8_t size,uint8_t start)
{
    for(uint8_t i = 0 ;  i < size && dataSize > 0 ; i++){
            UDSFrame.dataBuffer[curr_buff_idx ++] = recvMessage.data[i + start];
            dataSize--;
       }
//        curr_buff_idx += size;
}

void CanTP_init(can_instance_t* can_pal_instance, can_user_config_t* can_pal_Config, void (*ptr_func)(uint8_t *) )
{
    UDS_Callback = ptr_func;
    can_instance = can_pal_instance;
    CAN_Init(can_pal_instance, can_pal_Config);
    CAN_InstallEventCallback(can_pal_instance, interrupt_callback, NULL);
    can_buff_config_t buffConf = {false, false, 0xAA, CAN_MSG_ID_STD, false};
    CAN_ConfigRxBuff(&can_pal1_instance, RX_BUFF_NUM, &buffConf, 0x3);
    CAN_Receive(&can_pal1_instance, RX_BUFF_NUM, &recvMessage);
    currState = handleFirstFrame;

}
