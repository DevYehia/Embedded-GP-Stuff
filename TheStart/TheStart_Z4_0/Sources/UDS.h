#ifndef UDS_H  
#define UDS_H

#include "CanTP.h"

//HELPER MACROS
#define PADDING 0xAA
#define RESPONSE_BUFF_SIZE 8

#define DID_LOW_BYTE_POS 2
#define DID_HIGH_BYTE_POS 3
#define getSID(buffer) (buffer[1])




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

typedef enum NRC{
    GENERAL_REJECT = 0x10,
    SERVICE_NOT_SUPPORTED = 0x11,
    SUB_FUNC_NOT_SUPPORTED = 0x12,
    WRONG_MSG_LEN_OR_FORMAT = 0x13,
    CONDITIONS_NOT_CORRECCT = 0x22,
    REQ_OUT_OF_RANGE = 0x31
} NRC;

typedef struct UDS_Data{
    uint8_t isValid;
    uint8_t seq_number;
    uint8_t *data;
} Transferred_Data;



void UDS_Receive();
void UDS_Init(can_instance_t* can_pal1_instance, can_user_config_t* can_pal1_Config0);

//uses the SID Byte to identify type of UDS Service
UDS_SID UDS_Get_type(uint8_t* payload);

//Handle Session Control
void UDS_Session_Control(uint8_t* payload);

void UDS_ECU_Reset(uint8_t* payload);

void UDS_Read_by_ID(uint8_t* payload);
void UDS_Write_by_ID(uint8_t* payload);

/************RESPONSE APIs*************/
void UDS_Create_pos_response(uint8_t* request);
void UDS_Create_neg_response(uint8_t* request);

/********************* NEW **********************8*/
void UDS_Request_Download(uint8_t* payload);
void UDS_Transfer_Data(uint8_t* payload);

#endif
