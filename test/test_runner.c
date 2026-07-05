#include "unity.h"

#include "system_state.h"
#include "memory_protection.h"
#include "queue.h"

void setUp(void)
{
    test_freertos_reset();
    memory_protection_init();
}

void tearDown(void)
{
    test_freertos_reset();
}

void test_system_state_initial_state(void)
{
    system_state_init();
    TEST_ASSERT_EQUAL(SYSTEM_STATE_BOOT, system_state_get());
}

void test_system_state_request_change_enqueues(void)
{
    system_state_init();
    TEST_ASSERT_EQUAL(pdPASS, system_state_request_change(SYSTEM_STATE_DEGRADED, 0x10));
    TEST_ASSERT_NOT_NULL(xStateRequestQueue);
}

void test_memory_protection_unprotected_access_allowed(void)
{
    uint8_t buffer[16];

    TEST_ASSERT_TRUE(memory_protection_check_access((uintptr_t) buffer, sizeof(buffer),
                                                    MEM_PERM_READ | MEM_PERM_WRITE));
    TEST_ASSERT_EQUAL_UINT32(0, memory_protection_get_violation_count());
}

void test_memory_protection_blocks_write_on_readonly_region(void)
{
    uint8_t buffer[16];

    memory_protection_protect_region((uintptr_t) buffer, (uintptr_t) buffer + sizeof(buffer),
                                     MEM_PERM_READ, "TestReadOnly");

    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_WRITE));
    TEST_ASSERT_EQUAL_UINT32(1, memory_protection_get_violation_count());
    TEST_ASSERT_EQUAL_UINT8(1, memory_protection_get_region_count());
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_system_state_initial_state);
    RUN_TEST(test_system_state_request_change_enqueues);
    RUN_TEST(test_memory_protection_unprotected_access_allowed);
    RUN_TEST(test_memory_protection_blocks_write_on_readonly_region);

    return UNITY_END();
}