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

#ifndef ANDROID_HARDWARE_SPDIF_HARDWARE_INTERFACE_H
#define ANDROID_HARDWARE_SPDIF_HARDWARE_INTERFACE_H

#include <utils/RefBase.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>

namespace android {

class SPDIFHardwareInterface : public virtual RefBase {
public:
    virtual ~SPDIFHardwareInterface() { }


    virtual status_t    start(void) = 0;
    virtual ssize_t     write(const void* buffer, size_t size) = 0;
    virtual void        stop(void) = 0;
    virtual void        flush(void) = 0;
    virtual void        pause(void) = 0;
    virtual void        setSampleRate(int sampleRate) = 0;
    virtual void        setChannelCount(int channel) = 0;
    virtual void        setFrameSize(int framesize) = 0;

    virtual sp<IMemory> getCblk() const = 0;

    static SPDIFHardwareInterface* create();

};

/** factory function to instantiate a camera hardware object */
//extern "C" sp<SPDIFHardwareInterface> openSPDIFHardware();
extern "C" SPDIFHardwareInterface* createSPDIFHardware(void);

};  // namespace android

#endif
