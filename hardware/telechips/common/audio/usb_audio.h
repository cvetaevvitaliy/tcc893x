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

#ifndef ANDROID_USB_AUDIO_PARSER_H_
#define ANDROID_USB_AUDIO_PARSER_H_

#include <cutils/log.h>
#include <pthread.h>
#include <dirent.h>

#if __cplusplus
extern "C" {
#endif

enum{
    USB_AUDIO_TYPE_PLAYBACK,
    USB_AUDIO_TYPE_CAPTURE
};

typedef struct
{
    int format;
    int channels;
    int rate;    
}usb_audio_prop;

int loadUsbAudioProperties(int card_num, int audio_type, usb_audio_prop * audio_prop);

#if __cplusplus
}  // extern "C"
#endif


#endif /*ANDROID_USB_AUDIO_PARSER_H_*/
