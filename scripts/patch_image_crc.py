#!/usr/bin/env python3
"""Compute CRC32 over the RX load segment and patch g_image_crc_expected in kernel.bin."""

from __future__ import annotations

import struct
import subprocess
import sys
from pathlib import Path

LOAD_VADDR = 0x80000000
CRC32_POLY = 0xEDB88320
CRC_SLOT_SYMBOL = "g_image_crc_expected"
CRC_END_SYMBOL = "__image_integrity_end"


def crc32_ieee(data: bytes) -> int:
    crc = 0xFFFFFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ CRC32_POLY
            else:
                crc >>= 1
    return crc ^ 0xFFFFFFFF


def symbol_address(elf_path: Path, symbol: str) -> int:
    output = subprocess.check_output(
        ["riscv64-unknown-elf-nm", str(elf_path)],
        text=True,
        errors="replace",
    )
    for line in output.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[-1] == symbol:
            return int(parts[0], 16)
    raise RuntimeError(f"symbol not found: {symbol}")


def first_rx_segment_size(elf_path: Path) -> int:
    output = subprocess.check_output(
        ["riscv64-unknown-elf-readelf", "-l", str(elf_path)],
        text=True,
        errors="replace",
    )

    pending_load: str | None = None
    for line in output.splitlines():
        stripped = line.strip()
        if stripped.startswith("LOAD"):
            pending_load = stripped
            continue
        if pending_load is not None and stripped.startswith("0x"):
            fields = (pending_load + " " + stripped).split()
            pending_load = None
            if len(fields) < 7:
                continue
            flags = "".join(fields[6:])
            if "R" in flags and ("E" in flags or "X" in flags):
                return int(fields[4], 16)
    raise RuntimeError("RX LOAD segment not found")


def patch_kernel(elf_path: Path, bin_path: Path) -> int:
    slot_addr = symbol_address(elf_path, CRC_SLOT_SYMBOL)
    end_addr = symbol_address(elf_path, CRC_END_SYMBOL)
    slot_offset = slot_addr - LOAD_VADDR
    region_end_offset = end_addr - LOAD_VADDR
    segment_size = first_rx_segment_size(elf_path)

    image = bytearray(bin_path.read_bytes())
    if slot_offset + 4 > region_end_offset:
        raise RuntimeError("CRC slot is outside integrity region")
    if region_end_offset > len(image):
        raise RuntimeError("integrity region end outside binary image")
    if segment_size > len(image):
        raise RuntimeError("RX segment size exceeds binary length")

    payload = image[:region_end_offset]
    crc_input = payload[:slot_offset] + payload[slot_offset + 4 : region_end_offset]
    digest = crc32_ieee(crc_input)

    struct.pack_into("<I", image, slot_offset, digest)
    bin_path.write_bytes(bytes(image))

    print(
        f"image CRC patched: 0x{digest:08x} "
        f"(slot=0x{slot_addr:08x}, region_bytes={region_end_offset}, rx_bytes={segment_size})"
    )
    return digest


def main() -> int:
    if len(sys.argv) != 3:
        print(f"usage: {sys.argv[0]} <kernel.elf> <kernel.bin>", file=sys.stderr)
        return 2

    elf_path = Path(sys.argv[1])
    bin_path = Path(sys.argv[2])

    if not elf_path.is_file():
        print(f"error: {elf_path} not found", file=sys.stderr)
        return 1
    if not bin_path.is_file():
        print(f"error: {bin_path} not found", file=sys.stderr)
        return 1

    patch_kernel(elf_path, bin_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())