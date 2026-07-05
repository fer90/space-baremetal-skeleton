#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

KERNEL_BIN="${KERNEL_BIN:-dist/release/kernel.bin}"

if [[ ! -f "$KERNEL_BIN" ]]; then
    echo "kernel binary not found: ${KERNEL_BIN} (run make first)" >&2
    exit 1
fi

LOG="$(mktemp)"
trap 'rm -f "$LOG"' EXIT

set +e
# h=help, d=DEGRADED, a=SAFE, n=NOMINAL recovery, l=flight recorder dump
( sleep 3; printf 'hdan'; sleep 2; printf 'l' ) | timeout 20s qemu-system-riscv64 \
    -machine virt -cpu rv64 -nographic -kernel "$KERNEL_BIN" -bios none >"$LOG" 2>&1
code=$?
set -e

if [[ "$code" -ne 0 && "$code" -ne 124 ]]; then
    echo "QEMU exited with status ${code}" >&2
    cat "$LOG" >&2
    exit "$code"
fi

require_pattern() {
    local pattern="$1"
    local message="$2"

    if ! grep -qF "$pattern" "$LOG"; then
        echo "QEMU smoke: ${message}" >&2
        echo "Expected pattern: ${pattern}" >&2
        cat "$LOG" >&2
        exit 1
    fi
}

require_pattern "[BOOT] image CRC OK" "missing boot image CRC OK"
require_pattern "=== FreeRTOS Migration Started ===" "missing boot banner"
require_pattern "Fault injection initialized (SEU simulation)" "missing fault-inject init"
require_pattern "[STATE] NOMINAL" "missing NOMINAL state"
require_pattern "Memory scrub initialized (EDAC Simulation)" "missing scrub init"
require_pattern "[CMD] input ready" "missing command input"
require_pattern "[CMD] Commands:" "missing help output"
require_pattern "[STATE] changed to DEGRADED" "missing DEGRADED transition (UART d)"
require_pattern "[STATE] changed to SAFE" "missing SAFE transition (UART a)"
require_pattern "[SAFE] policy active" "missing SAFE policy banner"
require_pattern "[STATE] changed to NOMINAL" "missing NOMINAL transition (UART n)"
require_pattern "[REC] flight log (" "missing flight recorder dump (UART l)"
require_pattern "NOMINAL->DEGRADED" "missing recorder DEGRADED transition"
require_pattern "DEGRADED->SAFE" "missing recorder SAFE transition"
require_pattern "SAFE_ENTER" "missing recorder SAFE_ENTER entry"

echo "QEMU smoke test OK (${KERNEL_BIN})"