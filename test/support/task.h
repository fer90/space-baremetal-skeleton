#ifndef TASK_H
#define TASK_H

#include "FreeRTOS.h"

#define pdMS_TO_TICKS(xTimeInMs) ((TickType_t) (xTimeInMs))

void vTaskDelay(TickType_t xTicksToDelay);
TickType_t xTaskGetTickCount(void);

#endif /* TASK_H */