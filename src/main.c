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

    timer_init();
    watchdog_init();
    memory_scrub_init(scrub_area);
    fault_inject_init();
    stack_paint_init();

    while (1) {
        timer_tick();

        //watchdog_kick();

        if (get_system_ticks() % 600000 == 0) {     // Adjusted for visibility
            heartbeat();
        }
        if (get_system_ticks() % 150000 == 0) {
            memory_scrub(scrub_area);
        }
        if (get_system_ticks() % 80000 == 0) {
            inject_random_fault(scrub_area);
        }
        if (get_system_ticks() % 100000 == 0) {
            stack_check();
        }

        watchdog_check();

        // Small delay to prevent flooding QEMU console
        for (volatile int i = 0; i < 200; i++) {}
    }
    return 0;
}