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
*
* Company: Telechips.co.Ltd
* Author: ZzaU
*/

#define LOG_TAG "TCC_V4l2_Camera"
#include <utils/Log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/types.h>
#include <sys/mman.h>
#include <linux/fb.h>
//#include <sys/kd.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/poll.h>

#include "TCC_V4l2_Camera.h"
#include <cutils/properties.h>

#include <linux/fb.h>


int gcam_fd;

static int DEBUG 				= 0;
#define DBUG(msg...) 			if(DEBUG) { ALOGD( " " msg); }
#define VDBUG(msg...) 		if(DEBUG) { ALOGD( " " msg); }
#define FUNCTION_IN 		DBUG("%d: %s() In", __LINE__, __FUNCTION__);
#define FUNCTION_OUT 		DBUG("%d: %s() Out", __LINE__, __FUNCTION__);

const static unsigned char huffman_table[] = {
  0xff, 0xc4, 0x01, 0xa2, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
  0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x01, 0x00, 0x03,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
  0x0a, 0x0b, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05,
  0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d, 0x01, 0x02, 0x03, 0x00, 0x04,
  0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22,
  0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15,
  0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36,
  0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
  0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
  0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
  0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95,
  0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
  0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2,
  0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5,
  0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
  0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
  0xfa, 0x11, 0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05,
  0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00, 0x01, 0x02, 0x03, 0x11, 0x04,
  0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22,
  0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33,
  0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25,
  0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35, 0x36,
  0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
  0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
  0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
  0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94,
  0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
  0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
  0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
  0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
  0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa
};


namespace android {
#ifdef USE_CIFOUT_PMEM
class TCC_Camera_Mem {
	public:
		sp<MemoryHeapBase>         mMasterHeap;
		sp<MemoryHeapPmem>       mPmemHeap;

	TCC_Camera_Mem() {
		mMasterHeap = NULL;
		mPmemHeap = NULL;
		DBUG(" Create PMEM Device!!");
	}

	~TCC_Camera_Mem() {
		if(mPmemHeap != NULL)
		{
			mPmemHeap->unslap();
			mPmemHeap.clear();
		}
		if(mMasterHeap != NULL)
		{
			mMasterHeap.clear();
		}
		DBUG(" Close PMEM Device!!");
	}

	sp<MemoryHeapPmem>  allocSharedMem(int sizeInBytes) 
	{
		int ret;
		
		mMasterHeap = new MemoryHeapBase("/dev/pmem_adsp", sizeInBytes);

		if (mMasterHeap->heapID() < 0) 
		{
			ALOGE("Err :: pmem: could not create master heap!");
			return NULL;
		}

		if((ret = mMasterHeap->setDevice("/dev/pmem")) != NO_ERROR)
		{
			if(ret == ALREADY_EXISTS)
			{
				DBUG("MasterHeap Device already is setted.");
			}
		}

		DBUG("MasterHeap : Device(%s), Fd(%d), Base(0x%x), Size(%d)", mMasterHeap->getDevice(), mMasterHeap->getHeapID(), mMasterHeap->getBase(), mMasterHeap->getSize());
		
		mPmemHeap = new MemoryHeapPmem(mMasterHeap, 0);
		mPmemHeap->slap();

		DBUG(" Open PMEM Device!!");

		return mPmemHeap;
	}
	
};

TCC_Camera_Mem *mCameraMem;
#endif


/*********************************************************************************************/
/*                                                     Private Function												 */
/*********************************************************************************************/

/*===========================================================================
FUNCTION
===========================================================================*/
int TCC_V4l2_Camera::camif_open_device(int nCamIndex)
{	
	int ret;
    
	DBUG("********************** camif_open_device in");

	camif_init_data();

	if((mCamDevice.fd = open(CAM_0_DEVICE, O_RDWR)) <= 0) {
		//ALOGE("Err[%s] :: %s Open Failed!! ", strerror(errno), CAM_0_DEVICE);
		if((mCamDevice.fd = open(CAM_1_DEVICE, O_RDWR)) <= 0) {
			ALOGE("Err[%s] :: %s Open Failed!! ", strerror(errno), CAM_1_DEVICE);
			return -1;
		}
        else
        {
            DBUG(" %s is opened! ", CAM_1_DEVICE);
        }
	}
    else
    {
        DBUG(" %s is opened! ", CAM_0_DEVICE);
    }
    
	gcam_fd = mCamDevice.fd;

	// Camera Sensor Open for Multiple Camera!!
	if((ret = ioctl (mCamDevice.fd, VIDIOC_USER_SET_CAMINFO_TOBEOPEN, &nCamIndex)) < 0)
		ALOGE("Err[%s] :: VIDIOC_USER_SET_CAMINFO_TOBEOPEN failed", strerror(errno));
	else 
		DBUG("VIDIOC_USER_SET_CAMINFO_TOBEOPEN Success!!");

	// get sensor capture size
	if((ret = ioctl (mCamDevice.fd, VIDIOC_USER_GET_MAX_RESOLUTION, &nCamIndex)) < 0) {
		mUvc_camera = true;
		ALOGE("Err[%s] :: VIDIOC_USER_GET_MAX_RESOLUTION failed => This cam is UVC.", strerror(errno));
	} else {
		mUvc_camera = false;
		sensor_max_resolution = ret;
	}

	// get zoom support
	if((mZoomSupport = ioctl(mCamDevice.fd, VIDIOC_USER_GET_ZOOM_SUPPORT, &nCamIndex)) < 0) {
		ALOGE("Err[ %s ] ::  VIDIOC_USER_GET_ZOOM_SUPPORT failed.", strerror(errno));
	}

	if(mUvc_camera) {
		struct v4l2_frmsizeenum frmsize;
		struct v4l2_frmivalenum fival;

		mCamDevice.vid_fmt.type 				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		mCamDevice.vid_fmt.fmt.pix.pixelformat 	= V4L2_PIX_FMT_MJPEG;
		//mCamDevice.vid_fmt.fmt.pix.pixelformat 	= V4L2_PIX_FMT_YUYV;

		if((ret = ioctl(mCamDevice.fd, VIDIOC_TRY_FMT, &(mCamDevice.vid_fmt))) < 0) {
			ALOGE("Err[%s] ::  This module doesn't support MJPEG format!", strerror(errno));
			#if defined(TCC_8925S_INCLUDE) || defined(TCC_8935S_INCLUDE)
				return -1;
			#else
				mCamDevice.vid_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
				if((ret = ioctl(mCamDevice.fd, VIDIOC_TRY_FMT, &(mCamDevice.vid_fmt))) < 0) {
					ALOGE("Err[%s] :: This module doesn't support YUV422 format!", strerror(errno));
					mCamDevice.vid_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
				}
			#endif
		}
		
		mPreview_fmt = mCapture_fmt = mUVC_fmt = mCamDevice.vid_fmt.fmt.pix.pixelformat;

		frmsize.pixel_format = mCamDevice.vid_fmt.fmt.pix.pixelformat;
		frmsize.index = 0;

		mCamDevice.mUvc_MaxWidth  = DEF_UVC_WIDTH;
		mCamDevice.mUvc_MaxHeight = DEF_UVC_HEIGHT;

		mDef_MaxFps = mUvc_HDFps = 0;
		while(1) {
			frmsize.pixel_format = mPreview_fmt;
			
			if(ioctl(mCamDevice.fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) < 0) {
				ALOGE( "Err[%s] :: VIDIOC_ENUM_FRAMESIZES Failed!! %d : format 0x%x", strerror(errno), frmsize.index, frmsize.pixel_format);
				break;
			}

			if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
				DBUG("%d : %d x %d", frmsize.index, frmsize.discrete.width, frmsize.discrete.height);
				if(mCamDevice.mUvc_MaxWidth < frmsize.discrete.width || mCamDevice.mUvc_MaxHeight < frmsize.discrete.height) {
					mCamDevice.mUvc_MaxWidth  = frmsize.discrete.width;
					mCamDevice.mUvc_MaxHeight = frmsize.discrete.height;
				}

				fival.index = 0;
				while(1) {
					fival.pixel_format = mPreview_fmt;
					fival.width = frmsize.discrete.width;
					fival.height = frmsize.discrete.height;
					
					if(ioctl(mCamDevice.fd, VIDIOC_ENUM_FRAMEINTERVALS, &fival) < 0) {
						ALOGE( "Err[%s] :: VIDIOC_ENUM_FRAMEINTERVALS Failed!! ", strerror(errno));
						break;
					}

					if(fival.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
						if(fival.discrete.denominator <= (unsigned int)DEF_UVC_MAX_FPS && fival.discrete.denominator > (unsigned int)mDef_MaxFps)
							mDef_MaxFps = fival.discrete.denominator;
						if(fival.discrete.denominator <= (unsigned int)DEF_UVC_HD_FPS && fival.discrete.denominator > (unsigned int)mUvc_HDFps)
							mUvc_HDFps = fival.discrete.denominator;
					}
					fival.index += 1;
				}			
			}
			else {
				break;
			}

			frmsize.index += 1;				
		}
		ALOGI("UVC Max Resolution : %d x %d, fps: %d/%d", mCamDevice.mUvc_MaxWidth, mCamDevice.mUvc_MaxHeight, mDef_MaxFps, mUvc_HDFps);
	}
	else {
		#if defined(CONFIG_VIDEO_HDMI_IN_SENSOR) && defined(FEATURE_WDMA_RGB_MODE)
			#if defined(FEATURE_RGB_MODE_565)	
				mPreview_fmt = 	mCapture_fmt = 	V4L2_PIX_FMT_RGB565;  //RGB565
			#else								
				mPreview_fmt = 	mCapture_fmt = 	V4L2_PIX_FMT_RGB32; //aRGB8888
			#endif
		
		#else
			mPreview_fmt = V4L2_PIX_FMT_YVU420;
			mCapture_fmt = V4L2_PIX_FMT_YUYV;

		#endif

		if((ret = ioctl (mCamDevice.fd, VIDIOC_USER_GET_SENSOR_FRAMERATE, &mDef_MaxFps)) < 0) {
			ALOGE("Err[%s] :: VIDIOC_USER_GET_SENSOR_FRAMERATE failed", strerror(errno));
			mDef_MaxFps = 20;
		}
		ALOGI("Camera FrameRate : %d", mDef_MaxFps);
	}

	if((mCamDevice.jpegenc_fd = open(JPEGENC_DEVICE,O_RDWR)) <= 0) {
		ALOGE( "Err[%s] :: %s Open Failed!! ", strerror(errno), JPEGENC_DEVICE);
		return -1;
	}

	#ifdef VPU_HW_DECODE
	mVpuState = VPU_CLOSED;
	#else // VPU_HW_DECODE
	if((mCamDevice.jpegdec_fd = open(JPEGDEC_DEVICE,O_RDWR)) <= 0) {
		ALOGE( "Err[%s] :: %s Open Failed!! ", strerror(errno), JPEGDEC_DEVICE);
		return -1;
	}
	#endif // VPU_HW_DECODE

	if(mUvc_camera) {
		#ifdef USE_G2D_FOR_UVC
		if((mCamDevice.g2d_fd = open(GRAPHIC_DEVICE,O_RDWR)) <= 0) {
			ALOGE( "Err[%s] :: %s Open Failed!! ", strerror(errno), GRAPHIC_DEVICE);
			return -1;
		}
		#else // USE_G2D_FOR_UVC
		if((mCamDevice.convert_fd = open(SCALER_DEVICE,O_RDWR)) <= 0) {
			ALOGE( "Err[%s] :: %s Open Failed!! ", strerror(errno), SCALER_DEVICE);
			return -1;
		}
		#endif // USE_G2D_FOR_UVC
	}
	DBUG("********************** cam_device [%d]", mCamDevice.fd);

	camif_get_dev_info();

	DBUG("********************** camif_open_device out");

	return 0;
}

/*===========================================================================
FUNCTION
===========================================================================*/
int TCC_V4l2_Camera::camif_close_device(void)
{
	int ret;

	DBUG("********************** camif_close_device in");

	//camif_stop_stream(); //20101215 ysseung   modify to camera stop sequence.

	if(close(mCamDevice.fd) < 0){
		ALOGE("Err[%s] :: mCamDevice.fd :: close function is failed!", strerror(errno));
	}
	DBUG("camera release ok.");
	gcam_fd =0;
	if(close(mCamDevice.jpegenc_fd) < 0){
		ALOGE("Err[%s] :: mCamDevice.jpegenc_fd :: close function is failed!", strerror(errno));
	}
	DBUG("jpeg enc release ok.");
	#ifndef VPU_HW_DECODE
	if(close(mCamDevice.jpegdec_fd) < 0){
		ALOGE("Err[%s] :: mCamDevice.jpegdec_fd :: close function is failed!", strerror(errno));
	}
	DBUG("jpeg dec release ok.");
	#endif
	if(close(mCamDevice.convert_fd) < 0){
		ALOGE("Err[%s] :: mCamDevice.convert_fd :: close function is failed!", strerror(errno));
	}
	if(mCamDevice.g2d_fd>=0) {
		if(close(mCamDevice.g2d_fd) < 0){
		ALOGE("Err[%s] :: mCamDevice.g2d_fd :: close function is failed!", strerror(errno));
    	}
	}
	
	mCamDevice.fd = mCamDevice.jpegenc_fd = mCamDevice.jpegdec_fd = mCamDevice.convert_fd = mCamDevice.g2d_fd = -1;
	mCamDevice.cam_mode = MODE_IDLE;
	DBUG("********************** camif_close_device out");

	return 0;
}


int TCC_V4l2_Camera::camif_init_format (int width, int height)
{
    int ret;
	
    memset(&(mCamDevice.vid_fmt), 0,  sizeof(struct v4l2_format));
    mCamDevice.vid_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if((ret = ioctl(mCamDevice.fd, VIDIOC_G_FMT, &(mCamDevice.vid_fmt))) < 0) {
		ALOGE("Err[%s] :: VIDIOC_G_FMT failed!", strerror(errno));
	} else {
		int real_width, real_height;
		
		real_width		= width;
		real_height		= height;

		if(mUvc_camera) {
			int fix_width, fix_height;
			struct v4l2_frmsizeenum frmsize;
			struct v4l2_frmivalenum fival;
			
			mPreview_fmt = mCapture_fmt = mUVC_fmt;

			frmsize.pixel_format = mCamDevice.vid_fmt.fmt.pix.pixelformat;
			frmsize.index = 0;

			fix_width = mCamDevice.mUvc_MaxWidth; 
			fix_height = mCamDevice.mUvc_MaxHeight;
			while(1) {
				frmsize.pixel_format = mUVC_fmt;
				if(ioctl(mCamDevice.fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) < 0) {
					ALOGE( "Err[%s] :: VIDIOC_ENUM_FRAMESIZES Failed!! ", strerror(errno));
					break;
				}

				if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
					if(((unsigned int)width == frmsize.discrete.width) && ((unsigned int)height == frmsize.discrete.height)) {
						fix_width = frmsize.discrete.width;
						fix_height = frmsize.discrete.height;
						break;
					}
					else if((unsigned int)fix_width >= frmsize.discrete.width && (unsigned int)fix_height >= frmsize.discrete.height \
						&& (unsigned int)width <= frmsize.discrete.width && (unsigned int)height <= frmsize.discrete.height) {
						fix_width = frmsize.discrete.width;
						fix_height = frmsize.discrete.height;
					}
				} else {
					break;
				}

				frmsize.index += 1;				
			}

			real_width		= fix_width;
			real_height 	= fix_height;
			ALOGI("UVC Detected Resolution : %d x %d", fix_width, fix_height);
		}
	
		DBUG("VIDIOC_G_FMT: fmt(0x%x), size(%d x %d).", mCamDevice.vid_fmt.fmt.pix.pixelformat, 	\
								mCamDevice.vid_fmt.fmt.pix.width, mCamDevice.vid_fmt.fmt.pix.height);

		mCamDevice.vid_fmt.type 				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		mCamDevice.vid_fmt.fmt.pix.width 		= real_width;
		mCamDevice.vid_fmt.fmt.pix.height 		= real_height;
		mCamDevice.vid_fmt.fmt.pix.field       	= V4L2_FIELD_INTERLACED;
		DBUG("VIDIOC_S_FMT: fmt(0x%x), size(%d x %d).", mCamDevice.vid_fmt.fmt.pix.pixelformat, real_width, real_height);

		DBUG("format(%d), camera_mode(%d).", mCamDevice.preview_fmt, mCamDevice.cam_mode);
		if(mCamDevice.preview_fmt == MODE_YUV420
			#if !defined(SENSOR_5M) // jpeg capture !!
			&& mCamDevice.cam_mode != MODE_CAPTURE
			#endif
		) {
			mCamDevice.vid_fmt.fmt.pix.pixelformat 	= mPreview_fmt;
		} else {
			mCamDevice.vid_fmt.fmt.pix.pixelformat 	= mCapture_fmt;
		}

		if(mCamDevice.vid_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV || mCamDevice.vid_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB565) {
			mCamDevice.vid_fmt.fmt.pix.sizeimage 	= real_width * real_height * 2;
			
		} else if(mCamDevice.vid_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB24){
			mCamDevice.vid_fmt.fmt.pix.sizeimage 	= real_width * real_height * 3;	
			
		} else if(mCamDevice.vid_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB32){
			mCamDevice.vid_fmt.fmt.pix.sizeimage 	= real_width * real_height * 4;	
			
		} else {
			mCamDevice.vid_fmt.fmt.pix.sizeimage 	= real_width * real_height * 1.5;
		}
        ALOGI("Set Preview Fmt: fmt(0x%x), size(%d x %d).", mCamDevice.vid_fmt.fmt.pix.pixelformat, real_width, real_height);

	    if((ret = ioctl(mCamDevice.fd, VIDIOC_S_FMT, &(mCamDevice.vid_fmt))) < 0) {
			ALOGE("Err[%s] :: VIDIOC_S_FMT failed!", strerror(errno));
	    }
	}

    return ret;
}

/*===========================================================================
FUNCTION
===========================================================================*/
int TCC_V4l2_Camera::camif_init_buffer(void)
{
	int ret = 0;
	int cnt = 0;
	int i;
	#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	int iRecBuflen = 0;
	#endif
	void* mmap_addr;
	//struct v4l2_buffer buf;
	unsigned int phy_addr;

	FUNCTION_IN

	memset(&(mCamDevice.vid_buf), 0, sizeof(struct v4l2_requestbuffers));

	// internal buffer allocation!!
#if defined(USE_CIFOUT_GRALLOC)
    if(!mUvc_camera)
    {
    	mCamDevice.vid_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	mCamDevice.vid_buf.memory = V4L2_MEMORY_MMAP;

    	for(i=0; i < mPreviewBufferCount; i++) {
    		mCamDevice.vid_buf.count  		= i;
    		mCamDevice.vid_buf.reserved[0] 	= (unsigned int)mCameraBufferAddr[i];
    		//DBUG("%s: gralloc address=0x%08x.", __FUNCTION__, mCamDevice.vid_buf.reserved[0]);
    		if((ret = ioctl(mCamDevice.fd, VIDIOC_REQBUFS, &(mCamDevice.vid_buf))) < 0) {
    			ALOGE("Err[%s] :: VIDIOC_REQBUFS failed!", strerror(errno));
    			return ret;
    		} else {
    			mCamDevice.Buffer_cnt = mCamDevice.vid_buf.count;
    			DBUG("***** Buffer Count : %d ..", mCamDevice.Buffer_cnt);
    		}
    	}
    }
    else
    {
        struct v4l2_buffer buf;
        
		mCamDevice.vid_buf.count  = NUM_VIDBUF;
    	mCamDevice.vid_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	mCamDevice.vid_buf.memory = V4L2_MEMORY_MMAP;

    	if((ret = ioctl(mCamDevice.fd, VIDIOC_REQBUFS, &(mCamDevice.vid_buf))) < 0)
    	{
    		ALOGE("Err[%s] :: VIDIOC_REQBUFS failed!", strerror(errno));
    		return ret;
    	}
    	else
    	{    
    		if (mCamDevice.vid_buf.count < 1) 
    		{
    			ALOGE("Err :: Insufficient buffer memory on camera");
    			return -1;
    		}
    		mCamDevice.Buffer_cnt = mCamDevice.vid_buf.count;
    		DBUG("***** Buffer Count : %d ..", mCamDevice.Buffer_cnt);
    	}

    	memset(&buf, 0,  sizeof(struct v4l2_buffer));
    	buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	buf.memory      = V4L2_MEMORY_MMAP;
    	buf.index       = 0;

		mCamDevice.needYuv420Cvt	= -1;
		mCamDevice.buffers_lencvt	= (mCamDevice.preview_width* mCamDevice.preview_height*3/2);
		mCamDevice.buffers_lencvt   = ALIGNED_BUFF(mCamDevice.buffers_lencvt, 4096);

        {
    		unsigned int ui_frame_size, base_offset, offset, i_cnt, base_index;

    		// get internal buffer and Virtual address!!
    		for (buf.index=0; buf.index<mCamDevice.Buffer_cnt ; buf.index++) 
    		{
    			if ((ret = ioctl (mCamDevice.fd, VIDIOC_QUERYBUF, &buf)) < 0) 
    			{
    				ALOGE("Err[%s] :: VIDIOC_QUERYBUF failed!", strerror(errno));
    				goto Error;
    			}

            	buffer_manager[buf.index] = buf.index;

                pmem_buffers_offset[BUFFER_PREVIEW][buf.index] = buf.index*buf.length;
    			pmem_buffers[BUFFER_PREVIEW][buf.index] = mExtCameraPmap.base + pmem_buffers_offset[BUFFER_PREVIEW][buf.index];    			
    			DBUG("pmem init for preview :: %d - offset[0x%x] - Phy_addr[0x%x]- length[0x%x]",buf.index, pmem_buffers_offset[BUFFER_PREVIEW][buf.index], mExtCameraPmap.base+buf.m.offset, buf.length);
    		}
            
            buffer_manager_in_index = buffer_manager_out_index = 0;    		
    		mCamDevice.release_idx = 0;

    		pmem_mmaped = (unsigned int*)mmap(0, mExtCameraPmap.size, PROT_READ | PROT_WRITE, MAP_SHARED, mCamDevice.fd, mExtCameraPmap.base);
    		if (MAP_FAILED == pmem_mmaped) 
    		{
    			ALOGE("Err[%s] :: MMAP failed!", strerror(errno));
    			ret = -1;
    			goto Error;
    		}
    		DBUG("pmem mmap :: 0x%x - Phy_addr[0x%x]- length[0x%x]", (unsigned int)pmem_mmaped, mExtCameraPmap.base, PMEM_CAM_SIZE);		
    	}        
    }

#else // USE_CIFOUT_GRALLOC
	if(mCamDevice.cam_mode == MODE_CAPTURE && !mUvc_camera)
		mCamDevice.vid_buf.count  = 1;    
	else
		mCamDevice.vid_buf.count  = NUM_VIDBUF;  

	mCamDevice.vid_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	mCamDevice.vid_buf.memory = V4L2_MEMORY_MMAP;

	if((ret = ioctl(mCamDevice.fd, VIDIOC_REQBUFS, &(mCamDevice.vid_buf))) < 0)
	{
		ALOGE("Err[%s] :: VIDIOC_REQBUFS failed!", strerror(errno));
		return ret;
	}
	else
	{    
		if (mCamDevice.vid_buf.count < 1) 
		{
			ALOGE("Err :: Insufficient buffer memory on camera");
			return -1;
		}
		mCamDevice.Buffer_cnt = mCamDevice.vid_buf.count;
		DBUG("***** Buffer Count : %d ..", mCamDevice.Buffer_cnt);
	}

	memset(&buf, 0,  sizeof(struct v4l2_buffer));
	buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory      = V4L2_MEMORY_MMAP;
	buf.index       = 0;

	if(mUvc_camera)
	{
		mCamDevice.needYuv420Cvt	= -1;
		mCamDevice.buffers_lencvt	= (mCamDevice.preview_width* mCamDevice.preview_height*3/2);
		mCamDevice.buffers_lencvt   = ALIGNED_BUFF(mCamDevice.buffers_lencvt, 4096);
	}
#ifdef USE_CIFOUT_PMEM
	destopy_pmem();
	if((ret = create_pmem()) < 0)
	{
		goto Error;
	}

	{
		unsigned int ui_frame_size, base_offset, offset, i_cnt, base_index;

		// get internal buffer and Virtual address!!
		for (buf.index=0; buf.index<mCamDevice.Buffer_cnt ; buf.index++) 
		{
			if ((ret = ioctl (mCamDevice.fd, VIDIOC_QUERYBUF, &buf)) < 0) 
			{
				ALOGE("Err[%s] :: VIDIOC_QUERYBUF failed!", strerror(errno));
				goto Error;
			}

			pmem_buffers[BUFFER_PREVIEW][buf.index] = new MemoryBase(mPreviewHeap,buf.index*buf.length, buf.length);
			pmem_buffers_offset[BUFFER_PREVIEW][buf.index] = buf.index*buf.length;
			DBUG("pmem init for preview :: %d - offset[0x%x] - Phy_addr[0x%x]- length[0x%x]",buf.index, pmem_buffers_offset[BUFFER_PREVIEW][buf.index], PA_PREVIEW_BASE_ADDR+buf.m.offset, buf.length);
		}

		if(mCamDevice.cam_mode != MODE_CAPTURE)
		{
			base_offset = mCamDevice.Buffer_cnt*buf.length;

			ui_frame_size = 4096;
			DBUG("camcorder_addr_buffer ::  frame_size = %d  = 0x%x!", ui_frame_size, ui_frame_size);
			
			for (i_cnt=0; i_cnt<mCamDevice.Buffer_cnt; i_cnt++) 
			{
				offset = base_offset +(i_cnt*ui_frame_size);
				
				pmem_buffers[BUFFER_RECORD][i_cnt] = new MemoryBase(mPreviewHeap, offset, ui_frame_size);
				pmem_buffers_offset[BUFFER_RECORD][i_cnt] = offset;
				
				DBUG("pmem init for record :: %d - offset[0x%x] - Phy_addr[0x%x]- length[0x%x]", i_cnt, pmem_buffers_offset[BUFFER_RECORD][i_cnt], PA_PREVIEW_BASE_ADDR+pmem_buffers_offset[BUFFER_RECORD][i_cnt], ui_frame_size);
			}

#ifdef USE_CHANGE_SEMIPLANAR 
			//DBUG("width = %d, height=%d",mCamDevice.preview_width, mCamDevice.preview_height);
			if(mCamDevice.preview_width <= 800 || mCamDevice.preview_height <= 480)
			{
				base_offset = (mCamDevice.Buffer_cnt*buf.length) + (mCamDevice.Buffer_cnt*ui_frame_size);
				DBUG("pmem init for callback :: base_offset = [0x%x]",base_offset);
				for (i_cnt=0; i_cnt<1/*mCamDevice.Buffer_cnt*/; i_cnt++) 
				{
					offset = base_offset +(i_cnt*buf.length);
					pmem_buffers[BUFFER_CALLBACK][i_cnt] = new MemoryBase(mPreviewHeap,offset, buf.length);
					pmem_buffers_offset[BUFFER_CALLBACK][i_cnt] = offset;
					DBUG("pmem init for callback :: %d - offset[0x%x] - Phy_addr[0x%x]- length[0x%x]",i_cnt, pmem_buffers_offset[BUFFER_CALLBACK][i_cnt], PA_PREVIEW_BASE_ADDR+pmem_buffers_offset[BUFFER_CALLBACK][i_cnt], buf.length);
				}
				base_offset = (mCamDevice.Buffer_cnt+1)*buf.length + (mCamDevice.Buffer_cnt*ui_frame_size);
			}
            else
                base_offset = (mCamDevice.Buffer_cnt*buf.length) + (mCamDevice.Buffer_cnt*ui_frame_size);
#else
			base_offset = (mCamDevice.Buffer_cnt*buf.length) + (mCamDevice.Buffer_cnt*ui_frame_size);
#endif //USE_CHANGE_SEMIPLANAR				
			base_index = mCamDevice.Buffer_cnt;
			if(mUvc_camera)
			{		
				int total_len;
				total_len = base_offset + (mCamDevice.Buffer_cnt*mCamDevice.buffers_lencvt);

				if(total_len > PMEM_CAM_SIZE)
				{
					ALOGE("Not enough memory!! Increase Camera Memory(0x%x needed)!!)", total_len);
					ret = -1;
					goto Error;
				}
				
				for (i_cnt=0; i_cnt<mCamDevice.Buffer_cnt; i_cnt++) 
				{
					offset = base_offset +(i_cnt*mCamDevice.buffers_lencvt);
					
					pmem_buffers[BUFFER_UVC][i_cnt] = new MemoryBase(mPreviewHeap, offset, mCamDevice.buffers_lencvt);
					pmem_buffers_offset[BUFFER_UVC][i_cnt] = offset;
					
					DBUG("pmem init for uvc :: %d - offset[0x%x] - Phy_addr[0x%x]- length[0x%x]", i_cnt, pmem_buffers_offset[BUFFER_UVC][i_cnt], PA_PREVIEW_BASE_ADDR+pmem_buffers_offset[BUFFER_UVC][i_cnt], mCamDevice.buffers_lencvt);
				}
			}
		}
		mCamDevice.release_idx = 0;

		pmem_mmaped = (unsigned int*)mmap(0, PMEM_CAM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mCamDevice.fd, PA_PREVIEW_BASE_ADDR);
		if (MAP_FAILED == pmem_mmaped) 
		{
			ALOGE("Err[%s] :: MMAP failed!", strerror(errno));
			ret = -1;
			goto Error;
		}
		DBUG("pmem mmap :: 0x%x - Phy_addr[0x%x]- length[0x%x]", (unsigned int)pmem_mmaped, PA_PREVIEW_BASE_ADDR, PMEM_CAM_SIZE);		
	}
#endif //USE_CIFOUT_PMEM
#endif // USE_CIFOUT_GRALLOC

	FUNCTION_OUT

	return ret;

Error:
#ifdef USE_CIFOUT_PMEM
	destopy_pmem();
#endif		

	return ret;
}

/*===========================================================================
FUNCTION
===========================================================================*/
void TCC_V4l2_Camera::camif_uninit_buffer(void)
{
#if !defined(USE_CIFOUT_GRALLOC)
#ifdef USE_CIFOUT_PMEM
	for (int buff_idx=0; buff_idx<BUFFER_MAX; buff_idx++)
	{
		for (int i=0; i<mCamDevice.Buffer_cnt; i++) 
		{
			if(pmem_buffers[buff_idx][i] != NULL)
			{		
				DBUG("pmem deinit [%d][%d] - offset[0x%x] - Phy_addr[0x%x]", buff_idx, i, pmem_buffers_offset[buff_idx][i], PA_PREVIEW_BASE_ADDR+pmem_buffers_offset[buff_idx][i]);
				pmem_buffers[buff_idx][i].clear();
				pmem_buffers[buff_idx][i] = NULL;
				pmem_buffers_offset[buff_idx][i] = 0;
			}
		}
	}
	destopy_pmem();
#else // USE_CIFOUT_PMEM
	int cnt, ret;

	for (cnt=0; cnt<mCamDevice.Buffer_cnt; cnt++)
	{
		if((ret = munmap((void*)(mCamDevice.buffers_virt[cnt]), mCamDevice.buffers_len)) < 0)
		{
			ALOGE("Err[%s] :: UNMAP failed. addr = 0x%x, size = 0x%x", strerror(errno), mCamDevice.buffers_virt[cnt], mCamDevice.buffers_len);
			break;
		}

		DBUG("munmap %d[0x%x]- 0x%x", cnt, mCamDevice.buffers_virt[cnt], mCamDevice.buffers_len);
	}

#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	for (cnt=0; cnt<mCamDevice.Buffer_cnt; cnt++)
	{
		if((ret = munmap((void*)(mCamDevice.rec_buffers_virt[cnt]), mCamDevice.rec_buffers_len)) < 0)
		{
			ALOGE("Err[%s] :: UNMAP failed. addr = 0x%x, size = 0x%x", strerror(errno), mCamDevice.rec_buffers_virt[cnt], mCamDevice.rec_buffers_len);
			break;
		}

		DBUG("munmap %d[0x%x]- 0x%x", cnt, mCamDevice.rec_buffers_virt[cnt], mCamDevice.rec_buffers_len);
	}
#endif // defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
#endif // USE_CIFOUT_PMEM
#endif

    if(mUvc_camera)
    {
    	if(munmap((void*)pmem_mmaped, mExtCameraPmap.size) < 0)
    	{
    		ALOGE("Err[%s] :: UNMAP failed. addr = 0x%x, size = 0x%x", strerror(errno), (unsigned int)pmem_mmaped, mExtCameraPmap.size);
    	}
    	DBUG("pmem munmap for vpu :: 0x%x - Phy_addr[0x%x]- length[0x%x]", (unsigned int)pmem_mmaped, mExtCameraPmap.base, mExtCameraPmap.size);			
    }
}


/*===========================================================================
FUNCTION
===========================================================================*/
void  TCC_V4l2_Camera::camif_init_data(void)
{
	char video_call_enable[PROPERTY_VALUE_MAX];
	char camera_no_display[PROPERTY_VALUE_MAX];

	memset(&mCamDevice, 0, sizeof(CameraDevice));
	mCamDevice.fd 						= -1;
	mCamDevice.jpegenc_fd				= -1;
	mCamDevice.jpegdec_fd				= -1;
	mCamDevice.convert_fd				= -1;
	mCamDevice.g2d_fd					= -1;
	mCamDevice.preview_width			= 0; //PREVIEW_WIDTH;
	mCamDevice.preview_height			= 0; //PREVIEW_HEIGHT;
	mCamDevice.cam_mode					= MODE_IDLE;
	mCamDevice.preview_fmt				= MODE_YUV420;
	mCamDevice.preview_idx				= 0;
	mCamDevice.cap_info.width 			= 2048;
	mCamDevice.cap_info.height 			= 1536;
	mCamDevice.cap_info.jpeg_quality 	= 2;

	property_get("tcc.all.video.call.enable", video_call_enable, "");
	if(mRealKCount = atoi(video_call_enable)) {
		mRealKCount = NUM_OF_V2IP_PREVIEW_BUFFERS;
		mIsRunningV2IP = 1;
	} else {
		mRealKCount = NUM_OF_PREVIEW_BUFFERS;
		mIsRunningV2IP = 0;
	}

	property_get("tcc.all.camera.no.display", camera_no_display, "");
	mNoDisplay = atoi(camera_no_display);
}

/*===========================================================================
FUNCTION
===========================================================================*/
int TCC_V4l2_Camera::camif_get_dev_info (void)
{
	int ret;

	if((ret = ioctl (mCamDevice.fd, VIDIOC_QUERYCAP, &(mCamDevice.vid_cap))) < 0) {
		ALOGE("Err[%s] :: VIDIOC_QUERYCAP failed", strerror(errno));
	} else {
		DBUG("Driver: %s", mCamDevice.vid_cap.driver);
		DBUG("Card: %s", mCamDevice.vid_cap.card);
		DBUG("Capabilities: 0x%x", (unsigned int)(mCamDevice.vid_cap.capabilities));

		if((mCamDevice.vid_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
			ALOGE("Err :: unSupport capture i/o");
			return -1;
		}

		if(!(mCamDevice.vid_cap.capabilities & V4L2_CAP_STREAMING)) {
			ALOGE("Err :: unSupport streaming i/o");
			return -1;
		}		
	}

	return ret;
}
static void filewrite_memory(char* name, char* addr, unsigned int size)
{
	FILE *fp;

	fp = fopen(name, "ab+");		
	fwrite( addr, size, 1, fp);
	fclose(fp);
}

/*===========================================================================
FUNCTION
===========================================================================*/
unsigned int  TCC_V4l2_Camera::camif_encode_jpeg(void)
{
	int ret;
	unsigned char *virt_jpeg, *virt_encoded_jpeg;
	uint32_t src_buffer_size = 0;
	#if defined(_TCC8920_) || defined(_TCC8930_)
		const char JPU_Header_Offset = 0x08;
		const char JPU_Stream_Offset = 0x03;
	#endif
	FUNCTION_IN

	if(mDoneVideoSnapshot /* case of video snapshot */) {
		memset(&(mCamDevice.cap_info.jpeg_save_info), 0x00, sizeof(TCCXXX_JPEG_ENC_DATA));
	}

	if(mCamDevice.cap_info.jpeg_save_info.bitstream_size) {
		mCamDevice.cap_info.jpeg_save_info.source_addr	= (unsigned int)PA_JPEG_STREAM_BASE_ADDR;
		mCamDevice.cap_info.jpeg_save_info.width  		= mCamDevice.cap_info.width;
		mCamDevice.cap_info.jpeg_save_info.height 		= mCamDevice.cap_info.height;
		mCamDevice.cap_info.jpeg_save_info.src_format 	= ENC_INPUT_422;
		mCamDevice.cap_info.jpeg_save_info.target_addr	= PA_JPEG_RAW_BASE_ADDR;
		mCamDevice.cap_info.jpeg_save_info.target_size 	= JPEG_RAW_MEM_SIZE;

		if((ret = ioctl(mCamDevice.jpegenc_fd,TCC_JPEGE_IOCTL_MAKE_EXIF, &(mCamDevice.cap_info.jpeg_save_info))) < 0) {
			ALOGE("Err[%s] :: TCC_JPEG_IOCTL_MAKE_EXIFHEADER failed!", strerror(errno));
		}
	} else {
		if(!mbRotateCapture) {
			mCamDevice.cap_info.jpeg_save_info.source_addr 	= (unsigned int)PA_JPEG_RAW_BASE_ADDR;
			mCamDevice.cap_info.jpeg_save_info.width 		= mCamDevice.cap_info.width;
			mCamDevice.cap_info.jpeg_save_info.height 		= mCamDevice.cap_info.height;
			src_buffer_size 								= JPEG_RAW_MEM_SIZE;
		} else {
			mCamDevice.cap_info.jpeg_save_info.source_addr 	= mCamDevice.cap_info.jpeg_save_info.target_addr;
			src_buffer_size 								= mCamDevice.cap_info.width * mCamDevice.cap_info.height * 2;
		}

		mCamDevice.cap_info.jpeg_save_info.q_value 			= mJpegQuality;
		mCamDevice.cap_info.jpeg_save_info.src_format 		= ENC_INPUT_422;
		mCamDevice.cap_info.jpeg_save_info.target_addr 		= (unsigned int)PA_JPEG_STREAM_BASE_ADDR;
		mCamDevice.cap_info.jpeg_save_info.target_size 		= (unsigned int)JPEG_STREAM_MEM_SIZE;

		if(mDoneVideoSnapshot /* case of video snapshot */) {
			mCamDevice.cap_info.jpeg_save_info.width 		= mPreviewWidth;
			mCamDevice.cap_info.jpeg_save_info.height 		= mPreviewHeight;
			mCamDevice.cap_info.jpeg_save_info.src_format 	= ENC_INPUT_420;
		}

	#if defined(_TCC8920_) || defined(_TCC8930_)
		DBUG("addr=0x%x, size=0x%x", mCamDevice.cap_info.jpeg_save_info.source_addr, src_buffer_size);
		DBUG("capture=%d x %d", mCamDevice.cap_info.jpeg_save_info.width, mCamDevice.cap_info.jpeg_save_info.height);

		virt_jpeg 				= mJpegRawAddr;
		virt_encoded_jpeg 		= mJpegStreamAddr;
		DBUG("virt_jpeg[0x%x], virt_encoded_jpeg[0x%x], mCamDevice.fd[0x%x]",virt_jpeg, virt_encoded_jpeg, mCamDevice.fd);

		// Initialize JPU
		memset(&gsJPUEncodeInfo, 0x00, sizeof(conf_enc_t));
		gsJPUEncodeInfo.m_InpPhyYUVBuffAddr 		= (codec_addr_t)mCamDevice.cap_info.jpeg_save_info.source_addr;
		gsJPUEncodeInfo.m_InpVirtYUVBuffAddr 		= (codec_addr_t)virt_jpeg;
		gsJPUEncodeInfo.m_outPhyHeaderBuffAddr		= (codec_addr_t)PA_JPEG_STREAM_BASE_ADDR;
		gsJPUEncodeInfo.m_outVirtHeaderBuffAddr 	= (codec_addr_t)virt_encoded_jpeg;
		gsJPUEncodeInfo.m_iTotalFrames 			= 1;
		gsJPUEncodeInfo.m_iWidth 					= mCamDevice.cap_info.jpeg_save_info.width;
		gsJPUEncodeInfo.m_iHeight 					= mCamDevice.cap_info.jpeg_save_info.height;
		gsJPUEncodeInfo.m_iFramesPerSecond 		= 30;
		gsJPUEncodeInfo.m_iEncQuality 				= 2/*mCamDevice.cap_info.jpeg_save_info.q_value*/;
		if(mDoneVideoSnapshot /* case of video snapshot */) 		gsJPUEncodeInfo.m_iYuvFormat = YUV_FORMAT_420;
		else 												gsJPUEncodeInfo.m_iYuvFormat = YUV_FORMAT_422;

		if(0) { //for frame dump.
			FILE *pFs;
			unsigned int Y, U, V;

			Y = ((unsigned int)virt_jpeg);
			U = ((unsigned int)GET_ADDR_YUV42X_spU(Y , mCamDevice.cap_info.jpeg_save_info.width, mCamDevice.cap_info.jpeg_save_info.height));
			V = ((unsigned int)GET_ADDR_YUV422_spV(U , mCamDevice.cap_info.jpeg_save_info.width, mCamDevice.cap_info.jpeg_save_info.height));
			DBUG("Y[0x%x], U[0x%x], V[0x%x]", Y, U, V);

			pFs = fopen("/sdcard/jpeg_src_main.yuv", "ab+");
			if (!pFs) {
				ALOGE("Cannot open '/sdcard/jpeg_src_main.yuv'");
			} else {
				fwrite( (void*)Y, mCamDevice.cap_info.jpeg_save_info.width*mCamDevice.cap_info.jpeg_save_info.height, 1, pFs);
				fwrite( (void*)U, mCamDevice.cap_info.jpeg_save_info.width*mCamDevice.cap_info.jpeg_save_info.height/2, 1, pFs);
				fwrite( (void*)V, mCamDevice.cap_info.jpeg_save_info.width*mCamDevice.cap_info.jpeg_save_info.height/2, 1, pFs);
				fclose(pFs);
			}
		}

		DBUG("gsJPUEncodeInfo.m_outPhyHeaderBuffAddr = 0x%x , gsJPUEncodeInfo.m_outVirtHeaderBuffAddr = 0x%x", gsJPUEncodeInfo.m_outPhyHeaderBuffAddr, gsJPUEncodeInfo.m_outVirtHeaderBuffAddr);
		DBUG("JPU ENC Start!!");
		tcc_jpu_enc_jpeg(&gsJPUEncodeInfo);
		DBUG("JPU ENC End!!");
		DBUG("JPU ENC INFO:  jpeg_start_addr=0x%x, jpeg_header_size=0x%x, jpeg_stream_size=0x%x, jpeg_total_size=0x%x.", 	\
				gsJPUEncodeInfo.m_outVirtHeaderBuffAddr, gsJPUEncodeInfo.m_outSizeOfHeaderBuff, 							\
				gsJPUEncodeInfo.m_outSizeOfEncodedBuff, gsJPUEncodeInfo.m_outSizeOfTotalBuff);

		mCamDevice.cap_info.jpeg_save_info.header_size 		= 0;
		mCamDevice.cap_info.jpeg_save_info.header_offset 		= 0;
		if(supported_thumbnail) {
			mCamDevice.cap_info.jpeg_save_info.bitstream_size 	= gsJPUEncodeInfo.m_outSizeOfTotalBuff - JPU_Header_Offset - JPU_Stream_Offset;
			mCamDevice.cap_info.jpeg_save_info.bitstream_offset 	= gsJPUEncodeInfo.m_outSizeOfHeaderBuff;
		} else {
			mCamDevice.cap_info.jpeg_save_info.bitstream_size 	= gsJPUEncodeInfo.m_outSizeOfTotalBuff;
			mCamDevice.cap_info.jpeg_save_info.bitstream_offset 	= gsJPUEncodeInfo.m_outSizeOfHeaderBuff;
			return mCamDevice.cap_info.jpeg_save_info.bitstream_size;
		}
	#else
		if((ret = ioctl(mCamDevice.jpegenc_fd,TCC_JPEGE_IOCTL_ENC, &(mCamDevice.cap_info.jpeg_save_info))) < 0) {
			ALOGE("Err[%s] :: TCC_JPEG_IOCTL_V4L2_ENC failed!", strerror(errno));
		}
	#endif

		DBUG("Main-Stream Encode Done");
		DBUG("Stream[0x%x - 0x%x], Header[0x%x - 0x%x]!!", 																\
									(PA_JPEG_STREAM_BASE_ADDR + mCamDevice.cap_info.jpeg_save_info.bitstream_offset), 	\
									mCamDevice.cap_info.jpeg_save_info.bitstream_size, 									\
									(PA_JPEG_STREAM_BASE_ADDR + mCamDevice.cap_info.jpeg_save_info.header_offset), 		\
									mCamDevice.cap_info.jpeg_save_info.header_size);

	#if defined(MAKE_THUMB_JPEG)
		if(supported_thumbnail) {
			int width_ratio, height_ratio, sampleSize, scaler_fd, offset;
			SCALER_TYPE msc_data;
			TCCXXX_JPEG_DEC_DATA dec_data;
			TCCXXX_JPEG_ENC_DATA enc_data; 
			TCCXXX_JPEG_ENC_EXIF_DATA exif_data;
			struct pollfd poll_evt[1];
			#if defined(_TCC9300_) || defined(_TCC8800_) || defined(_TCC8920_) || defined(_TCC8930_)
			unsigned int dest_buff_info[4];
			unsigned char *virt_jpeg, *virt_jpeg_header, *virt_jpeg_stream;
			#endif

			//Decode for thumbnail !!
		#if defined(_TCC8920_) || defined(_TCC8930_)
			#if defined(FEATURE_NOT_DECODE_JPEG_FOR_THUMB)
				dec_data.width  		= mCamDevice.cap_info.jpeg_save_info.width;
				dec_data.height 		= mCamDevice.cap_info.jpeg_save_info.height;
				if(mDoneVideoSnapshot /* case of video snapshot */) 		dec_data.image_format = 1; // IMAGE_CHROMA_420
				else 												dec_data.image_format = 2; // IMAGE_CHROMA_422
			#else // FEATURE_NOT_DECODE_JPEG_FOR_THUMB
				// todo: 
			#endif // FEATURE_NOT_DECODE_JPEG_FOR_THUMB
		#elif defined(_TCC9300_) || defined(_TCC8800_)
			if(!mUvc_camera)
			{
				// copy to jpeg header & stream
				virt_jpeg = (unsigned char *)mmap(0, JPEG_RAW_MEM_SIZE, (PROT_READ|PROT_WRITE), MAP_SHARED, mCamDevice.fd, PA_JPEG_RAW_BASE_ADDR);
				virt_jpeg_stream = (unsigned char *)mmap(0, JPEG_STREAM_MEM_SIZE, (PROT_READ|PROT_WRITE), MAP_SHARED, mCamDevice.fd,
														(PA_JPEG_STREAM_BASE_ADDR + mCamDevice.cap_info.jpeg_save_info.bitstream_offset));
				virt_jpeg_header = virt_jpeg_stream + mCamDevice.cap_info.jpeg_save_info.bitstream_size;
				memcpy(virt_jpeg, virt_jpeg_header, mCamDevice.cap_info.jpeg_save_info.header_size);
				memcpy((virt_jpeg + mCamDevice.cap_info.jpeg_save_info.header_size), virt_jpeg_stream, mCamDevice.cap_info.jpeg_save_info.bitstream_size);
				DBUG("jpeg_data addr = 0x%x, jpeg_data size = 0x%x", (unsigned int)virt_jpeg, \
							(mCamDevice.cap_info.jpeg_save_info.header_size + mCamDevice.cap_info.jpeg_save_info.bitstream_size));

				// initialize vpu
				memset(&gsVDecInput, 0x00, sizeof(vdec_input_t));
				memset(&gsVDecOutput, 0x00, sizeof(vdec_output_t));
				memset(&gsVDecInit, 0x00, sizeof(vdec_init_t));
				memset(&gsVDecUserInfo, 0x00, sizeof(vdec_user_info_t));
				if(vjpeg_decode_operation(NULL, NULL, 0, mCamDevice.cap_info.width, mCamDevice.cap_info.height, \
										  NULL, NULL, NULL, NULL, NULL, NULL, 0, VDEC_INIT) < 0) {
					ALOGE("Thumbnail - VPU Operation Failed(VDEC_INIT)!!");
					goto thumb_exception;
				}

				if(mVpuState != VPU_INITED) {
					ALOGE("Thumbnail - VPU can't use. VpuState is %d!!", mVpuState);
					goto thumb_exception;
				}

				// jpeg header parsing
				memset(&dec_data, 0x00, sizeof(TCCXXX_JPEG_DEC_DATA));
				if(mVpuState == VPU_INITED) {
					if(vjpeg_decode_operation(PA_JPEG_RAW_BASE_ADDR, (unsigned int)virt_jpeg,
									(mCamDevice.cap_info.jpeg_save_info.header_size + mCamDevice.cap_info.jpeg_save_info.bitstream_size),
									mCamDevice.cap_info.width, mCamDevice.cap_info.height,
									&(dec_data.width), &(dec_data.height),
									&(dec_data.pad_width), &(dec_data.pad_height),
									&(dec_data.image_format), (unsigned char *)dest_buff_info,
									0, VDEC_DEC_SEQ_HEADER) < 0) {
						ALOGE("Thumbnail - JPEGD Operation Failed(vpu init)!!");
						goto thumb_exception;
					}
				}

				// jpeg decoding
				if(vjpeg_decode_operation(PA_JPEG_RAW_BASE_ADDR, (unsigned int)virt_jpeg, 													\
									(mCamDevice.cap_info.jpeg_save_info.header_size + mCamDevice.cap_info.jpeg_save_info.bitstream_size), 	\
									mCamDevice.cap_info.width, mCamDevice.cap_info.height, 													\
									&(dec_data.width), &(dec_data.height), 																	\
									&(dec_data.pad_width), &(dec_data.pad_height), 															\
									&(dec_data.image_format), (unsigned char *)dest_buff_info, 												\
									0, VDEC_DECODE) < 0) {
					ALOGE("Thumbnail - JPEGD Operation Failed(vpu decode)!!");
					goto thumb_exception;
				}

				// close vpu
				if(vjpeg_decode_operation(NULL, NULL, 0, mCamDevice.cap_info.width, mCamDevice.cap_info.height, \
										  NULL, NULL,NULL, NULL, NULL, NULL, 0, VDEC_CLOSE) < 0) {
					ALOGE("Thumbnail - JPEGD Operation Failed(VDEC_CLOSE)!!");
					goto thumb_exception;
				}

				if(munmap((void *)virt_jpeg, JPEG_RAW_MEM_SIZE) < 0) {
					ALOGE("Err[%s] :: UNMAP failed. addr = 0x%x, size = 0x%x", strerror(errno), (unsigned int)virt_jpeg, JPEG_RAW_MEM_SIZE);
				}

				if(munmap((void *)virt_jpeg_stream, JPEG_STREAM_MEM_SIZE) < 0) {
					ALOGE("Err[%s] :: UNMAP failed. addr = 0x%x, size = 0x%x", strerror(errno), (unsigned int)virt_jpeg_stream, JPEG_STREAM_MEM_SIZE);
				}
				DBUG("Decoded Y-addr = 0x%x, U-addr = 0x%x, V-addr = 0x%x", dest_buff_info[0], dest_buff_info[1], dest_buff_info[2]);
			}
		#else // _TCC9200_ or _TCC8900_
			memset(&dec_data, 0x00, sizeof(TCCXXX_JPEG_DEC_DATA));
			dec_data.file_length 		= mCamDevice.cap_info.jpeg_save_info.header_size;
			dec_data.split_body 		= 1;
			dec_data.stream_src_addr 	= (PA_JPEG_STREAM_BASE_ADDR + mCamDevice.cap_info.jpeg_save_info.bitstream_offset);
			dec_data.stream_src_len	 	= mCamDevice.cap_info.jpeg_save_info.bitstream_size;
			dec_data.header_src_addr 	= (PA_JPEG_STREAM_BASE_ADDR + mCamDevice.cap_info.jpeg_save_info.header_offset);
			dec_data.header_src_len  	= mCamDevice.cap_info.jpeg_save_info.header_size + 2;

			dec_data.target_addr	 	= PA_PREVIEW_BASE_ADDR;
			dec_data.target_size	 	= PMEM_CAM_SIZE;

			width_ratio  = (mCamDevice.cap_info.width/THUMB_WIDTH);
			height_ratio = (mCamDevice.cap_info.height/THUMB_HEIGHT);
			if(width_ratio > height_ratio) 		sampleSize = height_ratio;
			else 								sampleSize = width_ratio;
			
			switch(sampleSize) {
				case 1: dec_data.ratio = NO_SCALE; break;
				case 2: dec_data.ratio = HALF_SCALE; break;
				default:
				case 4: dec_data.ratio = QUARTER_SCALE; break;
			}

			if(ioctl(mCamDevice.jpegdec_fd,TCC_JPEGD_IOCTL_DEC, &dec_data) < 0) {
				ALOGE( "JPEG_IOCTL_DEC Failed!! ");
				goto thumb_exception;
			}
		#endif
			DBUG("Thumb-Stream Decode Done");
			
			//Scaler for thumbnail !!
			memset(&msc_data, 0x00, sizeof(SCALER_TYPE));
			msc_data.responsetype 	= SCALER_INTERRUPT;
			msc_data.src_ImgWidth 	= dec_data.width;
			msc_data.src_ImgHeight 	= dec_data.height;
		#if defined(_TCC8920_) || defined(_TCC8930_)
			#if defined(FEATURE_NOT_DECODE_JPEG_FOR_THUMB)
				msc_data.src_Yaddr 		= (char *)mCamDevice.cap_info.jpeg_save_info.source_addr;
			#else // FEATURE_NOT_DECODE_JPEG_FOR_THUMB
				// todo: 
			#endif // FEATURE_NOT_DECODE_JPEG_FOR_THUMB
			msc_data.src_Uaddr 		= (char *)GET_ADDR_YUV42X_spU(msc_data.src_Yaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight);
			if(dec_data.image_format == IMAGE_CHROMA_420) {
				msc_data.src_fmt 	= SCALER_YUV420_sp;
				msc_data.src_Vaddr 	= (char *)GET_ADDR_YUV420_spV(msc_data.src_Uaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight);
			} else {
				msc_data.src_fmt 	= SCALER_YUV422_sp;
				msc_data.src_Vaddr 	= (char *)GET_ADDR_YUV422_spV(msc_data.src_Uaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight);
			}
		#elif defined(_TCC9300_) || defined(_TCC8800_)
			msc_data.src_Yaddr 		= (char *)dest_buff_info[0];
			msc_data.src_Uaddr 		= (char *)dest_buff_info[1];
			msc_data.src_Vaddr 		= (char *)dest_buff_info[2];
			if(dec_data.image_format == 0 || dec_data.image_format == 4) 	msc_data.src_fmt = SCALER_YUV420_sp;
			else 															msc_data.src_fmt = SCALER_YUV422_sp;
		#else
			msc_data.src_Yaddr 		= (char*)PA_PREVIEW_BASE_ADDR;
			msc_data.src_Uaddr 		= (char*)GET_ADDR_YUV42X_spU(msc_data.src_Yaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight );
			if(dec_data.image_format == IMAGE_CHROMA_420) {
				msc_data.src_fmt 		= SCALER_YUV420_sp;
				msc_data.src_Vaddr 	= (char*)GET_ADDR_YUV420_spV(msc_data.src_Uaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight );
			} else {
				msc_data.src_fmt 		= SCALER_YUV422_sp;
				msc_data.src_Vaddr 	= (char*)GET_ADDR_YUV422_spV(msc_data.src_Uaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight );
			}
		#endif
			msc_data.src_winLeft 		= 0;
			msc_data.src_winTop 		= 0;
			msc_data.src_winRight 	 	= dec_data.width;
			msc_data.src_winBottom 	= dec_data.height;

			msc_data.dest_ImgWidth	= THUMB_WIDTH;
			msc_data.dest_ImgHeight	= THUMB_HEIGHT;
		#if defined(_TCC8920_) || defined(_TCC8930_)
			#if defined(FEATURE_NOT_DECODE_JPEG_FOR_THUMB)
				msc_data.dest_Yaddr		= (char *)PA_JPEG_HEADER_BASE_ADDR;
			#else // FEATURE_NOT_DECODE_JPEG_FOR_THUMB
				// todo: 
			#endif // FEATURE_NOT_DECODE_JPEG_FOR_THUMB
		#else
			msc_data.dest_Yaddr		= (char *)(PA_JPEG_RAW_BASE_ADDR + (JPEG_RAW_MEM_SIZE/2));
		#endif
			msc_data.dest_Uaddr		= (char *)GET_ADDR_YUV42X_spU(msc_data.dest_Yaddr , msc_data.dest_ImgWidth, msc_data.dest_ImgHeight);
			msc_data.dest_Vaddr		= (char *)GET_ADDR_YUV422_spV(msc_data.dest_Uaddr , msc_data.dest_ImgWidth, msc_data.dest_ImgHeight);
			msc_data.dest_fmt 		= SCALER_YUV422_sp;
			msc_data.dest_winLeft 		= 0;
			msc_data.dest_winTop 		= 0;
			msc_data.dest_winRight 	= THUMB_WIDTH;
			msc_data.dest_winBottom 	= THUMB_HEIGHT;

			if((scaler_fd = open(SCALER_DEVICE,O_RDWR)) < 0) {
				ALOGE("can't open[%s] '%s'", strerror(errno), SCALER_DEVICE);
				goto thumb_exception;
			}

			if(ioctl(scaler_fd, TCC_SCALER_IOCTRL, &msc_data) != 0) {
				ALOGE( "SCALER Operation Failed!! ");
				goto thumb_exception;
			}

			if(msc_data.responsetype == SCALER_INTERRUPT) {
				int ret;				
				memset(poll_evt, 0, sizeof(poll_evt));
				poll_evt[0].fd = scaler_fd;
				poll_evt[0].events = POLLIN;
				ret = poll((struct pollfd*)poll_evt, 1, 400);

				if(ret < 0) 		ALOGE("M2M_Scaler Poll Error!! \n");
				else if(ret == 0) 	ALOGE("M2M_Scaler Poll Timeout!! \n");
				else if(ret > 0) {
					if(poll_evt[0].revents & POLLERR) 	ALOGE("M2M_Scaler Poll POLLERR\n");
				}
			}
			close(scaler_fd);
			DBUG("Thumb-Stream Scaler Done");
			DBUG("Thumb Scaler Info : Src_W=%d, Src_H=%d, Src_W_L=%d, Src_W_T=%d, Src_W_R=%d, Src_W_B=%d", 	\
				msc_data.src_ImgWidth, msc_data.src_ImgHeight, msc_data.src_winLeft, msc_data.src_winTop, msc_data.src_winRight, msc_data.src_winBottom);
			DBUG("Thumb Scaler Info : Dst_W=%d, Dst_H=%d, Dst_W_L=%d, Dst_W_T=%d, Dst_W_R=%d, Dst_W_B=%d", 	\
				msc_data.dest_ImgWidth, msc_data.dest_ImgHeight, msc_data.dest_winLeft, msc_data.dest_winTop, msc_data.dest_winRight, msc_data.dest_winBottom);

			//Encode for thumbnail !!
			memset(&enc_data, 0x00, sizeof(TCCXXX_JPEG_ENC_DATA));
		#if defined(_TCC8920_) || defined(_TCC8930_)
			#if defined(FEATURE_NOT_DECODE_JPEG_FOR_THUMB)
			virt_jpeg 				= mJpegHeaderAddr;
			virt_encoded_jpeg 		= mJpegRawAddr;
			DBUG("virt_jpeg[0x%x] virt_encoded_jpeg[0x%x] mCamDevice.fd[0x%x]",virt_jpeg, virt_encoded_jpeg, mCamDevice.fd);
			
			// Initialize JPU
			memset(&gsJPUEncodeInfo, 0x00, sizeof(conf_enc_t));
			gsJPUEncodeInfo.m_InpPhyYUVBuffAddr 		= (codec_addr_t)msc_data.dest_Yaddr;
			gsJPUEncodeInfo.m_InpVirtYUVBuffAddr 		= (codec_addr_t)virt_jpeg;
			gsJPUEncodeInfo.m_outPhyHeaderBuffAddr		= (codec_addr_t)PA_JPEG_RAW_BASE_ADDR;
			gsJPUEncodeInfo.m_outVirtHeaderBuffAddr 	= (codec_addr_t)virt_encoded_jpeg;
			gsJPUEncodeInfo.m_iTotalFrames 			= 1;
			gsJPUEncodeInfo.m_iWidth 					= THUMB_WIDTH;
			gsJPUEncodeInfo.m_iHeight 					= THUMB_HEIGHT;
			gsJPUEncodeInfo.m_iFramesPerSecond 		= 30;
			gsJPUEncodeInfo.m_iEncQuality				= 2 /*mCamDevice.cap_info.jpeg_save_info.q_value*/;
			gsJPUEncodeInfo.m_iYuvFormat 				= YUV_FORMAT_422;

			if(0) //for frame dump.
			{
				FILE *pFs;
				unsigned int Y, U, V;

				Y = ((unsigned int)virt_jpeg);
				U = ((unsigned int)GET_ADDR_YUV42X_spU(Y , THUMB_WIDTH, THUMB_HEIGHT));
				V = ((unsigned int)GET_ADDR_YUV422_spV(U , THUMB_WIDTH, THUMB_HEIGHT));
				DBUG("Y[0x%x], U[0x%x], V[0x%x]", Y, U, V);

				pFs = fopen("/sdcard/jpeg_src_thumb.yuv", "ab+");
				if (!pFs) {
					ALOGE("Cannot open '/sdcard/jpeg_src_thumb.yuv'");
				}
				else
				{
					fwrite( (void*)Y, THUMB_WIDTH*THUMB_HEIGHT, 1, pFs);
					fwrite( (void*)U, THUMB_WIDTH*THUMB_HEIGHT/2, 1, pFs);
					fwrite( (void*)V, THUMB_WIDTH*THUMB_HEIGHT/2, 1, pFs);
					fclose(pFs);
				}
			}

			DBUG("JPU ENC(thumbnail) Start!!");
			tcc_jpu_enc_jpeg(&gsJPUEncodeInfo);
			DBUG("JPU ENC(thumbnail) End!!");
			DBUG("JPU ENC(thumbnail) INFO:  jpeg_start_addr=0x%08x, jpeg_header_size=0x%x, jpeg_stream_size=0x%x, jpeg_total_size=0x%x.", 	\
					gsJPUEncodeInfo.m_outVirtHeaderBuffAddr, gsJPUEncodeInfo.m_outSizeOfHeaderBuff, 										\
					gsJPUEncodeInfo.m_outSizeOfEncodedBuff, gsJPUEncodeInfo.m_outSizeOfTotalBuff);
			#else // FEATURE_NOT_DECODE_JPEG_FOR_THUMB
				// todo: 
			#endif // FEATURE_NOT_DECODE_JPEG_FOR_THUMB
		#else
			enc_data.source_addr 	= (PA_JPEG_RAW_BASE_ADDR + (JPEG_RAW_MEM_SIZE/2));
			enc_data.width 			= THUMB_WIDTH;
			enc_data.height 		= THUMB_HEIGHT;
			enc_data.q_value 		= JPEG_MEDIUM;
			enc_data.src_format 	= ENC_INPUT_422;
			enc_data.target_addr	= PA_JPEG_RAW_BASE_ADDR;
			enc_data.target_size	= (JPEG_RAW_MEM_SIZE/2);
			
			if((ret = ioctl(mCamDevice.jpegenc_fd, TCC_JPEGE_IOCTL_ENC, &enc_data)) < 0) {
				ALOGE("Err [%s] :: TCC_JPEGE_IOCTL_ENC failed!", strerror(errno));
				goto thumb_exception;
			}
		#endif
			DBUG("Thumb-Stream Encode Done");

			//Make Exif-Header with thumb-jpg !!
			memset(&exif_data, 0x00, sizeof(TCCXXX_JPEG_ENC_EXIF_DATA));
		#if defined(_TCC8920_) || defined(_TCC8930_)
			exif_data.thumbjpg_addr 	= (uint32_t)PA_JPEG_RAW_BASE_ADDR;
			exif_data.thumbjpg_size 	= gsJPUEncodeInfo.m_outSizeOfTotalBuff;
			exif_data.thumb_width 		= THUMB_WIDTH;
			exif_data.thumb_height 		= THUMB_HEIGHT;
			exif_data.bitstream_size 	= mCamDevice.cap_info.jpeg_save_info.bitstream_size;
			exif_data.bitstream_width 	= mCamDevice.cap_info.jpeg_save_info.width;
			exif_data.bitstream_height 	= mCamDevice.cap_info.jpeg_save_info.height;
			exif_data.header_addr 		= (unsigned int)PA_JPEG_HEADER_BASE_ADDR;
			exif_data.header_size 		= 60*1024;
		#else
			exif_data.thumbjpg_addr 	= enc_data.target_addr + enc_data.bitstream_offset;
			exif_data.thumbjpg_size 	= enc_data.bitstream_size;
			exif_data.thumb_width 		= THUMB_WIDTH;
			exif_data.thumb_height 		= THUMB_HEIGHT;
			exif_data.bitstream_size 	= mCamDevice.cap_info.jpeg_save_info.bitstream_size;
			exif_data.bitstream_width 	= mCamDevice.cap_info.jpeg_save_info.width;
			exif_data.bitstream_height 	= mCamDevice.cap_info.jpeg_save_info.height;
			exif_data.header_addr 		= PA_JPEG_STREAM_BASE_ADDR + mCamDevice.cap_info.jpeg_save_info.header_offset;
			exif_data.header_size 		= 100*1024;
		#endif

			//set time_stamp info with EXIF.
			{
				time_t 		rawtime;
				struct tm 	*timeinfo;
				
				time(&rawtime);
				timeinfo = localtime(&rawtime);

				exif_data.time_stamp.year 	= (unsigned short)timeinfo->tm_year + 1900;
				exif_data.time_stamp.month 	= (unsigned short)timeinfo->tm_mon + 1;
				exif_data.time_stamp.date 	= (unsigned short)timeinfo->tm_mday;
				exif_data.time_stamp.hour 	= (unsigned short)timeinfo->tm_hour;
				exif_data.time_stamp.min 	= (unsigned short)timeinfo->tm_min;
				exif_data.time_stamp.sec 	= (unsigned short)timeinfo->tm_sec;

				DBUG("Current Local Date and Time:  %d-%d-%d  %d:%d:%d.", exif_data.time_stamp.year, exif_data.time_stamp.month, \
					exif_data.time_stamp.date, exif_data.time_stamp.hour, exif_data.time_stamp.min, exif_data.time_stamp.sec);
				//ALOGD("Current Local Date and Time:  %s", asctime(timeinfo));
			}

			// set rotation info with EXIF.
			exif_data.rotation = mRotation;

			//set GPS info with EXIF.
			if(supported_gps) {
				struct tm *gpstimeinfo;
				time_t    rawtime = GPSInfo.Timestamp;

				exif_data.gps_info.Supported 			= supported_gps;
				exif_data.gps_info.Latitude.degrees 	= GPSInfo.Latitude.degrees;
				exif_data.gps_info.Latitude.minutes 	= GPSInfo.Latitude.minutes;
				exif_data.gps_info.Latitude.seconds 	= GPSInfo.Latitude.seconds;
				exif_data.gps_info.Longitude.degrees 	= GPSInfo.Longitude.degrees;
				exif_data.gps_info.Longitude.minutes 	= GPSInfo.Longitude.minutes;
				exif_data.gps_info.Longitude.seconds 	= GPSInfo.Longitude.seconds;
				exif_data.gps_info.Altitude 			= GPSInfo.Altitude;

				gpstimeinfo = localtime(&rawtime);

				exif_data.gps_info.time_stamp.year 	= (unsigned short)gpstimeinfo->tm_year + 1900;
				exif_data.gps_info.time_stamp.month 	= (unsigned short)gpstimeinfo->tm_mon + 1;
				exif_data.gps_info.time_stamp.date 	= (unsigned short)gpstimeinfo->tm_mday;
				exif_data.gps_info.time_stamp.hour 	= (unsigned short)gpstimeinfo->tm_hour;
				exif_data.gps_info.time_stamp.min 	= (unsigned short)gpstimeinfo->tm_min;
				exif_data.gps_info.time_stamp.sec 	= (unsigned short)gpstimeinfo->tm_sec;

				DBUG("Current Local Date and Time:  %d-%d-%d  %d:%d:%d.", exif_data.gps_info.time_stamp.year, exif_data.gps_info.time_stamp.month, \
					exif_data.gps_info.time_stamp.date, exif_data.gps_info.time_stamp.hour, exif_data.gps_info.time_stamp.min, exif_data.gps_info.time_stamp.sec);

				if(GPSInfo.Processing_Method != NULL)
					memcpy(exif_data.gps_info.Processing_Method, GPSInfo.Processing_Method, strlen(GPSInfo.Processing_Method)+1);
				else
					*exif_data.gps_info.Processing_Method = NULL;
			}

			if((ret = ioctl(mCamDevice.jpegenc_fd, TCC_JPEGE_IOCTL_MAKE_EXIF, &exif_data)) < 0) {
				ALOGE("Err [%s] :: TCC_JPEGE_IOCTL_MAKE_EXIF failed!", strerror(errno));
				goto thumb_exception;
			}

			mCamDevice.cap_info.jpeg_save_info.header_size 		= exif_data.header_size;
			#if defined(_TCC8920_) || defined(_TCC8930_)
				mCamDevice.cap_info.jpeg_save_info.header_offset 	= 0;
			#endif
			DBUG("Thumb-Stream Make-Header Done");

			DBUG("EXIF_DATA:  exif_data.header_addr = 0x%x , exif_data.header_size = 0x%x.", exif_data.header_addr, exif_data.header_size);
		}

		if(mDoneVideoSnapshot /* case of video snapshot */) 		mDoneVideoSnapshot = false;


	#endif // MAKE_THUMB_JPEG
	}
	
	DBUG("Encode Done = (%d)%d KB (%d / %d)!!", (mCamDevice.cap_info.jpeg_save_info.bitstream_size + mCamDevice.cap_info.jpeg_save_info.header_size), 	\
								(mCamDevice.cap_info.jpeg_save_info.bitstream_size + mCamDevice.cap_info.jpeg_save_info.header_size)/1024, 				\
								mCamDevice.cap_info.jpeg_save_info.header_size/1024, mCamDevice.cap_info.jpeg_save_info.bitstream_size/1024);


	return mCamDevice.cap_info.jpeg_save_info.bitstream_size + mCamDevice.cap_info.jpeg_save_info.header_size;

thumb_exception:
	ALOGE("Thumb-Stream Make fail!! No-thumb jpeg captured!!");
	DBUG("Encode Done = (%d)%d KB (%d / %d)!!", (mCamDevice.cap_info.jpeg_save_info.bitstream_size + mCamDevice.cap_info.jpeg_save_info.header_size), 	\
								(mCamDevice.cap_info.jpeg_save_info.bitstream_size + mCamDevice.cap_info.jpeg_save_info.header_size)/1024, 				\
								mCamDevice.cap_info.jpeg_save_info.header_size/1024, mCamDevice.cap_info.jpeg_save_info.bitstream_size/1024);

	FUNCTION_OUT
	return mCamDevice.cap_info.jpeg_save_info.bitstream_size + mCamDevice.cap_info.jpeg_save_info.header_size;

}


/*===========================================================================
FUNCTION
===========================================================================*/
unsigned int TCC_V4l2_Camera::camif_get_frame(uint32_t *used_bytes) //buffer is physical address!!
{
	struct v4l2_buffer buf;
    int ret;

	//FUNCTION_IN

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	
	#if defined(FEATURE_SENSOR_STABLE_DELAY)
		if(mSensor_StableTime){
			usleep(500*1000);
			mSensor_StableTime = false;
			ALOGE("%s:  Sensor Stable Time!!", __FUNCTION__);		
		}
	#endif
	
	if((ret = ioctl(mCamDevice.fd, VIDIOC_DQBUF, &buf)) < 0) {
			ALOGE("Err :: VIDIOC_DQBUF fail preview buffer");
		return ERR_GET_BUFFER;
	}
	*used_bytes = buf.bytesused;
	if((buf.index < 0) || (buf.index > mPreviewBufferCount)) {
		ALOGE("DQBUF Index = %d.", buf.index);
	}

	//DBUG("%s:  get frame index=%d.", __FUNCTION__, buf.index);
	//FUNCTION_OUT

	return buf.index;
}


/*===========================================================================
FUNCTION
===========================================================================*/
void TCC_V4l2_Camera::camif_set_queryctrl(unsigned int ctrl_id, int value)
{
	struct v4l2_queryctrl q;
	struct v4l2_control c;

	memset(&q, 0, sizeof(struct v4l2_queryctrl));
	q.id = ctrl_id;

	if(ioctl(mCamDevice.fd, VIDIOC_QUERYCTRL, &q) < 0 ) {
		ALOGE("Err :: VIDIOC_QUERYCTRL() set value");
	}

	memset(&c, 0, sizeof(struct v4l2_control));
	c.id = ctrl_id;

	if(c.id == V4L2_CID_EXPOSURE) {
		c.value = value;
		DBUG("V4L2_CID_EXPOSURE value[%d]", value);
	} else {
		if(value < 0) 	c.value = q.default_value;
		else 			c.value = value;
	}

	if(ioctl(mCamDevice.fd, VIDIOC_S_CTRL, &c) < 0) {
		ALOGE("Err :: VIDIOC_S_CTRL() set value");
	}
}

/*===========================================================================
FUNCTION
===========================================================================*/
int TCC_V4l2_Camera::camif_set_resolution (int width, int height)
{
	FUNCTION_IN

	if(camif_init_format(width, height) < 0) {
		ALOGE("Err :: camif_init_format() failed");
		return -1;
	}

	if(mCamDevice.cam_mode != MODE_CAPTURE || mUvc_camera) {
		if(camif_init_buffer() < 0) {
			ALOGE("Err :: camif_init_buffer() failed");
			return -1;
		}
	}

	FUNCTION_OUT

	return 0;
}


/*===========================================================================
FUNCTION
===========================================================================*/
int TCC_V4l2_Camera::camif_start_stream(int *fps)
{
    struct v4l2_buffer buf;
    int type, ret;

	FUNCTION_IN

	if(mCamDevice.cam_mode == MODE_PREVIEW) return 0;

	#if defined(VPU_HW_DECODE)
	if(mUvc_camera) {
		#if defined(TCC_8925S_INCLUDE) || defined(TCC_8935S_INCLUDE)
			mVpuState = VPU_INITED;
		#else
		memset(&gsVDecInput, 0x00, sizeof(vdec_input_t));
		memset(&gsVDecOutput, 0x00, sizeof(vdec_output_t));
		memset(&gsVDecInit, 0x00, sizeof(vdec_init_t));
		memset(&gsVDecUserInfo, 0x00, sizeof(vdec_user_info_t));

		if(vjpeg_decode_operation(NULL, NULL, 0, mCamDevice.preview_width, mCamDevice.preview_height, NULL, NULL, NULL, NULL, NULL, NULL, 0, VDEC_INIT) < 0)
		{
			ALOGE( "VPU Operation Failed (VDEC_INIT)!! ");
			return -1;
		}
		#endif
	}
	#endif

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	for(buf.index=0; buf.index < (unsigned int)mPreviewBufferCount; buf.index++) {
	    buf.type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    buf.memory 	= V4L2_MEMORY_MMAP;
	    if((ret = ioctl(mCamDevice.fd, VIDIOC_QBUF, &buf)) < 0) {
			ALOGE("Err[%s] :: VIDIOC_QBUF faild!!", strerror(errno));
			return ret;
	    }
		DBUG("VIDIOC_QBUF is Success(%d).", buf.index);
	}

    if(mUvc_camera) {
		struct v4l2_streamparm parm;
        
        if(mCamDevice.preview_width > DEF_UVC_WIDTH && mCamDevice.preview_height > DEF_UVC_HEIGHT)
            *fps = mUvc_HDFps;
        else
            *fps = mDef_MaxFps; 

		parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if(ioctl(mCamDevice.fd, VIDIOC_G_PARM, &parm) < 0)
			ALOGE( "Err[%s] :: VIDIOC_S_PARM %d fps Failed!! ", strerror(errno), parm.parm.output.timeperframe.denominator);

		if((unsigned int)*fps != parm.parm.output.timeperframe.denominator) {
			parm.parm.output.timeperframe.numerator = 1;
			parm.parm.output.timeperframe.denominator = *fps;
			
			if(ioctl(mCamDevice.fd, VIDIOC_S_PARM, &parm) < 0)
				ALOGE( "Err[%s] :: VIDIOC_S_PARM %d fps Failed!! ", strerror(errno), parm.parm.output.timeperframe.denominator);
		}
    }

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if((ret = ioctl (mCamDevice.fd, VIDIOC_STREAMON, &type)) < 0) {
		ALOGE("Err[%s] :: VIDIOC_STREAMON faild!!", strerror(errno));
		return ret;
	}

	mCamDevice.cam_mode = MODE_PREVIEW;

	FUNCTION_OUT
	
	return ret;
}

/*===========================================================================
FUNCTION
===========================================================================*/
int TCC_V4l2_Camera::camif_stop_stream()
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;

	FUNCTION_IN

	if(mCamDevice.cam_mode == MODE_PREVIEW_STOP) return 0;

	if((ret = ioctl(mCamDevice.fd, VIDIOC_STREAMOFF, &type)) < 0) 	{
		ALOGE("Err[%s] :: VIDIOC_STREAMOFF faild!!", strerror(errno));
		ALOGD("Err[%s] :: VIDIOC_STREAMOFF faild!!", strerror(errno));
		return -1;
	}

	mCamDevice.cam_mode = MODE_PREVIEW_STOP;
	camif_uninit_buffer();

	#if defined(VPU_HW_DECODE)
	if(mUvc_camera) {
		#if defined(TCC_8925S_INCLUDE) || defined(TCC_8935S_INCLUDE)
			mVpuState = VPU_CLOSED;
		#else
		if(vjpeg_decode_operation(NULL, NULL, 0, mCamDevice.preview_width, mCamDevice.preview_height, NULL, NULL, NULL, NULL, NULL, NULL, 0, VDEC_CLOSE) < 0)
			ALOGE( "JPEGD Operation Failed (VDEC_CLOSE)!! ");
		#endif
	}
	#endif

	FUNCTION_OUT

	return 0;
}

/*===========================================================================
FUNCTION
===========================================================================*/
int  TCC_V4l2_Camera::camif_capture(int nRotationOfCapture, bool bIsitPortraitLCD)
{
	int ret = 0, retry_cnt = 0, jpeg_qval = 0;
	struct pollfd capture_event;
    struct v4l2_buffer buf;

	retry_cnt = 0;
	jpeg_qval = mCamDevice.cap_info.jpeg_quality;
	
	FUNCTION_IN

	mCamDevice.cam_mode = MODE_CAPTURED;
	mbRotateCapture = false;

retry_capture:	
	if(mUvc_camera) {
		uint32_t curr_idx, used_bytes;
		int type;		

		#if (0) //def VPU_HW_DECODE //1. check whether this file is right or not.	
		if(vjpeg_decode_operation(NULL, NULL, 0, mCamDevice.preview_width, mCamDevice.preview_height, NULL, NULL, NULL, NULL, NULL, NULL, 0, VDEC_INIT) < 0)
		{
			ALOGE( "VPU Operation Failed (VDEC_INIT)!! ");
			return -1;
		}
		memset(&gsVDecInput, 0x00, sizeof(vdec_input_t));
		memset(&gsVDecOutput, 0x00, sizeof(vdec_output_t));
		memset(&gsVDecInit, 0x00, sizeof(vdec_init_t));
		memset(&gsVDecUserInfo, 0x00, sizeof(vdec_user_info_t));				
		#endif

    	for(buf.index=0; buf.index < mCamDevice.Buffer_cnt; buf.index++) {
    	    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	    buf.memory      = V4L2_MEMORY_MMAP;
    	    if((ret = ioctl (mCamDevice.fd, VIDIOC_QBUF, &buf)) < 0) {
    			ALOGE("Err[%s] :: VIDIOC_QBUF faild!!", strerror(errno));
    			return ret;
    	    }
    	}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if((ret = ioctl (mCamDevice.fd, VIDIOC_STREAMON, &type)) < 0) {
			ALOGE("Err[%s] :: VIDIOC_STREAMON faild!!", strerror(errno));
			return ret;
		}

retry_frame:
		if(retry_cnt == (CAP_RETRY_COUNT*2)) {
			ALOGE("CAP-FAIL:: Retry-Cap %d", retry_cnt);
			if((ret = ioctl (mCamDevice.fd, VIDIOC_STREAMOFF, &type)) < 0) {
				ALOGE("Err[%s] :: VIDIOC_STREAMOFF faild!!", strerror(errno));
				return -1;
			}
			
			#if (0)//def VPU_HW_DECODE //1. check whether this file is right or not.	
			if(vjpeg_decode_operation(NULL, NULL, 0, mCamDevice.preview_width, mCamDevice.preview_height, NULL, NULL, NULL, NULL, NULL, NULL, 0, VDEC_CLOSE) < 0)
			{
				ALOGE( "JPEGD Operation Failed (VDEC_CLOSE)!! ");
			}
			#endif
			
			return -1;
		} else {
			curr_idx = camif_get_frame(&used_bytes);
			if(curr_idx > ((uint32_t)mCamDevice.Buffer_cnt-1)) {
				ALOGE("Error Get_Frame :: %d > %d", curr_idx, mCamDevice.Buffer_cnt-1); 
				retry_cnt++;	
				usleep(33*1000);
				goto retry_frame;
			}

			if(get_currUvcJpeg(used_bytes, curr_idx) < 0) {
				retry_cnt++;	
				release_buffer(curr_idx);
				goto retry_frame;
			}

			if((ret = ioctl (mCamDevice.fd, VIDIOC_STREAMOFF, &type)) < 0) {
				ALOGE("Err[%s] :: VIDIOC_STREAMOFF faild!!", strerror(errno));
				return -1;
			}

			#if (0)//def VPU_HW_DECODE //1. check whether this file is right or not.	
			if(vjpeg_decode_operation(NULL, NULL, 0, mCamDevice.preview_width, mCamDevice.preview_height, NULL, NULL, NULL, NULL, NULL, NULL, 0, VDEC_CLOSE) < 0)
				ALOGE( "JPEGD Operation Failed (VDEC_CLOSE)!! ");
			#endif

            camif_uninit_buffer();
		}		
	}
	else {
	#if(0) //20111214 ysseung   test...
    	for(buf.index=0; buf.index < mCaptureBufferCount; buf.index++) {
    	    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	    buf.memory      = V4L2_MEMORY_MMAP;
    	    if((ret = ioctl (mCamDevice.fd, VIDIOC_QBUF, &buf)) < 0) {
    			ALOGE("Err[%s] :: VIDIOC_QBUF faild!!", strerror(errno));
    			return ret;
    	    }
    	}
	#endif

		if((ret = ioctl (mCamDevice.fd, VIDIOC_USER_JPEG_CAPTURE, &jpeg_qval)) < 0) {
		    ALOGE("Err[%s] :: VIDIOC_USER_JPEG_CAPTURE faild!!", strerror(errno));
		    return ret;
		}

		capture_event.fd = mCamDevice.fd;
		capture_event.events = POLLIN|POLLERR; // Zzau_Temp: POLLERR을 overflow용으로 임시사용.
		if((ret = poll((struct pollfd*)&capture_event, 1, 2000)) < 0) {
			ALOGE("Capture POLL Error!!");
			return -1;
		}

		if(ret == 0 && retry_cnt < (CAP_RETRY_COUNT+1)) {
			if(retry_cnt == CAP_RETRY_COUNT) {
				ALOGE("CAP-FAIL:: Retry-Cap %d because of No Interrupt ->", retry_cnt);
				return -1;
			}
			retry_cnt++;		
			goto retry_capture;
		} else if(capture_event.revents & POLLERR) {
			jpeg_qval++;
			ALOGE("Retry-Cap %d, %d because of Overflow ->", retry_cnt, jpeg_qval);
			if(jpeg_qval <= 8) 		goto retry_capture;
			else 					return -1;
		}
		DBUG("YUV Capture Done!!");

		memset(&(mCamDevice.cap_info.jpeg_save_info), 0x00, sizeof(TCCXXX_JPEG_ENC_DATA));
		if ((ret = ioctl (mCamDevice.fd, VIDIOC_USER_GET_CAPTURE_INFO, &(mCamDevice.cap_info.jpeg_save_info))) < 0) {
		    ALOGE("Err[%s] :: VIDIOC_USER_GET_CAPTURE_INFO faild!!", strerror(errno));
		    return ret;
		}

        // for Rotate Capture.
		#if defined(ROTATE_CAPTURE_SUPPORT) //20120616 ysseung   disable to rotate-capture.
		if(nRotationOfCapture >= 0) {  // for exception processing.
			if(bIsitPortraitLCD) {
				// for portrait LCD
				if(nRotationOfCapture+90 == 360) 	nRotationOfCapture = 0;
				else 								nRotationOfCapture += 90;
			} else {
				// for Landscape LCD
				if(nRotationOfCapture == 90) 		nRotationOfCapture = 270;
				else if(nRotationOfCapture == 270) 	nRotationOfCapture = 90;
			}

			ALOGD("Rotation[%d degree] Capture", nRotationOfCapture);
			if(nRotationOfCapture) {
				struct pollfd poll_event[1];
				unsigned int nWidth, nHeight, nSrcAddr, nDstAddr;
				G2D_COMMON_TYPE grp_arg;
				FILE *out;

				mbRotateCapture = true;
				memset(&grp_arg, NULL, sizeof(G2D_COMMON_TYPE));

				nWidth   = mCamDevice.cap_info.width;
				nHeight  = mCamDevice.cap_info.height;
				nSrcAddr = (uint32_t)PA_JPEG_RAW_BASE_ADDR;
				if(nWidth < 2048) {
					nDstAddr = (uint32_t)PA_PREVIEW_BASE_ADDR + PMEM_CAM_SIZE - (2048 * 1536 * 2); // for physical address bit align.
				} else {
					nDstAddr = (uint32_t)PA_PREVIEW_BASE_ADDR + PMEM_CAM_SIZE - (nWidth * nHeight * 2);
				}
				ALOGD("rotate capture(%d x %d).  src_addr = 0x%x , dst_addr = 0x%x", nWidth ,nHeight, nSrcAddr, nDstAddr);

				grp_arg.responsetype 	= G2D_INTERRUPT; //G2D_INTERRUPT, G2D_POLLING;
				grp_arg.srcfm.format 	= GE_YUV422_sp;
				grp_arg.srcfm.data_swap = 0;
				grp_arg.src_imgx 		= nWidth;
				grp_arg.src_imgy 		= nHeight;
				grp_arg.DefaultBuffer 	= 0;
				grp_arg.tgtfm.format 	= GE_YUV422_sp;
				grp_arg.tgtfm.data_swap = 0;
				//grp_arg.tgtfm.uv_order 	= 0;

				if(nRotationOfCapture == 90) 		grp_arg.ch_mode = 0x5;  // 0x5 = ROTATE_90, 0x7 = ROTATE_270
				else if(nRotationOfCapture == 180) 	grp_arg.ch_mode = 0x6;
				else if(nRotationOfCapture == 270) 	grp_arg.ch_mode = 0x7;

				grp_arg.crop_offx 		= 0;
				grp_arg.crop_offy 		= 0;
				grp_arg.crop_imgx 		= nWidth;
				grp_arg.crop_imgy 		= nHeight;

				if(nRotationOfCapture == 180) {
					grp_arg.dst_imgx 	= nWidth;
					grp_arg.dst_imgy 	= nHeight;
				} else {
					grp_arg.dst_imgx 	= nHeight;
					grp_arg.dst_imgy 	= nWidth;
				}

				grp_arg.dst_off_x 		= 0;
				grp_arg.dst_off_y 		= 0;

				grp_arg.src0 			= nSrcAddr;
				grp_arg.src1 			= GET_ADDR_YUV42X_spU(grp_arg.src0, nWidth, nHeight);
				grp_arg.src2 			= GET_ADDR_YUV422_spV(grp_arg.src1, nWidth, nHeight);
				grp_arg.tgt0 			= nDstAddr;
				grp_arg.tgt1 			= GET_ADDR_YUV42X_spU(grp_arg.tgt0, nHeight, nWidth);
				grp_arg.tgt2 			= GET_ADDR_YUV422_spV(grp_arg.tgt1, nHeight, nWidth);
            	
				if(mCamDevice.g2d_fd <= 0) {
					mCamDevice.g2d_fd = open(GRAPHIC_DEVICE, O_RDWR);
					if(mCamDevice.g2d_fd <= 0) ALOGE("can't open[%s] '%s'", strerror(errno), GRAPHIC_DEVICE);
				}

				if(ioctl(mCamDevice.g2d_fd, TCC_GRP_COMMON_IOCTRL, &grp_arg) != 0) {
					ALOGE("G2D Out Error!");
				}

				if(grp_arg.responsetype == G2D_INTERRUPT) {
					int ret;
					memset(poll_event, 0, sizeof(poll_event));
					poll_event[0].fd = mCamDevice.g2d_fd;
					poll_event[0].events = POLLIN;

					ret = poll((struct pollfd*)poll_event, 1, 400);
					if(ret < 0) {
						ALOGE("g2d poll error\n");
						return -1;
					} else if(ret == 0) {
						ALOGE("g2d poll timeout\n");
						return -1;
					} else if(ret > 0) {
						if(poll_event[0].revents & POLLERR) {
							ALOGE("g2d poll POLLERR\n");
							return -1;
						}
					}
				}
				ALOGD("g2d out!!\n");

				//memcpy((void *)(mCamDevice.cap_info.jpeg_save_info.target_addr), (void *)grp_arg.tgt0, nWidth*nHeight*2);
				mCamDevice.cap_info.jpeg_save_info.target_addr = grp_arg.tgt0;

				if(nRotationOfCapture == 180) {
					mCamDevice.cap_info.jpeg_save_info.width = nWidth;
					mCamDevice.cap_info.jpeg_save_info.height = nHeight;
				} else {
					mCamDevice.cap_info.jpeg_save_info.width = nHeight;
					mCamDevice.cap_info.jpeg_save_info.height = nWidth;
				}
			}
		}
		#endif
	}

	FUNCTION_OUT;
	return ret;
}

#ifdef VPU_HW_DECODE
int TCC_V4l2_Camera::vjpeg_decode_operation(unsigned int src_phyAddr, unsigned int src_virtAddr, unsigned int src_length, 
													unsigned int src_width, unsigned int src_height,
													unsigned int *dec_width, unsigned int *dec_height,
													unsigned int *dec_padWidth, unsigned int *dec_padHeight, 
													unsigned int *dec_format, unsigned char *dest_buff_info, 
													int ratio, int iOpCode)
{
	int ret = 0;

	switch(iOpCode) {
		case VDEC_INIT:
		{
			gsVDecInit.m_iBitstreamFormat		= STD_MJPG;
			gsVDecInit.m_bFilePlayEnable		= 1;
			gsVDecInit.m_iPicWidth				= src_width;
			gsVDecInit.m_iPicHeight 			= src_height;
			gsVDecInit.m_bEnableUserData		= 0;
			
			gsVDecUserInfo.bitrate_mbps 		= 100;
			gsVDecUserInfo.m_bJpegOnly			= 1;
			gsVDecUserInfo.jpg_ScaleRatio		= ratio;
			gsVDecUserInfo.jpg_length			= src_length;
			
			VDBUG( "vjpeg_decode:: VDEC_INIT !!================> ratio = %d", ratio);
			if((ret = vdec_vpu( VDEC_INIT, NULL, &gsVDecInit, &gsVDecUserInfo )) < 0) {
				ALOGE( "[VDEC_INIT] [Err:%d] video decoder init", ret );
				if((ret = vdec_vpu( VDEC_CLOSE, NULL, NULL, &gsVDecOutput )) < 0) {
					ALOGE( "[VDEC_CLOSE] [Err:%d] video decoder close", ret );
				}
			} else {
				mVpuState = VPU_INITED;
			}
		}
		break;

		case VDEC_DEC_SEQ_HEADER:	
		case VDEC_DECODE:
		{
			gsVDecInput.m_pInp[PA] = (unsigned char*)src_phyAddr;
			gsVDecInput.m_pInp[VA] = (unsigned char*)src_virtAddr;
			gsVDecInput.m_iInpLen  = src_length;

			if(iOpCode == VDEC_DEC_SEQ_HEADER) {
				VDBUG( "vjpeg_decode:: VDEC_DEC_SEQ_HEADER !!================>");
				if((ret = vdec_vpu( VDEC_DEC_SEQ_HEADER, NULL, &gsVDecInput, &gsVDecOutput )) < 0) {
					ALOGE( "[VDEC_DEC_SEQ_HEADER] [Err:%d] video sequence_header", ret );
				} else {
					mVpuState = VPU_SEQHEADER_DONE;
				}
			} else {
				if(1) { // (frameSearchOrSkip_flag == 1)
					gsVDecInput.m_iSkipFrameNum = 1;
					//gsVDecInput.m_iFrameSearchEnable = 0x001; //I-frame (IDR-picture for H.264)
					gsVDecInput.m_iFrameSearchEnable = 0x201;	//I-frame (I-slice for H.264) : Non IDR-picture
					gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;
				}
				
				VDBUG( "vjpeg_decode:: VDEC_DECODE !!================>");
				if((ret = vdec_vpu( VDEC_DECODE, NULL, &gsVDecInput, &gsVDecOutput )) < 0) {
					ALOGE( "[VDEC_DECODE] [Err:%d] video decode", ret );
				} else {
					int i = 0;					
					for(i=0; i < 3; i++) {
						memcpy(dest_buff_info+i*4, &gsVDecOutput.m_pDispOut[PA][i], 4);
					}
				}
			}

			if(ret >= 0) {
				*dec_format = gsVDecOutput.m_pInitialInfo->m_iMjpg_sourceFormat; //0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 

				if(ratio == 0) {
					*dec_width 		= gsVDecOutput.m_pInitialInfo->m_iPicWidth;
					*dec_height 	= gsVDecOutput.m_pInitialInfo->m_iPicHeight;
					*dec_padWidth	= ((gsVDecOutput.m_pInitialInfo->m_iPicWidth + 15)>>4)<<4;
					*dec_padHeight = gsVDecOutput.m_pInitialInfo->m_iPicHeight;
				} else {
					*dec_width = *dec_padWidth	= gsVDecOutput.m_pInitialInfo->m_iPicWidth >> ratio;
					*dec_height = *dec_padHeight = gsVDecOutput.m_pInitialInfo->m_iPicHeight >> ratio;

					if(*dec_format == 2 || *dec_format == 3 || *dec_format == 4)
						*dec_padWidth = ((*dec_padWidth + 7)>>3)<<3;
					else
						*dec_padWidth = ((*dec_padWidth + 15)>>4)<<4;

					//*dec_padWidth = ((*dec_padWidth + 15)>>4)<<4;
				}		
				VDBUG( "DecOutput(%d x %d) =>(ratio %d) %d x %d => pad(%d x %d) !!", gsVDecOutput.m_pInitialInfo->m_iPicWidth, gsVDecOutput.m_pInitialInfo->m_iPicHeight, 
									ratio, *dec_width, *dec_height, *dec_padWidth, *dec_padHeight);		
				VDBUG( "source format = %d (0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0)!!", gsVDecOutput.m_pInitialInfo->m_iMjpg_sourceFormat);
			}
		}
		break;

		case VDEC_CLOSE:
		{
			if(mVpuState != VPU_CLOSED) {
				DBUG( "vjpeg_decode:: VDEC_CLOSE !!================>"); 
				if((ret = vdec_vpu( VDEC_CLOSE, NULL, NULL, &gsVDecOutput )) < 0) {
					ALOGE( "[VDEC_CLOSE] [Err:%d] video decoder close", ret );
				}
				mVpuState = VPU_CLOSED;
			} else {
				DBUG( "vjpeg_decode:: vpu already is closed !!================>"); 
			}
		}
		break;
	}

	return ret;
}
#endif

#if defined(USE_CIFOUT_GRALLOC)
void TCC_V4l2_Camera::get_currPreviewFrameIndex(int *frame_index, int *error, bool skip)
{
	uint32_t used_bytes;

	if(mCamDevice.cam_mode != MODE_PREVIEW) {
		ALOGE("ERR_GET_BUFFER :: not preview => cur : %d!!", mCamDevice.cam_mode);
		*error = ERR_GET_BUFFER;
	}

	*frame_index = mCamDevice.preview_idx = camif_get_frame(&used_bytes);
	if(mCamDevice.preview_idx > mPreviewBufferCount) {
		DBUG("ERR_GET_BUFFER :: %d > %d!!", mCamDevice.preview_idx, mPreviewBufferCount);
		*error = ERR_GET_BUFFER;
	}

	if(mUvc_camera) {
    	struct v4l2_buffer buf;
        
		if(skip == true) {
			DBUG("ERR_BUFFER_PROC :: skip(%d) - %d'th frame!!", skip, *frame_index); 
			*error = ERR_BUFFER_PROC;
		}
		else if((*frame_index = get_currUvcYuv(used_bytes, mCamDevice.preview_idx)) == ERR_BUFFER_PROC) {
            *error = ERR_BUFFER_PROC;
		}

    	memset(&buf, 0, sizeof(struct v4l2_buffer));
    	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	buf.memory = V4L2_MEMORY_MMAP;
    	buf.index = mCamDevice.preview_idx;
    	 
    	if(ioctl(mCamDevice.fd, VIDIOC_QBUF, &buf) < 0) {
    	    ALOGE(" VIDIOC_QBUF failed, maybe already released(%d)!", buf.index);
    	}
	}
}
int TCC_V4l2_Camera::camif_Intrrupt_check()
{
	int ret = 1;
	int value;

	if(!mUvc_camera) ret = ioctl(mCamDevice.fd, VIDIOC_USER_INT_CHECK, &value);
	return ret;
}
void TCC_V4l2_Camera::camif_setCameraAddress(int index, int cameraStatus)
{
	int ret;
	struct v4l2_requestbuffers reqBuffers;

	//FUNCTION_IN

	memset(&reqBuffers, 0, sizeof(struct v4l2_requestbuffers));
	reqBuffers.type 			= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqBuffers.memory 			= V4L2_MEMORY_MMAP;
	reqBuffers.count  			= index;
	if(mCamDevice.cam_mode == MODE_PREVIEW) {
		reqBuffers.reserved[0] 	= (unsigned int)mCameraBufferAddr[index];
	} else {
		reqBuffers.reserved[0] 	= (unsigned int)PA_JPEG_RAW_BASE_ADDR;
	}
	reqBuffers.reserved[1] 		= (unsigned int)cameraStatus;

    if(mUvc_camera) {
        buffer_manager[buffer_manager_in_index] = index;
        buffer_manager_in_index = (buffer_manager_in_index+1)%mPreviewBufferCount;
    }
    else if((ret = ioctl(mCamDevice.fd, VIDIOC_USER_SET_CAMERA_ADDR, &reqBuffers)) < 0) {
		ALOGE("Err[%s] :: VIDIOC_USER_SET_CAMERA_ADDR failed!", strerror(errno));
	}

	//FUNCTION_OUT
}
#endif

#ifdef USE_CIFOUT_PMEM
int TCC_V4l2_Camera::create_pmem()
{	
	DBUG(" create_pmem!!");

	mCameraMem = new TCC_Camera_Mem();
	
	if((mPreviewHeap = mCameraMem->allocSharedMem(PMEM_CAM_SIZE)) == NULL)
	{
		return -1;
	}
	
	return 0;
}

int TCC_V4l2_Camera::destopy_pmem()
{	 
	if(mCameraMem)
	{
		DBUG(" destopy_pmem!!");

		for (int buff_idx=0; buff_idx<BUFFER_MAX; buff_idx++)
		{
			for (int i=0; i<mCamDevice.Buffer_cnt; i++) 
			{
				if(pmem_buffers[buff_idx][i] != NULL)
					pmem_buffers[buff_idx][i].clear();
				
				pmem_buffers[buff_idx][i] = NULL;
				pmem_buffers_offset[buff_idx][i] = 0;
			}
		}
		
		mPreviewHeap.clear();		
		mPreviewHeap = NULL;
		
		delete mCameraMem;
		mCameraMem = NULL;
	}
	
	return 0;
}

int* TCC_V4l2_Camera::get_pmem_virtual_addr(int buffer_type, int bufIdx)
{
	return (int*)(((unsigned char *)mPreviewHeap->base()) + pmem_buffers_offset[buffer_type][bufIdx]);;
}

int* TCC_V4l2_Camera::get_pmem_physical_addr(int buffer_type, int bufIdx)
{
	return (int*)(PA_PREVIEW_BASE_ADDR + pmem_buffers_offset[buffer_type][bufIdx]);
}
#else
int* TCC_V4l2_Camera::get_pmem_virtual_addr(int buffer_type, int bufIdx)
{
	return (int*)(((unsigned char *)pmem_mmaped) + pmem_buffers_offset[buffer_type][bufIdx]);;
}

int* TCC_V4l2_Camera::get_pmem_physical_addr(int buffer_type, int bufIdx)
{
	return (int*)(mExtCameraPmap.base + pmem_buffers_offset[buffer_type][bufIdx]);
}
#endif
unsigned int TCC_V4l2_Camera::get_pmem_buffer_offset(int buffer_type, int bufIdx)
{
	return pmem_buffers_offset[buffer_type][bufIdx];
}


/*********************************************************************************************/
/*                                                     Public Function												 */
/*********************************************************************************************/

TCC_V4l2_Camera::TCC_V4l2_Camera(int nCamIndex)
{
	camif_open_device(nCamIndex);

	pmap_get_info("ump_reserved", 	&mUmpReservedPmap);
	pmap_get_info("jpeg_header", 	&mJPEGHeaderPmap);
	pmap_get_info("jpeg_raw", 		&mJPEGRawPmap);
	pmap_get_info("jpeg_stream", 	&mJPEGStreamPmap);
	pmap_get_info("ext_camera", 	&mExtCameraPmap);

	mBufferHandle = NULL;
	mGrallocHandle = NULL;

	mVideoSnapshot = mDoneVideoSnapshot = false;

	// allocate virtual jpeg address.
	mJpegRawAddr = mJpegHeaderAddr = mJpegStreamAddr = NULL;
	mJpegRawAddr 		= (unsigned char *)mmap(0, JPEG_RAW_MEM_SIZE, (PROT_READ|PROT_WRITE), MAP_SHARED, 	\
												mCamDevice.fd, PA_JPEG_RAW_BASE_ADDR);
	mJpegHeaderAddr 		= (unsigned char *)mmap(0, JPEG_HEADER_MEM_SIZE, (PROT_READ|PROT_WRITE), MAP_SHARED, 	\
												mCamDevice.fd, PA_JPEG_HEADER_BASE_ADDR);
	mJpegStreamAddr 		= (unsigned char *)mmap(0, JPEG_STREAM_MEM_SIZE, (PROT_READ|PROT_WRITE), MAP_SHARED, 	\
												mCamDevice.fd, PA_JPEG_STREAM_BASE_ADDR);
}

TCC_V4l2_Camera::~TCC_V4l2_Camera()
{
	camif_close_device();

	#if defined(USE_CIFOUT_GRALLOC)
		if(mBufferHandle != NULL || mGrallocHandle != NULL) {
			delete mBufferHandle;
			delete mGrallocHandle;
			mBufferHandle = NULL;
			mGrallocHandle = NULL;
		}
	#endif	

	// de-allocate virtual jpeg address.
	munmap((void *)mJpegRawAddr, JPEG_RAW_MEM_SIZE);
	munmap((void *)mJpegHeaderAddr, JPEG_HEADER_MEM_SIZE);
	munmap((void *)mJpegStreamAddr, JPEG_STREAM_MEM_SIZE);
	mJpegRawAddr = mJpegHeaderAddr = mJpegStreamAddr = NULL;
}

/* Preview Function */
int TCC_V4l2_Camera::startPreview(const char* format, int *fps)
{
	int ret = 0;
    int ret_fps = 0;
	FUNCTION_IN;

	if(strcmp(format, "rgb565") == 0) 			mCamDevice.preview_fmt = MODE_RGB565;
	else if(strcmp(format, "yuv420sp") == 0) 	mCamDevice.preview_fmt = MODE_YUV420;
	else if(strcmp(format, "rgba8888") == 0) 	mCamDevice.preview_fmt = MODE_RGBA8888;
	else 										mCamDevice.preview_fmt = MODE_YUV422;
	

	if((ret = camif_set_resolution(mCamDevice.preview_width, mCamDevice.preview_height)) < 0) {
		ALOGE("Err:: err = %d  camif_set_resolution!", ret);
		return -1;
	}

	ret = camif_start_stream(&ret_fps);
    *fps = ret_fps;

		
	FUNCTION_OUT;
	return ret;
}

int TCC_V4l2_Camera::stopPreview()
{
	int ret = 0;

	FUNCTION_IN

	ret = camif_stop_stream();
	
	FUNCTION_OUT
		
	return ret;
}

/* Get Capture Frame (JPEG)*/
int TCC_V4l2_Camera::capture(uint32_t *size, uint16_t width, uint16_t height, int nRotationOfCapture, bool bIsitPortraitLCD)
{
	FUNCTION_IN
	int ret = 0;
	
	mCamDevice.cam_mode = MODE_CAPTURE;

	if(mUvc_camera) {
		if(width > mCamDevice.mUvc_MaxWidth || height > mCamDevice.mUvc_MaxHeight) {
			width 	= mCamDevice.mUvc_MaxWidth;
			height = mCamDevice.mUvc_MaxHeight;
		}
	}

	mCamDevice.cap_info.width  = width;
	mCamDevice.cap_info.height = height;

	if((ret = camif_set_resolution (mCamDevice.cap_info.width, mCamDevice.cap_info.height)) < 0) {
		return ret;
	}

	// set to capture address
	camif_setCameraAddress(0, MODE_CAPTURE);

	*size = 0;
	if((ret = camif_capture(nRotationOfCapture, bIsitPortraitLCD)) < 0) 	return ret;

	*size = mCamDevice.cap_info.jpeg_save_info.bitstream_size + mCamDevice.cap_info.jpeg_save_info.header_size;
	if(*size == 0 || mCamDevice.cap_info.jpeg_save_info.header_size == 0) {
		*size = camif_encode_jpeg();
	}

	FUNCTION_OUT;
	return ret;
}

int TCC_V4l2_Camera::release_buffer(unsigned char rel_index)
{
	struct v4l2_buffer buf;

    if(mUvc_camera)
        return 0;

	//FUNCTION_IN
	if(rel_index < 0 && rel_index >= (mRealKCount - 2)){
		ALOGE("Release index Error!! rel_index[%d]", rel_index);
		return 0;
	}

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = rel_index;
	 
	if(ioctl(mCamDevice.fd, VIDIOC_QBUF, &buf) < 0) {
	    ALOGE(" VIDIOC_QBUF failed, maybe already released(%d)!", buf.index);
	    return -1;
	}
	
	//DBUG("%s:  release buffer index=%d", __FUNCTION__, rel_index);
	//FUNCTION_OUT

	return 0;
}

int TCC_V4l2_Camera::get_bufferCount()
{
	return mCamDevice.Buffer_cnt;
}

int TCC_V4l2_Camera::get_currUvcJpeg(uint32_t frame_len, int buffer_index)
{
	unsigned char *mapped_SrcBuff = NULL, *mapped_DstBuff = NULL;
	TCCXXX_JPEG_DEC_DATA dec_data;
	unsigned int phy_SrcBuff;
	int jpeg_len = 0;
	int ret = 0;
#ifdef VPU_HW_DECODE
	unsigned int dest_buff_info[4];
#endif

	phy_SrcBuff = (unsigned int)get_pmem_physical_addr(BUFFER_PREVIEW, buffer_index);

	memset(&mCamDevice.cap_info.jpeg_save_info, 0x00, sizeof(TCCXXX_JPEG_ENC_DATA));

	if(mCapture_fmt == V4L2_PIX_FMT_MJPEG)//Mjpeg format
	{		
		unsigned char *temp_frame;
		int i = 0, isHuffman = 0;

		if(MAP_FAILED == (mapped_SrcBuff = (unsigned char*)mmap(0, frame_len, PROT_READ | PROT_WRITE, MAP_SHARED, mCamDevice.jpegenc_fd, phy_SrcBuff)))
		{
			ALOGE( "JPEGE mmap(mapped_SrcBuff) Operation Failed(%s)!! ", strerror(errno));
			return -1;
		}

		if(MAP_FAILED == (mapped_DstBuff = (unsigned char*)mmap(0, JPEG_STREAM_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mCamDevice.jpegenc_fd, PA_JPEG_STREAM_BASE_ADDR)))
		{
			ALOGE( "JPEGE mmap(mapped_DstBuff) Operation Failed(%s)!! ", strerror(errno));
			
			munmap(mapped_SrcBuff, frame_len);
			return -1;
		}

		temp_frame = mapped_SrcBuff;
		if(!(temp_frame[frame_len-2] == 0xFF && temp_frame[frame_len-1] == 0xD9))
		{
			ALOGE("abnormal frame.");
			ret = -1;
			goto Finish;
		}

#if 0 //1. check whether this file is right or not.		
		memset(&dec_data, 0x00, sizeof(TCCXXX_JPEG_DEC_DATA));
#ifdef VPU_HW_DECODE
		if(mVpuState == VPU_CLOSED)
		{
			ALOGE( "VPU can't use. VpuState is %d!! ", mVpuState);
			ret = -1;
			goto Finish;
		}
		
		if(mVpuState == VPU_INITED)
		{
			if(vjpeg_decode_operation(phy_SrcBuff, (unsigned int)mapped_SrcBuff, frame_len, 
							mCamDevice.preview_width, mCamDevice.preview_height,
							&(dec_data.width), &(dec_data.height),
							&(dec_data.pad_width), &(dec_data.pad_height), 
							&(dec_data.image_format), (unsigned char *)dest_buff_info,
							0, VDEC_DEC_SEQ_HEADER) < 0)
			{
				ALOGE( "JPEGD Operation Failed(vpu init)!! ");
				ret = -1;
				goto Finish;
			}
		}

		if(vjpeg_decode_operation((unsigned int)vpu_getBitstreamBufAddr(PA), (unsigned int)vpu_getBitstreamBufAddr(VA), frame_len, 
						mCamDevice.preview_width, mCamDevice.preview_height,
						&(dec_data.width), &(dec_data.height),
						&(dec_data.pad_width), &(dec_data.pad_height),
						&(dec_data.image_format), (unsigned char *)dest_buff_info,
						0, VDEC_DECODE) < 0)
		{
			ALOGE( "JPEGD Operation Failed(vpu decode)!! ");
			ret = -1;
			goto Finish;
		}
#else
		
		memcpy(mapped_DstBuff, mapped_SrcBuff, frame_len);
		dec_data.source_addr	 = (unsigned int)PA_JPEG_STREAM_BASE_ADDR;
		dec_data.file_length	 = frame_len;
		dec_data.target_addr	 = PA_CONVERT_TEMP_ADDR;
		dec_data.target_size	 = PA_CONVERT_TEMP_SIZE;
		dec_data.ratio 			 = NO_SCALE;

		if(ioctl(mCamDevice.jpegdec_fd,TCC_JPEGD_IOCTL_DEC, &dec_data) < 0)
		{
			ALOGE( "JPEGD Operation Failed(%s)!! ", strerror(errno));
			ret = -1;
			goto Finish;
		}
#endif
		DBUG("MJPEG got it!!(%d).", frame_len);
#endif

//2. check whether jpeg has huffman table or not.		
		while(((temp_frame[0] << 8) | temp_frame[1]) != 0xffda) {
			i++;
			if((uint32_t)i >= frame_len || i >= 64*1024) {
				isHuffman = 0;
				break;
			}
			if(((temp_frame[0] << 8) | temp_frame[1]) == 0xffc4) {
				isHuffman = 1;
				break;
			}
			temp_frame++;
		}
		ALOGI("isHuffman(%d) - 0x%x, 0x%x ~ 0x%x, 0x%x.", isHuffman, mapped_SrcBuff[0], mapped_SrcBuff[1], mapped_SrcBuff[frame_len-2], mapped_SrcBuff[frame_len-1]);

		if(isHuffman)
		{
			memcpy(mapped_DstBuff, mapped_SrcBuff, frame_len);
			jpeg_len = frame_len;
		}
		else
		{
			unsigned char *pEnd, *pCur;
			int iLen = 0, offset = 0;
			
			pCur = mapped_SrcBuff;
			pEnd = mapped_SrcBuff + frame_len;
			while ((((pCur[0] << 8) | pCur[1]) != 0xffc0) && (pCur < pEnd))
			  pCur++;
			if (pCur >= pEnd)
			{
				ALOGE("Fail to search Huffman position.");
				ret = -1;
				goto Finish;
			}
			iLen = pCur - mapped_SrcBuff;
			
			memcpy(mapped_DstBuff+offset, mapped_SrcBuff, iLen); 
			offset += iLen;
			memcpy(mapped_DstBuff+offset, huffman_table, sizeof(huffman_table)); 
			offset += sizeof(huffman_table);
			memcpy(mapped_DstBuff+offset, pCur, frame_len - iLen); 
			offset += (frame_len-iLen);

			jpeg_len = offset;
		}

		mCamDevice.cap_info.jpeg_save_info.target_addr = PA_JPEG_STREAM_BASE_ADDR;
		mCamDevice.cap_info.jpeg_save_info.target_size = jpeg_len;
		mCamDevice.cap_info.jpeg_save_info.header_offset = 0;
		mCamDevice.cap_info.jpeg_save_info.header_size = 256;
		mCamDevice.cap_info.jpeg_save_info.bitstream_offset = mCamDevice.cap_info.jpeg_save_info.header_size;
		mCamDevice.cap_info.jpeg_save_info.bitstream_size = jpeg_len - mCamDevice.cap_info.jpeg_save_info.header_size;

    	DBUG("JPEG :: Stream[0x%x - 0x%x], Header[0x%x - 0x%x]!!", (mCamDevice.cap_info.jpeg_save_info.target_addr + mCamDevice.cap_info.jpeg_save_info.bitstream_offset),
								mCamDevice.cap_info.jpeg_save_info.bitstream_size, 
								(mCamDevice.cap_info.jpeg_save_info.target_addr + mCamDevice.cap_info.jpeg_save_info.header_offset),
								mCamDevice.cap_info.jpeg_save_info.header_size);

        
	}
	else //YUV format
	{
		//encode		
		mCamDevice.cap_info.jpeg_save_info.source_addr	= get_currUvcYuv(frame_len, buffer_index);
		mCamDevice.cap_info.jpeg_save_info.width  		= mCamDevice.cap_info.width;
		mCamDevice.cap_info.jpeg_save_info.height 		= mCamDevice.cap_info.height;
		mCamDevice.cap_info.jpeg_save_info.q_value 		= JPEG_SUPERFINE;
		mCamDevice.cap_info.jpeg_save_info.src_format 	= ENC_INPUT_420;
		mCamDevice.cap_info.jpeg_save_info.target_addr	= PA_JPEG_STREAM_BASE_ADDR;
		mCamDevice.cap_info.jpeg_save_info.target_size 	= JPEG_STREAM_MEM_SIZE;

#if defined(_TCC8800_)		
		if(ioctl(mCamDevice.jpegenc_fd,TCC_JPEGE_IOCTL_ENC, &(mCamDevice.cap_info.jpeg_save_info)) < 0)
		{
			ALOGE("Err[%s] :: TCC_JPEG_IOCTL_V4L2_ENC failed!", strerror(errno));
			return -1;
		}
#else
    	DBUG("YUV :: Raw[0x%x ~]!!, Mode(%d)", mCamDevice.cap_info.jpeg_save_info.source_addr, mCamDevice.cam_mode);
#endif
    	DBUG("JPEG :: Stream[0x%x - 0x%x], Header[0x%x - 0x%x]!!", (mCamDevice.cap_info.jpeg_save_info.target_addr + mCamDevice.cap_info.jpeg_save_info.bitstream_offset),
								mCamDevice.cap_info.jpeg_save_info.bitstream_size, 
								(mCamDevice.cap_info.jpeg_save_info.target_addr + mCamDevice.cap_info.jpeg_save_info.header_offset),
								mCamDevice.cap_info.jpeg_save_info.header_size);
	}	

Finish:
	if(mCapture_fmt == V4L2_PIX_FMT_MJPEG){
		munmap(mapped_SrcBuff, frame_len);
		munmap(mapped_DstBuff, JPEG_STREAM_MEM_SIZE);			
	}
	
	return ret;
}

#ifdef USE_CIFOUT_PMEM
int TCC_V4l2_Camera::get_bufferIndex(unsigned int phy_addr)
{
	int index;

	for(index = 0; index <mCamDevice.Buffer_cnt; index ++){
		if(mUvc_camera)
		{
			if((mPreview_fmt == V4L2_PIX_FMT_MJPEG) || ((mPreview_fmt != V4L2_PIX_FMT_MJPEG) && mCamDevice.needYuv420Cvt))
			{
				if(phy_addr == (unsigned int)get_pmem_physical_addr(BUFFER_UVC, index))
					break;
			}
		}
		else
		{
			if(phy_addr == (unsigned int)get_pmem_physical_addr(BUFFER_PREVIEW, index))
				break;
		}
    }

	if (index == mCamDevice.Buffer_cnt)
	{
		ALOGE("Wrong buffer address");
		return -1;
	}

	return index;
}

unsigned int TCC_V4l2_Camera::get_currUvcYuv(uint32_t frame_len, unsigned int buffer_index)
{
	int old_needYuv420Cvt = mCamDevice.needYuv420Cvt;
	TCCXXX_JPEG_DEC_DATA dec_data;
	int scaler_fd;
#ifdef VPU_HW_DECODE
	unsigned int dest_buff_info[4];
#endif
	unsigned int src_virt_addr = (unsigned int)pmem_mmaped + get_pmem_buffer_offset(BUFFER_PREVIEW, mCamDevice.preview_idx);

	if(mPreview_fmt == V4L2_PIX_FMT_MJPEG)
	{	
		unsigned char *temp_frame = (unsigned char *)src_virt_addr;
		
		if(!(temp_frame[frame_len-2] == 0xFF && temp_frame[frame_len-1] == 0xD9))
			return ERR_BUFFER_PROC;
	
		memset(&dec_data, 0x00, sizeof(TCCXXX_JPEG_DEC_DATA));

		dec_data.source_addr	 = (unsigned int)get_pmem_physical_addr(BUFFER_PREVIEW, mCamDevice.preview_idx);
		dec_data.file_length	 = frame_len;
		if(old_needYuv420Cvt == 0)
		{
			dec_data.target_addr	 = (unsigned int)get_pmem_physical_addr(BUFFER_UVC, mCamDevice.preview_idx);
			dec_data.target_size	 = mCamDevice.buffers_lencvt;
		}
		else
		{
			dec_data.target_addr	 = PA_CONVERT_TEMP_ADDR;
			dec_data.target_size	 = PA_CONVERT_TEMP_SIZE;
		}
		dec_data.ratio 			 = NO_SCALE;

#ifdef VPU_HW_DECODE
		if(mVpuState == VPU_CLOSED)
		{
			ALOGE( "VPU can't use. VpuState is %d!! ", mVpuState);
			return ERR_BUFFER_PROC;
		}
		
		if(mVpuState == VPU_INITED)
		{
			if(vjpeg_decode_operation(dec_data.source_addr, src_virt_addr, frame_len, 
							mCamDevice.preview_width, mCamDevice.preview_height,
							&(dec_data.width), &(dec_data.height),
							&(dec_data.pad_width), &(dec_data.pad_height), 
							&(dec_data.image_format), (unsigned char *)dest_buff_info, 
							0, VDEC_DEC_SEQ_HEADER) < 0)
			{
				ALOGE( "JPEGD Operation Failed(vpu init)!! ");
				return ERR_BUFFER_PROC;
			}
		}

		if(vjpeg_decode_operation(dec_data.source_addr, src_virt_addr, frame_len, 
						mCamDevice.preview_width, mCamDevice.preview_height,
						&(dec_data.width), &(dec_data.height),
						&(dec_data.pad_width), &(dec_data.pad_height), 
						&(dec_data.image_format), (unsigned char *)dest_buff_info, 
						0, VDEC_DECODE) < 0)
		{
			ALOGE( "JPEGD Operation Failed(vpu decode)!! ");
			return ERR_BUFFER_PROC;
		}

		mCamDevice.needYuv420Cvt = 1;	
		
		if(dec_data.image_format == 0 || dec_data.image_format == 4)
			dec_data.image_format = IMAGE_CHROMA_420;
		//else if(dec_data.image_format == 2)
		//	dec_data.image_format = IMAGE_CHROMA_440;		
		//else if(dec_data.image_format == 3)
		//	dec_data.image_format = IMAGE_CHROMA_444;
		else
			dec_data.image_format = IMAGE_CHROMA_422;		
#else
		if(ioctl(mCamDevice.jpegdec_fd,TCC_JPEGD_IOCTL_DEC, &dec_data) < 0)
		{
			ALOGE( "JPEGD Operation Failed(%s)!! ", strerror(errno));
			return ERR_BUFFER_PROC;
		}	

		if(dec_data.image_format == IMAGE_CHROMA_420)
			mCamDevice.needYuv420Cvt = 0;
		else
			mCamDevice.needYuv420Cvt = 1;	
#endif
	}
	else
	{
		if(mPreview_fmt == V4L2_PIX_FMT_YUYV)
		{
			mCamDevice.needYuv420Cvt = 1;
		}
		else
		{
			mCamDevice.needYuv420Cvt = 0;
			return (unsigned int)get_pmem_physical_addr(BUFFER_PREVIEW, mCamDevice.preview_idx);
		}
	}

	if(old_needYuv420Cvt != 0)
	{
#ifdef USE_G2D_FOR_UVC
		G2D_COMMON_TYPE grp_arg;
		memset(&grp_arg, 0x00, sizeof(G2D_COMMON_TYPE));
		
		grp_arg.responsetype 	= G2D_INTERRUPT; //G2D_POLLING;	
		grp_arg.src_imgx 		= dec_data.pad_width;
		grp_arg.src_imgy 		= dec_data.pad_height;
		grp_arg.srcfm.format	= GE_YUV420_sp;
#ifdef VPU_HW_DECODE
		VDBUG( "vjpeg_decode src(0x%x - 0x%x - 0x%x) Success!! ", dest_buff_info[0], dest_buff_info[1], dest_buff_info[2]);
	
		grp_arg.src0			= dest_buff_info[0]; 		
		grp_arg.src1			= dest_buff_info[1]; 
		grp_arg.src2			= dest_buff_info[2];
		if(dec_data.image_format == IMAGE_CHROMA_420)
			grp_arg.srcfm.format = GE_YUV420_sp; 
		else
			grp_arg.srcfm.format = GE_YUV422_sp; 
#else
		grp_arg.src0			= PA_CONVERT_TEMP_ADDR;			
		grp_arg.src1			= GET_ADDR_YUV42X_spU(grp_arg.src0 , grp_arg.src_imgx, grp_arg.src_imgy );
		grp_arg.src2			= GET_ADDR_YUV422_spV(grp_arg.src1 , grp_arg.src_imgx, grp_arg.src_imgy );	
		grp_arg.srcfm.format	= GE_YUV422_sp; 	
#endif

		grp_arg.srcfm.data_swap	= 0;
		grp_arg.DefaultBuffer 	= 0;

		grp_arg.tgtfm.format 	= GE_YUV420_sp; 	
		grp_arg.tgtfm.data_swap	= 0;
        grp_arg.tgtfm.uv_order =  1;
		grp_arg.ch_mode 	= NOOP;

		grp_arg.crop_offx = 0;
		grp_arg.crop_offy = 0;
		grp_arg.crop_imgx	= mCamDevice.preview_width;
		grp_arg.crop_imgy	= mCamDevice.preview_height;	

		grp_arg.dst_imgx = mCamDevice.preview_width;
		grp_arg.dst_imgy = mCamDevice.preview_height;

		grp_arg.dst_off_x = 0;
		grp_arg.dst_off_y = 0;
		grp_arg.tgt0		= (unsigned int)get_pmem_physical_addr(BUFFER_UVC, mCamDevice.preview_idx);
		grp_arg.tgt1		= GET_ADDR_YUV42X_spU(grp_arg.tgt0 , grp_arg.dst_imgx, grp_arg.dst_imgy );
		grp_arg.tgt2		= GET_ADDR_YUV420_spV(grp_arg.tgt1 , grp_arg.dst_imgx, grp_arg.dst_imgy );
		
		if(ioctl(mCamDevice.g2d_fd, TCC_GRP_COMMON_IOCTRL, &grp_arg) != 0)
		{
			ALOGE( "G2D Operation Failed!! ");
			return ERR_BUFFER_PROC;
		}

		if(grp_arg.responsetype == G2D_INTERRUPT)
		{
			int ret;
			struct pollfd poll_event[1];
			
			memset(poll_event, 0, sizeof(poll_event));
			poll_event[0].fd = mCamDevice.g2d_fd;
			poll_event[0].events = POLLIN;
			ret = poll((struct pollfd*)poll_event, 1, 100);

			if (ret < 0) {
				ALOGE("g2d poll error\n");
				return ERR_BUFFER_PROC;
			}else if (ret == 0) {
				ALOGE("g2d poll timeout %d x %d (%d x %d) -> %d x %d", grp_arg.src_imgx, grp_arg.src_imgy, grp_arg.crop_offx, grp_arg.crop_offx, grp_arg.crop_imgx, grp_arg.crop_imgy);
				ALOGE("g2d poll timeout 0x%x - 0x%x - 0x%x => 0x%x - 0x%x - 0x%x", grp_arg.src0, grp_arg.src1, grp_arg.src2, grp_arg.tgt0, grp_arg.tgt2, grp_arg.tgt2);
				return ERR_BUFFER_PROC;
			}else if (ret > 0) {
				if (poll_event[0].revents & POLLERR) {
					ALOGE("g2d poll POLLERR\n");
					return ERR_BUFFER_PROC;
				}
			}
		}
#else
		SCALER_TYPE msc_data;

		memset(&msc_data, 0x00, sizeof(SCALER_TYPE));

		msc_data.responsetype 	= SCALER_INTERRUPT; 

		if(mPreview_fmt == V4L2_PIX_FMT_MJPEG){
				msc_data.src_ImgWidth 	= dec_data.pad_width;	
				msc_data.src_ImgHeight	= dec_data.pad_height;	
			#ifdef VPU_HW_DECODE
				VDBUG( "vjpeg_decode src(0x%x - 0x%x - 0x%x) Success!! ", dest_buff_info[0], dest_buff_info[1], dest_buff_info[2]);

				msc_data.src_Yaddr		= (char*)dest_buff_info[0];			
				msc_data.src_Uaddr		= (char*)dest_buff_info[1];	
				msc_data.src_Vaddr		= (char*)dest_buff_info[2];
				
				if(dec_data.image_format == IMAGE_CHROMA_420)
					msc_data.src_fmt		= SCALER_YUV420_sp; 
				else
					msc_data.src_fmt		= SCALER_YUV422_sp; 
			#else
				msc_data.src_Yaddr		= (char*)PA_CONVERT_TEMP_ADDR;			
				msc_data.src_Uaddr		= (char*)GET_ADDR_YUV42X_spU(msc_data.src_Yaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight );
				msc_data.src_Vaddr      = (char*)GET_ADDR_YUV422_spV(msc_data.src_Uaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight );	
				msc_data.src_fmt        = SCALER_YUV422_sp;		
			#endif

			msc_data.src_winRight 	= dec_data.width;
			msc_data.src_winBottom	= dec_data.height;
			msc_data.src_winLeft	= 0;
			msc_data.src_winTop		= 0;
		}
		else if(mPreview_fmt == V4L2_PIX_FMT_YUYV){
			msc_data.src_ImgWidth 	= mCamDevice.vid_fmt.fmt.pix.width; 	
			msc_data.src_ImgHeight	= mCamDevice.vid_fmt.fmt.pix.height; 

			#ifdef VPU_HW_DECODE
				msc_data.src_Yaddr		= (char*)get_pmem_physical_addr(BUFFER_PREVIEW, mCamDevice.preview_idx);			
				msc_data.src_fmt		= SCALER_YUV422_sq0; 			
			#else
				msc_data.src_Yaddr		= (char*)PA_CONVERT_TEMP_ADDR;			
				msc_data.src_Uaddr		= (char*)GET_ADDR_YUV42X_spU(msc_data.src_Yaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight );
				msc_data.src_Vaddr      = (char*)GET_ADDR_YUV422_spV(msc_data.src_Uaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight );	
				msc_data.src_fmt        = SCALER_YUV422_sp;		
			#endif

			msc_data.src_winRight 	= msc_data.src_ImgWidth;
			msc_data.src_winBottom	= msc_data.src_ImgHeight;
			msc_data.src_winLeft	= 0;
			msc_data.src_winTop		= 0;
		}			

		msc_data.dest_ImgWidth	= mCamDevice.preview_width;
		msc_data.dest_ImgHeight	= mCamDevice.preview_height;
		msc_data.dest_Yaddr		= (char*)get_pmem_physical_addr(BUFFER_UVC, mCamDevice.preview_idx);
		msc_data.dest_Uaddr		= (char*)GET_ADDR_YUV42X_spU(msc_data.dest_Yaddr , msc_data.dest_ImgWidth, msc_data.dest_ImgHeight );
		msc_data.dest_Vaddr		= (char*)GET_ADDR_YUV420_spV(msc_data.dest_Uaddr , msc_data.dest_ImgWidth, msc_data.dest_ImgHeight );
		msc_data.dest_fmt 		= SCALER_YUV420_sp; 	
		msc_data.dest_winLeft 	= 0;
		msc_data.dest_winTop	= 0;
		msc_data.dest_winRight	= msc_data.dest_ImgWidth;
		msc_data.dest_winBottom	= msc_data.dest_ImgHeight;
//		ALOGD("@@@ Scaler-SRC:: %d x %d, %d,%d ~ %d x %d  => %d x %d", msc_data.src_ImgWidth, msc_data.src_ImgHeight, 
//				msc_data.src_winLeft, msc_data.src_winTop, msc_data.src_winRight, msc_data.src_winBottom, msc_data.dest_ImgWidth, msc_data.dest_ImgHeight);

		if (ioctl(mCamDevice.convert_fd, TCC_SCALER_IOCTRL, &msc_data) != 0)
		{
			ALOGE( "SCALER Operation Failed!! ");
			return ERR_BUFFER_PROC;
		}

		if(msc_data.responsetype == SCALER_INTERRUPT)
		{
			int ret;
			struct pollfd poll_event[1];
			
			memset(poll_event, 0, sizeof(poll_event));
			poll_event[0].fd = mCamDevice.convert_fd;
			poll_event[0].events = POLLIN;
			ret = poll((struct pollfd*)poll_event, 1, 100);

			if (ret < 0) {
				ALOGE("m2m poll error\n");
				return ERR_BUFFER_PROC;
			}else if (ret == 0) {
				ALOGE("m2m poll timeout %d x %d (%d x %d) -> %d x %d", msc_data.src_ImgWidth, msc_data.src_ImgHeight, msc_data.src_winRight, msc_data.src_winBottom, msc_data.dest_ImgWidth, msc_data.dest_ImgHeight);
				ALOGE("m2m poll timeout 0x%x - 0x%x - 0x%x => 0x%x - 0x%x - 0x%x", msc_data.src_Yaddr, msc_data.src_Uaddr, msc_data.src_Vaddr, msc_data.dest_Yaddr, msc_data.dest_Uaddr, msc_data.dest_Vaddr);
				return ERR_BUFFER_PROC;
			}else if (ret > 0) {
				if (poll_event[0].revents & POLLERR) {
					ALOGE("m2m poll POLLERR\n");
					return ERR_BUFFER_PROC;
				}
			}
		}
#endif
	}

	return (unsigned int)get_pmem_physical_addr(BUFFER_UVC, mCamDevice.preview_idx);
}

//to call in case of not using overlay!!
sp<MemoryBase> TCC_V4l2_Camera::get_currPreviewFrame(int frame_idx)
{
	if(mCamDevice.cam_mode != MODE_PREVIEW)
		return 0;

	if(mUvc_camera)
	{
		if((mPreview_fmt == V4L2_PIX_FMT_MJPEG) || ((mPreview_fmt != V4L2_PIX_FMT_MJPEG) && mCamDevice.needYuv420Cvt))
			return pmem_buffers[BUFFER_UVC][frame_idx];
	}

	return pmem_buffers[BUFFER_PREVIEW][frame_idx];	
}

unsigned int TCC_V4l2_Camera::get_currPreviewPhyAddr(int frame_idx)
{
	if(mUvc_camera)
	{
		if((mPreview_fmt == V4L2_PIX_FMT_MJPEG) || ((mPreview_fmt != V4L2_PIX_FMT_MJPEG) && mCamDevice.needYuv420Cvt))
			return (unsigned int)get_pmem_physical_addr(BUFFER_UVC, frame_idx);
	}
	
	return (unsigned int)get_pmem_physical_addr(BUFFER_PREVIEW, frame_idx);
}

#ifdef USE_CHANGE_SEMIPLANAR
sp<MemoryBase> TCC_V4l2_Camera::get_currCallbackFrame(int frame_idx)
{
	if(mCamDevice.cam_mode != MODE_PREVIEW)
		return 0;

	if(mUvc_camera)
	{
		if((mPreview_fmt == V4L2_PIX_FMT_MJPEG) || ((mPreview_fmt != V4L2_PIX_FMT_MJPEG) && mCamDevice.needYuv420Cvt))
			return pmem_buffers[BUFFER_UVC][frame_idx];
	}

	return pmem_buffers[BUFFER_CALLBACK][0];	
}

unsigned int TCC_V4l2_Camera::get_currCallbackPhyAddr(int frame_idx)
{
	if(mUvc_camera)
	{
		if((mPreview_fmt == V4L2_PIX_FMT_MJPEG) || ((mPreview_fmt != V4L2_PIX_FMT_MJPEG) && mCamDevice.needYuv420Cvt))
			return (unsigned int)get_pmem_physical_addr(BUFFER_UVC, frame_idx);
	}
	
	return (unsigned int)get_pmem_physical_addr(BUFFER_CALLBACK, 0);
}
#endif //USE_CHANGE_SEMIPLANAR

sp<MemoryBase>  TCC_V4l2_Camera::get_nextRecFrame(int *frame_idx, int *iError, bool skip)
{
	uint32_t used_bytes;
	unsigned int *virt_Daddr, curr_idx;

	if(mCamDevice.cam_mode != MODE_PREVIEW)
	{
		ALOGE("ERR_GET_BUFFER :: not preview => cur : %d!!", mCamDevice.cam_mode);
		*iError = ERR_GET_BUFFER;
		return NULL;
	}

	*frame_idx = mCamDevice.preview_idx = camif_get_frame(&used_bytes);
	if(mCamDevice.preview_idx > (mCamDevice.Buffer_cnt-1))
	{
		DBUG("ERR_GET_BUFFER :: %d > %d!!", mCamDevice.preview_idx, (mCamDevice.Buffer_cnt-1));        
		*iError = ERR_GET_BUFFER;
		return NULL;
	}

	//insert physical-address
	virt_Daddr = (unsigned int *)get_pmem_virtual_addr(BUFFER_RECORD, mCamDevice.preview_idx);
	
	if(mUvc_camera)
	{
		if(skip == true){
            DBUG("ERR_BUFFER_PROC :: skip(%d) - %d'th frame!!", skip, *frame_idx); 
			*iError = ERR_BUFFER_PROC;
			return NULL;
		}
		else{
			virt_Daddr[0] = get_currUvcYuv(used_bytes, mCamDevice.preview_idx);
			if(virt_Daddr[0] == ERR_BUFFER_PROC)
			{
                DBUG("ERR_BUFFER_PROC :: get_currUvcYuv(%d) - %d'th frame!!", used_bytes, *frame_idx); 
				*iError = ERR_BUFFER_PROC;
				return NULL;
			}
		}
	}
	else {
		virt_Daddr[0] = (unsigned int)get_pmem_physical_addr(BUFFER_PREVIEW, mCamDevice.preview_idx);
	}

	return pmem_buffers[BUFFER_RECORD][mCamDevice.preview_idx];
}


sp<MemoryHeapBase> TCC_V4l2_Camera::get_previewHeap()
{
	 return mPreviewHeap;
}
#elif defined(USE_CIFOUT_GRALLOC)
int TCC_V4l2_Camera::get_bufferIndex(unsigned int phy_addr)
{
	int index;

	for(index=0; index < mPreviewBufferCount; index++) {
		//DBUG("phy_addr=0x%x, mCameraBufferAddr[%d]=0x%x", phy_addr, index, (uint32_t)mCameraBufferAddr[index]);
		if(phy_addr == (unsigned int)mCameraBufferAddr[index]) break;
	}

	if(index == mPreviewBufferCount) {
		ALOGE("Wrong buffer address");
		return -1;
	}

	return index;	
}

unsigned int TCC_V4l2_Camera::get_currUvcYuv(uint32_t frame_len, unsigned int buffer_index)
{
	int old_needYuv420Cvt = mCamDevice.needYuv420Cvt;
	TCCXXX_JPEG_DEC_DATA dec_data;
    int current_index;
	int scaler_fd;
#ifdef VPU_HW_DECODE
	unsigned int dest_buff_info[4];
#endif

#ifdef USE_G2D_FOR_UVC
	G2D_COMMON_TYPE grp_arg;
#else
	SCALER_TYPE msc_data;
#endif

	unsigned int src_virt_addr = (unsigned int)pmem_mmaped + get_pmem_buffer_offset(BUFFER_PREVIEW, buffer_index);

	if(mPreview_fmt == V4L2_PIX_FMT_MJPEG)
	{	
		unsigned char *temp_frame = (unsigned char *)src_virt_addr;
		
		if(!(temp_frame[frame_len-2] == 0xFF && temp_frame[frame_len-1] == 0xD9))
			return ERR_BUFFER_PROC;
	
		memset(&dec_data, 0x00, sizeof(TCCXXX_JPEG_DEC_DATA));

		dec_data.source_addr	 = (unsigned int)get_pmem_physical_addr(BUFFER_PREVIEW, buffer_index);
		dec_data.file_length	 = frame_len;
		if(old_needYuv420Cvt == 0)
		{
			dec_data.target_addr	 = (unsigned int)get_pmem_physical_addr(BUFFER_UVC, buffer_index);
			dec_data.target_size	 = mCamDevice.buffers_lencvt;
		}
		else
		{
			dec_data.target_addr	 = PA_CONVERT_TEMP_ADDR;
			dec_data.target_size	 = PA_CONVERT_TEMP_SIZE;
		}
		dec_data.ratio 			 = NO_SCALE;

#ifdef VPU_HW_DECODE
		#if defined(TCC_8925S_INCLUDE) || defined(TCC_8935S_INCLUDE)
			memset(&gsJPUDecodeInfo, 0x00, sizeof(conf_dec_t));
			gsJPUDecodeInfo.m_InpPhyJpegBuffAddr 	= dec_data.source_addr;
			gsJPUDecodeInfo.m_InpVirtJpegBuffAddr 	= (codec_addr_t)temp_frame;
			gsJPUDecodeInfo.m_InpSizeOfJpeg 		= frame_len + (100*1024);
			gsJPUDecodeInfo.m_iTotalFrames 		= 1;

			if(tcc_jpu_dec_jpeg(JPU_DEC_DECODE, NULL, &gsJPUDecodeInfo, NULL) < 0) {
				ALOGE("JPU Operation Failed(JPU_DEC_DECODE)!!");
				return ERR_BUFFER_PROC;
			} else {
				dest_buff_info[0] 		= (unsigned int)gsJPUDecodeInfo.m_OutPhyYAddr;
				dest_buff_info[1] 		= (unsigned int)gsJPUDecodeInfo.m_OutPhyUAddr;
				dest_buff_info[2] 		= (unsigned int)gsJPUDecodeInfo.m_OutPhyVAddr;
				dec_data.image_format = gsJPUDecodeInfo.m_OutImgFmt;
				dec_data.width 	= dec_data.pad_width 	= gsJPUDecodeInfo.m_OutWidth;
				dec_data.height 	= dec_data.pad_height 	= gsJPUDecodeInfo.m_OutHeight;
			}
		#else
		if(mVpuState == VPU_CLOSED)
		{
			ALOGE( "VPU can't use. VpuState is %d!! ", mVpuState);
			memset(&gsVDecInput, 0x00, sizeof(vdec_input_t));
			memset(&gsVDecOutput, 0x00, sizeof(vdec_output_t));
			memset(&gsVDecInit, 0x00, sizeof(vdec_init_t));
			memset(&gsVDecUserInfo, 0x00, sizeof(vdec_user_info_t));

			if(vjpeg_decode_operation(NULL, NULL, 0, mCamDevice.preview_width, mCamDevice.preview_height, NULL, NULL, NULL, NULL, NULL, NULL, 0, VDEC_INIT) < 0)
			{
				ALOGE( "VPU Operation Failed (VDEC_INIT)!! ");
				return ERR_BUFFER_PROC;
			}
		}
		
		if(mVpuState == VPU_INITED)
		{
			if(vjpeg_decode_operation(dec_data.source_addr, src_virt_addr, frame_len, 
							mCamDevice.preview_width, mCamDevice.preview_height,
							&(dec_data.width), &(dec_data.height),
							&(dec_data.pad_width), &(dec_data.pad_height), 
							&(dec_data.image_format), (unsigned char *)dest_buff_info, 
							0, VDEC_DEC_SEQ_HEADER) < 0)
			{
				ALOGE( "JPEGD Operation Failed(vpu init)!! ");
				vjpeg_decode_operation(NULL, NULL, 0, mCamDevice.preview_width, mCamDevice.preview_height, NULL, NULL, NULL, NULL, NULL, NULL, 0, VDEC_CLOSE);
				return ERR_BUFFER_PROC;
			}
		}

		if(vjpeg_decode_operation(dec_data.source_addr, src_virt_addr, frame_len, 
						mCamDevice.preview_width, mCamDevice.preview_height,
						&(dec_data.width), &(dec_data.height),
						&(dec_data.pad_width), &(dec_data.pad_height), 
						&(dec_data.image_format), (unsigned char *)dest_buff_info, 
						0, VDEC_DECODE) < 0)
		{
			ALOGE( "JPEGD Operation Failed(vpu decode)!! ");
			vjpeg_decode_operation(NULL, NULL, 0, mCamDevice.preview_width, mCamDevice.preview_height, NULL, NULL, NULL, NULL, NULL, NULL, 0, VDEC_CLOSE);
			return ERR_BUFFER_PROC;
		}
		#endif

		mCamDevice.needYuv420Cvt = 1;
		
		if(dec_data.image_format == 0 || dec_data.image_format == 4)
			dec_data.image_format = IMAGE_CHROMA_420;
		//else if(dec_data.image_format == 2)
		//	dec_data.image_format = IMAGE_CHROMA_440;		
		//else if(dec_data.image_format == 3)
		//	dec_data.image_format = IMAGE_CHROMA_444;
		else
			dec_data.image_format = IMAGE_CHROMA_422;		
#else
		if(ioctl(mCamDevice.jpegdec_fd,TCC_JPEGD_IOCTL_DEC, &dec_data) < 0)
		{
			ALOGE( "JPEGD Operation Failed(%s)!! ", strerror(errno));
			return ERR_BUFFER_PROC;
		}	

		if(dec_data.image_format == IMAGE_CHROMA_420)
			mCamDevice.needYuv420Cvt = 0;
		else
			mCamDevice.needYuv420Cvt = 1;	
#endif
	}
	else
	{
		if(mPreview_fmt == V4L2_PIX_FMT_YUYV)
		{
			mCamDevice.needYuv420Cvt = 1;
		}
		else
		{
			mCamDevice.needYuv420Cvt = 0;
//			return (unsigned int)get_pmem_physical_addr(BUFFER_PREVIEW, mCamDevice.preview_idx);
		}
	}

	//if(old_needYuv420Cvt != 0)
	{
#ifdef USE_G2D_FOR_UVC
		memset(&grp_arg, 0x00, sizeof(G2D_COMMON_TYPE));
		
		grp_arg.responsetype 	= G2D_INTERRUPT; //G2D_POLLING;	
		grp_arg.src_imgx 		= dec_data.pad_width;
		grp_arg.src_imgy 		= dec_data.pad_height;
		grp_arg.srcfm.format	= GE_YUV420_sp;
#ifdef VPU_HW_DECODE
		VDBUG( "vjpeg_decode src(0x%x - 0x%x - 0x%x) Success!! ", dest_buff_info[0], dest_buff_info[1], dest_buff_info[2]);
	
		grp_arg.src0			= dest_buff_info[0]; 		
		grp_arg.src1			= dest_buff_info[1]; 
		grp_arg.src2			= dest_buff_info[2];
		if(dec_data.image_format == IMAGE_CHROMA_420)
			grp_arg.srcfm.format = GE_YUV420_sp; 
		else
			grp_arg.srcfm.format = GE_YUV422_sp; 
#else
		grp_arg.src0			= PA_CONVERT_TEMP_ADDR;			
		grp_arg.src1			= GET_ADDR_YUV42X_spU(grp_arg.src0 , grp_arg.src_imgx, grp_arg.src_imgy );
		grp_arg.src2			= GET_ADDR_YUV422_spV(grp_arg.src1 , grp_arg.src_imgx, grp_arg.src_imgy );	
		grp_arg.srcfm.format	= GE_YUV422_sp; 	
#endif

		grp_arg.srcfm.data_swap	= 0;
		grp_arg.DefaultBuffer 	= 0;

		grp_arg.tgtfm.format 	= GE_YUV420_sp; 	
		grp_arg.tgtfm.data_swap	= 0;
        grp_arg.tgtfm.uv_order =  1;
		grp_arg.ch_mode 	= NOOP;

		grp_arg.crop_offx = 0;
		grp_arg.crop_offy = 0;
		grp_arg.crop_imgx	= mCamDevice.preview_width;
		grp_arg.crop_imgy	= mCamDevice.preview_height;	

		grp_arg.dst_imgx = mCamDevice.preview_width;
		grp_arg.dst_imgy = mCamDevice.preview_height;

		grp_arg.dst_off_x = 0;
		grp_arg.dst_off_y = 0;
		grp_arg.tgt0		= (unsigned int)get_pmem_physical_addr(BUFFER_UVC, mCamDevice.preview_idx);
		grp_arg.tgt1		= GET_ADDR_YUV42X_spU(grp_arg.tgt0 , grp_arg.dst_imgx, grp_arg.dst_imgy );
		grp_arg.tgt2		= GET_ADDR_YUV420_spV(grp_arg.tgt1 , grp_arg.dst_imgx, grp_arg.dst_imgy );
		
		if(ioctl(mCamDevice.g2d_fd, TCC_GRP_COMMON_IOCTRL, &grp_arg) != 0)
		{
			ALOGE( "G2D Operation Failed!! ");
			return ERR_BUFFER_PROC;
		}

		if(grp_arg.responsetype == G2D_INTERRUPT)
		{
			int ret;
			struct pollfd poll_event[1];
			
			memset(poll_event, 0, sizeof(poll_event));
			poll_event[0].fd = mCamDevice.g2d_fd;
			poll_event[0].events = POLLIN;
			ret = poll((struct pollfd*)poll_event, 1, 100);

			if (ret < 0) {
				ALOGE("g2d poll error\n");
				return ERR_BUFFER_PROC;
			}else if (ret == 0) {
				ALOGE("g2d poll timeout %d x %d (%d x %d) -> %d x %d", grp_arg.src_imgx, grp_arg.src_imgy, grp_arg.crop_offx, grp_arg.crop_offx, grp_arg.crop_imgx, grp_arg.crop_imgy);
				ALOGE("g2d poll timeout 0x%x - 0x%x - 0x%x => 0x%x - 0x%x - 0x%x", grp_arg.src0, grp_arg.src1, grp_arg.src2, grp_arg.tgt0, grp_arg.tgt2, grp_arg.tgt2);
				return ERR_BUFFER_PROC;
			}else if (ret > 0) {
				if (poll_event[0].revents & POLLERR) {
					ALOGE("g2d poll POLLERR\n");
					return ERR_BUFFER_PROC;
				}
			}
		}
#else
		memset(&msc_data, 0x00, sizeof(SCALER_TYPE));

		msc_data.responsetype 	= SCALER_INTERRUPT; 

		if(mPreview_fmt == V4L2_PIX_FMT_MJPEG){
				msc_data.src_ImgWidth 	= dec_data.pad_width;	
				msc_data.src_ImgHeight	= dec_data.pad_height;	
			#ifdef VPU_HW_DECODE
				VDBUG( "vjpeg_decode src(0x%x - 0x%x - 0x%x) Success!! ", dest_buff_info[0], dest_buff_info[1], dest_buff_info[2]);

				msc_data.src_Yaddr		= (char*)dest_buff_info[0];
				msc_data.src_Uaddr		= (char*)dest_buff_info[1];
				msc_data.src_Vaddr		= (char*)dest_buff_info[2];
				if(dec_data.image_format == IMAGE_CHROMA_420)
					msc_data.src_fmt		= SCALER_YUV420_sp; 
				else
					msc_data.src_fmt		= SCALER_YUV422_sp; 
			#else
				msc_data.src_Yaddr		= (char*)PA_CONVERT_TEMP_ADDR;			
				msc_data.src_Uaddr		= (char*)GET_ADDR_YUV42X_spU(msc_data.src_Yaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight );
				msc_data.src_Vaddr      = (char*)GET_ADDR_YUV422_spV(msc_data.src_Uaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight );	
				msc_data.src_fmt        = SCALER_YUV422_sp;		
			#endif

			msc_data.src_winRight 	= dec_data.width;
			msc_data.src_winBottom	= dec_data.height;
			msc_data.src_winLeft	= 0;
			msc_data.src_winTop		= 0;
		}
		else {
			msc_data.src_ImgWidth 	= mCamDevice.vid_fmt.fmt.pix.width; 	
			msc_data.src_ImgHeight	= mCamDevice.vid_fmt.fmt.pix.height; 

		#ifdef VPU_HW_DECODE
			msc_data.src_Yaddr		= (char*)get_pmem_physical_addr(BUFFER_PREVIEW, buffer_index);	
		#else
			msc_data.src_Yaddr		= (char*)PA_CONVERT_TEMP_ADDR;			
			msc_data.src_Uaddr		= (char*)GET_ADDR_YUV42X_spU(msc_data.src_Yaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight );
			msc_data.src_Vaddr      = (char*)GET_ADDR_YUV422_spV(msc_data.src_Uaddr , msc_data.src_ImgWidth, msc_data.src_ImgHeight );	
		#endif
            if(mPreview_fmt == V4L2_PIX_FMT_YUYV)
				msc_data.src_fmt		= SCALER_YUV422_sq0; 			
			else
				msc_data.src_fmt		= SCALER_YUV420_sp;

			msc_data.src_winRight 	= msc_data.src_ImgWidth;
			msc_data.src_winBottom	= msc_data.src_ImgHeight;
			msc_data.src_winLeft	= 0;
			msc_data.src_winTop		= 0;
		}			

        if(mCamDevice.cam_mode == MODE_CAPTURED)
        {
			msc_data.dest_ImgWidth	= mCamDevice.cap_info.width;
			msc_data.dest_ImgHeight	= mCamDevice.cap_info.height;

			msc_data.dest_Yaddr		= (char*)PA_JPEG_RAW_BASE_ADDR;
			msc_data.dest_Uaddr 	= (char *)GET_ADDR_YUV42X_spU(msc_data.dest_Yaddr , msc_data.dest_ImgWidth, msc_data.dest_ImgHeight);
			msc_data.dest_Vaddr 	= (char *)GET_ADDR_YUV422_spV(msc_data.dest_Uaddr , msc_data.dest_ImgWidth, msc_data.dest_ImgHeight);
			msc_data.dest_fmt 		= SCALER_YUV422_sp;

			if(0) //for frame dump.
			{
				FILE *pFs;

				pFs = fopen("/sdcard/uvc_src_422sq.yuv", "ab+");
				if (!pFs) {
					ALOGE("Cannot open '/sdcard/uvc_src_422sq.yuv'");
				}
				else
				{
					fwrite( (void*)src_virt_addr, msc_data.dest_ImgWidth*msc_data.dest_ImgHeight*2, 1, pFs);
					fclose(pFs);
				}
			}

			DBUG("@@@ Scaler-Operation :: %d x %d, 0x%x => %d x %d, 0x%x", msc_data.src_ImgWidth, msc_data.src_ImgHeight, (unsigned int)msc_data.src_Yaddr, msc_data.dest_ImgWidth, msc_data.dest_ImgHeight, (unsigned int)msc_data.dest_Yaddr);
		}
        else
        {
			unsigned int stride, stride_c;
			unsigned int framesize_Y, framesize_C;

			msc_data.dest_ImgWidth	= mCamDevice.preview_width;
			msc_data.dest_ImgHeight	= mCamDevice.preview_height;

			stride = ALIGNED_BUFF(msc_data.dest_ImgWidth, 16);
			framesize_Y = ALIGNED_BUFF(stride * msc_data.dest_ImgHeight, 64);

			stride_c = ALIGNED_BUFF(stride/2, 16);
			framesize_C = ALIGNED_BUFF(stride_c * msc_data.dest_ImgHeight/2, 64);

			msc_data.dest_Yaddr		= (char*)mCameraBufferAddr[buffer_manager[buffer_manager_out_index]];
			msc_data.dest_Vaddr		= (char*)msc_data.dest_Yaddr + framesize_Y;
			msc_data.dest_Uaddr		= (char*)msc_data.dest_Vaddr + framesize_C;
			msc_data.dest_fmt 		= SCALER_YUV420_sp;
		}

		msc_data.dest_winLeft 	= 0;
		msc_data.dest_winTop	= 0;
		msc_data.dest_winRight	= msc_data.dest_ImgWidth;
		msc_data.dest_winBottom	= msc_data.dest_ImgHeight;

		if (ioctl(mCamDevice.convert_fd, TCC_SCALER_IOCTRL, &msc_data) != 0)
		{
			ALOGE( "SCALER Operation Failed!! ");
			return ERR_BUFFER_PROC;
		}

		if(msc_data.responsetype == SCALER_INTERRUPT)
		{
			int ret;
			struct pollfd poll_event[1];
			
			memset(poll_event, 0, sizeof(poll_event));
			poll_event[0].fd = mCamDevice.convert_fd;
			poll_event[0].events = POLLIN;
			ret = poll((struct pollfd*)poll_event, 1, 100);

			if (ret < 0) {
				ALOGE("m2m poll error\n");
				return ERR_BUFFER_PROC;
			}else if (ret == 0) {
				ALOGE("m2m poll timeout %d x %d (%d x %d) -> %d x %d", msc_data.src_ImgWidth, msc_data.src_ImgHeight, msc_data.src_winRight, msc_data.src_winBottom, msc_data.dest_ImgWidth, msc_data.dest_ImgHeight);
				ALOGE("m2m poll timeout 0x%x - 0x%x - 0x%x => 0x%x - 0x%x - 0x%x", msc_data.src_Yaddr, msc_data.src_Uaddr, msc_data.src_Vaddr, msc_data.dest_Yaddr, msc_data.dest_Uaddr, msc_data.dest_Vaddr);
				return ERR_BUFFER_PROC;
			}else if (ret > 0) {
				if (poll_event[0].revents & POLLERR) {
					ALOGE("m2m poll POLLERR\n");
					return ERR_BUFFER_PROC;
				}
			}
		}
#endif
	}

    if(mCamDevice.cam_mode == MODE_CAPTURED) {
#ifdef USE_G2D_FOR_UVC
    	return (unsigned int)(grp_arg.tgt0);
#else
    	return (unsigned int)(msc_data.dest_Yaddr);
#endif
    }
    else {
        current_index = buffer_manager_out_index;
        buffer_manager_out_index = (buffer_manager_out_index+1)%mPreviewBufferCount;
    }

	return (unsigned int)current_index;
}

unsigned int TCC_V4l2_Camera::get_nextPhyFrame(int *frame_idx, bool skip)
{
	unsigned int index;
	uint32_t used_bytes;

	if(mCamDevice.cam_mode != MODE_PREVIEW) {
		ALOGE("Err :: not preview => cur : %d!!", mCamDevice.cam_mode);
		return ERR_GET_BUFFER;
	}
	
	index = camif_get_frame(&used_bytes);

	if(index > ((uint32_t)mCamDevice.Buffer_cnt - 1)) return ERR_GET_BUFFER;
	
	*frame_idx = mCamDevice.preview_idx = index;

	if(mUvc_camera) {
		if(skip == true)
			return ERR_BUFFER_PROC;
		else
			return get_currUvcYuv(used_bytes, mCamDevice.preview_idx);
	}

	return mCamDevice.buffers_phy[mCamDevice.preview_idx];
}

#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
unsigned int TCC_V4l2_Camera::get_nextRecVirtFrame(int frame_idx)
{
	uint8_t i_cnt;	
	uint32_t used_bytes;
	
	if(mCamDevice.cam_mode != MODE_PREVIEW)
		return 0;

	mCamDevice.preview_idx = camif_get_frame(&used_bytes);

	return mCamDevice.rec_buffers_virt[mCamDevice.preview_idx];
}

unsigned int TCC_V4l2_Camera::get_nextRecPhyFrame(uint8_t *buffer, int frame_idx)
{
	unsigned int index;
	uint32_t used_bytes;

	if(mCamDevice.cam_mode != MODE_PREVIEW)
	{
		ALOGE("Err :: not preview => cur : %d!!", mCamDevice.cam_mode);
		return ERR_GET_BUFFER;
	}
	
	//index = camif_get_frame(&used_bytes);

	//if(index > (mCamDevice.Buffer_cnt-1))
	//	return ERR_GET_BUFFER;
	
	//*frame_idx = mCamDevice.preview_idx;// = index;

	return mCamDevice.rec_buffers_phy[frame_idx];
}

int TCC_V4l2_Camera::get_RecbufferIndex(unsigned int phy_addr)
{
	int index;

	for(index = 0; index <mCamDevice.Buffer_cnt; index ++){
		if(phy_addr == mCamDevice.rec_buffers_phy[index])
			break;
    }

#if 0
	if (index == mCamDevice.Buffer_cnt)
	{
		ALOGE("Wrong buffer address");
		return -1;
	}
#endif

	return index;
}
#endif /* end of SENSOR_TVP5150 */
#endif

void TCC_V4l2_Camera::setParameter(int type, int value)
{
	if(mUvc_camera) return;
	camif_set_queryctrl(type, value); 
	DBUG("********* setParameter - type: %d, value: %d ***********", type, value);
}

int TCC_V4l2_Camera::setOperation(int type, int value)
{
	int proc_type;
	int proc_value = value;
	int ret = -1;

	if(mUvc_camera)
		return ret;
	
	switch(type)
	{
		case PROC_AF:
			proc_type = VIDIOC_USER_PROC_AUTOFOCUS;
			break;

		default:
			return ret;
	}
	
	if(ret=ioctl(mCamDevice.fd, proc_type, &proc_value) < 0)
	{
		ALOGE("Err[%s] :: setOperation (%d - %d) failed", strerror(errno), type, value);
	}

	DBUG("********* setOperation - type: %d, value: %d ***********", type, value);
	return ret;
}

void TCC_V4l2_Camera::set_fps(int fps)
{
	int ret, type;
	
	if(mUvc_camera) {
		struct v4l2_streamparm parm;
		parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if(ioctl(mCamDevice.fd, VIDIOC_G_PARM, &parm) < 0) {
			ALOGE( "Err[%s] :: VIDIOC_S_PARM %d fps Failed!! ", strerror(errno), parm.parm.output.timeperframe.denominator);
		}

		if((uint32_t)fps != parm.parm.output.timeperframe.denominator) {
			parm.parm.output.timeperframe.numerator = 1;
			parm.parm.output.timeperframe.denominator = fps;

			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if((ret = ioctl (mCamDevice.fd, VIDIOC_STREAMOFF, &type)) < 0) {
				ALOGE("Err[%s]-- :: VIDIOC_STREAMOFF faild!!", strerror(errno));
				return;
			}
			
			if(ioctl(mCamDevice.fd, VIDIOC_S_PARM, &parm) < 0) {
				ALOGE( "Err[%s] :: VIDIOC_S_PARM %d fps Failed!! ", strerror(errno), parm.parm.output.timeperframe.denominator);
			}

			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if((ret = ioctl(mCamDevice.fd, VIDIOC_STREAMON, &type)) < 0) {
				ALOGE("Err[%s]-- :: VIDIOC_STREAMON faild!!", strerror(errno));
				return;
			}
			ALOGD("Interval : %d, %d", parm.parm.output.timeperframe.numerator, parm.parm.output.timeperframe.denominator);
		}
	}
}

void TCC_V4l2_Camera::setPreviewSize(unsigned int width, unsigned int height, unsigned char landscape)
{
	DBUG("*Mode[%d] before setPreviewSize:: %d x %d * : * new setPreviewSize:: %d x %d *",mCamDevice.cam_mode ,mCamDevice.preview_width, mCamDevice.preview_height, width, height);

	if(mCamDevice.cam_mode == MODE_CAPTURED && (mCamDevice.preview_width != width || mCamDevice.preview_height != height))
		mPreviewSizeChanged = true;
	else
		mPreviewSizeChanged = false;

	if((width < height) && landscape) {
		mCamDevice.preview_width  = height;
		mCamDevice.preview_height = width;
	} else {
		mCamDevice.preview_width  = width;
		mCamDevice.preview_height = height;
	}

	#if defined(USE_CIFOUT_GRALLOC)
	mPreviewWidth  = mCamDevice.preview_width;
	mPreviewHeight = mCamDevice.preview_height;
	#endif

	DBUG("********* setPreviewSize-%d :: %d x %d ***********", landscape, mCamDevice.preview_width, mCamDevice.preview_height);
}

void TCC_V4l2_Camera::getPreviewSize(unsigned int *width, unsigned int *height)
{
	*width  = mCamDevice.preview_width;
	*height = mCamDevice.preview_height;

	DBUG("********* getPreviewSize :: %d x %d ***********", mCamDevice.preview_width, mCamDevice.preview_height);
}

int TCC_V4l2_Camera::getCapturedJpegData(uint8_t *buffer)
{
	FUNCTION_IN
	unsigned char *virJpegHeaderAddr = NULL, *virJpegStreamAddr = NULL;
	#if defined(_TCC8920_) || defined(_TCC8930_)
		const char JPU_Header_Offset 		= 0x08;
		const char JPU_Stream_Offset 		= 0x03;
		unsigned int exifHeaderSize 		= mCamDevice.cap_info.jpeg_save_info.header_size;
		unsigned int jpegStreamOffset 		= mCamDevice.cap_info.jpeg_save_info.bitstream_offset;
		unsigned int jpegStreamSize 		= mCamDevice.cap_info.jpeg_save_info.bitstream_size + JPU_Header_Offset + JPU_Stream_Offset - jpegStreamOffset;
		unsigned int jpegHeaderSize 		= jpegStreamOffset-JPU_Header_Offset-JPU_Stream_Offset;
	#else
		const unsigned int phyJpegHeaderAddr 	= (unsigned int)PA_JPEG_HEADER_BASE_ADDR;
		const unsigned int jpegHeaderBufferSize = (unsigned int)JPEG_HEADER_MEM_SIZE;
		const unsigned int jpegHeaderSize 		= mCamDevice.cap_info.jpeg_save_info.header_size;
		const unsigned int jpegHeaderOffset 	= mCamDevice.cap_info.jpeg_save_info.header_offset;
		const unsigned int phyJpegStreamAddr 	= (unsigned int)PA_JPEG_STREAM_BASE_ADDR;
		const unsigned int jpegStreamBufferSize = (unsigned int)JPEG_STREAM_MEM_SIZE;
		const unsigned int jpegStreamSize 		= mCamDevice.cap_info.jpeg_save_info.bitstream_size;
		const unsigned int jpegStreamOffset 	= mCamDevice.cap_info.jpeg_save_info.bitstream_offset;
	#endif

	// get virtual jpeg buffer address.
	#if defined(_TCC8920_) || defined(_TCC8930_)
		virJpegHeaderAddr = mJpegHeaderAddr;
		virJpegStreamAddr = mJpegStreamAddr;
		DBUG("getCapturedJpegData1:  virJpegHeaderAddr =0x%x , virJpegStreamAddr = 0x%x.", virJpegHeaderAddr, virJpegStreamAddr);
	#else
		//virJpegHeaderAddr = (char *)mmap(NULL, jpegHeaderBufferSize, (PROT_READ|PROT_WRITE), MAP_SHARED, mCamDevice.fd, phyJpegHeaderAddr);
		virJpegStreamAddr = (char *)mmap(NULL, jpegStreamBufferSize, (PROT_READ|PROT_WRITE), MAP_SHARED, mCamDevice.fd, phyJpegStreamAddr);
	#endif

	// copy to jpeg data.
	#if defined(_TCC8920_) || defined(_TCC8930_)
		if((virJpegHeaderAddr != MAP_FAILED) && (virJpegStreamAddr != MAP_FAILED)) {
			if(mUvc_camera && (mCapture_fmt == V4L2_PIX_FMT_MJPEG)) {
				jpegHeaderSize 	= mCamDevice.cap_info.jpeg_save_info.header_size;
				jpegStreamSize 	= mCamDevice.cap_info.jpeg_save_info.bitstream_size;
				memcpy(buffer, virJpegStreamAddr, (jpegHeaderSize+jpegStreamSize));
			} else {
				if(supported_thumbnail) {
					memcpy(buffer, virJpegHeaderAddr, exifHeaderSize); 						// exif header copy.
					buffer += exifHeaderSize;
					memcpy(buffer, (virJpegStreamAddr+JPU_Header_Offset), jpegHeaderSize); 	// jpu header copy.
					buffer += jpegHeaderSize;
					memcpy(buffer, (virJpegStreamAddr+jpegStreamOffset), jpegStreamSize); 	// jpu stream copy.
				} else {
					memcpy(buffer, virJpegStreamAddr, mCamDevice.cap_info.jpeg_save_info.bitstream_size); 	// jpu stream copy.
				}
			}
			DBUG("getCapturedJpegData2:  JPEG File Size = %d( %d Kbyte ).", jpegHeaderSize+jpegStreamSize, (jpegHeaderSize+jpegStreamSize)/1024);
		}
	#else
		if(virJpegHeaderAddr != MAP_FAILED) {
			memcpy(buffer, (virJpegStreamAddr + jpegHeaderOffset), jpegHeaderSize);
			buffer += jpegHeaderSize;
			if(mUvc_camera) 	memcpy(buffer, (virJpegStreamAddr + jpegStreamOffset), jpegStreamSize);
			else 			memcpy(buffer, (virJpegStreamAddr + jpegStreamOffset), (jpegStreamSize - jpegStreamOffset));
		}
	#endif
	else {
		ALOGE( "JPEGE mmap(jpeg_buffer) Operation Failed(%s)!! ", strerror(errno));
	}

	#if !defined(_TCC8920_) && !defined(_TCC8930_)
		//munmap(virJpegHeaderAddr, jpegHeaderBufferSize);
		munmap(virJpegStreamAddr, jpegStreamBufferSize);
	#endif

	FUNCTION_OUT;
	return 0;
}

int TCC_V4l2_Camera::getCapturedRawData(uint8_t *buffer)
{
	FUNCTION_IN
	char* virRawBuffer 					= NULL;
	const unsigned int phyRawBufferAddr = mCamDevice.cap_info.jpeg_save_info.source_addr;
	const unsigned int RawSize 			= mCamDevice.cap_info.width * mCamDevice.cap_info.height * 2;

	DBUG("raw data info:  phy_addr=0x%08x, size=0x%x.", phyRawBufferAddr, RawSize);

	// get virtual jpeg buffer address.
	virRawBuffer = (char *)mmap(NULL, RawSize, (PROT_READ|PROT_WRITE), MAP_SHARED, mCamDevice.fd, phyRawBufferAddr);

	// copy to jpeg data.
	if(virRawBuffer != MAP_FAILED) {
		memcpy(buffer, virRawBuffer, RawSize);
		munmap(virRawBuffer, RawSize);
	} else {
		ALOGE( "JPEGE mmap(jpeg_buffer) Operation Failed(%s)!! ", strerror(errno));
	}

	FUNCTION_OUT;
	return 0;
}

int TCC_V4l2_Camera::getPreviewRawData(uint8_t *buffer, uint8_t prv_idx)
{
	FUNCTION_IN
	char* virPreviewBuffer 					= NULL;
	unsigned int y_offset = 0, uv_offset = 0, stride = 0;
	unsigned int phyPreviewBufferAddr = (uint32_t)mCameraBufferAddr[prv_idx];
	const unsigned int previewBufferSize 	= mPreviewWidth * mPreviewHeight * 1.5;

	change_uv_data(phyPreviewBufferAddr, PA_CONVERT_TEMP_ADDR);

	// get virtual preview buffer address.
	virPreviewBuffer = (char *)mmap(NULL, previewBufferSize, (PROT_READ|PROT_WRITE), MAP_SHARED, mCamDevice.fd, PA_CONVERT_TEMP_ADDR);

	// copy to preview data.
	if(virPreviewBuffer != MAP_FAILED) {
		// Because of MAIL400, We have to copy preview buffer according to memeory align.
		memcpy(buffer, virPreviewBuffer, previewBufferSize);				
		munmap(virPreviewBuffer, previewBufferSize);
	} else {
		ALOGE( "JPEGE mmap(jpeg_buffer) Operation Failed(%s)!! ", strerror(errno));
	}
	
#if 0			
		stride = ALIGNED_BUFF(mPreviewWidth, 16);
		y_offset = stride * mPreviewHeight;
		uv_offset = ALIGNED_BUFF((stride/2), 16) * (mPreviewHeight/2);	

		DBUG("raw data info:  phy_addr=0x%08x, size=0x%x[0x%x].", phyPreviewBufferAddr, previewBufferSize, (y_offset+2*uv_offset));
		
		// get virtual preview buffer address.
		virPreviewBuffer = (char *)mmap(NULL, (y_offset+2*uv_offset)/*previewBufferSize*/, (PROT_READ|PROT_WRITE), MAP_SHARED, mCamDevice.fd, phyPreviewBufferAddr);

		// copy to preview data.
		if(virPreviewBuffer != MAP_FAILED) {
			// Because of MAIL400, We have to copy preview buffer according to memeory align.
			memcpy(buffer, virPreviewBuffer, (mPreviewWidth * mPreviewHeight));	
			virPreviewBuffer += y_offset;	
			buffer += (mPreviewWidth * mPreviewHeight);
			memcpy(buffer, virPreviewBuffer, (mPreviewWidth * mPreviewHeight/4));
			virPreviewBuffer += uv_offset;
			buffer += (mPreviewWidth * mPreviewHeight/4);
			memcpy(buffer, virPreviewBuffer, (mPreviewWidth * mPreviewHeight/4));
			munmap(virPreviewBuffer, previewBufferSize);
		} else {
			ALOGE( "JPEGE mmap(jpeg_buffer) Operation Failed(%s)!! ", strerror(errno));
		}

#endif


	FUNCTION_OUT;
	return 0;
}

int TCC_V4l2_Camera::change_uv_data(unsigned int srcY, unsigned int dest)
{
	FUNCTION_IN
	struct pollfd poll_event[1];
	unsigned int SoutWidth, SoutHeight;
	unsigned int y_offset = 0, uv_offset = 0, stride = 0;
	G2D_COMMON_TYPE grp_arg;
	//int mRt_fd;

	stride = ALIGNED_BUFF(mPreviewWidth, 16);
	y_offset = stride * mPreviewHeight;
	uv_offset = ALIGNED_BUFF((stride/2), 16) * (mPreviewHeight/2);

	memset(&grp_arg, NULL, sizeof(G2D_COMMON_TYPE));
	grp_arg.responsetype 	= G2D_INTERRUPT; //G2D_INTERRUPT, G2D_POLLING;
	grp_arg.srcfm.format 	= GE_YUV420_sp;
	grp_arg.srcfm.data_swap = 0;
	grp_arg.src_imgx 		= mPreviewWidth;
	grp_arg.src_imgy 		= mPreviewHeight;

	DBUG("change_uv_data: %dx%d ", grp_arg.src_imgx , grp_arg.src_imgy);

	grp_arg.DefaultBuffer 	= 0;

	grp_arg.tgtfm.format 	= GE_YUV420_in;
	grp_arg.tgtfm.data_swap = 0;
	grp_arg.tgtfm.uv_order 	= 1;

	grp_arg.ch_mode 		= NOOP;

	grp_arg.crop_offx 		= 0;
	grp_arg.crop_offy 		= 0;
	grp_arg.crop_imgx 		= grp_arg.src_imgx;
	grp_arg.crop_imgy 		= grp_arg.src_imgy;

	grp_arg.dst_imgx 		= grp_arg.crop_imgx;
	grp_arg.dst_imgy 		= grp_arg.crop_imgy;

	grp_arg.dst_off_x 		= 0;
	grp_arg.dst_off_y 		= 0;
	
	grp_arg.src0 			= (unsigned int)srcY;
	grp_arg.src1 			= srcY + y_offset + uv_offset;//GET_ADDR_YUV42X_spU(grp_arg.src0, iVideoDisplayWidth, iVideoDisplayHeight);
	grp_arg.src2 			= srcY + y_offset;//GET_ADDR_YUV420_spV(grp_arg.src1, iVideoDisplayWidth, iVideoDisplayHeight);
	grp_arg.tgt0 			= (unsigned int)dest;
	grp_arg.tgt1 			= GET_ADDR_YUV42X_spU(grp_arg.tgt0, mPreviewWidth, mPreviewHeight);
	grp_arg.tgt2 			= GET_ADDR_YUV420_spV(grp_arg.tgt1, mPreviewWidth, mPreviewHeight);
	
	if(mCamDevice.g2d_fd <= 0) {
		mCamDevice.g2d_fd = open(GRAPHIC_DEVICE, O_RDWR);
		if(mCamDevice.g2d_fd <= 0) ALOGE("can't open[%s] '%s'", strerror(errno), GRAPHIC_DEVICE);
	}

	if(ioctl(mCamDevice.g2d_fd, TCC_GRP_COMMON_IOCTRL, &grp_arg) != 0) {
		ALOGE("G2D Out Error!");
	}

	if(grp_arg.responsetype == G2D_INTERRUPT) {
		int ret;

		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd 	 = mCamDevice.g2d_fd;
		poll_event[0].events = POLLIN;

		ret = poll((struct pollfd*)poll_event, 1, 400);
		if(ret < 0) {
			ALOGE("g2d poll error\n");
			return -1;
		} else if(ret == 0) {
			ALOGE("g2d poll timeout\n");		
			return -1;
		} else if (ret > 0) {
			if(poll_event[0].revents & POLLERR) {
				ALOGE("g2d poll POLLERR\n");
				return -1;
			}
		}
	}

	FUNCTION_OUT;
	return NO_ERROR;
}

bool TCC_V4l2_Camera::get_cameraType_isUVC()
{
	if(mUvc_camera)
		return true;
	else
		return false;
}

int TCC_V4l2_Camera::get_default_framerate(int*fps)
{
    if(mDef_MaxFps == 30)
        *fps = 29;
    else
	    *fps = mDef_MaxFps;

    return 0;
}

void TCC_V4l2_Camera::setJPEGQuality(int quality)
{
	DBUG("jpeg quality is %d.", quality);
	switch(quality) {
		case 70: 	mJpegQuality = JPEG_MEDIUM;
			break;
		case 80: 	mJpegQuality = JPEG_FINE;
			break;
		case 90:
		default: 	mJpegQuality = JPEG_SUPERFINE;
			break;
	}
}

void TCC_V4l2_Camera::copyToVideoSnapshotFrame(unsigned int index)
{
	FUNCTION_IN
	char *virPreviewBuffer, *virCaptureBuffer 	= NULL;
	const unsigned int phyPreviewBufferAddr 	= (uint32_t)mCameraBufferAddr[index];
	const unsigned int previewBufferSize 		= mPreviewWidth * mPreviewHeight * 1.5;
	const unsigned int phyCaptureBufferAddr 	= (uint32_t)PA_JPEG_RAW_BASE_ADDR;
	const unsigned int captureBufferSize 		= JPEG_RAW_MEM_SIZE;
	const unsigned int y_offset 				= mPreviewWidth * mPreviewHeight;
	const unsigned int uv_offset 				= mPreviewWidth * mPreviewHeight / 4;

	DBUG("preview buffer address:  0x%08x, size=0x%x , capture buffer address:  0x%08x, size=0x%x.", 	\
						phyPreviewBufferAddr, previewBufferSize, phyCaptureBufferAddr, captureBufferSize);

	// get virtual preview buffer address , virtual capture buffer address.
	virPreviewBuffer = (char *)mmap(NULL, previewBufferSize, (PROT_READ|PROT_WRITE), MAP_SHARED, mCamDevice.fd, phyPreviewBufferAddr);
	if(virPreviewBuffer != MAP_FAILED) {
		virCaptureBuffer = (char *)mmap(NULL, captureBufferSize, (PROT_READ|PROT_WRITE), MAP_SHARED, mCamDevice.fd, phyCaptureBufferAddr);
	}

	// copy to preview data.
	if(virCaptureBuffer != MAP_FAILED) {
		memcpy((void *)virCaptureBuffer, (void *)virPreviewBuffer, y_offset);
		memcpy((void *)(virCaptureBuffer + y_offset), (void *)(virPreviewBuffer + y_offset + uv_offset), uv_offset);
		memcpy((void *)(virCaptureBuffer + y_offset + uv_offset), (void *)(virPreviewBuffer + y_offset), uv_offset);
		munmap(virPreviewBuffer, previewBufferSize);
		munmap(virCaptureBuffer, captureBufferSize);
	} else {
		ALOGE( "JPEGE mmap(jpeg_buffer) Operation Failed(%s)!! ", strerror(errno));
	}
	FUNCTION_OUT
}

#if (0)
int TCC_V4l2_Camera::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, 255, " width x height (%d x %d), counter (%d), check x-y coordinate(%d, %d)", mCamDevice.preview_width, mCamDevice.preview_height, 0, 0, 0);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}
#endif

#if(1) // 20131018 swhwang, for check video frame interrupt
int TCC_V4l2_Camera::camif_reopen_device(int nCamIndex)
{
	return camif_open_device(nCamIndex);
}
#endif

}; // namespace android
