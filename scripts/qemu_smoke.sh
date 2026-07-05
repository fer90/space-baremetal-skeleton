#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if [[ ! -f kernel.bin ]]; then
    echo "kernel.bin not found; run make first" >&2
    exit 1
fi

LOG="$(mktemp)"
trap 'rm -f "$LOG"' EXIT

set +e
( sleep 3; printf 'h' ) | timeout 10s qemu-system-riscv64 \
    -machine virt -cpu rv64 -nographic -kernel kernel.bin -bios none >"$LOG" 2>&1
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

require_pattern "=== FreeRTOS Migration Started ===" "missing boot banner"
require_pattern "[STATE] NOMINAL" "missing NOMINAL state"
require_pattern "Memory scrub initialized (EDAC Simulation)" "missing scrub init"
require_pattern "[CMD] input ready" "missing command input"
require_pattern "[CMD] Commands:" "missing help output"

echo "QEMU smoke test OK"