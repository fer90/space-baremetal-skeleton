#include "common.h"

static volatile bool fault_inject_enabled = true;

void fault_inject_init(void) {
    uart_puts("Fault injection initialized (SEU simulation)\r\n");
}

bool fault_inject_is_enabled(void)
{
    return fault_inject_enabled;
}

bool fault_inject_set_enabled(bool enabled)
{
    fault_inject_enabled = enabled;
    return fault_inject_enabled;
}

void inject_random_fault(volatile uint8_t *area) {
    TickType_t ticks = xTaskGetTickCount();
    uint32_t r = (uint32_t) ticks;

    int idx = r % SCRUB_SIZE;
    int bit = (r >> 8) % 8;

    area[idx] ^= (1u << bit);
    uart_puts("Simulated fault injected @ tick ");
    uart_put_hex((uint32_t) ticks);
    uart_puts(" — byte ");
    uart_put_hex(idx);
    uart_puts(" (bit ");
    uart_put_hex(bit);
    uart_puts(") !!!\r\n");

    uart_puts("Before scrub dump (first 64 bytes):\r\n");
    uart_hex_dump((const uint8_t *) area, 64);

    if (xFaultQueue != NULL) {
        FaultEvent_t event = {
            .index = (uint32_t) idx,
            .bit = (uint8_t) bit,
        };
        (void) xQueueSend(xFaultQueue, &event, 0);
    }
}

void vTaskFaultInject(void *pvParameters) {
    (void) pvParameters;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(3000));
        watchdog_kick(WATCHDOG_BIT_FAULTINJECT);
        if (fault_inject_enabled) {
            inject_random_fault(scrub_area);
        }
    }
}