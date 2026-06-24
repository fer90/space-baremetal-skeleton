#ifndef MEMORY_SCRUB_H
#define MEMORY_SCRUB_H

#include <stdbool.h>
#include <stdint.h>
#include "system_defs.h"

#define SCRUB_SIZE 512

extern volatile uint8_t scrub_area[SCRUB_SIZE];
extern const uint8_t golden_copy[SCRUB_SIZE];

void memory_scrub_init(volatile uint8_t *area);
void memory_scrub(volatile uint8_t *area);
bool memory_scrub_fix_event(volatile uint8_t *area, const FaultEvent_t *event);
void vTaskMemoryScrub(void *pvParameters);

#endif /* MEMORY_SCRUB_H */