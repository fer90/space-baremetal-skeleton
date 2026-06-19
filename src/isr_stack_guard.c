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

bool isr_stack_guard_check(void) {
    volatile uint32_t *p = (volatile uint32_t *)stack_low;
    volatile uint32_t *guard_end = (volatile uint32_t *)((uintptr_t)stack_low + ISR_STACK_GUARD_SIZE);
    bool corrupt = false;

    while (p < guard_end) {
        if (*p != ISR_STACK_GUARD_PATTERN) {
            corrupt = true;
            uint32_t usage = (uint32_t) ((uintptr_t)guard_end - (uintptr_t)p);
            if (usage > isr_stack_high_water_mark) {
                isr_stack_high_water_mark = usage;
            }
        }
        p++;
    }

    return corrupt;
}

uint32_t isr_stack_guard_get_hwm_bytes(void) {
    return isr_stack_high_water_mark;
}

#endif /* DEBUG */