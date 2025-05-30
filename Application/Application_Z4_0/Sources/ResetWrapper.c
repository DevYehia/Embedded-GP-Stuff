#include "ResetWrapper.h"

// To do functional reset we do two writes
// one with the key
// another one with the inverted key

/*
These bits provide the target chip mode to be entered by software programming. The mechanism to enter
into any mode by software requires the write operation twice: first time with key, and second time with
inverted key. These bits are automatically updated by hardware while entering SAFE on hardware
request. Also, while exiting from the STOP0 modes on hardware exit events, these are updated with the
appropriate RUN0…3 mode value.

*/

void Do_Reset(uint8_t resetType)
{

    uint32_t invKey = MC_ME_MCTL_KEY(MC_ME->MCTL);

    uint32_t firstWrite;
    uint32_t secondWrite;
    if (resetType == FUNC_RESET)
    {
        firstWrite = 0x00005AF0;
        secondWrite = 0x0000A50F;
    }
    else if (resetType == DEST_RESET)
    {
        firstWrite = 0xF0005AF0;
        secondWrite = 0xF000A50F;
    }

    // first write with key
    MC_ME->MCTL = firstWrite;

    // second write with inverted key
    MC_ME->MCTL = secondWrite;
}