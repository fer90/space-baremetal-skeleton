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
CSRC = src/main.c src/tasks.c src/system.c src/system_state.c src/state_machine.c src/critical_exec.c \
       src/command.c src/uart.c \
       src/string.c src/watchdog.c src/memory_protection.c src/golden_copy.c src/memory_scrub.c src/fault_inject.c src/fault_queue.c \
       src/isr_stack_guard.c src/telemetry.c src/timer.c src/interrupt.c \
       $(FREERTOS_SRC)
ASRC = src/startup.s
OBJ = $(ASRC:.s=.o) \
      $(patsubst %.c,%.o,$(filter %.c,$(CSRC))) \
      $(patsubst %.S,%.o,$(filter %.S,$(CSRC)))

all: kernel.elf kernel.bin

kernel.elf: $(OBJ)
	$(LD) -T linker.ld -Map=kernel.map --cref -o $@ $^

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

# Host unit tests (Unity)
HOST_CC ?= gcc
UNITY_DIR = test/Unity/src
TEST_SUPPORT = test/support
TEST_OBJ_DIR = test/obj

TEST_CFLAGS = -std=c11 -Wall -Wextra -O0 -g \
              -I$(TEST_SUPPORT) -Iinclude -I$(UNITY_DIR)

TEST_OBJ = $(TEST_OBJ_DIR)/test_runner.o \
           $(TEST_OBJ_DIR)/unity.o \
           $(TEST_OBJ_DIR)/freertos_stub.o \
           $(TEST_OBJ_DIR)/uart_stub.o \
           $(TEST_OBJ_DIR)/system_state.o \
           $(TEST_OBJ_DIR)/memory_protection.o \
           $(TEST_OBJ_DIR)/golden_copy.o

test_runner: $(TEST_OBJ)
	$(HOST_CC) $(TEST_CFLAGS) -o $@ $^

test: test_runner
	./test_runner

$(TEST_OBJ_DIR)/%.o: test/%.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/unity.o: $(UNITY_DIR)/unity.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/freertos_stub.o: $(TEST_SUPPORT)/freertos_stub.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/uart_stub.o: $(TEST_SUPPORT)/uart_stub.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/system_state.o: src/system_state.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/memory_protection.o: src/memory_protection.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/golden_copy.o: src/golden_copy.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

CPPCHECK ?= cppcheck

lint:
	$(CPPCHECK) --enable=warning,style,performance,portability --inconclusive --inline-suppr --quiet \
		-Iinclude \
		-IFreeRTOS/Source/include \
		-IFreeRTOS/Source/portable/GCC/RISC-V \
		--suppressions-list=cppcheck-suppressions.txt \
		--suppress=missingIncludeSystem \
		--error-exitcode=1 \
		src/

clean:
	rm -f $(OBJ) *.elf *.bin *.map test_runner
	rm -rf $(TEST_OBJ_DIR)

qemu: kernel.bin
	qemu-system-riscv64 -machine virt -cpu rv64 -nographic -kernel kernel.bin -bios none

debug:
	$(MAKE) DEBUG=1 all

.PHONY: all clean qemu debug test lint
