#ifndef WATCHDOG_H
#define WATCHDOG_H

void watchdog_init(void);
void watchdog_kick(void);
void watchdog_check(void);
void vTaskWatchdog(void *pvParameters);

#endif /* WATCHDOG_H */
