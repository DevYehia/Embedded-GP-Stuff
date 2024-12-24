#include "CanTP.h"




can_instance_t* can_instance;
typedef enum CanTP_States {WAIT_FOR_FIRST, SEND_FLOW_CTL, WAIT_FOR_CONSECUTIVE} CANTP_States;
typedef enum CANTP_Frame_Types {SINGLE, FIRST, FLOW, CONSECUTIVE} CANTP_Frame_Types;
uint32_t consecutive_cntr = 0;
CANTP_States currState = WAIT_FOR_FIRST;
void CanTP_init(can_instance_t* can_pal_instance, can_user_config_t* can_pal_Config){

    can_instance = can_pal_instance;
    CAN_Init(can_pal_instance, can_pal_Config);
    CAN_InstallEventCallback(can_pal_instance, interrupt_callback, NULL);
}

void interrupt_callback(uint32_t instance, can_event_t eventType, uint32_t objIdx, void *driverState){
    if(eventType == CAN_EVENT_RX_COMPLETE){
        if(currState == WAIT_FOR_FIRST){


        }
        else if(currState == WAIT_FOR_CONSECUTIVE){

        }


    }
    else if(eventType == CAN_EVENT_TX_COMPLETE){



    }

}

