#ifndef FAULT_QUEUE_H
#define FAULT_QUEUE_H

#include <stdbool.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "system_defs.h"

extern QueueHandle_t xFaultQueue;

bool fault_queue_init(void);

#endif /* FAULT_QUEUE_H */