#include "FreeRTOS.h"
#include "task.h"
#include "tasks.h"
#include "system.h"
#include "uart.h"
#include "string.h"
#include "system_defs.h"
#include "watchdog.h"
#include "state_machine.h"
#include "memory_scrub.h"
#include "fault_inject.h"
#include "command.h"
#include "safe_policy.h"
#include "system_state.h"
#ifdef DEBUG
#include "isr_stack_guard.h"
#include "telemetry.h"
#endif

typedef struct {
    const char *name;
    TaskFunction_t function;
    configSTACK_DEPTH_TYPE stack_size;
    UBaseType_t priority;
} task_config_t;

static void vTaskHeartbeat(void *pvParameters)
{
    (void) pvParameters;

    for (;;) {
        if (safe_policy_allows_heartbeat_uart(system_state_get())) {
            uart_puts("HEARTBEAT from FreeRTOS task\r\n");
        }
        watchdog_kick(WATCHDOG_BIT_HEARTBEAT);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#ifdef DEBUG
static bool task_config_reports_telemetry(const task_config_t *config)
{
    return strcmp(config->name, "StateMachine") != 0 &&
           strcmp(config->name, "CommandInput") != 0 &&
           strcmp(config->name, "CommandHandler") != 0;
}
#endif

static void create_task(const task_config_t *config)
{
    TaskHandle_t handle = NULL;

    if (xTaskCreate(config->function, config->name, config->stack_size, NULL,
                    config->priority, &handle) != pdPASS) {
        system_halt("xTaskCreate failed");
    }

#ifdef DEBUG
    if (task_config_reports_telemetry(config)) {
        telemetry_register_task(handle, config->name, config->stack_size);
    }
#endif
}

void tasks_create_all(void)
{
    static const task_config_t task_configs[] = {
        { "Watchdog",       vTaskWatchdog,       TASK_STACK_WATCHDOG,       TASK_PRIO_WATCHDOG       },
        { "StateMachine",   vTaskStateMachine,   TASK_STACK_STATEMACHINE,   TASK_PRIO_STATEMACHINE   },
        { "Heartbeat",      vTaskHeartbeat,      TASK_STACK_HEARTBEAT,      TASK_PRIO_HEARTBEAT      },
        { "MemScrub",       vTaskMemoryScrub,    TASK_STACK_MEMSCRUB,       TASK_PRIO_MEMSCRUB       },
        { "FaultInject",    vTaskFaultInject,    TASK_STACK_FAULTINJECT,    TASK_PRIO_FAULTINJECT    },
        { "CommandInput",   vTaskCommandInput,   TASK_STACK_COMMANDINPUT,   TASK_PRIO_COMMANDINPUT   },
        { "CommandHandler", vTaskCommandHandler, TASK_STACK_COMMANDHANDLER, TASK_PRIO_COMMANDHANDLER },
#ifdef DEBUG
        { "Telemetry",      vTaskTelemetry,      TASK_STACK_TELEMETRY,      TASK_PRIO_TELEMETRY      },
#endif
    };

#ifdef DEBUG
    isr_stack_guard_init();
#endif

    fault_inject_init();

    for (size_t i = 0; i < (sizeof(task_configs) / sizeof(task_configs[0])); i++) {
        create_task(&task_configs[i]);
    }

#ifdef DEBUG
    telemetry_register_kernel_tasks();
#endif
}