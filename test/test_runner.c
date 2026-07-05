#include "unity.h"

#include "fault_inject.h"
#include "fault_queue.h"
#include "memory_protection.h"
#include "memory_scrub.h"
#include "state_machine.h"
#include "system_state.h"
#include "queue.h"

static void test_exec_target(void)
{
}

void setUp(void)
{
    test_freertos_reset();
    system_state_init();
    fault_queue_init();
    memory_protection_init();
    fault_inject_set_enabled(true);
}

void tearDown(void)
{
    test_freertos_reset();
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

void test_memory_protection_allows_exec_on_read_exec_region(void)
{
    memory_protection_protect_region((uintptr_t) test_exec_target,
                                     (uintptr_t) test_exec_target + 1u,
                                     MEM_PERM_READ | MEM_PERM_EXEC,
                                     "CriticalText");

    TEST_ASSERT_TRUE(memory_protection_check_access((uintptr_t) test_exec_target, 1, MEM_PERM_EXEC));
    TEST_ASSERT_TRUE(memory_protection_check_access((uintptr_t) test_exec_target, 1, MEM_PERM_READ));
    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) test_exec_target, 1, MEM_PERM_WRITE));
    TEST_ASSERT_EQUAL_UINT32(1, memory_protection_get_violation_count());
}

void test_memory_protection_violation_threshold_requests_degraded(void)
{
    uint8_t buffer[16];
    StateRequest_t request;

    memory_protection_protect_region((uintptr_t) buffer, (uintptr_t) buffer + sizeof(buffer),
                                     MEM_PERM_READ, "TestReadOnly");

    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_WRITE));
    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_WRITE));
    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_WRITE));
    TEST_ASSERT_EQUAL_UINT32(3, memory_protection_get_violation_count());

    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xStateRequestQueue, &request, 0));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_DEGRADED, request.requested_state);
    TEST_ASSERT_EQUAL_UINT32(0x04u, request.reason_code);
}

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

void test_fault_queue_init_succeeds(void)
{
    test_freertos_reset();
    xFaultQueue = NULL;

    TEST_ASSERT_TRUE(fault_queue_init());
    TEST_ASSERT_NOT_NULL(xFaultQueue);
}

void test_fault_queue_send_receive_roundtrip(void)
{
    FaultEvent_t sent = {.index = 7u, .bit = 2u};
    FaultEvent_t received;

    TEST_ASSERT_EQUAL(pdPASS, xQueueSend(xFaultQueue, &sent, 0));
    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xFaultQueue, &received, 0));
    TEST_ASSERT_EQUAL_UINT32(sent.index, received.index);
    TEST_ASSERT_EQUAL_UINT8(sent.bit, received.bit);
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

void test_memory_protection_golden_copy_readonly_after_registration(void)
{
    memory_protection_protect_region((uintptr_t) golden_copy,
                                     (uintptr_t) golden_copy + SCRUB_SIZE,
                                     MEM_PERM_READ,
                                     "GoldenCopy");

    TEST_ASSERT_EQUAL_UINT8(0, golden_copy[0]);
    TEST_ASSERT_EQUAL_UINT8(255, golden_copy[255]);
    TEST_ASSERT_EQUAL_UINT8(0, golden_copy[256]);

    TEST_ASSERT_TRUE(memory_protection_check_access((uintptr_t) &golden_copy[0], 1, MEM_PERM_READ));
    TEST_ASSERT_TRUE(memory_protection_check_access((uintptr_t) &golden_copy[255], 1, MEM_PERM_READ));
    TEST_ASSERT_FALSE(memory_protection_check_access((uintptr_t) &golden_copy[0], 1, MEM_PERM_WRITE));
    TEST_ASSERT_EQUAL_UINT32(1, memory_protection_get_violation_count());
    TEST_ASSERT_EQUAL_UINT8(1, memory_protection_get_region_count());
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
    RUN_TEST(test_memory_protection_allows_exec_on_read_exec_region);
    RUN_TEST(test_memory_protection_violation_threshold_requests_degraded);
    RUN_TEST(test_memory_protection_golden_copy_readonly_after_registration);

    RUN_TEST(test_system_state_init_sets_boot_and_creates_queue);
    RUN_TEST(test_system_state_request_change_enqueues_state_and_reason);
    RUN_TEST(test_system_state_get_returns_current_state);
    RUN_TEST(test_system_state_request_change_fails_when_queue_uninitialized);

    RUN_TEST(test_memory_scrub_init_copies_golden_pattern);
    RUN_TEST(test_memory_scrub_fix_event_corrects_single_bit_flip);
    RUN_TEST(test_memory_scrub_fix_event_noop_when_bit_already_matches);
    RUN_TEST(test_memory_scrub_fix_event_rejects_invalid_index);
    RUN_TEST(test_memory_scrub_fix_event_rejects_invalid_bit);
    RUN_TEST(test_memory_scrub_fix_event_rejects_null_event);
    RUN_TEST(test_memory_scrub_full_scrub_corrects_multiple_bytes);
    RUN_TEST(test_memory_scrub_seu_threshold_requests_degraded);

    RUN_TEST(test_fault_inject_enabled_by_default);
    RUN_TEST(test_fault_inject_set_enabled_toggles);
    RUN_TEST(test_fault_inject_corrupts_buffer_and_queues_event);

    RUN_TEST(test_state_machine_boot_to_nominal_allowed);
    RUN_TEST(test_state_machine_nominal_to_degraded_allowed);
    RUN_TEST(test_state_machine_degraded_to_nominal_allowed_for_watchdog_recovery);
    RUN_TEST(test_state_machine_degraded_to_nominal_allowed_for_uart_command);
    RUN_TEST(test_state_machine_degraded_to_safe_allowed);
    RUN_TEST(test_state_machine_downgrade_rejected_without_recovery_reason);
    RUN_TEST(test_state_machine_same_state_request_is_noop);

    RUN_TEST(test_fault_queue_init_succeeds);
    RUN_TEST(test_fault_queue_send_receive_roundtrip);

    return UNITY_END();
}