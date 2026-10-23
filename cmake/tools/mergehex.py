#!/usr/bin/env python3
# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only
"""Merge Intel HEX files (e.g. bootloader + application) into one image.

Writes a merged .hex and, optionally, a flat .bin starting at --base with gaps
filled with 0xFF (erased flash). Fails if two inputs write the same address.
The start address record (entry point) is taken from the first input, i.e.
list the bootloader first so a debugger starts at the bootloader's reset handler.

    mergehex.py --base 0x08000000 --out-hex full.hex --out-bin full.bin boot.hex app.hex
"""

import argparse
import sys


def read_hex(path):
    """Returns ({address: byte}, start_address or None)."""
    data = {}
    start = None
    upper = 0
    with open(path, "r") as f:
        for lineno, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            if not line.startswith(":"):
                sys.exit(f"{path}:{lineno}: not an Intel HEX record")
            rec = bytes.fromhex(line[1:])
            if sum(rec) & 0xFF:
                sys.exit(f"{path}:{lineno}: bad record checksum")
            count, addr, rtype, payload = rec[0], (rec[1] << 8) | rec[2], rec[3], rec[4:-1]
            if len(payload) != count:
                sys.exit(f"{path}:{lineno}: bad record length")
            if rtype == 0x00:
                base = upper + addr
                for i, b in enumerate(payload):
                    if base + i in data:
                        sys.exit(f"{path}:{lineno}: address 0x{base + i:08X} written twice")
                    data[base + i] = b
            elif rtype == 0x01:
                break
            elif rtype == 0x02:
                upper = int.from_bytes(payload, "big") << 4
            elif rtype == 0x04:
                upper = int.from_bytes(payload, "big") << 16
            elif rtype in (0x03, 0x05):
                start = (rtype, payload)
            else:
                sys.exit(f"{path}:{lineno}: unsupported record type {rtype:02X}")
    return data, start


def record(rtype, addr, payload):
    rec = bytes([len(payload), (addr >> 8) & 0xFF, addr & 0xFF, rtype]) + bytes(payload)
    return ":" + (rec + bytes([(-sum(rec)) & 0xFF])).hex().upper() + "\n"


def write_hex(path, data, start):
    lines = []
    upper = None
    addrs = sorted(data)
    i = 0
    while i < len(addrs):
        addr = addrs[i]
        if addr >> 16 != upper:
            upper = addr >> 16
            lines.append(record(0x04, 0, upper.to_bytes(2, "big")))
        # up to 16 consecutive bytes that stay within the same 64 KB segment
        chunk = [data[addr]]
        while (i + len(chunk) < len(addrs) and len(chunk) < 16
               and addrs[i + len(chunk)] == addr + len(chunk)
               and (addr + len(chunk)) >> 16 == upper):
            chunk.append(data[addr + len(chunk)])
        lines.append(record(0x00, addr & 0xFFFF, chunk))
        i += len(chunk)
    if start:
        lines.append(record(start[0], 0, start[1]))
    lines.append(record(0x01, 0, b""))
    with open(path, "w", newline="\n") as f:
        f.writelines(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("inputs", nargs="+", help="Intel HEX files to merge")
    parser.add_argument("--out-hex", required=True, help="merged Intel HEX file")
    parser.add_argument("--out-bin", help="merged flat binary (from --base, gaps = 0xFF)")
    parser.add_argument("--base", type=lambda s: int(s, 0), default=0x08000000, help="start address of --out-bin")
    args = parser.parse_args()

    merged = {}
    start = None
    owner = {}
    for path in args.inputs:
        data, file_start = read_hex(path)
        clash = merged.keys() & data.keys()
        if clash:
            first = min(clash)
            sys.exit(f"{path} overlaps {owner[first]} at 0x{first:08X} ({len(clash)} bytes)")
        merged.update(data)
        owner.update(dict.fromkeys(data, path))
        if start is None:
            start = file_start
        lo, hi = min(data), max(data)
        print(f"{path}: 0x{lo:08X}-0x{hi:08X} ({len(data)} bytes)")

    write_hex(args.out_hex, merged, start)

    if args.out_bin:
        if min(merged) < args.base:
            sys.exit(f"data below --base 0x{args.base:08X}")
        image = bytearray(b"\xFF" * (max(merged) - args.base + 1))
        for addr, b in merged.items():
            image[addr - args.base] = b
        with open(args.out_bin, "wb") as f:
            f.write(image)

    print(f"{args.out_hex}: 0x{min(merged):08X}-0x{max(merged):08X}")


if __name__ == "__main__":
    main()
