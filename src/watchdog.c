#include "FreeRTOS.h"
#include "task.h"
#include "watchdog.h"
#include "safe_policy.h"
#include "system_state.h"
#include "system_defs.h"
#include "memory_protection.h"
#include "critical_exec.h"
#include "log.h"
#include "uart.h"

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

void watchdog_evaluate_cycle(uint32_t received_bits,
                             uint32_t expected_bits,
                             SystemState_t current_state,
                             uint32_t *successful_cycles)
{
    if (successful_cycles == NULL) {
        return;
    }

    if ((received_bits & expected_bits) == expected_bits) {
        if (current_state == SYSTEM_STATE_DEGRADED) {
            (*successful_cycles)++;

            if (*successful_cycles >= WATCHDOG_RECOVERY_THRESHOLD) {
                (void) system_state_request_change(SYSTEM_STATE_NOMINAL, 0x03u);
                *successful_cycles = 0;
            }
        } else {
            *successful_cycles = 0;
        }
        return;
    }

    if (current_state < SYSTEM_STATE_DEGRADED) {
        (void) system_state_request_change(SYSTEM_STATE_DEGRADED, 0x01u);
    } else if (current_state == SYSTEM_STATE_DEGRADED) {
        (void) system_state_request_change(SYSTEM_STATE_SAFE, 0x01u);
    }
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

    for (;;) {
        uint32_t received = 0;
        const SystemState_t current_state = system_state_get();
        const uint32_t expected_bits = safe_policy_watchdog_expected_bits(current_state);
        const TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(WATCHDOG_TIMEOUT_MS);

        while ((received & expected_bits) != expected_bits) {
            uint32_t bits = 0;
            TickType_t remaining = deadline - xTaskGetTickCount();

            if ((int32_t) remaining <= 0 ||
                xTaskNotifyWait(0, 0, &bits, remaining) != pdPASS) {
                break;
            }
            received |= bits;
        }

        if ((received & expected_bits) != expected_bits) {
            watchdog_print_missing_bits(received);
        }

        watchdog_evaluate_cycle(received, expected_bits, current_state, &successful_cycles);
    }
}