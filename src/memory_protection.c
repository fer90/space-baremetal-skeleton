#include "memory_protection.h"
#include "common.h"
#include "system_state.h"

#define MAX_PROTECTED_REGIONS              8
#define MEM_PROT_VIOLATION_THRESHOLD       3

static memory_region_t protected_regions[MAX_PROTECTED_REGIONS];
static uint8_t region_count = 0;
static uint32_t violation_count = 0;

static bool region_overlaps(const memory_region_t *region, uintptr_t address, size_t size)
{
    uintptr_t access_end = address + size;

    return (address < region->end) && (access_end > region->start);
}

static void record_violation(void)
{
    violation_count++;
    if (violation_count >= MEM_PROT_VIOLATION_THRESHOLD) {
        (void) system_state_request_change(SYSTEM_STATE_DEGRADED, 0x04);
    }
}

static void report_violation(uintptr_t address,
                             size_t size,
                             const char *access_type,
                             const memory_region_t *region)
{
    uart_puts("!!! MEMORY PROTECTION VIOLATION !!!\r\n");
    uart_puts("  Region : ");
    uart_puts(region->name);
    uart_puts("\r\n  Access : ");
    uart_puts(access_type);
    uart_puts("\r\n  Address: 0x");
    uart_put_hex((uint32_t) address);
    uart_puts("  Size: ");
    uart_put_dec((uint32_t) size);
    uart_puts("\r\n");

    record_violation();
}

static bool memory_protection_check(uintptr_t address, size_t size, mem_perm_t required_perm)
{
    const char *access_type = (required_perm == MEM_PERM_WRITE) ? "WRITE" : "READ";

    for (uint8_t i = 0; i < region_count; i++) {
        const memory_region_t *region = &protected_regions[i];

        if (!region_overlaps(region, address, size)) {
            continue;
        }

        if ((region->permissions & required_perm) == 0) {
            report_violation(address, size, access_type, region);
            return false;
        }
    }

    return true;
}

void memory_protection_init(void)
{
    region_count = 0;
    violation_count = 0;
    uart_puts("Memory Protection initialized\r\n");
}

void memory_protection_add_region(uintptr_t start, uintptr_t end, mem_perm_t perms, const char *name)
{
    if (region_count >= MAX_PROTECTED_REGIONS) {
        uart_puts("Memory Protection: Region table full!\r\n");
        return;
    }

    protected_regions[region_count].start = start;
    protected_regions[region_count].end = end;
    protected_regions[region_count].permissions = perms;
    protected_regions[region_count].name = name;
    region_count++;
}

bool memory_protection_check_read(uintptr_t address, size_t size)
{
    return memory_protection_check(address, size, MEM_PERM_READ);
}

bool memory_protection_check_write(uintptr_t address, size_t size)
{
    return memory_protection_check(address, size, MEM_PERM_WRITE);
}

uint32_t memory_protection_get_violation_count(void)
{
    return violation_count;
}

uint8_t memory_protection_get_region_count(void)
{
    return region_count;
}