#include "common.h"
#include "critical_exec.h"

TaskHandle_t xWatchdogTaskHandle = NULL;

static void CRITICAL_TEXT watchdog_kick_impl(uint32_t taskBit)
{
    if (xWatchdogTaskHandle != NULL) {
        xTaskNotify(xWatchdogTaskHandle, taskBit, eSetBits);
    }
}

void watchdog_kick(uint32_t taskBit)
{
    if (!memory_protection_check_access((uintptr_t) watchdog_kick_impl, 1, MEM_PERM_EXEC)) {
        uart_puts(LOG_PREFIX_VIOLATION "watchdog kick blocked by memory protection\r\n");
        return;
    }

    watchdog_kick_impl(taskBit);
}

static void watchdog_print_missing_bits(uint32_t received)
{
    uint32_t missing = WATCHDOG_EXPECTED_BITS & ~received;

    uart_puts(LOG_PREFIX_ERROR "WATCHDOG TIMEOUT missing:");
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

void vTaskWatchdog(void *pvParameters)
{
    (void) pvParameters;
    xWatchdogTaskHandle = xTaskGetCurrentTaskHandle();

    uint32_t successful_cycles = 0;
    const uint32_t RECOVERY_THRESHOLD = 5;

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

        SystemState_t current_state = system_state_get();

        if ((received & WATCHDOG_EXPECTED_BITS) == WATCHDOG_EXPECTED_BITS) {
            if (current_state == SYSTEM_STATE_DEGRADED) {
                successful_cycles++;

                if (successful_cycles >= RECOVERY_THRESHOLD) {
                    system_state_request_change(SYSTEM_STATE_NOMINAL, 0x03);
                    successful_cycles = 0;
                }
            } else {
                successful_cycles = 0;
            }
        } else {
            watchdog_print_missing_bits(received);

            if (current_state < SYSTEM_STATE_DEGRADED) {
                system_state_request_change(SYSTEM_STATE_DEGRADED, 0x01);
            } else if (current_state == SYSTEM_STATE_DEGRADED) {
                system_state_request_change(SYSTEM_STATE_SAFE, 0x01);
            }
        }
    }
}