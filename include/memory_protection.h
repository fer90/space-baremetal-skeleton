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
    const char *name;           // For debugging
} memory_region_t;

void memory_protection_init(void);
bool memory_protection_check_write(uintptr_t address, size_t size);
void memory_protection_add_region(uintptr_t start, uintptr_t end, mem_perm_t perms, const char *name);

#endif
