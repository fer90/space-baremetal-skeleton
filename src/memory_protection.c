#include "memory_protection.h"
#include "common.h"
#include "system_state.h"

#define MAX_PROTECTED_REGIONS 8

static memory_region_t protected_regions[MAX_PROTECTED_REGIONS];
static uint8_t region_count = 0;

void memory_protection_init(void)
{
    region_count = 0;
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

static void report_violation(uintptr_t address, size_t size, const char* access_type, memory_region_t *region) {
  uart_puts("!!! MEMORY PROTECTION VIOLATION !!!\r\n");
    uart_puts("  Region : ");
    uart_puts(region->name);
    uart_puts("\r\n  Access : ");
    uart_puts(access_type);
    uart_puts("\r\n  Address: 0x");
    uart_put_hex(address);
    uart_puts("  Size: ");
    uart_put_dec(size);
    uart_puts("\r\n");

    if (gSystemState < SYSTEM_STATE_DEGRADED) {
        system_state_request_change(SYSTEM_STATE_DEGRADED, 0x04);
    }
}

bool memory_protection_check_write(uintptr_t address, size_t size)
{
    for (uint8_t i = 0; i < region_count; i++) {
        memory_region_t *region = &protected_regions[i];

        if (address >= region->start && (address + size) <= region->end) {
            if ((region->permissions & MEM_PERM_WRITE) == 0) {
                report_violation(address, size, "WRITE", region);
                return false;
            }
            return true;
        }
    }
    return true; // Not in a protected region → allow
}

bool memory_protection_check_read(uintptr_t address, size_t size){

    for (uint8_t i = 0; i < region_count; i++) {
        memory_region_t *region = &protected_regions[i];

	if (address >= region->start && (address + size) <= region->end) {
	    if ((region->permissions & MEM_PERM_READ) == 0) {
	        report_violation(address, size, "READ", region);
		return false;
	    }
	    return true;
	}
    }
    return true;
}
