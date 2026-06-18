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
	watchdog_kick(WATCHDOG_BIT_HEARTBEAT);
        vTaskDelay( pdMS_TO_TICKS(1000) );   // 1 second
    }
}

int main(void)
{
    uart_puts("\r\n=== FreeRTOS Migration Started ===\r\n");

    // High priority watchdog task
    xTaskCreate(
        vTaskWatchdog,
	"Watchdog",
	configMINIMAL_STACK_SIZE * 2,
	NULL,
	4,
	NULL
    );

    // Create Heartbeat
    xTaskCreate(
        vTaskHeartbeat,          // Task function
        "Heartbeat",             // Task name
        configMINIMAL_STACK_SIZE * 2, // Stack size
        NULL,                    // Parameters
        1,                       // Priority
        NULL                     // Task handle
    );

    // Create Memory Scrub Task
    xTaskCreate(
        vTaskMemoryScrub,
        "MemScrub",
        configMINIMAL_STACK_SIZE * 3,   // Slightly bigger stack
        NULL,
        2,                    // Medium priority
        NULL
    );

    xTaskCreate(
        vTaskFaultInject,
        "FaultInject",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        1,
        NULL	
    );

    uart_puts("Starting FreeRTOS scheduler...\r\n");

    /* FreeRTOS owns all machine-mode traps from here on. */
    __asm volatile ( "csrw mtvec, %0" : : "r" ( freertos_risc_v_trap_handler ) );

    // Start the scheduler
    vTaskStartScheduler();

    // Should never reach here
    for( ;; );
}
