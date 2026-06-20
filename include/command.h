#ifndef COMMAND_H
#define COMMAND_H

#include "FreeRTOS.h"
#include "queue.h"

typedef enum {
    CMD_STATUS = 0,
    CMD_GO_NOMINAL,
    CMD_GO_DEGRADED,
    CMD_INJECT_FAULT,
    CMD_FORCE_SCRUB,
} CommandType_t;

extern QueueHandle_t xCommandQueue;

void command_init(void);
BaseType_t command_send(CommandType_t cmd);
void vTaskCommandHandler(void *pvParameters);
void vTaskCommandInput(void *pvParameters);

#endif
