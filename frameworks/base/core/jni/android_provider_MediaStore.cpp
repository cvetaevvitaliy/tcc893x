/*
 * Copyright (C) 2007 The Android Open Source Project
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
#define LOG_TAG "MediaStore-JNI"

// Set to 0 to enable verbose logging
#define LOG_NDEBUG 1


#include "utils/Log.h"

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <utils/threads.h>
#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include "utils/Errors.h"  // for status_t
#include "android_util_Binder.h"
#include <binder/Parcel.h>
#include <sys/ioctl.h>

namespace android {
// ----------------------------------------------------------------------------

#include <sys/mman.h>
#include <fcntl.h>

#ifdef _TCC9300_
#define CLK_CTLR_BASE               0xB0500000
#elif defined(_TCC8800_)
#define CLK_CTLR_BASE               0xF0400000
#else //_TCC8920_ || _TCC8930_
#define CLK_CTLR_BASE               0x74000000
#endif

#define CLK_VBUS_IDX                5
#define CLK_VCORE_IDX               6
#define CLK_CHECK_ENABLE            0x00200000

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define BUF_SIZE    (CLK_VCORE_IDX + 2)

#define VPU_CHECK_CODEC_STATUS  200

#define PRINTF(x...) ALOGD(x)


typedef enum {
	VPU_DEC = 0,
	VPU_DEC_EXT,	
	VPU_DEC_EXT2,
	VPU_DEC_EXT3,
	VPU_ENC,
	VPU_ENC_EXT,
	VPU_ENC_EXT2,
	VPU_ENC_EXT3,
	VPU_MAX
}vputype;


static int isVPURunningCheck()
{
    int dev_fd = -1;
    void *map_base;
    unsigned int buf[BUF_SIZE];
    int i;
    int ret = 0;

#if defined(_TCC9300_)  || defined(_TCC8920_) //ZzaU test until clock is controlled!!
    return 0;
#endif

    dev_fd = open("/dev/tmem", O_RDWR | O_NDELAY);
    if(!dev_fd)
    {
        PRINTF("physical memory open error!!\n");
        return -1;
    }

    map_base = (void *)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, CLK_CTLR_BASE & ~MAP_MASK);
    if(map_base == (void *) - 1)
    {
        PRINTF("isVPURunningCheck map_base error \n");
        ret = -1;
        goto error;
    }

    memcpy(buf,(unsigned int *)map_base,BUF_SIZE*4);

    if((buf[CLK_VBUS_IDX] & CLK_CHECK_ENABLE) && (buf[CLK_VCORE_IDX] & CLK_CHECK_ENABLE))
    {
        PRINTF("VPU is Running !! \n");
        ret = 1;
    }

    for(i =0 ; i < BUF_SIZE ; i++)
    {
        if(buf[i] & CLK_CHECK_ENABLE){
           PRINTF("buf[%d] %x \n",i,buf[i]);
        }
    }

    if (munmap(map_base, MAP_SIZE) == -1) { 
        PRINTF("munmap error!!\n");
        ret = -1;
        goto error;
    }

error:
    close(dev_fd);	

    return ret;
}

static int isVPU_DualRunningCheck()
{
	int dev_fd = -1;
	int ret = 0;
	int vpu_closed[VPU_MAX];

	dev_fd = open("/dev/vpu_mgr", O_RDWR | O_NDELAY);
	if( dev_fd <= 0 ){
	      PRINTF("vpu_mgr open error!!");
		return -1;
	}          
	                                
	if( 0 <= ioctl(dev_fd, VPU_CHECK_CODEC_STATUS, &vpu_closed) ){
	      if( vpu_closed[VPU_DEC] == 0 && vpu_closed[VPU_DEC_EXT] == 0 ){
	            PRINTF("VPU is Dual-Running !!");
	            ret = 1;
		}
	}

error:
close(dev_fd);

return ret;
}


static int
android_provider_MediaStore_check_vpu_running(JNIEnv *env, jobject thiz)
{
    int isRunning;

    //printf("##### android_media_MediaScanner_native_finalize: ctx=0x%p\n", ctx);

//    isRunning = isVPURunningCheck();
	 isRunning = isVPU_DualRunningCheck();

    return isRunning;
}

static JNINativeMethod gMethods[] = {
    {"check_vpu_running",       "()I",                   (void *)android_provider_MediaStore_check_vpu_running},
};

// This function only registers the native methods
int register_android_provider_MediaStore(JNIEnv *env)
{
    //printf("#### Called register_android_provider_MediaStore()");

    return AndroidRuntime::registerNativeMethods(env,
                "android/provider/MediaStore", gMethods, NELEM(gMethods));
}

}; // namespace android

