#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

KERNEL_BIN="${KERNEL_BIN:-dist/release/kernel.bin}"
SOAK_SECONDS="${SOAK_SECONDS:-30}"

if [[ ! -f "$KERNEL_BIN" ]]; then
    echo "kernel binary not found: ${KERNEL_BIN} (run make first)" >&2
    exit 1
fi

LOG="$(mktemp)"
trap 'rm -f "$LOG"' EXIT

set +e
timeout "${SOAK_SECONDS}s" qemu-system-riscv64 \
    -machine virt -cpu rv64 -nographic -kernel "$KERNEL_BIN" -bios none >"$LOG" 2>&1
code=$?
set -e

if [[ "$code" -ne 0 && "$code" -ne 124 ]]; then
    echo "QEMU soak exited with status ${code}" >&2
    cat "$LOG" >&2
    exit "$code"
fi

if grep -qF '[ERROR]' "$LOG"; then
    echo "QEMU soak: unexpected [ERROR] output" >&2
    grep -F '[ERROR]' "$LOG" >&2 || true
    exit 1
fi

if grep -qF 'STACK OVERFLOW' "$LOG"; then
    echo "QEMU soak: stack overflow detected" >&2
    exit 1
fi

echo "QEMU soak test OK (${SOAK_SECONDS}s, ${KERNEL_BIN})"