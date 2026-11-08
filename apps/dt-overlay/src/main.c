/*
 * Device Tree の overlay で宣言した LED と I2C を確かめるサンプル
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#define LED0_NODE DT_ALIAS(led0)
#define I2C_NODE  DT_NODELABEL(i2c0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct device *const i2c_dev = DEVICE_DT_GET(I2C_NODE);

static void show_i2c(void)
{
	uint32_t cfg;
	int found = 0;

	/* overlay に書いた値は、ビルド時にマクロとして取り出せる */
	printf("i2c0: clock-frequency (DT) = %d Hz\n",
	       DT_PROP(I2C_NODE, clock_frequency));

	if (!device_is_ready(i2c_dev)) {
		printf("i2c0: not ready\n");
		return;
	}

	/* ドライバが実際に設定した速度 */
	if (i2c_get_config(i2c_dev, &cfg) == 0) {
		printf("i2c0: speed (driver) = %s\n",
		       I2C_SPEED_GET(cfg) == I2C_SPEED_FAST ? "fast (400 kHz)" :
		       I2C_SPEED_GET(cfg) == I2C_SPEED_STANDARD ? "standard (100 kHz)" :
		       "other");
	}

	/* 0x08〜0x77 に1バイト読みを試し、応答したアドレスを出す */
	for (uint16_t addr = 0x08; addr <= 0x77; addr++) {
		uint8_t dummy;

		if (i2c_read(i2c_dev, &dummy, 1, addr) == 0) {
			printf("i2c0: found device at 0x%02x\n", addr);
			found++;
		}
	}
	printf("i2c0: scan done, %d device(s)\n", found);
}

int main(void)
{
	bool led_state = true;

	show_i2c();

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
		printf("LED state: %s\n", led_state ? "ON" : "OFF");
		k_msleep(1000);
	}
	return 0;
}
