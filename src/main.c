#include <stdint.h>
#include "uart.h"
#include "common.h"

volatile uint32_t system_ticks = 0;

void heartbeat(void) {
    uart_puts("HEARTBEAT - Dependability (MTTF / Watchdog) active\r\n");
}

int main(void) {
    uart_puts("\r\n=== Space Bare-Metal Skeleton Booted ===\r\n");
    uart_puts("Power, Dependability, Performance in action\r\n\r\n");

    watchdog_init();
    memory_scrub_init();

    while (1) {
        system_ticks++;

        //watchdog_kick();

        if (system_ticks % 500000 == 0) {     // Adjusted for visibility
            heartbeat();
        }
        if (system_ticks % 250000 == 0) {
            memory_scrub();
        }

        watchdog_check();

        // Small delay to prevent flooding QEMU console
        for (volatile int i = 0; i < 800; i++) {}
    }
    return 0;
}