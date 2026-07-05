#ifndef UART_TEST_H
#define UART_TEST_H

#include <stdbool.h>

void test_uart_reset(void);
const char *test_uart_output(void);
bool test_uart_contains(const char *substring);

#endif /* UART_TEST_H */