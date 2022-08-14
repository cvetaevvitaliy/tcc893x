/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

/****************************************************************************

  Revision History

 ****************************************************************************

 ****************************************************************************/
//#define LOG_NDEBUG 0
#define LOG_TAG	"TCC_DXB_SURFACE"

#include <utils/Log.h>
#include <cutils/properties.h>
#include <sys/mman.h>
#include <libpmap/pmap.h>
#include <mach/tcc_video_private.h>
#include <fcntl.h>
#include <dirent.h>

#include <OMX_IVCommon.h>

#include "tcc_dxb_surface.h"

#define TMEM_DEVICE "/dev/tmem"

extern int TCCDxB_ScalerOpen(void);
extern void TCCDxB_ScalerClose(void);
extern int TCCDxB_ScalerCopyData(unsigned int width, unsigned int height, unsigned char *YSrc, unsigned char *USrc, unsigned char *VSrc, 
								char bSrcYUVInter, unsigned char *addrDst, unsigned char ignoreAligne, int fieldInfomation, int interlaceMode);
extern int TCCDxB_CaptureImage(unsigned int width, unsigned int height, int *yuvbuffer, int format, int interlaceMode, int fieldInfomation, unsigned char *strFilePath);

using namespace android;

TCCDxBSurface::TCCDxBSurface(int flag)
{
    hw_module_t const* module;
	char value[PROPERTY_VALUE_MAX];

	property_get("tcc.dxb.hwc.disable", value, "0");
	mbUseOnlyMali = (atoi(value) == 1) ? 0 : flag;
	mWidth = mHeight = mFormat = mFlags = mOutMode = 0;
	mValidSurface = 1; //0:invalid, 1:valid
	mNativeWindow = NULL;

	pthread_mutex_init(&m_mutex, NULL);

    if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module) == 0)
    {
        m_grallocModule = (gralloc_module_t const *)module;
    }

	mTmem_fd = open(TMEM_DEVICE, O_RDWR);
	if (mTmem_fd >= 0)
	{
		pmap_get_info("ump_reserved", &mUmpReservedPmap);
		if( ( mTMapInfo = (void*)mmap(0, mUmpReservedPmap.size, PROT_READ | PROT_WRITE, MAP_SHARED, mTmem_fd, mUmpReservedPmap.base) ) == MAP_FAILED )
		{
			ALOGE("/dev/tmem device's mmap failed.");
			close(mTmem_fd);
			mTmem_fd = -1;
		}
	}
	else
	{
		ALOGE("can't open[%s] '%s'", strerror(errno), TMEM_DEVICE);
    }

	TCCDxB_ScalerOpen();

	ALOGV("%s - Use VideoOption = UseMali(%d)", __func__, mbUseOnlyMali);
}

TCCDxBSurface::~TCCDxBSurface(void)
{
	ALOGV("%s", __func__);

	pthread_mutex_lock(&m_mutex);
	{
		TCCDxB_ScalerClose();

		if(mTmem_fd >= 0)
		{
			munmap((void*)mTMapInfo, mUmpReservedPmap.size);
			if(close(mTmem_fd) < 0)
			{
				ALOGE("Error: close(omx_private->mTmem_fd)");
			}
		}
		mNativeWindow = NULL;
		mValidSurface = 0;
		mWidth = mHeight = mFormat = mFlags = mOutMode = 0;
	}
	pthread_mutex_unlock(&m_mutex);

	pthread_mutex_destroy(&m_mutex);
}

int TCCDxBSurface::SetVideoSurface(ANativeWindow *native)
{
	ALOGV("%s", __func__);

	pthread_mutex_lock(&m_mutex);
	{
		mNativeWindow = native;
		SetNativeWindow();
	}
	pthread_mutex_unlock(&m_mutex);

	return 0;
}

int TCCDxBSurface::UseSurface(void)
{
	ALOGV("%s", __func__);

	pthread_mutex_lock(&m_mutex);
	{
		mValidSurface = 1;
	}
	pthread_mutex_unlock(&m_mutex);

	return 0;
}

int TCCDxBSurface::ReleaseSurface(void)
{
	ALOGV("%s", __func__);

	pthread_mutex_lock(&m_mutex);
	{
		mValidSurface = 0;
	}
	pthread_mutex_unlock(&m_mutex);

	return 0;
}

int TCCDxBSurface::SetSurfaceParm(int width, int height, int format, int flags, int outmode)
{
	int ret;

	pthread_mutex_lock(&m_mutex);
	{
		ret = SetSurfaceParm_l(width, height, format, flags, outmode);
	}
	pthread_mutex_unlock(&m_mutex);

	return ret;
}

int TCCDxBSurface::SetSurfaceParm_l(int width, int height, int format, int flags, int outmode)
{
	ALOGV("%s - width=%d, height=%d, format=%d, flags=%d, outmode=%d", __func__, width, height, format, flags, outmode);

	mWidth = width;
	mHeight = height;
	mFormat = format;
	mFlags = flags; //0:interlaced , 1:progressive
	mOutMode = outmode; //0:YUV420 Interleave , 1:YUV420 sp
	SetNativeWindow();

	return 0;
}

int TCCDxBSurface::SetNativeWindow()
{
	ALOGV("%s", __func__);

	status_t err;

	if (mNativeWindow == NULL || ((mWidth == 0) && (mHeight == 0)))
	{
		return 1;
	}

	err = native_window_set_scaling_mode(mNativeWindow, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
	if (err != OK)
	{
		ALOGE("NATIVE_WINDOW_SET_SCALING_MODE failed: (%d)", err);
		return 1;
	}

	err = native_window_set_buffers_geometry(
			mNativeWindow,
			mWidth,
			mHeight,
			(mOutMode == 0) ? HAL_PIXEL_FORMAT_YCbCr_420_SP : HAL_PIXEL_FORMAT_YV12);
	if (err != OK)
	{
		ALOGE("NATIVE_WINDOW_SET_BUFFERS_GEOMETRY failed: (%d)", err);
		return 1;
	}

#if 0
	uint32_t transform = HAL_TRANSFORM_ROT_0;
	err = native_window_set_buffers_transform(mNativeWindow, transform);
	if (err != OK)
	{
		ALOGE("NATIVE_WINDOW_SET_BUFFERS_TRANSFORM failed: (%d)", err);
		return 1;
	}
#endif

	unsigned int usage = GRALLOC_USAGE_HW_RENDER;
	if (mbUseOnlyMali)
	{
		usage |= GRALLOC_USAGE_PRIVATE_3;
	}

	if (1) //usage & GRALLOC_USAGE_PROTECTED)
	{
		// Verify that the ANativeWindow sends images directly to
		// SurfaceFlinger.
		int queuesToNativeWindow = 0;
		err = mNativeWindow->query(mNativeWindow, NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER, &queuesToNativeWindow);
		if (err != 0)
		{
			ALOGE("error authenticating native window: %d", err);
			return 1;
		}
		if (queuesToNativeWindow != 1)
		{
			ALOGE("native window could not be authenticated");
			return PERMISSION_DENIED;
		}
	}

	err = native_window_set_usage(mNativeWindow, usage | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_EXTERNAL_DISP);
	if (err != 0)
	{
		ALOGE("native_window_set_usage failed: %s (%d)", strerror(-err), -err);
		return 1;
	}

	int minUndequeuedBufs = 0;
	err = mNativeWindow->query(mNativeWindow, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeuedBufs);
	if (err != 0)
	{
		ALOGE("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS query failed: %s (%d)", strerror(-err), -err);
		return err;
	}
	ALOGW("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS query success: (%d)", minUndequeuedBufs);

#if 0
	err = native_window_set_buffer_count(mNativeWindow, minUndequeuedBufs);
	if (err != 0)
	{
		ALOGE("native_window_set_buffer_count failed: %s (%d)", strerror(-err), -err);
		return 1;
	}
#endif

	return 0;
}

void* TCCDxBSurface::GetPrivateAddr(int fd_val, int width, int height)
{
	//ALOGV("%s", __func__);

	int stride, stride_c, frame_len = 0;

	if( mTmem_fd < 0 )
	{
		ALOGE("/dev/tmem device is not opened.");
		return NULL;
	}

	stride = (width + 15) & ~15;
	stride_c = (stride/2 + 15) & ~15;
	frame_len = height * (stride + stride_c);// + 100;

	//ALOGD("%s ? %p + (0x%x - 0x%x) + 0x%x", __func__, (void*)mTMapInfo, fd_val, mUmpReservedPmap.base, frame_len);
	return (void*)((unsigned int)mTMapInfo + (fd_val - mUmpReservedPmap.base) + frame_len);
}

int TCCDxBSurface::WriteFrameBuf(PDISP_INFO pDispInfo)
{
	//ALOGV("%s", __func__);

	int ret;

	pthread_mutex_lock(&m_mutex);
	{
		memcpy(&mDispInfo, pDispInfo, sizeof(DISP_INFO));
		ret = WriteFrameBuf();
	}
	pthread_mutex_unlock(&m_mutex);

	return ret;
}

int TCCDxBSurface::WriteFrameBuf()
{
	//ALOGV("%s", __func__);

	status_t err;
	TCC_PLATFORM_PRIVATE_PMEM_INFO *frame_pmem_info;
	ANativeWindowBuffer* native_buffer;
	unsigned char* buf;
	int iTopFieldFirst;
	int fenceFd = -1;
	int ret;
	unsigned int usage = GRALLOC_USAGE_HW_RENDER;

	if(mValidSurface == 0)
		return -1;

	if(mNativeWindow == NULL)
		return -2;

	if (mDispInfo.frame_width != mWidth || mDispInfo.frame_height != mHeight)
	{
		SetSurfaceParm_l(mDispInfo.frame_width, mDispInfo.frame_height, mFormat, mFlags, mOutMode);
	}

	// dequeue a buffer
	err = mNativeWindow->dequeueBuffer(mNativeWindow, &native_buffer, &fenceFd);
	if (err != 0) return -1;
	sp<Fence> fence(new Fence(fenceFd));
	ret = fence->wait(Fence::TIMEOUT_NEVER);
	if (ret != OK)
	{
		ALOGE("dequeueBuffer : Fence::wait returned an error: %d", ret);
		return -2;
	}

	if (mbUseOnlyMali)
	{
		usage |= GRALLOC_USAGE_PRIVATE_3;
	}

	sp<GraphicBuffer> graphicBuffer(new GraphicBuffer(native_buffer, false));
	m_grallocModule->lock(m_grallocModule, native_buffer->handle, usage, 0, 0, mDispInfo.frame_width, mDispInfo.frame_height, (void**)&buf);

	frame_pmem_info = (TCC_PLATFORM_PRIVATE_PMEM_INFO *)GetPrivateAddr((int)buf, mDispInfo.frame_width, mDispInfo.frame_height);

	if(frame_pmem_info)
	{
		frame_pmem_info->copied = 1;

		/* mDispInfo.interlace is interlaced information 0:interlaced frame 1:Non-interlaced*/
		frame_pmem_info->optional_info[7] = (mDispInfo.interlace==0) ? 0x40000000 : 0x0;//OMX_BUFFERFLAG_INTERLACED_FRAME;
		frame_pmem_info->optional_info[4] = mDispInfo.unique_id; //Unique ID
		frame_pmem_info->optional_info[5] = mDispInfo.pts; //PTS
		frame_pmem_info->optional_info[6] = mDispInfo.stc; //STC
		frame_pmem_info->optional_info[8] = mDispInfo.frame_rate; //frame_rate
		if(mDispInfo.field_info == 0) // 0: top, 1: bottom
			frame_pmem_info->optional_info[7] |= 0x10000000;
		if(mDispInfo.pts == 0)
			frame_pmem_info->optional_info[7] |= 0x00000002; // Ignore pts

		frame_pmem_info->unique_addr = mDispInfo.unique_addr;

		memcpy(frame_pmem_info->offset, mDispInfo.frame_addr, sizeof(int)*3);

		frame_pmem_info->width = mDispInfo.frame_width;
		frame_pmem_info->height = mDispInfo.frame_height;
		if(!mFormat)
			frame_pmem_info->format = OMX_COLOR_FormatYUV420SemiPlanar;
		else
			frame_pmem_info->format = OMX_COLOR_FormatYUV420Planar;

		sprintf((char*)frame_pmem_info->name, "video");
		frame_pmem_info->name[5] = 0;
		frame_pmem_info->optional_info[0] = mDispInfo.frame_width;
		frame_pmem_info->optional_info[1] = mDispInfo.frame_height;
		frame_pmem_info->optional_info[2] = mDispInfo.frame_width;
		frame_pmem_info->optional_info[3] = mDispInfo.frame_height;
		frame_pmem_info->optional_info[9] = 0;
		frame_pmem_info->optional_info[10] = 0;
		frame_pmem_info->optional_info[11] = 0;
		frame_pmem_info->optional_info[12] = 0;
		frame_pmem_info->optional_info[13] = 1; // vsync enable
		frame_pmem_info->optional_info[14] = mDispInfo.display_index;
		frame_pmem_info->optional_info[15] = mDispInfo.vpu_index;
	}
	else
	{
		ALOGE("frame_pmem_info is null");
		mDispInfo.vsync_enable = 0;
	}

	if(mDispInfo.vsync_enable)
	{
		frame_pmem_info->copied = 0;
		if (frame_pmem_info->optional_info[4] <= 1)
		{
			status_t err = native_window_set_synchronous_mode(mNativeWindow, NATIVE_WINDOW_SYNCHRONOUS_MODE);
			if (err != OK)
			{
				ALOGE("Failed to set sync mode: %d", err);
			}
		}
	}
	else
	{
		TCCDxB_ScalerCopyData(	mDispInfo.frame_width,
								mDispInfo.frame_height,
								(unsigned char*)mDispInfo.frame_addr[0],  // YSrc
								(unsigned char*)mDispInfo.frame_addr[1],  // USrc
								(unsigned char*)mDispInfo.frame_addr[2],  // VSrc
								mDispInfo.format,                 // bSrcYUVInter
								buf,                        // addrDst
								(unsigned char)1,           // ignoreAligne
								mDispInfo.field_info,                  // fieldInfomation
								1);                         // interlaceMode(0: deinterlacing, 1: bypass )
	}
	m_grallocModule->unlock(m_grallocModule, native_buffer->handle);
	native_window_set_buffers_timestamp(mNativeWindow, mDispInfo.pts);
	mNativeWindow->queueBuffer(mNativeWindow, graphicBuffer->getNativeBuffer(), -1);

	return 0;
}

int TCCDxBSurface::CaptureVideoFrame(char *strFilePath)
{
	ALOGV("%s", __func__);

	int ret;

	pthread_mutex_lock(&m_mutex);
	{
		ret = TCCDxB_CaptureImage(mDispInfo.frame_width, mDispInfo.frame_height, mDispInfo.frame_addr, 
									mDispInfo.format, mDispInfo.interlace, mDispInfo.field_info, (unsigned char*)strFilePath);
	}
	pthread_mutex_unlock(&m_mutex);

	return 0;
}