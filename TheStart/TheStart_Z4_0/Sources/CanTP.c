#include "CanTP.h"




can_instance_t* can_instance;
typedef enum CanTP_States {WAIT_FOR_FIRST, SEND_FLOW_CTL, WAIT_FOR_CONSECUTIVE} CANTP_States;
typedef enum CANTP_Frame_Types {SINGLE, FIRST,CONSECUTIVE ,FLOW} CANTP_Frame_Types;
uint32_t consecutive_cntr = 0;
CANTP_States currState = WAIT_FOR_FIRST;
uint16_t  dataSize = 0;
void CanTP_init(can_instance_t* can_pal_instance, can_user_config_t* can_pal_Config){

    can_instance = can_pal_instance;
    CAN_Init(can_pal_instance, can_pal_Config);
    CAN_InstallEventCallback(can_pal_instance, interrupt_callback, NULL);
}

void send_flow_control(uint32_t buffIdx)
{
    can_message_t message;
    message.id = 0x000;      //set later with tooling for HMI or application
    message.length = 8;
    message.data[0] = 0x30;
    message.data[1]= 0x04; // block size
    message.data[2] = 0x05;
    for(int i = 3;i<8;i++)
    {
        message.data[i] = 0xAA;
    }
    CAN_Send(can_instance,buffIdx,&message);
}

CANTP_Frame_Types get_type(can_message_t message)
{
    return message.data[0]>>4;
}

uint16_t get_size(can_message_t message)
{
    uint16_t size = message.data[0];
    size = size<<12 | message.data[1];
    return size;
}

void interrupt_callback(uint32_t instance, can_event_t eventType, uint32_t buffIdx, void *driverState){
    if(eventType == CAN_EVENT_RX_COMPLETE){
        can_message_t message;
        CAN_Receive(can_instance,buffIdx,&message);
        CANTP_Frame_Types type = get_type(message);
        if(currState == WAIT_FOR_FIRST){
            if(type == FIRST){
                dataSize = get_size(message);
                dataSize = dataSize - 6;
                currState = SEND_FLOW_CTL;
                send_flow_control(buffIdx);
                
            }
        }
        else if(currState == WAIT_FOR_CONSECUTIVE){
            if(type == CONSECUTIVE){
                consecutive_cntr++;
                if(dataSize >= 7)
                {
                    dataSize = dataSize-7;
                }
                else
                {
                    dataSize = 0;
                }
            }
            if(consecutive_cntr == 4) // block size 4 until
            {
                currState = SEND_FLOW_CTL;
                send_flow_control(buffIdx);
            }
            if(dataSize == 0)
            {
                currState = WAIT_FOR_FIRST;
            }
        }
    }
    else if(eventType == CAN_EVENT_TX_COMPLETE){
        currState = WAIT_FOR_CONSECUTIVE;
    }

}

