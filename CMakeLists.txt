# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.21)

# LXR V2 bootloader (STM32F765VG, flash sector 0: 0x08000000-0x08007FFF).
#
# Holding the encoder button at power-up flashes an LXR*.img update image from
# the SD card to 0x08008000; otherwise it jumps to the application there.
#
# Standalone:
#   cmake --preset release            (desktop)
#   cmake --preset eurorack-release   (Eurorack module, EURORACK_MODULE=1)
#   cmake --build --preset release
#
# The application project (../LXR_V2_STM32F765) also builds this as a
# subdirectory to produce a combined bootloader + application image.

set(LXR_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/../cmake")
if(NOT CMAKE_TOOLCHAIN_FILE AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CMAKE_TOOLCHAIN_FILE "${LXR_CMAKE_DIR}/arm-none-eabi.cmake")
endif()

project(LXR_V2_STM32F765_Bootloader
    DESCRIPTION "LXR V2 SD-card bootloader"
    LANGUAGES C ASM
)

if(PROJECT_IS_TOP_LEVEL)
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug or Release)" FORCE)
    endif()
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

option(LXR_BOOTLOADER_EURORACK "Build the Eurorack module bootloader (accepts LXRE*.img / LXRERIMG images)" OFF)

# Same optimisation as the original STM32CubeIDE configurations. These are
# directory-scoped, so they also apply when built inside the application tree.
set(CMAKE_C_FLAGS_DEBUG     "-Og -g3")
set(CMAKE_C_FLAGS_RELEASE   "-Os -g0")
set(CMAKE_ASM_FLAGS_DEBUG   "-g3")
set(CMAKE_ASM_FLAGS_RELEASE "")

include(${LXR_CMAKE_DIR}/stm32f765.cmake)
include(${LXR_CMAKE_DIR}/firmware_outputs.cmake)

# --- Board configuration (main.h, stm32f7xx_hal_conf.h, ffconf.h, ...) --------
add_library(lxrboot_config INTERFACE)
target_include_directories(lxrboot_config INTERFACE
    Core/Inc
    FATFS/App
    FATFS/Target
)
target_link_libraries(lxrboot_config INTERFACE lxr_mcu)

# --- ST HAL (the bootloader's own CubeMX copy) ---------------------------------
add_library(lxrboot_hal OBJECT
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma_ex.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_exti.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c_ex.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sd.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_sdmmc.c
)
target_include_directories(lxrboot_hal PUBLIC
    Drivers/CMSIS/Include
    Drivers/CMSIS/Device/ST/STM32F7xx/Include
    Drivers/STM32F7xx_HAL_Driver/Inc
    Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
)
target_link_libraries(lxrboot_hal PUBLIC lxrboot_config)
target_compile_options(lxrboot_hal PRIVATE -w)

# --- FatFs + CubeMX SD glue -----------------------------------------------------
add_library(lxrboot_fatfs OBJECT
    Middlewares/Third_Party/FatFs/src/diskio.c
    Middlewares/Third_Party/FatFs/src/ff.c
    Middlewares/Third_Party/FatFs/src/ff_gen_drv.c
    Middlewares/Third_Party/FatFs/src/option/syscall.c
    FATFS/App/fatfs.c
    FATFS/Target/bsp_driver_sd.c
    FATFS/Target/fatfs_platform.c
    FATFS/Target/sd_diskio.c
)
target_include_directories(lxrboot_fatfs PUBLIC Middlewares/Third_Party/FatFs/src)
target_link_libraries(lxrboot_fatfs PUBLIC lxrboot_hal)
target_compile_options(lxrboot_fatfs PRIVATE -w)

# --- Bootloader -------------------------------------------------------------------
add_executable(lxr_bootloader
    Core/Startup/startup_stm32f765vgtx.s
    Core/Src/main.c
    Core/Src/delay.c
    Core/Src/flash_if.c
    Core/Src/lcd.c
    Core/Src/stm32f7xx_hal_msp.c
    Core/Src/stm32f7xx_it.c
    Core/Src/syscalls.c
    Core/Src/sysmem.c
    Core/Src/system_stm32f7xx.c
)
target_link_libraries(lxr_bootloader PRIVATE lxrboot_hal lxrboot_fatfs)
target_compile_options(lxr_bootloader PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall>)

if(LXR_BOOTLOADER_EURORACK)
    target_compile_definitions(lxr_bootloader PRIVATE EURORACK_MODULE=1)
    set_target_properties(lxr_bootloader PROPERTIES OUTPUT_NAME LXR_V2_Eurorack_Bootloader)
else()
    set_target_properties(lxr_bootloader PROPERTIES OUTPUT_NAME LXR_V2_STM32F765_Bootloader)
endif()

lxr_firmware_executable(lxr_bootloader LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F765VGTX_FLASH.ld)
lxr_add_firmware_outputs(lxr_bootloader)

if(PROJECT_IS_TOP_LEVEL)
    # Overwrites the bootloader on the device: asks for confirmation first.
    lxr_add_openocd_flash_target(flash_bootloader
        FILE $<TARGET_FILE:lxr_bootloader>
        DEPENDS lxr_bootloader
        OVERWRITES_BOOTLOADER
        COMMENT "Flashing the bootloader with OpenOCD"
    )
endif()
