#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "uart.h"
#include "uart_test.h"

#define TEST_UART_CAP_SIZE 4096u

static char test_uart_buffer[TEST_UART_CAP_SIZE];
static size_t test_uart_length;

static void test_uart_append_char(char c)
{
    if (test_uart_length + 1u < TEST_UART_CAP_SIZE) {
        test_uart_buffer[test_uart_length++] = c;
        test_uart_buffer[test_uart_length] = '\0';
    }
}

static void test_uart_append_string(const char *s)
{
    while (s != NULL && *s != '\0') {
        test_uart_append_char(*s++);
    }
}

void test_uart_reset(void)
{
    test_uart_length = 0;
    test_uart_buffer[0] = '\0';
}

const char *test_uart_output(void)
{
    return test_uart_buffer;
}

bool test_uart_contains(const char *substring)
{
    if (substring == NULL) {
        return false;
    }

    return strstr(test_uart_buffer, substring) != NULL;
}

void uart_putc(char c)
{
    test_uart_append_char(c);
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
    test_uart_append_string(s);
}

void uart_put_dec(uint32_t value)
{
    char buffer[16];

    (void) snprintf(buffer, sizeof(buffer), "%lu", (unsigned long) value);
    test_uart_append_string(buffer);
}

void uart_put_hex(uint32_t value)
{
    char buffer[16];

    (void) snprintf(buffer, sizeof(buffer), "%08lX", (unsigned long) value);
    test_uart_append_string(buffer);
}

void uart_put_hex8(uint8_t value)
{
    char buffer[8];

    (void) snprintf(buffer, sizeof(buffer), "%02X", (unsigned int) value);
    test_uart_append_string(buffer);
}

void uart_hex_dump(const uint8_t *data, size_t len)
{
    if (data == NULL) {
        return;
    }

    for (size_t i = 0; i < len; i++) {
        uart_put_hex8(data[i]);
        uart_putc(' ');
        if ((i % 16u) == 15u) {
            uart_putc('\n');
        }
    }
}