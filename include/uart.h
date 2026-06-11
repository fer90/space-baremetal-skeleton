#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>

void uart_putc(char c);
void uart_puts(const char *s);

void uart_put_hex(uint32_t value);
void uart_put_hex8(uint8_t value);
void uart_hex_dump(const uint8_t *data, size_t len);

#endif