#ifndef STACK_HWM_H
#define STACK_HWM_H

#ifdef DEBUG

#include "FreeRTOS.h"
#include "task.h"

void stack_hwm_register(TaskHandle_t handle,
                        const char *name,
                        configSTACK_DEPTH_TYPE allocated_words);

void stack_hwm_print_snapshot(void);

void vTaskStackHwmMonitor(void *pvParameters);

#endif /* DEBUG */

#endif /* STACK_HWM_H */