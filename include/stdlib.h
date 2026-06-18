#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>   // for size_t

/* Minimal stubs so FreeRTOS compiles in bare-metal */

void abort(void);

void *malloc(size_t size);
void free(void *ptr);

int abs(int n);

#endif /* STDLIB_H */