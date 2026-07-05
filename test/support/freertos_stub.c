#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "queue.h"

#define TEST_QUEUE_CAPACITY 8

typedef struct {
    bool active;
    UBaseType_t length;
    UBaseType_t item_size;
    UBaseType_t head;
    UBaseType_t count;
    uint8_t storage[TEST_QUEUE_CAPACITY * 64];
} test_queue_t;

static test_queue_t test_queue;

void test_freertos_reset(void)
{
    memset(&test_queue, 0, sizeof(test_queue));
}

QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
{
    if (uxQueueLength == 0 || uxQueueLength > TEST_QUEUE_CAPACITY || uxItemSize == 0 ||
        uxItemSize > sizeof(test_queue.storage) / TEST_QUEUE_CAPACITY) {
        return NULL;
    }

    test_queue.active = true;
    test_queue.length = uxQueueLength;
    test_queue.item_size = uxItemSize;
    test_queue.head = 0;
    test_queue.count = 0;

    return (QueueHandle_t) &test_queue;
}

BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait)
{
    test_queue_t *queue = (test_queue_t *) xQueue;
    uint8_t *slot;

    (void) xTicksToWait;

    if (queue == NULL || !queue->active || pvItemToQueue == NULL) {
        return pdFAIL;
    }

    if (queue->count >= queue->length) {
        return pdFAIL;
    }

    slot = &queue->storage[queue->head * queue->item_size];
    memcpy(slot, pvItemToQueue, (size_t) queue->item_size);
    queue->head = (queue->head + 1) % queue->length;
    queue->count++;

    return pdPASS;
}