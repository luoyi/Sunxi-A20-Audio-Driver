/*
 * PCM510x ASoC codec driver
 *
 *	Andrea Venturi <be17068@iperbole.bo.it>
 *
 *	Borrowed from PCM510X, Copyright (c) StreamUnlimited GmbH 2013
 *	Marek Belisko <marek.belisko@streamunlimited.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/tlv.h>

#define PCM510X_PCM_FORMATS (SNDRV_PCM_FMTBIT_S16_LE  |		\
			     SNDRV_PCM_FMTBIT_S24_LE)

#define PCM510X_PCM_RATES   (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | \
			     SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100  | \
			     SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200  | \
			     SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000)

struct pcm510x_platform_data { 
	int format;
};
#define PCM510x_FORMAT_I2S   0
#define PCM510x_FORMAT_LJS   1

struct pcm510x_private {
	unsigned int format;
	/* Current rate for deemphasis control */
	unsigned int rate;
};

static int pcm510x_set_dai_fmt(struct snd_soc_dai *codec_dai,
			      unsigned int format)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct pcm510x_private *priv = snd_soc_codec_get_drvdata(codec);

	/* The PCM510X can only be slave to all clocks */
	if ((format & SND_SOC_DAIFMT_MASTER_MASK) != SND_SOC_DAIFMT_CBS_CFS) {
		dev_err(codec->dev, "Invalid clocking mode\n");
		return -EINVAL;
	}

	priv->format = format;

	return 0;
}

static int pcm510x_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params,
			     struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	struct pcm510x_private *priv = snd_soc_codec_get_drvdata(codec);

	priv->rate = params_rate(params);

	switch (priv->format & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		break;
	default:
		dev_err(codec->dev, "Invalid DAI format\n");
		return -EINVAL;
	}

	return 0;
}

static const struct snd_soc_dai_ops pcm510x_dai_ops = {
	.set_fmt	= pcm510x_set_dai_fmt,
	.hw_params	= pcm510x_hw_params,
};

static const struct snd_soc_dapm_widget pcm510x_dapm_widgets[] = {
SND_SOC_DAPM_OUTPUT("VOUT1"),
SND_SOC_DAPM_OUTPUT("VOUT2"),
};

static const struct snd_soc_dapm_route pcm510x_dapm_routes[] = {
	{ "VOUT1", NULL, "Playback" },
	{ "VOUT2", NULL, "Playback" },
};

static const struct snd_kcontrol_new pcm510x_controls[] = {
};

static struct snd_soc_dai_driver pcm510x_dai = {
	.name = "pcm510x-hifi",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = PCM510X_PCM_RATES,
		.formats = PCM510X_PCM_FORMATS,
		.sig_bits = 24,
	},
	.ops = &pcm510x_dai_ops,
};

#ifdef CONFIG_OF
static const struct of_device_id pcm510x_dt_ids[] = {
	{ .compatible = "ti,pcm510x", },
	{ }
};
MODULE_DEVICE_TABLE(of, pcm510x_dt_ids);
#endif

static const struct regmap_config pcm510x_regmap = {
	.reg_bits		= 8,
	.val_bits		= 8,
	.max_register		= 0x0,
};

static struct snd_soc_codec_driver soc_codec_dev_pcm510x = {
	.controls		= pcm510x_controls,
	.num_controls		= ARRAY_SIZE(pcm510x_controls),
	.dapm_widgets		= pcm510x_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(pcm510x_dapm_widgets),
	.dapm_routes		= pcm510x_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(pcm510x_dapm_routes),
};

static int pcm510x_probe(struct platform_device *pdev)
{
	//int ret;
	//struct pcm510x_platform_data *pdata = dev_get_platdata(&pdev->dev);
	struct pcm510x_private *priv;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

        platform_set_drvdata(pdev, priv);

	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_pcm510x, &pcm510x_dai, 1);
}

static int pcm510x_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static struct platform_driver pcm510x_driver = {
	.driver = {
		.name	= "pcm510x",
		.of_match_table = of_match_ptr(pcm510x_dt_ids),
	},
	.probe		= pcm510x_probe,
	.remove		= pcm510x_remove,
};

module_platform_driver(pcm510x_driver);

MODULE_DESCRIPTION("Texas Instruments PCM510X ALSA SoC Codec Driver");
MODULE_AUTHOR("Andrea Venturi <be17068@iperbole.bo.it>");
MODULE_LICENSE("GPL");
