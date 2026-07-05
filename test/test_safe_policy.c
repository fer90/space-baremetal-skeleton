#include "unity.h"

#include "safe_policy.h"
#include "system_defs.h"

void test_safe_policy_watchdog_expected_bits_nominal(void)
{
    TEST_ASSERT_EQUAL_UINT32(WATCHDOG_EXPECTED_BITS,
                             safe_policy_watchdog_expected_bits(SYSTEM_STATE_NOMINAL));
}

void test_safe_policy_watchdog_expected_bits_safe_is_minimal(void)
{
    TEST_ASSERT_EQUAL_UINT32(WATCHDOG_BIT_HEARTBEAT | WATCHDOG_BIT_MEMSCRUB,
                             safe_policy_watchdog_expected_bits(SYSTEM_STATE_SAFE));
}

void test_safe_policy_blocks_manual_fault_inject_in_safe(void)
{
    TEST_ASSERT_FALSE(safe_policy_allows_manual_fault_inject(SYSTEM_STATE_SAFE));
    TEST_ASSERT_TRUE(safe_policy_allows_manual_fault_inject(SYSTEM_STATE_DEGRADED));
}

void test_safe_policy_blocks_auto_fault_inject_outside_nominal(void)
{
    TEST_ASSERT_TRUE(safe_policy_allows_auto_fault_inject(SYSTEM_STATE_NOMINAL));
    TEST_ASSERT_FALSE(safe_policy_allows_auto_fault_inject(SYSTEM_STATE_DEGRADED));
    TEST_ASSERT_FALSE(safe_policy_allows_auto_fault_inject(SYSTEM_STATE_SAFE));
}

void test_safe_policy_blocks_background_scrub_in_safe(void)
{
    TEST_ASSERT_FALSE(safe_policy_allows_background_scrub(SYSTEM_STATE_SAFE));
    TEST_ASSERT_TRUE(safe_policy_allows_background_scrub(SYSTEM_STATE_NOMINAL));
}

void test_safe_policy_suppresses_heartbeat_uart_in_safe(void)
{
    TEST_ASSERT_FALSE(safe_policy_allows_heartbeat_uart(SYSTEM_STATE_SAFE));
    TEST_ASSERT_TRUE(safe_policy_allows_heartbeat_uart(SYSTEM_STATE_NOMINAL));
}

void test_safe_policy_blocks_seu_escalation_in_safe(void)
{
    TEST_ASSERT_FALSE(safe_policy_allows_seu_escalation(SYSTEM_STATE_SAFE));
    TEST_ASSERT_TRUE(safe_policy_allows_seu_escalation(SYSTEM_STATE_DEGRADED));
}