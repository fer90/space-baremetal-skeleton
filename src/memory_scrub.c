#include "common.h"

volatile uint8_t scrub_area[SCRUB_SIZE];

// Keep a golden copy in memory
static uint8_t golden_copy[SCRUB_SIZE];

#define SEU_THRESHOLD_FOR_DEGRADED 5
static uint32_t seu_counter = 0;

static void memory_scrub_record_seu(void) {
    seu_counter++;
    if (seu_counter >= SEU_THRESHOLD_FOR_DEGRADED) {
        (void) system_state_request_change(SYSTEM_STATE_DEGRADED, 0x02);
        seu_counter = 0;
    }
}

void memory_scrub_init(volatile uint8_t *area) {
    for (int i = 0; i < SCRUB_SIZE; i++) {
        area[i] = (uint8_t)i;
        golden_copy[i] = (uint8_t)i;
    }

    memory_protection_add_region((uintptr_t) golden_copy, (uintptr_t) golden_copy + sizeof(golden_copy), MEM_PERM_READ, "GoldenCopy");

    uart_puts("Memory scrub initialized (EDAC Simulation)\r\n");
}

bool memory_scrub_fix_event(volatile uint8_t *area, const FaultEvent_t *event) {
    uint32_t idx;
    uint8_t bit_mask;
    uint8_t actual;
    uint8_t expected;

    if (event == NULL || event->index >= SCRUB_SIZE || event->bit >= 8) {
        return false;
    }

    idx = event->index;
    bit_mask = (uint8_t) (1u << event->bit);
    actual = area[idx];
    expected = golden_copy[idx];

    if ((actual & bit_mask) == (expected & bit_mask)) {
        uart_puts("Memory scrub: targeted check @ byte ");
        uart_put_hex(idx);
        uart_puts(" bit ");
        uart_put_hex(event->bit);
        uart_puts(" — ok\r\n");
        return false;
    }

    area[idx] = (uint8_t) ((actual & ~bit_mask) | (expected & bit_mask));
    uart_puts("Memory scrub: SEU corrected @ byte ");
    uart_put_hex(idx);
    uart_puts(" bit ");
    uart_put_hex(event->bit);
    uart_puts("\r\n");
    memory_scrub_record_seu();
    return true;
}

void memory_scrub(volatile uint8_t *area) {
    uint32_t errors_corrected = 0;

    for (int i = 0; i < SCRUB_SIZE; i++) {
        if (area[i] != golden_copy[i]) {
            area[i] = golden_copy[i];
            errors_corrected++;
            memory_scrub_record_seu();
        }
    }

    if (errors_corrected > 0) {
        uart_puts("Memory scrub: SEU(s) detected and corrected\r\n");
    } else {
        uart_puts("Memory scrub completed - No errors\r\n");
    }
}

void vTaskMemoryScrub(void *pvParameters) {
    (void) pvParameters;
    FaultEvent_t event;
    bool had_event;

    memory_scrub_init(scrub_area);

    for (;;) {
        had_event = false;

        while (xQueueReceive(xFaultQueue, &event, 0) == pdPASS) {
            had_event = true;
            (void) memory_scrub_fix_event(scrub_area, &event);
        }

        if (!had_event) {
            memory_scrub(scrub_area);
        }

        watchdog_kick(WATCHDOG_BIT_MEMSCRUB);
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}
