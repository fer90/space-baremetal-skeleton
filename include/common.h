#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "system_defs.h"
#include "uart.h"
#include "memory_scrub.h"
#ifdef DEBUG
#include "isr_stack_guard.h"
#include "telemetry.h"
#endif
#include "fault_queue.h"
#include "fault_inject.h"
#include "watchdog.h"
#include "state_machine.h"
#include "system_state.h"

#endif /* COMMON_H */
