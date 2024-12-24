#ifndef CANTP_H
#define CANTP_H

#include "can_pal1.h";

void CanTP_init(can_instance_t* can_pal1_instance, can_user_config_t* can_pal1_Config0);

//Send control frame
//TODO handle differnet cases of FS
void send_flow_control(void);


void recv_single(void);


void recv_consecutive(void);





#endif