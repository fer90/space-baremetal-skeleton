#ifndef TASK_H
#define TASK_H

#include "FreeRTOS.h"

#define pdMS_TO_TICKS(xTimeInMs) ((TickType_t) (xTimeInMs))

typedef enum {
    eNoAction = 0,
    eSetBits,
    eIncrement,
    eSetValueWithOverwrite,
    eSetValueWithoutOverwrite,
} eNotifyAction;

void vTaskDelay(TickType_t xTicksToDelay);
TickType_t xTaskGetTickCount(void);
TaskHandle_t xTaskGetCurrentTaskHandle(void);
BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction);
BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry,
                           uint32_t ulBitsToClearOnExit,
                           uint32_t *pulNotificationValue,
                           TickType_t xTicksToWait);

uint32_t test_task_notify_get_bits(void);
void test_task_notify_reset(void);

#endif /* TASK_H */