/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "USB_AUDIO_PARSER"
//#define LOG_NDEBUG 0

#include "usb_audio.h"
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <tinyalsa/asoundlib.h>

#include <cutils/misc.h>


/////////////////////////////////////////////////
//      Local functions
/////////////////////////////////////////////////
#define USB_AUDIO_CONFIG "/proc/asound/card0/stream0"
#define MAX_BUFF_SIZE   128*1024

#define AUDIO_CAPTURE_TAG   "Capture:"
#define AUDIO_PLAYBACK_TAG  "Playback:"

#define AUDIO_FORMAT_TAG    "Format: " 
#define AUDIO_CHANNELS_TAG  "Channels: "
#define AUDIO_RATES_TAG     "Rates: "

static int getTagData(char* src, char* tag, char* out)
{
    char *start_point;
    char *end_point;
    int size;

    start_point = strstr(src, tag);
    if(start_point == NULL)
    {
        ALOGE("cannot find tag %s", tag);
        return -1;
    }
    
    end_point = strchr(start_point, '\n');

    start_point += strlen(tag);

    size = end_point - start_point;
    memcpy(out, start_point, size);
    out[size] = 0;

    return 0;
    
}


int loadUsbAudioProperties(int card_num, int audio_type, usb_audio_prop * audio_prop)
{
    char path[64];
    char *data;
    int sz;
    int fd;
    char *start_point;
    char *end_point;
    char temp_buff[128];
    int ret = -1;

    strcpy(path, USB_AUDIO_CONFIG);
    path[17] = card_num + '0';

    fd = open(path, O_RDONLY);
    if(fd < 0)
    {
        ALOGE("cannot open %s", path);
        return 0;
    }

    data = (char*) malloc(MAX_BUFF_SIZE);
    sz = read(fd, data, MAX_BUFF_SIZE);
    close(fd);

    if(sz > 0)
    {
        data[sz] = 0;
        ALOGV("load data: %s", data);
    }
    else
    {
        ALOGE("cannot load %s", path);
        ret = -ENODEV;
        goto error_;
    }

    if(audio_type == USB_AUDIO_TYPE_PLAYBACK)
    {
        start_point = strstr(data, AUDIO_PLAYBACK_TAG);
        if(start_point == NULL)
        {
            ALOGE("cannot find tag %s", AUDIO_PLAYBACK_TAG);
            goto error_;
        }
        
        end_point = strstr(start_point, AUDIO_CAPTURE_TAG);

    }
    else
    {
        start_point = strstr(data, AUDIO_CAPTURE_TAG);
        if(start_point == NULL)
        {
            ALOGE("cannot find tag %s", AUDIO_CAPTURE_TAG);
            goto error_;
        }
        
        end_point = strstr(start_point, AUDIO_PLAYBACK_TAG);

    }

    if(end_point == NULL || start_point > end_point)
    {
        end_point = &data[sz];
    }

    sz = end_point - start_point;

    if(sz < 0)
    {
        ALOGE("There is no data, %d", sz);
    }

    end_point[0] = 0;

    
    ret = getTagData(start_point, AUDIO_FORMAT_TAG, temp_buff);
    if(ret < 0)
        ALOGE("cannot find %s" , AUDIO_FORMAT_TAG);
    else
    {
        if(memcmp(temp_buff, "S16_LE", 6) == 0)
            audio_prop->format = PCM_FORMAT_S16_LE;
        else
        {
            ALOGE("%s is not supported", temp_buff);
            ret = -1;
            goto error_;
        }
    }

    ret = getTagData(start_point, AUDIO_CHANNELS_TAG, temp_buff);
    if(ret < 0)
    {
        ALOGE("cannot find %s" , AUDIO_CHANNELS_TAG);
        goto error_;
    }
    else
    {
        int channels;

        channels = atoi(temp_buff);
        if(audio_type == USB_AUDIO_TYPE_PLAYBACK && audio_prop->channels != channels)
        {
            ALOGE("cannot support this device for Playback. Android can support Only 2 channel device");
            ret = -1;
            goto error_;
        }
        else
        {
            if(channels < 1 && channels > 2)
            {
                ALOGE("channels %d is not supported", channels);
                ret = -1;
                goto error_;
            }
            audio_prop->channels = channels;
        }
    }

    ret = getTagData(start_point, AUDIO_RATES_TAG, temp_buff);
    if(ret < 0)
    {
        ALOGE("cannot find %s" , AUDIO_RATES_TAG);
        goto error_;
    }
    else
    {
        if(audio_type == USB_AUDIO_TYPE_PLAYBACK)
        {
            char * temp_ret = NULL;

            if(audio_prop->rate == 44100)
            {
                temp_ret = strstr(start_point, "44100");
            }
            else if(audio_prop->rate == 48000)
            {
                temp_ret = strstr(start_point, "48000");
            }
                
            if(temp_ret == NULL)
            {
                ALOGE("cannot support this device for Playback. This devices doesn't support %d Hz", audio_prop->rate);
                ret = -1;
                goto error_;
            }
        }
        else
        {

		// Planet20130612 ipod-support Start
            char * temp_ret = NULL;

            if(audio_prop->rate == 8000)
                temp_ret = strstr(start_point, " 8000");
            else if(audio_prop->rate == 11025)
                temp_ret = strstr(start_point, "11025");
            else if(audio_prop->rate == 12000)
                temp_ret = strstr(start_point, "12000");
            else if(audio_prop->rate == 16000)
                temp_ret = strstr(start_point, "16000");
            else if(audio_prop->rate == 22050)
                temp_ret = strstr(start_point, "22050");
            else if(audio_prop->rate == 24000)
                temp_ret = strstr(start_point, "24000");
            else if(audio_prop->rate == 32000)
                temp_ret = strstr(start_point, "32000");
            else if(audio_prop->rate == 44100)
                temp_ret = strstr(start_point, "44100");
            else if(audio_prop->rate == 48000)
                temp_ret = strstr(start_point, "48000");

            if(temp_ret == NULL){
                ALOGE("sample rate  %d is changed to %d", audio_prop->rate, atoi(temp_buff));
                audio_prop->rate = atoi(temp_buff);
            }

            if(audio_prop->rate < 8000 && audio_prop->rate > 48000)
                ALOGE("sample rate %d is not supported", audio_prop->rate);
		// Planet20130612 ipod-support End
        }
    }

    if(1)
    {
        ALOGI("new %s format : %d", (audio_type==USB_AUDIO_TYPE_PLAYBACK)?"out":"in", audio_prop->format);
        ALOGI("new %s channels : %d", (audio_type==USB_AUDIO_TYPE_PLAYBACK)?"out":"in", audio_prop->channels);
        ALOGI("new %s rate : %d", (audio_type==USB_AUDIO_TYPE_PLAYBACK)?"out":"in", audio_prop->rate);
    }

    ret = 0;
error_:
    
    free(data);
    return ret;
}

#if 0
int main() {
    int hdl;
    int ret;
    usb_audio_prop audio_prop;

    ret = loadUsbAudioProperties(1, USB_AUDIO_TYPE_CAPTURE, &audio_prop);

    ret = loadUsbAudioProperties(1, USB_AUDIO_TYPE_PLAYBACK, &audio_prop);

    ALOGV("init() done");
    return 0;
}
#endif


