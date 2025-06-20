#include "UDS.h"

#define UDS_APP

UDS_SID SID = REQUEST_DOWNLOAD;
UDS_SID prev_SID = ROUTINE_CONTROL;

DIAGNOSTIC_SESSION_SUBFUNC currentSession = DEFAULT_SESSION;
__attribute__((section(".noinit"))) volatile uint32_t initVAR;

/********************** Note: *************/
/* all variables initialization has changed for testing */

// UDS Buffers
dataFrame requestFrame = {0, NULL, 0};
dataFrame responseFrame = {0, NULL, 0};

uint8_t seq_number = 1;
uint16_t remaining_Data = 0;
uint32_t MaxNumberBlockLength = 0; /* max size in bytes (including SID and seq number) to be transmitted in every Transfer Data service */

BL_Data BL_data = {0, 0, 0, 0, 0, 0, {0}, {0}}; /* Shared struct with BL */

BL_Functions *BL_Callbacks;

uint8_t BL_Func_missing = 0;

static void reset_dataframe(dataFrame *frame)
{
	frame->ready = 0;
	memset(frame->dataBuffer, 0, MAX_BUFF_SIZE);
	frame->dataSize = 0;
}

static void Reinit_Req_Transfer_Exit()
{
	seq_number = 1;
}

void Finished_Routine_CTR()
{
	BL_data.N_paramteres = 0; // de-init N_parameters field

	for (int i = 0; i < 20; i++)
	{
		BL_data.parameters[i] = 0; // de-init parameters field
	}
}

static void REQ_Download_Abort()
{
	/* Verify */
	SID = prev_SID;
	prev_SID = PROGRAMMING_SESSION;

	remaining_Data = 0;
}

static void Transfer_Data_Abort()
{
	/* Verify */
	SID = REQUEST_TRANSFER_EXIT;
	prev_SID = PROGRAMMING_SESSION;

	remaining_Data = 0;
	seq_number = 1;
}

static void Add_DID(){
	for (int i = responseFrame.dataSize - 1; i >= 0; i--) {
		responseFrame.dataBuffer[i + 2] = responseFrame.dataBuffer[i];
	}

	responseFrame.dataBuffer[DIAG_ID_HIGH_BYTE_POS] = (LOCAL_DIAG_ID>>8)&0x7;
	responseFrame.dataBuffer[DIAG_ID_LOW_BYTE_POS] =  (uint8_t)LOCAL_DIAG_ID;

	responseFrame.dataSize += 2;

	responseFrame.ready = READY;
}

static DID_Found_Status checkIfIDExists(DID ID)
{
	switch (ID)
	{
	case CURR_SESSION_ID:
		return FOUND;

	default:
		return NOT_FOUND;
	}
}

// returns data by ID
static uint8_t get_data_by_ID(DID ID)
{
	switch (ID)
	{
	case CURR_SESSION_ID:
		return currentSession;

	default:
		return NOT_DEFINED_ID;
	}
}

static void write_data_by_ID(DID ID, uint8_t data)
{
	switch (ID)
	{

	default:
		break;
	}
}

void UDS_Create_pos_response(uint8_t isReady)
{
	responseFrame.dataBuffer[SID_POS] = requestFrame.dataBuffer[SID_POS] + 0x40;
	responseFrame.dataBuffer[SUB_BYTE_POS] = requestFrame.dataBuffer[SUB_BYTE_POS];
	responseFrame.dataSize = 2;
	responseFrame.ready = isReady;
}

void UDS_Create_neg_response(NRC neg_code, uint8_t isReady)
{
	responseFrame.dataBuffer[0] = 0x7F;
	responseFrame.dataBuffer[1] = requestFrame.dataBuffer[SID_POS];
	responseFrame.dataBuffer[2] = neg_code;
	responseFrame.dataSize = 3;
	Add_DID();
	responseFrame.ready = isReady;
}

void UDS_Session_Control()
{

	// check data size
	if (requestFrame.dataSize < SESSION_CTRL_MIN_SIZE)
	{
		UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
		return;
	}

	// get session and check if session is supported
	DIAGNOSTIC_SESSION_SUBFUNC requested_session = requestFrame.dataBuffer[SUB_BYTE_POS];
	if (requested_session != DEFAULT_SESSION && requested_session != PROGRAMMING_SESSION)
	{
		UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED, READY);
		return;
	}

	// reset if programming session is needed in application
	else
	{

		// respond
		UDS_Create_pos_response(NOTREADY);
		responseFrame.dataBuffer[2] = 0x23;
		responseFrame.dataBuffer[3] = 0x23;
		responseFrame.dataBuffer[4] = 0x23;
		responseFrame.dataBuffer[5] = 0x23;
		responseFrame.dataSize = 6;
		Add_DID();
		//		responseFrame.ready = READY;
		if (requested_session == PROGRAMMING_SESSION && currentSession == DEFAULT_SESSION)
		{

			currentSession = requested_session;
			// taskYIELD();
			// TODO wait for sending response
			initVAR = 0xFFFFAAAA;
			PINS_DRV_WritePin(PTA, 4, 1);
			/* Set flag ... to be in flash/eeprom */
			// #ifndef UDS_BOOTLOADER
			// SOFT_RESET();
			// HARD_RESET();
//			vTaskDelay(pdMS_TO_TICKS(20));
//#ifndef UDS_BOOTLOADER
//			// SOFT_RESET();
//			// HARD_RESET();
//			*((uint32_t *) (0x40040010)) = PIT_DRV_GetCurrentTimerCount(INST_PIT1, 3);
//			//			volatile uint32_t v1 = PIT_DRV_GetCurrentTimerCount(INST_PIT1, 3);
//			CAN_Deinit(&can_pal1_instance);
//			PIT_DRV_Deinit(INST_PIT1);
//			__asm__("e_lis %r12,0x00F9");
//			__asm__("e_or2i %r12,0x8010");
//			__asm__("e_lwz %r0,0(%r12) ");
//			__asm__("mtlr %r0");
//			__asm__("se_blrl");
//#endif
			// #endif
		}
	}
}

void UDS_ECU_Reset()
{
	uint8_t *payload = requestFrame.dataBuffer;
	ECU_RESET_SUBFUNC requested_reset = payload[SID_POS];

	// check message length
	if (requestFrame.dataSize < ECU_RESET_MIN_SIZE)
	{
		UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
		return;
	}

	// check if reset is supported
	if (requested_reset != SOFT_RESET && requested_reset != HARD_RESET)
	{
		UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED, READY);
		return;
	}
	// respond
	UDS_Create_pos_response(NOTREADY);
	Add_DID();

	vTaskDelay(pdMS_TO_TICKS(20));
	// TODO wait for response to be sent

	if (requested_reset == HARD_RESET)
	{
		HARD_RESET();
	}
	else if (requested_reset == SOFT_RESET)
	{
		SOFT_RESET();
	}
}

void UDS_Read_by_ID()
{
	uint8_t *payload = requestFrame.dataBuffer;
	uint16_t requested_ID = (uint16_t)payload[DID_HIGH_BYTE_POS] << 8 | payload[DID_LOW_BYTE_POS];

	// make response
	UDS_Create_pos_response(NOTREADY);
	responseFrame.dataBuffer[DID_HIGH_BYTE_POS] = requestFrame.dataBuffer[DID_HIGH_BYTE_POS];
	responseFrame.dataBuffer[DID_LOW_BYTE_POS] = requestFrame.dataBuffer[DID_LOW_BYTE_POS];

	// Return Data By ID
	uint8_t status = checkIfIDExists(requested_ID);
	if (status == NOT_FOUND)
	{
		UDS_Create_neg_response(GENERAL_REJECT, READY);
		return;
	}
	uint8_t data = get_data_by_ID(requested_ID);

	responseFrame.dataBuffer[DATA_START_POS]; // TODO Replace with actual identifier
	responseFrame.dataSize = 5;
	Add_DID();
}

void UDS_Write_by_ID()
{
	uint8_t *payload = requestFrame.dataBuffer;
	uint16_t requested_ID = (uint16_t)payload[DID_HIGH_BYTE_POS] << 8 | payload[DID_LOW_BYTE_POS];
	uint8_t data = payload[DATA_START_POS];

	// make response
	uint8_t status = checkIfIDExists(requested_ID);
	if (status == NOT_FOUND)
	{
		UDS_Create_neg_response(GENERAL_REJECT, READY);
		return;
	}
	write_data_by_ID(requested_ID, data);
	UDS_Create_pos_response(NOTREADY);
	responseFrame.dataBuffer[DID_HIGH_BYTE_POS] = requestFrame.dataBuffer[DID_HIGH_BYTE_POS];
	responseFrame.dataBuffer[DID_LOW_BYTE_POS] = requestFrame.dataBuffer[DID_LOW_BYTE_POS];
	responseFrame.dataSize = 4;
	Add_DID();
}

/* Assuming payload[0] is SID */
/* Receives frame of type Request_Download from the client */

#ifdef UDS_BOOTLOADER
void UDS_Request_Download()
{
	volatile uint8_t NBytesDataLength = 0;
	volatile uint8_t memory_address_size = 0;
	volatile uint8_t last_address = 0;
	status_t status = 0;
	if (!(currentSession == PROGRAMMING_SESSION || prev_SID == ROUTINE_CONTROL || prev_SID == REQUEST_TRANSFER_EXIT))
	{
		UDS_Create_neg_response(UPLOAD_DOWNLOAD_NOT_ACCEPTED, READY);
		return;
	}
	else
	{
		if (requestFrame.dataSize < 3)
		{
			REQ_Download_Abort();
			UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
			return;
		}

		BL_data.compression_flag = requestFrame.dataBuffer[1]>>4; /* DataFormatIdentifier(1byte) ... Bits 7-4 -> Compression method (default 0x0 - >Nocompression) */
		NBytesDataLength = requestFrame.dataBuffer[2] >> 4;  /* Defines number of bytes of MEMORY LENGTH parameter */
		NBytesDataLength = requestFrame.dataBuffer[2] >> 4;		 /* Defines number of bytes of MEMORY LENGTH parameter */
		memory_address_size = requestFrame.dataBuffer[2] & 0x0F; /* Defines number of bytes of START MEMORY ADDRESS parameter*/

		if (memory_address_size < 1 || memory_address_size > ECU_ADDRESS_LENGTH || NBytesDataLength < 1 || NBytesDataLength > 4)
		{
			REQ_Download_Abort();
			UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
			return;
		}

		uint8_t expected_length = 3 + NBytesDataLength + memory_address_size;
		if (requestFrame.dataSize != expected_length)
		{
			REQ_Download_Abort();
			UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
			return;
		}

		BL_data.mem_start_address = 0;
		BL_data.total_size = 0; /* specifies the total size of the data that will be transferred during the subsequent (multiple) transfer data services */

		for (uint8_t i = 3; i < 3 + memory_address_size; i++)
		{
			BL_data.mem_start_address <<= 8;
			BL_data.mem_start_address |= requestFrame.dataBuffer[i];
		}
		for (uint8_t i = 3 + memory_address_size; i < 7 + NBytesDataLength; i++)
		{
			BL_data.total_size <<= 8;
			BL_data.total_size |= requestFrame.dataBuffer[i];
		}
		remaining_Data = BL_data.total_size;

		last_address = BL_data.req_down_info[BL_data.req_down_size].mem_start_address + BL_data.req_down_info[BL_data.req_down_size].total_size - 1;
		if (last_address + 1 == BL_data.mem_start_address)
		{
			// increase new req download data size to the one before if they are consecuent in address
			BL_data.req_down_info[BL_data.req_down_size].total_size += BL_data.total_size;
		}
		else
		{
			BL_data.req_down_size++;
			BL_data.req_down_info[BL_data.req_down_size].mem_start_address = BL_data.mem_start_address;
			BL_data.req_down_info[BL_data.req_down_size].total_size = BL_data.total_size;
		}

		BL_data.request_flag = 1;

		UDS_Create_pos_response(NOTREADY);
		responseFrame.dataBuffer[1] = 0x20; /* MaxNumberBlockLength = 2 bytes, followed by reserved 4 bits = 0 */

		responseFrame.dataBuffer[2] = (uint8_t)(MaxNumberBlockLength >> 8); /* 1st byte */
		responseFrame.dataBuffer[3] = (uint8_t)(MaxNumberBlockLength);		/* 2nd byte  e.g: 0x0FFA = 4090 ... max size in bytes (including SID) to be transmitted using Transfer Data service */
		responseFrame.dataSize = 4;
		Add_DID();
		/*  convey the MaxNumberBlockLength for each TransferData request to the client. This length encompasses the service identifier
			and data parameters within the TransferData request message. */
	}
}


/* Receives frame of type Transfer Data from the client */
void UDS_Transfer_Data()
{
	uint8_t remainder = 0;
	status_t status = 0;
	if (requestFrame.dataSize < 2)
	{
		Transfer_Data_Abort();
		UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
		return;
	}

	/* Ensure the message does not exceed MaxNumberOfBlockLength */
	if (requestFrame.dataSize - 2 > MaxNumberBlockLength)
	{
		Transfer_Data_Abort();
		UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
		return;
	}

	BL_data.data_block_size = requestFrame.dataSize - 2;

	/* Note: seq_number starts from 1 till 255 then goes back to 0 */
	if ((seq_number == 255 && requestFrame.dataBuffer[1] == 0) || (requestFrame.dataBuffer[1] == seq_number))
	{
		for (int i = 0; i < BL_data.data_block_size; i++)
		{
			BL_data.data[i] = requestFrame.dataBuffer[2 + i];
			remaining_Data--;
		}

		remainder = BL_data.data_block_size % 4;
		if(remainder){
			uint8_t idx = BL_data.data_block_size;
			while(remainder--){
				BL_data.data[idx];
				idx++;
			}
			BL_data.data_block_size+= remainder;
		}

		// No Function was passed (TESTING PURPOSES ONLY)
		if (BL_Callbacks->BL_TransferDataHandler == NULL)
		{
			BL_Func_missing = 1;
			return;
		}
		else
		{
			BL_Func_missing = 0;
		}
		// END OF TESTING CODE

		status = BL_Callbacks->BL_TransferDataHandler();
		BL_data.mem_start_address += BL_data.data_block_size;
		if (status == STATUS_ERROR)
		{
			Transfer_Data_Abort();

			UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY); /* Data couldn't be written */
			return;
		}
		else
		{
			/* +ve Response */
			UDS_Create_pos_response(NOTREADY);
			responseFrame.dataBuffer[1] = seq_number;
			responseFrame.dataSize = 2;

			seq_number++; /* Update sequence number */
			Add_DID();
			// call CAN_TP send?
		}
	}
	else
	{
		Transfer_Data_Abort();
		UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);
		return;
	}
}

void UDS_Request_Transfer_Exit()
{
	BL_data.request_flag = 1;
	if (requestFrame.dataSize != 1)
	{
		UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
		return;
	}
	else
	{
		/* +ve Response */
		if (remaining_Data == 0)
		{
			Reinit_Req_Transfer_Exit();
			UDS_Create_pos_response(NOTREADY);
			responseFrame.dataBuffer[1] = 0x00;
			responseFrame.dataSize = 1;
			Add_DID();
		}
		else
		{
			UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
			return;
		}
	}
}

void UDS_Routine_Control()
{
	if (requestFrame.dataSize < ROUTINE_CTRL_SIZE)
	{
		UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
		return;
	}
	uint8_t expected_size = 0;
	uint16_t routine_id = 0;
	status_t status = 0;
	uint8_t remainder = 0;

	static RoutineControlType prev_ctrl_type = STOP_ROUTINE;
	volatile uint8_t NBytesDataLength = 0;
	volatile uint8_t memory_address_size = 0;

	if (requestFrame.dataBuffer[1] == START_ROUTINE)
	{
		routine_id = (requestFrame.dataBuffer[2] << 8) | requestFrame.dataBuffer[3];
		BL_data.N_paramteres = requestFrame.dataBuffer[4];

		if (routine_id == ERASE_MEMORY)
		{
			status = STATUS_BUSY;
			expected_size = 7 + BL_data.N_paramteres;
			if (requestFrame.dataSize < 3)
			{
				REQ_Download_Abort();
				UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
				return;
			}

			NBytesDataLength = requestFrame.dataBuffer[4] >> 4;		 /* Defines number of bytes of MEMORY LENGTH parameter */
			memory_address_size = requestFrame.dataBuffer[4] & 0x0F; /* Defines number of bytes of START MEMORY ADDRESS parameter*/

			if (memory_address_size < 1 || memory_address_size > ECU_ADDRESS_LENGTH || NBytesDataLength < 1 || NBytesDataLength > 4)
			{
				UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
				return;
			}

			uint8_t expected_length = 5 + NBytesDataLength + memory_address_size;
			if (requestFrame.dataSize != expected_length)
			{
				UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
				return;
			}

			BL_data.ers_mem_start_address = 0;
			BL_data.ers_total_size = 0; /* specifies the total size of the data that will be transferred during the subsequent (multiple) transfer data services */

			for (uint8_t i = 5; i < 5 + memory_address_size; i++)
			{
				BL_data.ers_mem_start_address <<= 8;
				BL_data.ers_mem_start_address |= requestFrame.dataBuffer[i];
			}
			for (uint8_t i = 5 + memory_address_size; i < expected_length; i++)
			{
				BL_data.ers_total_size <<= 8;
				BL_data.ers_total_size |= requestFrame.dataBuffer[i];
			}


			remainder = BL_data.ers_total_size % 4;
			if(remainder){
				BL_data.ers_total_size += remainder;
			}

			UDS_Erase_Memory(&status); /* pass needed parameters */
			if (status == STATUS_SUCCESS)
			{
				UDS_Create_pos_response(NOTREADY);
				responseFrame.dataBuffer[2] = requestFrame.dataBuffer[2];
				responseFrame.dataBuffer[3] = requestFrame.dataBuffer[3];
				// responseFrame.dataBuffer[4] = 0;
				// responseFrame.dataSize = 5;
				responseFrame.dataSize = 4;
				Add_DID();
			}
		}
		else if (routine_id == CHECK_MEMORY)
		{
			status = STATUS_BUSY;
			expected_size = 5 + BL_data.N_paramteres * 2;
			if (BL_data.N_paramteres != 0x04 && BL_data.N_paramteres != 0x02 && BL_data.N_paramteres != 0x01)
			{
				/* -ve response */
				UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
				return;
			}
			else if (requestFrame.dataSize != expected_size)
			{
				// Ensure full parameters are received
				UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
				return;
			}
			else
			{
				for (uint8_t i = 0; i < BL_data.N_paramteres * 2; i++)
				{
					BL_data.CRC_Field <<= 8;
					BL_data.CRC_Field |= requestFrame.dataBuffer[5 + i];
				}
			}
			UDS_Check_Memory(&status); /* pass needed parameters */
		}
		else if (routine_id == FINALIZE_PROGRAMMING)
		{
			status = STATUS_BUSY;
			uint8_t signature = requestFrame.dataBuffer[4];
			if (signature == ECDSA_SIGNATURE)
			{
				for (uint8_t i = 5; i < requestFrame.dataSize; i++)
				{
					BL_data.signature[i-5] = requestFrame.dataBuffer[i];
				}
				//				portENTER_CRITICAL();
				status = BL_Callbacks->BL_Finalize_Programming();
				//				portEXIT_CRITICAL();
				if (status == STATUS_ERROR)
				{
					UDS_Create_neg_response(INVALID_KEY, READY);
					return;
				}
				UDS_Create_pos_response(NOTREADY);
				responseFrame.dataBuffer[2] = requestFrame.dataBuffer[2];
				responseFrame.dataBuffer[3] = requestFrame.dataBuffer[3];
				// responseFrame.dataBuffer[4] = 0;
				// responseFrame.dataSize = 5;
				responseFrame.dataSize = 4;
				Add_DID();
			}
			else if (signature == NO_SIGNATURE)
			{
				UDS_Create_pos_response(NOTREADY);
				responseFrame.dataBuffer[2] = requestFrame.dataBuffer[2];
				responseFrame.dataBuffer[3] = requestFrame.dataBuffer[3];
				// responseFrame.dataBuffer[4] = 0;
				// responseFrame.dataSize = 5;
				responseFrame.dataSize = 4;
				Add_DID();
			}
			else
			{
				UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED, READY);
				return;
			}
		}
		else
		{ /* -ve response Invalid subfunction routine */
			UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED, READY);
			return;
		}
	}
	else if (requestFrame.dataBuffer[1] == STOP_ROUTINE && prev_ctrl_type == START_ROUTINE)
	{
		Finished_Routine_CTR(); /* Clear buffers and variables used */

		UDS_Create_pos_response(NOTREADY);
		for (uint8_t i = 1; i < 3; i++)
		{
			responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i + 1];
		}
		responseFrame.dataSize = 4;
		Add_DID();
	}
	else if (requestFrame.dataBuffer[1] == REQ_ROUTINE_CTRL && prev_ctrl_type == START_ROUTINE)
	{
		routine_id = (requestFrame.dataBuffer[2] << 8) | requestFrame.dataBuffer[3];
		BL_data.N_paramteres = requestFrame.dataBuffer[4];
		expected_size = 5 + BL_data.N_paramteres;
		if (routine_id == ERASE_MEMORY)
		{

			if (BL_data.N_paramteres > 0)
			{
				if (requestFrame.dataSize != expected_size) /* Ensure full parameters are received */
				{
					/* -ve response */
					UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
					return;
				}
			}

			/* +ve response */
			UDS_Create_pos_response(NOTREADY);
			for (uint8_t i = 1; i < 3; i++)
			{
				responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i];
			}
			responseFrame.dataBuffer[4] = status;
			responseFrame.dataSize = 5;
			Add_DID();
		}
		else if (routine_id == CHECK_MEMORY)
		{

			if (BL_data.N_paramteres > 0)
			{
				if (requestFrame.dataSize != expected_size) /* Ensure full parameters are received */
				{
					/* -ve response */
					UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
					return;
				}
			}

			UDS_Create_pos_response(NOTREADY);
			for (uint8_t i = 1; i < 3; i++)
			{
				responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i];
			}
			responseFrame.dataBuffer[4] = status;
			responseFrame.dataSize = 5;
			Add_DID();
		}
		else
		{
			/* -ve response Invalid subfunction routine */
			UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED, READY);
			return;
		}
	}
	else
	{
		/* -ve response */
		UDS_Create_neg_response(GENERAL_REJECT, READY);
		return;
	}
	prev_ctrl_type = requestFrame.dataBuffer[1];
}

void UDS_Check_Memory(status_t *status)
{
	// No Function was passed (TESTING PURPOSES ONLY)
	if (BL_Callbacks->BL_Erase_Memory == NULL)
	{
		BL_Func_missing = 1;
		return;
	}
	else
	{
		BL_Func_missing = 0;
	}
	// END OF TESTING CODE
	*status = BL_Callbacks->BL_Check_Memory();
	if (*status == STATUS_ERROR)
	{
		responseFrame.dataBuffer[0] = 0x7F;
		for (uint8_t i = 1; i < 4; i++)
		{
			responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i];
		}
		responseFrame.dataBuffer[4] = GENERAL_PROGRAMMING_FAILURE;
		Add_DID();
		Finished_Routine_CTR();
		return;
	}
	else
	{
		UDS_Create_pos_response(NOTREADY);
		for (uint8_t i = 1; i < 4; i++)
		{
			responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i];
		}
		responseFrame.dataBuffer[4] = 0x00;
		responseFrame.dataSize = 5;
		Add_DID();
		Finished_Routine_CTR();
	}
}

void UDS_Erase_Memory(status_t *status)
{
	// No Function was passed (TESTING PURPOSES ONLY)
	if (BL_Callbacks->BL_Erase_Memory == NULL)
	{
		BL_Func_missing = 1;
		return;
	}
	else
	{
		BL_Func_missing = 0;
	}
	// END OF TESTING CODE
	*status = BL_Callbacks->BL_Erase_Memory();

	if (*status == STATUS_ERROR)
	{
		responseFrame.dataBuffer[0] = 0x7F;
		for (uint8_t i = 1; i < 4; i++)
		{
			responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i];
		}
		responseFrame.dataBuffer[4] = CRC_ERROR;
		Add_DID();

		Finished_Routine_CTR();
		return;
	}
	else
	{
		UDS_Create_pos_response(NOTREADY);
		for (uint8_t i = 1; i < 4; i++)
		{
			responseFrame.dataBuffer[i] = requestFrame.dataBuffer[i + 1];
		}
		responseFrame.dataBuffer[4] = 0x00;
		responseFrame.dataSize = 5;
		Add_DID();
		Finished_Routine_CTR();
	}
}

#endif
void UDS_Init(BL_Functions *BL_Funcs)
{

	if (BL_Funcs == NULL)
	{
		BL_Func_missing = 1;
		return;
	}
	else
	{
		BL_Func_missing = 0;
	}
	BL_Callbacks = BL_Funcs;
	if (initVAR == 0x0)
		currentSession = DEFAULT_SESSION;
	else
		currentSession = PROGRAMMING_SESSION;
}
//TODO NRC = 0x78 ... P2 Star*/

void UDS_Receive(void)
{
	uint16_t diagnostic_Id = 0;
	//	while(1){
	if (requestFrame.ready == READY)
	{

		diagnostic_Id = ((requestFrame.dataBuffer[DIAG_ID_HIGH_BYTE_POS]<<8)&0x7) | requestFrame.dataBuffer[DIAG_ID_LOW_BYTE_POS];
		requestFrame.dataSize -= 2;
		for (uint32_t i = 0; i < requestFrame.dataSize; i++) {
			requestFrame.dataBuffer[i] = requestFrame.dataBuffer[i + 2];
		}
		prev_SID = SID;
		SID = requestFrame.dataBuffer[SID_POS];
		if(diagnostic_Id==EXPECTED_DIAG_ID){
			if (SID == DIAGNOSTIC_SESSION_CONTROL)
			{
				UDS_Session_Control();
			}
			else if (SID == ECU_RESET)
			{
				UDS_ECU_Reset();
			}
			else if (SID == READ_DATA_BY_ID)
			{
				UDS_Read_by_ID();
			}
			else if (SID == WRITE_DATA_BY_ID)
			{
				UDS_Write_by_ID();
			}
#ifdef UDS_BOOTLOADER
			else if (SID == REQUEST_DOWNLOAD && currentSession == PROGRAMMING_SESSION)
			{
				UDS_Request_Download();
			}
			else if (SID == ROUTINE_CONTROL && currentSession == PROGRAMMING_SESSION)
			{
				UDS_Routine_Control();
			}
			else if (SID == TRANSFER_DATA && currentSession == PROGRAMMING_SESSION)
			{
				UDS_Transfer_Data();
			}
			else if (SID == REQUEST_TRANSFER_EXIT && currentSession == PROGRAMMING_SESSION)
			{
				UDS_Request_Transfer_Exit();
			}
#endif

			else
			{
				UDS_Create_neg_response(SERVICE_NOT_SUPPORTED, READY);
			}

			reset_dataframe(&requestFrame);
		}
	}
	else if(requestFrame.ready == 2)
	{
		requestFrame.ready = 0;
		*((uint32_t *) (0x40040010)) = PIT_DRV_GetCurrentTimerCount(INST_PIT1, 3);
		//			volatile uint32_t v1 = PIT_DRV_GetCurrentTimerCount(INST_PIT1, 3);
		CAN_Deinit(&can_pal1_instance);
		PIT_DRV_Deinit(INST_PIT1);
		__asm__("e_lis %r12,0x00F9");
		__asm__("e_or2i %r12,0x8010");
		__asm__("e_lwz %r0,0(%r12) ");
		__asm__("mtlr %r0");
		__asm__("se_blrl");
	}

}

BL_Data *UDS_BL_Receive(void)
{
	return &BL_data;
}
