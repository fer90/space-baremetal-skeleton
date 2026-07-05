#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "uart.h"

void uart_putc(char c)
{
    (void) c;
}

bool uart_rx_ready(void)
{
    return false;
}

char uart_getc(void)
{
    return '\0';
}

void uart_puts(const char *s)
{
    (void) s;
}

void uart_put_dec(uint32_t value)
{
    (void) value;
}

void uart_put_hex(uint32_t value)
{
    (void) value;
}

void uart_put_hex8(uint8_t value)
{
    (void) value;
}

void uart_hex_dump(const uint8_t *data, size_t len)
{
    (void) data;
    (void) len;
}