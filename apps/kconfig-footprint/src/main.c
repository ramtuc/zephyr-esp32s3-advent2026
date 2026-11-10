/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Kconfig でフットプリントを削る記事のアプリ。
 * LED を1秒ごとに点滅させ、状態をログに出すだけ。
 * ソースは変えずに、prj*.conf だけを変えて大きさを比べる。
 */
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int main(void)
{
	bool led_state = true;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}
	if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
		return 0;
	}

	while (1) {
		if (gpio_pin_toggle_dt(&led) < 0) {
			return 0;
		}
		led_state = !led_state;
		LOG_INF("LED state: %s", led_state ? "ON" : "OFF");
		k_msleep(1000);
	}
	return 0;
}
