#include "unity.h"

#include "fault_inject.h"
#include "fault_queue.h"
#include "memory_scrub.h"
#include "queue.h"

void test_fault_inject_enabled_by_default(void)
{
    TEST_ASSERT_TRUE(fault_inject_is_enabled());
}

void test_fault_inject_set_enabled_toggles(void)
{
    TEST_ASSERT_FALSE(fault_inject_set_enabled(false));
    TEST_ASSERT_FALSE(fault_inject_is_enabled());
    TEST_ASSERT_TRUE(fault_inject_set_enabled(true));
    TEST_ASSERT_TRUE(fault_inject_is_enabled());
}

void test_fault_inject_corrupts_buffer_and_queues_event(void)
{
    volatile uint8_t area[SCRUB_SIZE];
    FaultEvent_t event;
    uint8_t before;

    test_freertos_set_tick_count(42u);
    memory_scrub_init(area);
    before = area[42];

    inject_random_fault(area);

    TEST_ASSERT_NOT_EQUAL(before, area[42]);
    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xFaultQueue, &event, 0));
    TEST_ASSERT_EQUAL_UINT32(42u, event.index);
    TEST_ASSERT_EQUAL_UINT8(0u, event.bit);
}