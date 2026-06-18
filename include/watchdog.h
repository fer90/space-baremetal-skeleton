#ifndef WATCHDOG_H
#define WATCHDOG_H

// Define bit <-> task mapping
#define WATCHDOG_BIT_HEARTBEAT (1 << 0)
#define WATCHDOG_BIT_MEMSCRUB (1 << 1)
#define WATCHDOG_BIT_FAULTINJECT (1 << 2)

#define WATCHDOG_EXPECTED_BITS (WATCHDOG_BIT_HEARTBEAT | WATCHDOG_BIT_MEMSCRUB | WATCHDOG_BIT_FAULTINJECT)

/* Must exceed the slowest task kick period (fault inject = 3 s). */
#define WATCHDOG_TIMEOUT_MS  3500

void watchdog_init(void);
void watchdog_kick(uint32_t taskBit);
void watchdog_check(void);
void vTaskWatchdog(void *pvParameters);

#endif /* WATCHDOG_H */
