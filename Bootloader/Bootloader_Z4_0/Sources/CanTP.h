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

void CanTP_init();


void send_flow_control(char type, uint32_t buffIdx);

void recv_single(void);

void recv_consecutive(void);

void handleConsecutiveFrame();

void handleFlowCtl();

void handleFirstFrame();

void readCanTPPayload(uint8_t size, uint8_t start);

void send_single_frame(uint8_t *payload, uint32_t buffIdx);

uint8_t get_payload_size(uint8_t *payload);

void handleSingleFrame();

void sendFromUDS(void *pv);
void recieve(void *pv);
void timeOutHandle();
void Can_init(can_instance_t *can_pal_instance, can_user_config_t *can_pal_Config);

void resetCanTP();
void send_consecutive_frame(uint8_t *payload, uint32_t buffIdx);

#endif
