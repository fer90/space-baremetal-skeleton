#include <stdint.h>
#include "uart.h"
#include "common.h"
#include "watchdog.h"
#include "memory_scrub.h"
#include "fault_inject.h"
#include "stack_paint.h"
#include "timer.h"

volatile uint8_t scrub_area[SCRUB_SIZE];

void heartbeat(void) {
    uart_puts("HEARTBEAT - Dependability (MTTF / Watchdog) active\r\n");
}

int main(void) {
    uart_puts("\r\n=== Space Bare-Metal Skeleton Booted ===\r\n");
    uart_puts("Power, Dependability, Performance in action\r\n\r\n");

    // interrupt_init() and timer_init() are now called from startup.s

    watchdog_init();
    memory_scrub_init(scrub_area);
    
    // Dump first 64 bytes of scrub_area
    uart_puts("Scrub area dump:\r\n");
    uart_hex_dump(scrub_area, 64);

    fault_inject_init();
    stack_paint_init();

    uint32_t last_ticks = 0;

    while (1) {

        //watchdog_kick();

        uint32_t ticks = get_system_ticks();

        // Only run periodic tasks when ticks have advanced
        if (ticks != last_ticks) {
            last_ticks = ticks;

            // Heartbeat every ~5s (500 ticks @ 10ms each)
            if (ticks % 500 == 0) heartbeat();
            // Memory scrub every ~2s
            if (ticks % 200 == 0) memory_scrub(scrub_area);
            // Fault injection every ~3s
            if (ticks % 300 == 0) inject_random_fault(scrub_area);
            // Stack check every ~4s
            if (ticks % 400 == 0) stack_check();
        }

        watchdog_check();

        // Low-power idle: wait for next interrupt
        asm volatile("wfi");
    }
    return 0;
}