#include "unity.h"

#include "test_support.h"
#include "system_defs.h"
#include "watchdog.h"
#include "system_state.h"
#include "queue.h"
#include "task.h"

extern TaskHandle_t xWatchdogTaskHandle;

void test_watchdog_complete_cycle_from_nominal_resets_recovery_counter(void)
{
    uint32_t successful_cycles = 3u;

    watchdog_evaluate_cycle(WATCHDOG_EXPECTED_BITS, SYSTEM_STATE_NOMINAL, &successful_cycles);

    TEST_ASSERT_EQUAL_UINT32(0u, successful_cycles);
    TEST_ASSERT_TRUE(test_state_request_queue_empty());
}

void test_watchdog_timeout_from_nominal_requests_degraded(void)
{
    uint32_t successful_cycles = 0u;
    StateRequest_t request;

    watchdog_evaluate_cycle(WATCHDOG_BIT_HEARTBEAT, SYSTEM_STATE_NOMINAL, &successful_cycles);

    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xStateRequestQueue, &request, 0));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_DEGRADED, request.requested_state);
    TEST_ASSERT_EQUAL_UINT32(0x01u, request.reason_code);
}

void test_watchdog_timeout_from_degraded_requests_safe(void)
{
    uint32_t successful_cycles = 0u;
    StateRequest_t request;

    watchdog_evaluate_cycle(0u, SYSTEM_STATE_DEGRADED, &successful_cycles);

    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xStateRequestQueue, &request, 0));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_SAFE, request.requested_state);
    TEST_ASSERT_EQUAL_UINT32(0x01u, request.reason_code);
}

void test_watchdog_complete_cycle_in_degraded_increments_recovery_counter(void)
{
    uint32_t successful_cycles = 0u;

    watchdog_evaluate_cycle(WATCHDOG_EXPECTED_BITS, SYSTEM_STATE_DEGRADED, &successful_cycles);

    TEST_ASSERT_EQUAL_UINT32(1u, successful_cycles);
    TEST_ASSERT_TRUE(test_state_request_queue_empty());
}

void test_watchdog_fifth_complete_cycle_requests_nominal_recovery(void)
{
    uint32_t successful_cycles = 0u;
    StateRequest_t request;

    for (uint32_t i = 0; i < 4u; i++) {
        watchdog_evaluate_cycle(WATCHDOG_EXPECTED_BITS, SYSTEM_STATE_DEGRADED, &successful_cycles);
        TEST_ASSERT_EQUAL_UINT32(i + 1u, successful_cycles);
        TEST_ASSERT_TRUE(test_state_request_queue_empty());
    }

    watchdog_evaluate_cycle(WATCHDOG_EXPECTED_BITS, SYSTEM_STATE_DEGRADED, &successful_cycles);

    TEST_ASSERT_EQUAL_UINT32(0u, successful_cycles);
    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xStateRequestQueue, &request, 0));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_NOMINAL, request.requested_state);
    TEST_ASSERT_EQUAL_UINT32(0x03u, request.reason_code);
}

void test_watchdog_complete_cycle_from_safe_resets_recovery_counter(void)
{
    uint32_t successful_cycles = 2u;

    watchdog_evaluate_cycle(WATCHDOG_EXPECTED_BITS, SYSTEM_STATE_SAFE, &successful_cycles);

    TEST_ASSERT_EQUAL_UINT32(0u, successful_cycles);
    TEST_ASSERT_TRUE(test_state_request_queue_empty());
}

void test_watchdog_kick_notifies_watchdog_task(void)
{
    xWatchdogTaskHandle = (TaskHandle_t) (uintptr_t) 0x42u;
    test_task_notify_reset();

    watchdog_kick(WATCHDOG_BIT_HEARTBEAT);

    TEST_ASSERT_EQUAL_UINT32(WATCHDOG_BIT_HEARTBEAT, test_task_notify_get_bits());
}