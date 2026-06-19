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

void uart_put_dec(uint32_t value) {
    char buf[11];
    int i = 10;

    buf[i] = '\0';
    if (value == 0) {
        uart_putc('0');
        return;
    }

    while (value > 0) {
        buf[--i] = (char) ('0' + (value % 10));
        value /= 10;
    }

    uart_puts(&buf[i]);
}

static const char hex_chars[] = "0123456789ABCDEF";

void uart_put_hex(uint32_t value) {
    // Print 8 hex digits (32-bit value)
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (value >> (i * 4)) & 0xF;
        uart_putc(hex_chars[nibble]);
    }
}

void uart_put_hex8(uint8_t value) {
    uart_putc(hex_chars[(value >> 4) & 0xF]);
    uart_putc(hex_chars[value & 0xF]);
}

void uart_hex_dump(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uart_put_hex8(data[i]);
        uart_putc(' ');

        // New line every 16 bytes for readability
        if ((i + 1) % 16 == 0) {
            uart_puts("\r\n");
        }
    }
    uart_puts("\r\n");
}
