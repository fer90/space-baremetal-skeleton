#include "command.h"
#include "common.h"
#include "system_state.h"

QueueHandle_t xCommandQueue = NULL;

void command_init(void)
{
    xCommandQueue = xQueueCreate(8, sizeof(CommandType_t));
}

BaseType_t command_send(CommandType_t cmd)
{
    if (xCommandQueue == NULL) {
        return pdFAIL;
    }
    return xQueueSend(xCommandQueue, &cmd, 0);
}

static void command_print_help(void)
{
    uart_puts(
        "Commands: s=status n=nominal d=degraded f=fault c=scrub ?=help\r\n");
}

static bool command_dispatch_char(char c)
{
    CommandType_t cmd;
    BaseType_t sent;

    switch (c) {
        case 's':
            cmd = CMD_STATUS;
            break;
        case 'n':
            cmd = CMD_GO_NOMINAL;
            break;
        case 'd':
            cmd = CMD_GO_DEGRADED;
            break;
        case 'f':
            cmd = CMD_INJECT_FAULT;
            break;
        case 'c':
            cmd = CMD_FORCE_SCRUB;
            break;
        case '?':
            command_print_help();
            return true;
        default:
            return false;
    }

    sent = command_send(cmd);
    if (sent != pdPASS) {
        uart_puts("CMD: queue full\r\n");
    }
    return true;
}

void vTaskCommandInput(void *pvParameters)
{
    (void) pvParameters;

    uart_puts("Command input ready (? for help)\r\n");

    for (;;) {
        if (uart_rx_ready()) {
            char c = uart_getc();

            if (c == '\r' || c == '\n') {
                continue;
            }

            if (!command_dispatch_char(c)) {
                uart_puts("CMD: unknown key '");
                uart_putc(c);
                uart_puts("' (? for help)\r\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vTaskCommandHandler(void *pvParameters)
{
    (void) pvParameters;
    CommandType_t cmd;

    for (;;)
    {
        if (xQueueReceive(xCommandQueue, &cmd, portMAX_DELAY) == pdPASS)
        {
            switch (cmd)
            {
                case CMD_STATUS:
#ifdef DEBUG
                    telemetry_print_snapshot();
#else
                    uart_puts("CMD_STATUS: telemetry requires DEBUG build\r\n");
#endif
                    break;

                case CMD_GO_NOMINAL:
                    system_state_request_change(SYSTEM_STATE_NOMINAL, 0x10);
                    break;

                case CMD_GO_DEGRADED:
                    system_state_request_change(SYSTEM_STATE_DEGRADED, 0x10);
                    break;

                case CMD_INJECT_FAULT:
                    inject_random_fault(scrub_area);
                    break;

                case CMD_FORCE_SCRUB:
                    memory_scrub(scrub_area);
                    break;

                default:
                    uart_puts("CMD: unknown command\r\n");
                    break;
            }
        }
    }
}
