#include "UDS.h"

UDS_SID SID = 0;
DIAGNOSTIC_SESSION_SUBFUNC Subfunc_ID = 0;
void (*curr_state)();

void UDS_Init(can_instance_t* can_pal1_instance, can_user_config_t* can_pal1_Config0){
    CanTP_init(can_pal1_instance,can_pal1_Config0, UDS_Receive);
}

void UDS_Receive(uint8_t *data){
    SID = data[0];
    int data1 = data[1];   
    if(SID == DIAGNOSTIC_SESSION_CONTROL){
        Subfunc_ID = data[1];
//        if(Subfunc_ID == DEFAULT_SESSION ){
//
//        }
//        else if(Subfunc_ID == PROGRAMMING_SESSION ){
//
//        }
    }
    if(SID == REQUEST_DOWNLOAD && Subfunc_ID == PROGRAMMING_SESSION){
        uint8_t memory_length = data[2]>>4;
        volatile uint8_t memory_address_size = data[2] & 0x0F;
        volatile uint32_t mem_start_address = 0;
        volatile uint32_t data_size = 0;
        for(int i=3; i<3+memory_length ;i++){
            mem_start_address <<= 8;
            mem_start_address |= data[i];
        }
            for(int i=3+memory_length; i<7+memory_address_size ;i++){
            data_size <<= 8;
            data_size |= data[i];
        }
    }
//    else if(SID == TRANSFER_DATA && Subfunc_ID == PROGRAMMING_SESSION){
//
//    }
//    else if(SID == REQUEST_TRANSFER_EXIT && Subfunc_ID == PROGRAMMING_SESSION){
//
//    }
//    else if(SID == READ_DATA_BY_ID && Subfunc_ID == PROGRAMMING_SESSION){
//
//    }
//    else if(SID == WRITE_DATA_BY_ID && Subfunc_ID == PROGRAMMING_SESSION){
//
//    }
}
