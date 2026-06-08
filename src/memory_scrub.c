#include "common.h"
#include "uart.h"

#define SCRUB_SIZE 512 // Bytes to scrub per call (small for demo)

volatile uint8_t scrub_area[SCRUB_SIZE];

void memory_scrub_init(void) {
    // Fill with a pattern (0xAA) to show it's working
    for (int i = 0; i < SCRUB_SIZE; i++) {
        scrub_area[i] = (uint8_t)i;
    }
    uart_puts("Memory scrub initialized (EDAC Simulation)\r\n");
}

void memory_scrub(void) {
    // In a real system, this would:
    // 1. Read EDAC error registers
    // 2. Identify bad memory addresses
    // 3. Fetch correct data from ECC or backup
    // 4. Write corrected data to bad address

    // For demo: just toggle pattern every call
    for (int i = 0; i < SCRUB_SIZE; i += 8) {
        // Simulate SEU check/correct
        if ((scrub_area[i] & 0x01) == 0 && (i % 32 == 0)) { // Fake error condition
            scrub_area[i] ^= 0x01; // "Correct it"
        }
    }
    uart_puts("Memory scrub completed - SEU mitigation\r\n");
}
