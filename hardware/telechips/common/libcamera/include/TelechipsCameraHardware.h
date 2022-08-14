/*
**
** Copyright 2009, Telechips, Inc.
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

#ifndef ANDROID_TELECHIPS_CAMERA_HARDWARE_H
#define ANDROID_TELECHIPS_CAMERA_HARDWARE_H

//ZzaU :: TCC V4L2 Camera
//#define PV_CAMCORDER_TEST //to test pv encorder component!!

#include <utils/threads.h>

#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <libpmap/pmap.h>

#include <camera/CameraParameters.h>
#include <hardware/camera.h>

#include "MessageQueue.h"
#include "TCC_V4l2_Camera.h"


#define MAX_CAMERAS_SUPPORTED 			2
#define TCC_CAMHAL_PIXEL_FORMAT_NV12 	0x100
#define TCC_CAMHAL_GRALLOC_USAGE 		GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_EXTERNAL_DISP 	\
										| GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_PRIVATE_3
#define	FOCUS_DISABLE						0
#define	FOCUS_ENABLE						1
#define	CANCEL_FOCUS						2
#define	WAIT_FOCUS							3


struct str_map {
    const char *const desc;
    int val;
};

typedef struct {
	unsigned int sx;
	unsigned int sy;
	unsigned int width;
	unsigned int height;
	unsigned int transform;
} pos_info_t;

typedef struct {
	unsigned int sx;
	unsigned int sy;
	unsigned int srcW;
	unsigned int srcH;
	unsigned int dstW;
	unsigned int dstH;
} crop_info_t;

typedef struct {
	int lcdCtrlNo;
	int scalerCh;
	int onTheFly;

	unsigned int srcWidth;
	unsigned int srcHeight;
	unsigned int crop_top, crop_bottom, crop_left, crop_right;

	int deinterlaceM2M;
	int renderCnt;
	int OddFirst;
	int DI_use;

	unsigned int address[6];
	unsigned int dstAddr;
} viqe_arg_t;

typedef struct {
	int mode;
	int region;
	int strength1;
	int strength2;
	int modeparam;
} viqe_param_t;

typedef enum {
	SKIP_NO = 0,
	SKIP_HALF,   	// OXOXOXOXOXO
	SKIP_THIRD,  	// OXXOXXOXXOX
	SKIP_QUARTER, 	// OXXXOXXXOXX
	SKIP_FIFTH 		// OXXXxOXxXXO
} skip_mode;


namespace android {

// forward declarations
class TelechipsCameraHardware;

class TelechipsCameraHardware {
	private: // declare to private member function.
		void 								createPreviewThread();
		void 								destoryPreviewThread();
		status_t 							previewThread();

#if (1) // 20131018 swhwang, for check video frame interrupt
		status_t							checkThread();
		void						 		createCheckThread();
		void						 		destoryCheckThread();
#endif
		status_t 							call_startPreview();
		status_t 							call_stopPreview();
		status_t 							call_nextPreview();

		void 								initDefaultParameters();
		void 								initHeapLocked();

		static status_t 					beginAutoFocusThread(void *cookie);
		status_t 							autoFocusThread();
	
		static status_t 					beginPictureThread(void *cookie);
		status_t 							pictureThread();
		static status_t 					beginVideoSnapshotThread(void *cookie);
		status_t 							videoSnapshotThread();

		#if defined(USE_CHANGE_SEMIPLANAR)
		status_t change_uv_data(unsigned int srcY, uint8_t *dest);
		#endif

		uint8_t is_PixelFormat_Rgb565();


	private: // declare to private member variables.
		static const int 					kBufferCount = NUM_OF_PREVIEW_BUFFERS;

		mutable Mutex 						mLock;

		CameraParameters 					mParameters;

		sp<MemoryHeapBase> 					mPreviewHeap;
		sp<MemoryHeapBase> 					mRecHeap;
		sp<MemoryHeapBase> 					mRawHeap;
		sp<MemoryBase> 						mBuffers[kBufferCount];
		sp<MemoryBase> 						mRecBuffers[kBufferCount];

		// Index of current camera adapter
		int mCameraIndex;

		TCC_V4l2_Camera 					*mV4l2Camera;
		bool 								mPreviewRunning;
		int 								mPreviewFrameSize;
		int 								mPreviewRealFrameSize;
		int 								mCaptureRealFrameSize;

#if(1) // 20131018 swhwang, for check video frame interrupt
		class CheckThread : public Thread {
			TelechipsCameraHardware* mHardware;
			public:
				CheckThread(TelechipsCameraHardware* hw)
					: Thread(false), mHardware(hw) { }

				virtual bool threadLoop() {
					mHardware->checkThread();
					// loop until we need to quit
					return false;//true;
				}
		};
		  sp<CheckThread> mCheckThread;
#endif

		// protected by mLock
		class PreviewThread : public Thread {
			TelechipsCameraHardware* mHardware;
			public:
				PreviewThread(TelechipsCameraHardware* hw)
					: Thread(false), mHardware(hw) { }
		
				virtual bool threadLoop() {
					mHardware->previewThread();
					// loop until we need to quit
					return false;//true;
				}
		};
		sp<PreviewThread> 					mPreviewThread;

		camera_notify_callback 				mNotifyCb;
		camera_data_callback 				mDataCb;
		camera_data_timestamp_callback  	mDataCbTimestamp;
		camera_request_memory 				mRequestMemory;
		void 								*mCallbackCookie;

		int32_t 							mMsgEnabled;

		int 								mFb_fd;
		int 								mRt_fd;
		int 								mM2m_fd;
		int 								mM2m_fd1;
		int 								mComposite_fd;
		int 								mComponent_fd;
		int 								mViqe_fd;
	
		unsigned int 						mLcd_width;
		unsigned int 						mLcd_height;

		unsigned int 						mHdmi_width;
		unsigned int 						mHdmi_height;

		bool 								mHDMIOutput;
		bool 								mCompositeOutput;
		bool 								mComponentOutput;

		unsigned int 						iVideoDisplayWidth; // Source In!!
		unsigned int 						iVideoDisplayHeight;	
		unsigned int 						mDispWidth; 	// Scaler OUT!!
		unsigned int 						mDispHeight;
		unsigned int 						mFinal_DPWidth; // Crop OUT!!
		unsigned int 						mFinal_DPHeight;
		unsigned int 						mFinal_stride;
		unsigned int 						mRight_truncation;

		pos_info_t 							mOrigin_DispInfo;
		pos_info_t 							mTarget_DispInfo;
		crop_info_t 						mTarget_CropInfo;

		viqe_arg_t 							mViqe_arg;
		viqe_param_t 						mViqe_param;

		bool 								mFirst_Frame;
		bool 								mParallel;
		bool 								mOrder_Parallel;
		bool 								mNeed_scaler;
		bool 								mNeed_transform;
		bool 								mNeed_rotate;
		bool 								mNeed_crop;
		bool 								mRecordRunning;
		bool 								mRecordFrmRelease;

		unsigned long 						mRecordFrmSend_Total;
		unsigned long 						mRecordFrmSend_Last;
		int 								mPrevRelFrm_idx;

		SCALER_TYPE 						mScaler_arg;
		G2D_COMMON_TYPE 					mGrp_arg;
		SCALER_TYPE 						mScaler1_arg;

		// only used from PreviewThread
		int 								mCurrentPreviewFrame;

		// preview flickering problem when camera-mode is changed!!
		int 								mCamStatus;

		// Temporarily Skip frame after preview start!! for one second.
		bool 								mFrameSkip;
		int 								mMaxSkipframe_cnt;
		nsecs_t 							mCurSkipframe_cnt;
		nsecs_t 							mPreviewStart_ns;

		//to control fps
		unsigned char 						mFps_mode;
		unsigned char 						mFps_OnTime;

		//Precess camera.
		enum CameraThreadCmd {
			// Cmd
			CAMERA_INIT,
			CAMERA_START,
			CAMERA_STOP,
			CAMERA_PAUSE,  // 20131018 swhwang, for check video frame interrupt
			CAMERA_KILL,
			CAMERA_REC_START,
			CAMERA_REC_STOP,
			CAMERA_REC_SNAPSHOT,
			//Ack
			CAMERA_ACK,
			CAMERA_NACK,
		};

		MessageQueue 						*CameraCmdQ;
		MessageQueue 						*CameraAckQ; 

		Mutex 								mRecordingLock;

		#if defined(PV_CAMCORDER_TEST)
		sp<MemoryHeapBase> 					mRecordHeap;
		sp<MemoryBase> 						mRecordBuffers[kBufferCount];
		int 								mCurrentRecordFrame;
		#endif

		// to improve performance when specific operation need preview-callback as like linphone.
		// It will only give the physical address that point to real frame from camera.
		bool 								mCvtCallback;

		int 								mNormalFps;
#if defined(USE_CIFOUT_GRALLOC)
		int									mSaveOutputMode;
#endif


	public: // declare to public member function.
		TelechipsCameraHardware(int cameraId);
		~TelechipsCameraHardware();

		sp<IMemoryHeap> 					getPreviewHeap() const;
		sp<IMemoryHeap> 					getRawHeap() const;

		void								enableMsgType(int32_t msgType);
		void								disableMsgType(int32_t msgType);
		bool								msgTypeEnabled(int32_t msgType);

		status_t							startPreview();
		void								stopPreview();
		bool								previewEnabled();

#if(1) // 20131018 swhwang, for check video frame interrupt
		status_t							resume();
		void								pause();
#endif

		status_t							startRecording();
		void								stopRecording();
		bool								recordingEnabled();
		void								releaseRecordingFrame(const void* mem);

		status_t							autoFocus();
		status_t							cancelAutoFocus();

		status_t							takePicture();
		status_t							cancelPicture();

		status_t							dump(int fd) const;

		int 								getParm(const char *parm_str, const struct str_map *const parm_map);
		char*								getParameters();

		void								setParm();
		status_t							setParameters(const CameraParameters& params);
		status_t 							setParameters(const char* parameters);

		void								putParameters(char *parms);

		status_t							sendCommand(int32_t command, int32_t arg1, int32_t arg2);

		void								release();

		status_t 							setPreviewWindow(struct preview_stream_ops *window);

		void								setCallbacks(camera_notify_callback notify_cb, 					\
															camera_data_callback data_cb, 						\
															camera_data_timestamp_callback data_cb_timestamp, 	\
															camera_request_memory get_memory, 					\
															void* user);

		status_t 							storeMetaDataInBuffers(int enable);

		status_t 							allocPreviewBuffers(int width, int height, const char* format, \
                                        							int buffercount);
		status_t 							allocCaptureBuffers(int width, int height, const char* format, \
																	int buffercount);
		void* 								allocateBuffer(int width, int height, const char* format, \
															 int &bytes, int numBufs);
		void 								lockCurrentPreviewBuffer(int index);
		void* 								freeCameraBuffers(unsigned char cameraStatus);


	public: // declare to public member variables.
		static const uint32_t 				VFR_SCALE = 1000;

		int 								mPreviewLength;
		int 								mCaptureLength;
		preview_stream_ops_t 				*mCameraWindow;
		unsigned int 						*mPreviewBuffers;
		unsigned int 						*mCaptureBuffers;
		bool 								mCreateCameraWindow;
		camera_memory_t *                   enc_metadata_buffer[kBufferCount];
};

}; // namespace android

#endif



