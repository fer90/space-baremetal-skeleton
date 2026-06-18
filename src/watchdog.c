#include "common.h"

TaskHandle_t xWatchdogTaskHandle = NULL;

void watchdog_kick(void) {
    if (xWatchdogTaskHandle != NULL) {
        xTaskNotifyGive(xWatchdogTaskHandle);
    }
}

void vTaskWatchdog(void *pvParameters) {

    (void) pvParameters;
    xWatchdogTaskHandle = xTaskGetCurrentTaskHandle();

    for(;;) {
        uint32_t kickCount = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000));
	if (kickCount > 0) {
	    // System is alive
	} else {
	    uart_puts("WATCHDOG TIMEOUT - No Kick received!\r\n");
	    for(;;); // Here we would trigger recovery
	}
    }
}
