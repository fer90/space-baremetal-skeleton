#include "unity.h"

#include "event_log.h"
#include "log.h"
#include "system_state.h"
#include "uart_test.h"
#include "queue.h"

void test_event_log_init_records_boot_entry(void)
{
    event_log_init();

    TEST_ASSERT_EQUAL_UINT16(1u, event_log_count());

    EventLogEntry_t entry;
    TEST_ASSERT_TRUE(event_log_get_entry_chronological(0u, &entry));
    TEST_ASSERT_EQUAL(EVENT_LOG_BOOT, entry.type);
}

void test_event_log_state_change_roundtrip(void)
{
    event_log_init();
    test_freertos_set_tick_count(1000u);

    event_log_record_state_change(SYSTEM_STATE_NOMINAL,
                                  SYSTEM_STATE_DEGRADED,
                                  0x10u);

    TEST_ASSERT_EQUAL_UINT16(2u, event_log_count());

    EventLogEntry_t entry;
    TEST_ASSERT_TRUE(event_log_get_entry_chronological(1u, &entry));
    TEST_ASSERT_EQUAL(EVENT_LOG_STATE_CHANGE, entry.type);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_NOMINAL, entry.arg_a);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_DEGRADED, entry.arg_b);
    TEST_ASSERT_EQUAL_UINT32(0x10u, entry.data);
    TEST_ASSERT_EQUAL_UINT32(1000u, entry.tick);
}

void test_event_log_ring_overwrites_oldest(void)
{
    uint16_t index;

    event_log_reset();

    for (index = 0; index < EVENT_LOG_CAPACITY + 3u; index++) {
        event_log_record_operator((char) ('0' + (index % 10u)));
    }

    TEST_ASSERT_EQUAL_UINT16(EVENT_LOG_CAPACITY, event_log_count());

    EventLogEntry_t first;
    EventLogEntry_t last;

    TEST_ASSERT_TRUE(event_log_get_entry_chronological(0u, &first));
    TEST_ASSERT_TRUE(event_log_get_entry_chronological(EVENT_LOG_CAPACITY - 1u, &last));
    TEST_ASSERT_EQUAL(EVENT_LOG_OPERATOR, first.type);
    TEST_ASSERT_EQUAL('3', first.arg_a);
    TEST_ASSERT_EQUAL(EVENT_LOG_OPERATOR, last.type);
    TEST_ASSERT_EQUAL('0', last.arg_a);
}

void test_event_log_dump_prints_header_and_entries(void)
{
    event_log_init();
    event_log_record_state_change(SYSTEM_STATE_DEGRADED,
                                  SYSTEM_STATE_SAFE,
                                  0x01u);

    event_log_dump();

    TEST_ASSERT_TRUE(test_uart_contains(LOG_PREFIX_REC "flight log ("));
    TEST_ASSERT_TRUE(test_uart_contains(LOG_PREFIX_REC "t="));
    TEST_ASSERT_TRUE(test_uart_contains("STATE "));
    TEST_ASSERT_TRUE(test_uart_contains("DEGRADED->SAFE"));
}