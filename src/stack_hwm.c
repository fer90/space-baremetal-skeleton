#ifdef DEBUG

#include "common.h"
#include "stack_hwm.h"

#define STACK_HWM_MAX_TASKS 8

typedef struct {
    TaskHandle_t handle;
    const char *name;
    configSTACK_DEPTH_TYPE allocated_words;
    UBaseType_t min_free_words;
} stack_hwm_entry_t;

static stack_hwm_entry_t entries[STACK_HWM_MAX_TASKS];
static size_t entry_count = 0;

static void print_dec(uint32_t value) {
    char buf[11];
    int i = 10;

    buf[i] = '\0';
    if (value == 0) {
        uart_putc('0');
        return;
    }

    while (value > 0) {
        buf[--i] = (char) ('0' + (value % 10));
        value /= 10;
    }

    uart_puts(&buf[i]);
}

static void print_task_line(stack_hwm_entry_t *entry) {
    UBaseType_t free_words;

    if (entry->handle == NULL) {
        return;
    }

    free_words = uxTaskGetStackHighWaterMark(entry->handle);
    if (free_words < entry->min_free_words) {
        entry->min_free_words = free_words;
    }

    uart_puts(entry->name);
    uart_puts(": alloc=");
    print_dec((uint32_t) entry->allocated_words);
    uart_puts(" free=");
    print_dec((uint32_t) free_words);
    uart_puts(" peak=");
    print_dec((uint32_t) (entry->allocated_words - entry->min_free_words));
    uart_puts("\r\n");
}

void stack_hwm_register(TaskHandle_t handle,
                        const char *name,
                        configSTACK_DEPTH_TYPE allocated_words) {
    if (entry_count >= STACK_HWM_MAX_TASKS) {
        uart_puts("stack_hwm_register: table full\r\n");
        return;
    }

    entries[entry_count].handle = handle;
    entries[entry_count].name = name;
    entries[entry_count].allocated_words = allocated_words;
    entries[entry_count].min_free_words = allocated_words;
    entry_count++;
}

void stack_hwm_print_snapshot(void) {
    taskENTER_CRITICAL();
    uart_puts("--- task stack HWM (words) ---\r\n");

    for (size_t i = 0; i < entry_count; i++) {
        print_task_line(&entries[i]);
    }
    taskEXIT_CRITICAL();
}

void vTaskStackHwmMonitor(void *pvParameters) {
    (void) pvParameters;

    for (;;) {
        stack_hwm_print_snapshot();
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

#endif /* DEBUG */