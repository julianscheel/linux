/*
 * Driver for the PCM1796 codec
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/regmap.h>

#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/tlv.h>

#define PCM1796_REG16_ATTL 16
#define PCM1796_REG17_ATTR 17
#define PCM1796_REG18_CTL 18

#define PCM1796_REGMASK_FMT 0x70
#define PCM1796_REGMASK_ATLD 0x80

#define PCM1796_FMT_16_I2S 0x40
#define PCM1796_FMT_24_I2S 0x50

static const DECLARE_TLV_DB_SCALE(master_tlv, -12000, 50, 0);
static const struct snd_kcontrol_new pcm1796_snd_controls[] = {
	SOC_DOUBLE_R_RANGE_TLV("Master Volume", PCM1796_REG16_ATTL,
			PCM1796_REG17_ATTR, 0, 15, 255, 0, master_tlv),
};

static int pcm1796_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params,
		struct snd_soc_dai *dai) {
	int pcm_format = params_format(params);
	int fmt_register = PCM1796_FMT_16_I2S;
	int ret = 0;

	printk("> %s()\n", __func__);
	if (pcm_format == SNDRV_PCM_FORMAT_S24_LE)
		fmt_register = PCM1796_FMT_24_I2S;

	/* enable attenuator */
	ret = snd_soc_update_bits(dai->codec, PCM1796_REG18_CTL,
			PCM1796_REGMASK_ATLD, PCM1796_REGMASK_ATLD);

	ret = snd_soc_update_bits(dai->codec, PCM1796_REG18_CTL,
			PCM1796_REGMASK_FMT, fmt_register);
	if (ret < 0)
		return ret;

	printk("< %s()\n", __func__);
	return 0;
}

static const struct snd_soc_dai_ops pcm1796_ops = {
	.hw_params = pcm1796_hw_params,
};

static struct snd_soc_dai_driver pcm1796_dai = {
	.name = "pcm1796-hifi",
	.playback = {
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_48000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			SNDRV_PCM_FMTBIT_S24_LE,
	},
	.ops = &pcm1796_ops,
};

static int pcm1796_probe(struct snd_soc_codec *codec)
{
	return 0;
}

static int pcm1796_remove(struct snd_soc_codec *codec)
{
	return 0;
}

static struct snd_soc_codec_driver pcm1796_codec_driver = {
	.probe = pcm1796_probe,
	.remove = pcm1796_remove,
	.controls = pcm1796_snd_controls,
	.num_controls = ARRAY_SIZE(pcm1796_snd_controls),
};

static const struct regmap_config pcm1796_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 23,
};

static int pcm1796_i2c_probe(struct i2c_client *i2c,
		const struct i2c_device_id *id)
{
	struct regmap *regmap;
	int ret;

	regmap = devm_regmap_init_i2c(i2c, &pcm1796_regmap);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	ret = snd_soc_register_codec(&i2c->dev, &pcm1796_codec_driver,
			&pcm1796_dai, 1);
	return ret;
}

static int pcm1796_i2c_remove(struct i2c_client *i2c)
{
	snd_soc_unregister_codec(&i2c->dev);
	return 0;
}

static const struct i2c_device_id pcm1796_i2c_id[] = {
	{ "pcm1796", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pcm1796_i2c_id);

static struct i2c_driver pcm1796_i2c_driver = {
	.driver = {
		.name = "pcm1796",
		.owner = THIS_MODULE,
	},
	.probe = pcm1796_i2c_probe,
	.remove = pcm1796_i2c_remove,
	.id_table = pcm1796_i2c_id,
};

module_i2c_driver(pcm1796_i2c_driver);

MODULE_DESCRIPTION("ASoC PCM1796 codec driver");
MODULE_AUTHOR("Dennis Hamester <dennis.hamester@gmail.com>");
MODULE_LICENSE("GPL v2");
