#include "critical_exec.h"
#include "memory_protection.h"

void memory_protection_register_critical_text(void)
{
    uintptr_t start = (uintptr_t) __critical_text_start;
    uintptr_t end = (uintptr_t) __critical_text_end;

    if (start >= end) {
        return;
    }

    memory_protection_protect_region(start, end, MEM_PERM_READ | MEM_PERM_EXEC, "CriticalText");
}