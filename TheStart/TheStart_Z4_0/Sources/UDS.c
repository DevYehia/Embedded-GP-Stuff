#include "UDS.h"

UDS_SID SID = 0;
UDS_SID prev_SID = 0;

DIAGNOSTIC_SESSION_SUBFUNC currentSession = DEFAULT_SESSION;
void (*curr_state)();
void (*send_lower_layer)(uint8_t*);


//UDS Buffers
//uint8_t response[RESPONSE_BUFF_SIZE];
dataFrame requestFrame;
dataFrame responseFrame;

uint8_t send_frame[8];

/* Req Download Variables */
BL_Data BL_data = {0,0,0};

Transferred_Data UDS_Data = {0, 0, NULL};
uint8_t block_seq_no = 0; /* Sequence number of current received block */


static DID_Found_Status checkIfIDExists(DID ID){
    switch (ID){
        case CURR_SESSION_ID:
            return FOUND;

        default:
            return NOT_FOUND;

    }
}

//returns data by ID
static uint8_t get_data_by_ID(DID ID){
    switch (ID){
        case CURR_SESSION_ID:
            return currentSession;

        default:
            return NOT_DEFINED_ID;

    }

}

static void write_data_by_ID(DID ID, uint8_t data){
    switch (ID){
        case CURR_SESSION_ID:
            currentSession = data;
            break;

        default:
            break;

    }    
}

void UDS_Create_pos_response(uint8_t isReady){


    responseFrame.dataBuffer[SID_POS] = requestFrame.dataBuffer[SID_POS] + 0x40;
    responseFrame.dataBuffer[SUB_BYTE_POS] = requestFrame.dataBuffer[SUB_BYTE_POS];
    responseFrame.dataSize = 2;

    responseFrame.ready = isReady;

}

void UDS_Create_neg_response(NRC neg_code){
    responseFrame.dataBuffer[0] = 0x7F;
    responseFrame.dataBuffer[1] = requestFrame.dataBuffer[SID_POS];
    responseFrame.dataBuffer[2] = neg_code;
    responseFrame.dataBuffer[3] = requestFrame.dataBuffer[SUB_BYTE_POS];
    responseFrame.dataSize = 4;
    responseFrame.ready = 1;

}

void UDS_Session_Control(){
    DIAGNOSTIC_SESSION_SUBFUNC requested_session = requestFrame.dataBuffer[SUB_BYTE_POS];
    if(requested_session != DEFAULT_SESSION && requested_session != PROGRAMMING_SESSION){
        UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED);
        return;
    }
    else{
        UDS_Create_response(READY);
    }

    if(requested_session == PROGRAMMING_SESSION && currentSession == DEFAULT_SESSION){
        currentSession = requested_session;

        Do_Reset(SOFT_RESET);
        
        //set new SW Flag
        //execute reset
        
    }
}

void UDS_Read_by_ID(){
    uint8_t* payload = requestFrame.dataBuffer;
    uint16_t requested_ID = (uint16_t)payload[DID_HIGH_BYTE_POS] << 8 | payload[DID_LOW_BYTE_POS];


    //make response
    UDS_Create_pos_response(NOTREADY);
    responseFrame.dataBuffer[DID_HIGH_BYTE_POS] = requestFrame.dataBuffer[DID_HIGH_BYTE_POS];
    responseFrame.dataBuffer[DID_LOW_BYTE_POS] = requestFrame.dataBuffer[DID_LOW_BYTE_POS];

    
    //Return Data By ID
    uint8_t status = checkIfIDExists(requested_ID);
    if(status == NOT_FOUND){
        UDS_Create_neg_response(GENERAL_REJECT);
        return;
    }
    uint8_t data = get_data_by_ID(requested_ID);

    responseFrame.dataBuffer[DATA_START_POS]; //TODO Replace with actual identifier
    responseFrame.dataSize = 5;
    responseFrame.ready = READY;

}

void UDS_Write_by_ID(){
    uint8_t* payload = requestFrame.dataBuffer;
    uint16_t requested_ID = (uint16_t)payload[DID_HIGH_BYTE_POS] << 8 | payload[DID_LOW_BYTE_POS];
    uint8_t data = payload[DATA_START_POS];

    //make response
    uint8_t status = checkIfIDExists(requested_ID);
    if(status == NOT_FOUND){
        UDS_Create_neg_response(GENERAL_REJECT);
        return;
    }
    write_data_by_ID(requested_ID, data);
    UDS_Create_pos_response(payload);
    responseFrame.dataBuffer[DID_HIGH_BYTE_POS] = requestFrame.dataBuffer[DID_HIGH_BYTE_POS];
    responseFrame.dataBuffer[DID_LOW_BYTE_POS] = requestFrame.dataBuffer[DID_LOW_BYTE_POS];
    responseFrame.dataSize = 4;
    responseFrame.ready = READY;
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

void UDS_ECU_Reset(){
    uint8_t* payload = requestFrame.dataBuffer;
    ECU_RESET_SUBFUNC requested_reset = payload[SID_POS];

    //respond
    UDS_Create_response(READY);
    //send_single_frame(response);

    if(requested_reset == HARD_RESET){
        //hard reset code
    }
    else if(requested_reset == SOFT_RESET){
        //soft reset code
    }

}

void UDS_Init(){


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
