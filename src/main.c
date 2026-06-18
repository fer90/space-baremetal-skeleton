#include "FreeRTOS.h"
#include "task.h"
#include "common.h"

extern void freertos_risc_v_trap_handler( void );

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    uart_puts("!!! STACK OVERFLOW in task: ");
    uart_puts(pcTaskName);
    uart_puts(" !!!\r\n");
    for( ;; );
}

void vTaskHeartbeat(void *pvParameters)
{
    for( ;; )
    {
        uart_puts("HEARTBEAT from FreeRTOS task\r\n");
        vTaskDelay( pdMS_TO_TICKS(1000) );   // 1 second
    }
}

int main(void)
{
    uart_puts("\r\n=== FreeRTOS Migration Started ===\r\n");

    // Create first task
    xTaskCreate(
        vTaskHeartbeat,          // Task function
        "Heartbeat",             // Task name
        configMINIMAL_STACK_SIZE * 2, // Stack size
        NULL,                    // Parameters
        1,                       // Priority
        NULL                     // Task handle
    );

    uart_puts("Starting FreeRTOS scheduler...\r\n");

    /* FreeRTOS owns all machine-mode traps from here on. */
    __asm volatile ( "csrw mtvec, %0" : : "r" ( freertos_risc_v_trap_handler ) );

    // Start the scheduler
    vTaskStartScheduler();

    // Should never reach here
    for( ;; );
}