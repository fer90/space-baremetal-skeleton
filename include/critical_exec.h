#ifndef CRITICAL_EXEC_H
#define CRITICAL_EXEC_H

#include <stdint.h>

#define CRITICAL_TEXT __attribute__((section(".text.critical"), noinline))

extern uint8_t __critical_text_start[];
extern uint8_t __critical_text_end[];

void memory_protection_register_critical_text(void);

#endif /* CRITICAL_EXEC_H */