#ifndef SYSTEM_DEFS_H
#define SYSTEM_DEFS_H

#include <stdint.h>
#include "FreeRTOS.h"

/* --------------------------------------------------------------------------
 * Fault injection event (FaultInject -> MemScrub via xFaultQueue)
 * -------------------------------------------------------------------------- */
typedef struct {
    uint32_t index;
    uint8_t  bit;
} FaultEvent_t;

/* --------------------------------------------------------------------------
 * Watchdog task-notification bits
 * -------------------------------------------------------------------------- */
#define WATCHDOG_BIT_HEARTBEAT     (1u << 0)
#define WATCHDOG_BIT_MEMSCRUB      (1u << 1)
#define WATCHDOG_BIT_FAULTINJECT   (1u << 2)
#define WATCHDOG_EXPECTED_BITS \
    (WATCHDOG_BIT_HEARTBEAT | WATCHDOG_BIT_MEMSCRUB | WATCHDOG_BIT_FAULTINJECT)

/* Must exceed the slowest monitored kick period (FaultInject = 3 s). */
#define WATCHDOG_TIMEOUT_MS        3500u

/* --------------------------------------------------------------------------
 * Task priorities (configMAX_PRIORITIES = 5, valid range 0..4)
 *
 * Watchdog (4): highest app priority so it can detect stalled peers even
 *               when lower tasks are ready; it mostly blocks on notify.
 * MemScrub (2): above heartbeat/fault-inject so targeted SEU fixes run
 *               promptly after a fault event is queued.
 * Heartbeat / FaultInject / Telemetry (1): periodic work; share time slice.
 * Idle (0):     kernel idle task.
 * -------------------------------------------------------------------------- */
#define TASK_PRIO_WATCHDOG         4
#define TASK_PRIO_MEMSCRUB         2
#define TASK_PRIO_HEARTBEAT        1
#define TASK_PRIO_FAULTINJECT      1
#define TASK_PRIO_TELEMETRY        1

/* --------------------------------------------------------------------------
 * Task stack depths (FreeRTOS words; ×8 bytes on rv64)
 * -------------------------------------------------------------------------- */
#define TASK_STACK_WATCHDOG        (configMINIMAL_STACK_SIZE * 2)
#define TASK_STACK_HEARTBEAT       (configMINIMAL_STACK_SIZE * 2)
#define TASK_STACK_MEMSCRUB        (configMINIMAL_STACK_SIZE * 3)
#define TASK_STACK_FAULTINJECT     (configMINIMAL_STACK_SIZE * 2)
#define TASK_STACK_TELEMETRY       (configMINIMAL_STACK_SIZE * 2)

#endif /* SYSTEM_DEFS_H */