# Creating the Zephyr Logger custom board

## Motivation

An application overlay changes an existing board. A custom board definition describes
your hardware itself: its MCU, clocks, memory, enabled peripherals, pin routing, and
default Kconfig features. Once the board exists, applications can target it with
`-b zephyr_logger` without pretending that the hardware is an STM32H7B3I Discovery kit.

This first board revision intentionally contains only:

- two GPIO outputs on PG11 and PG2;
- USART1 TX/RX on PA9 and PA10;
- the UART console and Zephyr shell;
- the STM32H7B3XXQ MCU, internal flash, and SRAM.

## Directory layout

```text
boards/custom/zephyr_logger/
├── board.yml
├── board.cmake
├── Kconfig.defconfig
├── Kconfig.zephyr_logger
├── zephyr_logger.dts
├── zephyr_logger.yaml
└── zephyr_logger_defconfig
```

`CMakeLists.txt` adds the application directory to `BOARD_ROOT` before loading
Zephyr. This makes the local `boards/` directory discoverable:

```cmake
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
```

## What each file does

- `board.yml` declares the board name, vendor, and supported SoC qualifier.
- `zephyr_logger.dts` describes the physical hardware and enabled peripherals.
- `zephyr_logger_defconfig` enables drivers that are normally available on this board.
- `Kconfig.zephyr_logger` selects the exact STM32H7B3XXQ SoC variant.
- `zephyr_logger.yaml` provides test-runner metadata and supported features.
- `board.cmake` declares supported flash/debug runners.
- `Kconfig.defconfig` is available for board-specific Kconfig defaults as the board grows.

## Devicetree design

The SoC and pinctrl includes provide the STM32H7B3 peripheral nodes and valid pin
functions. The board DTS then enables only the hardware that is actually present.

The two outputs use the same pins as the Discovery kit LEDs:

```dts
debug_led: led_0 {
	gpios = <&gpiog 11 GPIO_ACTIVE_HIGH>;
};

status_led: led_1 {
	gpios = <&gpiog 2 GPIO_ACTIVE_HIGH>;
};
```

They are modeled with `gpio-leds` because they drive LEDs in this example. If the
same pins later control unrelated signals, use a binding that represents that
hardware instead of describing every output as an LED.

USART1 is routed to the same pins as the Discovery kit:

```dts
&usart1 {
	pinctrl-0 = <&usart1_tx_pa9 &usart1_rx_pa10>;
	pinctrl-names = "default";
	current-speed = <115200>;
	status = "okay";
};
```

The `chosen` properties make it the console and shell transport:

```dts
chosen {
	zephyr,console = &usart1;
	zephyr,shell-uart = &usart1;
};
```

`CONFIG_SHELL=y` and `CONFIG_SHELL_BACKEND_SERIAL=y` enable the corresponding
software in `prj.conf`.

## Build and connect

From the west workspace root:

```bash
.venv/bin/west build -p always -b zephyr_logger \
  zephyr-production-reference -d build-zephyr-logger
```

Connect a 3.3 V USB-to-UART adapter with common ground:

- adapter RX to MCU PA9 (USART1 TX);
- adapter TX to MCU PA10 (USART1 RX);
- adapter GND to board GND.

Do not connect a 5 V UART signal directly to the MCU pins. On macOS, locate and
open the adapter at 115200 baud:

```bash
ls /dev/cu.*
screen /dev/cu.usbserial-DEVICE 115200
```

Press Enter to display the shell prompt. Exit `screen` with `Ctrl-A`, then
`Ctrl-\\`, then confirm with `y`.

## Verify generated configuration

The generated Devicetree is the best way to confirm what Zephyr actually used:

```bash
rg -n "zephyr,console|zephyr,shell-uart|current-speed|debug-led|status-led" \
  build-zephyr-logger/zephyr/zephyr.dts
```

The final Kconfig selection can be checked with:

```bash
rg "CONFIG_(BOARD|SOC|SHELL|UART_CONSOLE|GPIO)" \
  build-zephyr-logger/zephyr/.config
```

## Common mistakes

- Putting `BOARD_ROOT` after `find_package(Zephyr)`. Board discovery has already
  happened by then.
- Using the display name `zephyr-logger` as the build target. The board target is
  the identifier `zephyr_logger`.
- Copying the entire Discovery DTS. That incorrectly claims your custom board has
  its display, SDRAM, camera, SD card, CAN transceiver, and other peripherals.
- Using `debug-led` inside `DT_ALIAS()` in C. Devicetree hyphens become underscores,
  so the C form is `DT_ALIAS(debug_led)`.
- Forgetting that `chosen` selects a device but does not enable the shell software;
  both the DTS and Kconfig sides are required.

## Hardware assumptions to verify

This first revision copies two board-level assumptions from the STM32H7B3I
Discovery kit because the custom schematic details are not yet available:

- a 24 MHz high-speed external oscillator (`clk_hse`);
- direct SMPS power mode (`power-supply = "smps-direct"`).

These settings are not determined solely by using the same MCU. Confirm them
against the oscillator and power connections on the Zephyr Logger schematic before
flashing. If either differs, update the clock tree or power setting first. Also
confirm that PG11/PG2 LEDs are active-high and have appropriate current-limiting
resistors.

## Production perspective

Keep the board DTS limited to hardware that exists on every unit of the board.
Use application overlays for product variants or temporary experiments. As the
board grows, add peripherals one at a time and verify the generated DTS, Kconfig,
pin conflicts, clock source, and electrical behavior after each addition.

## References

- Zephyr board definition: `zephyr/boards/st/stm32h7b3i_dk/`
- STM32H7B3 SoC definition: `zephyr/dts/arm/st/h7/stm32h7b3Xi.dtsi`
- STM32H7B3 pinctrl definitions: `zephyr/dts/arm/st/h7/stm32h7b3lihxq-pinctrl.dtsi`
