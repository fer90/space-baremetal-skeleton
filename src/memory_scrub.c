#include "common.h"
#include "uart.h"
#include "memory_scrub.h"

// Keep a golden copy in memory
static uint8_t golden_copy[SCRUB_SIZE];

void memory_scrub_init(volatile uint8_t *area) {
    // Fill with a pattern (0xAA) to show it's working
    for (int i = 0; i < SCRUB_SIZE; i++) {
        area[i] = (uint8_t)i;
        golden_copy[i] = (uint8_t)i; // Save the original reference
    }
    uart_puts("Memory scrub initialized (EDAC Simulation)\r\n");
}

void memory_scrub(volatile uint8_t *area) {
    uint32_t errors_corrected = 0;
    // In a real system, this would:
    // 1. Read EDAC error registers
    // 2. Identify bad memory addresses
    // 3. Fetch correct data from ECC or backup
    // 4. Write corrected data to bad address

    // For demo: just toggle pattern every call
    for (int i = 0; i < SCRUB_SIZE; i++) {
        // Simulate SEU check/correct
        if (area[i] != golden_copy[i]) {
            area[i] = golden_copy[i]; // "Correct it"
            errors_corrected++;
        }
    }
    if (errors_corrected > 0) {
        uart_puts("Memory scrub: SEU(s) detected and corrected\r\n");
    } else {
        uart_puts("Memory scrub completed - No errors\r\n");
    }
}
