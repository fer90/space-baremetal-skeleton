#include "common.h"
#include "uart.h"
#include "fault_inject.h"
#include "memory_scrub.h"
#include "timer.h"

#define FAULT_PROBABILITY 20

// Very simple Linear Congruential Generator (pseudo-random)
static uint32_t rand_state = 12345UL;

static uint32_t simple_rand(void) {
    rand_state = rand_state * 1103515245UL + 12345UL;
    return rand_state;
}

void fault_inject_init(void) {
    uart_puts("Fault injection initialized (SEU simulation)\r\n");
}

void inject_random_fault(volatile uint8_t *area) {
    if ((get_system_ticks() % FAULT_PROBABILITY) == 0) {
        // Flipping random bit in the scrub area
        uint32_t r = simple_rand();
        int idx = r % SCRUB_SIZE;
        int bit = (r >> 8) % 8;
        area[idx] ^= (1u << bit);
        uart_puts("Simulated fault injected\r\n");
    }
}
