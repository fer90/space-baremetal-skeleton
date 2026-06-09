#include "common.h"
#include "timer.h"
#include "uart.h"

// QEMU virt machine RISC-V timer (CLINT)
#define CLINT_BASE 0x2000000UL
#define CLINT_MTIMECMP (CLINT_BASE + 0x4000)
#define CLINT_MTIME (CLINT_BASE + 0xBFF8)

volatile uint32_t system_ticks = 0;

static void timer_set_next(void) {
    uint64_t now = *(volatile uint64_t *)CLINT_MTIME;
    *(volatile uint64_t *)CLINT_MTIMECMP = now + 100000;  // ~1ms tick (adjust)
}

void timer_init(void) {
    timer_set_next();
    uart_puts("Timer initialized (system ticks)\r\n");
}

uint32_t get_system_ticks(void) {
    return system_ticks;
}

void timer_tick(void) {
    system_ticks++;
    timer_set_next();
}
