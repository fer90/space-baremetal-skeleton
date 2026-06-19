#ifdef DEBUG

#include "common.h"
#include "telemetry.h"

#define TELEMETRY_MAX_TASKS 8
#define TELEMETRY_PERIOD_MS 15000

typedef struct {
    TaskHandle_t handle;
    const char *name;
    configSTACK_DEPTH_TYPE allocated_words;
    UBaseType_t min_free_words;
} telemetry_task_entry_t;

static telemetry_task_entry_t task_entries[TELEMETRY_MAX_TASKS];
static size_t task_entry_count = 0;

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

static void print_uptime(void) {
    uint32_t uptime_s = (uint32_t) (xTaskGetTickCount() / configTICK_RATE_HZ);

    uart_puts("uptime_s=");
    print_dec(uptime_s);
    uart_puts("\r\n");
}

static void print_heap(void) {
    uart_puts("heap: free=");
    print_dec((uint32_t) xPortGetFreeHeapSize());
    uart_puts(" min_ever=");
    print_dec((uint32_t) xPortGetMinimumEverFreeHeapSize());
    uart_puts("\r\n");
}

static void print_isr_stack(void) {
    bool corrupt = isr_stack_guard_check();

    uart_puts("isr_stack: ");
    uart_puts(corrupt ? "CORRUPT" : "ok");
    uart_puts(" hwm_bytes=");
    print_dec(isr_stack_guard_get_hwm_bytes());
    uart_puts("\r\n");
}

static void print_task_line(telemetry_task_entry_t *entry) {
    UBaseType_t free_words;

    if (entry->handle == NULL) {
        return;
    }

    free_words = uxTaskGetStackHighWaterMark(entry->handle);
    if (free_words < entry->min_free_words) {
        entry->min_free_words = free_words;
    }

    uart_puts("  ");
    uart_puts(entry->name);
    uart_puts(": alloc=");
    print_dec((uint32_t) entry->allocated_words);
    uart_puts(" free=");
    print_dec((uint32_t) free_words);
    uart_puts(" peak=");
    print_dec((uint32_t) (entry->allocated_words - entry->min_free_words));
    uart_puts("\r\n");
}

static void print_task_stacks(void) {
    uart_puts("stacks (words):\r\n");

    for (size_t i = 0; i < task_entry_count; i++) {
        print_task_line(&task_entries[i]);
    }
}

void telemetry_register_task(TaskHandle_t handle,
                             const char *name,
                             configSTACK_DEPTH_TYPE allocated_words) {
    if (task_entry_count >= TELEMETRY_MAX_TASKS) {
        uart_puts("telemetry_register_task: table full\r\n");
        return;
    }

    task_entries[task_entry_count].handle = handle;
    task_entries[task_entry_count].name = name;
    task_entries[task_entry_count].allocated_words = allocated_words;
    task_entries[task_entry_count].min_free_words = allocated_words;
    task_entry_count++;
}

void telemetry_print_snapshot(void) {
    taskENTER_CRITICAL();
    uart_puts("=== telemetry ===\r\n");
    print_uptime();
    print_heap();
    print_isr_stack();
    print_task_stacks();
    uart_puts("=================\r\n");
    taskEXIT_CRITICAL();
}

void vTaskTelemetry(void *pvParameters) {
    (void) pvParameters;

    for (;;) {
        telemetry_print_snapshot();
        vTaskDelay(pdMS_TO_TICKS(TELEMETRY_PERIOD_MS));
    }
}

#endif /* DEBUG */