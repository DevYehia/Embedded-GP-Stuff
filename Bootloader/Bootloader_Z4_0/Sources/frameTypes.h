#ifndef FRAME_TYPES_H
#define FRAME_TYPES_H

#define MAX_BUFF_SIZE 400


#include "can_pal1.h"


typedef struct dataFrame
{
    uint8_t ready;
    uint8_t dataBuffer[MAX_BUFF_SIZE];
    uint16_t dataSize;
} dataFrame;






#endif
