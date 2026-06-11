# space-baremetal-skeleton

Minimal bare-metal firmware skeleton for RISC-V, designed with **space / aerospace constraints** in mind.

This project serves as a learning platform to understand embedded systems for space.

## Goals

This skeleton directly applies core concepts from **Computer Architecture: A Quantitative Approach**:

- **Dependability** (MTTF, FIT, fault tolerance): Software watchdog + FDIR simulation
- **Radiation Hardening**: Memory scrubbing with golden-copy EDAC simulation + fault injection (SEU emulation)
- **Power Awareness**: Opportunities for `wfi` and deterministic loops
- **Determinism & Performance**: Super-loop architecture with measurable timing

## Features

- **Deterministic Boot**: Custom assembly startup (`startup.s`) with stack setup and BSS zeroing
- **Hardware Timer + Interrupts** — CLINT-based system tick with machine-mode interrupts
- **Dependability Mechanisms (FDIR)**:
  - Software watchdog
  - Memory scrubbing with golden-copy EDAC simulation
  - Stack guard region + overflow detection
- **Fault Injection**: Simulated SEUs (bit flips) to test resilience
- **UART Telemetry**: Simple console output (emulates downlink)

## Build & Run (QEMU)

```bash
# Prerequisites
sudo apt install gcc-riscv64-unknown-elf qemu-system-riscv64  # Ubuntu/Debian

# Build and run
make clean
make
make qemu