#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>

void watchdog_kick(uint32_t taskBit);
void vTaskWatchdog(void *pvParameters);

#endif /* WATCHDOG_H */