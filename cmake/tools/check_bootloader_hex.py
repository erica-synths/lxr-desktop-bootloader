#!/usr/bin/env python3
# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only
"""Check that an Intel HEX file is an LXR bootloader before it is merged into a
full-flash image.

    check_bootloader_hex.py --magic LXRV2IMG bootloader.hex

Fails (exit status 1, with an explanation) unless the file:
  - only contains data inside the bootloader sector 0x08000000-0x08007FFF,
  - starts with a plausible Cortex-M vector table (stack pointer in RAM, reset
    handler inside the bootloader sector),
  - accepts update images with the given magic (LXRV2IMG = desktop LXR,
    LXRERIMG = Eurorack module), i.e. is the bootloader for the right product.
"""

import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mergehex import read_hex  # noqa: E402

SECTOR_START = 0x08000000
SECTOR_END = 0x08008000          # exclusive; the application starts here
RAM_START, RAM_END = 0x20000000, 0x20080000
KNOWN_MAGICS = {b"LXRV2IMG": "desktop LXR", b"LXRERIMG": "Eurorack LXR module"}


def fail(path, message):
    sys.exit(f"{path}: not a usable LXR bootloader: {message}")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--magic", required=True, help="update-image magic the bootloader must accept")
    parser.add_argument("hex", help="bootloader Intel HEX file")
    args = parser.parse_args()

    if not os.path.isfile(args.hex):
        fail(args.hex, "file not found")
    data, _ = read_hex(args.hex)
    if not data:
        fail(args.hex, "the file contains no data")

    lo, hi = min(data), max(data)
    if lo < SECTOR_START or hi >= SECTOR_END:
        fail(args.hex, f"it covers 0x{lo:08X}-0x{hi:08X}, but a bootloader must lie entirely inside "
                       f"0x{SECTOR_START:08X}-0x{SECTOR_END - 1:08X}. Is this the application or a "
                       f"full image instead of the bootloader?")

    def word(addr):
        if any(addr + i not in data for i in range(4)):
            fail(args.hex, f"no vector table at 0x{addr:08X}")
        return int.from_bytes(bytes(data[addr + i] for i in range(4)), "little")

    sp, reset = word(SECTOR_START), word(SECTOR_START + 4)
    if not (RAM_START < sp <= RAM_END) or sp & 3:
        fail(args.hex, f"initial stack pointer 0x{sp:08X} is not in RAM")
    if not (reset & 1) or not (SECTOR_START <= (reset & ~1) < SECTOR_END):
        fail(args.hex, f"reset handler 0x{reset:08X} is not inside the bootloader sector")

    image = bytes(data[a] for a in sorted(data))
    expected = args.magic.encode("ascii")
    found = [m for m in KNOWN_MAGICS if m in image]
    if expected not in image:
        if found:
            fail(args.hex, f"this is the bootloader for the {KNOWN_MAGICS[found[0]]} "
                           f"(it accepts {found[0].decode()} images), not for this firmware "
                           f"({expected.decode()} images)")
        fail(args.hex, f"it does not accept {expected.decode()} update images")

    print(f"{args.hex}: bootloader OK ({KNOWN_MAGICS.get(expected, expected.decode())}, "
          f"{len(data)} bytes, 0x{lo:08X}-0x{hi:08X})")


if __name__ == "__main__":
    main()
