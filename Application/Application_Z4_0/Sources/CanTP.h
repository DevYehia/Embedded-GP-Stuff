#ifndef CANTP_H
#define CANTP_H

#include "can_pal1.h"
#include "frameTypes.h"
#include <queue.h>
#include <semphr.h>

#define TX_BUFF_NUM 1
#define RX_BUFF_NUM 0

#define MAX_TP_SIZE 4096

#define consecutiveFrameSize 7
#define firstFrameSize 6

#define startFirst 2
#define startConsecutive 1

//Inputs:
// 1) Buffer used to send data 
// 2) Buffer used to recieve buffer 
//Output: None
//Desc: Initialize canTP parameters like currState and 
void CanTP_init(dataFrame* sendingBuffer, dataFrame* recieveBuffer);

//Inputs:
// 1) type of flow control 'E' ---> Error, 'A' ---> Normal
// 2) buffIdx: index of CAN buffer used
//Output: None
//Desc: Send Flow Control frame 
void send_flow_control(char type, uint32_t buffIdx);


//Inputs: None
//Outputs: None
//Desc: Handles a received consective frame
void handleConsecutiveFrame();

//Inputs: None
//Outputs: None
//Desc: Handles a received flow control frame
void handleFlowCtl();

//Inputs: None
//Outputs: None
//Desc: Handles a received first frame
void handleFirstFrame();

//Inputs:
// 1) size: size of frame to be read
// 2) start: Start on actual data without headers
//Outputs: None
//Desc: Extracts Data from frame removing the CanTP headers
void readCanTPPayload(uint8_t size, uint8_t start);

//Inputs:
// 1) payload: buffer with data to be sent
// 2) buffIdx: Index of CAN buffer for sending
//Outputs: None
//Desc: Send Single Frame (No Shit)
void send_single_frame(uint8_t *payload, uint32_t buffIdx);

//Inputs:
// 1) payload: buffer with data to be sent
//Outputs: 
//Desc: gets payload size excluding the padding (including the headers)
uint8_t get_payload_size(uint8_t *payload);

//Inputs: None
//Outputs: None
//Desc: Handles a received single frame
void handleSingleFrame();


//Inputs: None
//Outputs: None
//Desc: CAN Sending Function --> Checks if the sending buffer is ready, then sends its data
//It sends either single or consecutive frame depending on the data
void sendFromUDS(void *pv);

//Inputs: None
//Outputs: None
//Desc: CAN Receiving Function --> Checks if the receiving buffer is ready, then reads its data
//It also has a timeout of 100 function calls for consective frames
void recieve(void *pv);

//Inputs: None
//Outputs: None
//Desc: Resets the CanTP parameters
void timeOutHandle();

//Inputs:
//1) can_pal_instance: CAN instance used
//2) can_pal_config: Can instance configuration
//Both are got from generated code
//Outputs: None
//Desc: Initializes the CAN bus to be used by CANTP
void Can_init(can_instance_t *can_pal_instance, can_user_config_t *can_pal_Config);

//Inputs: None
//Outputs: None
//Desc: Resets the CanTP parameters
void resetCanTP();

//Inputs:
// 1) payload: buffer with data to be sent
// 2) buffIdx: Index of CAN buffer for sending
//Outputs: None
//Desc: Send Consecutive Frame (No Shit)
void send_consecutive_frame(uint8_t *payload, uint32_t buffIdx);

#endif
