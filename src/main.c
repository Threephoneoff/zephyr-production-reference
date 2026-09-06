/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#ifdef CONFIG_MYFUNCTION
#include "myfunction.h"
#endif

#define SLEEP_TIME_MS 500

static const struct gpio_dt_spec debug_led = GPIO_DT_SPEC_GET(DT_ALIAS(debug_led), gpios);
static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(DT_ALIAS(status_led), gpios);

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&debug_led) || !gpio_is_ready_dt(&status_led)) {
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

	printk("Zephyr Logger ready; shell is available on USART1 at 115200 baud\n");

	while (1)
	{
		gpio_pin_toggle_dt(&debug_led);
		gpio_pin_toggle_dt(&status_led);
		printk("LED toggle \n");
		k_msleep(SLEEP_TIME_MS);
	}
}
