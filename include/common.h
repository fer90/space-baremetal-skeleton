#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

// Shared minimal state
extern volatile uint32_t system_ticks;

// Watchdog
void watchdog_init(void);
void watchdog_kick(void);
void watchdog_check(void);

// Memory scrub
#define SCRUB_SIZE 512
void memory_scrub_init(volatile uint8_t *area);
void memory_scrub(volatile uint8_t *area);

// Fault injection
void fault_inject_init(void);
void inject_random_fault(volatile uint8_t *area);

#endif