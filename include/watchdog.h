#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>

#include "system_state.h"

#define WATCHDOG_RECOVERY_THRESHOLD 5u

void watchdog_kick(uint32_t taskBit);
void watchdog_evaluate_cycle(uint32_t received_bits,
                             SystemState_t current_state,
                             uint32_t *successful_cycles);
void vTaskWatchdog(void *pvParameters);

#endif /* WATCHDOG_H */