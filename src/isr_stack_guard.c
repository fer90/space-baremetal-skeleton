#ifdef DEBUG

#include "common.h"
#include "isr_stack_guard.h"

#define ISR_STACK_GUARD_PATTERN 0xDEADBEEF
#define ISR_STACK_GUARD_SIZE    512

/* Repurposed by FreeRTOS as __freertos_irq_stack_top (see startup.s). */
extern uint8_t stack_low[];
extern uint8_t stack_top[];

static uint32_t isr_stack_high_water_mark = 0;

void isr_stack_guard_init(void) {
    volatile uint32_t *p = (volatile uint32_t *)stack_low;
    volatile uint32_t *end = (volatile uint32_t *)((uintptr_t)stack_low + ISR_STACK_GUARD_SIZE);

    while (p < end) {
        *p++ = ISR_STACK_GUARD_PATTERN;
    }
    uart_puts("ISR stack guard painted (0xDEADBEEF at stack_low)\r\n");
}

void isr_stack_guard_check(void) {
    volatile uint32_t *p = (volatile uint32_t *)stack_low;
    volatile uint32_t *guard_end = (volatile uint32_t *)((uintptr_t)stack_low + ISR_STACK_GUARD_SIZE);
    uint32_t corrupted = 0;

    while (p < guard_end) {
        if (*p != ISR_STACK_GUARD_PATTERN) {
            corrupted++;
            uint32_t usage = (uintptr_t)guard_end - (uintptr_t)p;
            if (usage > isr_stack_high_water_mark) {
                isr_stack_high_water_mark = usage;
            }
        }
        p++;
    }

    if (corrupted > 0) {
        uart_puts("!!! ISR STACK CORRUPTION DETECTED (possible overflow) !!!\r\n");
    }
}

void isr_stack_guard_print_usage(void) {
    uart_puts("ISR stack guard high water mark: ");
    uart_puts((isr_stack_high_water_mark > 0) ? "used" : "low");
    uart_puts("\r\n");
}

void vTaskIsrStackGuard(void *pvParameters) {
    (void) pvParameters;

    isr_stack_guard_init();

    for (;;) {
        isr_stack_guard_check();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

#endif /* DEBUG */