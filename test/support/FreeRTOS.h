#ifndef FREERTOS_H
#define FREERTOS_H

#include <stddef.h>
#include <stdint.h>

#include "projdefs.h"

typedef uint32_t TickType_t;
typedef int32_t BaseType_t;
typedef unsigned long UBaseType_t;
typedef void *TaskHandle_t;

#define portMAX_DELAY 0xFFFFFFFFu

#endif /* FREERTOS_H */