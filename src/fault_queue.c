#include "fault_queue.h"

#define FAULT_QUEUE_LENGTH 5

QueueHandle_t xFaultQueue = NULL;

bool fault_queue_init(void) {
    xFaultQueue = xQueueCreate(FAULT_QUEUE_LENGTH, sizeof(FaultEvent_t));
    return xFaultQueue != NULL;
}