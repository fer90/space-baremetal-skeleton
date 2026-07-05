#include "unity.h"

void test_memory_protection_unprotected_access_allowed(void);
void test_memory_protection_blocks_write_on_readonly_region(void);
void test_memory_protection_allows_read_on_readonly_region(void);
void test_memory_protection_blocks_read_on_exec_only_region(void);
void test_memory_protection_blocks_exec_on_readwrite_region(void);
void test_memory_protection_mem_perm_none_always_passes(void);
void test_memory_protection_violation_count_accumulates(void);
void test_memory_protection_region_table_full(void);
void test_memory_protection_partial_overlap_detected(void);
void test_memory_protection_allows_exec_on_read_exec_region(void);
void test_memory_protection_violation_threshold_requests_degraded(void);
void test_memory_protection_golden_copy_readonly_after_registration(void);

void test_system_state_init_sets_boot_and_creates_queue(void);
void test_system_state_request_change_enqueues_state_and_reason(void);
void test_system_state_get_returns_current_state(void);
void test_system_state_request_change_fails_when_queue_uninitialized(void);

void test_memory_scrub_init_copies_golden_pattern(void);
void test_memory_scrub_fix_event_corrects_single_bit_flip(void);
void test_memory_scrub_fix_event_noop_when_bit_already_matches(void);
void test_memory_scrub_fix_event_rejects_invalid_index(void);
void test_memory_scrub_fix_event_rejects_invalid_bit(void);
void test_memory_scrub_fix_event_rejects_null_event(void);
void test_memory_scrub_full_scrub_corrects_multiple_bytes(void);
void test_memory_scrub_seu_threshold_requests_degraded(void);

void test_fault_inject_enabled_by_default(void);
void test_fault_inject_set_enabled_toggles(void);
void test_fault_inject_corrupts_buffer_and_queues_event(void);

void test_state_machine_boot_to_nominal_allowed(void);
void test_state_machine_nominal_to_degraded_allowed(void);
void test_state_machine_degraded_to_nominal_allowed_for_watchdog_recovery(void);
void test_state_machine_degraded_to_nominal_allowed_for_uart_command(void);
void test_state_machine_degraded_to_safe_allowed(void);
void test_state_machine_downgrade_rejected_without_recovery_reason(void);
void test_state_machine_same_state_request_is_noop(void);

void test_fault_queue_init_succeeds(void);
void test_fault_queue_send_receive_roundtrip(void);

void test_watchdog_complete_cycle_from_nominal_resets_recovery_counter(void);
void test_watchdog_timeout_from_nominal_requests_degraded(void);
void test_watchdog_timeout_from_degraded_requests_safe(void);
void test_watchdog_complete_cycle_in_degraded_increments_recovery_counter(void);
void test_watchdog_fifth_complete_cycle_requests_nominal_recovery(void);
void test_watchdog_complete_cycle_from_safe_resets_recovery_counter(void);
void test_watchdog_timeout_from_safe_does_not_request_state_change(void);
void test_watchdog_kick_notifies_watchdog_task(void);

void test_command_dispatch_help_prints_command_list(void);
void test_command_dispatch_help_accepts_question_mark(void);
void test_command_dispatch_nominal_enqueues_command(void);
void test_command_dispatch_inject_fault_enqueues_command(void);
void test_command_dispatch_unknown_key_returns_false(void);
void test_command_dispatch_queue_full_prints_message(void);

void test_command_handle_go_nominal_enqueues_state_request(void);
void test_command_handle_go_degraded_enqueues_state_request(void);
void test_command_handle_print_violations_reports_count(void);
void test_command_handle_print_seu_count_reports_total(void);
void test_command_handle_toggle_fault_inject_prints_state(void);
void test_command_handle_status_requires_debug_build(void);
void test_command_handle_inject_fault_queues_scrub_event(void);

void test_safe_policy_watchdog_expected_bits_nominal(void);
void test_safe_policy_watchdog_expected_bits_safe_is_minimal(void);
void test_safe_policy_blocks_fault_inject_in_safe(void);
void test_safe_policy_blocks_background_scrub_in_safe(void);
void test_safe_policy_suppresses_heartbeat_uart_in_safe(void);
void test_safe_policy_blocks_seu_escalation_in_safe(void);

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

    RUN_TEST(test_watchdog_complete_cycle_from_nominal_resets_recovery_counter);
    RUN_TEST(test_watchdog_timeout_from_nominal_requests_degraded);
    RUN_TEST(test_watchdog_timeout_from_degraded_requests_safe);
    RUN_TEST(test_watchdog_complete_cycle_in_degraded_increments_recovery_counter);
    RUN_TEST(test_watchdog_fifth_complete_cycle_requests_nominal_recovery);
    RUN_TEST(test_watchdog_complete_cycle_from_safe_resets_recovery_counter);
    RUN_TEST(test_watchdog_timeout_from_safe_does_not_request_state_change);
    RUN_TEST(test_watchdog_kick_notifies_watchdog_task);

    RUN_TEST(test_command_dispatch_help_prints_command_list);
    RUN_TEST(test_command_dispatch_help_accepts_question_mark);
    RUN_TEST(test_command_dispatch_nominal_enqueues_command);
    RUN_TEST(test_command_dispatch_inject_fault_enqueues_command);
    RUN_TEST(test_command_dispatch_unknown_key_returns_false);
    RUN_TEST(test_command_dispatch_queue_full_prints_message);

    RUN_TEST(test_command_handle_go_nominal_enqueues_state_request);
    RUN_TEST(test_command_handle_go_degraded_enqueues_state_request);
    RUN_TEST(test_command_handle_print_violations_reports_count);
    RUN_TEST(test_command_handle_print_seu_count_reports_total);
    RUN_TEST(test_command_handle_toggle_fault_inject_prints_state);
    RUN_TEST(test_command_handle_status_requires_debug_build);
    RUN_TEST(test_command_handle_inject_fault_queues_scrub_event);

    RUN_TEST(test_safe_policy_watchdog_expected_bits_nominal);
    RUN_TEST(test_safe_policy_watchdog_expected_bits_safe_is_minimal);
    RUN_TEST(test_safe_policy_blocks_fault_inject_in_safe);
    RUN_TEST(test_safe_policy_blocks_background_scrub_in_safe);
    RUN_TEST(test_safe_policy_suppresses_heartbeat_uart_in_safe);
    RUN_TEST(test_safe_policy_blocks_seu_escalation_in_safe);

    return UNITY_END();
}