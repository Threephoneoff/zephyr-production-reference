/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>

static const struct gpio_dt_spec debug_led =
	GPIO_DT_SPEC_GET(DT_ALIAS(debug_led), gpios);

static int set_debug_led(const struct shell *sh, int value)
{
	int ret;

	if (!gpio_is_ready_dt(&debug_led)) {
		shell_error(sh, "Debug LED GPIO device is not ready");
		return -ENODEV;
	}

	ret = gpio_pin_set_dt(&debug_led, value);
	if (ret < 0) {
		shell_error(sh, "Failed to set debug LED: %d", ret);
		return ret;
	}

	shell_print(sh, "Debug LED %s", value ? "on" : "off");
	return 0;
}

static int cmd_debug_led_on(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	return set_debug_led(sh, 1);
}

static int cmd_debug_led_off(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	return set_debug_led(sh, 0);
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_debug_led,
	SHELL_CMD_ARG(on, NULL, "Turn the debug LED on", cmd_debug_led_on, 1, 0),
	SHELL_CMD_ARG(off, NULL, "Turn the debug LED off", cmd_debug_led_off, 1, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(debug_led, &sub_debug_led, "Control the debug LED", NULL);
