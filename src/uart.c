#include <stdint.h>
#include "uart.h"

#define UART_BASE 0x10000000UL

void uart_putc(char c) {
    volatile uint8_t *uart = (uint8_t *)UART_BASE;
    // Simple 16550-like TX ready poll
    while ((*(uart + 5) & 0x20) == 0);
    *uart = c;
}

void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}