#!/usr/bin/env python3
# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only
"""Wrap a raw firmware binary into an LXR V2 SD-card update image.

The bootloader (LXR_V2_STM32F765_Bootloader) expects a 16 byte header followed
by the raw binary that gets programmed at 0x08008000:

    8 bytes  magic   ("LXRV2IMG" desktop, "LXRERIMG" Eurorack)
    4 bytes  size of the binary, little endian
    1 byte   checksum: 0xFF - (sum of all binary bytes & 0xFF)
    3 bytes  padding (zero)
    ...      binary

Same format as FirmwareImageTool/firmwareBuilder.py. The input must be the
objcopy -O binary output, not the ELF.
"""

import argparse
import struct
import sys

APP_FLASH_SIZE = 0x080FFFFF - 0x08008000  # limit checked by the bootloader


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--magic", default="LXRV2IMG", help="8 character image magic")
    parser.add_argument("input", help="raw firmware binary (.bin)")
    parser.add_argument("output", help="update image to write (.img)")
    args = parser.parse_args()

    magic = args.magic.encode("ascii")
    if len(magic) != 8:
        sys.exit("magic must be exactly 8 characters")

    with open(args.input, "rb") as f:
        payload = f.read()

    if payload[:4] == b"\x7fELF":
        sys.exit(f"{args.input} is an ELF file; pass the raw .bin produced by objcopy -O binary")
    if not payload or len(payload) >= APP_FLASH_SIZE:
        sys.exit(f"{args.input}: unexpected size {len(payload)} bytes")

    checksum = 0xFF - (sum(payload) & 0xFF)
    header = struct.pack("<8sIB3x", magic, len(payload), checksum)

    with open(args.output, "wb") as f:
        f.write(header)
        f.write(payload)

    print(f"{args.output}: {len(payload)} bytes, checksum 0x{checksum:02X}")


if __name__ == "__main__":
    main()
