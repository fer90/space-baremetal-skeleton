#include "unity.h"

#include "memory_protection.h"

void setUp(void)
{
    memory_protection_init();
}

void tearDown(void)
{
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

void test_memory_protection_allows_read_on_readonly_region(void)
{
    uint8_t buffer[16];

    memory_protection_protect_region((uintptr_t) buffer, (uintptr_t) buffer + sizeof(buffer),
                                     MEM_PERM_READ, "TestReadOnly");

    TEST_ASSERT_TRUE(memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_READ));
    TEST_ASSERT_EQUAL_UINT32(0, memory_protection_get_violation_count());
}

void test_memory_protection_blocks_read_on_exec_only_region(void)
{
    uint8_t buffer[16];

    memory_protection_protect_region((uintptr_t) buffer, (uintptr_t) buffer + sizeof(buffer),
                                     MEM_PERM_EXEC, "TestExecOnly");

    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_READ));
    TEST_ASSERT_EQUAL_UINT32(1, memory_protection_get_violation_count());
}

void test_memory_protection_blocks_exec_on_readwrite_region(void)
{
    uint8_t buffer[16];

    memory_protection_protect_region((uintptr_t) buffer, (uintptr_t) buffer + sizeof(buffer),
                                     MEM_PERM_READ | MEM_PERM_WRITE, "TestReadWrite");

    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_EXEC));
    TEST_ASSERT_EQUAL_UINT32(1, memory_protection_get_violation_count());
}

void test_memory_protection_mem_perm_none_always_passes(void)
{
    uint8_t buffer[16];

    memory_protection_protect_region((uintptr_t) buffer, (uintptr_t) buffer + sizeof(buffer),
                                     MEM_PERM_READ, "TestReadOnly");

    TEST_ASSERT_TRUE(memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_NONE));
    TEST_ASSERT_EQUAL_UINT32(0, memory_protection_get_violation_count());
}

void test_memory_protection_violation_count_accumulates(void)
{
    uint8_t buffer[16];

    memory_protection_protect_region((uintptr_t) buffer, (uintptr_t) buffer + sizeof(buffer),
                                     MEM_PERM_READ, "TestReadOnly");

    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_WRITE));
    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_WRITE));
    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_WRITE));
    TEST_ASSERT_EQUAL_UINT32(3, memory_protection_get_violation_count());
}

void test_memory_protection_region_table_full(void)
{
    for (uint8_t i = 0; i < 8; i++) {
        uintptr_t start = 0x1000u + ((uintptr_t) i * 0x100u);
        memory_protection_protect_region(start, start + 0x10u, MEM_PERM_READ, "Region");
    }

    TEST_ASSERT_EQUAL_UINT8(8, memory_protection_get_region_count());

    memory_protection_protect_region(0x9000u, 0x9010u, MEM_PERM_READ, "Overflow");

    TEST_ASSERT_EQUAL_UINT8(8, memory_protection_get_region_count());
}

void test_memory_protection_partial_overlap_detected(void)
{
    uint8_t buffer[16];

    memory_protection_protect_region((uintptr_t) &buffer[4], (uintptr_t) &buffer[12],
                                     MEM_PERM_READ, "Partial");

    TEST_ASSERT_TRUE(memory_protection_check_access((uintptr_t) &buffer[2], 2, MEM_PERM_WRITE));
    TEST_ASSERT_EQUAL_UINT32(0, memory_protection_get_violation_count());

    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) &buffer[11], 2, MEM_PERM_WRITE));
    TEST_ASSERT_EQUAL_UINT32(1, memory_protection_get_violation_count());
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_memory_protection_unprotected_access_allowed);
    RUN_TEST(test_memory_protection_blocks_write_on_readonly_region);
    RUN_TEST(test_memory_protection_allows_read_on_readonly_region);
    RUN_TEST(test_memory_protection_blocks_read_on_exec_only_region);
    RUN_TEST(test_memory_protection_blocks_exec_on_readwrite_region);
    RUN_TEST(test_memory_protection_mem_perm_none_always_passes);
    RUN_TEST(test_memory_protection_violation_count_accumulates);
    RUN_TEST(test_memory_protection_region_table_full);
    RUN_TEST(test_memory_protection_partial_overlap_detected);

    return UNITY_END();
}