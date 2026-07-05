#include "unity.h"

#include "fault_queue.h"
#include "queue.h"

void test_fault_queue_init_succeeds(void)
{
    test_freertos_reset();
    xFaultQueue = NULL;

    TEST_ASSERT_TRUE(fault_queue_init());
    TEST_ASSERT_NOT_NULL(xFaultQueue);
}

void test_fault_queue_send_receive_roundtrip(void)
{
    FaultEvent_t sent = {.index = 7u, .bit = 2u};
    FaultEvent_t received;

    TEST_ASSERT_EQUAL(pdPASS, xQueueSend(xFaultQueue, &sent, 0));
    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xFaultQueue, &received, 0));
    TEST_ASSERT_EQUAL_UINT32(sent.index, received.index);
    TEST_ASSERT_EQUAL_UINT8(sent.bit, received.bit);
}