#include "interrupt.h"
#include "log.h"
#include "uart.h"
#include "timer.h"
#include <stdint.h>

// Machine-mode trap handler
// This is the single entry point for ALL traps (interrupts + exceptions).
// It reads mcause to determine what happened and dispatches accordingly.
void trap_handler(void) __attribute__((interrupt("machine")));

void trap_handler(void) {
    uint64_t mcause;
    asm volatile("csrr %0, mcause" : "=r"(mcause));

    // MSB set = interrupt, MSB clear = exception
    if (mcause & (1ULL << 63)) {
        // Interrupt
        uint64_t cause = mcause & 0x3FF;
        if (cause == 7) {
            // Machine Timer Interrupt (MTI)
            timer_isr();
        } else {
            uart_puts(LOG_PREFIX_ERROR "unhandled interrupt\r\n");
        }
    } else {
        // Synchronous exception — halt
        uart_puts(LOG_PREFIX_ERROR "exception — system halted\r\n");
        while (1) {
            asm volatile("wfi");
        }
    }
}

void interrupt_init(void) {
    // Set mtvec to direct mode, pointing to our trap handler
    asm volatile("csrw mtvec, %0" : : "r"((uintptr_t)trap_handler));

    // Enable Machine Timer Interrupt (MTIE = bit 7)
    asm volatile("csrs mie, %0" : : "r"(1 << 7));

    // Enable global interrupts (MIE = bit 3 in mstatus)
    asm volatile("csrs mstatus, %0" : : "r"(1 << 3));

    uart_puts("Interrupt system initialized (Machine mode)\r\n");
}
