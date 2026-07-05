#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>

#include "FreeRTOS.h"
#include "queue.h"

typedef enum {
    CMD_STATUS = 0,
    CMD_GO_NOMINAL,
    CMD_GO_DEGRADED,
    CMD_GO_SAFE,
    CMD_INJECT_FAULT,
    CMD_FORCE_SCRUB,
    CMD_PRINT_VIOLATIONS,
    CMD_PRINT_SEU_COUNT,
    CMD_TOGGLE_FAULT_INJECT,
    CMD_DUMP_EVENT_LOG,
} CommandType_t;

extern QueueHandle_t xCommandQueue;

void command_init(void);
BaseType_t command_send(CommandType_t cmd);
bool command_dispatch_char(char c);
void command_handle(CommandType_t cmd);
void vTaskCommandHandler(void *pvParameters);
void vTaskCommandInput(void *pvParameters);

#endif
