/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "audio_hw_primary"
#define LOG_NDEBUG 0

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/str_parms.h>

#include <hardware/audio.h>
#include <hardware/hardware.h>

#include <system/audio.h>

#include <tinyalsa/asoundlib.h>

#include <audio_utils/resampler.h>

#include <usb_audio.h>		// Planet 20130228 Usb_Audio

#include "audio_route.h"

#define UAC_ENABLE

//#define WIDHOUT_SOUND_CARD

#define PCM_CARD 0
#define PCM_DEVICE 0
#define PCM_DEVICE_SCO -1

#define OUT_PERIOD_SIZE 512
#define OUT_SHORT_PERIOD_COUNT 2
#define OUT_LONG_PERIOD_COUNT 8
#define OUT_SAMPLING_RATE 44100

#define IN_PERIOD_SIZE 1024
#define IN_PERIOD_COUNT 4
#define IN_SAMPLING_RATE 44100

#define SCO_PERIOD_SIZE 256
#define SCO_PERIOD_COUNT 4
#define SCO_SAMPLING_RATE 8000

/* minimum sleep time in out_write() when write threshold is not reached */
#define MIN_WRITE_SLEEP_US 2000
#define MAX_WRITE_SLEEP_US ((OUT_PERIOD_SIZE * OUT_SHORT_PERIOD_COUNT * 1000000) \
                                / OUT_SAMPLING_RATE)

enum {
    OUT_BUFFER_TYPE_UNKNOWN,
    OUT_BUFFER_TYPE_SHORT,
    OUT_BUFFER_TYPE_LONG,
};

struct pcm_config pcm_config_out = {
    .channels = 2,
    .rate = OUT_SAMPLING_RATE,
    .period_size = OUT_PERIOD_SIZE,
    .period_count = OUT_LONG_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
    .start_threshold = OUT_PERIOD_SIZE * OUT_SHORT_PERIOD_COUNT,
};

struct pcm_config pcm_config_in = {
    .channels = 2,
    .rate = IN_SAMPLING_RATE,
    .period_size = IN_PERIOD_SIZE,
    .period_count = IN_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
    .start_threshold = 1,
    .stop_threshold = (IN_PERIOD_SIZE * IN_PERIOD_COUNT),
};

struct pcm_config pcm_config_sco = {
    .channels = 1,
    .rate = SCO_SAMPLING_RATE,
    .period_size = SCO_PERIOD_SIZE,
    .period_count = SCO_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct audio_device {
    struct audio_hw_device hw_device;

    pthread_mutex_t lock; /* see note below on mutex acquisition order */
    unsigned int out_device;
    unsigned int in_device;
    bool standby;
    bool mic_mute;
    struct audio_route *ar;
    int orientation;
    bool screen_off;

    struct stream_out *active_out;
    struct stream_in *active_in;
};

struct stream_out {
    struct audio_stream_out stream;

    pthread_mutex_t lock; /* see note below on mutex acquisition order */
    struct pcm *pcm;
    struct pcm_config *pcm_config;
    bool standby;
    uint64_t written; /* total frames written, not cleared when entering standby */

    struct resampler_itfe *resampler;
    int16_t *buffer;
    size_t buffer_frames;

    int write_threshold;
    int cur_write_threshold;
    int buffer_type;
	int voip_mode;		// VoipAudio Planet 20121212

    struct audio_device *dev;
};

struct stream_in {
    struct audio_stream_in stream;

    pthread_mutex_t lock; /* see note below on mutex acquisition order */
    struct pcm *pcm;
    struct pcm_config *pcm_config;
    bool standby;

    unsigned int requested_rate;
    struct resampler_itfe *resampler;
    struct resampler_buffer_provider buf_provider;
    int16_t *buffer;
    size_t buffer_size;
    size_t frames_in;
    int read_status;
	int voip_mode;		// VoipAudio Planet 20121212

    struct audio_device *dev;
};

enum {
    ORIENTATION_LANDSCAPE,
    ORIENTATION_PORTRAIT,
    ORIENTATION_SQUARE,
    ORIENTATION_UNDEFINED,
};

static uint32_t out_get_sample_rate(const struct audio_stream *stream);
static size_t out_get_buffer_size(const struct audio_stream *stream);
static audio_format_t out_get_format(const struct audio_stream *stream);
static uint32_t in_get_sample_rate(const struct audio_stream *stream);
static size_t in_get_buffer_size(const struct audio_stream *stream);
static audio_format_t in_get_format(const struct audio_stream *stream);
static int get_next_buffer(struct resampler_buffer_provider *buffer_provider,
                                   struct resampler_buffer* buffer);
static void release_buffer(struct resampler_buffer_provider *buffer_provider,
                                  struct resampler_buffer* buffer);

/*
 * NOTE: when multiple mutexes have to be acquired, always take the
 * audio_device mutex first, followed by the stream_in and/or
 * stream_out mutexes.
 */

/* Helper functions */

static void select_devices(struct audio_device *adev)
{
    int headphone_on;
    int speaker_on;
    int main_mic_on;
    int line_in_on;
	int hdmi_on;		// Planet 20130220 hdmi_on add

    headphone_on = adev->out_device & (AUDIO_DEVICE_OUT_WIRED_HEADSET |
                                    AUDIO_DEVICE_OUT_WIRED_HEADPHONE);
    speaker_on = adev->out_device & AUDIO_DEVICE_OUT_SPEAKER;
    main_mic_on = adev->in_device & AUDIO_DEVICE_IN_BUILTIN_MIC;
    line_in_on = adev->in_device & AUDIO_DEVICE_IN_LINE;
	hdmi_on = adev->out_device & AUDIO_DEVICE_OUT_AUX_DIGITAL;	// Planet 20130220 hdmi_on add

#ifndef WIDHOUT_SOUND_CARD
    reset_mixer_state(adev->ar);

#if defined(TCC_AUDIO_RT5625)
    if (adev->mode == AUDIO_MODE_IN_CALL) {
        if (!headphone_on) {
            audio_route_apply_path(adev->ar, "mic_for_call_with_spk");
            audio_route_apply_path(adev->ar, "speaker_for_call");
        }
        else {
            audio_route_apply_path(adev->ar, "mic_for_call_with_hp");
            audio_route_apply_path(adev->ar, "headphone_for_call");
        }
    }
    else
#endif
    {
        if (speaker_on)
            audio_route_apply_path(adev->ar, "speaker");
        if (headphone_on)
            audio_route_apply_path(adev->ar, "headphone");
        if (main_mic_on) {
#if 0   // case of Nexus7
            if (adev->orientation == ORIENTATION_LANDSCAPE)
                audio_route_apply_path(adev->ar, "main-mic-left");
            else
                audio_route_apply_path(adev->ar, "main-mic-top");
#endif
            audio_route_apply_path(adev->ar, "main-mic");

        }
        if (line_in_on) {
            audio_route_apply_path(adev->ar, "line-in");
        }
		if (hdmi_on) {	// Planet 20130220 hdmi_on add
            audio_route_apply_path(adev->ar, "hdmi-on");
        }

    }

    update_mixer_state(adev->ar);
#endif /* WIDHOUT_SOUND_CARD */

    ALOGE("hp=%c speaker=%c main-mic=%c line-in=%c", headphone_on ? 'y' : 'n',
          speaker_on ? 'y' : 'n', main_mic_on ? 'y' : 'n', line_in_on ? 'y' : 'n');
}

/* must be called with hw device and output stream mutexes locked */
static void do_out_standby(struct stream_out *out)
{
    struct audio_device *adev = out->dev;

    if (!out->standby) {
#ifndef WIDHOUT_SOUND_CARD
        pcm_close(out->pcm);
#endif /* WIDHOUT_SOUND_CARD */
        out->pcm = NULL;
        adev->active_out = NULL;
        if (out->resampler) {
            release_resampler(out->resampler);
            out->resampler = NULL;
        }
        if (out->buffer) {
            free(out->buffer);
            out->buffer = NULL;
        }
        out->standby = true;
    }
}

/* must be called with hw device and input stream mutexes locked */
static void do_in_standby(struct stream_in *in)
{
    struct audio_device *adev = in->dev;

    if (!in->standby) {
#ifndef WIDHOUT_SOUND_CARD
        pcm_close(in->pcm);
#endif /* WIDHOUT_SOUND_CARD */
        in->pcm = NULL;
        adev->active_in = NULL;
        if (in->resampler) {
            release_resampler(in->resampler);
            in->resampler = NULL;
        }
        if (in->buffer) {
            free(in->buffer);
            in->buffer = NULL;
        }
        in->standby = true;
    }
}

#ifdef UAC_ENABLE
/*
  type : 0 -> playback, 1 -> capture  
*/
#define DEVICE_TYPE_OUT	0
#define DEVICE_TYPE_IN	1
#define MAX_CARD_NUM    2

int get_external_card(int type)
{
    int card_num = 1;   // start num, 0 is defualt sound card.
    
    struct stat card_stat;
    char fpath[256];
    int ret;


    while(card_num <= MAX_CARD_NUM)
    {
        snprintf(fpath, sizeof(fpath), "/proc/asound/card%d", card_num);

        ret = stat(fpath, &card_stat);
        ALOGV("stat : %s : %d type: %d\n", fpath, ret, type);

        if(ret < 0)
        {
            ret = -1;
        }
        else
        {
            snprintf(fpath, sizeof(fpath), "/dev/snd/pcmC%uD0%c", card_num, 
                type ? 'c' : 'p');
            
            ret = stat(fpath, &card_stat);
            ALOGV("stat : %s : %d, type: %d\n", fpath, ret, type);

            if(ret == 0)
            {                
                return card_num;
            }

        }

        card_num++;
    }


    return ret;
}

static int check_out_stream(struct stream_out *out)
{
    int ret = 0;
    int card = PCM_CARD;

    card = get_external_card(DEVICE_TYPE_OUT);
    if(card < 0)
    {
        card = PCM_CARD;
    }
    else
    {
    	// Planet 20130228 Usb_Audio Start
        usb_audio_prop audio_prop;
		
		audio_prop.channels = out->pcm_config->channels;
		audio_prop.rate = out->pcm_config->rate;
		ret = loadUsbAudioProperties(card, USB_AUDIO_TYPE_PLAYBACK, &audio_prop);
        if(ret < 0) {
            ALOGE("cannot support this usb audio");
            card = PCM_CARD;
        }
        else {
            out->pcm_config->format = audio_prop.format;
            out->pcm_config->channels = audio_prop.channels;
            out->pcm_config->rate = audio_prop.rate;
        }
		// Planet 20130228 Usb_Audio End
    }
    return card;
}


static int check_input_stream(struct stream_in *in)
{
    int ret = 0;
    int card = PCM_CARD;

    card = get_external_card(DEVICE_TYPE_IN);
    if(card < 0)
    {
        card = PCM_CARD;
    }
    else
    {
    	// Planet 20130228 Usb_Audio Start
        usb_audio_prop audio_prop;
		
		audio_prop.channels = in->pcm_config->channels;
		audio_prop.rate = in->requested_rate;
		ret = loadUsbAudioProperties(card, USB_AUDIO_TYPE_CAPTURE, &audio_prop);
        if(ret < 0) {
            ALOGE("cannot support this usb audio");
            card = PCM_CARD;
        }
        else {
            in->pcm_config->format = audio_prop.format;
            in->pcm_config->channels = audio_prop.channels;
            in->pcm_config->rate = audio_prop.rate;
        }
		// Planet 20130228 Usb_Audio End
    }
    return card;
}
#endif

/* must be called with hw device and output stream mutexes locked */
static int start_output_stream(struct stream_out *out)
{
    struct audio_device *adev = out->dev;
    unsigned int device;
    int ret;
    unsigned int card = PCM_CARD;
    int ext_card;

	char value_audio[PROPERTY_VALUE_MAX];	// SPDIF PCM Planet 20121107
	char value[PROPERTY_VALUE_MAX];
	memset(value, NULL, PROPERTY_VALUE_MAX);
    /*
     * Due to the lack of sample rate converters in the SoC,
     * it greatly simplifies things to have only the main
     * (speaker/headphone) PCM or the BC SCO PCM open at
     * the same time.
     */
    if (adev->out_device & AUDIO_DEVICE_OUT_ALL_SCO) {
        device = PCM_DEVICE_SCO;
        out->pcm_config = &pcm_config_sco;
    } else {
        device = PCM_DEVICE;
        out->pcm_config = &pcm_config_out;
        out->buffer_type = OUT_BUFFER_TYPE_UNKNOWN;
    }

#if 0   // case of Nexus7
    /*
     * All open PCMs can only use a single group of rates at once:
     * Group 1: 11.025, 22.05, 44.1
     * Group 2: 8, 16, 32, 48
     * Group 1 is used for digital audio playback since 44.1 is
     * the most common rate, but group 2 is required for SCO.
     */
    if (adev->active_in) {
        struct stream_in *in = adev->active_in;
        pthread_mutex_lock(&in->lock);
        if (((out->pcm_config->rate % 8000 == 0) &&
                 (in->pcm_config->rate % 8000) != 0) ||
                 ((out->pcm_config->rate % 11025 == 0) &&
                 (in->pcm_config->rate % 11025) != 0))
            do_in_standby(in);
        pthread_mutex_unlock(&in->lock);
    }
#endif

#ifdef UAC_ENABLE
	card = check_out_stream(out);
#endif

    // SPDIF PCM Planet 20121107 Start
	property_get("persist.sys.spdif_setting", value, "");
    ALOGE("start_output_stream()  persist.sys.spdif_setting=%s", value);
	if (strcmp(value, "1") == 0)
	{
        // None of the Android defined audio devices exist. Open a spdif device.
        //devName = "hw:0,1";
        out->pcm = pcm_open(0, 1, PCM_OUT, out->pcm_config);
        ALOGE("Device Name (for spdif) is 001");
	}
    else if(strcmp(value, "0") == 0 && (adev->out_device & 0x10000000)) // adew->devices is SPDIF
	{
        // None of the Android defined audio devices exist. Open a spdif device.
        //devName = "hw:0,1";
        out->pcm = pcm_open(0, 1, PCM_OUT, out->pcm_config);
        ALOGE("Device Name (for spdif) is 002");
    }
    else	// SPDIF PCM Planet 20121107 End
    {
    	out->pcm = pcm_open(card, device, PCM_OUT | PCM_NORESTART, out->pcm_config);
		ALOGE("Device Name Normal Play");
    }

#ifndef WIDHOUT_SOUND_CARD
    if (out->pcm && !pcm_is_ready(out->pcm)) {
        ALOGE("pcm_open(out) failed: %s", pcm_get_error(out->pcm));
        pcm_close(out->pcm);
        return -ENOMEM;
    }
#endif /* WIDHOUT_SOUND_CARD */

    ALOGI("start_output_stream, channel(%d), samplerate(%d)", out->pcm_config->channels, out->pcm_config->rate);


	// SPDIF Planet 20120323 Start
	memset(value_audio, 0, 10);
	value_audio[0] = (char)out->pcm_config->channels;
	value_audio[0] += '0';
	property_set("tcc.audio.channels", value_audio);

	memset(value_audio, 0, 10);
	switch(out->pcm_config->rate) {
	    case 32000: 
			value_audio[0] = (char)0;	// SF_32KHZ
	        break;
	    default:
	    case 44100: 
			value_audio[0] = (char)1;	// SF_44KHZ
	        break;
	    case 88200: 
			value_audio[0] = (char)2;	// SF_88KHZ
	        break;
	    case 176000: 
			value_audio[0] = (char)3;	// SF_176KHZ
	        break;
	    case 48000: 
			value_audio[0] = (char)4;	// SF_48KHZ
	        break;
	    case 96000: 
			value_audio[0] = (char)5;	// SF_96KHZ
	        break;
	    case 192000: 
			value_audio[0] = (char)6;	// SF_192KHZ
	        break;
	}
	value_audio[0] += '0';
	property_set("tcc.audio.sampling_rate", value_audio);
	// SPDIF Planet 20120323 End

    /*
     * If the stream rate differs from the PCM rate, we need to
     * create a resampler.
     */
    if (out_get_sample_rate(&out->stream.common) != out->pcm_config->rate) {
        ret = create_resampler(out_get_sample_rate(&out->stream.common),
                               out->pcm_config->rate,
                               out->pcm_config->channels,
                               RESAMPLER_QUALITY_DEFAULT,
                               NULL,
                               &out->resampler);
        out->buffer_frames = (pcm_config_out.period_size * out->pcm_config->rate) /
                out_get_sample_rate(&out->stream.common) + 1;

        out->buffer = malloc(pcm_frames_to_bytes(out->pcm, out->buffer_frames));
    }

    adev->active_out = out;

    return 0;
}

/* must be called with hw device and input stream mutexes locked */
static int start_input_stream(struct stream_in *in)
{
    struct audio_device *adev = in->dev;
    unsigned int device;
    int ret;
    unsigned int card = PCM_CARD;
    int ext_card;

    /*
     * Due to the lack of sample rate converters in the SoC,
     * it greatly simplifies things to have only the main
     * mic PCM or the BC SCO PCM open at the same time.
     */
    if (adev->in_device & AUDIO_DEVICE_IN_ALL_SCO) {
        device = PCM_DEVICE_SCO;
        in->pcm_config = &pcm_config_sco;
    } else {
        device = PCM_DEVICE;
        in->pcm_config = &pcm_config_in;
    }

#if 0   // case of Nexus7
    /*
     * All open PCMs can only use a single group of rates at once:
     * Group 1: 11.025, 22.05, 44.1
     * Group 2: 8, 16, 32, 48
     * Group 1 is used for digital audio playback since 44.1 is
     * the most common rate, but group 2 is required for SCO.
     */
    if (adev->active_out) {
        struct stream_out *out = adev->active_out;
        pthread_mutex_lock(&out->lock);
        if (((in->pcm_config->rate % 8000 == 0) &&
                 (out->pcm_config->rate % 8000) != 0) ||
                 ((in->pcm_config->rate % 11025 == 0) &&
                 (out->pcm_config->rate % 11025) != 0))
            do_out_standby(out);
        pthread_mutex_unlock(&out->lock);
    }
#endif

#ifdef UAC_ENABLE
	if(device == PCM_DEVICE)
		card = check_input_stream(in);
#endif

    in->pcm = pcm_open(card, device, PCM_IN, in->pcm_config);

#ifndef WIDHOUT_SOUND_CARD
    if (in->pcm && !pcm_is_ready(in->pcm)) {
        ALOGE("pcm_open(in) failed: %s", pcm_get_error(in->pcm));
        pcm_close(in->pcm);
        return -ENOMEM;
    }
#endif /* WIDHOUT_SOUND_CARD */

    ALOGI("start_input_stream, channel(%d), samplerate(%d)", in->pcm_config->channels, in->pcm_config->rate);
    /*
     * If the stream rate differs from the PCM rate, we need to
     * create a resampler.
     */
    if (in_get_sample_rate(&in->stream.common) != in->pcm_config->rate) {
        in->buf_provider.get_next_buffer = get_next_buffer;
        in->buf_provider.release_buffer = release_buffer;

        ret = create_resampler(in->pcm_config->rate,
                               in_get_sample_rate(&in->stream.common),
                               1,
                               RESAMPLER_QUALITY_DEFAULT,
                               &in->buf_provider,
                               &in->resampler);
    }
    in->buffer_size = pcm_frames_to_bytes(in->pcm,
                                          in->pcm_config->period_size);
    in->buffer = malloc(in->buffer_size);
    in->frames_in = 0;

    adev->active_in = in;

    return 0;
}

static int get_next_buffer(struct resampler_buffer_provider *buffer_provider,
                                   struct resampler_buffer* buffer)
{
    struct stream_in *in;

    if (buffer_provider == NULL || buffer == NULL)
        return -EINVAL;

    in = (struct stream_in *)((char *)buffer_provider -
                                   offsetof(struct stream_in, buf_provider));

    if (in->pcm == NULL) {
        buffer->raw = NULL;
        buffer->frame_count = 0;
        in->read_status = -ENODEV;
        return -ENODEV;
    }

    if (in->frames_in == 0) {
        in->read_status = pcm_read(in->pcm,
                                   (void*)in->buffer,
                                   in->buffer_size);
        if (in->read_status != 0) {
            ALOGE("get_next_buffer() pcm_read error %d", in->read_status);
            buffer->raw = NULL;
            buffer->frame_count = 0;
            return in->read_status;
        }
        in->frames_in = in->pcm_config->period_size;
        if (in->pcm_config->channels == 2) {
            unsigned int i;

            /* Discard right channel */
            for (i = 1; i < in->frames_in; i++)
                in->buffer[i] = in->buffer[i * 2];
        }
    }

    buffer->frame_count = (buffer->frame_count > in->frames_in) ?
                                in->frames_in : buffer->frame_count;
    buffer->i16 = in->buffer + (in->pcm_config->period_size - in->frames_in);

    return in->read_status;

}

static void release_buffer(struct resampler_buffer_provider *buffer_provider,
                                  struct resampler_buffer* buffer)
{
    struct stream_in *in;

    if (buffer_provider == NULL || buffer == NULL)
        return;

    in = (struct stream_in *)((char *)buffer_provider -
                                   offsetof(struct stream_in, buf_provider));

    in->frames_in -= buffer->frame_count;
}

/* read_frames() reads frames from kernel driver, down samples to capture rate
 * if necessary and output the number of frames requested to the buffer specified */
static ssize_t read_frames(struct stream_in *in, void *buffer, ssize_t frames)
{
    ssize_t frames_wr = 0;

    while (frames_wr < frames) {
        size_t frames_rd = frames - frames_wr;
        if (in->resampler != NULL) {
            in->resampler->resample_from_provider(in->resampler,
                    (int16_t *)((char *)buffer +
                            frames_wr * audio_stream_frame_size(&in->stream.common)),
                    &frames_rd);
        } else {
            struct resampler_buffer buf = {
                    { raw : NULL, },
                    frame_count : frames_rd,
            };
            get_next_buffer(&in->buf_provider, &buf);
            if (buf.raw != NULL) {
                memcpy((char *)buffer +
                           frames_wr * audio_stream_frame_size(&in->stream.common),
                        buf.raw,
                        buf.frame_count * audio_stream_frame_size(&in->stream.common));
                frames_rd = buf.frame_count;
            }
            release_buffer(&in->buf_provider, &buf);
        }
        /* in->read_status is updated by getNextBuffer() also called by
         * in->resampler->resample_from_provider() */
        if (in->read_status != 0)
            return in->read_status;

        frames_wr += frames_rd;
    }
    return frames_wr;
}

/* API functions */

static uint32_t out_get_sample_rate(const struct audio_stream *stream)
{
	// Planet 20130205 setSamplingRate Start
    struct stream_out *out = (struct stream_out *)stream;
	if(out->pcm_config) {
		return out->pcm_config->rate;
	}
	else {
		return pcm_config_out.rate;
	}
	// Planet 20130205 setSamplingRate End
	
    //return pcm_config_out.rate;
}

static int out_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    return -ENOSYS;
}

static size_t out_get_buffer_size(const struct audio_stream *stream)
{
	// Planet 20130205 setSamplingRate Start
    size_t size = pcm_config_out.period_size;	
    size = ((size + 15) / 16) * 16;

    return size * audio_stream_frame_size((struct audio_stream *)stream);
	// Planet 20130205 setSamplingRate End
	
    //return pcm_config_out.period_size *
    //           audio_stream_frame_size((struct audio_stream *)stream);
}

static uint32_t out_get_channels(const struct audio_stream *stream)
{
    return AUDIO_CHANNEL_OUT_STEREO;
}

static audio_format_t out_get_format(const struct audio_stream *stream)
{
    return AUDIO_FORMAT_PCM_16_BIT;
}

static int out_set_format(struct audio_stream *stream, audio_format_t format)
{
    return -ENOSYS;
}

static int out_standby(struct audio_stream *stream)
{
    struct stream_out *out = (struct stream_out *)stream;

    pthread_mutex_lock(&out->dev->lock);
    pthread_mutex_lock(&out->lock);
    do_out_standby(out);
    pthread_mutex_unlock(&out->lock);
    pthread_mutex_unlock(&out->dev->lock);

    return 0;
}

static int out_dump(const struct audio_stream *stream, int fd)
{
    return 0;
}

static int out_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    struct stream_out *out = (struct stream_out *)stream;
    struct audio_device *adev = out->dev;
    struct str_parms *parms;
    char value[32];
    int ret;
    unsigned int val;

    parms = str_parms_create_str(kvpairs);

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING,
                            value, sizeof(value));
    pthread_mutex_lock(&adev->lock);
    if (ret >= 0) {
        val = atoi(value);
        ALOGI("adev->out_device(%x), val(%x)", adev->out_device, val);
        if ((adev->out_device != val) && (val != 0)) {
            /*
             * If SCO is turned on/off, we need to put audio into standby
             * because SCO uses a different PCM.
             */
            if ((val & AUDIO_DEVICE_OUT_ALL_SCO) ^
                    (adev->out_device & AUDIO_DEVICE_OUT_ALL_SCO)) {
                pthread_mutex_lock(&out->lock);
                do_out_standby(out);
                pthread_mutex_unlock(&out->lock);
            }

			// SPDIF PCM Planet 20121107 Start
			if((!(adev->out_device & AUDIO_DEVICE_OUT_SPDIF_PCM) && (val & AUDIO_DEVICE_OUT_SPDIF_PCM)) ||
				((adev->out_device & AUDIO_DEVICE_OUT_SPDIF_PCM) && !(val & AUDIO_DEVICE_OUT_SPDIF_PCM)))
			{
				pthread_mutex_lock(&out->lock);
				if (!out->standby) {	// SPDIF device setting Planet 20121204
					pcm_close(out->pcm);
					adev->out_device = val;
					start_output_stream(out);
				}
				pthread_mutex_unlock(&out->lock);
			}
			// SPDIF PCM Planet 20121107 End

            adev->out_device = val;
            select_devices(adev);
        }
    }
    pthread_mutex_unlock(&adev->lock);

	// Planet 20130205 setSamplingRate Start
	ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_SAMPLING_RATE, value, sizeof(value));

	pthread_mutex_lock(&adev->lock);
	if (ret >= 0) {
        val = atoi(value);
        if (out == adev->active_out) {
            pthread_mutex_lock(&out->lock);
    		if((out->pcm_config->rate != val) && (val != 0)) {
    			out->pcm_config->rate = val;

                ALOGV("audio stand by");
                do_out_standby(out);
    		}
            else
            {
                ALOGV("same samplerate %d", val);
            }
            pthread_mutex_unlock(&out->lock);
        }
        else
        {
            out->pcm_config->rate = val;
            ALOGV("diff outdevice %x %x", (unsigned int)out, (unsigned int)(adev->active_out));
        }
	}
	pthread_mutex_unlock(&adev->lock);
	// Planet 20130205 setSamplingRate End

    str_parms_destroy(parms);
	return 0;	// Planet 20130205 setSamplingRate
    //return ret;
}

static char * out_get_parameters(const struct audio_stream *stream, const char *keys)
{
    return strdup("");
}

static uint32_t out_get_latency(const struct audio_stream_out *stream)
{
    struct stream_out *out = (struct stream_out *)stream;
    struct audio_device *adev = out->dev;
    size_t period_count;
	uint32_t latency;

    pthread_mutex_lock(&adev->lock);

    if (adev->screen_off && !adev->active_in && !(adev->out_device & AUDIO_DEVICE_OUT_ALL_SCO))
        period_count = OUT_LONG_PERIOD_COUNT;
    else
        period_count = OUT_SHORT_PERIOD_COUNT;

    pthread_mutex_unlock(&adev->lock);

	latency = (pcm_config_out.period_size * period_count * 1000) / pcm_config_out.rate;

	ALOGV("%s : latency(%d)", __func__, latency);

    return latency;
}

static int out_set_volume(struct audio_stream_out *stream, float left,
                          float right)
{
    return -ENOSYS;
}

// for debugging.
//#define AUDIO_OUTPUT_DUMP
static ssize_t out_write(struct audio_stream_out *stream, const void* buffer,
                         size_t bytes)
{
    int ret = 0;
    struct stream_out *out = (struct stream_out *)stream;
    struct audio_device *adev = out->dev;
    size_t frame_size = audio_stream_frame_size(&out->stream.common);
    int16_t *in_buffer = (int16_t *)buffer;
    size_t in_frames = bytes / frame_size;
    size_t out_frames;
    int buffer_type;
    int kernel_frames = 0;	// Planet 20121024 AudioPlay bug fix
    bool sco_on;

#ifdef AUDIO_OUTPUT_DUMP
	FILE *dump_out_fp = NULL;

	dump_out_fp= fopen("/sdcard/dump_out.pcm", "a+");

    if(dump_out_fp == NULL)
        ALOGE("cannot open /sdcard/dump_out.pcm");

    if(dump_out_fp)
    {
 		fwrite(buffer, bytes, 1, dump_out_fp);
		fclose(dump_out_fp);
    }
#endif

	// VoipAudio Planet 20121212 Start
#ifdef WIDHOUT_SOUND_CARD
	if(1)
#else
	if(out->voip_mode)
#endif /* WIDHOUT_SOUND_CARD */
    {
        int sleepTime;

        sleepTime = 1000*bytes/(out->pcm_config->channels * out->pcm_config->rate*2);

        char prop_value[PROPERTY_VALUE_MAX];
		memset(prop_value, NULL, PROPERTY_VALUE_MAX);

        property_get("tcc.media.wfd.sink.run", prop_value, "0");
        if(prop_value[0] == '1')
            usleep(sleepTime*800);//WFD 1000 -> 800
        else
            usleep(sleepTime*1000);
        return bytes;
    }
	// VoipAudio Planet 20121212 End

    /*
     * acquiring hw device mutex systematically is useful if a low
     * priority thread is waiting on the output stream mutex - e.g.
     * executing out_set_parameters() while holding the hw device
     * mutex
     */
    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&out->lock);
    if (out->standby) {
        ret = start_output_stream(out);
        if (ret != 0) {
            pthread_mutex_unlock(&adev->lock);
            goto exit;
        }
        out->standby = false;
    }
    buffer_type = (adev->screen_off && !adev->active_in) ?
            OUT_BUFFER_TYPE_LONG : OUT_BUFFER_TYPE_SHORT;
    sco_on = (adev->out_device & AUDIO_DEVICE_OUT_ALL_SCO);
    pthread_mutex_unlock(&adev->lock);

    /* detect changes in screen ON/OFF state and adapt buffer size
     * if needed. Do not change buffer size when routed to SCO device. */
    if (!sco_on && (buffer_type != out->buffer_type)) {
        size_t period_count;

        if (buffer_type == OUT_BUFFER_TYPE_LONG)
            period_count = OUT_LONG_PERIOD_COUNT;
        else
            period_count = OUT_SHORT_PERIOD_COUNT;

        out->write_threshold = out->pcm_config->period_size * period_count;
        /* reset current threshold if exiting standby */
        if (out->buffer_type == OUT_BUFFER_TYPE_UNKNOWN)
            out->cur_write_threshold = out->write_threshold;
        out->buffer_type = buffer_type;
    }

    /* Reduce number of channels, if necessary */
    if (popcount(out_get_channels(&stream->common)) >
                 (int)out->pcm_config->channels) {
        unsigned int i;

        /* Discard right channel */
        for (i = 1; i < in_frames; i++)
            in_buffer[i] = in_buffer[i * 2];

        /* The frame size is now half */
        frame_size /= 2;
    }

// Planet 20130205 setSamplingRate Start
    /* Change sample rate, if necessary */
    //if (out_get_sample_rate(&stream->common) != out->pcm_config->rate) {
    //    out_frames = out->buffer_frames;
    //    out->resampler->resample_from_input(out->resampler,
    //                                        in_buffer, &in_frames,
    //                                        out->buffer, &out_frames);
    //    in_buffer = out->buffer;
    //} else {
        out_frames = in_frames;
    //}
// Planet 20130205 setSamplingRate End

    if (!sco_on) {
        int total_sleep_time_us = 0;
        size_t period_size = out->pcm_config->period_size;

        /* do not allow more than out->cur_write_threshold frames in kernel
         * pcm driver buffer */
        do {
            struct timespec time_stamp;
            if (pcm_get_htimestamp(out->pcm,
                                   (unsigned int *)&kernel_frames,
                                   &time_stamp) < 0)
                break;
            kernel_frames = pcm_get_buffer_size(out->pcm) - kernel_frames;

            if (kernel_frames > out->cur_write_threshold) {
                int sleep_time_us =
                    (int)(((int64_t)(kernel_frames - out->cur_write_threshold)
                                    * 1000000) / out->pcm_config->rate);
                if (sleep_time_us < MIN_WRITE_SLEEP_US)
                    break;
                total_sleep_time_us += sleep_time_us;
                if (total_sleep_time_us > MAX_WRITE_SLEEP_US) {
                    ALOGW("out_write() limiting sleep time %d to %d",
                          total_sleep_time_us, MAX_WRITE_SLEEP_US);
                    sleep_time_us = MAX_WRITE_SLEEP_US -
                                        (total_sleep_time_us - sleep_time_us);
                }
                usleep(sleep_time_us);
            }

        } while ((kernel_frames > out->cur_write_threshold) &&
                (total_sleep_time_us <= MAX_WRITE_SLEEP_US));

        /* do not allow abrupt changes on buffer size. Increasing/decreasing
         * the threshold by steps of 1/4th of the buffer size keeps the write
         * time within a reasonable range during transitions.
         * Also reset current threshold just above current filling status when
         * kernel buffer is really depleted to allow for smooth catching up with
         * target threshold.
         */
        if((out->cur_write_threshold >=0) && (out->write_threshold >= 0) && (kernel_frames >= 0))	// Planet 20121024 AudioPlay bug fix
        {
	        if (out->cur_write_threshold > out->write_threshold) {
	            out->cur_write_threshold -= period_size / 4;
	            if (out->cur_write_threshold < out->write_threshold) {
	                out->cur_write_threshold = out->write_threshold;
	            }
	        } else if (out->cur_write_threshold < out->write_threshold) {
	            out->cur_write_threshold += period_size / 4;
	            if (out->cur_write_threshold > out->write_threshold) {
	                out->cur_write_threshold = out->write_threshold;
	            }
	        } else if ((kernel_frames < out->write_threshold) &&
	            ((out->write_threshold - kernel_frames) >
	                (int)(period_size * OUT_SHORT_PERIOD_COUNT))) {
	            out->cur_write_threshold = (kernel_frames / period_size + 1) * period_size;
	            out->cur_write_threshold += period_size / 4;
	        }
        }
    }

    ret = pcm_write(out->pcm, in_buffer, out_frames * frame_size);
    if (ret == -EPIPE) {
        /* In case of underrun, don't sleep since we want to catch up asap */
        pthread_mutex_unlock(&out->lock);
        return ret;
    }
    if (ret == 0) {
        out->written += out_frames;
    }

exit:
    pthread_mutex_unlock(&out->lock);

    if (ret != 0) {
        usleep(bytes * 1000000 / audio_stream_frame_size(&stream->common) /
               out_get_sample_rate(&stream->common));
    }

    return bytes;
}

static int out_get_render_position(const struct audio_stream_out *stream,
                                   uint32_t *dsp_frames)
{
    return -EINVAL;
}

static int out_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    return 0;
}

static int out_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    return 0;
}

static int out_get_next_write_timestamp(const struct audio_stream_out *stream,
                                        int64_t *timestamp)
{
    return -EINVAL;
}

static int out_get_presentation_position(const struct audio_stream_out *stream,
                                   uint64_t *frames, struct timespec *timestamp)
{
    struct stream_out *out = (struct stream_out *)stream;
    int ret = -1;

    if(out->pcm == NULL)
        return -1;

    pthread_mutex_lock(&out->lock);

    size_t avail;
    if (pcm_get_htimestamp(out->pcm, &avail, timestamp) == 0) {
        size_t kernel_buffer_size = out->pcm_config->period_size * out->pcm_config->period_count;
        // FIXME This calculation is incorrect if there is buffering after app processor
        int64_t signed_frames = out->written - kernel_buffer_size + avail;
        // It would be unusual for this value to be negative, but check just in case ...
        if (signed_frames >= 0) {
            *frames = signed_frames;
            ret = 0;
        }
    }

    pthread_mutex_unlock(&out->lock);

    return ret;
}

// VoipAudio Planet 20121212 Start
static int out_set_voip_mode(const struct audio_stream *stream, int mode)
{
    struct stream_out *out = (struct stream_out *)stream;
    ALOGE("call out_set_voip_mode old:%d, new:%d", out->voip_mode, mode);

    if(out->voip_mode == mode)
        return 0;
    else if(mode == 0)
    {
        out->voip_mode = mode;
    }
    else
    {
        out->voip_mode = mode;    

        out_standby(stream);
    }

    return 0;
}
// VoipAudio Planet 20121212 End

/** audio_stream_in implementation **/
static uint32_t in_get_sample_rate(const struct audio_stream *stream)
{
    struct stream_in *in = (struct stream_in *)stream;

    return in->requested_rate;
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    return 0;
}

static size_t in_get_buffer_size(const struct audio_stream *stream)
{
    struct stream_in *in = (struct stream_in *)stream;
    size_t size;

    /*
     * take resampling into account and return the closest majoring
     * multiple of 16 frames, as audioflinger expects audio buffers to
     * be a multiple of 16 frames
     */
    size = (in->pcm_config->period_size * in_get_sample_rate(stream)) /
            in->pcm_config->rate;
    size = ((size + 15) / 16) * 16;

    return size * audio_stream_frame_size((struct audio_stream *)stream);
}

static uint32_t in_get_channels(const struct audio_stream *stream)
{
    return AUDIO_CHANNEL_IN_MONO;
}

static audio_format_t in_get_format(const struct audio_stream *stream)
{
    return AUDIO_FORMAT_PCM_16_BIT;
}

static int in_set_format(struct audio_stream *stream, audio_format_t format)
{
    return -ENOSYS;
}

static int in_standby(struct audio_stream *stream)
{
    struct stream_in *in = (struct stream_in *)stream;

    pthread_mutex_lock(&in->dev->lock);
    pthread_mutex_lock(&in->lock);
    do_in_standby(in);
    pthread_mutex_unlock(&in->lock);
    pthread_mutex_unlock(&in->dev->lock);

    return 0;
}

static int in_dump(const struct audio_stream *stream, int fd)
{
    return 0;
}

static int in_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    struct stream_in *in = (struct stream_in *)stream;
    struct audio_device *adev = in->dev;
    struct str_parms *parms;
    char value[32];
    int ret;
    unsigned int val;

    parms = str_parms_create_str(kvpairs);

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING,
                            value, sizeof(value));
    pthread_mutex_lock(&adev->lock);
    if (ret >= 0) {
        val = atoi(value) & ~AUDIO_DEVICE_BIT_IN;
        ALOGI("adev->in_device(%x), val(%x)", adev->in_device, val);
        if ((adev->in_device != val) && (val != 0)) {
            /*
             * If SCO is turned on/off, we need to put audio into standby
             * because SCO uses a different PCM.
             */
            if ((val & AUDIO_DEVICE_IN_ALL_SCO) ^
                    (adev->in_device & AUDIO_DEVICE_IN_ALL_SCO)) {
                pthread_mutex_lock(&in->lock);
                do_in_standby(in);
                pthread_mutex_unlock(&in->lock);
            }

            adev->in_device = val;
            select_devices(adev);
        }
    }
    pthread_mutex_unlock(&adev->lock);

    str_parms_destroy(parms);
    return ret;
}

static char * in_get_parameters(const struct audio_stream *stream,
                                const char *keys)
{
    return strdup("");
}

static int in_set_gain(struct audio_stream_in *stream, float gain)
{
    return 0;
}

// for debugging.
//#define AUDIO_INPUT_DUMP
static ssize_t in_read(struct audio_stream_in *stream, void* buffer,
                       size_t bytes)
{
    int ret = 0;
    struct stream_in *in = (struct stream_in *)stream;
    struct audio_device *adev = in->dev;
    size_t frames_rq = bytes / audio_stream_frame_size(&stream->common);

	// VoipAudio Planet 20121212 Start
#ifdef WIDHOUT_SOUND_CARD
	if(1)
#else
	if(in->voip_mode)
#endif
    {
        int sleepTime;

        sleepTime = 1000*bytes/(in->pcm_config->channels * in->pcm_config->rate*2);
        usleep(sleepTime*1000);
        return bytes;
    }
	// VoipAudio Planet 20121212 End

    /*
     * acquiring hw device mutex systematically is useful if a low
     * priority thread is waiting on the input stream mutex - e.g.
     * executing in_set_parameters() while holding the hw device
     * mutex
     */
    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&in->lock);
    if (in->standby) {
        ret = start_input_stream(in);
        if (ret == 0)
            in->standby = 0;
    }
    pthread_mutex_unlock(&adev->lock);

    if (ret < 0)
        goto exit;

    /*if (in->num_preprocessors != 0) {
        ret = process_frames(in, buffer, frames_rq);
    } else */if (in->resampler != NULL) {
        ret = read_frames(in, buffer, frames_rq);
    } else if (in->pcm_config->channels == 2) {
        /*
         * If the PCM is stereo, capture twice as many frames and
         * discard the right channel.
         */
        unsigned int i;
        int16_t *in_buffer = (int16_t *)buffer;

        ret = pcm_read(in->pcm, in->buffer, bytes * 2);

        /* Discard right channel */
        for (i = 0; i < frames_rq; i++)
            in_buffer[i] = in->buffer[i * 2];
    } else {
        ret = pcm_read(in->pcm, buffer, bytes);
    }

    if (ret > 0)
        ret = 0;

    /*
     * Instead of writing zeroes here, we could trust the hardware
     * to always provide zeroes when muted.
     */
    if (ret == 0 && adev->mic_mute)
        memset(buffer, 0, bytes);

#ifdef AUDIO_INPUT_DUMP
	FILE *dump_input_fp = NULL;

	dump_input_fp= fopen("/data/dump_in.pcm", "a+");

    if(dump_input_fp == NULL)
        ALOGE("cannot open /data/dump_in.pcm");

    if(dump_input_fp)
    {
 		fwrite(buffer, bytes, 1, dump_input_fp);
		fclose(dump_input_fp);
    }
#endif

exit:
    if (ret < 0)
        usleep(bytes * 1000000 / audio_stream_frame_size(&stream->common) /
               in_get_sample_rate(&stream->common));

    pthread_mutex_unlock(&in->lock);
    return bytes;
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream)
{
    return 0;
}

static int in_add_audio_effect(const struct audio_stream *stream,
                               effect_handle_t effect)
{
    return 0;
}

static int in_remove_audio_effect(const struct audio_stream *stream,
                                  effect_handle_t effect)
{
    return 0;
}

// VoipAudio Planet 20121212 Start
static int in_set_voip_mode(const struct audio_stream *stream, int mode)
{
    struct stream_in *in= (struct stream_in *)stream;
    ALOGE("call in_set_voip_mode old:%d, new:%d", in->voip_mode, mode);

    if(in->voip_mode == mode)
        return 0;
    else if(mode == 0)
    {
        in->voip_mode = mode;
    }
    else
    {
        in_standby(stream);

        in->voip_mode = mode;    
    }

    return 0;
}
// VoipAudio Planet 20121212 End

static int adev_open_output_stream(struct audio_hw_device *dev,
                                   audio_io_handle_t handle,
                                   audio_devices_t devices,
                                   audio_output_flags_t flags,
                                   struct audio_config *config,
                                   struct audio_stream_out **stream_out)
{
    struct audio_device *adev = (struct audio_device *)dev;
    struct stream_out *out;
    int ret;

    out = (struct stream_out *)calloc(1, sizeof(struct stream_out));
    if (!out)
        return -ENOMEM;

    out->stream.common.get_sample_rate = out_get_sample_rate;
    out->stream.common.set_sample_rate = out_set_sample_rate;
    out->stream.common.get_buffer_size = out_get_buffer_size;
    out->stream.common.get_channels = out_get_channels;
    out->stream.common.get_format = out_get_format;
    out->stream.common.set_format = out_set_format;
    out->stream.common.standby = out_standby;
    out->stream.common.dump = out_dump;
    out->stream.common.set_parameters = out_set_parameters;
    out->stream.common.get_parameters = out_get_parameters;
    out->stream.common.add_audio_effect = out_add_audio_effect;
    out->stream.common.remove_audio_effect = out_remove_audio_effect;
	out->stream.common.set_voip_mode = out_set_voip_mode;		// VoipAudio Planet 20121212
    out->stream.get_latency = out_get_latency;
    out->stream.set_volume = out_set_volume;
    out->stream.write = out_write;
    out->stream.get_render_position = out_get_render_position;
    out->stream.get_next_write_timestamp = out_get_next_write_timestamp;
    out->stream.get_presentation_position = out_get_presentation_position;

    out->dev = adev;

	ALOGD("OUT: config->sample_rate : %d, config->channel_count : %d, config->format : %d", config->sample_rate, popcount(config->channel_mask), config->format);
	
    config->format = out_get_format(&out->stream.common);
    config->channel_mask = out_get_channels(&out->stream.common);
    config->sample_rate = out_get_sample_rate(&out->stream.common);

    out->standby = true;

    ALOGD("OUT: pcm_config (%x), config (%x)", (unsigned int)(out->pcm_config), (unsigned int)config);

    *stream_out = &out->stream;
    return 0;

err_open:
    free(out);
    *stream_out = NULL;
    return ret;
}

static void adev_close_output_stream(struct audio_hw_device *dev,
                                     struct audio_stream_out *stream)
{
    out_standby(&stream->common);
    free(stream);
}

static int adev_set_parameters(struct audio_hw_device *dev, const char *kvpairs)
{
    struct audio_device *adev = (struct audio_device *)dev;
    struct str_parms *parms;
    char *str;
    char value[32];
    int ret;

    parms = str_parms_create_str(kvpairs);
    ret = str_parms_get_str(parms, "orientation", value, sizeof(value));
    if (ret >= 0) {
        int orientation;

        if (strcmp(value, "landscape") == 0)
            orientation = ORIENTATION_LANDSCAPE;
        else if (strcmp(value, "portrait") == 0)
            orientation = ORIENTATION_PORTRAIT;
        else if (strcmp(value, "square") == 0)
            orientation = ORIENTATION_SQUARE;
        else
            orientation = ORIENTATION_UNDEFINED;

        pthread_mutex_lock(&adev->lock);
        if (orientation != adev->orientation) {
            adev->orientation = orientation;
            /*
             * Orientation changes can occur with the input device
             * closed so we must call select_devices() here to set
             * up the mixer. This is because select_devices() will
             * not be called when the input device is opened if no
             * other input parameter is changed.
             */
            select_devices(adev);
        }
        pthread_mutex_unlock(&adev->lock);
    }

    ret = str_parms_get_str(parms, "screen_state", value, sizeof(value));
    if (ret >= 0) {
        if (strcmp(value, AUDIO_PARAMETER_VALUE_ON) == 0)
            adev->screen_off = false;
        else
            adev->screen_off = true;
    }

    str_parms_destroy(parms);
    return ret;
}

static char * adev_get_parameters(const struct audio_hw_device *dev,
                                  const char *keys)
{
    return strdup("");
}

static int adev_init_check(const struct audio_hw_device *dev)
{
    return 0;
}

static int adev_set_voice_volume(struct audio_hw_device *dev, float volume)
{
    return -ENOSYS;
}

static int adev_set_master_volume(struct audio_hw_device *dev, float volume)
{
    return -ENOSYS;
}

static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode)
{
    return 0;
}

static int adev_set_mic_mute(struct audio_hw_device *dev, bool state)
{
    struct audio_device *adev = (struct audio_device *)dev;

    adev->mic_mute = state;

    return 0;
}

static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state)
{
    struct audio_device *adev = (struct audio_device *)dev;

    *state = adev->mic_mute;

    return 0;
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev,
                                         const struct audio_config *config)
{
    size_t size;

    /*
     * take resampling into account and return the closest majoring
     * multiple of 16 frames, as audioflinger expects audio buffers to
     * be a multiple of 16 frames
     */
    size = (pcm_config_in.period_size * config->sample_rate) / pcm_config_in.rate;
    size = ((size + 15) / 16) * 16;

    return (size * popcount(config->channel_mask) *
                audio_bytes_per_sample(config->format));
}

static int adev_open_input_stream(struct audio_hw_device *dev,
                                  audio_io_handle_t handle,
                                  audio_devices_t devices,
                                  struct audio_config *config,
                                  struct audio_stream_in **stream_in)
{
    struct audio_device *adev = (struct audio_device *)dev;
    struct stream_in *in;
    int ret;

    *stream_in = NULL;

    /* Respond with a request for mono if a different format is given. */
    if (config->channel_mask != AUDIO_CHANNEL_IN_MONO) {
        config->channel_mask = AUDIO_CHANNEL_IN_MONO;
        return -EINVAL;
    }

    in = (struct stream_in *)calloc(1, sizeof(struct stream_in));
    if (!in)
        return -ENOMEM;

    in->stream.common.get_sample_rate = in_get_sample_rate;
    in->stream.common.set_sample_rate = in_set_sample_rate;
    in->stream.common.get_buffer_size = in_get_buffer_size;
    in->stream.common.get_channels = in_get_channels;
    in->stream.common.get_format = in_get_format;
    in->stream.common.set_format = in_set_format;
    in->stream.common.standby = in_standby;
    in->stream.common.dump = in_dump;
    in->stream.common.set_parameters = in_set_parameters;
    in->stream.common.get_parameters = in_get_parameters;
    in->stream.common.add_audio_effect = in_add_audio_effect;
    in->stream.common.remove_audio_effect = in_remove_audio_effect;
	in->stream.common.set_voip_mode = in_set_voip_mode;		// VoipAudio Planet 20121212
    in->stream.set_gain = in_set_gain;
    in->stream.read = in_read;
    in->stream.get_input_frames_lost = in_get_input_frames_lost;

    in->dev = adev;
    in->standby = true;
    in->requested_rate = config->sample_rate;

    pcm_config_in.channels = 2;
    pcm_config_in.rate = IN_SAMPLING_RATE;
	pcm_config_in.format = PCM_FORMAT_S16_LE;
    in->pcm_config = &pcm_config_in; /* default PCM config */

    *stream_in = &in->stream;

    ALOGD("IN: sample_rate(%d), channel(%d), format(%d)", config->sample_rate, popcount(config->channel_mask), config->format);
    return 0;
}

static void adev_close_input_stream(struct audio_hw_device *dev,
                                   struct audio_stream_in *stream)
{
    struct stream_in *in = (struct stream_in *)stream;

    in_standby(&stream->common);
    free(stream);
}

static int adev_dump(const audio_hw_device_t *device, int fd)
{
    return 0;
}

static int adev_close(hw_device_t *device)
{
    struct audio_device *adev = (struct audio_device *)device;

	if(adev->ar)
	    audio_route_free(adev->ar);

    free(device);
    return 0;
}

static int adev_open(const hw_module_t* module, const char* name,
                     hw_device_t** device)
{
    struct audio_device *adev;
    int ret;

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
        return -EINVAL;

    adev = calloc(1, sizeof(struct audio_device));
    if (!adev)
        return -ENOMEM;

    adev->hw_device.common.tag = HARDWARE_DEVICE_TAG;
    adev->hw_device.common.version = AUDIO_DEVICE_API_VERSION_2_0;
    adev->hw_device.common.module = (struct hw_module_t *) module;
    adev->hw_device.common.close = adev_close;

    adev->hw_device.init_check = adev_init_check;
    adev->hw_device.set_voice_volume = adev_set_voice_volume;
    adev->hw_device.set_master_volume = adev_set_master_volume;
    adev->hw_device.set_mode = adev_set_mode;
    adev->hw_device.set_mic_mute = adev_set_mic_mute;
    adev->hw_device.get_mic_mute = adev_get_mic_mute;
    adev->hw_device.set_parameters = adev_set_parameters;
    adev->hw_device.get_parameters = adev_get_parameters;
    adev->hw_device.get_input_buffer_size = adev_get_input_buffer_size;
    adev->hw_device.open_output_stream = adev_open_output_stream;
    adev->hw_device.close_output_stream = adev_close_output_stream;
    adev->hw_device.open_input_stream = adev_open_input_stream;
    adev->hw_device.close_input_stream = adev_close_input_stream;
    adev->hw_device.dump = adev_dump;

#ifndef WIDHOUT_SOUND_CARD
    adev->ar = audio_route_init();
#endif

    adev->orientation = ORIENTATION_UNDEFINED;
    adev->out_device = AUDIO_DEVICE_OUT_SPEAKER;
    adev->in_device = AUDIO_DEVICE_IN_BUILTIN_MIC & ~AUDIO_DEVICE_BIT_IN;
	select_devices(adev);

    *device = &adev->hw_device.common;

    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = adev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "Telechips audio HW HAL",
        .author = "The Android Open Source Project",
        .methods = &hal_module_methods,
    },
};
