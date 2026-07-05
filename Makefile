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
       src/isr_stack_guard.c src/telemetry.c src/timer.c src/interrupt.c src/safe_policy.c \
       $(FREERTOS_SRC)
ASRC = src/startup.s

ifeq ($(DEBUG),1)
BUILD_FLAVOR = debug
else
BUILD_FLAVOR = release
endif

BUILD_DIR = build/$(BUILD_FLAVOR)
DIST_DIR = dist/$(BUILD_FLAVOR)

OBJ = $(addprefix $(BUILD_DIR)/,$(ASRC:.s=.o)) \
      $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(filter %.c,$(CSRC)))) \
      $(addprefix $(BUILD_DIR)/,$(patsubst %.S,%.o,$(filter %.S,$(CSRC))))

KERNEL_ELF = $(DIST_DIR)/kernel.elf
KERNEL_BIN = $(DIST_DIR)/kernel.bin
KERNEL_MAP = $(DIST_DIR)/kernel.map
SIZE = $(CROSS_COMPILE)size

all: $(KERNEL_ELF) $(KERNEL_BIN)
	@$(SIZE) $(KERNEL_ELF) | tee $(DIST_DIR)/size.txt

$(DIST_DIR):
	mkdir -p $@

$(KERNEL_ELF): $(OBJ) | $(DIST_DIR)
	$(LD) -T linker.ld -Map=$(KERNEL_MAP) --cref -o $@ $^

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/FreeRTOS/Source/portable/GCC/RISC-V/%.o: FreeRTOS/Source/portable/GCC/RISC-V/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/FreeRTOS/Source/portable/MemMang/%.o: FreeRTOS/Source/portable/MemMang/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Host unit tests (Unity)
HOST_CC ?= gcc
UNITY_DIR = test/Unity/src
TEST_SUPPORT = test/support
TEST_OBJ_DIR = test/obj

# Stubs omit -Iinclude so <string.h> resolves to libc, not include/string.h.
TEST_CFLAGS_STUB = -std=c11 -Wall -Wextra -O0 -g \
                   -I$(TEST_SUPPORT) -I$(UNITY_DIR)
TEST_CFLAGS = $(TEST_CFLAGS_STUB) -Iinclude

TEST_CASE_OBJ = test_runner \
                test_memory_protection \
                test_system_state \
                test_memory_scrub \
                test_fault_inject \
                test_state_machine \
                test_fault_queue \
                test_watchdog \
                test_command \
                test_safe_policy

TEST_OBJ = $(addprefix $(TEST_OBJ_DIR)/,$(addsuffix .o,$(TEST_CASE_OBJ))) \
           $(TEST_OBJ_DIR)/test_support.o \
           $(TEST_OBJ_DIR)/unity.o \
           $(TEST_OBJ_DIR)/freertos_stub.o \
           $(TEST_OBJ_DIR)/uart_stub.o \
           $(TEST_OBJ_DIR)/system_state.o \
           $(TEST_OBJ_DIR)/watchdog.o \
           $(TEST_OBJ_DIR)/command.o \
           $(TEST_OBJ_DIR)/memory_protection.o \
           $(TEST_OBJ_DIR)/memory_scrub.o \
           $(TEST_OBJ_DIR)/fault_queue.o \
           $(TEST_OBJ_DIR)/fault_inject.o \
           $(TEST_OBJ_DIR)/state_machine.o \
           $(TEST_OBJ_DIR)/golden_copy.o \
           $(TEST_OBJ_DIR)/safe_policy.o

test_runner: $(TEST_OBJ)
	$(HOST_CC) $(TEST_CFLAGS) -o $@ $^

test: test_runner
	./test_runner

check: test lint

$(TEST_OBJ_DIR)/%.o: test/%.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/unity.o: $(UNITY_DIR)/unity.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/freertos_stub.o: $(TEST_SUPPORT)/freertos_stub.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS_STUB) -c $< -o $@

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

$(TEST_OBJ_DIR)/memory_scrub.o: src/memory_scrub.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/fault_queue.o: src/fault_queue.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/fault_inject.o: src/fault_inject.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/state_machine.o: src/state_machine.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/watchdog.o: src/watchdog.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/command.o: src/command.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/safe_policy.o: src/safe_policy.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/test_support.o: test/support/test_support.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(HOST_CC) $(TEST_CFLAGS) -c $< -o $@

CPPCHECK ?= cppcheck

lint:
	@command -v $(CPPCHECK) >/dev/null 2>&1 || { \
		echo "error: cppcheck not found (required for make lint / make check)" >&2; \
		echo "  Ubuntu/Debian: sudo apt install cppcheck" >&2; \
		exit 127; \
	}
	$(CPPCHECK) --enable=warning,style,performance,portability --inconclusive --inline-suppr --quiet \
		-Iinclude \
		-IFreeRTOS/Source/include \
		-IFreeRTOS/Source/portable/GCC/RISC-V \
		--suppressions-list=cppcheck-suppressions.txt \
		--suppress=missingIncludeSystem \
		--error-exitcode=1 \
		src/

clean:
	rm -rf build dist $(TEST_OBJ_DIR) test_runner
	find src FreeRTOS/Source -name '*.o' -delete

qemu: $(KERNEL_BIN)
	qemu-system-riscv64 -machine virt -cpu rv64 -nographic -kernel $(KERNEL_BIN) -bios none

qemu-smoke:
	chmod +x scripts/qemu_smoke.sh
	KERNEL_BIN=$(if $(KERNEL_BIN),$(KERNEL_BIN),dist/release/kernel.bin) scripts/qemu_smoke.sh

qemu-soak:
	chmod +x scripts/qemu_soak.sh
	KERNEL_BIN=$(if $(KERNEL_BIN),$(KERNEL_BIN),dist/release/kernel.bin) scripts/qemu_soak.sh

debug:
	$(MAKE) DEBUG=1 all

.PHONY: all clean qemu qemu-smoke qemu-soak debug test lint check
