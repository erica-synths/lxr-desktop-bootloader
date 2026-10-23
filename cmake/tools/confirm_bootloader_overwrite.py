#!/usr/bin/env python3
# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only
"""Ask for confirmation before a flash target overwrites the bootloader.

Exit status 0 = go ahead, 1 = abort (the build target then fails before
anything is written to the device).

Set the environment variable LXR_CONFIRM_BOOTLOADER_OVERWRITE=yes to skip the
question, e.g. in scripts or for factory programming.
"""

import os
import sys

ENV_VAR = "LXR_CONFIRM_BOOTLOADER_OVERWRITE"

WARNING = f"""
======================================================================
  WARNING: THIS WILL OVERWRITE THE BOOTLOADER
======================================================================

  The bootloader (flash sector 0, 0x08000000-0x08007FFF) is what lets
  the device install firmware updates from the SD card. This command
  removes its write protection, erases it and replaces it with the
  bootloader from this build.

  If programming is interrupted, or the new bootloader is broken, the
  device can no longer be updated from the SD card and can only be
  recovered with a debug probe (ST-Link).

  You only need this for:
    - blank devices that have never been programmed, or
    - deliberately updating the bootloader itself.

  To update just the firmware, use the "flash" target instead:
    cmake --build --preset release --target flash

  To skip this question (scripts, factory programming), set the
  environment variable {ENV_VAR}=yes
======================================================================
"""


def main():
    what = sys.argv[1] if len(sys.argv) > 1 else "the bootloader"

    if os.environ.get(ENV_VAR, "").strip().lower() in ("1", "yes", "y", "true", "on"):
        print(f"{ENV_VAR} is set: overwriting {what} without asking.", flush=True)
        return 0

    print(WARNING, flush=True)

    if not sys.stdin or not sys.stdin.isatty():
        print("No interactive terminal to ask for confirmation, so nothing was flashed.\n"
              f"Run the command again from a terminal, or set {ENV_VAR}=yes.", flush=True)
        return 1

    try:
        answer = input(f"Overwrite {what}? Type 'yes' to continue: ")
    except (EOFError, KeyboardInterrupt):
        answer = ""
    if answer.strip().lower() != "yes":
        print("Aborted, nothing was flashed.", flush=True)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
