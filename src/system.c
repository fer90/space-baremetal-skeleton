#include "uart.h"
#include "log.h"

void system_halt(const char *reason)
{
    uart_puts("\r\n");
    uart_puts(LOG_PREFIX_ERROR "System halted: ");
    uart_puts(reason);
    uart_puts("\r\n");
    for (;;) {
    }
}