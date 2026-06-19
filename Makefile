# Makefile for space-baremetal-skeleton (RISC-V QEMU)
CROSS_COMPILE ?= riscv64-unknown-elf-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

# Set DEBUG=1 or run `make debug` to enable telemetry task
DEBUG ?= 0
DEBUG_CFLAGS = $(if $(filter 1,$(DEBUG)),-DDEBUG,)

CFLAGS = -march=rv64imac_zicsr -mabi=lp64 -mcmodel=medany -O2 -Wall -Wextra \
         -ffreestanding -nostdlib -nostartfiles -Iinclude -IFreeRTOS/Source/include \
         -IFreeRTOS/Source/portable/GCC/RISC-V $(DEBUG_CFLAGS)
ASFLAGS = -march=rv64imac_zicsr -mabi=lp64 -IFreeRTOS/Source/portable/GCC/RISC-V

# FreeRTOS source files
FREERTOS_SRC = \
    FreeRTOS/Source/tasks.c \
    FreeRTOS/Source/queue.c \
    FreeRTOS/Source/list.c \
    FreeRTOS/Source/timers.c \
    FreeRTOS/Source/event_groups.c \
    FreeRTOS/Source/stream_buffer.c \
    FreeRTOS/Source/portable/MemMang/heap_4.c \
    FreeRTOS/Source/portable/GCC/RISC-V/port.c \
    FreeRTOS/Source/portable/GCC/RISC-V/portASM.S

# Sources - startup first for link order
CSRC = src/main.c src/uart.c src/string.c src/watchdog.c src/memory_scrub.c \
       src/fault_inject.c src/isr_stack_guard.c src/telemetry.c src/timer.c src/interrupt.c \
       $(FREERTOS_SRC)
ASRC = src/startup.s
OBJ = $(ASRC:.s=.o) \
      $(patsubst %.c,%.o,$(filter %.c,$(CSRC))) \
      $(patsubst %.S,%.o,$(filter %.S,$(CSRC)))

all: kernel.elf kernel.bin

kernel.elf: $(OBJ)
	$(LD) -T linker.ld -o $@ $^

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: FreeRTOS/Source/portable/GCC/RISC-V/%.S
	$(AS) $(ASFLAGS) -c $< -o $@

# Special rule for heap_4.c (in subdirectory)
FreeRTOS/Source/portable/MemMang/%.o: FreeRTOS/Source/portable/MemMang/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) *.elf *.bin

qemu: kernel.bin
	qemu-system-riscv64 -machine virt -cpu rv64 -nographic -kernel kernel.bin -bios none

debug:
	$(MAKE) DEBUG=1 all

.PHONY: all clean qemu debug
