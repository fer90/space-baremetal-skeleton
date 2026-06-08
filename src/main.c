#include <stdint.h>
#include "uart.h"

volatile uint32_t system_ticks = 0;

void heartbeat(void) {
    uart_puts("HEARTBEAT - Dependability (MTTF / Watchdog) active\r\n");
}

void memory_scrub(void) {
    uart_puts("Memory scrub - Simulating EDAC / SEU mitigation\r\n");
}

int main(void) {
    uart_puts("\r\n=== Space Bare-Metal Skeleton Booted (Ch1 Concepts) ===\r\n");
    uart_puts("Power, Dependability, Performance in action\r\n\r\n");

    while (1) {
        system_ticks++;

        if (system_ticks % 200000 == 0) {     // Adjusted for visibility
            heartbeat();
        }
        if (system_ticks % 100000 == 0) {
            memory_scrub();
        }

        // Small delay to prevent flooding QEMU console
        for (volatile int i = 0; i < 500; i++) {}
    }
    return 0;
}