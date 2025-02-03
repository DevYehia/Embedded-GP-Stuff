#ifndef UDS_H  
#define UDS_H

#include "frameTypes.h"
#include "ResetWrapper.h"

//HELPER MACROS
#define PADDING 0xAA
#define RESPONSE_BUFF_SIZE 8
#define REQUEST_BUFF_SIZE  400

#define SID_POS 0
#define SUB_BYTE_POS 1
#define DID_LOW_BYTE_POS 2
#define DID_HIGH_BYTE_POS 3
#define DATA_START_POS 4
#define getSID(buffer) (buffer[SID_POS])

#define READY 1
#define NOTREADY 0


typedef struct dataFrame
{
    uint8_t ready;
    uint8_t dataBuffer[REQUEST_BUFF_SIZE];
    uint8_t dataSize;
} dataFrame;



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
    CONDITIONS_NOT_CORRECT = 0x22,
    REQ_SEQ_ERROR = 0x24,
    REQ_OUT_OF_RANGE = 0x31,
    SECURITY_ACCESS_DENIED = 0x33, /* still unused */
    UPLOAD_DOWNLOAD_NOT_ACCEPTED = 0x70
} NRC;

typedef struct UDS_Data{
    uint8_t isValid;
    uint8_t seq_number;
    uint8_t *data;
} Transferred_Data;

typedef struct BootLoader_Data{
        uint32_t mem_start_address;
        uint32_t total_size; /* Total size of data to be received 0 ~ 4095*/
        uint16_t MaxNumberBlockLength; /* Max size to be received with each Transfer Data service request */
} BL_Req_Donwload_Data;



void UDS_Receive();
void UDS_Init(can_instance_t* can_pal1_instance, can_user_config_t* can_pal1_Config0);


//Handle Session Control
void UDS_Session_Control();

void UDS_ECU_Reset();

void UDS_Read_by_ID();
void UDS_Write_by_ID();


void UDS_Request_Download();
void UDS_Transfer_Data();

/************RESPONSE APIs*************/
void UDS_Create_pos_response(uint8_t isReady);
void UDS_Create_neg_response(NRC neg_code);

#endif