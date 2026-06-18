#include "common.h"

TaskHandle_t xWatchdogTaskHandle = NULL;

void watchdog_kick(uint32_t taskBit) {
    if (xWatchdogTaskHandle != NULL) {
        xTaskNotify(xWatchdogTaskHandle, taskBit, eSetBits);
    }
}

void vTaskWatchdog(void *pvParameters) {
    (void) pvParameters;
    xWatchdogTaskHandle = xTaskGetCurrentTaskHandle();

    for (;;) {
        uint32_t received = 0;
        TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(WATCHDOG_TIMEOUT_MS);

        while ((received & WATCHDOG_EXPECTED_BITS) != WATCHDOG_EXPECTED_BITS) {
            TickType_t now = xTaskGetTickCount();
            TickType_t wait = deadline - now;

            if ((int32_t) wait <= 0) {
                break;
            }

            uint32_t bits = 0;
            if (xTaskNotifyWait(0, 0, &bits, wait) == pdPASS) {
                received |= bits;
            } else {
                break;
            }
        }

        if ((received & WATCHDOG_EXPECTED_BITS) == WATCHDOG_EXPECTED_BITS) {
            continue;
        }

        uart_puts("WATCHDOG TIMEOUT - Missing ticks!\r\n");
        for (;;) {
        }
    }
}