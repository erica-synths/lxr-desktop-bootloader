# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only

# Cross-compilation toolchain file for the GNU Arm Embedded toolchain.
#
# The compiler is taken from ARM_TOOLCHAIN_DIR (cache variable or environment
# variable, pointing at the toolchain's bin/ directory) or, if unset, from PATH.
#
#   cmake --preset release
#   cmake --preset release -DARM_TOOLCHAIN_DIR=C:/toolchain_arm-none-eabi/11.3rel1/bin

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Bare-metal: the compiler check must not try to link a host executable.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT ARM_TOOLCHAIN_DIR AND DEFINED ENV{ARM_TOOLCHAIN_DIR})
    set(ARM_TOOLCHAIN_DIR "$ENV{ARM_TOOLCHAIN_DIR}")
endif()
set(ARM_TOOLCHAIN_DIR "${ARM_TOOLCHAIN_DIR}" CACHE PATH "bin/ directory of the arm-none-eabi toolchain (empty = use PATH)")
# Make the value visible inside try_compile() projects as well.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES ARM_TOOLCHAIN_DIR)

if(ARM_TOOLCHAIN_DIR)
    file(TO_CMAKE_PATH "${ARM_TOOLCHAIN_DIR}" _arm_dir)
    set(_arm_prefix "${_arm_dir}/arm-none-eabi-")
else()
    set(_arm_prefix "arm-none-eabi-")
endif()

if(CMAKE_HOST_WIN32)
    set(_arm_exe ".exe")
else()
    set(_arm_exe "")
endif()

set(CMAKE_C_COMPILER   "${_arm_prefix}gcc${_arm_exe}")
set(CMAKE_ASM_COMPILER "${_arm_prefix}gcc${_arm_exe}")
set(CMAKE_OBJCOPY      "${_arm_prefix}objcopy${_arm_exe}" CACHE FILEPATH "" FORCE)
set(CMAKE_OBJDUMP      "${_arm_prefix}objdump${_arm_exe}" CACHE FILEPATH "" FORCE)
set(CMAKE_SIZE         "${_arm_prefix}size${_arm_exe}"    CACHE FILEPATH "" FORCE)
set(CMAKE_NM           "${_arm_prefix}nm${_arm_exe}"      CACHE FILEPATH "" FORCE)

set(CMAKE_EXECUTABLE_SUFFIX_C   ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
