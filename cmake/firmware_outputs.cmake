# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only

# Firmware executable setup, post-build artefacts (.hex/.bin/.list, SD-card
# update image, combined bootloader+application image) and OpenOCD flash targets.

include_guard(GLOBAL)

# The firmware and the bootloader each ship a copy of this file. When the
# firmware build includes the bootloader sources (LXR_BOOTLOADER_DIR), the copy
# that was loaded first (the firmware's) stays in charge.
if(COMMAND lxr_add_firmware_outputs)
    return()
endif()

set(LXR_CMAKE_TOOLS_DIR "${CMAKE_CURRENT_LIST_DIR}/tools")

find_package(Python3 COMPONENTS Interpreter QUIET)
find_program(LXR_OPENOCD openocd)

set(LXR_GENERATE_LISTING OFF CACHE BOOL "Also write a disassembly listing (.list) after each build")
set(LXR_OPENOCD_INTERFACE "interface/stlink.cfg" CACHE STRING "OpenOCD interface config")
set(LXR_OPENOCD_TRANSPORT "" CACHE STRING "OpenOCD transport (empty = let OpenOCD choose, e.g. swd or hla_swd)")
# A 64 KB work area at the default SWD clock failed half-way through programming
# with an ST-Link V2 (firmware V2J29); these values (the work area from the old
# STM32CubeIDE OpenOCD config) program reliably.
set(LXR_OPENOCD_WORKAREA "0x8000" CACHE STRING "OpenOCD flash-loader work area size (WORKAREASIZE)")
set(LXR_OPENOCD_SPEED "1000" CACHE STRING "OpenOCD SWD clock in kHz")

# Base name of the files a firmware target produces (OUTPUT_NAME or the target name).
function(lxr_firmware_basename target out_var)
    get_target_property(name ${target} OUTPUT_NAME)
    if(NOT name)
        set(name ${target})
    endif()
    set(${out_var} ${name} PARENT_SCOPE)
endfunction()

# lxr_firmware_executable(<target> LINKER_SCRIPT <file>)
#
# Links <target> with the given linker script and writes a .map file next to it.
function(lxr_firmware_executable target)
    cmake_parse_arguments(ARG "" "LINKER_SCRIPT" "" ${ARGN})
    lxr_firmware_basename(${target} name)

    target_link_options(${target} PRIVATE
        -T${ARG_LINKER_SCRIPT}
        -Wl,-Map=$<TARGET_FILE_DIR:${target}>/${name}.map
    )
    set_target_properties(${target} PROPERTIES
        LINK_DEPENDS ${ARG_LINKER_SCRIPT}
        ADDITIONAL_CLEAN_FILES ${name}.map
    )
endfunction()

# lxr_add_firmware_outputs(<target> [IMAGE_MAGIC <8 chars> IMAGE_NAME <basename>])
#
# Writes <name>.hex, <name>.bin and prints the section sizes. With IMAGE_MAGIC and
# Python 3 available it also wraps the .bin into the bootloader's SD-card update
# image <IMAGE_NAME>.img (see tools/mkimage.py for the format).
function(lxr_add_firmware_outputs target)
    cmake_parse_arguments(ARG "" "IMAGE_MAGIC;IMAGE_NAME" "" ${ARGN})
    lxr_firmware_basename(${target} name)

    set(out_dir $<TARGET_FILE_DIR:${target}>)
    set(hex ${out_dir}/${name}.hex)
    set(bin ${out_dir}/${name}.bin)

    set(commands
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${target}> ${hex}
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${target}> ${bin}
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${target}>
    )
    set(byproducts ${name}.hex ${name}.bin)

    if(LXR_GENERATE_LISTING)
        list(APPEND commands
            COMMAND ${CMAKE_COMMAND} -DOBJDUMP=${CMAKE_OBJDUMP} -DELF=$<TARGET_FILE:${target}>
                    -DOUT=${out_dir}/${name}.list -P ${LXR_CMAKE_TOOLS_DIR}/listing.cmake
        )
        list(APPEND byproducts ${name}.list)
    endif()

    if(ARG_IMAGE_MAGIC AND ARG_IMAGE_NAME)
        if(Python3_EXECUTABLE)
            list(APPEND commands
                COMMAND ${Python3_EXECUTABLE} ${LXR_CMAKE_TOOLS_DIR}/mkimage.py
                        --magic ${ARG_IMAGE_MAGIC} ${bin} ${out_dir}/${ARG_IMAGE_NAME}.img
            )
            list(APPEND byproducts ${ARG_IMAGE_NAME}.img)
        else()
            message(WARNING "Python 3 not found: the SD-card update image (${ARG_IMAGE_NAME}.img) will not be generated.")
        endif()
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        ${commands}
        BYPRODUCTS ${byproducts}
        COMMENT "Generating firmware outputs for ${name}"
        VERBATIM
    )
endfunction()

# lxr_add_combined_image(<custom target> OUTPUT_NAME <basename>
#                        BOOTLOADER <firmware target or .hex file>
#                        APPLICATION <firmware target>
#                        BOOTLOADER_MAGIC <8 chars>)
#
# Merges bootloader and application into one image covering the whole flash:
# <basename>.hex and <basename>.bin (from 0x08000000, gaps filled with 0xFF), to
# program blank devices in one go. BOOTLOADER is either a target built in this
# project (set up with lxr_add_firmware_outputs) or a prebuilt .hex file. Before
# merging, tools/check_bootloader_hex.py checks that it really is a bootloader
# for this product (accepts BOOTLOADER_MAGIC update images); the build also fails
# if the images overlap. Built as part of "all".
function(lxr_add_combined_image name)
    cmake_parse_arguments(ARG "" "OUTPUT_NAME;BOOTLOADER;APPLICATION;BOOTLOADER_MAGIC" "" ${ARGN})
    if(NOT Python3_EXECUTABLE)
        message(WARNING "Python 3 not found: the combined image (${ARG_OUTPUT_NAME}.hex) will not be generated.")
        return()
    endif()

    set(depends)
    set(inputs)
    foreach(part IN ITEMS "${ARG_BOOTLOADER}" "${ARG_APPLICATION}")  # bootloader first: its entry point is used
        if(TARGET "${part}")
            lxr_firmware_basename(${part} part_name)
            get_target_property(part_dir ${part} BINARY_DIR)
            list(APPEND inputs ${part_dir}/${part_name}.hex)
            list(APPEND depends ${part})
        else()
            list(APPEND inputs "${part}")
        endif()
    endforeach()
    list(GET inputs 0 bootloader_hex)

    set(hex ${CMAKE_CURRENT_BINARY_DIR}/${ARG_OUTPUT_NAME}.hex)
    set(bin ${CMAKE_CURRENT_BINARY_DIR}/${ARG_OUTPUT_NAME}.bin)
    add_custom_command(
        OUTPUT ${hex} ${bin}
        COMMAND ${Python3_EXECUTABLE} ${LXR_CMAKE_TOOLS_DIR}/check_bootloader_hex.py
                --magic ${ARG_BOOTLOADER_MAGIC} ${bootloader_hex}
        COMMAND ${Python3_EXECUTABLE} ${LXR_CMAKE_TOOLS_DIR}/mergehex.py
                --base 0x08000000 --out-hex ${hex} --out-bin ${bin} ${inputs}
        DEPENDS ${depends} ${inputs}
                ${LXR_CMAKE_TOOLS_DIR}/mergehex.py ${LXR_CMAKE_TOOLS_DIR}/check_bootloader_hex.py
        COMMENT "Merging ${ARG_OUTPUT_NAME}.hex"
        VERBATIM
    )
    add_custom_target(${name} ALL DEPENDS ${hex} ${bin})
    set_target_properties(${name} PROPERTIES LXR_IMAGE_HEX ${hex})
endfunction()

# lxr_add_update_info(<custom target> IMAGE <path to .img> DEPENDS <targets>...)
#
# Prints where the SD-card update image is and how to install it at the end of
# every build (also when nothing had to be rebuilt). Does nothing without
# Python 3, because then no update image is generated.
function(lxr_add_update_info name)
    cmake_parse_arguments(ARG "" "IMAGE" "DEPENDS" ${ARGN})
    if(NOT Python3_EXECUTABLE)
        return()
    endif()

    file(TO_NATIVE_PATH "${ARG_IMAGE}" image)
    get_filename_component(image_name "${ARG_IMAGE}" NAME)
    set(lines
        ""
        "======================================================================"
        "  Firmware update file:"
        "    ${image}"
        ""
        "  To install it on the LXR:"
        "    1. Copy ${image_name} to the top folder of the SD card and"
        "       delete any older LXR*.img files from the card."
        "    2. Put the card in the LXR, hold down the encoder button and"
        "       switch the LXR on. Keep holding the button until"
        "       'LXR Updater' appears on the display."
        "    3. Wait for 'Update OK!'. The LXR then restarts by itself."
        "======================================================================"
    )
    set(commands)
    foreach(line IN LISTS lines)
        list(APPEND commands COMMAND ${CMAKE_COMMAND} -E echo "${line}")
    endforeach()

    add_custom_target(${name} ALL
        ${commands}
        DEPENDS ${ARG_DEPENDS}
        COMMENT "Firmware update instructions"
        VERBATIM
    )
endfunction()

# lxr_add_unavailable_target(<custom target> <line>...)
#
# Adds a target that can't do its job in this configuration: running it prints
# the given lines (what is missing and how to fix it) and fails, instead of
# Ninja's bare "unknown target" error.
function(lxr_add_unavailable_target name)
    # The text goes into a file so that the failing command Ninja echoes stays short.
    list(JOIN ARGN "\n" text)
    set(file ${CMAKE_CURRENT_BINARY_DIR}/${name}-not-available.txt)
    file(WRITE ${file} "\n${text}\n\n")
    add_custom_target(${name}
        COMMAND ${CMAKE_COMMAND} -E cat ${file}
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "${name} is not available"
        VERBATIM
    )
endfunction()

# lxr_add_openocd_flash_target(<custom target> FILE <.elf or .hex> DEPENDS <targets>...
#                              [OVERWRITES_BOOTLOADER] [COMMENT <text>])
#
# Programs FILE through an ST-Link with OpenOCD (connect under reset, like the
# former STM32CubeIDE debug configuration). Only the flash sectors covered by FILE are
# erased. Without OpenOCD the target only explains how to install it.
#
# OVERWRITES_BOOTLOADER marks targets that rewrite flash sector 0:
#  - the user is warned and has to confirm first (tools/confirm_bootloader_overwrite.py;
#    LXR_CONFIRM_BOOTLOADER_OVERWRITE=yes in the environment skips the question);
#  - the write protection of sector 0 is cleared before programming. The
#    bootloader write-protects its own sector on every boot (and re-protects it
#    after this), so a device that has run it before refuses the erase otherwise.
function(lxr_add_openocd_flash_target name)
    cmake_parse_arguments(ARG "OVERWRITES_BOOTLOADER" "FILE;COMMENT" "DEPENDS" ${ARGN})
    if(NOT LXR_OPENOCD)
        string(TOLOWER "${CMAKE_BUILD_TYPE}" preset)
        lxr_add_unavailable_target(${name}
            "'${name}' needs OpenOCD (and an ST-Link debug probe), but OpenOCD was not found."
            "Install it, then set up the build again with:  cmake --preset ${preset}"
            "  macOS:   brew install open-ocd"
            "  Linux:   sudo apt install openocd"
            "  Windows: see https://openocd.org (add its bin folder to PATH)"
            "Firmware updates don't need this: use the SD card update file (.img) instead."
        )
        return()
    endif()

    set(confirm)
    if(ARG_OVERWRITES_BOOTLOADER)
        if(Python3_EXECUTABLE)
            set(confirm COMMAND ${Python3_EXECUTABLE} ${LXR_CMAKE_TOOLS_DIR}/confirm_bootloader_overwrite.py
                                "the bootloader on the connected device")
        else()
            set(confirm COMMAND ${CMAKE_COMMAND} -P ${LXR_CMAKE_TOOLS_DIR}/confirm_bootloader_overwrite.cmake)
        endif()
    endif()

    set(transport)
    if(LXR_OPENOCD_TRANSPORT)
        set(transport -c "transport select ${LXR_OPENOCD_TRANSPORT}")
    endif()
    set(unprotect)
    if(ARG_OVERWRITES_BOOTLOADER)
        set(unprotect -c init -c "reset halt" -c "flash protect 0 0 0 off")
    endif()

    add_custom_target(${name}
        ${confirm}
        COMMAND ${LXR_OPENOCD}
            -c "set WORKAREASIZE ${LXR_OPENOCD_WORKAREA}"
            -f ${LXR_OPENOCD_INTERFACE}
            ${transport}
            -f target/stm32f7x.cfg
            -c "adapter speed ${LXR_OPENOCD_SPEED}"
            -c "reset_config srst_only srst_nogate connect_assert_srst"
            ${unprotect}
            -c "program ${ARG_FILE} verify reset exit"
        DEPENDS ${ARG_DEPENDS}
        COMMENT "${ARG_COMMENT}"
        USES_TERMINAL
        VERBATIM
    )
endfunction()
