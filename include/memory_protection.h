#ifndef MEMORY_PROTECTION_H
#define MEMORY_PROTECTION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    MEM_PERM_NONE   = 0,
    MEM_PERM_READ   = (1 << 0),
    MEM_PERM_WRITE  = (1 << 1),
    MEM_PERM_EXEC   = (1 << 2),
} mem_perm_t;

typedef struct {
    uintptr_t   start;
    uintptr_t   end;
    mem_perm_t  permissions;
    const char *name;
} memory_region_t;

void memory_protection_init(void);
void memory_protection_add_region(uintptr_t start, uintptr_t end, mem_perm_t perms, const char *name);
bool memory_protection_check_read(uintptr_t address, size_t size);
bool memory_protection_check_write(uintptr_t address, size_t size);
uint32_t memory_protection_get_violation_count(void);
uint8_t memory_protection_get_region_count(void);

#endif /* MEMORY_PROTECTION_H */