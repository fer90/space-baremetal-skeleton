#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "queue.h"
#include "task.h"

#define TEST_QUEUE_SLOTS     4
#define TEST_QUEUE_CAPACITY  8

typedef struct {
    bool active;
    UBaseType_t length;
    UBaseType_t item_size;
    UBaseType_t head;
    UBaseType_t count;
    uint8_t storage[TEST_QUEUE_CAPACITY * 64];
} test_queue_t;

static test_queue_t test_queues[TEST_QUEUE_SLOTS];
static TickType_t test_tick_count;
static TaskHandle_t test_current_task;
static uint32_t test_notification_bits;

void test_freertos_reset(void)
{
    memset(test_queues, 0, sizeof(test_queues));
    test_tick_count = 0;
    test_current_task = NULL;
    test_notification_bits = 0;
}

void test_freertos_set_tick_count(TickType_t ticks)
{
    test_tick_count = ticks;
}

void test_task_notify_reset(void)
{
    test_notification_bits = 0;
}

uint32_t test_task_notify_get_bits(void)
{
    return test_notification_bits;
}

QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
{
    size_t max_item_size = sizeof(test_queues[0].storage) / TEST_QUEUE_CAPACITY;

    if (uxQueueLength == 0 || uxQueueLength > TEST_QUEUE_CAPACITY || uxItemSize == 0 ||
        uxItemSize > max_item_size) {
        return NULL;
    }

    for (size_t i = 0; i < TEST_QUEUE_SLOTS; i++) {
        test_queue_t *queue = &test_queues[i];

        if (queue->active) {
            continue;
        }

        queue->active = true;
        queue->length = uxQueueLength;
        queue->item_size = uxItemSize;
        queue->head = 0;
        queue->count = 0;
        return (QueueHandle_t) queue;
    }

    return NULL;
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

BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait)
{
    test_queue_t *queue = (test_queue_t *) xQueue;
    UBaseType_t tail;
    uint8_t *slot;

    (void) xTicksToWait;

    if (queue == NULL || !queue->active || pvBuffer == NULL || queue->count == 0) {
        return pdFAIL;
    }

    tail = (queue->head + queue->length - queue->count) % queue->length;
    slot = &queue->storage[tail * queue->item_size];
    memcpy(pvBuffer, slot, (size_t) queue->item_size);
    queue->count--;

    return pdPASS;
}

void vTaskDelay(TickType_t xTicksToDelay)
{
    (void) xTicksToDelay;
}

TickType_t xTaskGetTickCount(void)
{
    return test_tick_count;
}

TaskHandle_t xTaskGetCurrentTaskHandle(void)
{
    return test_current_task;
}

BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction)
{
    (void) eAction;

    if (xTaskToNotify == NULL) {
        return pdFAIL;
    }

    test_notification_bits |= ulValue;
    return pdPASS;
}

BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry,
                           uint32_t ulBitsToClearOnExit,
                           uint32_t *pulNotificationValue,
                           TickType_t xTicksToWait)
{
    uint32_t bits;

    (void) ulBitsToClearOnEntry;
    (void) ulBitsToClearOnExit;
    (void) xTicksToWait;

    if (pulNotificationValue == NULL) {
        return pdFAIL;
    }

    bits = test_notification_bits;
    if (bits == 0) {
        return pdFAIL;
    }

    *pulNotificationValue = bits;
    test_notification_bits = 0;
    return pdPASS;
}