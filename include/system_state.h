#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include "FreeRTOS.h"
#include "queue.h"

typedef enum {
    SYSTEM_STATE_BOOT = 0,
    SYSTEM_STATE_NOMINAL,
    SYSTEM_STATE_DEGRADED,
    SYSTEM_STATE_SAFE,
} SystemState_t;

typedef struct {
    SystemState_t requested_state;
    uint32_t reason_code;
} StateRequest_t;

extern volatile SystemState_t gSystemState;
extern QueueHandle_t xStateRequestQueue;

void system_state_init(void);
SystemState_t system_state_get(void);

// Called by other tasks to request a state change
BaseType_t system_state_request_change(SystemState_t new_state, uint32_t reason);

#endif
