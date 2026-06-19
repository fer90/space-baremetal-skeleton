#include "FreeRTOS.h"
#include "task.h"
#include "common.h"
#include "system.h"

extern void freertos_risc_v_trap_handler(void);

static void create_app_task(TaskFunction_t fn,
                            const char *name,
                            configSTACK_DEPTH_TYPE stack_depth,
                            UBaseType_t priority,
                            TaskHandle_t *handle) {
    if (xTaskCreate(fn, name, stack_depth, NULL, priority, handle) != pdPASS) {
        system_halt("xTaskCreate failed");
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void) xTask;
    uart_puts("!!! STACK OVERFLOW in task: ");
    uart_puts(pcTaskName);
    uart_puts(" !!!\r\n");
    for (;;) {
    }
}

void vTaskHeartbeat(void *pvParameters) {
    (void) pvParameters;

    for (;;) {
        uart_puts("HEARTBEAT from FreeRTOS task\r\n");
        watchdog_kick(WATCHDOG_BIT_HEARTBEAT);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void) {
    TaskHandle_t xWatchdogHandle = NULL;
    TaskHandle_t xHeartbeatHandle = NULL;
    TaskHandle_t xMemScrubHandle = NULL;
    TaskHandle_t xFaultInjectHandle = NULL;
#ifdef DEBUG
    TaskHandle_t xTelemetryHandle = NULL;
#endif

    if (!fault_queue_init()) {
        system_halt("fault_queue_init failed");
    }

    uart_puts("\r\n=== FreeRTOS Migration Started ===\r\n");

    create_app_task(vTaskWatchdog, "Watchdog", TASK_STACK_WATCHDOG,
                    TASK_PRIO_WATCHDOG, &xWatchdogHandle);
    create_app_task(vTaskHeartbeat, "Heartbeat", TASK_STACK_HEARTBEAT,
                    TASK_PRIO_HEARTBEAT, &xHeartbeatHandle);
    create_app_task(vTaskMemoryScrub, "MemScrub", TASK_STACK_MEMSCRUB,
                    TASK_PRIO_MEMSCRUB, &xMemScrubHandle);
    create_app_task(vTaskFaultInject, "FaultInject", TASK_STACK_FAULTINJECT,
                    TASK_PRIO_FAULTINJECT, &xFaultInjectHandle);

#ifdef DEBUG
    isr_stack_guard_init();
    telemetry_register_task(xWatchdogHandle, "Watchdog", TASK_STACK_WATCHDOG);
    telemetry_register_task(xHeartbeatHandle, "Heartbeat", TASK_STACK_HEARTBEAT);
    telemetry_register_task(xMemScrubHandle, "MemScrub", TASK_STACK_MEMSCRUB);
    telemetry_register_task(xFaultInjectHandle, "FaultInject", TASK_STACK_FAULTINJECT);
    create_app_task(vTaskTelemetry, "Telemetry", TASK_STACK_TELEMETRY,
                    TASK_PRIO_TELEMETRY, &xTelemetryHandle);
    telemetry_register_task(xTelemetryHandle, "Telemetry", TASK_STACK_TELEMETRY);
#endif

    uart_puts("Starting FreeRTOS scheduler...\r\n");

    /* FreeRTOS owns all machine-mode traps from here on. */
    __asm volatile("csrw mtvec, %0" : : "r"(freertos_risc_v_trap_handler));

    vTaskStartScheduler();

    system_halt("scheduler returned");
}