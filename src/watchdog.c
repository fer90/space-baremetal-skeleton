#include "common.h"

TaskHandle_t xWatchdogTaskHandle = NULL;

void watchdog_kick(uint32_t taskBit) {
    if (xWatchdogTaskHandle != NULL) {
        xTaskNotify(xWatchdogTaskHandle, taskBit, eSetBits);
    }
}

static void watchdog_print_missing_bits(uint32_t received) {
    uint32_t missing = WATCHDOG_EXPECTED_BITS & ~received;

    uart_puts("WATCHDOG TIMEOUT - missing:");
    if (missing & WATCHDOG_BIT_HEARTBEAT) {
        uart_puts(" Heartbeat");
    }
    if (missing & WATCHDOG_BIT_MEMSCRUB) {
        uart_puts(" MemScrub");
    }
    if (missing & WATCHDOG_BIT_FAULTINJECT) {
        uart_puts(" FaultInject");
    }
    uart_puts("\r\n");
}

void vTaskWatchdog(void *pvParameters) {
    (void) pvParameters;
    xWatchdogTaskHandle = xTaskGetCurrentTaskHandle();

    for (;;) {
        uint32_t received = 0;
        const TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(WATCHDOG_TIMEOUT_MS);

        while ((received & WATCHDOG_EXPECTED_BITS) != WATCHDOG_EXPECTED_BITS) {
            uint32_t bits = 0;
            TickType_t remaining = deadline - xTaskGetTickCount();

            if ((int32_t) remaining <= 0 ||
                xTaskNotifyWait(0, 0, &bits, remaining) != pdPASS) {
                break;
            }
            received |= bits;
        }

        if ((received & WATCHDOG_EXPECTED_BITS) != WATCHDOG_EXPECTED_BITS) {
            watchdog_print_missing_bits(received);
            for (;;) {
            }
        }
    }
}