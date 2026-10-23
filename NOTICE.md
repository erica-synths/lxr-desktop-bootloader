# LXR firmware: copyright, licence and trademark notice

Copyright (C) 2012-2026 Erica Synths. All rights not expressly granted below are reserved.

## What this notice covers

This notice and the licence in [LICENSE](LICENSE) apply to:

- `LXR_V2_STM32F765/`: the LXR firmware
- `LXR_V2_STM32F765_Bootloader/`: the LXR bootloader
- `cmake/`: the build system and tools shared by both

Third-party components keep their own licences, listed [below](#third-party-components).

## Licence

The firmware, bootloader and build system are free software: you can redistribute and/or modify them
under the terms of the **GNU General Public License version 3** (GPL-3.0-only) as published by the
Free Software Foundation, with the additional permission and the additional terms below
(GPL-3.0 section 7). The full licence text is in [LICENSE](LICENSE).

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

### Additional permission under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or combining it with software from
STMicroelectronics that is distributed under STMicroelectronics licence terms, in particular the
"Ultimate Liberty" licence SLA0044 (or a modified version of that software), the licensors of this
Program grant you additional permission to convey the resulting work. This includes the STM32 USB
Device Library, the STM32CubeMX-generated code and the STMicroelectronics example code listed below.
Corresponding Source for a non-source form of such a combination shall include the source code for the
parts of that software used as well as that of the covered work.

### Additional terms under GNU GPL version 3 section 7

1. **Trademarks (section 7(e)).** This licence does not grant any rights under trademark law in the
   names "Erica Synths", "LXR" and "LXR-02" or in the associated logos. They may not be used for any
   product, modified firmware, service or other activity, commercial or not, in a way that suggests
   that it comes from, is approved by or is affiliated with Erica Synths, without prior written
   permission from Erica Synths. Truthful statements of compatibility, such as "custom firmware for
   the Erica Synths LXR-02", are allowed.
2. **Marking of modified versions (section 7(c)).** Modified versions must be marked in reasonable ways
   as different from the original version. At a minimum, the start-up screen of the device must show
   that the firmware is modified (for example a different name or version text), and a modified version
   must not be presented as official Erica Synths firmware.

## Hardware

The LXR hardware is the property of Erica Synths. That includes the electronic design (schematics,
printed circuit board layouts, bills of materials), the front panel, enclosure and industrial design, and
the product's appearance. It is protected by copyright, design rights and other intellectual property
rights. The hardware is **not** part of this repository and is **not** licensed under GPL-3.0 or any
other licence. It may not be copied, reproduced, manufactured or sold, in whole or in part, without
prior written permission from Erica Synths.

The GPL-3.0 licence of the firmware does not grant any rights to the hardware.

## Third-party components

These components are included in source form and are distributed under their own licences. The
bootloader contains its own copies of the HAL, CMSIS and FatFs files.

| Component | Location | Licence |
|---|---|---|
| STM32F7 HAL drivers (STMicroelectronics) | `Drivers/STM32F7xx_HAL_Driver/` | BSD-3-Clause, see the `LICENSE.txt` there |
| CMSIS core (Arm) | `Drivers/CMSIS/Include/` | Apache-2.0 |
| CMSIS STM32F7 device files, system and startup code (STMicroelectronics) | `Drivers/CMSIS/Device/ST/`, `Core/Src/system_stm32f7xx.c`, `Core/Startup/` | Apache-2.0 / BSD-3-Clause |
| STM32 USB Device Library, modified into a USB-MIDI class (STMicroelectronics) | `Middlewares/JS/STM32_USB_Device_Library/` | ST SLA0044 |
| STM32CubeMX-generated code with Erica Synths additions (STMicroelectronics) | `USB_DEVICE_MIDI/`, `FATFS/`, `Core/Src/stm32f7xx_it.c`, `Core/Src/stm32f7xx_hal_msp.c`, bootloader `Core/Src/main.c` | ST SLA0044 or BSD-3-Clause as stated in each file; Erica Synths additions under GPL-3.0 |
| newlib system call stubs (STMicroelectronics) | `Core/Src/syscalls.c`, `Core/Src/sysmem.c` | BSD-3-Clause style, as stated in the files |
| Flash programming helpers, from an STMicroelectronics example (LwIP IAP) | `Core/Src/flash_if.c`, `Core/Inc/flash_if.h` | STMicroelectronics example code, as stated in the files |
| FatFs R0.12c (ChaN) | `Middlewares/Third_Party/FatFs/` | FatFs licence (BSD-style, 1 clause) |
| HD44780 LCD routines, based on the mikrocontroller.net AVR-GCC tutorial | `Core/Src/lcd.c`, `Core/Inc/lcd.h` | as published on mikrocontroller.net |
