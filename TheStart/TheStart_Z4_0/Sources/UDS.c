#include "UDS.h"

UDS_SID SID = 0;
UDS_SID prev_SID = 0;

DIAGNOSTIC_SESSION_SUBFUNC currentSession = DEFAULT_SESSION;
void (*curr_state)();
void (*send_lower_layer)(uint8_t*);
uint8_t response[RESPONSE_BUFF_SIZE];

uint8_t send_frame[8];

/* Req Download Variables */
BL_Data BL_data = {0,0,0};

Transferred_Data UDS_Data = {0, 0, NULL};
uint8_t block_seq_no = 0; /* Sequence number of current received block */

UDS_SID UDS_Get_type(uint8_t* payload){
    //payload[0] and [1] are for the diagnostic ID
    return payload[2];
}

void UDS_Create_pos_response(uint8_t* request){
    for(int i = 0 ; i < 8 ; i++){
        response[i] = request[i];
    }
    response[2] += 0x40;
}

void UDS_Create_neg_response(uint8_t* request, NRC neg_code){
    response[0] = 0x7F;
    response[1] = getSID(request);
    response[2] = neg_code;
    for(int i = 3 ; i < RESPONSE_BUFF_SIZE ; i++){
        response[i] = PADDING;
    }

}

void UDS_Session_Control(uint8_t* payload){
    DIAGNOSTIC_SESSION_SUBFUNC requested_session = payload[3];
    if(requested_session != DEFAULT_SESSION && requested_session != PROGRAMMING_SESSION){
        //send negative response
        UDS_Create_neg_response(payload, SUB_FUNC_NOT_SUPPORTED);
        send_lower_layer(response);
    }
    else{
        UDS_Create_response(payload);
        send_lower_layer(response);
    }

    if(requested_session == PROGRAMMING_SESSION && currentSession == DEFAULT_SESSION){
        currentSession = requested_session;
        
        //set new SW Flag
        //execute reset
    }
}

void UDS_Read_by_ID(uint8_t* payload){
    uint16_t requested_ID = (uint16_t)payload[DID_HIGH_BYTE_POS] << 8 | payload[DID_LOW_BYTE_POS];
    UDS_Create_pos_response(payload);
    response[DATA_START_POS] = 0xAB; //TODO Replace with actual identifier
    send_lower_layer(response);
}

void UDS_Write_by_ID(uint8_t* payload){
    uint16_t requested_ID = (uint16_t)payload[DID_HIGH_BYTE_POS] << 8 | payload[DID_LOW_BYTE_POS];
    uint8_t data = payload[DATA_START_POS];
    UDS_Create_pos_response(payload);
    response[DATA_START_POS] = PADDING;
    send_lower_layer(response);    
}

/* Assuming payload[0] is SID */
/* Receives frame of type Request_Download from the client */
void UDS_Request_Download(uint8_t* payload, uint8_t payload_len){
    volatile uint8_t memory_length = 0;
    volatile uint8_t memory_address_size = 0;

    if(SID == REQUEST_DOWNLOAD && prev_SID == TRANSFER_DATA){
        UDS_Create_neg_response(payload, CONDITIONS_NOT_CORRECT);
        return;
    }

    if (payload_len < 3) {
        UDS_Create_neg_response(payload, WRONG_MSG_LEN_OR_FORMAT);
        return;
    }

    memory_length = data[2]>>4; /* Defines number of bytes of MEMORY LENGTH parameter */
    memory_address_size = data[2] & 0x0F; /* Defines number of bytes of MEMORY ADDRESS parameter*/
    
    if (memory_length < 1 || memory_length > 4 || memory_address_size < 1 || memory_address_size > 4) {
        UDS_Create_neg_response(payload, WRONG_MSG_LEN_OR_FORMAT);
        return;
    }
    
    uint8_t expected_length = 3 + memory_length + memory_address_size;
    if (payload_len < expected_length || payload_len > expected_length) {
        UDS_Create_neg_response(payload, WRONG_MSG_LEN_OR_FORMAT);
        return;
    }
    currentSession = PROGRAMMING_SESSION;
    BL_data.mem_start_address = 0;
    BL_data.data_size = 0; /* specifies the total size of the data that will be transferred during the subsequent (multiple) transfer data services */
    BL.MaxNumberBlockLength = 0; /* max size in bytes (including SID) to be transmitted in every Transfer Data service */
    for(uint8_t i=3; i<3+memory_length ;i++){
        BL_data.mem_start_address <<= 8;
        BL_data.mem_start_address |= payload[i];
    }
    for(uint8_t i=3+memory_length; i<7+memory_address_size ;i++){
        if(payload.size() > 7+memory_address_size){
            UDS_Create_neg_response(payload, WRONG_MSG_LEN_OR_FORMAT);
            return;
        }
        BL_data.total_size <<= 8;
        BL_data.total_size |= payload[i];
        
    }
    /* Do memory adressing range check .. if invalid NRC: REQ_OUT_OF_RANGE*/
    else{
        /* +ve Response */
        response[1] = 0x74; /* +ve SID */
        response[2] = 0x20; /* MaxNumberBlockLength = 2 bytes, followed by reserved 4 bits = 0 */ 
        response[3] = 0x0F; /* 1st byte */
        response[4] = 0xFA; /* 2nd byte  0x0FFA = 4090 ... max size in bytes (including SID) to be transmitted using Transfer Data service */
        BL_data.MaxNumberBlockLength = reponse[3] << 8 | response[4];
        /*  convey the MaxNumberBlockLength for each TransferData request to the client. This length encompasses the service identifier
                and data parameters within the TransferData request message. The parameter serves the purpose of enabling the client to adapt 
                to the server’s receive buffer size before initiating the data transfer process.
        */
    }
}

/* Receives frame of type Transfer Data from the client */
void UDS_Transfer_Data(uint8_t* payload, uint8_t payload_len){
    
    if (payload_len < 2) {   
        UDS_Create_neg_response(payload, INCORRECT_MSG_LENGTH_OR_FORMAT);  
        return; 
    } 
    
    /* Ensure the message does not exceed MaxNumberOfBlockLength */  
    if (payload_len > BL_data.MaxNumberOfBlockLength) {   
        UDS_Create_neg_response(payload, INCORRECT_MSG_LENGTH_OR_FORMAT);  
        return; 
    }

    currentSession = PROGRAMMING_SESSION;
    /* Note: seq_number starts from 1 till 255 then goes back to 0 */
    if( UDS_Data.seq_number == 15 && payload[1] == 0){
        UDS_Data.seq_number = payload[1];
        UDS_Data.isValid = 1;
        for(int i=0 ; i<BL_data.MaxNumberBlockLength; i++){
            data[i] = payload[2+i];
        }
        /* +ve Response */
        response[1] = 0x76;
        response[2] = UDS_Data.seq_number;
    }else if(payload[1] == ((UDS_Data.seq_number) + 1)){
        UDS_Data.seq_number = payload[1];
        UDS_Data.isValid = 1;
        for(int i=0 ; i<BL_data.MaxNumberBlockLength; i++){
            data[i] = payload[2+i];
        }
        /* +ve Response */
        response[1] = 0x76;
        response[2] = UDS_Data.seq_number;
    }else{
        UDS_Create_neg_response(payload, REQ_SEQ_ERROR);
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

void UDS_Init(can_instance_t* can_pal1_instance, can_user_config_t* can_pal1_Config0, void (*lower_send)(uint8_t*)){
    send_lower_layer = lower_send;
    CanTP_init(can_pal1_instance,can_pal1_Config0, UDS_Receive);

}

void UDS_Receive(uint8_t *data){
    prev_SID = SID;
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
        /* Set flag ... to be in flash/eeprom */
        /* Reset */
        /* UDS_erase_memory() */
        UDS_Request_Download(data);
    }
    if(SID == TRANSFER_DATA && currentSession == PROGRAMMING_SESSION){
        UDS_Transfer_Data(data);
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
