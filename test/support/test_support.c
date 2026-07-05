#include "test_support.h"

#include "command.h"
#include "fault_inject.h"
#include "fault_queue.h"
#include "memory_protection.h"
#include "queue.h"
#include "system_state.h"
#include "uart_test.h"

void setUp(void)
{
    test_freertos_reset();
    test_uart_reset();
    system_state_init();
    fault_queue_init();
    command_init();
    memory_protection_init();
    fault_inject_set_enabled(true);
}

void tearDown(void)
{
    test_freertos_reset();
    test_uart_reset();
}

bool test_state_request_queue_empty(void)
{
    StateRequest_t request;

    return xQueueReceive(xStateRequestQueue, &request, 0) != pdPASS;
}