/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS 500

static const struct gpio_dt_spec debug_led = GPIO_DT_SPEC_GET(DT_ALIAS(debug_led), gpios);
static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(DT_ALIAS(status_led), gpios);
static const struct gpio_dt_spec user_button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback button_cb_data;

static void button_pressed(const struct device *port, struct gpio_callback *cb,
			   gpio_port_pins_t pins)
{
	ARG_UNUSED(port);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	gpio_pin_toggle_dt(&debug_led);
	printk("Button pressed! Debug LED toggled.\n");
}

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&debug_led) || !gpio_is_ready_dt(&status_led) ||
	    !gpio_is_ready_dt(&user_button)) {
		printk("GPIO device is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&debug_led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Failed to configure debug LED: %d\n", ret);
		return 0;
	}

	ret = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Failed to configure status LED: %d\n", ret);
		return 0;
	}

	ret = gpio_pin_configure_dt(&user_button, GPIO_INPUT);
	if (ret < 0) {
		printk("Failed to configure user button: %d\n", ret);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&user_button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		printk("Failed to configure button interrupt: %d\n", ret);
		return 0;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(user_button.pin));
	ret = gpio_add_callback(user_button.port, &button_cb_data);
	if (ret < 0) {
		printk("Failed to add button callback: %d\n", ret);
		return 0;
	}

	printk("Zephyr Logger ready; press the button to toggle the debug LED\n");
	printk("Shell is available on USART1 at 115200 baud\n");

	while (1)
	{
		gpio_pin_toggle_dt(&status_led);
		k_msleep(SLEEP_TIME_MS);
	}
}
