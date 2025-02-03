#ifndef FRAME_TYPES_H
#define FRAME_TYPES_H

#define MAX_BUFF_SIZE 400


typedef struct dataFrame
{
    uint8_t ready;
    uint8_t dataBuffer[MAX_BUFF_SIZE];
    uint8_t dataSize;
} dataFrame;






#endif