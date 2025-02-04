#include <stdint.h>
#include <stdbool.h>
#include "MPC5748G.h"

#define TEST_MEMORY_ADDRESS  0x4000FF27  // Test memory location for bootloader task

void delay(uint32_t count) {
    while (count--) {
    }
}

int main(void) {
    while (1) {
    	*((volatile uint32_t *)TEST_MEMORY_ADDRESS) = 0xBEEFBEEF;
        delay(1000000);
    }
    return 0;
}
