#include "unity.h"

#include "memory_scrub.h"
#include "system_state.h"
#include "queue.h"

void test_memory_scrub_init_copies_golden_pattern(void)
{
    volatile uint8_t area[SCRUB_SIZE];

    memory_scrub_init(area);

    for (uint32_t i = 0; i < SCRUB_SIZE; i++) {
        TEST_ASSERT_EQUAL_UINT8(golden_copy[i], area[i]);
    }
}

void test_memory_scrub_fix_event_corrects_single_bit_flip(void)
{
    volatile uint8_t area[SCRUB_SIZE];
    FaultEvent_t event = {.index = 10u, .bit = 3u};

    memory_scrub_init(area);
    area[10] ^= (uint8_t) (1u << 3u);

    TEST_ASSERT_TRUE(memory_scrub_fix_event(area, &event));
    TEST_ASSERT_EQUAL_UINT8(golden_copy[10], area[10]);
    TEST_ASSERT_EQUAL_UINT32(1u, memory_scrub_get_seu_count());
}

void test_memory_scrub_fix_event_noop_when_bit_already_matches(void)
{
    volatile uint8_t area[SCRUB_SIZE];
    FaultEvent_t event = {.index = 4u, .bit = 1u};

    memory_scrub_init(area);

    TEST_ASSERT_FALSE(memory_scrub_fix_event(area, &event));
    TEST_ASSERT_EQUAL_UINT32(0u, memory_scrub_get_seu_count());
}

void test_memory_scrub_fix_event_rejects_invalid_index(void)
{
    volatile uint8_t area[SCRUB_SIZE];
    FaultEvent_t event = {.index = SCRUB_SIZE, .bit = 0u};

    memory_scrub_init(area);
    TEST_ASSERT_FALSE(memory_scrub_fix_event(area, &event));
}

void test_memory_scrub_fix_event_rejects_invalid_bit(void)
{
    volatile uint8_t area[SCRUB_SIZE];
    FaultEvent_t event = {.index = 0u, .bit = 8u};

    memory_scrub_init(area);
    TEST_ASSERT_FALSE(memory_scrub_fix_event(area, &event));
}

void test_memory_scrub_fix_event_rejects_null_event(void)
{
    volatile uint8_t area[SCRUB_SIZE];

    memory_scrub_init(area);
    TEST_ASSERT_FALSE(memory_scrub_fix_event(area, NULL));
}

void test_memory_scrub_full_scrub_corrects_multiple_bytes(void)
{
    volatile uint8_t area[SCRUB_SIZE];

    memory_scrub_init(area);
    area[1] ^= 0x01u;
    area[2] ^= 0x02u;
    area[3] ^= 0x04u;

    memory_scrub(area);

    TEST_ASSERT_EQUAL_UINT8(golden_copy[1], area[1]);
    TEST_ASSERT_EQUAL_UINT8(golden_copy[2], area[2]);
    TEST_ASSERT_EQUAL_UINT8(golden_copy[3], area[3]);
    TEST_ASSERT_EQUAL_UINT32(3u, memory_scrub_get_seu_count());
}

void test_memory_scrub_seu_threshold_requests_degraded(void)
{
    volatile uint8_t area[SCRUB_SIZE];
    StateRequest_t request;

    memory_scrub_init(area);

    for (uint8_t i = 0; i < 5u; i++) {
        FaultEvent_t event = {.index = (uint32_t) (20u + i), .bit = 0u};

        area[20u + i] ^= 0x01u;
        TEST_ASSERT_TRUE(memory_scrub_fix_event(area, &event));
    }

    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xStateRequestQueue, &request, 0));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_DEGRADED, request.requested_state);
    TEST_ASSERT_EQUAL_UINT32(0x02u, request.reason_code);
    TEST_ASSERT_EQUAL_UINT32(5u, memory_scrub_get_seu_count());
}