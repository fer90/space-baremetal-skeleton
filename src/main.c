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
    TaskHandle_t xIsrGuardHandle = NULL;
    TaskHandle_t xStackHwmHandle = NULL;

    const configSTACK_DEPTH_TYPE watchdog_stack = configMINIMAL_STACK_SIZE * 2;
    const configSTACK_DEPTH_TYPE heartbeat_stack = configMINIMAL_STACK_SIZE * 2;
    const configSTACK_DEPTH_TYPE memscrub_stack = configMINIMAL_STACK_SIZE * 3;
    const configSTACK_DEPTH_TYPE isrguard_stack = configMINIMAL_STACK_SIZE * 2;
    const configSTACK_DEPTH_TYPE faultinject_stack = configMINIMAL_STACK_SIZE * 2;
    const configSTACK_DEPTH_TYPE stack_hwm_stack = configMINIMAL_STACK_SIZE * 2;
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
#ifdef DEBUG
    xTaskCreate(vTaskIsrStackGuard, "IsrGuard", isrguard_stack, NULL, 2, &xIsrGuardHandle);
#endif
    xTaskCreate(vTaskFaultInject, "FaultInject", faultinject_stack, NULL, 1, &xFaultInjectHandle);

#ifdef DEBUG
    stack_hwm_register(xWatchdogHandle, "Watchdog", watchdog_stack);
    stack_hwm_register(xHeartbeatHandle, "Heartbeat", heartbeat_stack);
    stack_hwm_register(xMemScrubHandle, "MemScrub", memscrub_stack);
    stack_hwm_register(xIsrGuardHandle, "IsrGuard", isrguard_stack);
    stack_hwm_register(xFaultInjectHandle, "FaultInject", faultinject_stack);
    xTaskCreate(vTaskStackHwmMonitor, "StackHwm", stack_hwm_stack, NULL, 1, &xStackHwmHandle);
    stack_hwm_register(xStackHwmHandle, "StackHwm", stack_hwm_stack);
#endif

    uart_puts("Starting FreeRTOS scheduler...\r\n");

    /* FreeRTOS owns all machine-mode traps from here on. */
    __asm volatile ( "csrw mtvec, %0" : : "r" ( freertos_risc_v_trap_handler ) );

    // Start the scheduler
    vTaskStartScheduler();

    // Should never reach here
    for( ;; );
}
