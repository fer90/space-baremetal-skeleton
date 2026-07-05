#include "unity.h"

#include "system_state.h"
#include "queue.h"

void test_system_state_init_sets_boot_and_creates_queue(void)
{
    TEST_ASSERT_NOT_NULL(xStateRequestQueue);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_BOOT, system_state_get());
}

void test_system_state_request_change_enqueues_state_and_reason(void)
{
    StateRequest_t request;

    TEST_ASSERT_EQUAL(pdPASS, system_state_request_change(SYSTEM_STATE_DEGRADED, 0x10u));
    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xStateRequestQueue, &request, 0));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_DEGRADED, request.requested_state);
    TEST_ASSERT_EQUAL_UINT32(0x10u, request.reason_code);
}

void test_system_state_get_returns_current_state(void)
{
    gSystemState = SYSTEM_STATE_SAFE;
    TEST_ASSERT_EQUAL(SYSTEM_STATE_SAFE, system_state_get());
}

void test_system_state_request_change_fails_when_queue_uninitialized(void)
{
    xStateRequestQueue = NULL;
    TEST_ASSERT_EQUAL(pdFAIL, system_state_request_change(SYSTEM_STATE_DEGRADED, 0x10u));
}