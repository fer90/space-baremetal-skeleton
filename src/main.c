#include "FreeRTOS.h"
#include "task.h"
#include "common.h"
#include "log.h"
#include "system.h"
#include "tasks.h"

extern void freertos_risc_v_trap_handler(void);

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void) xTask;
    uart_puts(LOG_PREFIX_ERROR "STACK OVERFLOW in task: ");
    uart_puts(pcTaskName);
    uart_puts("\r\n");
    for (;;) {
    }
}

int main(void)
{
    if (!fault_queue_init()) {
        system_halt("fault_queue_init failed");
    }

    command_init();
    memory_protection_init();
    memory_protection_register_critical_text();
    uart_puts("\r\n=== FreeRTOS Migration Started ===\r\n");

    tasks_create_all();

    uart_puts("Starting FreeRTOS scheduler...\r\n");

    /* FreeRTOS owns all machine-mode traps from here on. */
    __asm volatile("csrw mtvec, %0" : : "r"(freertos_risc_v_trap_handler));

    vTaskStartScheduler();

    system_halt("scheduler returned");
}