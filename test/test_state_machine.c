#include "unity.h"

#include "state_machine.h"

void test_state_machine_boot_to_nominal_allowed(void)
{
    SystemState_t current = SYSTEM_STATE_BOOT;
    StateRequest_t request = {.requested_state = SYSTEM_STATE_NOMINAL, .reason_code = 0u};

    TEST_ASSERT_TRUE(state_machine_apply_request(&current, &request));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_NOMINAL, current);
}

void test_state_machine_nominal_to_degraded_allowed(void)
{
    SystemState_t current = SYSTEM_STATE_NOMINAL;
    StateRequest_t request = {.requested_state = SYSTEM_STATE_DEGRADED, .reason_code = 0x01u};

    TEST_ASSERT_TRUE(state_machine_apply_request(&current, &request));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_DEGRADED, current);
}

void test_state_machine_degraded_to_nominal_allowed_for_watchdog_recovery(void)
{
    SystemState_t current = SYSTEM_STATE_DEGRADED;
    StateRequest_t request = {.requested_state = SYSTEM_STATE_NOMINAL, .reason_code = 0x03u};

    TEST_ASSERT_TRUE(state_machine_apply_request(&current, &request));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_NOMINAL, current);
}

void test_state_machine_degraded_to_nominal_allowed_for_uart_command(void)
{
    SystemState_t current = SYSTEM_STATE_DEGRADED;
    StateRequest_t request = {.requested_state = SYSTEM_STATE_NOMINAL, .reason_code = 0x10u};

    TEST_ASSERT_TRUE(state_machine_apply_request(&current, &request));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_NOMINAL, current);
}

void test_state_machine_degraded_to_safe_allowed(void)
{
    SystemState_t current = SYSTEM_STATE_DEGRADED;
    StateRequest_t request = {.requested_state = SYSTEM_STATE_SAFE, .reason_code = 0x01u};

    TEST_ASSERT_TRUE(state_machine_apply_request(&current, &request));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_SAFE, current);
}

void test_state_machine_downgrade_rejected_without_recovery_reason(void)
{
    SystemState_t current = SYSTEM_STATE_SAFE;
    StateRequest_t request = {.requested_state = SYSTEM_STATE_NOMINAL, .reason_code = 0x01u};

    TEST_ASSERT_FALSE(state_machine_apply_request(&current, &request));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_SAFE, current);
}

void test_state_machine_same_state_request_is_noop(void)
{
    SystemState_t current = SYSTEM_STATE_DEGRADED;
    StateRequest_t request = {.requested_state = SYSTEM_STATE_DEGRADED, .reason_code = 0x02u};

    TEST_ASSERT_FALSE(state_machine_apply_request(&current, &request));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_DEGRADED, current);
}