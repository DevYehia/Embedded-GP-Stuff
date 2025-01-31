#include "UDS.h"

UDS_SID SID = 0;
DIAGNOSTIC_SESSION_SUBFUNC currentSession = DEFAULT_SESSION;
void (*curr_state)();
uint8_t response[8];

UDS_SID UDS_Get_type(uint8_t* payload){
    //payload[0] and [1] are for the diagnostic ID
    return payload[2];
}

void UDS_Create_response(uint8_t* request){
    for(int i = 0 ; i < 8 ; i++){
        response[i] = request[i];
    }
    response[2] += 0x40;
}

void UDS_Session_Control(uint8_t* payload){
    DIAGNOSTIC_SESSION_SUBFUNC requested_session = payload[3];
    if(requested_session != DEFAULT_SESSION && requested_session != PROGRAMMING_SESSION){
        //send negative response
    }
    else{
    UDS_Create_response(payload);

    send_single_frame(response);
    }

    if(requested_session == PROGRAMMING_SESSION && currentSession == DEFAULT_SESSION){
        currentSession = requested_session;

        //set new SW Flag
        //execute reset
    }
}

void UDS_ECU_Reset(uint8_t* payload){
    ECU_RESET_SUBFUNC requested_reset = payload[3];

    //respond
    UDS_Create_response(payload);
    send_single_frame(response);

    if(requested_reset == HARD_RESET){
        //hard reset code
    }
    else if(requested_reset == SOFT_RESET){
        //soft reset code
    }

}

void UDS_Init(can_instance_t* can_pal1_instance, can_user_config_t* can_pal1_Config0){
    CanTP_init(can_pal1_instance,can_pal1_Config0, UDS_Receive);
}

void UDS_Receive(uint8_t *data){
    SID = data[0];
    int data1 = data[1];   
    if(SID == DIAGNOSTIC_SESSION_CONTROL){
        currentSession = data[1];
//        if(Subfunc_ID == DEFAULT_SESSION ){
//
//        }
//        else if(Subfunc_ID == PROGRAMMING_SESSION ){
//
//        }
    }
    if(SID == REQUEST_DOWNLOAD && currentSession == PROGRAMMING_SESSION){
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
