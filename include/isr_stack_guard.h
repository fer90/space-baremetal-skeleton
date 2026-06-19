#ifndef ISR_STACK_GUARD_H
#define ISR_STACK_GUARD_H

#ifdef DEBUG

#include <stdbool.h>
#include <stdint.h>

void isr_stack_guard_init(void);
bool isr_stack_guard_check(void);
uint32_t isr_stack_guard_get_hwm_bytes(void);

#endif /* DEBUG */

#endif /* ISR_STACK_GUARD_H */