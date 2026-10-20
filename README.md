# LXR V2 bootloader (STM32F765VG)

Lives in flash sector 0 (0x08000000–0x08007FFF, 32 KB).

- **Normal boot:** jumps to the application at 0x08008000, provided a valid application is there
  (its initial stack pointer is 0x20080000).
- **Update:** with the encoder button held at power-up, it reads an update image from the SD card
  (`LXR*.img`, or `LXRE*.img` for the Eurorack module), checks its header and checksum, programs it
  to 0x08008000 and resets.

Update images are produced by the application build (`LXR_V2_V<version>.img`).

## Building

To program blank devices, the firmware build can merge the bootloader with the firmware into one
full-flash image (`LXR_V2_V<version>_full.hex`) and a `flash_full` target. Give it either the
`LXR_V2_STM32F765_Bootloader.hex` built here (`-DLXR_BOOTLOADER_HEX=...`) or this source folder
(`-DLXR_BOOTLOADER_DIR=...`); see the firmware README.

Building the bootloader on its own needs the same tools as the firmware (CMake ≥ 3.21, Ninja, arm-none-eabi-gcc):

```sh
cmake --preset release              # desktop
cmake --preset eurorack-release     # Eurorack module (EURORACK_MODULE=1)
cmake --build --preset release
cmake --build --preset release --target flash_bootloader   # program the bootloader only (OpenOCD + ST-Link)
```

`flash_bootloader` warns and asks you to type `yes` before it erases the bootloader on the device
(set `LXR_CONFIRM_BOOTLOADER_OVERWRITE=yes` to skip the question in scripts). If programming is
interrupted, the device can no longer update itself from the SD card and has to be recovered with
the debug probe.

| Preset             | Flags                        | Output                                  |
|--------------------|------------------------------|-----------------------------------------|
| `release`          | `-Os`                        | `LXR_V2_STM32F765_Bootloader.{elf,hex,bin,map}` |
| `eurorack-release` | `-Os`, `EURORACK_MODULE=1`   | `LXR_V2_Eurorack_Bootloader.{elf,hex,bin,map}`  |
| `debug`            | `-Og -g3`, `DEBUG` defined   | `LXR_V2_STM32F765_Bootloader.*`         |

The toolchain file and shared CMake modules are in the repository's top-level `cmake/` folder.

### Cleaning the build

Remove the compiled files but keep the settings:

```sh
cmake --build --preset release --target clean
```

To start completely from scratch, also forgetting the settings, delete the `build/` folder
(`rm -rf build`, or `Remove-Item -Recurse -Force build` in PowerShell) and run `cmake --preset release`
again. Do this after switching to a different compiler. See "Cleaning the build" in the firmware README
for details.

## Licence

Copyright (C) 2012-2026 Erica Synths. Licensed under the [GNU General Public License version 3](../LICENSE)
with the additional permission and terms in [NOTICE.md](../NOTICE.md). The names "Erica Synths" and "LXR"
and the LXR hardware are the property of Erica Synths and are not covered by this licence; see
[NOTICE.md](../NOTICE.md).
