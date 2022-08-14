/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_VOIP_HARDWARE_INTERFACE_H
#define ANDROID_HARDWARE_VOIP_HARDWARE_INTERFACE_H


#include <sys/types.h>

#define TCC_VOIP_DIRECTION_PLAYBACK     0
#define TCC_VOIP_DIRECTION_CAPTURE      1

namespace android {

class VoipHardwareInterface /* : public virtual RefBase */ {
public:
    VoipHardwareInterface() { }
    virtual ~VoipHardwareInterface() { }


    virtual int    open(int b, int l, int p) = 0;
    virtual int    close(void) = 0;
    virtual int    start(void) = 0;
    virtual int     write(const void* buffer, size_t size) = 0;
    virtual int     read(const void* buffer, size_t size) = 0;
    virtual void        stop(void) = 0;
    virtual void        flush(void) = 0;
    virtual void        pause(void) = 0;
    virtual void        setSampleRate(int sampleRate) = 0;
    virtual void        setChannelCount(int channel) = 0;
    virtual void        setFrameSize(int framesize) = 0;
    virtual int         getDelay(long *delayp) = 0;
    virtual int		    setRoute(int deviceName) = 0;
    virtual int		    setMute(bool mute) = 0;     // only for input device
    virtual int		    setVolume(float volume) = 0;

    // virtual sp<IMemory> getCblk() const = 0;

    static VoipHardwareInterface* create(uint32_t mode);

};

/** factory function to instantiate a camera hardware object */
//extern "C" sp<VoipHardwareInterface> openVoipHardware();
extern "C" VoipHardwareInterface* createVoipHardware(uint32_t mode);

};  // namespace android

#endif
