#ifndef RESET_WRAPPER_H
#define RESET_WRAPPER_H

#include "MPC5748G.h"

#define FUNC_RESET 0x0
#define DEST_RESET 0xF



void Do_Reset(uint8_t);

#define SOFT_RESET() (Do_Reset(FUNC_RESET))
#define HARD_RESET() (Do_Reset(DEST_RESET))



#endif