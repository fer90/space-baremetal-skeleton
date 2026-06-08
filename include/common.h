#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

// Watchdog
void watchdog_init(void);
void watchdog_kick(void);
void watchdog_check(void);

// Memory scrub
void memory_scrub_init(void);
void memory_scrub(void);

#endif