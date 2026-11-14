/*
 * Sensor API (Read/Decode) でセンサを読むサンプル
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/dsp/print_format.h>

/* Device Tree の別名 ambient-temp0 が指すセンサ (型番は overlay で決まる) */
#define SENSOR_NODE DT_ALIAS(ambient_temp0)

static const struct device *const dev = DEVICE_DT_GET(SENSOR_NODE);

/* 読むチャンネルの組を、ビルド時に I/O デバイスとして定義する */
SENSOR_DT_READ_IODEV(iodev, SENSOR_NODE,
		     {SENSOR_CHAN_AMBIENT_TEMP, 0},
		     {SENSOR_CHAN_PRESS, 0},
		     {SENSOR_CHAN_HUMIDITY, 0});

/* 要求 (SQ) と完了 (CQ) のキューを 1 つずつ持つ RTIO のコンテキスト */
RTIO_DEFINE(ctx, 1, 1);

static void print_channel(const struct sensor_decoder_api *decoder, const uint8_t *buf,
			  const char *name, enum sensor_channel chan, const char *unit)
{
	struct sensor_q31_data data = {0};
	uint32_t fit = 0;
	int n = decoder->decode(buf, (struct sensor_chan_spec){chan, 0}, &fit, 1, &data);

	if (n <= 0) {
		printk("  %-8s: not available (%d)\n", name, n);
		return;
	}

	/* 値は q31 (固定小数点) と shift の組。値 = value / 2^31 * 2^shift */
	printk("  %-8s: value=%d shift=%d -> %" PRIq(2) " %s\n", name,
	       data.readings[0].value, data.shift,
	       PRIq_arg(data.readings[0].value, 2, data.shift), unit);
}

int main(void)
{
	if (!device_is_ready(dev)) {
		printk("sensor %s is not ready\n", dev->name);
		return 0;
	}
	printk("sensor: %s\n", dev->name);

	const struct sensor_decoder_api *decoder;
	int rc = sensor_get_decoder(dev, &decoder);

	if (rc < 0) {
		printk("sensor_get_decoder failed: %d\n", rc);
		return 0;
	}

	while (1) {
		uint8_t buf[64];
		uint32_t start = k_cycle_get_32();

		/* 読み取りを要求し、終わるまで待つ。buf にはセンサ固有の形のデータが入る */
		rc = sensor_read(&iodev, &ctx, buf, sizeof(buf));

		uint32_t us = k_cyc_to_us_floor32(k_cycle_get_32() - start);

		if (rc < 0) {
			printk("sensor_read failed: %d\n", rc);
		} else {
			printk("read: %u us\n", us);
			print_channel(decoder, buf, "temp", SENSOR_CHAN_AMBIENT_TEMP, "C");
			print_channel(decoder, buf, "press", SENSOR_CHAN_PRESS, "kPa");
			print_channel(decoder, buf, "humidity", SENSOR_CHAN_HUMIDITY, "%RH");
		}
		k_msleep(2000);
	}
	return 0;
}
