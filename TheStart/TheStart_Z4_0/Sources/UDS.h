#ifndef UDS_H  
#define UDS_H

#include "CanTP.h"

typedef enum UDS_SID {
    DIAGNOSTIC_SESSION_CONTROL = 0X10, 
    ECU_RESET=0X11,
    COMMUNICATION_CONTROL = 0X28,
    TESTER_PRESENT = 0X3E,
    READ_DATA_BY_ID = 0X22,
    WRITE_DATA_BY_ID = 0X2E,
    REQUEST_DOWNLOAD = 0X34,
    TRANSFER_DATA = 0X36, 
    REQUEST_TRANSFER_EXIT = 0X37
} UDS_SID;
typedef enum DIAGNOSTIC_SESSION_SUBFUNC {
    DEFAULT_SESSION = 0X01 ,
    PROGRAMMING_SESSION = 0X02
} DIAGNOSTIC_SESSION_SUBFUNC;

typedef enum ECU_RESET_SUBFUNC {
    HARD_RESET = 0X01 ,
    KEYOFF_RESET = 0X02,
    SOFT_RESET = 0X03
} ECU_RESET_SUBFUNC;

typedef enum COMMUNICATION_CONTROL_SUBFUNC {
    EN_RX_EN_TX = 0X00 ,
    EN_RX_DIS_TX = 0X01,
    DIS_RX_EN_TX = 0X02, 
    DIS_RX_DIS_TX = 0X03
} COMMUNICATION_CONTROL_SUBFUNC;

void UDS_Receive();
void UDS_Init(can_instance_t* can_pal1_instance, can_user_config_t* can_pal1_Config0);

//uses the SID Byte to identify type of UDS Service
UDS_SID UDS_Get_type(uint8_t* payload);

//Handle Session Control
void UDS_Session_Control(uint8_t* payload);

void UDS_ECU_Reset(uint8_t* payload);

void UDS_Create_response(uint8_t* request);

#endif
