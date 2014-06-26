/*
 * ASoC Driver for ProCLIENT DAC
 *
 * Author:	Dennis Hamester <dennis.hamester@gmail.com>
 *		Copyright 2013
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

/* new style switch */
#define NVR101_MUX_1 24
#define NVR101_MUX_0 25

/* old style switch */
#define NVR101_EN_48KHZ 10
#define NVR101_EN_96KHZ 24
#define NVR101_EN_192KHZ 25

static int snd_rpi_nvr101_dac_init(struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

static int snd_rpi_nvr101_dac_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int rate = params_rate(params);

	/* FIXME: Do we really have to disable all before enabling? */
	gpio_set_value(NVR101_EN_48KHZ, 0);
	gpio_set_value(NVR101_EN_96KHZ, 0);
	gpio_set_value(NVR101_EN_192KHZ, 0);
	usleep_range(10000, 50000);

#if 1
	if (rate == 48000) {
		printk("48kHz, set bclk to 2,304 MHz");
		gpio_set_value(NVR101_MUX_0, 1);
		gpio_set_value(NVR101_MUX_1, 1);
	} else if (rate == 96000) {
		printk("96kHz, set bclk to 4,608 MHz");
		gpio_set_value(NVR101_MUX_0, 0);
		gpio_set_value(NVR101_MUX_1, 1);
	} else if (rate == 192000) {
		printk("192kHz, set bclk to 9,216 MHz");
		gpio_set_value(NVR101_MUX_0, 1);
		gpio_set_value(NVR101_MUX_1, 0);
	}
#else
	gpio_set_value(NVR101_EN_48KHZ, rate == 48000 ? 1 : 0);
	gpio_set_value(NVR101_EN_96KHZ, rate == 96000 ? 1 : 0);
	gpio_set_value(NVR101_EN_192KHZ, rate == 192000 ? 1 : 0);
#endif

	return snd_soc_dai_set_bclk_ratio(cpu_dai, 48);
}

static struct snd_soc_ops snd_rpi_nvr101_dac_ops = {
	.hw_params = snd_rpi_nvr101_dac_hw_params,
};

static struct snd_soc_dai_link snd_rpi_nvr101_dac_dai[] = {
	{
		.name = "ProCLIENT DAC",
		.stream_name = "ProCLIENT DAC HiFi",
		.cpu_dai_name = "bcm2708-i2s.0",
		.platform_name = "bcm2708-i2s.0",
		.codec_name = "pcm1796.1-004c",
		.codec_dai_name = "pcm1796-hifi",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
			SND_SOC_DAIFMT_CBM_CFS,
		.ops = &snd_rpi_nvr101_dac_ops,
		.init = snd_rpi_nvr101_dac_init,
	},
};

static struct snd_soc_card snd_rpi_nvr101_dac = {
	.name = "snd_rpi_nvr101_dac",
	.dai_link = snd_rpi_nvr101_dac_dai,
	.num_links = 1,
};

static int snd_rpi_nvr101_dac_probe(struct platform_device *pdev)
{
	int ret = 0;

	snd_rpi_nvr101_dac.dev = &pdev->dev;

#if 1
	if (gpio_request(NVR101_MUX_0, "NVR101_MUX_0")) {
		printk(KERN_ERR "%s: Failed ro request GPIO_%d\n", __func__, NVR101_MUX_0);
		return -EBUSY;
	}
	if (gpio_request(NVR101_MUX_1, "NVR101_MUX_1")) {
		printk(KERN_ERR "%s: Failed ro request GPIO_%d\n", __func__, NVR101_MUX_1);
		return -EBUSY;
	}
	gpio_direction_output(NVR101_MUX_0, 0);
	gpio_direction_output(NVR101_MUX_1, 0);
#else
	if (gpio_request(NVR101_EN_48KHZ, "NVR101_EN_48KHZ")) {
		printk(KERN_ERR "%s: Failed ro request GPIO_%d\n", __func__, NVR101_EN_48KHZ);
		return -EBUSY;
	}
	if (gpio_request(NVR101_EN_96KHZ, "NVR101_EN_96KHZ")) {
		printk(KERN_ERR "%s: Failed ro request GPIO_%d\n", __func__, NVR101_EN_96KHZ);
		gpio_free(NVR101_EN_48KHZ);
		return -EBUSY;
	}
	if (gpio_request(NVR101_EN_192KHZ, "NVR101_EN_192KHZ")) {
		printk(KERN_ERR "%s: Failed ro request GPIO_%d\n", __func__, NVR101_EN_192KHZ);
		gpio_free(NVR101_EN_48KHZ);
		gpio_free(NVR101_EN_96KHZ);
		return -EBUSY;
	}
	gpio_direction_output(NVR101_EN_48KHZ, 0);
	gpio_direction_output(NVR101_EN_96KHZ, 0);
	gpio_direction_output(NVR101_EN_192KHZ, 0);
#endif

	ret = snd_soc_register_card(&snd_rpi_nvr101_dac);
	if (ret)
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n", ret);

	return ret;
}

static int snd_rpi_nvr101_dac_remove(struct platform_device *pdev)
{
#if 1
	gpio_free(NVR101_MUX_0);
	gpio_free(NVR101_MUX_1);
#else
	gpio_free(NVR101_EN_48KHZ);
	gpio_free(NVR101_EN_96KHZ);
	gpio_free(NVR101_EN_192KHZ);
#endif
	return snd_soc_unregister_card(&snd_rpi_nvr101_dac);
}

static struct platform_driver snd_rpi_nvr101_dac_driver = {
	.driver = {
		.name = "snd-nvr101-dac",
		.owner = THIS_MODULE,
	},
	.probe = snd_rpi_nvr101_dac_probe,
	.remove = snd_rpi_nvr101_dac_remove,
};

module_platform_driver(snd_rpi_nvr101_dac_driver);

MODULE_AUTHOR("Dennis Hamester <dennis.hamester@gmail.com>");
MODULE_DESCRIPTION("ASoC Driver for NVR101-P DAC");
MODULE_LICENSE("GPL v2");
