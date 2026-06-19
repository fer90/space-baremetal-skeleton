#ifndef TELEMETRY_H
#define TELEMETRY_H

#ifdef DEBUG

#include "FreeRTOS.h"
#include "task.h"

void telemetry_register_task(TaskHandle_t handle,
                             const char *name,
                             configSTACK_DEPTH_TYPE allocated_words);

void telemetry_print_snapshot(void);

void vTaskTelemetry(void *pvParameters);

#endif /* DEBUG */

#endif /* TELEMETRY_H */