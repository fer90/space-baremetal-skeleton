#include "uart.h"

void system_halt(const char *message) {
    uart_puts("!!! FATAL: ");
    uart_puts(message);
    uart_puts(" !!!\r\n");
    for (;;) {
    }
}