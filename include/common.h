#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "memory_scrub.h"
#ifdef DEBUG
#include "isr_stack_guard.h"
#endif
#include "fault_inject.h"
#include "watchdog.h"

#endif /* COMMON_H */
