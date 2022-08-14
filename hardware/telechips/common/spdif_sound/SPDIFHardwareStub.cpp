/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
//#define LOG_TAG "GtestHardwareStub"
#include <utils/Log.h>

#include "SPDIFHardwareStub.h"
#include <utils/threads.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <tinyalsa/asoundlib.h>	// Planet 20120126

#include <cutils/properties.h> // for property_get
#if defined(HDMI_V1_3)
#include <mach/hdmi_1_3_audio.h>	// For HDMI, B090183
#elif defined(HDMI_V1_4)
#include <mach/hdmi_1_4_audio.h>	// For HDMI, B090183
#endif//


#undef LOG_TAG
#define LOG_TAG "SPDIFHardwareStub"

//#define SPDIF_DEBUG_ON
#undef SPDIF_DEBUG_ON

#if defined(SPDIF_DEBUG_ON)
#define  GET_SHORT_FILENAME(longfilename)   \
    ( (strrchr(longfilename, '/')==NULL) ? \
        longfilename: ((char*)(strrchr(longfilename, '/')+1)))


#define gprintf(fmt, args...)      \
    ALOGE("[%d], %s:%d, %s(): "fmt,           \
        gettid(), GET_SHORT_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##args)                         
#else

#define gprintf
#endif

#define AUDIO_OUTPUT_MODE_SPDIF_COMPRESS    2
#define AUDIO_OUTPUT_MODE_DAI_LPCM          3
#define AUDIO_OUTPUT_MODE_DAI_HBR           4

namespace android {

static struct pcm_config pcm_config_out_2ch = {
    2,
    48000,
    2048,
    4,
    PCM_FORMAT_S16_LE,
    2048*4/2,
};
static struct pcm_config pcm_config_out_multi = {
    8,
    192000,
    4096,
    4,
    PCM_FORMAT_S16_LE,
    8192*4/2,
};


SPDIFHardwareStub::SPDIFHardwareStub()
{
    gprintf("SPDIFHardwareStub creator");
	char value[PROPERTY_VALUE_MAX];
	char prop_spdif[PROPERTY_VALUE_MAX];

	property_get("persist.sys.spdif_setting", value, "");
	memset(prop_spdif, NULL, PROPERTY_VALUE_MAX);
	property_get("tcc.audio.path.spdif_pcm.enable", prop_spdif, "0");

    if (value[0] == '3') {
		mSpdifPcm	= NULL;	// Planet 20120126
        mConfig = pcm_config_out_multi;
        mAudioOutputMode = AUDIO_OUTPUT_MODE_DAI_LPCM;
        ALOGD("SPDIFHardwareStub creator: LPCM");
    }
    else if (value[0] == '4')	// HBR
    {
        mSpdifPcm	= NULL;	// Planet 20120126
        mConfig = pcm_config_out_multi;
		char hdmi_audio[PROPERTY_VALUE_MAX];
		memset(hdmi_audio, NULL, PROPERTY_VALUE_MAX);
		property_get("tcc.hdmi.audio_type", hdmi_audio, "0");
		
        mAudioOutputMode = AUDIO_OUTPUT_MODE_DAI_HBR;
 		ALOGD("SPDIFHardwareStub creator: DAI HBR");
   }
    else {						// SPDIF Compressed Data
        mSpdifPcm	= NULL;	// Planet 20120126
        mConfig = pcm_config_out_2ch;       
        mAudioOutputMode = AUDIO_OUTPUT_MODE_SPDIF_COMPRESS;
		ALOGD("SPDIFHardwareStub creator: SPDIF Compress Data End");
    }
}

SPDIFHardwareStub::~SPDIFHardwareStub()
{
	gprintf("SPDIFHardwareStub destroyed");
}


status_t SPDIFHardwareStub::start(void)
{
    int err = NO_ERROR;
    pcm* h;		// Planet 20120126
    uint32_t channels = mConfig.channels;
	char value[PROPERTY_VALUE_MAX];

	gprintf("SPDIF device start in");

    if(mSpdifPcm != NULL)
		return NO_ERROR;

    if (mAudioOutputMode == AUDIO_OUTPUT_MODE_DAI_LPCM)         // Multichannel LPCM
    {
        ALOGD("start()    open default(LPCM) channel(%d), samplerate(%d) ", mConfig.channels, mConfig.rate);
        if(mConfig.channels > 2)
        {
			mConfig.channels = 8;
        }
		else
		{
			mConfig.channels = 2;
		}
		h = pcm_open(0, 0, PCM_OUT , &mConfig);
    }
    else if (mAudioOutputMode == AUDIO_OUTPUT_MODE_DAI_HBR)    // HBR
    {
		char hdmi_audio[PROPERTY_VALUE_MAX];
		memset(hdmi_audio, NULL, PROPERTY_VALUE_MAX);
		property_get("tcc.hdmi.audio_type", hdmi_audio, "0");

        ALOGD("start()    open default(HBR) hdmi_audio(%c) channel(%d), samplerate(%d) ", 
                                            hdmi_audio[0], mConfig.channels, mConfig.rate);
		if(hdmi_audio[0]=='1') {
			mConfig.channels = 2;
			mConfig.rate = mConfig.rate *4;
			if(mConfig.rate > 192000)
				mConfig.rate = 192000;
		}
		else if(hdmi_audio[0]=='2') {
			mConfig.channels = 2;
		}
		else if(hdmi_audio[0]=='3') {
			if(mConfig.channels > 2)
				mConfig.channels = 8;
			else
				mConfig.channels = 2;			
		}
		else{
            mConfig.rate = 192000;
            mConfig.channels = 8;
        }

        h = pcm_open(0, 0, PCM_OUT , &mConfig);
    }
    else {
		char hdmi_audio[PROPERTY_VALUE_MAX];
		memset(hdmi_audio, NULL, PROPERTY_VALUE_MAX);
		property_get("tcc.hdmi.audio_type", hdmi_audio, "0");
        mConfig.channels   = 2;

        if(hdmi_audio[0]!='2')
            mConfig.format = PCM_FORMAT_U16_LE;	// Planet 20120306

        ALOGD("start()    open default(SPDIF) hdmi_audio(%c) channel(%d), samplerate(%d) ", 
                                            hdmi_audio[0], mConfig.channels, mConfig.rate);
		
		h = pcm_open(0, 1, PCM_OUT, &mConfig);
    }

	if (!pcm_is_ready(h)) {
        ALOGE("cannot open pcm_out driver: %s", pcm_get_error(h));
        pcm_close(h);
        //adev->active_output = NULL;
        return -ENOMEM;
	}

    if(h == NULL) {
        ALOGE("SPDIF device start failure.. ");
        return (-1);
    }
	else
	{
		mSpdifPcm = h;
	}

	memset(value, NULL, PROPERTY_VALUE_MAX);
	value[0] = (char)mConfig.channels;
	value[0] += '0';
	property_set("tcc.audio.channels", value);

	memset(value, NULL, PROPERTY_VALUE_MAX);
    switch(mConfig.rate) {
        case 32000:
            value[0] = (char)SF_32KHZ;
            break;
        default:
        case 44100:
            value[0] = (char)SF_44KHZ;
            break;
        case 88200:
            value[0] = (char)SF_88KHZ;
            break;
        case 176000:
            value[0] = (char)SF_176KHZ;
            break;
        case 48000:
            value[0] = (char)SF_48KHZ;
            break;
        case 96000:
            value[0] = (char)SF_96KHZ;
            break;
        case 192000:
            value[0] = (char)SF_192KHZ;
            break;
    }
	
	value[0] += '0';
    property_set("tcc.audio.sampling_rate", value);

    gprintf("SPDIF device start out\n");
    
	return NO_ERROR;
}

ssize_t SPDIFHardwareStub::write(const void* buffer, size_t size)
{
    size_t written = 0;
    size_t n = 0;
	int kernel_frames;

#if 0
    FILE *dump_fp = NULL;

    dump_fp = fopen("/data/dump_out", "a+");
    if (dump_fp != NULL) {
        fwrite(buffer, size, 1, dump_fp);
        fclose(dump_fp);
    }
    else {
        ALOGW("[Error] Don't write to /data/dump_out.pcm");
    }
#endif

	 //mSpdifPcm.write_threshold = 4 * 512;

	if (mSpdifPcm == NULL) {
        gprintf("SPDIF handler is NULL");
    }
    else {
		n = pcm_write(mSpdifPcm, (char *)buffer, size);
        
        if (n == -EBADFD) {
            // Somehow the stream is in a bad state. The driver probably
            // has a bug and snd_pcm_recover() doesn't seem to handle this.
            pcm_close(mSpdifPcm);
            mSpdifPcm = NULL;
            start();
			ALOGE("spdif alsa prepare [BADFD error]\n");
        }
		else if(n == -EPIPE)
		{
			//ALOGE("spdif ALSA Underrun.. %d\n",SPDIF_FIFO_GetLength());
			//snd_pcm_prepare(mSpdifHandle);		// Planet 20120126
			n = 0;
			ALOGE("spdif alsa prepare [PIPE error]\n");
		} 
        else if(n < 0) {
			ALOGE("spdif alsa write fail!!!.. %d\n", n);
            return size;
		}
    }
	
    return size;
}

void SPDIFHardwareStub::stop(void)
{
    gprintf("SPDIF device stop");

    if(mSpdifPcm == NULL)
        gprintf("SPDIF handler is NULL");
    else {
        pcm_close(mSpdifPcm);
        mSpdifPcm = NULL;
    }

    gprintf("SPDIF device stop done...");
}


void SPDIFHardwareStub::flush(void)
{
    gprintf("SPDIF device flush");
}

void SPDIFHardwareStub::pause(void)
{
    gprintf("SPDIF device pause");
    if(mSpdifPcm == NULL)
        gprintf("SPDIF handler is NULL");
    else {
        pcm_close(mSpdifPcm);
        mSpdifPcm = NULL;
    }
    gprintf("SPDIF device pause done...");
}

void SPDIFHardwareStub::setSampleRate(int sampleRate)
{
    ALOGD("SPDIF device setSampleRate=%d", sampleRate);
    mConfig.rate = sampleRate;
}

void SPDIFHardwareStub::setChannelCount(int channel)
{
	ALOGD("SPDIF device setChannelCount() Chn : %d !!!!!", channel);
	mConfig.channels = channel;
}

void SPDIFHardwareStub::setFrameSize(int framesize)
{
	gprintf("Start setFrameSize() size:%d !!!!!", framesize);
}

sp<IMemory> SPDIFHardwareStub::getCblk() const
{
	gprintf("Start getCblk() !!!!!");
    return mCblkMemory;
}

SPDIFHardwareInterface *SPDIFHardwareStub::create()
{
    return new SPDIFHardwareStub();
}

extern "C" SPDIFHardwareInterface *createSPDIFHardware(void)
{
	gprintf("openSPDIFHardware()");
    return android::SPDIFHardwareStub::create();
}

}; // namespace android

