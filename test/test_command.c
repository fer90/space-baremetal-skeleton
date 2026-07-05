#include "unity.h"

#include "command.h"
#include "log.h"
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