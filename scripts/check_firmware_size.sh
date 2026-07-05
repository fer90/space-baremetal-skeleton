#!/usr/bin/env bash
# Fail if kernel.elf text or bss exceeds the budget for the given build flavor.
set -euo pipefail

FLAVOR="${1:?usage: check_firmware_size.sh <release|debug> <kernel.elf>}"
ELF="${2:?usage: check_firmware_size.sh <release|debug> <kernel.elf>}"

if [ ! -f "$ELF" ]; then
    echo "error: $ELF not found" >&2
    exit 1
fi

case "$FLAVOR" in
    release)
        MAX_TEXT=32000
        MAX_BSS=46000
        ;;
    debug)
        MAX_TEXT=34000
        MAX_BSS=47000
        ;;
    *)
        echo "error: unknown flavor '$FLAVOR' (expected release or debug)" >&2
        exit 1
        ;;
esac

text=$(riscv64-unknown-elf-size "$ELF" | awk 'END { print $1 }')
bss=$(riscv64-unknown-elf-size "$ELF" | awk 'END { print $3 }')

echo "Firmware size ($FLAVOR): text=${text} bss=${bss} (budget text<=${MAX_TEXT} bss<=${MAX_BSS})"

if [ "$text" -gt "$MAX_TEXT" ]; then
    echo "error: .text ${text} exceeds budget ${MAX_TEXT}" >&2
    exit 1
fi

if [ "$bss" -gt "$MAX_BSS" ]; then
    echo "error: .bss ${bss} exceeds budget ${MAX_BSS}" >&2
    exit 1
fi