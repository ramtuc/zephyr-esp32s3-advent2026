/*
 * Sensor API (Fetch/Get) でセンサを読むサンプル
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

/* Device Tree の別名 ambient-temp0 が指すセンサ (型番は overlay で決まる) */
static const struct device *const dev = DEVICE_DT_GET(DT_ALIAS(ambient_temp0));

static void print_channel(const char *name, enum sensor_channel chan, const char *unit)
{
	struct sensor_value val;
	int rc = sensor_channel_get(dev, chan, &val);

	if (rc < 0) {
		printk("  %-8s: not available (%d)\n", name, rc);
		return;
	}

	/* val1 は整数部、val2 は小数部 (100 万分の 1 単位) */
	printk("  %-8s: val1=%d val2=%d -> %.2f %s\n", name, val.val1, val.val2,
	       sensor_value_to_double(&val), unit);
}

int main(void)
{
	if (!device_is_ready(dev)) {
		printk("sensor %s is not ready\n", dev->name);
		return 0;
	}
	printk("sensor: %s\n", dev->name);

	while (1) {
		uint32_t start = k_cycle_get_32();
		int rc = sensor_sample_fetch(dev);
		uint32_t us = k_cyc_to_us_floor32(k_cycle_get_32() - start);

		if (rc < 0) {
			printk("sensor_sample_fetch failed: %d\n", rc);
		} else {
			printk("fetch: %u us\n", us);
			print_channel("temp", SENSOR_CHAN_AMBIENT_TEMP, "C");
			print_channel("press", SENSOR_CHAN_PRESS, "kPa");
			print_channel("humidity", SENSOR_CHAN_HUMIDITY, "%RH");
		}
		k_msleep(2000);
	}
	return 0;
}
