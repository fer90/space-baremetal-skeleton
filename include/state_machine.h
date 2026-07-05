#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdbool.h>

#include "system_state.h"

bool state_machine_apply_request(SystemState_t *current, const StateRequest_t *request);
void vTaskStateMachine(void *pvParameters);

#endif