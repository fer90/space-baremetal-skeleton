#include "FreeRTOS.h"
#include "task.h"
#include "system_state.h"
#include "common.h"

void vTaskStateMachine(void *pvParameters) {
    (void) pvParameters;
    StateRequest_t request;

    system_state_init();

    gSystemState = SYSTEM_STATE_NOMINAL;
    uart_puts("System State: NOMINAL\r\n");

    for (;;) {
        if (xQueueReceive(xStateRequestQueue, &request, pdMS_TO_TICKS(1000)) == pdPASS) {
            if (request.requested_state >= gSystemState) {
                if (gSystemState != request.requested_state) {
                    gSystemState = request.requested_state;

                    uart_puts("System State changed to: ");
                    switch (gSystemState) {
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
            }
        }
    }
}