#ifndef MEMORY_SCRUB_H
#define MEMORY_SCRUB_H

#include <stdint.h>

#define SCRUB_SIZE 512

extern volatile uint8_t scrub_area[SCRUB_SIZE];

void memory_scrub_init(volatile uint8_t *area);
void memory_scrub(volatile uint8_t *area);
void vTaskMemoryScrub(void *pvParameters);

#endif /* MEMORY_SCRUB_H */
