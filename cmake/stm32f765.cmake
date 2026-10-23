# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only

# MCU-wide compile and link settings for the STM32F765VG (Cortex-M7, FPv5-D16).
#
# Every target links lxr_mcu so that all objects (application and bootloader)
# are built for the same ABI. The flags mirror the original STM32CubeIDE managed
# builds. Project-specific code-generation flags and the linker script are set
# by each project (see lxr_firmware_executable() in firmware_outputs.cmake).

include_guard(GLOBAL)

# The firmware and the bootloader each ship a copy of this file; when both are
# built together (LXR_BOOTLOADER_DIR) the first one loaded defines lxr_mcu.
if(TARGET lxr_mcu)
    return()
endif()

set(LXR_MCU_FLAGS
    -mcpu=cortex-m7
    -mthumb
    -mfpu=fpv5-d16
    -mfloat-abi=hard
)

add_library(lxr_mcu INTERFACE)

target_compile_definitions(lxr_mcu INTERFACE
    STM32F765xx
    USE_HAL_DRIVER
    $<$<CONFIG:Debug>:DEBUG>
)

target_compile_options(lxr_mcu INTERFACE
    ${LXR_MCU_FLAGS}
    --specs=nano.specs
    $<$<COMPILE_LANGUAGE:C>:-ffunction-sections -fdata-sections>
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_link_options(lxr_mcu INTERFACE
    ${LXR_MCU_FLAGS}
    --specs=nosys.specs
    --specs=nano.specs
    -static
    -Wl,--gc-sections
    -Wl,--print-memory-usage
)

target_link_libraries(lxr_mcu INTERFACE
    -Wl,--start-group c m -Wl,--end-group
)
