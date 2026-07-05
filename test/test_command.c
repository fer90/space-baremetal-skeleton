#include "unity.h"

#include "command.h"
#include "fault_queue.h"
#include "log.h"
#include "memory_protection.h"
#include "memory_scrub.h"
#include "system_state.h"
#include "queue.h"
#include "uart_test.h"

static bool command_queue_empty(void)
{
    CommandType_t cmd;

    return xQueueReceive(xCommandQueue, &cmd, 0) != pdPASS;
}

static bool command_queue_receive(CommandType_t *cmd)
{
    return xQueueReceive(xCommandQueue, cmd, 0) == pdPASS;
}

void test_command_dispatch_help_prints_command_list(void)
{
    TEST_ASSERT_TRUE(command_dispatch_char('h'));
    TEST_ASSERT_TRUE(test_uart_contains(LOG_PREFIX_CMD "Commands:"));
    TEST_ASSERT_TRUE(test_uart_contains("n  request NOMINAL state"));
}

void test_command_dispatch_help_accepts_question_mark(void)
{
    TEST_ASSERT_TRUE(command_dispatch_char('?'));
    TEST_ASSERT_TRUE(test_uart_contains(LOG_PREFIX_CMD "Commands:"));
}

void test_command_dispatch_nominal_enqueues_command(void)
{
    CommandType_t cmd;

    TEST_ASSERT_TRUE(command_dispatch_char('n'));
    TEST_ASSERT_TRUE(command_queue_receive(&cmd));
    TEST_ASSERT_EQUAL(CMD_GO_NOMINAL, cmd);
    TEST_ASSERT_TRUE(command_queue_empty());
}

void test_command_dispatch_inject_fault_enqueues_command(void)
{
    CommandType_t cmd;

    TEST_ASSERT_TRUE(command_dispatch_char('f'));
    TEST_ASSERT_TRUE(command_queue_receive(&cmd));
    TEST_ASSERT_EQUAL(CMD_INJECT_FAULT, cmd);
}

void test_command_dispatch_unknown_key_returns_false(void)
{
    TEST_ASSERT_FALSE(command_dispatch_char('q'));
    TEST_ASSERT_TRUE(command_queue_empty());
}

void test_command_dispatch_queue_full_prints_message(void)
{
    CommandType_t cmd;
    uint8_t i;

    for (i = 0; i < 8u; i++) {
        TEST_ASSERT_EQUAL(pdPASS, command_send(CMD_STATUS));
    }

    test_uart_reset();
    TEST_ASSERT_TRUE(command_dispatch_char('v'));
    TEST_ASSERT_TRUE(test_uart_contains(LOG_PREFIX_CMD "queue full"));

    for (i = 0; i < 8u; i++) {
        TEST_ASSERT_TRUE(command_queue_receive(&cmd));
        TEST_ASSERT_EQUAL(CMD_STATUS, cmd);
    }

    TEST_ASSERT_FALSE(command_queue_receive(&cmd));
}

void test_command_handle_go_nominal_enqueues_state_request(void)
{
    StateRequest_t request;

    command_handle(CMD_GO_NOMINAL);

    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xStateRequestQueue, &request, 0));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_NOMINAL, request.requested_state);
    TEST_ASSERT_EQUAL_UINT32(0x10u, request.reason_code);
}

void test_command_handle_go_degraded_enqueues_state_request(void)
{
    StateRequest_t request;

    command_handle(CMD_GO_DEGRADED);

    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xStateRequestQueue, &request, 0));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_DEGRADED, request.requested_state);
    TEST_ASSERT_EQUAL_UINT32(0x10u, request.reason_code);
}

void test_command_handle_print_violations_reports_count(void)
{
    uint8_t buffer[16];

    memory_protection_protect_region((uintptr_t) buffer, (uintptr_t) buffer + sizeof(buffer),
                                     MEM_PERM_READ, "TestReadOnly");
    (void) memory_protection_check_access((uintptr_t) buffer, 1, MEM_PERM_WRITE);

    test_uart_reset();
    command_handle(CMD_PRINT_VIOLATIONS);

    TEST_ASSERT_TRUE(test_uart_contains(LOG_PREFIX_CMD "mem_prot violations: "));
    TEST_ASSERT_TRUE(test_uart_contains("1"));
}

void test_command_handle_print_seu_count_reports_total(void)
{
    volatile uint8_t area[SCRUB_SIZE];
    FaultEvent_t event = {.index = 2u, .bit = 0u};

    memory_scrub_init(area);
    area[2] ^= 0x01u;
    (void) memory_scrub_fix_event(area, &event);

    test_uart_reset();
    command_handle(CMD_PRINT_SEU_COUNT);

    TEST_ASSERT_TRUE(test_uart_contains(LOG_PREFIX_CMD "SEU count: "));
    TEST_ASSERT_TRUE(test_uart_contains("1"));
}

void test_command_handle_toggle_fault_inject_prints_state(void)
{
    test_uart_reset();
    command_handle(CMD_TOGGLE_FAULT_INJECT);
    TEST_ASSERT_TRUE(test_uart_contains(LOG_PREFIX_CMD "fault injection: OFF"));

    test_uart_reset();
    command_handle(CMD_TOGGLE_FAULT_INJECT);
    TEST_ASSERT_TRUE(test_uart_contains(LOG_PREFIX_CMD "fault injection: ON"));
}

void test_command_handle_status_requires_debug_build(void)
{
    test_uart_reset();
    command_handle(CMD_STATUS);
    TEST_ASSERT_TRUE(test_uart_contains(LOG_PREFIX_CMD "telemetry requires DEBUG build"));
}

void test_command_handle_inject_fault_queues_scrub_event(void)
{
    FaultEvent_t event;

    test_freertos_set_tick_count(100u);
    memory_scrub_init(scrub_area);

    command_handle(CMD_INJECT_FAULT);

    TEST_ASSERT_EQUAL(pdPASS, xQueueReceive(xFaultQueue, &event, 0));
    TEST_ASSERT_EQUAL_UINT32(100u, event.index);
    TEST_ASSERT_EQUAL_UINT8(0u, event.bit);
}