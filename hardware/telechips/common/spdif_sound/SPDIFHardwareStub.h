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

#ifndef ANDROID_HARDWARE_SPDIF_HARDWARE_STUB_H
#define ANDROID_HARDWARE_SPDIF_HARDWARE_STUB_H

#if 0
extern "C" {
#include <stdio.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/soundcard.h>      /* For AFMT_* on linux */
}
#endif


#include <utils/threads.h>
#include <hardware_legacy/SPDIFHardwareInterface.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <utils/threads.h>

#include <tinyalsa/asoundlib.h>

namespace android {

class SPDIFHardwareStub : public SPDIFHardwareInterface {
public:
    virtual status_t    start(void);
    virtual ssize_t     write(const void* buffer, size_t size);
    virtual void        stop(void);
    virtual void        flush(void);
    virtual void        pause(void);
    virtual void        setSampleRate(int sampleRate);
    virtual void        setChannelCount(int channel);
    virtual void        setFrameSize(int framesize);

    virtual sp<IMemory> getCblk() const;

    static SPDIFHardwareInterface* create();

private:
                        SPDIFHardwareStub();
    virtual             ~SPDIFHardwareStub();
    int                 inited;

    pcm*				mSpdifPcm;
	struct pcm_config	mConfig;
    sp<IMemory>         mCblkMemory;

    uint32_t            mAudioOutputMode;

};

}; // namespace android

#endif

