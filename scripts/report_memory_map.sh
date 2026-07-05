#!/usr/bin/env bash
# Summarize flash/RAM use from a linked kernel and its .map file.
set -euo pipefail

FLAVOR="${1:?usage: report_memory_map.sh <release|debug> <kernel.elf> <kernel.map>}"
ELF="${2:?usage: report_memory_map.sh <release|debug> <kernel.elf> <kernel.map>}"
MAP="${3:?usage: report_memory_map.sh <release|debug> <kernel.elf> <kernel.map>}"

if [[ ! -f "$ELF" ]]; then
    echo "error: $ELF not found" >&2
    exit 1
fi

if [[ ! -f "$MAP" ]]; then
    echo "error: $MAP not found" >&2
    exit 1
fi

read -r text data bss dec hex _ <<< "$(riscv64-unknown-elf-size "$ELF" | awk 'END { print $0 }')"

critical_start=$(grep -m1 '__critical_text_start' "$MAP" | awk '{print $1}')
critical_end=$(grep -m1 '__critical_text_end' "$MAP" | awk '{print $1}')
bss_start=$(grep -m1 '__bss_start' "$MAP" | awk '{print $1}')
bss_end=$(grep -m1 '__bss_end' "$MAP" | awk '{print $1}')
scrub_area=$(grep -m1 ' scrub_area$' "$MAP" | awk '{print $1}')

hex_to_dec() {
    printf '%d' "$1"
}

critical_bytes=0
bss_span=0
if [[ -n "$critical_start" && -n "$critical_end" ]]; then
    critical_bytes=$(( $(hex_to_dec "$critical_end") - $(hex_to_dec "$critical_start") ))
fi
if [[ -n "$bss_start" && -n "$bss_end" ]]; then
    bss_span=$(( $(hex_to_dec "$bss_end") - $(hex_to_dec "$bss_start") ))
fi

echo "Memory map summary ($FLAVOR)"
echo "  ELF: $ELF"
echo "  Load base: 0x80000000"
echo "  Flash (text+rodata): ${text} bytes"
echo "  Initialized data:    ${data} bytes"
echo "  BSS (size tool):     ${bss} bytes"
echo "  Total image (dec):   ${dec} bytes (${hex})"
echo "  .text.critical span: ${critical_bytes} bytes"
echo "  BSS span (.map):     ${bss_span} bytes (${bss_start:-?} .. ${bss_end:-?})"
echo "  scrub_area:          ${scrub_area:-not found}"
echo "  FreeRTOS heap budget: 32768 bytes (configTOTAL_HEAP_SIZE)"