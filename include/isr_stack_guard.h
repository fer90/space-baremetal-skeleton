#ifndef ISR_STACK_GUARD_H
#define ISR_STACK_GUARD_H

#ifdef DEBUG

void isr_stack_guard_init(void);
void isr_stack_guard_check(void);
void isr_stack_guard_print_usage(void);
void vTaskIsrStackGuard(void *pvParameters);

#endif /* DEBUG */

#endif /* ISR_STACK_GUARD_H */