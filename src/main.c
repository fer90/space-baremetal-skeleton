#include "FreeRTOS.h"
#include "task.h"
#include "common.h"

extern void freertos_risc_v_trap_handler( void );

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    (void) xTask;
    uart_puts("!!! STACK OVERFLOW in task: ");
    uart_puts(pcTaskName);
    uart_puts(" !!!\r\n");
    for( ;; );
}

void vTaskHeartbeat(void *pvParameters)
{
    (void) pvParameters;

    for( ;; )
    {
        uart_puts("HEARTBEAT from FreeRTOS task\r\n");
        watchdog_kick(WATCHDOG_BIT_HEARTBEAT);
        vTaskDelay( pdMS_TO_TICKS(1000) );   // 1 second
    }
}

int main(void)
{
    TaskHandle_t xWatchdogHandle = NULL;
    TaskHandle_t xHeartbeatHandle = NULL;
    TaskHandle_t xMemScrubHandle = NULL;
    TaskHandle_t xFaultInjectHandle = NULL;
#ifdef DEBUG
    TaskHandle_t xTelemetryHandle = NULL;

    const configSTACK_DEPTH_TYPE watchdog_stack = configMINIMAL_STACK_SIZE * 2;
    const configSTACK_DEPTH_TYPE heartbeat_stack = configMINIMAL_STACK_SIZE * 2;
    const configSTACK_DEPTH_TYPE memscrub_stack = configMINIMAL_STACK_SIZE * 3;
    const configSTACK_DEPTH_TYPE faultinject_stack = configMINIMAL_STACK_SIZE * 2;
    const configSTACK_DEPTH_TYPE telemetry_stack = configMINIMAL_STACK_SIZE * 2;
#else
    const configSTACK_DEPTH_TYPE watchdog_stack = configMINIMAL_STACK_SIZE * 2;
    const configSTACK_DEPTH_TYPE heartbeat_stack = configMINIMAL_STACK_SIZE * 2;
    const configSTACK_DEPTH_TYPE memscrub_stack = configMINIMAL_STACK_SIZE * 3;
    const configSTACK_DEPTH_TYPE faultinject_stack = configMINIMAL_STACK_SIZE * 2;
#endif

    uart_puts("\r\n=== FreeRTOS Migration Started ===\r\n");

    xTaskCreate(vTaskWatchdog, "Watchdog", watchdog_stack, NULL, 4, &xWatchdogHandle);
    xTaskCreate(vTaskHeartbeat, "Heartbeat", heartbeat_stack, NULL, 1, &xHeartbeatHandle);
    xTaskCreate(vTaskMemoryScrub, "MemScrub", memscrub_stack, NULL, 2, &xMemScrubHandle);
    xTaskCreate(vTaskFaultInject, "FaultInject", faultinject_stack, NULL, 1, &xFaultInjectHandle);

#ifdef DEBUG
    isr_stack_guard_init();
    telemetry_register_task(xWatchdogHandle, "Watchdog", watchdog_stack);
    telemetry_register_task(xHeartbeatHandle, "Heartbeat", heartbeat_stack);
    telemetry_register_task(xMemScrubHandle, "MemScrub", memscrub_stack);
    telemetry_register_task(xFaultInjectHandle, "FaultInject", faultinject_stack);
    xTaskCreate(vTaskTelemetry, "Telemetry", telemetry_stack, NULL, 1, &xTelemetryHandle);
    telemetry_register_task(xTelemetryHandle, "Telemetry", telemetry_stack);
#endif

    uart_puts("Starting FreeRTOS scheduler...\r\n");

    /* FreeRTOS owns all machine-mode traps from here on. */
    __asm volatile ( "csrw mtvec, %0" : : "r" ( freertos_risc_v_trap_handler ) );

    // Start the scheduler
    vTaskStartScheduler();

    // Should never reach here
    for( ;; );
}
