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

uint16_t remaining_Data = 0;

/* Req Download Variables */
BL_Req_Download_Data BL_data = {0,0,0};

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

void UDS_Create_neg_response(NRC neg_code, uint8_t isReady){
    responseFrame.dataBuffer[0] = 0x7F;
    responseFrame.dataBuffer[1] = requestFrame.dataBuffer[SID_POS];
    responseFrame.dataBuffer[2] = neg_code;
    responseFrame.dataBuffer[3] = requestFrame.dataBuffer[SUB_BYTE_POS];
    responseFrame.dataSize = 4;
    responseFrame.ready = isReady;

}

void UDS_Session_Control(){
    DIAGNOSTIC_SESSION_SUBFUNC requested_session = requestFrame.dataBuffer[SUB_BYTE_POS];
    if(requested_session != DEFAULT_SESSION && requested_session != PROGRAMMING_SESSION){
        UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED, READY);
        return;
    }
    else{
        UDS_Create_pos_response(READY);
    }

    if(requested_session == PROGRAMMING_SESSION && currentSession == DEFAULT_SESSION){
        currentSession = requested_session;

       // Do_Reset(SOFT_RESET);
        
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
        UDS_Create_neg_response(GENERAL_REJECT, READY);
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
        UDS_Create_neg_response(GENERAL_REJECT, READY);
        return;
    }
    write_data_by_ID(requested_ID, data);
    UDS_Create_pos_response(NOTREADY);
    responseFrame.dataBuffer[DID_HIGH_BYTE_POS] = requestFrame.dataBuffer[DID_HIGH_BYTE_POS];
    responseFrame.dataBuffer[DID_LOW_BYTE_POS] = requestFrame.dataBuffer[DID_LOW_BYTE_POS];
    responseFrame.dataSize = 4;
    responseFrame.ready = READY;
}

/* Assuming payload[0] is SID */
/* Receives frame of type Request_Download from the client */
void UDS_Request_Download(){
    volatile uint8_t memory_length = 0;
    volatile uint8_t memory_address_size = 0;

    if(SID == REQUEST_DOWNLOAD && prev_SID == TRANSFER_DATA){
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, NOTREADY);  
        responseFrame.dataBuffer[3] = 0x00;
        responseFrame.dataSize = 3;
        return; 
    }

    if (requestFrame.dataSize < 3) {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, NOTREADY);  
        responseFrame.dataBuffer[3] = 0x00;
        responseFrame.dataSize = 3;
        return; 
    }

    memory_length = requestFrame.dataBuffer[2]>>4; /* Defines number of bytes of MEMORY LENGTH parameter */
    memory_address_size = requestFrame.dataBuffer[2] & 0x0F; /* Defines number of bytes of MEMORY ADDRESS parameter*/
    
    if (memory_length < 1 || memory_length > ECU_ADDRESS_LENGTH || memory_address_size < 1 || memory_address_size > 4) {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, NOTREADY);  
        responseFrame.dataBuffer[3] = 0x00;
        responseFrame.dataSize = 3;
        return; 
    }
    
    uint8_t expected_length = 3 + memory_length + memory_address_size;
    if (requestFrame.dataSize < expected_length || requestFrame.dataSize > expected_length) {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, NOTREADY);  
        responseFrame.dataBuffer[3] = 0x00;
        responseFrame.dataSize = 3;
        return; 
    }

    BL_data.mem_start_address = 0;
    BL_data.total_size = 0; /* specifies the total size of the data that will be transferred during the subsequent (multiple) transfer data services */
    BL_data.MaxNumberBlockLength = 0; /* max size in bytes (including SID) to be transmitted in every Transfer Data service */

    for(uint8_t i=3; i<3+memory_length ;i++){
        BL_data.mem_start_address <<= 8;
        BL_data.mem_start_address |= requestFrame.dataBuffer[i];
    }
    for(uint8_t i=3+memory_length; i<7+memory_address_size ;i++){
        if(requestFrame.dataSize > 7+memory_address_size){
            UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, NOTREADY);  
            responseFrame.dataBuffer[3] = 0x00;
            responseFrame.dataSize = 3;
            return; 
        }
        BL_data.total_size <<= 8;
        BL_data.total_size |= requestFrame.dataBuffer[i];
    }
    remaining_Data = BL_data.total_size;
    /* Do memory adressing range check .. if invalid NRC: REQ_OUT_OF_RANGE*/
    //else{
        /* +ve Response */
        UDS_Create_pos_response(NOTREADY); /* isReady parameter is set to 0*/
        responseFrame.dataBuffer[SID_POS] = 0x74; /* +ve SID */
        responseFrame.dataBuffer[1] = 0x20; /* MaxNumberBlockLength = 2 bytes, followed by reserved 4 bits = 0 */ 
        responseFrame.dataBuffer[2] = 0x0F; /* 1st byte */
        responseFrame.dataBuffer[3] = 0xFA; /* 2nd byte  0x0FFA = 4090 ... max size in bytes (including SID) to be transmitted using Transfer Data service */
        responseFrame.ready = READY;
        BL_data.MaxNumberBlockLength = responseFrame.dataBuffer[2] << 8 | responseFrame.dataBuffer[3];
        /*  convey the MaxNumberBlockLength for each TransferData request to the client. This length encompasses the service identifier
                and data parameters within the TransferData request message. The parameter serves the purpose of enabling the client to adapt 
                to the server’s receive buffer size before initiating the data transfer process.
        */
    //}
}

/* Receives frame of type Transfer Data from the client */
void UDS_Transfer_Data(){
    
    if (requestFrame.dataSize < 2) {   
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, NOTREADY);  
        responseFrame.dataBuffer[3] = 0x00;
        responseFrame.dataSize = 3;
        return; 
    } 
    
    /* Ensure the message does not exceed MaxNumberOfBlockLength */  
    if (requestFrame.dataSize > BL_data.MaxNumberBlockLength) {   
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, NOTREADY);  
        responseFrame.dataBuffer[3] = 0x00;
        responseFrame.dataSize = 3;
        return; 
    }

    /* Note: seq_number starts from 1 till 255 then goes back to 0 */
    if( (UDS_Data.seq_number == 15 && requestFrame.dataBuffer[1] == 0) || (requestFrame.dataBuffer[1] == ((UDS_Data.seq_number) + 1)) ){
        for(int i=0 ; i<BL_data.MaxNumberBlockLength; i++){
            UDS_Data.data[i] = requestFrame.dataBuffer[2+i];
            remaining_Data--;
        }
        UDS_Data.seq_number = requestFrame.dataBuffer[1];
        // UDS_Data.isValid = 1;
        /* +ve Response */
        UDS_Create_pos_response(NOTREADY);
        responseFrame.dataBuffer[1] = UDS_Data.seq_number;
        responseFrame.ready = READY;
    }else if(requestFrame.dataBuffer[1] == ((UDS_Data.seq_number) + 1)){
        for(int i=0 ; i<BL_data.MaxNumberBlockLength; i++){
            UDS_Data.data[i] = requestFrame.dataBuffer[2+i];
            remaining_Data--;
        }
        UDS_Data.seq_number = requestFrame.dataBuffer[1];
        /* Add last block flag */

        /* +ve Response */
        UDS_Create_pos_response(NOTREADY);
        responseFrame.dataBuffer[1] = UDS_Data.seq_number;
        responseFrame.ready = READY;
    }else{
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, NOTREADY);  
        responseFrame.dataBuffer[3] = 0x00;
        responseFrame.dataSize = 3;
        return; 
    }
}

void UDS_Request_Transfer_Exit(){
    if (requestFrame.dataSize != 1) {   
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, NOTREADY);  
        responseFrame.dataBuffer[3] = 0x00;
        responseFrame.dataSize = 3;
        return; 
    }else{
        /* +ve Response */
        if(remaining_Data == 0){
            UDS_Create_pos_response(NOTREADY);
            responseFrame.dataBuffer[1] = 0x00;
            responseFrame.ready = READY;
        }else{
            /* CRC check */
            if(0){

            }
            else{
                /* +ve response */
            }
            
        }

    }
}

void UDS_ECU_Reset(){
    uint8_t* payload = requestFrame.dataBuffer;
    ECU_RESET_SUBFUNC requested_reset = payload[SID_POS];

    //respond
    UDS_Create_pos_response(READY);
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

void UDS_Receive(void* params){
	while(1){

		if(requestFrame.ready == NOTREADY){
			continue;
		}
	    if(SID == DIAGNOSTIC_SESSION_CONTROL){
	    	UDS_Session_Control();
	    }
	    if(SID == REQUEST_DOWNLOAD && currentSession == PROGRAMMING_SESSION){
	        /* Set flag ... to be in flash/eeprom */
	        /* Reset */
	        /* UDS_erase_memory() */
	        UDS_Request_Download();
	    }
	    if(SID == TRANSFER_DATA && currentSession == PROGRAMMING_SESSION){
	        UDS_Transfer_Data();
	    }
	    /* some check on SID == REQUEST_TRANSFER_EXIT */
	       // UDS_Request_Transfer_Exit();
	    requestFrame.ready = NOTREADY;
	}
    vTaskDelay(pdMS_TO_TICKS(5));





}
