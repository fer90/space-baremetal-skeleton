#include "FreeRTOS.h"
#include "task.h"
#include "state_machine.h"
#include "safe_policy.h"
#include "event_log.h"
#include "log.h"
#include "uart.h"

static bool state_machine_allows_request(SystemState_t current, const StateRequest_t *request)
{
    SystemState_t requested = request->requested_state;

    if (requested >= current) {
        return true;
    }

    if (requested == SYSTEM_STATE_NOMINAL &&
        (current == SYSTEM_STATE_DEGRADED || current == SYSTEM_STATE_SAFE) &&
        (request->reason_code == 0x03u || request->reason_code == 0x10u)) {
        return true;
    }

    return false;
}

bool state_machine_apply_request(SystemState_t *current, const StateRequest_t *request)
{
    if (current == NULL || request == NULL) {
        return false;
    }

    if (!state_machine_allows_request(*current, request)) {
        return false;
    }

    if (*current == request->requested_state) {
        return false;
    }

    *current = request->requested_state;
    return true;
}

static void state_machine_log_transition(SystemState_t state)
{
    uart_puts(LOG_PREFIX_STATE "changed to ");
    switch (state) {
        case SYSTEM_STATE_NOMINAL:
            uart_puts("NOMINAL\r\n");
            break;
        case SYSTEM_STATE_DEGRADED:
            uart_puts("DEGRADED\r\n");
            break;
        case SYSTEM_STATE_SAFE:
            uart_puts("SAFE\r\n");
            break;
        default:
            uart_puts("UNKNOWN\r\n");
            break;
    }
}

void vTaskStateMachine(void *pvParameters)
{
    (void) pvParameters;
    StateRequest_t request;

    system_state_init();

    gSystemState = SYSTEM_STATE_NOMINAL;
    uart_puts(LOG_PREFIX_STATE "NOMINAL\r\n");
    event_log_record_state_change(SYSTEM_STATE_BOOT, SYSTEM_STATE_NOMINAL, 0u);

    for (;;) {
        if (xQueueReceive(xStateRequestQueue, &request, pdMS_TO_TICKS(1000)) == pdPASS) {
            SystemState_t current = gSystemState;

            if (state_machine_apply_request(&current, &request)) {
                const SystemState_t previous = gSystemState;

                gSystemState = current;
                state_machine_log_transition(current);
                event_log_record_state_change(previous, current, request.reason_code);

                if (current == SYSTEM_STATE_SAFE && previous != SYSTEM_STATE_SAFE) {
                    safe_policy_on_enter();
                } else if (previous == SYSTEM_STATE_SAFE && current != SYSTEM_STATE_SAFE) {
                    safe_policy_on_exit();
                }
            }
        }
    }
}