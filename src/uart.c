#include <stdint.h>
#include "uart.h"

#define UART_BASE     0x10000000UL
#define UART_LSR_THRE 0x20u
#define UART_LSR_DR   0x01u

static volatile uint8_t *uart_regs(void) {
    return (volatile uint8_t *) UART_BASE;
}

void uart_putc(char c) {
    volatile uint8_t *uart = uart_regs();

    while ((uart[5] & UART_LSR_THRE) == 0) {
    }
    uart[0] = (uint8_t) c;
}

bool uart_rx_ready(void) {
    return (uart_regs()[5] & UART_LSR_DR) != 0;
}

char uart_getc(void) {
    volatile uint8_t *uart = uart_regs();

    while ((uart[5] & UART_LSR_DR) == 0) {
    }
    return (char) uart[0];
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
