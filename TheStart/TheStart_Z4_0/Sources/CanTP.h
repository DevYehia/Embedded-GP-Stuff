#ifndef CANTP_H
#define CANTP_H

#include "can_pal1.h"


#define TX_BUFF_NUM 1
#define RX_BUFF_NUM 0

#define MAX_TP_SIZE 400

#define consecutiveFrameSize 7
#define firstFrameSize 6

#define startFirst 2
#define startConsecutive 1

void CanTP_init(can_instance_t* can_pal1_instance, can_user_config_t* can_pal1_Config0);

//Send control frame
//TODO handle differnet cases of FS
void send_flow_control(uint32_t buffIdx);


void recv_single(void);


void recv_consecutive(void);

void handleConsecutiveFrame();

void handleFlowCtl();

void handleFirstFrame();

void readCanTPPayload(uint8_t size ,uint8_t start);

void send_single_frame(uint8_t *payload);

uint8_t get_payload_size(uint8_t *payload);

void handleSingleFrame();


#endif
