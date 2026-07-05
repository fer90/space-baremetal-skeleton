#include "FreeRTOS.h"
#include "task.h"
#include "command.h"
#include "log.h"
#include "uart.h"
#include "system_state.h"
#include "fault_inject.h"
#include "memory_scrub.h"
#include "memory_protection.h"
#include "safe_policy.h"
#ifdef DEBUG
#include "telemetry.h"
#endif

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
    uart_puts(LOG_PREFIX_CMD "Commands:\r\n");
    uart_puts(LOG_PREFIX_CMD "  s  status (DEBUG telemetry snapshot)\r\n");
    uart_puts(LOG_PREFIX_CMD "  n  request NOMINAL state\r\n");
    uart_puts(LOG_PREFIX_CMD "  d  request DEGRADED state\r\n");
    uart_puts(LOG_PREFIX_CMD "  a  request SAFE state\r\n");
    uart_puts(LOG_PREFIX_CMD "  f  inject one fault now\r\n");
    uart_puts(LOG_PREFIX_CMD "  r  force full memory scrub\r\n");
    uart_puts(LOG_PREFIX_CMD "  v  print memory protection violation count\r\n");
    uart_puts(LOG_PREFIX_CMD "  u  print SEU count\r\n");
    uart_puts(LOG_PREFIX_CMD "  x  toggle automatic fault injection\r\n");
    uart_puts(LOG_PREFIX_CMD "  h  show this help\r\n");
}

bool command_dispatch_char(char c)
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
        case 'a':
            cmd = CMD_GO_SAFE;
            break;
        case 'f':
            cmd = CMD_INJECT_FAULT;
            break;
        case 'r':
            cmd = CMD_FORCE_SCRUB;
            break;
        case 'v':
            cmd = CMD_PRINT_VIOLATIONS;
            break;
        case 'u':
            cmd = CMD_PRINT_SEU_COUNT;
            break;
        case 'x':
            cmd = CMD_TOGGLE_FAULT_INJECT;
            break;
        case 'h':
        case '?':
            command_print_help();
            return true;
        default:
            return false;
    }

    sent = command_send(cmd);
    if (sent != pdPASS) {
        uart_puts(LOG_PREFIX_CMD "queue full\r\n");
    }
    return true;
}

void command_handle(CommandType_t cmd)
{
    switch (cmd) {
        case CMD_STATUS:
#ifdef DEBUG
            telemetry_print_snapshot();
#else
            uart_puts(LOG_PREFIX_CMD "telemetry requires DEBUG build\r\n");
#endif
            break;

        case CMD_GO_NOMINAL:
            system_state_request_change(SYSTEM_STATE_NOMINAL, 0x10);
            break;

        case CMD_GO_DEGRADED:
            system_state_request_change(SYSTEM_STATE_DEGRADED, 0x10);
            break;

        case CMD_GO_SAFE:
            system_state_request_change(SYSTEM_STATE_SAFE, 0x10);
            break;

        case CMD_INJECT_FAULT:
            if (!safe_policy_allows_fault_inject(system_state_get())) {
                uart_puts(LOG_PREFIX_CMD "fault inject blocked in SAFE\r\n");
                break;
            }
            inject_random_fault(scrub_area);
            break;

        case CMD_FORCE_SCRUB:
            memory_scrub(scrub_area);
            break;

        case CMD_PRINT_VIOLATIONS:
            uart_puts(LOG_PREFIX_CMD "mem_prot violations: ");
            uart_put_dec(memory_protection_get_violation_count());
            uart_puts("\r\n");
            break;

        case CMD_PRINT_SEU_COUNT:
            uart_puts(LOG_PREFIX_CMD "SEU count: ");
            uart_put_dec(memory_scrub_get_seu_count());
            uart_puts("\r\n");
            break;

        case CMD_TOGGLE_FAULT_INJECT:
            fault_inject_set_enabled(!fault_inject_is_enabled());
            uart_puts(LOG_PREFIX_CMD "fault injection: ");
            uart_puts(fault_inject_is_enabled() ? "ON\r\n" : "OFF\r\n");
            break;

        default:
            uart_puts(LOG_PREFIX_CMD "unknown command\r\n");
            break;
    }
}

void vTaskCommandInput(void *pvParameters)
{
    (void) pvParameters;

    uart_puts(LOG_PREFIX_CMD "input ready (h for help)\r\n");

    for (;;) {
        if (uart_rx_ready()) {
            char c = uart_getc();

            if (c == '\r' || c == '\n') {
                continue;
            }

            if (!command_dispatch_char(c)) {
                uart_puts(LOG_PREFIX_CMD "unknown key '");
                uart_putc(c);
                uart_puts("' (h for help)\r\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vTaskCommandHandler(void *pvParameters)
{
    (void) pvParameters;
    CommandType_t cmd;

    for (;;) {
        if (xQueueReceive(xCommandQueue, &cmd, portMAX_DELAY) == pdPASS) {
            command_handle(cmd);
        }
    }
}