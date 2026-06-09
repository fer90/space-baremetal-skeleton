#include "common.h"
#include "uart.h"

#define STACK_PAINT_PATTERN 0xDEADBEEF
#define GUARD_SIZE 512

// These symbols come from startup.s / linker
extern uint8_t stack_low[];
extern uint8_t stack_top[];

static uint32_t high_water_mark = 0;

void stack_paint_init(void) {
    // Paint the entire stack area (except a small guard at top)
    volatile uint32_t *p = (volatile uint32_t *)stack_low;
    volatile uint32_t *end = (volatile uint32_t *)((uintptr_t)stack_low + GUARD_SIZE);

    while (p < end) {
        *p++ = STACK_PAINT_PATTERN;
    }
    uart_puts("Stack guard region painted with 0xDEADBEEF pattern\r\n");
}

void stack_check(void) {
    volatile uint32_t *p = (volatile uint32_t *)stack_low;
    volatile uint32_t *guard_end = (volatile uint32_t *)((uintptr_t)stack_low + GUARD_SIZE);
    uint32_t corrupted = 0;

    while (p < guard_end) {
        if (*p != STACK_PAINT_PATTERN) {
            corrupted++;
        }
        // Track high water mark (how deep we went)
        if (*p != STACK_PAINT_PATTERN) {
            uint32_t usage = (uintptr_t)guard_end - (uintptr_t)p;
            if (usage > high_water_mark) high_water_mark = usage;
        }
        p++;
    }

    if (corrupted > 0) {
        uart_puts("!!! STACK CORRUPTION DETECTED (possible overflow) !!!\r\n");
        // In real system: trigger FDIR / safe mode
    } else {
        // uart_puts("Stack check OK\r\n");
    }
}

void stack_print_usage(void) {
    uart_puts("Stack high water mark: ");
    // TODO: proper number printing
    uart_puts("~");
    uart_puts((high_water_mark > 0) ? "used" : "low");
    uart_puts(" bytes\r\n");
}
