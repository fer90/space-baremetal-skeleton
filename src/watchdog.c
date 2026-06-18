#include "common.h"

#define WATCHDOG_TIMEOUT 50 // Adjust based on loop speed

volatile uint32_t watchdog_counter = 0;

void watchdog_init(void) {
    watchdog_counter = WATCHDOG_TIMEOUT;
    uart_puts("Watchdog initialized\r\n");
}

void watchdog_kick(void) {
    watchdog_counter = 0;
    uart_puts("Watchdog kicked\r\n");
}

void watchdog_check(void) {
    if (watchdog_counter > 0) {
        watchdog_counter--;
    } else {
        uart_puts("WATCHDOG TIMEOUT - Resetting (FDIR demo)\r\n");
        // In real hardware: trigger reset (e.g., via watchdog peripheral)
        while(1);  // Simulate reset hang for demo
    }
}

void vTaskWatchdog(void *pvParameters) {

    (void) pvParameters;

    for(;;) {
        watchdog_kick();
	watchdog_check();

	vTaskDelay(pdMS_TO_TICKS(200));
    }
}
