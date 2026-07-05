#include "event_log.h"
#include "log.h"
#include "uart.h"

#include "FreeRTOS.h"
#include "task.h"

static EventLogEntry_t event_log_entries[EVENT_LOG_CAPACITY];
static uint16_t event_log_head;
static uint16_t event_log_count_value;

static void event_log_store(uint8_t type,
                            uint8_t arg_a,
                            uint8_t arg_b,
                            uint32_t data)
{
    EventLogEntry_t *entry = &event_log_entries[event_log_head];

    entry->type = type;
    entry->arg_a = arg_a;
    entry->arg_b = arg_b;
    entry->tick = (uint32_t) xTaskGetTickCount();
    entry->data = data;

    event_log_head = (uint16_t) ((event_log_head + 1u) % EVENT_LOG_CAPACITY);
    if (event_log_count_value < EVENT_LOG_CAPACITY) {
        event_log_count_value++;
    }
}

void event_log_init(void)
{
    event_log_reset();
    event_log_store(EVENT_LOG_BOOT, 0u, 0u, 0u);
}

void event_log_reset(void)
{
    event_log_head = 0u;
    event_log_count_value = 0u;
}

void event_log_record_state_change(SystemState_t from,
                                   SystemState_t to,
                                   uint32_t reason)
{
    event_log_store(EVENT_LOG_STATE_CHANGE,
                    (uint8_t) from,
                    (uint8_t) to,
                    reason);
}

void event_log_record_watchdog_timeout(uint32_t missing_bits, SystemState_t state)
{
    event_log_store(EVENT_LOG_WATCHDOG_TIMEOUT,
                    (uint8_t) state,
                    0u,
                    missing_bits);
}

void event_log_record_seu_corrected(uint32_t byte_index, uint8_t bit)
{
    event_log_store(EVENT_LOG_SEU_CORRECTED,
                    (uint8_t) byte_index,
                    bit,
                    0u);
}

void event_log_record_mem_prot_threshold(uint32_t violation_count)
{
    event_log_store(EVENT_LOG_MEM_PROT_THRESHOLD, 0u, 0u, violation_count);
}

void event_log_record_safe_policy(bool entering_safe)
{
    event_log_store(EVENT_LOG_SAFE_POLICY,
                    entering_safe ? 1u : 0u,
                    0u,
                    0u);
}

void event_log_record_operator(char command_key)
{
    event_log_store(EVENT_LOG_OPERATOR, (uint8_t) command_key, 0u, 0u);
}

uint16_t event_log_count(void)
{
    return event_log_count_value;
}

bool event_log_get_entry_chronological(uint16_t index, EventLogEntry_t *entry)
{
    uint16_t start;
    uint16_t slot;

    if (entry == NULL || index >= event_log_count_value) {
        return false;
    }

    start = (event_log_count_value < EVENT_LOG_CAPACITY) ? 0u : event_log_head;
    slot = (uint16_t) ((start + index) % EVENT_LOG_CAPACITY);
    *entry = event_log_entries[slot];
    return true;
}

static void event_log_put_state_name(SystemState_t state)
{
    switch (state) {
        case SYSTEM_STATE_BOOT:
            uart_puts("BOOT");
            break;
        case SYSTEM_STATE_NOMINAL:
            uart_puts("NOMINAL");
            break;
        case SYSTEM_STATE_DEGRADED:
            uart_puts("DEGRADED");
            break;
        case SYSTEM_STATE_SAFE:
            uart_puts("SAFE");
            break;
        default:
            uart_puts("UNKNOWN");
            break;
    }
}

static void event_log_dump_entry(const EventLogEntry_t *entry)
{
    uart_puts(LOG_PREFIX_REC "t=");
    uart_put_dec(entry->tick);
    uart_puts(" ");

    switch ((EventLogType_t) entry->type) {
        case EVENT_LOG_BOOT:
            uart_puts("BOOT");
            break;

        case EVENT_LOG_STATE_CHANGE:
            uart_puts("STATE ");
            event_log_put_state_name((SystemState_t) entry->arg_a);
            uart_puts("->");
            event_log_put_state_name((SystemState_t) entry->arg_b);
            uart_puts(" reason=0x");
            uart_put_hex(entry->data);
            break;

        case EVENT_LOG_WATCHDOG_TIMEOUT:
            uart_puts("WDT_TIMEOUT state=");
            event_log_put_state_name((SystemState_t) entry->arg_a);
            uart_puts(" missing=0x");
            uart_put_hex(entry->data);
            break;

        case EVENT_LOG_SEU_CORRECTED:
            uart_puts("SEU byte=");
            uart_put_dec(entry->arg_a);
            uart_puts(" bit=");
            uart_put_dec(entry->arg_b);
            break;

        case EVENT_LOG_MEM_PROT_THRESHOLD:
            uart_puts("MPU_THRESHOLD count=");
            uart_put_dec(entry->data);
            break;

        case EVENT_LOG_SAFE_POLICY:
            uart_puts(entry->arg_a != 0u ? "SAFE_ENTER" : "SAFE_EXIT");
            break;

        case EVENT_LOG_OPERATOR:
            uart_puts("OP key='");
            uart_putc((char) entry->arg_a);
            uart_puts("'");
            break;

        default:
            uart_puts("UNKNOWN");
            break;
    }

    uart_puts("\r\n");
}

void event_log_dump(void)
{
    uint16_t index;

    uart_puts(LOG_PREFIX_REC "flight log (");
    uart_put_dec(event_log_count_value);
    uart_puts("/");
    uart_put_dec(EVENT_LOG_CAPACITY);
    uart_puts(" entries)\r\n");

    for (index = 0; index < event_log_count_value; index++) {
        EventLogEntry_t entry;

        if (event_log_get_entry_chronological(index, &entry)) {
            event_log_dump_entry(&entry);
        }
    }
}