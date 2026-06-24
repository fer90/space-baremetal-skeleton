#ifndef FAULT_INJECT_H
#define FAULT_INJECT_H

#include <stdbool.h>
#include <stdint.h>

void fault_inject_init(void);
void inject_random_fault(volatile uint8_t *area);
bool fault_inject_is_enabled(void);
bool fault_inject_set_enabled(bool enabled);
void vTaskFaultInject(void *pvParameters);

#endif /* FAULT_INJECT_H */
