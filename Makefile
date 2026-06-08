# Makefile for space-baremetal-skeleton (RISC-V QEMU)
CROSS_COMPILE ?= riscv64-unknown-elf-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -march=rv64imac -mabi=lp64 -mcmodel=medany -O2 -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -Iinclude
ASFLAGS = -march=rv64imac -mabi=lp64

# Sources - startup first for link order
CSRC = src/main.c src/uart.c src/watchdog.c src/memory_scrub.c src/fault_inject.c
ASRC = src/startup.s
OBJ = $(ASRC:.s=.o) $(CSRC:.c=.o)

all: kernel.elf kernel.bin

kernel.elf: $(OBJ)
	$(LD) -T linker.ld -o $@ $^

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f src/*.o *.elf *.bin

qemu: kernel.bin
	qemu-system-riscv64 -machine virt -cpu rv64 -nographic -kernel kernel.bin -bios none

.PHONY: all clean qemu