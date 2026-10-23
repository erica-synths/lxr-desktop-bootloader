# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only

# Fallback for confirm_bootloader_overwrite.py when Python 3 is not installed.
# CMake cannot ask questions, so this only accepts the environment variable.
#   cmake -P confirm_bootloader_overwrite.cmake

set(var LXR_CONFIRM_BOOTLOADER_OVERWRITE)
string(TOLOWER "$ENV{${var}}" answer)
if(answer MATCHES "^(1|yes|y|true|on)$")
    message(STATUS "${var} is set: overwriting the bootloader without asking.")
    return()
endif()

message(FATAL_ERROR
    "\n"
    "WARNING: this target overwrites the bootloader (flash sector 0), which is what\n"
    "lets the device install firmware updates from the SD card. If programming is\n"
    "interrupted the device can only be recovered with a debug probe.\n"
    "\n"
    "To update just the firmware, use the \"flash\" target instead.\n"
    "\n"
    "Nothing was flashed. To really overwrite the bootloader, set the environment\n"
    "variable ${var}=yes and run the command again.\n")
