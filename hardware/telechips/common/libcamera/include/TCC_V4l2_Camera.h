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

#ifndef ANDROID_HARDWARE_TCCCAMERA_H
#define ANDROID_HARDWARE_TCCCAMERA_H

#include <mach/tcc_jpeg_ioctl.h>
#include <mach/tcc_cam_ioctrl.h>
#include <mach/tcc_scaler_ioctrl.h>
#include <mach/tcc_grp_ioctrl.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_composite_ioctl.h>
#include <mach/tcc_component_ioctl.h>
#include <mach/tcc_overlay_ioctl.h>
#include <mach/tcc_viqe_ioctl.h>
#include <mach/tcc_hdmi_in_parameters.h>
#include <libpmap/pmap.h>

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>
#include <linux/fb.h>
#include <autoconf.h> 


#define CAM_0_DEVICE		"/dev/video1"
#define CAM_1_DEVICE		"/dev/video0"
#define JPEGENC_DEVICE		"/dev/jpegenc"
#define JPEGDEC_DEVICE		"/dev/jpegdec"
#if defined(_TCC8900_) || defined(_TCC92XX_)
#define SCALER_DEVICE		"/dev/scaler1"
#define SCALER1_DEVICE		"/dev/scaler"
#else
#define SCALER_DEVICE		"/dev/scaler"
#define SCALER1_DEVICE		"/dev/scaler1"
#endif
#define GRAPHIC_DEVICE		"/dev/graphic"
#define FB0_DEVICE			"/dev/graphics/fb0"
#define COMPOSITE_DEVICE 	"/dev/composite"
#define COMPONENT_DEVICE 	"/dev/component"
#define VIQE_DEVICE			"/dev/viqe"

#define MAX_CAMERAS_SUPPORTED	2

//#define HDMI_OUTPUT_MODE_OFF		//enable : hdmi power off when start Camera app

//#define FEATURE_SENSOR_STABLE_DELAY

//#define USE_G2D_FOR_UVC

//#define FEATURE_WDMA_RGB_MODE // feature enable : HDMI IN MODULE RGB MODE, diable : HDMI IN MODULE YUV MODE

#if defined(FEATURE_WDMA_RGB_MODE)

#define FEATURE_RGB_MODE_565 // feature enable : RGB 565, disable : RGBA8888

#endif

typedef enum {
	QQXGA, QXGA, UXGA, 	SXGA, XGA, SVGA, VGA, QVGA, 	QCIF
} image_size;
//#define SENSOR_5M
//#define SENSOR_3M
//#define SENSOR_2M
//#define SENSOR_VGA
//#define SENSOR_TVP5150
//#define SENSOR_RDA5888

//#define ROTATE_CAPTURE_SUPPORT
#define DUAL_CAMERA_SUPPORT

//#define USE_OVERLAY
#if !(defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888))
//#define USE_CIFOUT_PMEM
//#define USE_CHANGE_SEMIPLANAR
#endif
#define USE_CIFOUT_GRALLOC

//for USE_USB_CAMERA
#define DEF_UVC_WIDTH           640 //640
#define DEF_UVC_HEIGHT          480 //480
#define DEF_UVC_MAX_FPS			30//25
#define DEF_UVC_HD_FPS			20

#define PA_PREVIEW_BASE_ADDR 			mUmpReservedPmap.base
#define PMEM_CAM_SIZE 					mUmpReservedPmap.size

#define PA_JPEG_HEADER_BASE_ADDR 		mJPEGHeaderPmap.base
#define JPEG_HEADER_MEM_SIZE 			mJPEGHeaderPmap.size

#define PA_JPEG_RAW_BASE_ADDR 			mJPEGRawPmap.base
#define JPEG_RAW_MEM_SIZE 				mJPEGRawPmap.size

#define PA_JPEG_STREAM_BASE_ADDR 		mJPEGStreamPmap.base
#define JPEG_STREAM_MEM_SIZE 			mJPEGStreamPmap.size

#define PA_CONVERT_TEMP_SIZE 			(1280 * 720 * 2)
#define PA_CONVERT_TEMP_ADDR 			(mExtCameraPmap.base + mExtCameraPmap.size - PA_CONVERT_TEMP_SIZE)

#if defined(_TCC9300_) || defined(_TCC8800_) || defined(_TCC8920_) || defined(_TCC8930_)
#define VPU_HW_DECODE
#define MAKE_THUMB_JPEG
#else
#define MAKE_THUMB_JPEG
#endif

#ifdef  MAKE_THUMB_JPEG
#define THUMB_WIDTH 	320
#define THUMB_HEIGHT 	240
#define FEATURE_NOT_DECODE_JPEG_FOR_THUMB
#endif

#if defined(USE_CIFOUT_GRALLOC)
#include <ui/GraphicBufferMapper.h>
#include <ui/GraphicBuffer.h>
#include <hardware/gralloc.h>
#endif
#if defined(USE_CIFOUT_PMEM)
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <binder/MemoryHeapPmem.h>
#if HAVE_ANDROID_OS
	#include <linux/android_pmem.h>
#endif
#endif

#ifdef VPU_HW_DECODE
extern "C" {
//#include <mach/TCC_VPU_CODEC.h> // VPU video codec
#include "vpudec.h"
#include "jpu_jpeg.h"
}
#endif

#if defined(CONFIG_VIDEO_HDMI_IN_SENSOR)
#define NUM_OF_V2IP_PREVIEW_BUFFERS		8
#define NUM_OF_PREVIEW_BUFFERS 			6
#define NUM_OF_CAPTURE_BUFFERS 			3

#define NUM_VIDBUF 							NUM_OF_PREVIEW_BUFFERS // 10
#define NUM_CAPTURE_RAW 					1
#define CAP_RETRY_COUNT 					4
#else
#define NUM_OF_V2IP_PREVIEW_BUFFERS		8
#define NUM_OF_PREVIEW_BUFFERS 			12
#define NUM_OF_CAPTURE_BUFFERS 			3

#define NUM_VIDBUF 							NUM_OF_PREVIEW_BUFFERS // 10
#define NUM_CAPTURE_RAW 					1
#define CAP_RETRY_COUNT 					4

#endif

#define ERR_GET_BUFFER 						100
#define ERR_BUFFER_PROC 					101
#define NO_SIGNAL							102

#define ALIGNED_BUFF(buf, mul) ( ( (unsigned int)buf + (mul-1) ) & ~(mul-1) ) // mulÀÇ ¹è¼ö

typedef enum{
	MODE_IDLE = 0,
	MODE_PREVIEW,
	MODE_PREVIEW_STOP,
	MODE_CAPTURE,
	MODE_CAPTURED
}camera_mode;

typedef enum{
	MODE_RGB565 = 0,
	MODE_YUV422,
	MODE_YUV420,
	MODE_RGBA8888
}preview_format;

typedef enum{
	PROC_AF = 0,
	PROC_MAX
}proc_type;

typedef struct {
	unsigned int				width;
	unsigned int				height;

	unsigned char				jpeg_quality;
	unsigned char				jpeg_capture;
	
	TCCXXX_JPEG_ENC_DATA 		jpeg_save_info;
	TCCXXX_JPEG_DEC_DATA 		jpeg_dec_info;
}CaptureInfo;

typedef struct {
	int 						fd;	
	int							jpegenc_fd;
	int							jpegdec_fd;
	int							convert_fd;
	int							g2d_fd;
	unsigned int				preview_width;
	unsigned int				preview_height;
	camera_mode					cam_mode;
	
	struct v4l2_capability		vid_cap;
	struct v4l2_format			vid_fmt;
	struct v4l2_streamparm		vid_parm;
	struct v4l2_requestbuffers	vid_buf;
	int							Buffer_cnt;
	int							release_idx;
	
	int							preview_idx;
#ifndef USE_CIFOUT_PMEM
	unsigned int				buffers_virt[NUM_VIDBUF];
	unsigned int				buffers_phy[NUM_VIDBUF];
	unsigned int				buffers_len;

#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	unsigned int				rec_buffers_virt[NUM_VIDBUF];
	unsigned int				rec_buffers_phy[NUM_VIDBUF];
	unsigned int				rec_buffers_len;
#endif

#endif	
	int							needYuv420Cvt;
	unsigned int				buffers_phycvt[NUM_VIDBUF];
	unsigned int				buffers_lencvt;

	preview_format				preview_fmt; //preview format

	unsigned int				mUvc_MaxWidth;
	unsigned int				mUvc_MaxHeight;
	
	CaptureInfo					cap_info;
} CameraDevice;

typedef struct {
	int degrees;
	int minutes;
	int seconds;
} GPS_Latitude_Type;

typedef struct {
	int degrees;
	int minutes;
	int seconds;
} GPS_Longitude_Type;

typedef struct {
	GPS_Latitude_Type 	Latitude;
	GPS_Longitude_Type 	Longitude;
	int 				Altitude;
	unsigned int 		Timestamp;
	const char 		 	*Processing_Method;
} GPS_Info_Type;


namespace android {

class TCC_V4l2_Camera {
public:
	TCC_V4l2_Camera(int nCamIndex);
	~TCC_V4l2_Camera();

	int startPreview(const char* format, int *fps);
	int stopPreview();
	int capture(uint32_t *size, uint16_t width, uint16_t height, int nRotationOfCapture, bool bIsitPortraitLCD);
	int release_buffer(unsigned char rel_index);
	int get_bufferCount();
	int get_bufferIndex(unsigned int phy_addr);
	
	void setParameter(int type, int value);	
	int setOperation(int type, int value);
	void set_fps(int fps);
	
	void setPreviewSize(unsigned int width, unsigned int height, unsigned char landscape);
	void getPreviewSize(unsigned int *width, unsigned int *height);	
	int getCapturedJpegData(uint8_t *buffer);
	int getCapturedRawData(uint8_t *buffer);
	int getPreviewRawData(uint8_t *buffer, uint8_t prv_idx);
	bool get_cameraType_isUVC();
	int get_default_framerate(int*fps);
	void setJPEGQuality(int quality);
	void copyToVideoSnapshotFrame(unsigned int index);
	unsigned int  camif_encode_jpeg();
	int change_uv_data(unsigned int srcY, unsigned int dest);
#if(1) // 20131018 swhwang, for check video frame interrupt
	int camif_reopen_device(int nCamIndex); 
	int camif_Intrrupt_check();
#endif
#if defined(USE_CIFOUT_GRALLOC)
	void get_currPreviewFrameIndex(int *frame_index, int *error, bool skip);
	void camif_setCameraAddress(int index, int cameraStatus);
#endif

#ifdef USE_CIFOUT_PMEM
	int create_pmem();
	int destopy_pmem();

	sp<MemoryHeapBase> get_previewHeap();
	sp<MemoryBase> get_currPreviewFrame(int frame_idx);	
	unsigned int get_currPreviewPhyAddr(int frame_idx);
	sp<MemoryBase>  get_nextRecFrame(int *frame_idx, int *iError, bool skip);
#ifdef USE_CHANGE_SEMIPLANAR	
	sp<MemoryBase> get_currCallbackFrame(int frame_idx);	
	unsigned int get_currCallbackPhyAddr(int frame_idx);
#endif	
#else
	unsigned int  get_nextPhyFrame(int *frame_idx, bool skip);

#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	unsigned int get_nextRecVirtFrame(int frame_idx);
	unsigned int  get_nextRecPhyFrame(uint8_t *buffer, int frame_idx);
	int get_RecbufferIndex(unsigned int phy_addr);
#endif

#endif
	int* get_pmem_virtual_addr(int buffer_type, int bufIdx);
	int* get_pmem_physical_addr(int buffer_type, int bufIdx);
	unsigned int get_pmem_buffer_offset(int buffer_type, int bufIdx);

	#if (0)
	int dump(int fd, const Vector<String16>& args);
	#endif

	int mRotation;
	bool supported_thumbnail;
	
	bool 		supported_gps;
	GPS_Info_Type GPSInfo;

	int 			sensor_max_resolution;
	bool			mbRotateCapture;
	bool 			mZoomSupport;
	char 			mJpegQuality;
#if defined(USE_CIFOUT_GRALLOC)
	buffer_handle_t** mBufferHandle;
	buffer_handle_t** mGrallocHandle;

	int mPreviewBufferCount;
	int mCaptureBufferCount;
	int mPreviewWidth;
	int mPreviewHeight;
	bool mPreviewSizeChanged;
	void* mCameraBufferAddr[NUM_OF_PREVIEW_BUFFERS];
	int mRealKCount;
	bool mIsRunningV2IP;
	bool mNoDisplay;
#endif
	bool mVideoSnapshot;
	bool mDoneVideoSnapshot;
	unsigned int mCapture_fmt;

#if defined(FEATURE_SENSOR_STABLE_DELAY)
	bool mSensor_StableTime;
#endif
private:
	int camif_open_device(int nCamIndex);
	int camif_close_device();	
	int camif_init_format (int width, int height);
	int camif_init_buffer();
	void camif_uninit_buffer();
	void  camif_init_data();
	int camif_get_dev_info ();
	int camif_set_resolution (int width, int height);
	int camif_start_stream(int *fps);
	int camif_stop_stream();
	int  camif_capture(int nRotationOfCapture, bool bIsitPortraitLCD);
	unsigned int camif_get_frame(uint32_t *used_bytes);
	void camif_set_queryctrl(unsigned int ctrl_id, int value);
	unsigned int get_currUvcYuv(uint32_t frame_len, unsigned int buffer_index);
	int get_currUvcJpeg(uint32_t frame_len, int buffer_index);
#ifdef VPU_HW_DECODE
	int vjpeg_decode_operation(unsigned int src_phyAddr, unsigned int src_virtAddr, unsigned int src_length, 
										unsigned int src_width, unsigned int src_height,
										unsigned int *dec_width, unsigned int *dec_height,
										unsigned int *dec_padWidth, unsigned int *dec_padHeight, 
										unsigned int *dec_format, unsigned char *dest_buff_info, 
										int ratio, int iOpCode);
#endif

	enum CameraPmemBufferType {
			BUFFER_PREVIEW,
			BUFFER_RECORD,
			BUFFER_UVC,
			BUFFER_MAX
	};

#ifdef USE_CIFOUT_PMEM
	sp<MemoryHeapPmem>  		mPreviewHeap;
	sp<MemoryBase>      		pmem_buffers[BUFFER_MAX][NUM_VIDBUF];
#else
	unsigned int      			pmem_buffers[BUFFER_MAX][NUM_VIDBUF];
#endif
	unsigned int				pmem_buffers_offset[BUFFER_MAX][NUM_VIDBUF];
	unsigned int				*pmem_mmaped;
#if defined(USE_CIFOUT_GRALLOC)
	unsigned int				buffer_manager[NUM_VIDBUF];
	unsigned int				buffer_manager_in_index;
	unsigned int				buffer_manager_out_index;
#endif

	bool 						mUvc_camera;
	int							mUvc_capture_buffers;
	int							mDef_MaxFps;
	int							mUvc_HDFps;
	unsigned int				mPreview_fmt;
	unsigned int				mUVC_fmt;
	CameraDevice 				mCamDevice;

	pmap_t mUmpReservedPmap;
	pmap_t mJPEGHeaderPmap;
	pmap_t mJPEGRawPmap;
	pmap_t mJPEGStreamPmap;
	pmap_t mExtCameraPmap;
	
#ifdef VPU_HW_DECODE
	enum VpuState {
		VPU_INITED,
		VPU_SEQHEADER_DONE,
		VPU_CLOSED
	};

	unsigned int mVpuState;
	vdec_input_t gsVDecInput;
	vdec_output_t gsVDecOutput;
	vdec_init_t gsVDecInit;
	vdec_user_info_t gsVDecUserInfo;
	conf_enc_t		gsJPUEncodeInfo;
	conf_dec_t		gsJPUDecodeInfo;
#endif	

	// virtual jpeg address.
	unsigned char *mJpegRawAddr;
	unsigned char *mJpegHeaderAddr;
	unsigned char *mJpegStreamAddr;
	};

}; // namespace android

#endif // ANDROID_HARDWARE_TCCCAMERA_H

