#ifndef FAULT_INJECT_H
#define FAULT_INJECT_H

#include <stdint.h>

void fault_inject_init(void);
void inject_random_fault(volatile uint8_t *area);
void vTaskFaultInject(void *pvParameters);

#endif /* FAULT_INJECT_H */
