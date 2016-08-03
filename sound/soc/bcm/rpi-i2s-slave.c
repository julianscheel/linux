/*
 * ASoC Driver for RPi as I2S slave with generic codec
 *
 * Author: Julian Scheel <julian@jusst.de>
 * Copyright 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/jack.h>

static int snd_rpi_i2s_slave_init(struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

static struct snd_soc_dai_link snd_rpi_i2s_slave_dai[] = {
	{
		.name = "NVT-1212 I2S DAC",
		.stream_name = "NVT-1212 I2S HiFi",
		.cpu_dai_name = "bcm2708-i2s.0",
		.platform_name = "bcm2708-i2s.0",
		.codec_dai_name = "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
			SND_SOC_DAIFMT_CBS_CFS,
		.init = snd_rpi_i2s_slave_init,
		.playback_only = false,
		.capture_only = false,
	},
};

static struct snd_soc_card snd_rpi_i2s_slave = {
	.name = "snd_rpi_i2s_slave_dai",
	.dai_link = snd_rpi_i2s_slave_dai,
	.num_links = 1,
};

static int snd_rpi_i2s_slave_probe(struct platform_device *pdev)
{
	int ret = 0;

	snd_rpi_i2s_slave.dev = &pdev->dev;

	if (pdev->dev.of_node) {
		struct device_node *i2s_node;
		struct snd_soc_dai_link *dai = &snd_rpi_i2s_slave_dai[0];
		printk("%s(): Check of_node\n", __func__);
		i2s_node = of_parse_phandle(pdev->dev.of_node, "i2s-controller", 0);

		if (i2s_node) {
			printk("%s(): have i2s node\n", __func__);
			dai->cpu_dai_name = NULL;
			dai->cpu_of_node = i2s_node;
			dai->platform_name = NULL;
			dai->platform_of_node = i2s_node;
		}
	}

	ret = snd_soc_register_card(&snd_rpi_i2s_slave);
	if (ret)
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n", ret);

	return ret;
}

static int snd_rpi_i2s_slave_remove(struct platform_device *pdev)
{
	return snd_soc_unregister_card(&snd_rpi_i2s_slave);
}

static const struct of_device_id snd_rpi_i2s_slave_of_match[] = {
	{ .compatible = "rpi,i2s-slave", },
	{},
};
MODULE_DEVICE_TABLE(of, snd_rpi_i2s_slave_of_match);

static struct platform_driver snd_rpi_i2s_slave_driver = {
	.driver = {
		.name = "snd-rpi-i2s-slave",
		.owner = THIS_MODULE,
		.of_match_table = snd_rpi_i2s_slave_of_match,
	},
	.probe = snd_rpi_i2s_slave_probe,
	.remove = snd_rpi_i2s_slave_remove,
};

module_platform_driver(snd_rpi_i2s_slave_driver);

MODULE_AUTHOR("Julian Scheel <julian@jusst.de>");
MODULE_DESCRIPTION("ASoC Driver for RPi with generic I2S DAC (rpi slave)");
MODULE_LICENSE("GPL v2");
