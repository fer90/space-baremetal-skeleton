#include "system_state.h"
#include "common.h"

volatile SystemState_t gSystemState = SYSTEM_STATE_BOOT;
QueueHandle_t xStateRequestQueue = NULL;

void system_state_init(void)
{
    xStateRequestQueue = xQueueCreate(4, sizeof(StateRequest_t));
    gSystemState = SYSTEM_STATE_BOOT;
    uart_puts(LOG_PREFIX_STATE "BOOT\r\n");
}

SystemState_t system_state_get(void)
{
    return gSystemState;
}

BaseType_t system_state_request_change(SystemState_t new_state, uint32_t reason) {
    if (xStateRequestQueue == NULL) {
        return pdFAIL;
    }

    StateRequest_t request = {
        .requested_state = new_state,
        .reason_code = reason,
    };

    return xQueueSend(xStateRequestQueue, &request, 0);
}

