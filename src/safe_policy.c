#include "safe_policy.h"
#include "system_defs.h"
#include "fault_inject.h"
#include "log.h"
#include "uart.h"
#include "event_log.h"

#define WATCHDOG_SAFE_EXPECTED_BITS \
    (WATCHDOG_BIT_HEARTBEAT | WATCHDOG_BIT_MEMSCRUB)

uint32_t safe_policy_watchdog_expected_bits(SystemState_t state)
{
    if (state == SYSTEM_STATE_SAFE) {
        return WATCHDOG_SAFE_EXPECTED_BITS;
    }

    return WATCHDOG_EXPECTED_BITS;
}

bool safe_policy_allows_auto_fault_inject(SystemState_t state)
{
    return state == SYSTEM_STATE_NOMINAL;
}

bool safe_policy_allows_manual_fault_inject(SystemState_t state)
{
    return state != SYSTEM_STATE_SAFE;
}

bool safe_policy_allows_background_scrub(SystemState_t state)
{
    return state != SYSTEM_STATE_SAFE;
}

bool safe_policy_allows_heartbeat_uart(SystemState_t state)
{
    return state != SYSTEM_STATE_SAFE;
}

bool safe_policy_allows_seu_escalation(SystemState_t state)
{
    return state < SYSTEM_STATE_SAFE;
}

void safe_policy_on_degraded_enter(void)
{
    (void) fault_inject_set_enabled(false);
    uart_puts(LOG_PREFIX_DEGRADED "policy active: auto-inject off ");
    uart_puts("(background scrub and full watchdog unchanged)\r\n");
}

void safe_policy_on_enter(void)
{
    event_log_record_safe_policy(true);
    (void) fault_inject_set_enabled(false);
    uart_puts(LOG_PREFIX_SAFE "policy active: auto-inject off, background scrub off, ");
    uart_puts("watchdog expects Heartbeat+MemScrub only, heartbeat log suppressed\r\n");
}

void safe_policy_on_exit(void)
{
    event_log_record_safe_policy(false);
    uart_puts(LOG_PREFIX_SAFE "policy cleared: nominal operations restored ");
    uart_puts("(fault inject remains off until 'x')\r\n");
}