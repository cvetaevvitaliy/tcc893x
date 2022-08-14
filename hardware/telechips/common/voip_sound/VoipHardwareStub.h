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

#ifndef ANDROID_HARDWARE_VOIP_HARDWARE_STUB_H
#define ANDROID_HARDWARE_VOIP_HARDWARE_STUB_H

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


#include <hardware_legacy/VoipHardwareInterface.h>
//#include <binder/MemoryBase.h>
//#include <binder/MemoryHeapBase.h>
//#include <utils/threads.h>

#include <tinyalsa/asoundlib.h>


//#define USE_VOIP_CLOCK_THREAD

namespace android {

#ifdef USE_VOIP_CLOCK_THREAD
class VoipClockThread : public Thread {
#define THREAD_CHECK_TIME   100     // ms
#define AFTER_RECOVERY_TIME	10000	// ms
public:
	VoipClockThread(): Thread(false), runState(1), discountTime(0), callBackFn(0){ }
	~VoipClockThread();

	void setDiscountTime(int msTime){ discountTime = msTime;}

private:
	bool		runState; 
	int 		discountTime;
	void 	(*callBackFn)(void);
	virtual bool threadLoop();
};
#endif

class VoipHardwareStub : public VoipHardwareInterface {
public:
	virtual int   	 	open(int b, int l, int p);
	virtual int   		close(void);
	virtual int 	    start(void);
    virtual int			write(const void* buffer, size_t size);
    virtual int 	    read(const void* buffer, size_t size);
    virtual void        stop(void);
    virtual void        flush(void);
    virtual void        pause(void);
    virtual void        setSampleRate(int sampleRate);
    virtual void        setChannelCount(int channel);
    virtual void        setFrameSize(int framesize);
    virtual int         getDelay(long *delayp);
    virtual int		    setRoute(int device);
    virtual int		    setMute(bool mute);
    virtual int		    setVolume(float volume);

    static VoipHardwareInterface* create(uint32_t mode);

private:
                        VoipHardwareStub(uint32_t mode);
    virtual             ~VoipHardwareStub();
    void 				pcm_stereo_to_mono(void *input, void *output, int size);
    void 				pcm_mono_to_stereo(void *input, void *output, int size);

    int                 inited;
    struct pcm*			mVoipHandle;
	struct pcm_config	mConfig;

    static Mutex        mLock;
    Mutex               mAccessLock;

    uint32_t            mMode;
    uint32_t            mFrameSize;
	uint32_t			mRequestChannels;

    static float        mVolume;

#define PCM_TEMP_BUFF_SIZE      4096
    char               mWriteTempPcmBuff[PCM_TEMP_BUFF_SIZE];
    char               mReadTempPcmBuff[PCM_TEMP_BUFF_SIZE];

};

}; // namespace android

#endif

