#ifndef SAFE_POLICY_H
#define SAFE_POLICY_H

#include <stdbool.h>
#include <stdint.h>

#include "system_state.h"

uint32_t safe_policy_watchdog_expected_bits(SystemState_t state);

bool safe_policy_allows_fault_inject(SystemState_t state);
bool safe_policy_allows_background_scrub(SystemState_t state);
bool safe_policy_allows_heartbeat_uart(SystemState_t state);
bool safe_policy_allows_seu_escalation(SystemState_t state);

void safe_policy_on_enter(void);
void safe_policy_on_exit(void);

#endif /* SAFE_POLICY_H */