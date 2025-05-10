#ifndef UDS_H
#define UDS_H

#include "frameTypes.h"
#include "ResetWrapper.h"
#include "status.h"

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

//Session Control Macros
#define SESSION_CTRL_MIN_SIZE  2

//ECU Reset Macros
#define ECU_RESET_MIN_SIZE 2


#define ROUTINE_CTRL_SIZE 5

//make bootloader define the max block size
#ifndef MAX_BLOCK_NUMBER
#define MAX_BLOCK_NUMBER 100
#endif

#define READY 1
#define NOTREADY 0

#define ECU_ADDRESS_LENGTH 4


typedef enum DID{
    NOT_DEFINED_ID = 0x0000,
    CURR_SESSION_ID = 0xF186,
} DID;

typedef enum DID_Found_Status{
    NOT_FOUND = 0,
    FOUND =1
} DID_Found_Status;

typedef enum UDS_SID {
    DIAGNOSTIC_SESSION_CONTROL  =       0X10,
    ECU_RESET                   =       0X11,
    READ_DATA_BY_ID             =       0X22,
    COMMUNICATION_CONTROL       =       0X28,
    WRITE_DATA_BY_ID            =       0X2E,
    ROUTINE_CONTROL             =       0x31,
    REQUEST_DOWNLOAD            =       0X34,
    TRANSFER_DATA               =       0X36,
    REQUEST_TRANSFER_EXIT       =       0X37,
    TESTER_PRESENT              =       0X3E
} UDS_SID;


typedef enum ROUTINE_CTRL_ROUTINE_ID {
    ERASE_MEMORY                =       0Xff00,
    CHECK_MEMORY                =       0Xfe00
} ROUTINE_IDENTIFIER;

typedef enum RoutineControlType {
    START_ROUTINE               =       0X01,
    STOP_ROUTINE                =       0X02,
    REQ_ROUTINE_CTRL            =       0X03
} RoutineControlType;

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
    CRC_ERROR = 0x35,
    UPLOAD_DOWNLOAD_NOT_ACCEPTED = 0x70,
    GENERAL_PROGRAMMING_FAILURE = 0x72
} NRC;

typedef struct BL_Data{
    /* Data Size */
    uint32_t CRC_Field;
    uint32_t mem_start_address;
    uint32_t total_size; /* Total size of data to be received 0 ~ X*/
    /* UDS_Routine_Control */
    uint8_t data_block_size;
    uint8_t app_id;
    uint8_t N_paramteres;
    uint8_t parameters[20];
    uint8_t data[MAX_BLOCK_NUMBER];
} BL_Data;

/*
typedef struct BootLoader_Data{
        uint32_t mem_start_address;
        uint32_t total_size; /* Total size of data to be received 0 ~ X
        uint16_t MaxNumberBlockLength; /* Max size to be received with each Transfer Data service request 0 ~ 4095
        // to be replaced in impementation by passing ptr as a parameter to callout func s*
} BL_Req_Download_Data;
*/

typedef struct BL_Functions{
    status_t (*BL_RequestDownloadHandler)(void);
    void (*BL_Max_N_Block) (uint32_t *);
    status_t (*BL_TransferDataHandler)(void);
    status_t (*BL_Check_Memory)(void);
    status_t (*BL_Erase_Memory)(void);
}BL_Functions;


void UDS_Receive(void);

#ifdef UDS_BOOTLOADER
void UDS_Init(BL_Functions*);
#endif

//Handle Session Control
void UDS_Session_Control();

void UDS_ECU_Reset();

void UDS_Read_by_ID();
void UDS_Write_by_ID();


void UDS_Request_Download();
void UDS_Transfer_Data();
void UDS_Request_Transfer_Exit();
void UDS_Routine_Control();
void UDS_Check_Memory(status_t *status);
void UDS_Erase_Memory(status_t *status);


/************RESPONSE APIs*************/
void UDS_Create_pos_response(uint8_t isReady);
void UDS_Create_neg_response(NRC neg_code, uint8_t isReady);

#endif
