# Phase 3: microSD card and FAT filesystem

## Motivation

The logger needs persistent, removable storage. The STM32H7B3I-DK connects its
microSD socket directly to the MCU's SDMMC1 peripheral, so Zephyr can expose the
card as a block device and mount a FAT filesystem on top of it.

The same wiring is now part of the `zephyr_logger` board definition:

| Signal | MCU pin |
| --- | --- |
| SDMMC1_D0 | PC8 |
| SDMMC1_D1 | PC9 |
| SDMMC1_D2 | PC10 |
| SDMMC1_D3 | PC11 |
| SDMMC1_CK | PC12 |
| SDMMC1_CMD | PD2 |
| Card detect | PI8, active low |

SDMMC1 also requires a 48 MHz kernel clock. The system PLL1 output used by this
board is 80 MHz and therefore cannot drive SDMMC1 directly. PLL2 is configured
with a dedicated 48 MHz R output and selected as the SDMMC1 clock source. If this
is omitted, the STM32 driver rejects initialization with `-ENOTSUP` (`-134`).

The ST schematic includes pull-ups and protection components around the socket.
Those are hardware details; Devicetree describes only the connections needed by
the driver. On a new custom PCB, copy the circuit requirements as well as the MCU
pin assignments.

## Software layers

The SDMMC1 driver presents a disk named `SD`. FatFs consumes that disk and Zephyr's
filesystem API mounts it at `/SD:`. `storage_mount()` initializes the card, prints
its capacity, mounts it, and lists the root directory.

The firmware intentionally does not format a card when mounting fails. Automatic
formatting is convenient in a demo but can destroy data in a production logger.
Use a FAT32-formatted card for the first test.

The filesystem shell is enabled, so after a successful mount useful commands
include:

```text
fs ls /SD:
fs read /SD:/example.txt
```

Run `fs help` to see the commands supported by this Zephyr version.

## Build and flash

```sh
source ../.venv/bin/activate
west build -b zephyr_logger -d ../build-zephyr-logger --pristine
west flash -d ../build-zephyr-logger
```

Insert the card before reset. The USART1 console reports card capacity, mount
status, and the root-directory contents. The rest of the application continues
running if no card is present or mounting fails.

## Common mistakes

- The card is exFAT instead of FAT32. Large cards are commonly sold as exFAT.
- The custom PCB does not include the required SD signal pull-ups.
- Card-detect polarity is wrong. The Discovery kit drives PI8 low when inserted.
- The SDMMC kernel clock is not 48 MHz. In this project PLL2_R supplies it.
- A camera is connected at the same time on the Discovery kit. PC9 and PC11 are
  shared with camera signals there.
- Files are removed without closing or syncing them first. Call `fs_close()` and
  unmount before power removal when possible.

## Production perspective

Boot-time mounting is enough for this learning phase. A production design should
also handle insertion/removal events, serialize filesystem access, periodically
sync important data, use temporary files or journaling for critical records, and
define behavior for full or corrupted media.

## References

- STMicroelectronics, STM32H7B3I-DK schematic pack, microSD card sheet
- STMicroelectronics, STM32H7B3I-DK user manual, microSD card section
- Zephyr `stm32h7b3i_dk.dts` board definition
- Zephyr filesystem and FAT filesystem APIs
