# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only

# Writes a disassembly listing: cmake -DOBJDUMP=... -DELF=... -DOUT=... -P listing.cmake
execute_process(
    COMMAND ${OBJDUMP} -h -S ${ELF}
    OUTPUT_FILE ${OUT}
    RESULT_VARIABLE result
)
if(result)
    message(FATAL_ERROR "objdump failed: ${result}")
endif()
