#ifndef FAULT_QUEUE_H
#define FAULT_QUEUE_H

#include "FreeRTOS.h"
#include "queue.h"

typedef struct {
    uint32_t index;
    uint8_t  bit;
} FaultEvent_t;

extern QueueHandle_t xFaultQueue;

void fault_queue_init(void);

#endif
