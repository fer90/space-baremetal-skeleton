#include "fault_queue.h"

QueueHandle_t xFaultQueue = NULL;

void fault_queue_init(void)
{
    xFaultQueue = xQueueCreate(5, sizeof(FaultEvent_t));  // Queue of 5 events
}

