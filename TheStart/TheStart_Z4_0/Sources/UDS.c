#include "UDS.h"

UDS_SID SID = 0;
UDS_SID prev_SID = 0;

DIAGNOSTIC_SESSION_SUBFUNC currentSession = DEFAULT_SESSION;

//UDS Buffers
dataFrame requestFrame;
dataFrame responseFrame;

uint8_t seq_number = 1;
uint16_t remaining_Data = 0;
uint32_t MaxNumberBlockLength = 0;  /* max size in bytes (including SID) to be transmitted in every Transfer Data service */

BL_Data BL_data = {0,0,0,NULL,0,0,NULL};   /* Shared struct with BL */

static void reset_dataframe(dataFrame *frame) {
    frame->ready = 0;
    memset(frame->dataBuffer, 0, MAX_BUFF_SIZE);
    frame->dataSize = 0;
}

static void Reinit_Req_Transfer_Exit(){
    seq_number = 1;
}

static void REQ_Download_Abort(){
    /* Verify */
    SID = prev_SID;
    prev_SID = PROGRAMMING_SESSION;

    remaining_Data = 0;
}

static void Transfer_Data_Abort(){
    /* Verify */
    SID = REQUEST_TRANSFER_EXIT;
    prev_SID = PROGRAMMING_SESSION;

    remaining_Data = 0;
    seq_number = 1;
}

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
    responseFrame.dataSize = 3;
    responseFrame.ready = isReady;
}

void UDS_Session_Control(){
    DIAGNOSTIC_SESSION_SUBFUNC requested_session = requestFrame.dataBuffer[SUB_BYTE_POS];
    if(requested_session != DEFAULT_SESSION && requested_session != PROGRAMMING_SESSION){
        UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED, READY);
        return;
    }
    else{

        if(requested_session == PROGRAMMING_SESSION && currentSession == DEFAULT_SESSION){
            currentSession = requested_session;
            /* Set flag ... to be in flash/eeprom */
            #ifdef UDS_APP
                SOFT_RESET();
            #endif
        }
        
        UDS_Create_pos_response(READY);
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
    status_t status = 0;

    if (requestFrame.dataSize < 3) {
        REQ_Download_Abort();
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);  
        return; 
    }
    
    memory_length = requestFrame.dataBuffer[2]>>4; /* Defines number of bytes of MEMORY LENGTH parameter */
    memory_address_size = requestFrame.dataBuffer[2] & 0x0F; /* Defines number of bytes of START MEMORY ADDRESS parameter*/
    
    if (memory_length < 1 || memory_length > ECU_ADDRESS_LENGTH || memory_address_size < 1 || memory_address_size > 4) {
        REQ_Download_Abort();
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);  
        return; 
    }
    
    uint8_t expected_length = 3 + memory_length + memory_address_size;
    if (requestFrame.dataSize != expected_length) { 
        REQ_Download_Abort();
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);  
        return; 
    }

    BL_data.mem_start_address = 0;
    BL_data.total_size = 0; /* specifies the total size of the data that will be transferred during the subsequent (multiple) transfer data services */

    for(uint8_t i=3; i<3+memory_address_size ;i++){
        BL_data.mem_start_address <<= 8;
        BL_data.mem_start_address |= requestFrame.dataBuffer[i];
    }
    for(uint8_t i=3+memory_length; i<7+memory_length ;i++){
        BL_data.total_size <<= 8;
        BL_data.total_size |= requestFrame.dataBuffer[i];
    }
    remaining_Data = BL_data.total_size;
    
    status = BL_RequestDownloadHandler();
    if(status == STATUS_ERROR){
        REQ_Download_Abort();
        UDS_Create_neg_response(REQ_OUT_OF_RANGE,READY); /* Do memory adressing range check .. if invalid NRC: REQ_OUT_OF_RANGE */
        return;
    }else{
        REQ_Download_Abort();
        UDS_Create_pos_response(NOTREADY);        /* isReady parameter is set to 0 */
        responseFrame.dataBuffer[1] = 0x20;       /* MaxNumberBlockLength = 2 bytes, followed by reserved 4 bits = 0 */ 
        
        //test with vlaue = 5
        BL_Max_N_Block(&MaxNumberBlockLength);
        responseFrame.dataBuffer[2] = (uint8_t)(MaxNumberBlockLength>>8); /* 1st byte */
        responseFrame.dataBuffer[3] = (uint8_t)(MaxNumberBlockLength);    /* 2nd byte  e.g: 0x0FFA = 4090 ... max size in bytes (including SID) to be transmitted using Transfer Data service */
        responseFrame.dataSize = 4;
        responseFrame.ready = READY;
        /*  convey the MaxNumberBlockLength for each TransferData request to the client. This length encompasses the service identifier
                and data parameters within the TransferData request message. */
    }
}

/* Receives frame of type Transfer Data from the client */
void UDS_Transfer_Data(){
    status_t status = 0;
    if (requestFrame.dataSize < 2) {   
        Transfer_Data_Abort();
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);  
        return; 
    } 
    
    /* Ensure the message does not exceed MaxNumberOfBlockLength */  
    if (requestFrame.dataSize > MaxNumberBlockLength) {   
        Transfer_Data_Abort();
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);  
        return; 
    }

    /* Note: seq_number starts from 1 till 255 then goes back to 0 */
    if( (seq_number == 255 && requestFrame.dataBuffer[1] == 0) || (requestFrame.dataBuffer[1] == (seq_number + 1)) ){
        for(int i=0 ; i<MaxNumberBlockLength; i++){
            BL_data.data[i] = requestFrame.dataBuffer[2+i];
            remaining_Data--;
        }
        
        status = BL_ProgramHandler();
        
        if(status == STATUS_ERROR){    
            Transfer_Data_Abort();

            UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY); /* Data couldn't be written */
            return;
        }else{
            seq_number = requestFrame.dataBuffer[1]; /* Update sequence number */
            /* +ve Response */
            UDS_Create_pos_response(NOTREADY);
            responseFrame.dataBuffer[1] = seq_number;
            responseFrame.dataSize = 2;
            responseFrame.ready = READY;
            // call CAN_TP send?
        }
    }else{
        Transfer_Data_Abort();
        UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);  
        return; 
    }
}

void UDS_Request_Transfer_Exit(){
    if (requestFrame.dataSize == 33) {   
        for(uint8_t i = 0; i<32; i++){
            BL_data.CRC_Field <<= 8;
            BL_data.CRC_Field |= requestFrame.dataBuffer[1 + i];
        }
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);  
        return; 
    }else{
        /* +ve Response */
        if(remaining_Data == 0){
            Reinit_Req_Transfer_Exit();
            UDS_Create_pos_response(NOTREADY);
            responseFrame.dataBuffer[1] = 0x00;
            responseFrame.dataSize = 1;
            responseFrame.ready = READY;
        }else{
            UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);  
            return; 
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

void UDS_Routine_Control(){
    if(requestFrame.dataSize < ROUTINE_CTRL_SIZE){
        /* fix */
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);  
        return; 
    }
    uint8_t expected_size = 0;
    uint16_t routine_id = 0;
    status_t status = 0;
    static RoutineControlType prev_ctrl_type = STOP_ROUTINE;
    
    if(requestFrame.dataBuffer[1] == START_ROUTINE && prev_ctrl_type == STOP_ROUTINE){

        routine_id = (requestFrame.dataBuffer[2] << 8) | requestFrame.dataBuffer[3];

        if(routine_id == ERASE_MEMORY){
            status = STATUS_BUSY;
            BL_data.N_paramteres = requestFrame.dataBuffer[4];
            expected_size = 5 + BL_data.N_paramteres;
            
            if (BL_data.N_paramteres > 0) {
                if (requestFrame.dataSize != expected_size) {  // Ensure full parameters are received
                    /* -ve response */
                    UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT,READY);
                    return;
                }
            }
            
            for(uint8_t i = 0; i<BL_data.N_paramteres; i++){
                BL_data.parameters[i] = requestFrame.dataBuffer[5+i];
            }
            UDS_Erase_Memory(&status); /* pass needed parameters */

        }else if(routine_id == CHECK_MEMORY){
            status = STATUS_BUSY;
            BL_data.N_paramteres = requestFrame.dataBuffer[4];
            
            if (BL_data.N_paramteres > 0) {
                if (requestFrame.dataSize != expected_size) {  // Ensure full parameters are received
                    /* -ve response */
                    UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT,READY);
                    return;
                }
            }
            
            for(uint8_t i = 0; i<BL_data.N_paramteres; i++){
                BL_data.parameters[i] = requestFrame.dataBuffer[5+i]; 
            }
            
            UDS_Check_Memory(&status); /* pass needed parameters */
        }else{
            /* -ve response Invalid subfunction routine */
            UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED,READY);
            return;
        }
    }else if(requestFrame.dataBuffer[1] == STOP_ROUTINE && prev_ctrl_type == START_ROUTINE){
        BL_data.N_paramteres = 0; //de-init N_parameters field
        
        for (int i = 0; i < 20; i++) {
            BL_data.parameters[i] = 0;  //de-init parameters field
        }
        
        UDS_Create_pos_response(NOTREADY);
        for(uint8_t i=1 ; i < 3; i++){
            responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i];    
        }
        responseFrame.dataSize = 4;
        responseFrame.ready = READY;    
    }else if(requestFrame.dataBuffer[1] == REQ_ROUTINE_CTRL && prev_ctrl_type == START_ROUTINE){
        routine_id = (requestFrame.dataBuffer[2] << 8) | requestFrame.dataBuffer[3];
        BL_data.N_paramteres = requestFrame.dataBuffer[4];
        expected_size = 5 + BL_data.N_paramteres;
        if(routine_id == ERASE_MEMORY){
            
            if (BL_data.N_paramteres > 0) {
                if (requestFrame.dataSize != expected_size) {  // Ensure full parameters are received
                    /* -ve response */
                    UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT,READY);
                    return;
                }
            }
            
            /* +ve response */
            UDS_Create_pos_response(NOTREADY);
            for(uint8_t i=1 ; i < 3; i++){
                responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i];    
            }
            responseFrame.dataBuffer[4] = status;  
            responseFrame.dataSize = 5;
            responseFrame.ready = READY;

        }else if(routine_id == CHECK_MEMORY){
            
            if (BL_data.N_paramteres > 0) {
                if (requestFrame.dataSize != expected_size) {  // Ensure full parameters are received
                    /* -ve response */
                    UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT,READY);
                    return;
                }
            }
            
            UDS_Create_pos_response(NOTREADY);
            for(uint8_t i=1 ; i < 3; i++){
                responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i];    
            }
            responseFrame.dataBuffer[4] = status;  
            responseFrame.dataSize = 5;
            responseFrame.ready = READY;
        }else{
            /* -ve response Invalid subfunction routine */
            UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED,READY);
            return;
        }
    }else{
        /* -ve response */
        UDS_Create_neg_response(GENERAL_REJECT,READY);
        return;
    }
    prev_ctrl_type = requestFrame.dataBuffer[1];
}

void UDS_Check_Memory(status_t *status){
    *status = BL_Check_Memory();
    if(*status == STATUS_ERROR){    
        UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE,READY);
        return;
    }else{
        UDS_Create_pos_response(NOTREADY);
        for(uint8_t i=1 ; i < 3; i++){
            responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i];    
        }
        responseFrame.dataSize = 4;
        responseFrame.ready = READY;
    }
}

void UDS_Erase_Memory(status_t *status){
    *status = BL_Erase_Memory();
    if(*status == STATUS_ERROR){    
        UDS_Create_neg_response(CRC_ERROR,READY);
        return;
    }else{
        UDS_Create_pos_response(NOTREADY);
        for(uint8_t i=1 ; i < 3; i++){
            responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i];    
        }
        responseFrame.dataSize = 4;
        responseFrame.ready = READY;
    }
}


void UDS_Init(){


}

/* Don't forget NRC = 0x78 ... P2 Star*/

void UDS_Receive(void *params){
    uint8_t *data = params ;
    prev_SID = SID;
    SID = data[SID_POS];
    int data1 = data[1];
	while(1){

		if(requestFrame.ready == NOTREADY){
			vTaskDelay(pdMS_TO_TICKS(5));
			continue;
		}else{
            reset_dataframe(&responseFrame);
            if(SID == DIAGNOSTIC_SESSION_CONTROL){
	    	    UDS_Session_Control();
            }
            if(SID == REQUEST_DOWNLOAD && currentSession == PROGRAMMING_SESSION && (prev_SID == PROGRAMMING_SESSION || prev_SID == REQUEST_TRANSFER_EXIT)){
                UDS_Request_Download();
            }else{
                /* -ve response 0x70 */
                UDS_Create_neg_response(UPLOAD_DOWNLOAD_NOT_ACCEPTED, READY);
                return;
            }
            if(SID == ROUTINE_CONTROL && currentSession == PROGRAMMING_SESSION){
                UDS_Routine_Control();
            }
            if(SID == TRANSFER_DATA && currentSession == PROGRAMMING_SESSION){
                UDS_Transfer_Data();
            }
            if(SID == REQUEST_TRANSFER_EXIT && currentSession == PROGRAMMING_SESSION){
                UDS_Request_Transfer_Exit();
            }
        }
    }
}
