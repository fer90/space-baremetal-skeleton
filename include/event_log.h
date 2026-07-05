#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <stdbool.h>
#include <stdint.h>

#include "system_state.h"

#define EVENT_LOG_CAPACITY 48u

typedef enum {
    EVENT_LOG_BOOT = 0,
    EVENT_LOG_STATE_CHANGE,
    EVENT_LOG_WATCHDOG_TIMEOUT,
    EVENT_LOG_SEU_CORRECTED,
    EVENT_LOG_MEM_PROT_THRESHOLD,
    EVENT_LOG_SAFE_POLICY,
    EVENT_LOG_OPERATOR,
    EVENT_LOG_IMAGE_CRC_FAIL,
} EventLogType_t;

typedef struct {
    uint8_t type;
    uint8_t arg_a;
    uint8_t arg_b;
    uint8_t _pad;
    uint32_t tick;
    uint32_t data;
} EventLogEntry_t;

void event_log_init(void);
void event_log_reset(void);

void event_log_record_state_change(SystemState_t from,
                                   SystemState_t to,
                                   uint32_t reason);
void event_log_record_watchdog_timeout(uint32_t missing_bits, SystemState_t state);
void event_log_record_seu_corrected(uint32_t byte_index, uint8_t bit);
void event_log_record_mem_prot_threshold(uint32_t violation_count);
void event_log_record_safe_policy(bool entering_safe);
void event_log_record_operator(char command_key);
void event_log_record_image_crc_fail(uint32_t expected, uint32_t computed);

uint16_t event_log_count(void);
bool event_log_get_entry_chronological(uint16_t index, EventLogEntry_t *entry);
void event_log_dump(void);

#endif /* EVENT_LOG_H */