/*
 * tegra_soc_controls.c -- alsa controls for tegra SoC
 *
 * Copyright (c) 2010-2011, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "tegra_soc.h"
#include "../codecs/codec_param.h"
#include "../codecs/wm8903.h"

#define EP_101 101
#define EP_102 102

static struct tegra_audio_data *audio_data;
static int tegra_jack_func;
static int tegra_spk_func;
extern bool lineout_alive;
bool need_spk;
EXPORT_SYMBOL(need_spk);
extern int PRJ_ID;
extern int spk_status;
extern struct wm8903_parameters audio_params[];

static void tegra_audio_route(struct tegra_audio_data* audio_data,
			      int device_new, int is_call_mode_new)
{
	int play_device_new = device_new & TEGRA_AUDIO_DEVICE_OUT_ALL;
	int capture_device_new = device_new & TEGRA_AUDIO_DEVICE_IN_ALL;
	int codec_con = audio_data->codec_con;
	int is_bt_sco_mode =
		(play_device_new & TEGRA_AUDIO_DEVICE_OUT_BT_SCO) ||
		(capture_device_new & TEGRA_AUDIO_DEVICE_IN_BT_SCO);
	int was_bt_sco_mode =
		(audio_data->play_device & TEGRA_AUDIO_DEVICE_OUT_BT_SCO) ||
		(audio_data->capture_device & TEGRA_AUDIO_DEVICE_IN_BT_SCO);

	if (play_device_new) {
		codec_con &= ~(TEGRA_HEADPHONE | TEGRA_LINEOUT |
			TEGRA_SPK | TEGRA_EAR_SPK | TEGRA_HEADSET);

		if ((play_device_new & TEGRA_AUDIO_DEVICE_OUT_HEADPHONE)&&(play_device_new & TEGRA_AUDIO_DEVICE_OUT_SPEAKER)) {
			codec_con |= TEGRA_HEADPHONE;
			codec_con |= TEGRA_SPK;
			printk("Routing to Headphone and Speaker output\n");
			need_spk = true;
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_4, 0x0003); /* MIXSPK Enable*/
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_5, 0x0003); /* SPK Enable*/
			if(spk_status==ENABLE_SPEAKER)
				snd_soc_write(audio_data->codec, WM8903_GPIO_CONTROL_3, 0x0033); /* GPIO3 configure: EN_SPK*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_SPK_MIX_LEFT_0, 0x0008); /* DACR_TO_MIXSPK Enable*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_SPK_MIX_RIGHT_0, 0x0004); /* DACL_TO_MIXSPK Enable*/
			if(PRJ_ID == EP_101){
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_LEFT,audio_params[EP101].analog_speaker_volume | 0x80); /* SPKL Volume: 4dB*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_RIGHT,audio_params[EP101].analog_speaker_volume | 0x80); /* SPKR Volume: 4dB*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_LEFT,audio_params[EP101].analog_headset_volume | 0x80); /* HPOUTL Volume: -2dB */
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_RIGHT,audio_params[EP101].analog_headset_volume | 0x80); /* HPOUTR Volume: -2dB */
			}else if(PRJ_ID == EP_102){
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_LEFT,audio_params[EP102].analog_speaker_volume | 0x80); /* SPKL Volume: 0dB*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_RIGHT,audio_params[EP102].analog_speaker_volume | 0x80); /* SPKR Volume: 0dB*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_LEFT,audio_params[EP102].analog_headset_volume | 0x80); /* HPOUTL Volume: -16dB */
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_RIGHT, audio_params[EP102].analog_headset_volume | 0x80); /* HPOUTR Volume: -16dB */
			}
		}
		else if (play_device_new & TEGRA_AUDIO_DEVICE_OUT_HEADPHONE) {
			codec_con |= TEGRA_HEADPHONE;
			printk("Routing to Headphone output\n");
			need_spk = false;
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_4, 0x0000); /* MIXSPK Disable*/
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_5, 0x0000); /* SPK Disable*/
			snd_soc_write(audio_data->codec, WM8903_GPIO_CONTROL_3, 0x0000); /* Media stream, Mute the speaker */
			if(PRJ_ID == EP_101){
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_LEFT, audio_params[EP101].analog_headset_volume | 0x80); /* HPOUTL Volume: -2dB */
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_RIGHT,audio_params[EP101].analog_headset_volume | 0x80); /* HPOUTR Volume: -2dB */
			}else if(PRJ_ID == EP_102){
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_LEFT,audio_params[EP102].analog_headset_volume | 0x80); /* HPOUTL Volume: -16dB */
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_RIGHT,audio_params[EP102].analog_headset_volume | 0x80); /* HPOUTR Volume: -16dB */
			}
		}
		else if (play_device_new & TEGRA_AUDIO_DEVICE_OUT_HEADSET) {
			codec_con |= TEGRA_HEADSET;
			printk("Routing to Headset output\n");
			need_spk = false;
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_4, 0x0000); /* MIXSPK Disable*/
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_5, 0x0000); /* SPK Disable*/
			snd_soc_write(audio_data->codec, WM8903_GPIO_CONTROL_3, 0x0000); /* Media stream, Mute the speaker */
			if(PRJ_ID == EP_101){
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_LEFT,audio_params[EP101].analog_headset_volume | 0x80); /* HPOUTL Volume: -2dB */
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_RIGHT,audio_params[EP101].analog_headset_volume | 0x80); /* HPOUTR Volume: -2dB */
			}else if(PRJ_ID == EP_102){
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_LEFT,audio_params[EP102].analog_headset_volume | 0x80); /* HPOUTL Volume: -16dB */
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT1_RIGHT,audio_params[EP102].analog_headset_volume | 0x80); /* HPOUTR Volume: -16dB */
			}
		}
		else if (play_device_new & TEGRA_AUDIO_DEVICE_OUT_LINE) {
			codec_con |= TEGRA_LINEOUT;
			printk("Routing to Line output\n");
			need_spk = false;
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_4, 0x0000); /* MIXSPK Disable*/
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_5, 0x0000); /* SPK Disable*/
			snd_soc_write(audio_data->codec, WM8903_GPIO_CONTROL_3, 0x0000); /* Media stream, Mute the speaker */
		}

		if (play_device_new & TEGRA_AUDIO_DEVICE_OUT_SPEAKER) {
			codec_con |= TEGRA_SPK;
			printk("Routing to Speaker output\n");
			need_spk = true;
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_4, 0x0003); /* MIXSPK Enable*/
			if(PRJ_ID == EP_101){
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_LEFT,audio_params[EP101].analog_speaker_volume | 0x80); /* SPKL Volume: 4dB*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_RIGHT,audio_params[EP101].analog_speaker_volume | 0x80); /* SPKR Volume: 4dB*/
			}else if(PRJ_ID == EP_102){
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_LEFT,audio_params[EP102].analog_speaker_volume | 0x80); /* SPKL Volume: 0dB*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_RIGHT,audio_params[EP102].analog_speaker_volume | 0x80); /* SPKR Volume: 0dB*/
			}
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_5, 0x0003); /* SPK Enable*/
			if(spk_status==ENABLE_SPEAKER)
				snd_soc_write(audio_data->codec, WM8903_GPIO_CONTROL_3, 0x0033); /* GPIO3 configure: EN_SPK*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_SPK_MIX_LEFT_0, 0x0008); /* DACR_TO_MIXSPK Enable*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_SPK_MIX_RIGHT_0, 0x0004); /* DACL_TO_MIXSPK Enable*/
		}
		else if (play_device_new & TEGRA_AUDIO_DEVICE_OUT_EAR_SPEAKER) {
			codec_con |= TEGRA_EAR_SPK;
			printk("Routing to Ear Speaker output\n");
			need_spk = true;
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_4, 0x0003); /* MIXSPK Enable*/
			if(PRJ_ID == EP_101){
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_LEFT,audio_params[EP101].analog_speaker_volume | 0x80); /* SPKL Volume: 4dB*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_RIGHT,audio_params[EP101].analog_speaker_volume | 0x80); /* SPKR Volume: 4dB*/
			}else if(PRJ_ID == EP_102){
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_LEFT,audio_params[EP102].analog_speaker_volume | 0x80); /* SPKL Volume: 0dB*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_OUT3_RIGHT,audio_params[EP102].analog_speaker_volume | 0x80); /* SPKR Volume: 0dB*/
			}
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_5, 0x0003); /* SPK Enable*/
			if(spk_status==ENABLE_SPEAKER)
				snd_soc_write(audio_data->codec, WM8903_GPIO_CONTROL_3, 0x0033); /* GPIO3 configure: EN_SPK*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_SPK_MIX_LEFT_0, 0x0008); /* DACR_TO_MIXSPK Enable*/
			snd_soc_write(audio_data->codec, WM8903_ANALOGUE_SPK_MIX_RIGHT_0, 0x0004); /* DACL_TO_MIXSPK Enable*/
		}
		else {
			printk("Routing to unknown output\n");
			need_spk = false;
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_4, 0x0000); /* MIXSPK Disable*/
			snd_soc_write(audio_data->codec, WM8903_POWER_MANAGEMENT_5, 0x0000); /* SPK Disable*/
			snd_soc_write(audio_data->codec, WM8903_GPIO_CONTROL_3, 0x0000); /* Mute the speaker */
		}

		tegra_ext_control(audio_data->codec, codec_con);
		audio_data->play_device = play_device_new;
	}

	if (capture_device_new != audio_data->capture_device) {
		codec_con &= ~(TEGRA_INT_MIC | TEGRA_EXT_MIC |
			TEGRA_LINEIN | TEGRA_HEADSET);

		if (capture_device_new & (TEGRA_AUDIO_DEVICE_IN_BUILTIN_MIC |
			TEGRA_AUDIO_DEVICE_IN_BACK_MIC))
			codec_con |= TEGRA_INT_MIC;

		if (capture_device_new & TEGRA_AUDIO_DEVICE_IN_MIC)
			codec_con |= TEGRA_EXT_MIC;

		if (capture_device_new & TEGRA_AUDIO_DEVICE_IN_LINE)
			codec_con |= TEGRA_LINEIN;

		if (capture_device_new & TEGRA_AUDIO_DEVICE_IN_HEADSET)
			codec_con |= TEGRA_HEADSET;

		tegra_ext_control(audio_data->codec, codec_con);
		audio_data->capture_device = capture_device_new;
	}

	if ((is_call_mode_new != audio_data->is_call_mode) ||
		(is_bt_sco_mode != was_bt_sco_mode)) {
		if (is_call_mode_new && is_bt_sco_mode) {
			tegra_das_set_connection
				(tegra_das_port_con_id_voicecall_with_bt);
		}
		else if (is_call_mode_new && !is_bt_sco_mode) {
			tegra_das_set_connection
				(tegra_das_port_con_id_voicecall_no_bt);
		}
		else if (!is_call_mode_new && is_bt_sco_mode) {
			tegra_das_set_connection
				(tegra_das_port_con_id_bt_codec);
		}
		else {
			tegra_das_set_connection
				(tegra_das_port_con_id_hifi);
		}
		audio_data->is_call_mode = is_call_mode_new;
	}
}

static int tegra_play_route_info(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = TEGRA_AUDIO_DEVICE_NONE;
	uinfo->value.integer.max = TEGRA_AUDIO_DEVICE_MAX;
	return 0;
}

static int tegra_play_route_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct tegra_audio_data* audio_data = snd_kcontrol_chip(kcontrol);

	ucontrol->value.integer.value[0] = TEGRA_AUDIO_DEVICE_NONE;
	if (audio_data) {
		ucontrol->value.integer.value[0] = audio_data->play_device;
		return 0;
	}
	return -EINVAL;
}

static int tegra_play_route_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct tegra_audio_data* audio_data = snd_kcontrol_chip(kcontrol);

	if (audio_data) {
		int play_device_new = ucontrol->value.integer.value[0] &
				TEGRA_AUDIO_DEVICE_OUT_ALL;

		if (audio_data->play_device != play_device_new) {
			tegra_audio_route(audio_data,
				play_device_new | audio_data->capture_device,
				audio_data->is_call_mode);
			return 1;
		}
		return 0;
	}
	return -EINVAL;
}

struct snd_kcontrol_new tegra_play_route_control = {
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "Pcm Playback Route",
	.private_value = 0xffff,
	.info = tegra_play_route_info,
	.get = tegra_play_route_get,
	.put = tegra_play_route_put
};

static int tegra_capture_route_info(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = TEGRA_AUDIO_DEVICE_NONE;
	uinfo->value.integer.max = TEGRA_AUDIO_DEVICE_MAX;
	return 0;
}

static int tegra_capture_route_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct tegra_audio_data* audio_data = snd_kcontrol_chip(kcontrol);

	ucontrol->value.integer.value[0] = TEGRA_AUDIO_DEVICE_NONE;
	if (audio_data) {
		ucontrol->value.integer.value[0] = audio_data->capture_device;
		return 0;
	}
	return -EINVAL;
}

static int tegra_capture_route_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct tegra_audio_data* audio_data = snd_kcontrol_chip(kcontrol);

	if (audio_data) {
		int capture_device_new = ucontrol->value.integer.value[0] &
				TEGRA_AUDIO_DEVICE_IN_ALL;

		if (audio_data->capture_device != capture_device_new) {
			tegra_audio_route(audio_data,
				audio_data->play_device | capture_device_new,
				audio_data->is_call_mode);
			return 1;
		}
		return 0;
	}
	return -EINVAL;
}

struct snd_kcontrol_new tegra_capture_route_control = {
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "Pcm Capture Route",
	.private_value = 0xffff,
	.info = tegra_capture_route_info,
	.get = tegra_capture_route_get,
	.put = tegra_capture_route_put
};

static int tegra_call_mode_info(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}

static int tegra_call_mode_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct tegra_audio_data* audio_data = snd_kcontrol_chip(kcontrol);

	ucontrol->value.integer.value[0] = TEGRA_AUDIO_DEVICE_NONE;
	if (audio_data) {
		ucontrol->value.integer.value[0] = audio_data->is_call_mode;
		return 0;
	}
	return -EINVAL;
}

static int tegra_call_mode_put(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	struct tegra_audio_data* audio_data = snd_kcontrol_chip(kcontrol);

	if (audio_data) {
		int is_call_mode_new = ucontrol->value.integer.value[0];

		if (audio_data->is_call_mode != is_call_mode_new) {
			tegra_audio_route(audio_data,
				audio_data->play_device |
				audio_data->capture_device,
				is_call_mode_new);
			return 1;
		}
		return 0;
	}
	return -EINVAL;
}

struct snd_kcontrol_new tegra_call_mode_control = {
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "Call Mode Switch",
	.private_value = 0xffff,
	.info = tegra_call_mode_info,
	.get = tegra_call_mode_get,
	.put = tegra_call_mode_put
};

int tegra_controls_init(struct snd_soc_codec *codec)
{
	struct tegra_audio_data* audio_data = codec->socdev->codec_data;
	int err;

	/* Add play route control */
	err = snd_ctl_add(codec->card,
		snd_ctl_new1(&tegra_play_route_control, audio_data));
	if (err < 0)
		return err;

	/* Add capture route control */
	err = snd_ctl_add(codec->card,
		snd_ctl_new1(&tegra_capture_route_control, audio_data));
	if (err < 0)
		return err;

	/* Add call mode switch control */
	err = snd_ctl_add(codec->card,
		snd_ctl_new1(&tegra_call_mode_control, audio_data));
	if (err < 0)
		return err;

	return 0;
}
