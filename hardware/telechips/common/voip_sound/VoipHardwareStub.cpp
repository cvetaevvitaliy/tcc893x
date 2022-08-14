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
#include <unistd.h> // for getpid

//#define LOG_TAG "GtestHardwareStub"
#include <utils/Log.h>

#include <utils/threads.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <media/AudioSystem.h>

#include <cutils/properties.h> // for property_get

//#include "mach/audio.h" // For HDMI, B090183
#include "VoipHardwareStub.h"

#undef LOG_TAG
#define LOG_TAG "VoipHardwareStub"

namespace android {

#define VOIP_DEBUG_ON
#undef VOIP_DEBUG_ON

#if defined(VOIP_DEBUG_ON)
#define  GET_SHORT_FILENAME(longfilename)   \
    ( (strrchr(longfilename, '/')==NULL) ? \
        longfilename: ((char*)(strrchr(longfilename, '/')+1)))


#define gprintf(fmt, args...)      \
    ALOGE("[%d], %s:%d, %s(): "fmt,           \
        gettid(), GET_SHORT_FILENAME(__FILE__), __LINE__, __FUNCTION__, ##args)                         
#else

#define gprintf
#endif

#define TCC_CLOCKCTRL_VOIP_MAX_CLOCK_DISABLE 106
#define TCC_CLOCKCTRL_VOIP_MAX_CLOCK_ENABLE  107
static int mOpenCount = 0;

#ifdef USE_VOIP_CLOCK_THREAD
static VoipClockThread * mVoipClockThread = 0;
VoipClockThread::VoipClockThread(): Thread(false), runState(1), discountTime(-1), callBackFn(0)
{
	int dev_fd = -1;
    int ret_val;

	dev_fd = open("/dev/clockctrl", O_WRONLY);
	
	if(dev_fd < 0)
	{
		if (ENOENT != errno )
		{
			ALOGE("clockctrl open error!!\n");
		}
	}
    else
    {
    	ret_val = ioctl(dev_fd, TCC_CLOCKCTRL_VOIP_MAX_CLOCK_ENABLE, 1);
    	close(dev_fd);        
    }       

    ALOGD("Create VoipClockThread");
}

bool VoipClockThread::threadLoop()
{
    while(runState)
    {
        usleep(THREAD_CHECK_TIME*1000);

        if(discountTime>0)
        {
            discountTime = ( discountTime > THREAD_CHECK_TIME)?(discountTime -THREAD_CHECK_TIME):0;
        }
        else if(discountTime == 0)
        {
        	int dev_fd = -1;
            int ret_val;

        	dev_fd = open("/dev/clockctrl", O_WRONLY);
        	
        	if(dev_fd < 0)
        	{
        		if (ENOENT != errno )
        		{
        			ALOGE("clockctrl open error!!\n");
        		}
        	}
            else
            {
            	ret_val = ioctl(dev_fd, TCC_CLOCKCTRL_VOIP_MAX_CLOCK_DISABLE, 1);
            	close(dev_fd);
            }

            runState = 0;
            discountTime = -1;

            ALOGD("Execute the loop by time out");
        }
    }
    
    return false;
}

VoipClockThread::~VoipClockThread()
{
	int dev_fd = -1;
    int ret_val;

	dev_fd = open("/dev/clockctrl", O_WRONLY);
	
	if(dev_fd < 0)
	{
		if (ENOENT != errno )
		{
			ALOGE("clockctrl open error!!\n");
		}
	}
    else
    {
    	ret_val = ioctl(dev_fd, TCC_CLOCKCTRL_VOIP_MAX_CLOCK_DISABLE, 1);
    	close(dev_fd);        
    }       

    discountTime = -1;
    ALOGD("Destroyed VoipClockThread");
}
#endif

Mutex VoipHardwareStub::mLock;
float VoipHardwareStub::mVolume = 1.0f;

VoipHardwareStub::VoipHardwareStub(uint32_t mode)
{
    ALOGD("VoipHardwareStub creator %d", mode);

    mMode       = mode;
    mFrameSize  = 4;
    mRequestChannels = 2;

	mConfig.channels = 2;
	mConfig.rate = 44100;
	mConfig.period_size = 512;
	mConfig.period_count = 4;
	mConfig.format = PCM_FORMAT_S16_LE;
	mConfig.start_threshold = 1;
	mConfig.stop_threshold = 512*4;
	mConfig.silence_threshold = 0;
	mConfig.avail_min = 512;

    mVoipHandle = NULL;

}

VoipHardwareStub::~VoipHardwareStub()
{
	gprintf("VoipHardwareStub destroyed");
}

static int voip_clock_set(bool mode)
{
    if(mode)
    {
    	int dev_fd = -1;
        int ret_val;

    	dev_fd = open("/dev/clockctrl", O_WRONLY);
    	
    	if(dev_fd < 0)
    	{
   			ALOGE("clockctrl open error!! %d\n", dev_fd );
    	}
        else
        {
#ifdef USE_VOIP_CLOCK_THREAD
        	ret_val = ioctl(dev_fd, TCC_CLOCKCTRL_VOIP_MAX_CLOCK_ENABLE, 0);
#else
        	ret_val = ioctl(dev_fd, TCC_CLOCKCTRL_VOIP_MAX_CLOCK_ENABLE, 1);
#endif

        	close(dev_fd);        
        }

    }
    else
    {
    	int dev_fd = -1;
        int ret_val;

    	dev_fd = open("/dev/clockctrl", O_WRONLY);
    	
    	if(dev_fd < 0)
    	{
   			ALOGE("clockctrl open error!! %d\n", dev_fd );
    	}
        else
        {
#ifdef USE_VOIP_CLOCK_THREAD
        	ret_val = ioctl(dev_fd, TCC_CLOCKCTRL_VOIP_MAX_CLOCK_DISABLE, 0);
#else
        	ret_val = ioctl(dev_fd, TCC_CLOCKCTRL_VOIP_MAX_CLOCK_DISABLE, 1);
#endif
        	close(dev_fd);        
        }       
    }

    return 0;
}

status_t VoipHardwareStub::open(int b, int l, int p)
{
    int err = NO_ERROR;
    int voip_pid;

    AutoMutex lock(mLock);
    AutoMutex lock2(mAccessLock);

	gprintf("Voip device start in");

    ALOGD("mode(%d): b(%d), l(%d), p(%d)", mMode, b, l,p);

    if(mVoipHandle != NULL)
    {
        ALOGE("Error: Already opened");        
        return -1;
    }
    else if(p < 10000)
    {
        
        ALOGE("Error: too short period, it must be over 100000");        
        return NO_ERROR;
    }

    mConfig.period_size = (mConfig.rate * (p/1000))/1000;

    if(mConfig.period_size)
    {
        int comp_val;
        int rest;
        int i;

        for(i=31; i>=0; i--)
        {
            if(mConfig.period_size & (0x00000001 << i))
                break;
        }

        comp_val = 0x00000001 << i;

        if(comp_val < mConfig.period_size)
        {
            rest = mConfig.period_size - comp_val;
            
            if(rest >= (comp_val/2))
                mConfig.period_size = comp_val << 0x01;
            else
                mConfig.period_size = comp_val;                       
        }            
    }
    else
    {
        ALOGD("mConfig.period_size %d", mConfig.period_size);    
        return -1;
    }

    mConfig.period_count = b / mConfig.period_size;

    if(mConfig.period_count < 2)
    {
        ALOGE("Error: too small mConfig.period_count, it must be over 2");        

    }

	mConfig.stop_threshold = mConfig.period_size*mConfig.period_count;
	mConfig.avail_min = mConfig.period_size;

    ALOGD("mConfig.period_size %d", mConfig.period_size);
    ALOGD("mConfig.period_count %d", mConfig.period_count);
    
    voip_pid= getpid();    
    if(0 > AudioSystem::setVoipPid(1, voip_pid))
    {
        ALOGE("fail to register pid to audioflinger");
    }

    err = AudioSystem::setVoipMode(1);
    ALOGD("AudioSystem::setVoipMode(1), reuslt(%d)", err);
    
    if(mOpenCount++ == 0)
    {
        voip_clock_set(1);

#ifdef USE_VOIP_CLOCK_THREAD
        mVoipClockThread = new VoipClockThread;
        mVoipClockThread->run();
        mVoipClockThread->setDiscountTime(AFTER_RECOVERY_TIME);
#endif
    }

    if(mMode == TCC_VOIP_DIRECTION_PLAYBACK)
    	mVoipHandle = pcm_open(0, 0, PCM_OUT, &mConfig);	// Planet 20120126
    else
    	mVoipHandle = pcm_open(0, 0, PCM_IN,  &mConfig);	// Planet 20120126        

    if(!pcm_is_ready(mVoipHandle))
	{
        AudioSystem::setVoipPid(0, voip_pid);

        if(mOpenCount > 0)
        {
            if(--mOpenCount == 0)
            {
                voip_clock_set(0);
                AudioSystem::setVoipMode(0);

#ifdef USE_VOIP_CLOCK_THREAD
                if(mVoipClockThread)
                {
                   delete mVoipClockThread;
                   mVoipClockThread = NULL;
                }
#endif
            }
        }

		ALOGE("pcm not open!!!!!! : %s", pcm_get_error(mVoipHandle));
        pcm_close(mVoipHandle);
        return -1;
	}

	return err;
}


status_t VoipHardwareStub::close(void)
{
    status_t err = NO_ERROR;
    int voip_pid;

    AutoMutex lock(mLock);
    AutoMutex lock2(mAccessLock);


    if(mVoipHandle != NULL && pcm_is_ready(mVoipHandle))
    {
        pcm_close(mVoipHandle);
        mVoipHandle = NULL;
    }

    if(mOpenCount > 0)
    {
        if(--mOpenCount == 0)
        {
            voip_clock_set(0);
            AudioSystem::setVoipMode(0);
        }


#ifdef USE_VOIP_CLOCK_THREAD
        if(mVoipClockThread)
        {
           delete mVoipClockThread;
           mVoipClockThread = NULL;
        }
#endif
    }

    voip_pid= getpid();    
    if(0 > AudioSystem::setVoipPid(0, voip_pid))
    {
        ALOGE("fail to unregister pid at audioflinger");
    }

    ALOGD("mode(%d): close", mMode);
    return err;
}


status_t VoipHardwareStub::start(void)
{
	ALOGD("mMode(%d), Voip device start in", mMode);

    if(!pcm_is_ready(mVoipHandle))
        return NO_ERROR;

    return open(4096, 80000, 10000);
}

int VoipHardwareStub::write(const void* buffer, size_t size)
{
    size_t remain_size = 0;
    size_t written = 0;
    const size_t max_size = PCM_TEMP_BUFF_SIZE/2;
    short *sbuffer;
    status_t err = 0;

    AutoMutex lock(mAccessLock);


    if(!pcm_is_ready(mVoipHandle)){
        ALOGE("Voip handler write is NULL");
        return (-1);
    }
    else {
    	
        if(mRequestChannels == 1){
            remain_size = size;
            sbuffer = (short*)buffer;
            
            while(remain_size)
            {
                if(remain_size > max_size)
                {                   
                    pcm_mono_to_stereo(&sbuffer[written], mWriteTempPcmBuff, max_size);

                    err = pcm_write(mVoipHandle, mWriteTempPcmBuff, PCM_TEMP_BUFF_SIZE);
                    if(err < 0)
                        break;

                    if(pcm_check_underrun(mVoipHandle))
                        ALOGE("VoipHardwareStub::write pipe error %d", pcm_check_underrun(mVoipHandle));
                        
                    remain_size -= max_size;
                    written += max_size;
                }
                else
                {
                    pcm_mono_to_stereo(&sbuffer[written], mWriteTempPcmBuff, remain_size);
                    err = pcm_write(mVoipHandle, mWriteTempPcmBuff, remain_size*2);
                    if(err < 0)
                        break;

                    if(pcm_check_underrun(mVoipHandle))
                        ALOGE("VoipHardwareStub::write pipe error %d", pcm_check_underrun(mVoipHandle));
                    remain_size = 0;
                }
            }
        }
        else{
            err = pcm_write(mVoipHandle, (char *)buffer, size); // Planet 20120126
            if(pcm_check_underrun(mVoipHandle))
                ALOGE("VoipHardwareStub::write pipe error %d", pcm_check_underrun(mVoipHandle));
        }
      
        if(err < 0) {
			ALOGE("Voip alsa write fail!!!.. %s\n", pcm_get_error(mVoipHandle));
            return err;
		}
			
    }

//    gprintf("write: buffer=0x%p, bytes=%d, size=%d, frames=%d", buffer, written, size, snd_pcm_bytes_to_frames(mVoipHandle, size));
    return size;
}


int VoipHardwareStub::read(const void* buffer, size_t size)
{
    status_t err = 0;
    size_t remain_size = 0;
    size_t readbytes = 0;
    const size_t max_size = PCM_TEMP_BUFF_SIZE/2;
    short *sbuffer;

    AutoMutex lock(mAccessLock);

    if(!pcm_is_ready(mVoipHandle)){
        ALOGE("Voip handler read is NULL");
        return -1;
    }
    else {    	
		if(mRequestChannels == 1){
            remain_size = size;
            sbuffer = (short*)buffer;
            
            while(remain_size)
            {
                if(remain_size > max_size)
                {                   
        			err = pcm_read(mVoipHandle, mReadTempPcmBuff, PCM_TEMP_BUFF_SIZE);	// Planet 20120126
        			if(err < 0)
                        break;

                    if(pcm_check_underrun(mVoipHandle))
                        ALOGE("VoipHardwareStub::read pipe error %d", pcm_check_underrun(mVoipHandle));
                    
        			pcm_stereo_to_mono(mReadTempPcmBuff, &sbuffer[readbytes], max_size);
                    remain_size -= max_size;
                    readbytes += max_size;
                }
                else
                {
        			err = pcm_read(mVoipHandle, mReadTempPcmBuff, remain_size*2);	// Planet 20120126
        			if(err < 0)
                        break;

                    if(pcm_check_underrun(mVoipHandle))
                        ALOGE("VoipHardwareStub::read pipe error %d", pcm_check_underrun(mVoipHandle));

        			pcm_stereo_to_mono(mReadTempPcmBuff, &sbuffer[readbytes], remain_size);
                    remain_size = 0;
                }
            }
		}
		else{
			err = pcm_read(mVoipHandle, (char *)buffer, size);	// Planet 20120126
            if(pcm_check_underrun(mVoipHandle))
                ALOGE("VoipHardwareStub::read pipe error %d", pcm_check_underrun(mVoipHandle));
		}
        
        if(err < 0) {
			ALOGE("Voip alsa read fail!!!.. %s\n", pcm_get_error(mVoipHandle));
            return err;
		}
			
    }

//    return static_cast<ssize_t>(snd_pcm_frames_to_bytes(mVoipHandle, n));
    return size;
}

void VoipHardwareStub::stop(void)
{
    gprintf("Voip device stop");

    close();

    gprintf("Voip device stop done...");
}


void VoipHardwareStub::flush(void)
{
    gprintf("Voip device flush");
}

void VoipHardwareStub::pause(void)
{
    gprintf("Voip device pause");

    close();

    gprintf("Voip device pause done...");
}

void VoipHardwareStub::setSampleRate(int sampleRate)
{

    gprintf("Voip device setSampleRate=%d", sampleRate);
	mConfig.rate = sampleRate;
}

void VoipHardwareStub::setChannelCount(int channel)
{

    gprintf("Voip device setChannelCount=%d", channel);

//	mConfig.channels = channel;
		mRequestChannels = channel;
}

void VoipHardwareStub::setFrameSize(int framesize)
{
    mFrameSize = framesize;
}

int VoipHardwareStub::getDelay(long *delayp)
{
   int err = 0;
//   err = pcm_avail_update(mVoipHandle);
   return err;
}

int VoipHardwareStub::setRoute(int device)
{
/*
    const sp<IAudioFlinger>& af = AudioSystem::get_audio_flinger();
    if (af == 0) return PERMISSION_DENIED;

    AudioParameter param = AudioParameter();
    param.addInt(String8(AudioParameter::keyRouting), (int)device);


    return af->setParameters(0, param.toString());
*/
	return 0;    
}

int VoipHardwareStub::setMute(bool mute)
{
    // To do.
    ALOGD("setMute(%d)", mute);
    
    return 0;
}

int VoipHardwareStub::setVolume(float volume)
{
    // To do.
    ALOGD("setVolume(%f)", volume);
    
    mVolume = volume;
    return 0;
}

void VoipHardwareStub::pcm_stereo_to_mono(void *input, void *output, int size)
{	
    size /=2;
    for(int i = 0; i < size; i++){
        ((short *)output)[i] = ((short *)input)[i*2];											
    }	
}

void VoipHardwareStub::pcm_mono_to_stereo(void *input, void *output, int size)
{
    int out_idx = 0;    

    size /=2;
    for(int i = 0; i < size; i++, out_idx+=2){							
        ((short *)output)[out_idx] = ((short *)input)[i];						
        ((short *)output)[out_idx+1] = ((short *)input)[i];
    }	
}

VoipHardwareInterface *VoipHardwareStub::create(uint32_t mode)
{
    return new VoipHardwareStub(mode);
}

extern "C" VoipHardwareInterface *createVoipHardware(uint32_t mode)
{
	gprintf("openVoipHardware()");
    return android::VoipHardwareStub::create(mode);
}

}; // namespace android

