#ifndef MEMORY_SCRUB_H
#define MEMORY_SCRUB_H

#include <stdint.h>

#define SCRUB_SIZE 512

void memory_scrub_init(volatile uint8_t *area);
void memory_scrub(volatile uint8_t *area);

#endif /* MEMORY_SCRUB_H */
