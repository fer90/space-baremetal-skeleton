#include "unity.h"

#include "crc32.h"

void test_crc32_empty_is_zero(void)
{
    TEST_ASSERT_EQUAL_UINT32(0u, crc32_compute(NULL, 0u));
}

void test_crc32_known_ieee_vector(void)
{
    static const uint8_t payload[] = "123456789";

    TEST_ASSERT_EQUAL_UINT32(0xCBF43926u, crc32_compute(payload, sizeof(payload) - 1u));
}