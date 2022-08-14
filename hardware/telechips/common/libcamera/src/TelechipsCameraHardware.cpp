/*
**
** ** Copyright 2009, Telechips, Inc.
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

#define LOG_TAG	"TCC_CAMERA_HAL"

#include <math.h>
#include <utils/Log.h>
#include <utils/threads.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <camera/CameraParameters.h>
#include <autoconf.h> 

#include "TelechipsCameraHardware.h"
#include "EncoderMetadata.h"


#if HAVE_ANDROID_OS
#include <linux/android_pmem.h>
#endif

//#define TEST_CAPTURE //send fake jpeg file without capture!!
#ifdef TEST_CAPTURE
#include <test_jpeg.h>
#endif

static int DEBUG	   = 0;
#define DBUG(msg...)	if (DEBUG) { ALOGD( " " msg); }
#define FUNCTION_IN     DBUG("%d: %s() In", __LINE__, __FUNCTION__);
#define FUNCTION_OUT    DBUG("%d: %s() Out", __LINE__, __FUNCTION__);

#define FEATURE_DESTROY_PREVIEW_THREAD



int FOCUS_MSG =	FOCUS_ENABLE;
bool check_threadloop; // 20131018 swhwang, for check video frame interrupt

/*******************************************************************
 * If Camera problem comes up about function of Zoom, Do register Camera sensor feature.
 *******************************************************************/
 
#if defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9P111)

#define SENSOR_5M_ENABLE
	
#endif

//******************************************************************/
//******************************************************************/

#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	#define	TVP5150_SCALE_TOP_OFFSET		12
	
	#define	TVP5150_WIDTH					640
	#define	TVP5150_HEIGHT				    240
#endif

#define IGNORE_RATIO_ON_OUTPUT_DISPLAY //Ignore display ratio on lcd.
//#define FRAMESKIP_NS				660000000 //660 ms
#define FRAMESKIP_NS				0
//#define DEBUG_FPS

enum {
    CAMERA_ORIENTATION_UNKNOWN = 0,
    CAMERA_ORIENTATION_PORTRAIT = 1,
    CAMERA_ORIENTATION_LANDSCAPE = 2,
};

enum {
    CAM_STATUS_INIT = 0,
    CAM_STATUS_PREVIEW_START = 1,
    CAM_STATUS_PREVIEW_STOP = 2,
	CAM_STATUS_CAPTURE = 3,
	CAM_STATUS_CAPTURE_COMPLETE = 4,
    CAM_STATUS_RECORD = 5,
    CAM_STATUS_PAUSE = 6,
};

/*******************************************************************
 * implementation of camera_module functions
 *******************************************************************/

/* open device handle to one of the cameras
 *
 * assume camera service will keep singleton of each camera
 * so this function will always only be called once per camera instance
 */

static int attr_lookup(const struct str_map *const arr, const char *name)
{
    if(name) {
        const struct str_map *trav = arr;
        while(trav->desc) {
            if(!strcmp(trav->desc, name))
                return trav->val;

            trav++;
        }
    }

    return -1;
}

#define INIT_VALUES_FOR(parm) do {                               \
    if (!parm##_values) {                                        \
        parm##_values = (char *)malloc(sizeof(parm)/             \
                                       sizeof(parm[0])*30);      \
        char *ptr = parm##_values;                               \
        const str_map *trav;                                     \
        for (trav = parm; trav->desc; trav++) {                  \
            int len = strlen(trav->desc);                        \
            strcpy(ptr, trav->desc);                             \
            ptr += len;                                          \
            *ptr++ = ',';                                        \
        }                                                        \
        *--ptr = 0;                                              \
    }                                                            \
} while(0)

char framerate_values[20] = {0, };

static const str_map whitebalance[] = {
    { "auto",         	0 },
    { "daylight", 		1 },
    { "incandescent",   2 },
    { "fluorescent",    3 },
    { "cloudy-daylight",4 },
    { "shade",     		5 },
    { NULL, 0 }
};
static char *whitebalance_values;

static const str_map effect[] = {
    { "none",       0 },
    { "mono",       1 },
    { "negative",   2 },
    { "sepia",      3 },
    { "aqua",  		4 },
    { "posterize", 	5 },
    { NULL, 0 }
};
static char *effect_values;

static const str_map autofocus[] = {
    { "auto",       0 },
    { NULL, 0 }
};
static char *autofocus_values;

static const str_map flash[] = {
    { "off",       0 },
    { NULL, 0 }
};
static char *flash_values;


namespace android {

TelechipsCameraHardware::TelechipsCameraHardware(int cameraId)
                  : mParameters(),
                    mPreviewHeap(NULL),
                    mRawHeap(NULL),
#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
                    mRecHeap(NULL),
#endif
                    mV4l2Camera(0),
                    mPreviewRunning(0),
                    mPreviewFrameSize(0),
                    mPreviewRealFrameSize(0),
                    mCaptureRealFrameSize(0),
                    mNotifyCb(0),
                    mDataCb(0),
                    mDataCbTimestamp(0),
                    mCallbackCookie(0),
                    mMsgEnabled(0),
                    mCurrentPreviewFrame(0),
                    mRecordRunning(false),
					mRecordFrmSend_Total(0),
					mRecordFrmSend_Last(0),
					mRecordFrmRelease(false),
					mCamStatus(0),

					#if defined(USE_CIFOUT_GRALLOC)
					mCameraWindow(NULL),
					mPreviewBuffers(NULL),
					mCaptureBuffers(NULL),
					mPreviewLength(0),
					mCaptureLength(0),
					mCreateCameraWindow(0)
					#endif
{
	struct fb_var_screeninfo info;
	char value[PROPERTY_VALUE_MAX];
	unsigned int	uiOutputMode = 0;
	int iSupport_Hdmi;

    mCameraIndex = cameraId;
	
	if(mV4l2Camera != NULL)
		delete mV4l2Camera;
	
	mV4l2Camera = new TCC_V4l2_Camera(mCameraIndex);

	whitebalance_values = NULL;
	effect_values = NULL;
	autofocus_values = NULL;
	flash_values = NULL;

	memset(&mScaler_arg, 0x00, sizeof(mScaler_arg));
	#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	memset(&mScaler1_arg, 0x00, sizeof(mScaler_arg));
	#endif
	memset(&mGrp_arg, 0x00, sizeof(mGrp_arg));

	CameraCmdQ = CameraAckQ = NULL;
	mPreviewThread = NULL;
	createPreviewThread();
	createCheckThread(); // 20131018 swhwang, for check video frame interrupt

	mFirst_Frame = mParallel = mOrder_Parallel = false;
	mNeed_scaler = mNeed_transform = mNeed_rotate = mNeed_crop = false;

	mRt_fd = mM2m_fd = mFb_fd = mComposite_fd = mComponent_fd = -1;
	#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	mM2m_fd1 = -1;
	#endif
	mHDMIOutput = false;
	mCompositeOutput = false;
	mComponentOutput = false;
	mHdmi_width = mHdmi_height = 0;
	
	mFb_fd = open(FB0_DEVICE, O_RDWR);	
	if(mFb_fd <= 0) {
		ALOGE("can't open[%s] '%s'", strerror(errno), FB0_DEVICE);
	}
	
	ioctl(mFb_fd, FBIOGET_VSCREENINFO, &info);
	mLcd_width = info.xres;
	mLcd_height = info.yres;

	for(int i=0; i < mV4l2Camera->mRealKCount; i++) {
		mBuffers[i] = NULL;
	}

	#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	for(int i=0; i < mV4l2Camera->mRealKCount; i++) {
		mRecBuffers[i] = NULL;
	}
	#endif

	// check HDMI mode for camera by system property.  
	property_get("tcc.all.output.support.camera", value, "");
	if(iSupport_Hdmi = atoi(value)) {
		property_get("persist.sys.output_mode", value, "");
		mSaveOutputMode = uiOutputMode = atoi(value);
	
		#if defined(TCC_HDMI_UI_SIZE_1280_720)
		if(uiOutputMode == OUTPUT_HDMI || uiOutputMode == OUTPUT_NONE)
		#else
		if(uiOutputMode == OUTPUT_HDMI)
		#endif
		{
			ALOGV("HDMI output enabled");
			mHDMIOutput = true;
		}

		if(uiOutputMode == OUTPUT_COMPOSITE) {
			// COMPOSITE Device Open!!
			mComposite_fd = open(COMPOSITE_DEVICE, O_RDWR);
			if (mComposite_fd <= 0) {
				ALOGE("can't open[%s] '%s'", strerror(errno), COMPOSITE_DEVICE);
			}
			mCompositeOutput = true;
		} else if (uiOutputMode == OUTPUT_COMPONENT) {
			// COMPOSITE Device Open!!
			mComponent_fd = open(COMPONENT_DEVICE, O_RDWR);
			if (mComponent_fd <= 0) {
				ALOGE("can't open[%s] '%s'", strerror(errno), COMPONENT_DEVICE);
			}
			mComponentOutput = true;
		}

		if(mHDMIOutput || mCompositeOutput || mComponentOutput) {
			TCC_HDMI_M hdmi_video;
			hdmi_video = TCC_HDMI_VIDEO_START;
			if(ioctl( mFb_fd, TCC_LCDC_HDMI_MODE_SET, &hdmi_video) != 0) {
				ALOGE("Fb0 HDMI ctrl Error!");
			}	
		}
	}
	else {
		#if defined(USE_CIFOUT_GRALLOC)
			#if	defined(HDMI_OUTPUT_MODE_OFF)
			property_get("persist.sys.output_mode", value, "");
			mSaveOutputMode = atoi(value);

			property_set("persist.sys.output_mode", "0");
			#endif
		#else		
			unsigned int uiHdmi = TCC_HDMI_SUSEPNED;
			if(ioctl( mFb_fd, TCC_LCDC_HDMI_MODE_SET, &uiHdmi) != 0) {
				ALOGE("Fb0 HDMI ctrl Error!");
			}
		#endif
	}

	{
		property_get("tcc.camera.change.previewcb", value, "");
		mCvtCallback = atoi(value);		
		ALOGD("mCvtCallback = %d", mCvtCallback);
	}
	initDefaultParameters();
}

void TelechipsCameraHardware::createPreviewThread()
{
	FUNCTION_IN

	Mutex::Autolock lock(mLock);

	if(mPreviewThread != NULL) {
		ALOGE("Preview Thread is already activated.");
		return;
	}

	CameraCmdQ = new MessageQueue();
	CameraAckQ = new MessageQueue();

	mPreviewThread = new PreviewThread(this);
	mPreviewThread->run("CameraPreviewThread", PRIORITY_URGENT_DISPLAY);

	FUNCTION_OUT
}

void TelechipsCameraHardware::destoryPreviewThread()
{
	FUNCTION_IN
	sp<PreviewThread> previewThread;

	if(mPreviewThread == NULL) {
		ALOGE("Preview Thread is not activated.");
		return;
	}

	Message msg;
	msg.command = CAMERA_KILL;
	CameraCmdQ->put(&msg);
	CameraAckQ->get(&msg);

	{ // scope for the lock
		Mutex::Autolock lock(mLock);
		previewThread = mPreviewThread;
	}

	// don't hold the lock while waiting for the thread to quit
	if(previewThread != 0) {
		previewThread->requestExitAndWait();
	}

	{ // scope for the lock
		Mutex::Autolock lock(mLock);
		mPreviewThread.clear();
	}

	delete CameraCmdQ;
	delete CameraAckQ;

	CameraCmdQ = CameraAckQ = NULL;

	FUNCTION_OUT
}

#if (1) // 20131018 swhwang, for check video frame interrupt
void TelechipsCameraHardware::createCheckThread()
{

	Mutex::Autolock autoLock(mLock);

	if(mCheckThread != NULL) {
		ALOGE("HW Check Thread is already activated.");
		return;
	}

	mCheckThread = new CheckThread(this);
	mCheckThread->run("CheckThread", PRIORITY_URGENT_DISPLAY);

	DBUG("%s: createCheckThread and run",__func__);

}

void TelechipsCameraHardware::destoryCheckThread()
{
	sp<CheckThread> checkThread;
	check_threadloop = false;

	if(mCheckThread == NULL) {
		ALOGE("Thread is not activated.");
		return;
	}
	
	DBUG("%s ",__func__);

	{ // scope for the lock
		Mutex::Autolock lock(mLock);
		checkThread = mCheckThread;
	}

	// don't hold the lock while waiting for the thread to quit
	if(checkThread != 0) {
		checkThread->requestExitAndWait();
	}

	{ // scope for the lock
		Mutex::Autolock lock(mLock);
		mCheckThread.clear();
	}
}
status_t TelechipsCameraHardware::checkThread()
{
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	check_threadloop= true;
	Message msg;
	int ret = -1;

	while(check_threadloop) {
		ret = v4L2Camera->camif_Intrrupt_check();
	
		if (ret == 0){
			if(mCamStatus == CAM_STATUS_PREVIEW_START){
				ALOGE("plug out video source!");
				pause();
				v4L2Camera->camif_reopen_device(mCameraIndex);
				if(mPreviewThread==NULL) createPreviewThread();
				resume();
			}
		}
		usleep(500*1000);
	}
	return NO_ERROR;
}
#endif
void TelechipsCameraHardware::initDefaultParameters()
{
	FUNCTION_IN
	CameraParameters p;
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;

    v4L2Camera->get_default_framerate(&mNormalFps);
	if(v4L2Camera->get_cameraType_isUVC()) {
		DBUG("USB Camera Detect...Start USB Camera Parameters setting...");
		p.setPreviewSize(DEF_UVC_WIDTH, DEF_UVC_HEIGHT);
		p.setPictureSize(DEF_UVC_WIDTH, DEF_UVC_HEIGHT);
		p.set("zoom-supported", "false");
		p.set("picture-size-values", "1280x720,640x480,320x240");
		p.setUsbCameraSupported(1); 								/* 20130429 swhwang, added for Support USB Camera Frame */
		p.set("preview-size-values", "1280x720,800x480,720x480,640x480,576x432,480x320,400x240,384x288,352x288,320x240,272x272,240x240,240x160,176x144,160x120");
	} else {
	#if defined (CONFIG_VIDEO_HDMI_IN_SENSOR)
		DBUG("HDMI Receiver Detect...Start HDMI Receiver Parameters setting...");
		p.setUsbCameraSupported(0);
		p.set("zoom", "0");
		p.set("smooth-zoom-supported", "false");
		p.set("zoom-supported", "false");
		p.set("zoom-ratios", "100,120,140,160,180,200,220,240,260,280,300,320,340,360,380,400");
		p.set("max-zoom", "15");
		mMaxSkipframe_cnt = 20;
		switch(v4L2Camera->sensor_max_resolution) {

		case SIZE_640_480P:
		case SIZE_640_480I:	
		p.setPreviewSize(640, 480);
		p.setPictureSize(640, 480);
		break;

		// 1280 x 720
		case SIZE_1280_720P:
		case SIZE_1280_720I:			
		p.setPreviewSize(1280, 720);
		p.setPictureSize(1280, 720);
		break;

		// 1920 x 1080
		case SIZE_1920_1080P:
		case SIZE_1920_1080I:			
		p.setPreviewSize(1920, 1080);
		p.setPictureSize(1920, 1080);
		break;

		// 720 x 480
		case SIZE_720_480P:
		case SIZE_720_480I:			
		p.setPreviewSize(720, 480);
		p.setPictureSize(720, 480);
		break;

		// 720 x 576
		case SIZE_720_576P:
		case SIZE_720_576I:			
		p.setPreviewSize(720, 576);
		p.setPictureSize(720, 576);
		break;

		//1440 x 480
		case SIZE_1440_480P:
		case SIZE_1440_480I:			
		p.setPreviewSize(1440, 480);
		p.setPictureSize(1440, 480);
		break;

		// 1440 x 240
		case SIZE_1440_240P:			
		case SIZE_1440_240I:				
		p.setPreviewSize(1440, 240);
		p.setPictureSize(1440, 240);
		break;

		// 1440 x 576
		case SIZE_1440_576P:	
		case SIZE_1440_576I:			
		p.setPreviewSize(1440, 576);
		p.setPictureSize(1440, 576);
		break;

		// 1440 x 288
		case SIZE_1440_288P:			
		case SIZE_1440_288I:			
		p.setPreviewSize(1440, 288);
		p.setPictureSize(1440, 288);
		break;

		// 2880 x 480
		case SIZE_2880_480P: 
		case SIZE_2880_480I:			
		p.setPreviewSize(2880, 480);
		p.setPictureSize(2880, 480);
		break;

		// 2880 x 240
		case SIZE_2880_240P: 
		case SIZE_2880_240I:			
		p.setPreviewSize(2880, 240);
		p.setPictureSize(2880, 240);
		break;

		// 2880 x 576
		case SIZE_2880_576P:
		case SIZE_2880_576I:			
		p.setPreviewSize(2880, 576);
		p.setPictureSize(2880, 576);
		break;

		default :
		p.setPreviewSize(1920, 1080);
		p.setPictureSize(1920, 1080);

		}
		p.set("preview-size-values", "1920x1080,1280x720,800x480,720x480,640x480,576x432,480x320,400x240,384x288,352x288,320x240,272x272,240x240,240x160,176x144,160x120"); 
		p.set("picture-size-values", "1920x1080,1280x720,800x480,720x480,640x480,576x432,480x320,400x240,384x288,352x288,320x240,272x272,240x240,240x160,176x144,160x120");
	#else
		p.setUsbCameraSupported(0);
		p.setPreviewSize(640, 480);
		//p.setVideoSize(640,480);
		p.set("zoom", "0");
		p.set("smooth-zoom-supported", "false");
		if(v4L2Camera->mZoomSupport) 	p.set("zoom-supported", "true");
		else 						p.set("zoom-supported", "false");
		mMaxSkipframe_cnt = 20;

		switch(v4L2Camera->sensor_max_resolution) {
			case QQXGA: 	// 5m sensor
				p.setPreviewSize(1280, 960);
				p.setPictureSize(2560, 1920);
				p.set("picture-size-values", "2560x1920,2048x1536,1600x1200,1024x768");
				break;

			case QXGA: 		// 3m sensor
				p.setPictureSize(2048, 1536);
				p.set("picture-size-values", "2048x1536,1600x1200,1024x768");				
				//p.setPictureSize(1024,768);
				//p.set("piccture-size-values","1024x768");
				break;

			case UXGA: 		// 2m sensor
				p.setPictureSize(1600, 1200);
				p.set("picture-size-values", "1600x1200,1024x768");
				break;

			case SXGA: 		// 1.3m sensor
				p.setPictureSize(1280, 1024);
				p.set("picture-size-values", "1280x1024,1024x768");
				break;

			case XGA: 		// analog tv
				p.setPreviewSize(720, 480);
				p.setPictureSize(1600, 1200);
				p.set("picture-size-values", "1600x1200,1024x768");
				break;

			case VGA: 		// vga sensor
				p.setPictureSize(640, 480);
				p.set("picture-size-values", "640x480,352x288,320x240,176x144");
				break;
		}

		#if defined(SENSOR_5M_ENABLE)
			p.set("zoom-ratios", "100,120,140,160,180,200");
			p.set("max-zoom", "5");
			DBUG("5M sensor zoom value");
		#else		
			p.set("zoom-ratios", "100,120,140,160,180,200,220,240,260,280,300,320,340,360,380,400");
			p.set("max-zoom", "15");
		#endif

		p.set("preview-size-values", "1280x720,800x480,720x480,640x480,576x432,480x320,400x240,384x288,352x288,320x240,272x272,240x240,240x160,176x144,160x120");
	#endif
	}

	//p.set("video-size-values", "1280x720,640x480,176x144");
	sprintf(framerate_values, "%d,%d,%d,%d,%d", mNormalFps, mNormalFps/2, mNormalFps/3, mNormalFps/4, mNormalFps/5); 
	p.set("preview-frame-rate-values", framerate_values);
	ALOGD("fps = %s", framerate_values);
	p.setPreviewFrameRate(mNormalFps);	
	mFps_mode = SKIP_NO;
	mFps_OnTime = 0;

    p.set("preview-fps-range", "5000,30000");
    p.set("preview-fps-range-values", "(5000,30000)");
    p.set("focus-distances", "0.049,0.05,0.051");

	INIT_VALUES_FOR(effect);
	p.set("effect", "none");
	p.set("effect-values", effect_values);

	INIT_VALUES_FOR(whitebalance);
	p.set("whitebalance", "auto");
	p.set("whitebalance-values", whitebalance_values);

	INIT_VALUES_FOR(autofocus);
	p.set("focus-mode", "auto");
	p.set("focus-mode-values", autofocus_values);

	INIT_VALUES_FOR(flash);
	p.set("flash-mode", "off");
	p.set("flash-mode-values", flash_values);

#if defined(CONFIG_VIDEO_HDMI_IN_SENSOR) && defined(FEATURE_WDMA_RGB_MODE)
	#if defined(FEATURE_RGB_MODE_565)
	    p.set("preview-format-values", "rgb565");  //RGB565
	    p.setPreviewFormat("rgb565");
	#else
		p.set("preview-format-values", "rgba8888");  //aRGB8888
		p.setPreviewFormat("rgba8888");
	#endif

#else
	#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
    p.set("preview-format-values", "yuv422p");
    p.setPreviewFormat("yuv422p");
    ALOGD("SENSOR_TVP5150 setPreviewFormat = yuv422p");
	#else // defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	p.set("preview-format-values", "yuv420sp,yuv420p");
	p.setPreviewFormat("yuv420sp");
	#endif // defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
#endif
	p.set("video-frame-format", "yuv420p");
    p.set("picture-format-values", "jpeg");
    p.setPictureFormat("jpeg");
	p.set("jpeg-quality", "100");

	p.set("focal-length", "2.8");
	p.set("horizontal-view-angle", "62.9");
	p.set("vertical-view-angle", "24.8");

	p.set("max-exposure-compensation", "20");
	p.set("min-exposure-compensation", "-20");
	p.set("exposure-compensation-step", "0.1");
	p.set("exposure-compensation", "0");

	p.set("jpeg-thumbnail-size-values", "320x240,0x0");
	p.set("jpeg-thumbnail-width", "320");
	p.set("jpeg-thumbnail-height", "240");
	p.set("jpeg-thumbnail-quality", "90");

	p.set("video-snapshot-supported", "false");

	p.set("auto-exposure-lock-supported", "false");
	p.set("auto-whitebalance-lock-supported", "false");
	p.set("max-num-metering-areas", "0");
	p.set("max-num-detected-faces-hw", "0");
	p.set("max-num-detected-faces-sw", "0");

	if(setParameters(p) != NO_ERROR) ALOGE("Failed to set default parameters?!");
	FUNCTION_OUT
}

void TelechipsCameraHardware::initHeapLocked()
{
	FUNCTION_IN
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	int how_big;

	// note that we enforce yuv422 in setParameters().
	int preview_width, preview_height;
	mParameters.getPreviewSize(&preview_width, &preview_height);
	DBUG("preview size = %d x %d.", preview_width, preview_height);

	if(strcmp(mParameters.getPreviewFormat(), "yuv420p") == 0)
		how_big = preview_width * preview_height * 3 / 2;
	else if(strcmp(mParameters.getPreviewFormat(), "rgba8888") == 0)
		how_big = preview_width * preview_height * 4;
	else 
		how_big = preview_width * preview_height * 2;


	// if we are being reinitialized to the same size as before, no work needs to be done.
	if(how_big == mPreviewRealFrameSize) return;

	mPreviewRealFrameSize = how_big;
	DBUG("preview size resetting = %d x %d.", preview_width, preview_height);

	mPreviewFrameSize = mPreviewRealFrameSize;
	v4L2Camera->setPreviewSize(preview_width, preview_height, false);

	FUNCTION_OUT
}

TelechipsCameraHardware::~TelechipsCameraHardware()
{
	FUNCTION_IN

    destoryPreviewThread();
	destoryCheckThread(); // 20131018 swhwang, for check video frame interrupt
	delete mV4l2Camera;
	mV4l2Camera = NULL;

	{

		#if defined(USE_CIFOUT_GRALLOC)

			#if defined(HDMI_OUTPUT_MODE_OFF)
			if(mSaveOutputMode)
				property_set("persist.sys.output_mode","1");
			else
				property_set("persist.sys.output_mode", "0");
			#endif
		#else
			unsigned int uiHdmi = TCC_HDMI_RESUME;

			if(ioctl( mFb_fd, TCC_LCDC_HDMI_MODE_SET, &uiHdmi) != 0) {
				ALOGE("Fb0 HDMI ctrl Error!");
			}
		#endif
	}

	if(mHDMIOutput || mCompositeOutput || mComponentOutput) {
		TCC_HDMI_M hdmi_video;
		struct tcc_lcdc_image_update hdmi_display;

		memset(&hdmi_display, 0x00, sizeof(hdmi_display));
		hdmi_display.Lcdc_layer = LCDC_LAYER_1;
		hdmi_display.enable = 0;
		ioctl(mFb_fd, TCC_LCDC_HDMI_DISPLAY, &hdmi_display);

		hdmi_video = TCC_HDMI_VIDEO_END;
		if(ioctl( mFb_fd, TCC_LCDC_HDMI_MODE_SET, &hdmi_video) != 0) {
			ALOGE("Fb0 HDMI ctrl Error!");
		}
	}

	#if !defined(TCC_HDMI_UI_SIZE_1280_720)
	if(mFb_fd != -1) {
		TCC_HDMI_M hdmi_pause;
		hdmi_pause = TCC_HDMI_RESUME;
		ioctl( mFb_fd, TCC_LCDC_HDMI_MODE_SET, &hdmi_pause);
	}
	#endif

	if(mFb_fd != -1) {
		close(mFb_fd);
		mFb_fd = -1;
	}

	if(mComposite_fd!= -1) {
		close(mComposite_fd);
		mComposite_fd = -1;
	}
	
	if(mComponent_fd!= -1) {
		close(mComponent_fd);
		mComponent_fd = -1;
	}

	if(mRawHeap != 0) {
		mRawHeap.clear();
		mRawHeap = NULL;
	}

	#if !defined(USE_CIFOUT_PMEM)
	if(mPreviewHeap != 0){
		for (int i = 0; i < mV4l2Camera->mRealKCount; i++) {
			if(mBuffers[i] != NULL)
				mBuffers[i].clear();
				mBuffers[i] = NULL;
		}
		mPreviewHeap.clear();
		mPreviewHeap = NULL;
	}
	#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	if(mRecHeap != 0) {
		for(int i=0; i < mV4l2Camera->mRealKCount; i++) {
			if(mRecBuffers[i] != NULL) {
				mRecBuffers[i].clear();
				mRecBuffers[i] = NULL;
			}
		}
		mRecHeap.clear();
		mRecHeap = NULL;
	}
	#endif // defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	#endif // USE_CIFOUT_PMEM

	if(whitebalance_values)
		free(whitebalance_values);

	if(effect_values)
		free(effect_values);

	if(autofocus_values)
		free(autofocus_values);

	if(flash_values)
		free(flash_values);

	FUNCTION_OUT
}

sp<IMemoryHeap> TelechipsCameraHardware::getPreviewHeap() const
{
	#if defined(USE_CIFOUT_PMEM)
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	return v4L2Camera->get_previewHeap();
	#else // USE_CIFOUT_PMEM
	return mPreviewHeap;
	#endif // USE_CIFOUT_PMEM
}

sp<IMemoryHeap> TelechipsCameraHardware::getRawHeap() const
{
	return mRawHeap;
}

void TelechipsCameraHardware::setCallbacks(camera_notify_callback notify_cb,
                                           camera_data_callback data_cb,
                                           camera_data_timestamp_callback data_cb_timestamp,
                                           camera_request_memory get_memory,
                                           void* user)
{
	Mutex::Autolock lock(mLock);
    mNotifyCb 			= notify_cb;
    mDataCb 			= data_cb;
    mDataCbTimestamp 	= data_cb_timestamp;
    mRequestMemory 		= get_memory;
    mCallbackCookie 	= user;
}

void TelechipsCameraHardware::enableMsgType(int32_t msgType)
{
	Mutex::Autolock lock(mLock);
	mMsgEnabled |= msgType;
}

void TelechipsCameraHardware::disableMsgType(int32_t msgType)
{
    Mutex::Autolock lock(mLock);
    mMsgEnabled &= ~msgType;
}

bool TelechipsCameraHardware::msgTypeEnabled(int32_t msgType)
{
    Mutex::Autolock lock(mLock);
    return (mMsgEnabled & msgType);
}

// ---------------------------------------------------------------------------

status_t TelechipsCameraHardware::call_startPreview()
{
	FUNCTION_IN;
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	int width, height, buffer_cnt, ret_fps, ret;

	mParameters.getPreviewSize(&width, &height);
	v4L2Camera->setPreviewSize(width, height, false);
	
	if(v4L2Camera->startPreview(mParameters.getPreviewFormat(), &ret_fps) < 0) {
		ALOGE("Start Preview ERROR!!");
		return INVALID_OPERATION;
	}

    if(v4L2Camera->get_cameraType_isUVC()) {
        if(mNormalFps != ret_fps) {
            ALOGI("UVC fps = %d", ret_fps);
            mNormalFps = ret_fps;
        }
    }

	mFirst_Frame = true;
	DBUG("TS set for FrameSkip start");
	mPreviewStart_ns = systemTime();
	DBUG("TS for FrameSkip = %u", mPreviewStart_ns);
	mFrameSkip = true;
	mCurSkipframe_cnt = 0;		

	if(mCamStatus != CAM_STATUS_PREVIEW_STOP) mCamStatus = CAM_STATUS_PREVIEW_START;

	FUNCTION_OUT;
	return NO_ERROR;
}

status_t TelechipsCameraHardware::call_stopPreview()
{
	FUNCTION_IN
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;

	v4L2Camera->stopPreview();
	mCamStatus = CAM_STATUS_PREVIEW_STOP;

	FUNCTION_OUT;
	return NO_ERROR;
}

#if defined(DEBUG_FPS)
static int skip_count = 0;
static void debugShowFPS()
{
	static int mFrameCount = 0;
	static int mLastFrameCount = 0;
	static nsecs_t mLastFpsTime = 0;
	static float mFps = 0;

	mFrameCount++;
	if(!(mFrameCount & 0x1F)) {
		nsecs_t now = systemTime();
		nsecs_t diff = now - mLastFpsTime;
		mFps = ((mFrameCount - mLastFrameCount) * float(s2ns(1))) / diff;
		mLastFpsTime = now;
		mLastFrameCount = mFrameCount;
		ALOGD("C: %d Frames, %f FPS (%f skipped)", mFrameCount, mFps, (skip_count* float(s2ns(1)))/diff);
		skip_count = 0;
	}
}
#endif // DEBUG_FPS

status_t TelechipsCameraHardware::call_nextPreview()
{
//	FUNCTION_IN;
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	int BufferCnt = v4L2Camera->get_bufferCount();
	int currentGetFrame = 0, Backupcurrentframe=0, error, ret;
	bool skip_frame;
#if defined(USE_CIFOUT_GRALLOC)
	GraphicBufferMapper &mapper = GraphicBufferMapper::get();
	camera_memory_t *preview_buffer = NULL;
	camera_frame_metadata_t *prv_metadata = NULL;
	enc_metadata_t *enc_metadata = NULL;
	sp<MemoryHeapBase> MetaDataHeap = NULL;
	void * temp;
#endif
	
#ifndef USE_CIFOUT_GRALLOC //for gingerbread
	if(!mRecordRunning && mRecordFrmSend_Total) {
		int remainFrmIndex = BufferCnt;
		while(remainFrmIndex) {
			DBUG("try frame(%d)-release !!", remainFrmIndex-1);
			v4L2Camera->release_buffer(remainFrmIndex-1);
			remainFrmIndex--;
		}
		mRecordFrmSend_Total = 0;
	}
#endif	
	//to display for preview!!
	skip_frame = false;

	if((mFps_mode != SKIP_NO) && (mFps_OnTime != 1)) skip_frame = true;
	
	#if defined(CONFIG_USB_VIDEO_CLASS)
	while(mRecordRunning && mRecordFrmSend_Total)		usleep(1000);
	#endif
	
	#if defined(USE_CIFOUT_GRALLOC)
	v4L2Camera->get_currPreviewFrameIndex(&currentGetFrame, &error, skip_frame);
	#else
	buffer = v4L2Camera->get_nextRecFrame(&currentGetFrame, &Error, skip_frame);
	#endif

#if defined(USE_CIFOUT_GRALLOC) //add for release record buffer after recording stop.
	if(!mRecordRunning && mRecordFrmSend_Total) {
		int remainFrmIndex = mRecordFrmSend_Total;
		int recordFrmIndex = currentGetFrame-1;
		while(remainFrmIndex) {
			DBUG("currentGetFram=%d, recordFrmIndex=%d", currentGetFrame, recordFrmIndex);
			//v4L2Camera->release_buffer(remainFrmIndex-1);
			DBUG("try frame(%d)-release !!", remainFrmIndex);
			v4L2Camera->release_buffer(recordFrmIndex);			
			remainFrmIndex--;
			recordFrmIndex--;
		}
		mRecordFrmSend_Total = 0;
	}
#endif

	if(currentGetFrame > v4L2Camera->mPreviewBufferCount) {
		if(error == ERR_BUFFER_PROC) {
			if(mFps_OnTime > 1) {
				mFps_OnTime--;
				#if defined(DEBUG_FPS)
				skip_count++;
				#endif
			}
			v4L2Camera->release_buffer(currentGetFrame);
			
		} else {
			usleep(15*1000); // 15ms
		}
		return NO_ERROR;
	}

	if(mFps_mode != SKIP_NO) {
		if(mFps_OnTime == 1) {
			mFps_OnTime = mFps_mode + 1;
		} else {
			mFps_OnTime--;
			#if defined(DEBUG_FPS)
			skip_count++;
			#endif
			v4L2Camera->release_buffer(currentGetFrame);
			return NO_ERROR;
		}
	}

#if defined(USE_CIFOUT_GRALLOC)
	if(mRecordRunning) {
		enc_metadata = (enc_metadata_t *) enc_metadata_buffer[currentGetFrame]->data;
		enc_metadata->handle 	= NULL;
		enc_metadata->fd_0 		= (int)v4L2Camera->mCameraBufferAddr[currentGetFrame];
		//DBUG("current frame:  index=0x%x, addr=0x%08x.", currentGetFrame, enc_metadata->fd_0);
	}
#else // USE_CIFOUT_GRALLOC
	Preview_frame = v4L2Camera->get_currPreviewFrame(currentGetFrame);
	srcFrameAddr = (unsigned int)v4L2Camera->get_currPreviewPhyAddr(currentGetFrame);
	#if defined(USE_CHANGE_SEMIPLANAR)
	Callback_frame = v4L2Camera->get_currCallbackFrame(currentGetFrame);
	CallbackSrcFrameAddr = (unsigned int)v4L2Camera->get_currCallbackPhyAddr(currentGetFrame);
	#endif // USE_CHANGE_SEMIPLANAR
#endif // USE_CIFOUT_GRALLOC

	#if defined(DEBUG_FPS)
	debugShowFPS();
	#endif

	if(mHDMIOutput || mCompositeOutput || mComponentOutput) {
		//todo: 
	}

	// processing to application callback func.
	if(!mFirst_Frame) {
		if(mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME) {
			DBUG("data(preview) callback.");
			preview_buffer = mRequestMemory(-1, (v4L2Camera->mPreviewWidth*v4L2Camera->mPreviewHeight*1.5), 1, NULL);
			if(mCvtCallback && !mRecordRunning) {
				preview_buffer->handle = (void *)v4L2Camera->mCameraBufferAddr[currentGetFrame];
				mDataCb(CAMERA_MSG_PREVIEW_FRAME, preview_buffer, 0, prv_metadata, mCallbackCookie);							
			} else {
				if(preview_buffer && preview_buffer->data) {
					v4L2Camera->getPreviewRawData((uint8_t *)preview_buffer->data, currentGetFrame);
				}
				mDataCb(CAMERA_MSG_PREVIEW_FRAME, preview_buffer, 0, prv_metadata, mCallbackCookie);
			}
			if(preview_buffer != NULL) preview_buffer->release(preview_buffer);
		}

		if(!mRecordRunning) {
			mapper.unlock((buffer_handle_t) v4L2Camera->mGrallocHandle[currentGetFrame]);
			mCameraWindow->enqueue_buffer(mCameraWindow, v4L2Camera->mBufferHandle[currentGetFrame]);	
//			DBUG("enqueue buffer Index = %d, Addr = 0x%x",currentGetFrame,v4L2Camera->mBufferHandle[currentGetFrame]);
			lockCurrentPreviewBuffer(currentGetFrame);
			
		}
	} else {
		if(mFrameSkip && (systemTime()- mPreviewStart_ns >= FRAMESKIP_NS)) {
			//DBUG(" PreviewStart_ns1 = %lu", systemTime());
			mFrameSkip = false;
 		} else {		
			if(mFrameSkip) {
				if(mCurSkipframe_cnt >= mMaxSkipframe_cnt) {
					//DBUG(" PreviewStart_ns1 = %lu", systemTime());
					mFrameSkip = false;
				} else {
					DBUG("Frame Skip");
				}
				mCurSkipframe_cnt++;
			} else {
				mFirst_Frame = false;
			}
		}
	}

	mRecordingLock.lock();

	if(mRecordRunning) {
	#if defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
		unsigned int rec_buf;
		int rec_buf_idx;

		mLock.lock();
		// Find the offset within the heap of the current buffer.
		ssize_t recoffset = mCurrentPreviewFrame * mPreviewFrameSize*2;

		sp<MemoryHeapBase> recheap = mRecHeap;
		sp<MemoryBase> rec_buffer = mRecBuffers[mCurrentPreviewFrame];
		
		// This is always valid, even if the client died -- the memory is still mapped in our process.
		void *rec_base = recheap->base();

		// Fill the current frame with the fake camera.
		uint8_t *rec_frame = ((uint8_t *)rec_base) + recoffset;
		uint32_t *rec_frame_buffer = ((uint32_t *)rec_frame);
		mLock.unlock();
		
		rec_frame_buffer[0] = v4L2Camera->get_nextRecPhyFrame(rec_frame, currentGetFrame);
		if(rec_frame_buffer[0]== ERR_GET_BUFFER || rec_frame_buffer[0] == ERR_BUFFER_PROC) {
			if(rec_frame_buffer[0] == ERR_BUFFER_PROC){
				//v4L2Camera->release_buffer(currentGetFrame);
				}
			else
				usleep(15 * 1000); //15ms

			return NO_ERROR;
		}
		
		//rec_frame_buffer[1] = currentGetFrame;
		
		// Scaler Setup.
		{
			memset(&mScaler1_arg, 0x00, sizeof(mScaler1_arg));
			mScaler1_arg.responsetype 	= SCALER_INTERRUPT; //SCALER_INTERRUPT - SCALER_POLLING
			mScaler1_arg.src_fmt 		= SCALER_YUV422_sp;
			mScaler1_arg.src_ImgWidth 	= iVideoDisplayWidth;
			mScaler1_arg.src_ImgHeight 	= iVideoDisplayHeight;
			mScaler1_arg.src_winLeft 	= 0;
			mScaler1_arg.src_winTop  	= TVP5150_SCALE_TOP_OFFSET; 
			mScaler1_arg.src_winRight 	= iVideoDisplayWidth;
			mScaler1_arg.src_winBottom 	= iVideoDisplayHeight;
			mScaler1_arg.dest_fmt 		= SCALER_YUV420_sp;

			mScaler1_arg.dest_ImgWidth 	= iVideoDisplayWidth;
			mScaler1_arg.dest_ImgHeight = iVideoDisplayHeight;	
			mScaler1_arg.dest_winLeft 	= 0;
			mScaler1_arg.dest_winTop 	= 0;
			mScaler1_arg.dest_winRight 	= iVideoDisplayWidth;
			mScaler1_arg.dest_winBottom = iVideoDisplayHeight;
		}
		
		#if(0) //20111210 ysseung   not used overlay engine.
		if(mParallel)
			ret = OverlayProcess_Parallel(mM2m_fd1, &mScaler1_arg, &mGrp_arg, frame_buffer[0], NULL, NULL, (uint8_t *)rec_frame_buffer[0]);
		else
			ret = OverlayProcess(mM2m_fd1, &mScaler1_arg, &mGrp_arg, frame_buffer[0], NULL, NULL, (uint8_t *)rec_frame_buffer[0]);
		#endif

		if(ret >= 0) {
			if(mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
				mDataCbTimestamp(systemTime(), CAMERA_MSG_VIDEO_FRAME, preview_buffer, currentGetFrame, mCallbackCookie);
			}
			mRecordFrmSend_Last = currentGetFrame;
			mRecordFrmSend_Total++;
			//DBUG("SendRecFrm = %d-%d'th", currentGetFrame, mRecordFrmSend_Total);
			mCurrentPreviewFrame = (mCurrentPreviewFrame + 1) % BufferCnt;
		}
	#else // defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
		if(mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
			mDataCbTimestamp(systemTime(), CAMERA_MSG_VIDEO_FRAME, enc_metadata_buffer[currentGetFrame], 0, mCallbackCookie); 
		}

		#if 1 //20120329 ysseung   test...
		// unlock & display current frame.
		if(!v4L2Camera->mNoDisplay) {
			mapper.unlock((buffer_handle_t) v4L2Camera->mGrallocHandle[currentGetFrame]);
			mCameraWindow->enqueue_buffer(mCameraWindow, v4L2Camera->mBufferHandle[currentGetFrame]);
			//DBUG("enqueue buffer Index = %d, Addr = 0x%x",currentGetFrame,v4L2Camera->mBufferHandle[currentGetFrame]);  // enqueue buffer check!!

		}
		#else
		if(!v4L2Camera->mIsRunningV2IP) {
			// unlock to display current frame. 
			mapper.unlock((buffer_handle_t) v4L2Camera->mGrallocHandle[currentGetFrame]);
			mCameraWindow->enqueue_buffer(mCameraWindow, v4L2Camera->mBufferHandle[currentGetFrame]);
		}
		#endif

		if(v4L2Camera->mVideoSnapshot) {
			v4L2Camera->copyToVideoSnapshotFrame(currentGetFrame);
			v4L2Camera->mVideoSnapshot = false;
		}

		mRecordFrmSend_Last = currentGetFrame;
		mRecordFrmSend_Total++;
		//DBUG("SendRecFrm = %d-%d'th", currentGetFrame, mRecordFrmSend_Total);
		mCurrentPreviewFrame = (mCurrentPreviewFrame + 1) % BufferCnt;
	#endif // defined(SENSOR_TVP5150) || defined(SENSOR_RDA5888)
	}
	else {
		v4L2Camera->release_buffer(currentGetFrame);
		mCurrentPreviewFrame = (mCurrentPreviewFrame + 1) % BufferCnt;
	}
	
	mRecordingLock.unlock();

//	FUNCTION_OUT;
	return NO_ERROR;
}

status_t TelechipsCameraHardware::previewThread()
{
	FUNCTION_IN
	int ret;
    Message msg;
    bool cmd_process;	
    bool  threadloop = true;
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;

	while(threadloop) {
		cmd_process = false;

		if(mPreviewRunning && mCreateCameraWindow && mCamStatus == CAM_STATUS_PREVIEW_START) {
			call_nextPreview();
			if(!CameraCmdQ->isEmpty()) {
				CameraCmdQ->get(&msg);
				cmd_process = true;
			}			
		} else {
			DBUG("No mPreviewRunning! Wait Command!!");
            CameraCmdQ->get(&msg);
            cmd_process = true;
		}

		if(!cmd_process) continue;

		ALOGE("cmd-id = %d.", msg.command);
		ret = 0;
		switch(msg.command) {
			case CAMERA_INIT:
				mPreviewRunning = true;
				msg.command = ret ? CAMERA_NACK : CAMERA_ACK;
				CameraAckQ->put(&msg);
				break;

			case CAMERA_START:
				DBUG("mPreviewRunning [%d] mCreateCameraWindow[%d] ", mPreviewRunning, mCreateCameraWindow);
				if(mPreviewRunning && mCreateCameraWindow) {
					DBUG("camera preview start...");
					ret = call_startPreview();			
				}
				msg.command = ret ? CAMERA_NACK : CAMERA_ACK;
				CameraAckQ->put(&msg);
				break;

			case CAMERA_STOP:
				if(mPreviewRunning && mCreateCameraWindow) {
					ret = v4L2Camera->stopPreview();
					ALOGD("!!!camera stop result=%d!!!", ret);
					if(!ret) mPreviewRunning = false;
					if(mPreviewBuffers != NULL) {
						ALOGD("!!!camera stop mPreviewBuffers is not null!!!");
						#if !defined(FEATURE_DESTROY_PREVIEW_THREAD)
							if(mCamStatus != CAM_STATUS_CAPTURE) 
						#endif
								mPreviewBuffers = (unsigned int *)freeCameraBuffers(CAM_STATUS_PREVIEW_START);
						mCreateCameraWindow = false;
					}
				}
				msg.command = ret ? CAMERA_NACK : CAMERA_ACK;
				CameraAckQ->put(&msg);
				break;
				
			#if (1) // 20131018 swhwang, for check video frame interrupt
			case CAMERA_PAUSE:
				if(mPreviewRunning && mCreateCameraWindow) {
					ret = v4L2Camera->stopPreview();
					if(!ret) mPreviewRunning = false;
				}
				msg.command = ret ? CAMERA_NACK : CAMERA_ACK;
				CameraAckQ->put(&msg);
				break;
			#endif
			
			case CAMERA_KILL:
				if(mPreviewRunning) ret = v4L2Camera->stopPreview();
				if(!ret) mPreviewRunning = false;
				threadloop = false;
				if(mPreviewBuffers != NULL) {
					ALOGD("!!!camera kill mPreviewBuffers is not null!!!");
					#if(0) //20111215 ysseung	capture 시 pmap을 사용하므로, gralloc으로 할당된 preview buffer를 free하지 않는다.
					mPreviewBuffers = (unsigned int *)freeCameraBuffers(CAM_STATUS_PREVIEW_START);
					#endif
					mCreateCameraWindow = false;
				}
				msg.command = ret ? CAMERA_NACK : CAMERA_ACK;
				CameraAckQ->put(&msg);
				break;

			case CAMERA_REC_START:
				mRecordingLock.lock();
				mRecordRunning 	= true;
                mFrameSkip = false;
                mFps_OnTime = 1;
				mRecordFrmSend_Total = mRecordFrmSend_Last = 0;
				mPrevRelFrm_idx = -1;
				mRecordingLock.unlock();
				msg.command = CAMERA_ACK;
				CameraAckQ->put(&msg);
				break;

			case CAMERA_REC_STOP:
				mRecordingLock.lock();
				DBUG("Record Stopped! mRecordFrmSend_Total = %lu, mRecordFrmSend_Last = %lu", mRecordFrmSend_Total, mRecordFrmSend_Last);
				mRecordRunning 	= false;
				mRecordingLock.unlock();
				msg.command = CAMERA_ACK;
				CameraAckQ->put(&msg);
				break;			

			case CAMERA_REC_SNAPSHOT:
				ALOGD("rec. snap-shot jpeg encoding start.");
				msg.command = CAMERA_ACK;
				CameraAckQ->put(&msg);
				break;
		}
	}

	FUNCTION_OUT;
	return NO_ERROR;
}

status_t TelechipsCameraHardware::startPreview()
{
	FUNCTION_IN
	Message msg;

	if(mPreviewThread == NULL) 	createPreviewThread();

	msg.command = CAMERA_INIT;
	CameraCmdQ->put(&msg);
	CameraAckQ->get(&msg);

	FUNCTION_OUT;
	return NO_ERROR;
}


#if(1) // 20131018 swhwang, for check video frame interrupt
status_t TelechipsCameraHardware::resume()
{
	Message msg;
	int ret = NO_ERROR;
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	
	DBUG("%s ",__func__);


	if(mCameraWindow != NULL)
	{
		ret = setPreviewWindow(mCameraWindow);

		if(ret == NO_ERROR && mPreviewBuffers !=NULL)
		{
			Message msg;	 
			
			mPreviewRunning = true;
			msg.command = CAMERA_START;
//			mCamStatus = CAM_STATUS_PREVIEW_START;
			
			CameraCmdQ->put(&msg);
			CameraCmdQ->get(&msg);
		
			ret = (msg.command == CAMERA_ACK) ? NO_ERROR: INVALID_OPERATION;	
		}
		
	}

	return ret;
}

void TelechipsCameraHardware::pause()
{
	Message msg;
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;

	DBUG("%s ",__func__);
	

	
	mCamStatus = CAM_STATUS_PAUSE;
	
	
	if(mPreviewThread == NULL) {
		ALOGE("Thread is not activated.");
		return;
	}
	
	msg.command = CAMERA_PAUSE;
	CameraCmdQ->put(&msg);
	CameraCmdQ->get(&msg);
	
}


#endif

#if(0)
int TelechipsCameraHardware::VIQEProcess(unsigned int PA_Y, unsigned int PA_U, unsigned int PA_V, unsigned int DestAddr)
{
	int inFrameWidth, inFrameHeight;
	int iRet;

	inFrameWidth = ((mDecodedWidth+15)>>4)<<4;
	inFrameHeight = mDecodedHeight;

	
	mViqe_arg.address[0] = PA_Y; mViqe_arg.address[1] = PA_U; mViqe_arg.address[2] = PA_V;
	mViqe_arg.address[3] = mPrevY; mViqe_arg.address[4] = mPrevU; mViqe_arg.address[5] = mPrevV;
	mViqe_arg.dstAddr = DestAddr;
	
	if(mVIQE_firstfrm == 1)
	{
		ALOGI("Process First Frame. pMParm->mVIQE_firstfrm %d", mVIQE_firstfrm);
		
		mViqe_arg.onTheFly = mViqe_arg.deinterlaceM2M = mVIQE_onthefly;		//// VIQE always pass to M2MScaler on-the-fly.
		mViqe_arg.srcWidth = inFrameWidth;
		mViqe_arg.srcHeight = inFrameHeight;
		mViqe_arg.scalerCh = 0;

		if(mHDMIOutput == 1)
			mViqe_arg.lcdCtrlNo = 0;
		else
			mViqe_arg.lcdCtrlNo = 1;
		mViqe_arg.address[3] = PA_Y; mViqe_arg.address[4] = PA_U; mViqe_arg.address[5] = PA_V;

		mViqe_fd = open(VIQE_DEVICE, O_RDWR);
		if (mViqe_fd <= 0)
			ALOGE("can't open[%s] '%s'", strerror(errno), VIQE_DEVICE);
		
		if(mM2m_fd > 0)
		{
			DBUG("Now pMParm->mM2m_fd is not closed normaly , close and open pMParm->mM2m_fd %d ", mM2m_fd);
			close(pMParm->mM2m_fd);
			pMParm->mM2m_fd = -1;
		}

		if(mM2m_fd < 0)
		{
			mM2m_fd = open(VIDEO_PLAY_SCALER, O_RDWR);
			if (mM2m_fd <= 0)
				ALOGE("can't open[%s] '%s'", strerror(errno), VIDEO_PLAY_SCALER);
		}

		iRet = ioctl(mViqe_fd, IOCTL_VIQE_INITIALIZE, &mViqe_arg);
		if(iRet < 0)
			ALOGE("Viqe Initialize Fail!!\n");
		
		mVIQE_firstfrm = 0;
	}

	iRet = ioctl(mViqe_fd, IOCTL_VIQE_EXCUTE, &mViqe_arg);
	if(iRet < 0)
	{
		mVIQE_onoff = false;
		DBUG("VIQE OFF %d\n", mVIQE_onoff);
	}
	

	return 0;
}

status_t TelechipsCameraHardware::setOverlay(const sp<Overlay> &overlay) 
{
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;

	FUNCTION_IN

    Mutex::Autolock lock(mLock);

    DBUG("setOverlay/1/%08lx/%08lx", (long unsigned int)overlay.get(), (long unsigned int)mOverlay.get());
    // De-alloc any stale data
    if(mOverlay.get() != NULL)
    {
        ALOGE("Destroying current overlay");
		mOverlay_Setuped = false;
		mOverlay_recheck = false;
		//usleep(100*1000); //100ms  //20110811 ysseung   prevent to overlay error.

        mOverlay->destroy();
		mOverlay = NULL;
		usleep(100*1000); //100ms

		if(mHDMIOutput || mCompositeOutput || mComponentOutput)
		{
			struct tcc_lcdc_image_update hdmi_display;

			memset(&hdmi_display, 0x00, sizeof(hdmi_display));
			hdmi_display.Lcdc_layer = LCDC_LAYER_1;
			hdmi_display.enable = 0;
			ioctl(mFb_fd, TCC_LCDC_HDMI_DISPLAY, &hdmi_display);
		}
    }

    mOverlay = overlay;
    if(mOverlay == NULL)
    {
        ALOGE("Trying to set overlay, but overlay is null!");
    }
	else
	{	
		int buffer_cnt;
		
		mOverlayBuffer_Cnt = mOverlay->getBufferCount(); 
		mOverlayBuffer_Current = 0;
		//DBUG("mOverlayBuffer_Cnt = %d ", mOverlayBuffer_Cnt);

		v4L2Camera->getPreviewSize(&iVideoDisplayWidth, &iVideoDisplayHeight);
		mOverlay_recheck = false;

		if(mHDMIOutput || mCompositeOutput || mComponentOutput){
			OverlayCheckExternalDisplay(mOverlay_recheck);
		}
		else{
			if(OverlayCheck(mOverlay_recheck)){
				mOverlay->resizeInput(mTarget_DispInfo.sx, mTarget_DispInfo.sy, mTarget_DispInfo.width, mTarget_DispInfo.height);
			}
		}
		
		for(buffer_cnt=0; buffer_cnt < mOverlayBuffer_Cnt; buffer_cnt++)
		{
			mOverlayBuffer_PhyAddr[buffer_cnt] = mOverlay->getBufferAddress((void*)buffer_cnt);
			DBUG("Overlay :: %d/%d = 0x%x ", buffer_cnt, mOverlayBuffer_Cnt, (unsigned int)mOverlayBuffer_PhyAddr[buffer_cnt]);
		}

		mOverlay_Setuped = true;
		mCamStatus = CAM_STATUS_PREVIEW_START;
	}

	FUNCTION_OUT
	
    return NO_ERROR;
} 
#endif

void TelechipsCameraHardware::stopPreview()
{
	FUNCTION_IN
	Message msg;

	if(mPreviewThread == NULL) {
		ALOGE("Preview Thread is not activated.");
		if(mCamStatus != CAM_STATUS_CAPTURE) { //20120417 ysseung   if the camera restarted before the stop, modify to system hang-up issue.
			return;
		} else {
			createPreviewThread();
		}
	}

	msg.command = CAMERA_STOP;
	CameraCmdQ->put(&msg);
	CameraAckQ->get(&msg);
	FUNCTION_OUT
}

bool TelechipsCameraHardware::previewEnabled()
{
	if(mPreviewRunning) {
		DBUG("preview Enabled");
	} else {
		DBUG("preview Disabled");
	}
	return mPreviewRunning;
}

status_t TelechipsCameraHardware::startRecording()
{
	FUNCTION_IN
	int i;
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	Message msg;

	usleep(500*1000); // by kwon
	if(mPreviewThread == NULL) {
		ALOGE("Preview Thread is not activated.");
		return INVALID_OPERATION;
	}

	for(i = 0; i< v4L2Camera->mPreviewBufferCount; i++) {
		enc_metadata_buffer[i] = mRequestMemory(-1, (v4L2Camera->mPreviewWidth*v4L2Camera->mPreviewHeight*3/2), 1, NULL);
		if((NULL == enc_metadata_buffer[i]) || (NULL == enc_metadata_buffer[i]->data)) {
			DBUG("Error! Could not allocate memory for Video Metadata Buffers");
		}
	}

	msg.command = CAMERA_REC_START;
	CameraCmdQ->put(&msg);
	CameraAckQ->get(&msg);

	mRecordFrmRelease = true;

	FUNCTION_OUT
	return (msg.command == CAMERA_ACK) ? NO_ERROR : INVALID_OPERATION;
}

void TelechipsCameraHardware::stopRecording()
{
	FUNCTION_IN
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;	
	Message msg;
	int i;

	DBUG("recording STOP!!!");
	if(mPreviewThread == NULL) {
		ALOGE("Preview Thread is not activated.");
		return;
	}

	mRecordFrmRelease = false;
	mRecordRunning 	= false;
	msg.command = CAMERA_REC_STOP;
	CameraCmdQ->put(&msg);
	CameraAckQ->get(&msg);

	//20111020 ysseung    make video thumbnail that operating to VPU. - start -
	//TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	if(0) {//v4L2Camera->get_cameraType_isUVC()) {
		msg.command = CAMERA_STOP;
		CameraCmdQ->put(&msg);
		CameraAckQ->get(&msg);
	}
	//20111020 ysseung    make video thumbnail that operating to VPU. - end -

	for(i=0; i < v4L2Camera->mPreviewBufferCount; i++) {
		enc_metadata_buffer[i]->release(enc_metadata_buffer[i]);
	}
#if defined(CONFIG_SND_USB_AUDIO) && defined(CONFIG_USB_VIDEO_CLASS) //2013.10.31 swhwang, fixed to don`t call next preview issue when UAC enable
	call_startPreview();
	DBUG("Preview resume!!!!!");
#endif
	FUNCTION_OUT
}

bool TelechipsCameraHardware::recordingEnabled()
{
	if(mRecordRunning) {
		DBUG("recording Enabled");
	} else {
		DBUG("recording Disabled");
	}
	return mRecordRunning;
}

void TelechipsCameraHardware::releaseRecordingFrame(const void* mem)
{
 
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	GraphicBufferMapper &mapper = GraphicBufferMapper::get();
	int BufferCnt = v4L2Camera->get_bufferCount();
	uint32_t phy_address;
	int buffer_index = NULL;
    enc_metadata_t *enc_metadata;
	#if defined(USE_CIFOUT_GRALLOC)
	if(!mRecordFrmRelease) return;
	
	#if defined(CONFIG_USB_VIDEO_CLASS)
	while(mRecordRunning&&!mRecordFrmSend_Total)		usleep(1000);
	#endif
	
	enc_metadata = (enc_metadata_t *) mem;
	#if defined(SENSOR_TVP5150)	|| defined(SENSOR_RDA5888)
	buffer_index = v4L2Camera->get_RecbufferIndex(phy_address);
	#else
	buffer_index = v4L2Camera->get_bufferIndex(enc_metadata->fd_0);
	#endif
	//DBUG("record frame:  index=0x%x, addr=0x%08x.", buffer_index, enc_metadata->fd_0);
	if(buffer_index >= 0) {
		#if 1 //20120329 ysseung   test...
		// todo : 
		#else
		if(v4L2Camera->mIsRunningV2IP) {
			mapper.unlock((buffer_handle_t) v4L2Camera->mGrallocHandle[buffer_index]);
			mCameraWindow->enqueue_buffer(mCameraWindow, v4L2Camera->mBufferHandle[buffer_index]);			
		}
		#endif
			if(!v4L2Camera->mNoDisplay) {
				lockCurrentPreviewBuffer(buffer_index);
			}
			mPrevRelFrm_idx = buffer_index;
			mRecordFrmSend_Total--;
			v4L2Camera->release_buffer(buffer_index);

	} else {
		ALOGE("REC. release frame error.");
	}
	#else // USE_CIFOUT_GRALLOC
	if(!mRecordFrmRelease)
		return;

	mLock.lock();

	ssize_t offset;
	size_t size;		
	sp<IMemoryHeap> heap = (IMemoryHeap)mem->getMemory(&offset, &size);

	void *base = heap->base();

	mLock.unlock();

	uint8_t *frame = ((uint8_t *)base) + mem->offset();
	uint32_t *frame_buffer = ((uint32_t *)frame);
	phy_address = frame_buffer[0];

	#if defined(SENSOR_TVP5150)	|| defined(SENSOR_RDA5888)
	buffer_index = v4L2Camera->get_RecbufferIndex(phy_address);
	#else
	buffer_index = v4L2Camera->get_bufferIndex(phy_address);
	#endif

	if(buffer_index >= 0) {
		mPrevRelFrm_idx = buffer_index;
		v4L2Camera->release_buffer(buffer_index);
		mRecordFrmSend_Total--;
		//DBUG("Rel-RecFrm = %d-%d'th", buffer_index, mRecordFrmSend_Total);
	} else {
		ALOGE("Rel-RecFrm Error");
	}
	#endif // USE_CIFOUT_GRALLOC
}

// ---------------------------------------------------------------------------

status_t TelechipsCameraHardware::beginAutoFocusThread(void *cookie)
{
    TelechipsCameraHardware *c = (TelechipsCameraHardware *)cookie;
    return c->autoFocusThread();
}

status_t TelechipsCameraHardware::autoFocusThread()
{
	int ret,loop_cnt;
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;

	FOCUS_MSG = FOCUS_DISABLE;
	if((ret = v4L2Camera->setOperation(PROC_AF, 0))< 0){
		ALOGE("autofocus fail!!!");	
		if(mMsgEnabled & CAMERA_MSG_FOCUS) 	mNotifyCb(CAMERA_MSG_FOCUS, true, 0, mCallbackCookie);
		FOCUS_MSG = FOCUS_ENABLE;
		return NO_ERROR;
	}
	for(loop_cnt=0;loop_cnt<30;loop_cnt++){
		DBUG("FOCUS_MSG : %d(%d)",FOCUS_MSG,loop_cnt);
		if(FOCUS_MSG== WAIT_FOCUS||FOCUS_MSG== CANCEL_FOCUS){
			break;
		}
			usleep(33*1000);
	}
	FOCUS_MSG = FOCUS_ENABLE;
	if(mMsgEnabled & CAMERA_MSG_FOCUS) 	mNotifyCb(CAMERA_MSG_FOCUS, true, 0, mCallbackCookie);
	return NO_ERROR;
}

status_t TelechipsCameraHardware::autoFocus()
{
    Mutex::Autolock lock(mLock);
	int loop_cnt;
	if(FOCUS_MSG == FOCUS_DISABLE)FOCUS_MSG=WAIT_FOCUS;
	if(FOCUS_MSG==WAIT_FOCUS){
		for(loop_cnt=0;loop_cnt<5;loop_cnt++){
			ALOGE("wait focus enable!! FOCUS_MSG : %d",FOCUS_MSG);
			if(FOCUS_MSG == FOCUS_ENABLE)break;
			usleep(10*1000);
		}
	}
	DBUG("auto Focus start!!! start FOCUS_MSG : %d",FOCUS_MSG);
    if(createThread(beginAutoFocusThread, this) == false) return UNKNOWN_ERROR;
	FUNCTION_OUT
    return NO_ERROR;
}

status_t TelechipsCameraHardware::cancelAutoFocus()
{
	FOCUS_MSG = CANCEL_FOCUS;
    return NO_ERROR;
}

status_t TelechipsCameraHardware::beginPictureThread(void *cookie)
{
    TelechipsCameraHardware *c = (TelechipsCameraHardware *)cookie;
    return c->pictureThread();
}

status_t TelechipsCameraHardware::pictureThread()
{
	FUNCTION_IN
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	int capture_width, capture_height;
	camera_memory_t* picture = NULL;
	unsigned int jpeg_size = 0;
	int nRotationOfCapture = 0;
	bool IsitPortraitLCD = false;
	float gpsLatitude = 0;
	float gpsLongitude = 0;
	float tmp1 = 0; 
	float tmp2 = 0;
	float tmp3 = 0;  // 0228
	int gpsProcess = 0;
	const int t_v1 = 1000000;
	const int t_v2 = 100000;
	const int t_v3 = 10000;
	const int m_v1 = 60;
	Message msg;

	// for rotate capture to match orientation.
	if(mLcd_height > mLcd_width) IsitPortraitLCD = true;

	mCamStatus = CAM_STATUS_CAPTURE;
	
	#if !defined(FEATURE_DESTROY_PREVIEW_THREAD)
		msg.command = CAMERA_STOP;
		CameraCmdQ->put(&msg);
		CameraAckQ->get(&msg);
	#endif
	
	mParameters.getPictureSize(&capture_width, &capture_height);
	DBUG("picture size: %d x %d  --> capture start", capture_width, capture_height);

	// allocate to capture buffer
	#if (0) //20111215 ysseung   capture 시에는 gralloc buffer를 사용하지 않고, pmap을 사용함.
	allocCaptureBuffers(capture_width, capture_height, mParameters.getPreviewFormat(), NUM_OF_CAPTURE_BUFFERS);
	#endif

	// check to rotation info.
	nRotationOfCapture = mParameters.getInt("rotation");
	switch(nRotationOfCapture) {
		case 0: 	v4L2Camera->mRotation = 1;
			break;
		case 90: 	v4L2Camera->mRotation = 6;
			break;
		case 180: 	v4L2Camera->mRotation = 3;
			break;
		case 270: 	v4L2Camera->mRotation = 8;
			break;
		default: 	v4L2Camera->mRotation = 1;
			break;
	}

	// check to thumbnail info.
	if((mParameters.getInt("jpeg-thumbnail-width") > 0) && (mParameters.getInt("jpeg-thumbnail-height") > 0))
		v4L2Camera->supported_thumbnail = true;
	else
		v4L2Camera->supported_thumbnail = false;

	if(v4L2Camera->get_cameraType_isUVC() && v4L2Camera->mCapture_fmt == V4L2_PIX_FMT_MJPEG)
		v4L2Camera->supported_thumbnail = false;

	// check to GPS info.
	gpsLatitude  = mParameters.getFloat("gps-latitude");
	gpsLongitude = mParameters.getFloat("gps-longitude");
	gpsProcess = mParameters.getInt("gps-processing-method");

	v4L2Camera->supported_gps = ((gpsLatitude != 0.0) || (gpsLongitude != 0.0)) && ((gpsLatitude != -1) || (gpsLongitude != -1));

	if(v4L2Camera->supported_gps) {
		// set to latitude
		if(gpsLatitude > 0) {
			v4L2Camera->GPSInfo.Latitude.degrees = floor(gpsLatitude);
			tmp1 = ceil((gpsLatitude -(int)gpsLatitude)*t_v1)/t_v1*m_v1;
			v4L2Camera->GPSInfo.Latitude.minutes = floor(tmp1);
			tmp2 = ceil((tmp1 - (int)tmp1)*t_v2)/t_v2*m_v1;
                        tmp3 = ceil(tmp2 * t_v3);
			v4L2Camera->GPSInfo.Latitude.seconds = tmp3; // floor(tmp2);
		} else {
                        tmp3 = fabs(gpsLatitude);
			v4L2Camera->GPSInfo.Latitude.degrees = (0 - floor(tmp3));
			tmp1 = ceil((tmp3 -(int)tmp3)*t_v1)/t_v1*m_v1;
                        if(v4L2Camera->GPSInfo.Latitude.degrees == 0){
			    v4L2Camera->GPSInfo.Latitude.minutes = (0 - floor(tmp1));
                        } else {  
			    v4L2Camera->GPSInfo.Latitude.minutes = floor(tmp1);
                        }
			tmp2 = ceil((tmp1 - (int)tmp1)*t_v2)/t_v2*m_v1;
			tmp3 = ceil(tmp2 * t_v3);
                        if((v4L2Camera->GPSInfo.Latitude.degrees == 0) && (v4L2Camera->GPSInfo.Latitude.degrees == 0)){
                            v4L2Camera->GPSInfo.Latitude.seconds = (0 - tmp3);
                        } else {
                            v4L2Camera->GPSInfo.Latitude.seconds = tmp3;

                        }
		}

		// set to longitude
		if(gpsLongitude > 0) {
			v4L2Camera->GPSInfo.Longitude.degrees = floor(gpsLongitude);
			tmp1 = ceil((gpsLongitude - (int)gpsLongitude)*t_v1)/t_v1*m_v1;
			v4L2Camera->GPSInfo.Longitude.minutes = floor(tmp1);
			tmp2 = ceil((tmp1 - (int)tmp1)*t_v1)/t_v1*m_v1;
			tmp3 = ceil(tmp2 * t_v3);
			v4L2Camera->GPSInfo.Longitude.seconds = tmp3;
		} else {
                        tmp3 = fabs(gpsLongitude);
			v4L2Camera->GPSInfo.Longitude.degrees = (0 - floor(tmp3));
			tmp1 = ceil((tmp3 - (int)tmp3)*t_v1)/t_v1*m_v1;
                        if(v4L2Camera->GPSInfo.Longitude.degrees == 0){
                            v4L2Camera->GPSInfo.Longitude.minutes = (0 - floor(tmp1));
                        } else {
                            v4L2Camera->GPSInfo.Longitude.minutes = floor(tmp1);
                        }
			tmp2 = ceil((tmp1 - (int)tmp1)*t_v2)/t_v2*m_v1;
			tmp3 = ceil(tmp2 * t_v3);
                        if((v4L2Camera->GPSInfo.Longitude.degrees == 0) && (v4L2Camera->GPSInfo.Longitude.degrees == 0)){
			    v4L2Camera->GPSInfo.Longitude.seconds = (0 - tmp3);
                        } else {
			    v4L2Camera->GPSInfo.Longitude.seconds = tmp3;
                        }
		}

		// set to Altitude
		v4L2Camera->GPSInfo.Altitude = mParameters.getInt("gps-altitude");

		// set to GPS time-stamp
		v4L2Camera->GPSInfo.Timestamp = (unsigned int)mParameters.getInt("gps-timestamp");

		// set to GPS processing method
		v4L2Camera->GPSInfo.Processing_Method = mParameters.get("gps-processing-method");
	}
	else {
		memset(&v4L2Camera->GPSInfo, 0x00, sizeof(GPS_Info_Type));
	}
	
	DBUG("GPS-Latitude: degrees = %d, minutes = %d, seconds = %d.", v4L2Camera->GPSInfo.Latitude.degrees, \
			v4L2Camera->GPSInfo.Latitude.minutes, v4L2Camera->GPSInfo.Latitude.seconds);
	DBUG("GPS-Longitude: degrees = %d, minutes = %d, seconds = %d.", v4L2Camera->GPSInfo.Longitude.degrees, \
			v4L2Camera->GPSInfo.Longitude.minutes, v4L2Camera->GPSInfo.Longitude.seconds);
	DBUG("GPS-Altitude: %d, GPS-Timestamp = %d, GPS-Processing-Method = %s.", v4L2Camera->GPSInfo.Altitude, \
			v4L2Camera->GPSInfo.Timestamp, v4L2Camera->GPSInfo.Processing_Method);

	v4L2Camera->capture(&jpeg_size, capture_width, capture_height, nRotationOfCapture, IsitPortraitLCD);
	mCamStatus = CAM_STATUS_CAPTURE_COMPLETE; //20120417 ysseung   if the camera restarted before the stop, modify to system hang-up issue.
	DBUG("capture done:  (%d)%d KB.", jpeg_size, (jpeg_size/1024));

	if(mMsgEnabled & CAMERA_MSG_SHUTTER) {
		DBUG("notify(Shutter) callback.");
		mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
	}

	if(mMsgEnabled & CAMERA_MSG_RAW_IMAGE) {
		DBUG("data(raw-image) callback.");
		picture = mRequestMemory(-1, (capture_width*capture_height*2), 1, NULL);
		if(picture && picture->data) {
			v4L2Camera->getCapturedRawData((uint8_t *)picture->data);
		}
		mDataCb(CAMERA_MSG_RAW_IMAGE, picture, 0, NULL, mCallbackCookie);
		if(picture != NULL) picture->release(picture);
	}

	if (mMsgEnabled & CAMERA_MSG_RAW_IMAGE_NOTIFY) {
		DBUG("CAMERA_MSG_RAW_IMAGE_NOTIFY callback.");
		mNotifyCb(CAMERA_MSG_RAW_IMAGE_NOTIFY, 0, 0, mCallbackCookie);
	}
	if(mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE) {
		DBUG("data(jpeg) callback.");
		if(jpeg_size > 0) {
			picture = mRequestMemory(-1, jpeg_size, 1, NULL);
			if(picture && picture->data) {
				v4L2Camera->getCapturedJpegData((uint8_t *)picture->data);
			}
			mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, picture, 0, NULL, mCallbackCookie);
		} else {
			DBUG("data(jpeg) NULL callback.");
			mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, NULL, 0, NULL, mCallbackCookie);
		}
		if(picture != NULL) picture->release(picture);
	}

	#if defined(FEATURE_DESTROY_PREVIEW_THREAD)
		if(mPreviewThread==NULL) createPreviewThread();
	#endif

	FUNCTION_OUT;
	return NO_ERROR;
}

status_t TelechipsCameraHardware::videoSnapshotThread()
{
	FUNCTION_IN
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	float gpsLatitude=0, gpsLongitude=0, tmp1=0, tmp2=0, tmp3=0;
	const int t_v1 = 1000000;
	const int t_v2 = 100000;
	const int t_v3 = 10000;
	const int m_v1 = 60;
	camera_memory_t* picture = NULL;
	unsigned int jpeg_size = 0;
	int iRotation = 0;
	int capture_width, capture_height;
	int gpsProcess = 0;
	
	v4L2Camera->mVideoSnapshot = true;
	DBUG("picture size:  %d x %d  --> capture start.", v4L2Camera->mPreviewWidth, v4L2Camera->mPreviewHeight);

	// check to rotation info.
	iRotation = mParameters.getInt("rotation");
	switch(iRotation) {
		case 0: 	v4L2Camera->mRotation = 1;
			break;
		case 90: 	v4L2Camera->mRotation = 6;
			break;
		case 180: 	v4L2Camera->mRotation = 3;
			break;
		case 270: 	v4L2Camera->mRotation = 8;
			break;
		default: 	v4L2Camera->mRotation = 1;
			break;
	}

	// check to thumbnail info.
	if(v4L2Camera->get_cameraType_isUVC()) {
		v4L2Camera->supported_thumbnail = false;
	} else {
		if((mParameters.getInt("jpeg-thumbnail-width") > 0) && (mParameters.getInt("jpeg-thumbnail-height") > 0)) {
			v4L2Camera->supported_thumbnail = true;
		} else {
			v4L2Camera->supported_thumbnail = false;
		}
	}

	// check to GPS info.
	gpsLatitude  = mParameters.getFloat("gps-latitude");
	gpsLongitude = mParameters.getFloat("gps-longitude");
	gpsProcess = mParameters.getInt("gps-processing-method");
	v4L2Camera->supported_gps = ((gpsLatitude != 0.0) || (gpsLongitude != 0.0)) && ((gpsLatitude != -1) || (gpsLongitude != -1));
	if(v4L2Camera->supported_gps) {
		// set to latitude
		if(gpsLatitude > 0) {
			v4L2Camera->GPSInfo.Latitude.degrees = floor(gpsLatitude);
			tmp1 = ceil((gpsLatitude -(int)gpsLatitude)*t_v1)/t_v1*m_v1;
			v4L2Camera->GPSInfo.Latitude.minutes = floor(tmp1);
			tmp2 = ceil((tmp1 - (int)tmp1)*t_v2)/t_v2*m_v1;
                        tmp3 = ceil(tmp2 * t_v3);
			v4L2Camera->GPSInfo.Latitude.seconds = tmp3; // floor(tmp2);
		} else {
                        tmp3 = fabs(gpsLatitude);
			v4L2Camera->GPSInfo.Latitude.degrees = (0 - floor(tmp3));
			tmp1 = ceil((tmp3 -(int)tmp3)*t_v1)/t_v1*m_v1;
                        if(v4L2Camera->GPSInfo.Latitude.degrees == 0){
			    v4L2Camera->GPSInfo.Latitude.minutes = (0 - floor(tmp1));
                        } else {  
			    v4L2Camera->GPSInfo.Latitude.minutes = floor(tmp1);
                        }
			tmp2 = ceil((tmp1 - (int)tmp1)*t_v2)/t_v2*m_v1;
			tmp3 = ceil(tmp2 * t_v3);
                        if((v4L2Camera->GPSInfo.Latitude.degrees == 0) && (v4L2Camera->GPSInfo.Latitude.degrees == 0)){
                            v4L2Camera->GPSInfo.Latitude.seconds = (0 - tmp3);
                        } else {
                            v4L2Camera->GPSInfo.Latitude.seconds = tmp3;

                        }
		}

		// set to longitude
		if(gpsLongitude > 0) {
			v4L2Camera->GPSInfo.Longitude.degrees = floor(gpsLongitude);
			tmp1 = ceil((gpsLongitude - (int)gpsLongitude)*t_v1)/t_v1*m_v1;
			v4L2Camera->GPSInfo.Longitude.minutes = floor(tmp1);
			tmp2 = ceil((tmp1 - (int)tmp1)*t_v1)/t_v1*m_v1;
			tmp3 = ceil(tmp2 * t_v3);
			v4L2Camera->GPSInfo.Longitude.seconds = tmp3;
		} else {
                        tmp3 = fabs(gpsLongitude);
			v4L2Camera->GPSInfo.Longitude.degrees = (0 - floor(tmp3));
			tmp1 = ceil((tmp3 - (int)tmp3)*t_v1)/t_v1*m_v1;
                        if(v4L2Camera->GPSInfo.Longitude.degrees == 0){
                            v4L2Camera->GPSInfo.Longitude.minutes = (0 - floor(tmp1));
                        } else {
                            v4L2Camera->GPSInfo.Longitude.minutes = floor(tmp1);
                        }
			tmp2 = ceil((tmp1 - (int)tmp1)*t_v2)/t_v2*m_v1;
			tmp3 = ceil(tmp2 * t_v3);
                        if((v4L2Camera->GPSInfo.Longitude.degrees == 0) && (v4L2Camera->GPSInfo.Longitude.degrees == 0)){
			    v4L2Camera->GPSInfo.Longitude.seconds = (0 - tmp3);
                        } else {
			    v4L2Camera->GPSInfo.Longitude.seconds = tmp3;
                        }
		}

		// set to Altitude
		v4L2Camera->GPSInfo.Altitude = mParameters.getInt("gps-altitude");

		// set to GPS time-stamp
		v4L2Camera->GPSInfo.Timestamp = (unsigned int)mParameters.getInt("gps-timestamp");

		// set to GPS processing method
		v4L2Camera->GPSInfo.Processing_Method = mParameters.get("gps-processing-method");
	}
	else {
		memset(&v4L2Camera->GPSInfo, 0x00, sizeof(GPS_Info_Type));
	}
	DBUG("GPS-Latitude: degrees = %d, minutes = %d, seconds = %d.", v4L2Camera->GPSInfo.Latitude.degrees, \
			v4L2Camera->GPSInfo.Latitude.minutes, v4L2Camera->GPSInfo.Latitude.seconds);
	DBUG("GPS-Longitude: degrees = %d, minutes = %d, seconds = %d.", v4L2Camera->GPSInfo.Longitude.degrees, \
			v4L2Camera->GPSInfo.Longitude.minutes, v4L2Camera->GPSInfo.Longitude.seconds);
	DBUG("GPS-Altitude: %d, GPS-Timestamp = %d, GPS-Processing-Method = %s.", v4L2Camera->GPSInfo.Altitude, \
			v4L2Camera->GPSInfo.Timestamp, v4L2Camera->GPSInfo.Processing_Method);

	for(int i=0; i < 30; i++) {
		DBUG("waiting:  finish video snap-shot frame copy.");
		usleep(20*1000); // sleep is 20ms.
		if(!v4L2Camera->mVideoSnapshot) {
			v4L2Camera->mDoneVideoSnapshot = true;
			break;
		}
	}

	v4L2Camera->mbRotateCapture = false; // rotate-capture option.
	jpeg_size = v4L2Camera->camif_encode_jpeg();
	mParameters.getPictureSize(&capture_width, &capture_height);
	
	if(mMsgEnabled & CAMERA_MSG_SHUTTER) {
		DBUG("notify(Shutter) callback.");
		mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
	}

	if(mMsgEnabled & CAMERA_MSG_RAW_IMAGE) {
		DBUG("data(raw-image) callback.");
		picture = mRequestMemory(-1, (capture_width*capture_height*2), 1, NULL);
		if(picture && picture->data) {
			v4L2Camera->getCapturedRawData((uint8_t *)picture->data);
		}
		mDataCb(CAMERA_MSG_RAW_IMAGE, picture, 0, NULL, mCallbackCookie);
		if(picture != NULL) picture->release(picture);
	}

	if (mMsgEnabled & CAMERA_MSG_RAW_IMAGE_NOTIFY) {
		DBUG("CAMERA_MSG_RAW_IMAGE_NOTIFY callback.");
		mNotifyCb(CAMERA_MSG_RAW_IMAGE_NOTIFY, 0, 0, mCallbackCookie);
	}

	if(mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE) {
		DBUG("data(jpeg) callback.");
		if(jpeg_size > 0) {
			picture = mRequestMemory(-1, jpeg_size, 1, NULL);
			if(picture && picture->data) {
				v4L2Camera->getCapturedJpegData((uint8_t *)picture->data);
			}
			mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, picture, 0, NULL, mCallbackCookie);
		} else {
			mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, NULL, 0, NULL, mCallbackCookie);
		}
		if(picture != NULL) picture->release(picture);
	}

	return NO_ERROR;
}

status_t TelechipsCameraHardware::beginVideoSnapshotThread(void *cookie)
{
    TelechipsCameraHardware *c = (TelechipsCameraHardware *)cookie;
    return c->videoSnapshotThread();
}

status_t TelechipsCameraHardware::takePicture()
{
	if(mRecordRunning) {
		DBUG("call to recording snap-shot.");
		if(createThread(beginVideoSnapshotThread, this) == false) {
			ALOGE("video snapshot create thread FAIL!!");
			return -1;
		}
	} else {
		#if defined(FEATURE_DESTROY_PREVIEW_THREAD)
			destoryPreviewThread(); 
		#endif
	
		DBUG("create picture thread for capture.");
		if(createThread(beginPictureThread, this) == false) {
			ALOGE("T:// takePicture CreateThread FAIL!! ");
			return -1;
		}
	}

	return NO_ERROR;
}

status_t TelechipsCameraHardware::cancelPicture()
{
	FUNCTION_IN;
	disableMsgType(CAMERA_MSG_RAW_IMAGE);
	disableMsgType(CAMERA_MSG_COMPRESSED_IMAGE);
	FUNCTION_OUT;
    return NO_ERROR;
}

uint8_t TelechipsCameraHardware::is_PixelFormat_Rgb565()
{
	if(strcmp(mParameters.getPreviewFormat(), "rgb565") == 0)
		return 1;
	else
		return 0;
}

status_t TelechipsCameraHardware::dump(int fd) const
{
    return NO_ERROR;
}

int TelechipsCameraHardware::getParm(const char *parm_str, const struct str_map *const parm_map)
{
	// check if the parameter exists.
	const char *str = mParameters.get(parm_str);

	if(str == NULL) return -1;

	// look up the parameter value.
	return attr_lookup(parm_map, str);
}

void TelechipsCameraHardware::setParm()
{
	FUNCTION_IN
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	int value;

	if(mParameters.getInt("jpeg-quality") != v4L2Camera->mJpegQuality)
		v4L2Camera->setJPEGQuality(mParameters.getInt("jpeg-quality"));

	if((value = getParm("effect", effect)) != -1)
		v4L2Camera->setParameter(V4L2_CID_EFFECT, value);

	if((value = getParm("whitebalance", whitebalance)) != -1)
		v4L2Camera->setParameter(V4L2_CID_AUTO_WHITE_BALANCE, value);

	if((value = mParameters.getInt("exposure-compensation")) != -1)
		v4L2Camera->setParameter(V4L2_CID_EXPOSURE, value);

	if((value = mParameters.getInt("zoom")) != -1)
		v4L2Camera->setParameter(V4L2_CID_ZOOM, value);

	if((value = mParameters.getInt("preview-frame-rate")) != -1) {		
		int cur_cnt, max_cnt;
		int target_value;

		max_cnt = 5;
		cur_cnt = 5;
		target_value = 0;

		while(cur_cnt > 0) {            
			if(mNormalFps/cur_cnt >= value) {
				target_value = cur_cnt;
				DBUG("1 inFps %d/%d -> %d :: %d/%d", value, mNormalFps, target_value, cur_cnt, max_cnt); 
				break;
			}
			cur_cnt--;
		}

        if(target_value > 1) 	target_value -= 1;
        else 					target_value = 0;
        DBUG("2 inFps %d/%d -> %d :: %d/%d", value, mNormalFps, target_value, cur_cnt, max_cnt); 
        
		if(0) { // if(v4L2Camera->get_cameraType_isUVC()) {
			v4L2Camera->set_fps(mNormalFps/cur_cnt);
		} else {
			if(mFps_mode != target_value) {
				mFps_mode = target_value;
				mFps_OnTime = 1;//mFps_mode + 1; for V2IP
				ALOGD("inFps %d/%d => skip mode = %d", value, mNormalFps, mFps_mode);
			}
		}
	}
	FUNCTION_OUT
}

status_t TelechipsCameraHardware::setParameters(const char* parameters)
{
	CameraParameters params;

	String8 str_params(parameters);
	params.unflatten(str_params);

	return setParameters(params);
}

status_t TelechipsCameraHardware::setParameters(const CameraParameters& params)
{
	FUNCTION_IN
	const char *str = NULL;
	int minFPS, maxFPS;
	int preview_W, preview_H;
	Mutex::Autolock lock(mLock);

	// verify params
	if( strcmp(params.getPreviewFormat(), "yuv422p") != 0 &&
		strcmp(params.getPreviewFormat(), "yuv420sp") != 0 &&
		strcmp(params.getPreviewFormat(), "yuv420p") != 0 &&
		strcmp(params.getPreviewFormat(), "rgba8888") != 0 &&
		strcmp(params.getPreviewFormat(), "rgb565") != 0 ) {
		ALOGE("Only yuv422p preview is supported");
		return -1;
	}
	//[add for ICS cts
	 if((str = params.get("preview-size")) != NULL) {
		params.getPreviewSize(&preview_W, &preview_H);
		if(preview_W <= 0 || preview_H <= 0)
		{
			ALOGE("Preview size error");
			return -EINVAL;
		}
		//mParameters.set("video-size-values", str);	
	 }
	//]
	if(strcmp(params.getPictureFormat(), "jpeg") != 0) {
		ALOGE("Only jpeg still pictures are supported");
		return -1;
	}

	if(params.getInt("zoom") > params.getInt("max-zoom")) return BAD_VALUE;

    if((str = params.get("preview-fps-range")) != NULL) {
        params.getPreviewFpsRange(&minFPS, &maxFPS);
        if((0 > minFPS) || (0 > maxFPS)) {
             ALOGE("FPS Range is negative! minFPS(%d), maxFPS(%d)", minFPS, maxFPS);
             return -EINVAL;
        }
		
        minFPS /= TelechipsCameraHardware::VFR_SCALE;
        maxFPS /= TelechipsCameraHardware::VFR_SCALE;

        if((0 == minFPS) || (0 == maxFPS)) {
             ALOGE("FPS Range is invalid!");
             return -EINVAL;
        }
		
        if(maxFPS < minFPS) {
             ALOGE("Max FPS is smaller than Min FPS!");
             return -EINVAL;
        }

        mParameters.set("preview-fps-range", str);                
    }
	//[add for ICS cts
	if((str = params.get("flash-mode")) != NULL)
	{
		if(strcmp(str,"invalid")== 0)
		{
			return -EINVAL;
		}
	}
	
	if((str = params.get("focus-mode")) != NULL)
	{
		if(strcmp(str, "invalid")== 0)
		{
			return -EINVAL;
		}
	}
	//]
	mParameters = params;
	
#if 0  // if case Camera Zoom Max ratio diff. replaced switching camera zoom value to switched camera zoom max value

	if(mParameters.getInt("zoom") > mParameters.getInt("max-zoom")){  //20130520 swhwang, fixed for Dual Camera mismatch between zoom ratio value and supported max zoom value, at Camera switching
		char str[15];
		sprintf(str, "%d", mParameters.getInt("max-zoom"));
		mParameters.set("zoom", str);
		ALOGE("zoom value > max zoom value");
	}
	
#endif

	setParm();
	initHeapLocked();

	if((str = params.get("gps-altitude")) != NULL)
		mParameters.set("gps-altitude", str);

	if((str = params.get("gps-latitude")) != NULL)
		mParameters.set("gps-latitude", str);

	if((str = params.get("gps-longitude")) != NULL)
		mParameters.set("gps-longitude", str);

	if((str = params.get("gps-timestamp")) != NULL)
		mParameters.set("gps-timestamp", str);

	if((str = params.get("gps-processing-method")) != NULL)
		mParameters.set("gps-processing-method", str);

	FUNCTION_OUT;
	return NO_ERROR;
}

char* TelechipsCameraHardware::getParameters()
{
	FUNCTION_IN
	String8 params_str8;
	char* params_string;
	const char* valstr = NULL;
	CameraParameters mParams = mParameters;

	Mutex::Autolock lock(mLock);

	// do not send internal parameters to upper layers
	mParams.remove("recording-hint");

	// camera service frees this string...
	params_str8 = mParams.flatten();
	params_string = (char*) malloc(sizeof(char) * (params_str8.length()+1));
	strcpy(params_string, params_str8.string());

	FUNCTION_OUT;
	return params_string;
}


void TelechipsCameraHardware::putParameters(char *parms)
{
	free(parms);
}

status_t TelechipsCameraHardware::sendCommand(int32_t command, int32_t arg1, int32_t arg2)
{
	return BAD_VALUE;
}

void TelechipsCameraHardware::release()
{
	FUNCTION_IN
}

#if(0) //20111205 ysseung   not used this function.
extern "C" int HAL_getNumberOfCameras()
{
	return sizeof(sCameraInfo) / sizeof(sCameraInfo[0]);
}

extern "C" void HAL_getCameraInfo(int cameraId, struct CameraInfo* cameraInfo)
{
	memcpy(cameraInfo, &sCameraInfo[cameraId], sizeof(CameraInfo));
}

extern "C" sp<CameraHardwareInterface> HAL_openCameraHardware(int cameraId)
{
	ALOGV("openCameraHardware: call createInstance");
	return TelechipsCameraHardware::createInstance(cameraId);
}
#endif

status_t TelechipsCameraHardware::setPreviewWindow(struct preview_stream_ops *window)
{
	FUNCTION_IN;
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	int ret = NO_ERROR;

	// if the camera service passes a null window, we destroy existing window.
	DBUG("window ops is 0x%x.", window);
	if(!window) {
		mCameraWindow = NULL;
	} else {
		mCameraWindow = window;
	
		if(v4L2Camera->mPreviewSizeChanged){
			ALOGD("!!!preview size changed. preview buffer in before removed!!!");
			mPreviewBuffers = (unsigned int *)freeCameraBuffers(CAM_STATUS_PREVIEW_START);
			mCreateCameraWindow = false;			
		}
		
		#if(0) //20111215 ysseung   capture 시에는 gralloc buffer를 사용하지 않고, pmap을 사용함.
		if(mCaptureBuffers != NULL) freeCameraBuffers(CAM_STATUS_CAPTURE);
		#endif
		if(!mPreviewBuffers) {
			DBUG("Preview Buffer is NULL, and create Preview Buffers.");
			allocPreviewBuffers(v4L2Camera->mPreviewWidth, v4L2Camera->mPreviewHeight, \
								mParameters.getPreviewFormat(), v4L2Camera->mRealKCount);
		} else {
			DBUG("Preview Buffer is not NULL.");
		}

		if(mPreviewBuffers) {
			Message msg;			
			mPreviewRunning = mCreateCameraWindow = true;
			
			msg.command = CAMERA_START;
			while(CameraCmdQ == NULL && CameraAckQ == NULL)
			{
				usleep(10*1000); // sleep is 10ms.
			}
			CameraCmdQ->put(&msg);
			CameraAckQ->get(&msg);

			ret = (msg.command == CAMERA_ACK) ? NO_ERROR: INVALID_OPERATION;			
		}
	}

	FUNCTION_OUT;
	return ret;
}

status_t TelechipsCameraHardware::storeMetaDataInBuffers(int enable)
{
	//return enable ? INVALID_OPERATION: OK;
	return NO_ERROR;
}

status_t TelechipsCameraHardware::allocPreviewBuffers(int width, int height, const char* format, int buffercount)
{
	FUNCTION_IN;
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	
	#if defined(FEATURE_SENSOR_STABLE_DELAY)
		v4L2Camera->mSensor_StableTime = true;
	#endif
	
	mPreviewLength = 0;
	mPreviewBuffers = (unsigned int *)allocateBuffer(width, height, format, mPreviewLength, buffercount);
	DBUG("mPreviewBuffer=0x%08x, mPreviewLength=%d.", mPreviewBuffers, mPreviewLength);

	if(!mPreviewBuffers) {
		ALOGE("Couldn't allocate preview buffers.");
		return NO_MEMORY;
	}

	FUNCTION_OUT;
	return NO_ERROR;
}

status_t TelechipsCameraHardware::allocCaptureBuffers(int width, int height, const char* format, int buffercount)
{
	FUNCTION_IN
	mCaptureLength = 0;
	mCaptureBuffers = (unsigned int *)allocateBuffer(width, height, format, mCaptureLength, buffercount);

	DBUG("mCaptureBuffers=0x%08x. mCaptureLength=%d.", mCaptureBuffers, mCaptureLength);
	if(!mCaptureBuffers) {
		ALOGE("Couldn't allocate capture buffers.");
		return NO_MEMORY;
	}

	FUNCTION_OUT;
	return NO_ERROR;
}

void* TelechipsCameraHardware::allocateBuffer(int width, int height, const char* format, int &bytes, int numBufs)
{
	FUNCTION_IN;
	int i = -1, err = -1, undequeueBufferCount = 0, actualBufferCount = numBufs;
	GraphicBufferMapper &mapper = GraphicBufferMapper::get();
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	Rect bounds;

	v4L2Camera->mBufferHandle = new buffer_handle_t*[actualBufferCount];
	v4L2Camera->mGrallocHandle = new buffer_handle_t*[actualBufferCount];
	if((v4L2Camera->mBufferHandle == NULL) || (v4L2Camera->mGrallocHandle == NULL)) {
		ALOGE("Couldn't create array for preview window buffers");
		FUNCTION_OUT;
		return NULL;
	}

	// set gralloc usage bits for window.
	err = mCameraWindow->set_usage(mCameraWindow, TCC_CAMHAL_GRALLOC_USAGE);
	if(err != 0) {
		ALOGE("native_window_set_usage failed: %s (%d)", strerror(-err), -err);
		if(err == ENODEV) {
			ALOGE("Preview surface abandoned!");
			mCameraWindow = NULL;
		}
		return NULL;
	}

	// set the number of buffers needed for camera preview
	err = mCameraWindow->set_buffer_count(mCameraWindow, actualBufferCount);
	if(err != 0) {
		ALOGE("native_window_set_buffer_count failed: %s (%d)", strerror(-err), -err);
		if(err == ENODEV) {
			ALOGE("Preview surface abandoned!");
			mCameraWindow = NULL;
		}
		return NULL;
	}
	
	DBUG("Configuring %d buffers for window", actualBufferCount);

	// set window geometry for camera preview
	#if defined(CONFIG_VIDEO_HDMI_IN_SENSOR) && defined(FEATURE_WDMA_RGB_MODE)
		#if defined(FEATURE_RGB_MODE_565)
			err = mCameraWindow->set_buffers_geometry(mCameraWindow, width, height, HAL_PIXEL_FORMAT_RGB_565);   //RGB565
		#else
			err = mCameraWindow->set_buffers_geometry(mCameraWindow, width, height, HAL_PIXEL_FORMAT_BGRA_8888); //aRGB8888
		#endif
	#else
		err = mCameraWindow->set_buffers_geometry(mCameraWindow, width, height, HAL_PIXEL_FORMAT_YV12);
	
	#endif

	if(err != 0) {
		ALOGE("native_window_set_buffers_geometry failed: %s (%d)", strerror(-err), -err);
		if(err == ENODEV) {
			ALOGE("Preview surface abandoned!");
			mCameraWindow = NULL;
		}
		return NULL;
	}

	for(i=0; i < actualBufferCount; i++) {
		buffer_handle_t** hndl2hndl;
		buffer_handle_t* handle;

		err = mCameraWindow->dequeue_buffer(mCameraWindow, (buffer_handle_t**)&hndl2hndl, &bytes);
		if(err != 0) {
			ALOGE("native_window_dequeue_buffer failed: %s (%d)", strerror(-err), -err);
			if(err == ENODEV) {
				ALOGE("Preview surface abandoned!");
				mCameraWindow = NULL;
			}
			goto fail;
		}

		handle = *hndl2hndl;
		v4L2Camera->mBufferHandle[i]  = (buffer_handle_t*) hndl2hndl;
		v4L2Camera->mGrallocHandle[i] = (buffer_handle_t*) handle;
		DBUG("mBufferHandle[%d]=0x%x, mGrallocHandle[%d]=0x%x.", i, v4L2Camera->mBufferHandle[i], i, v4L2Camera->mGrallocHandle[i]);
	}

	// lock the initial queueable buffers
	bounds.left 	= 0;
	bounds.top		= 0;
	bounds.right	= width;
	bounds.bottom	= height;

	mCameraWindow->get_min_undequeued_buffer_count(mCameraWindow, &undequeueBufferCount);
	for(i=0;  i < (actualBufferCount - undequeueBufferCount); i++) {
		mCameraWindow->lock_buffer(mCameraWindow, v4L2Camera->mBufferHandle[i]);
		mapper.lock((buffer_handle_t) v4L2Camera->mGrallocHandle[i], TCC_CAMHAL_GRALLOC_USAGE, bounds, &v4L2Camera->mCameraBufferAddr[i]);
		DBUG("locked mBufferHandle[%d]=0x%x, mGrallocHandle[%d]=0x%x.", i, v4L2Camera->mBufferHandle[i], i, v4L2Camera->mGrallocHandle[i]);
		DBUG("gralloc phy_addr=0x%08x", v4L2Camera->mCameraBufferAddr[i]);
	}

	// return the rest of the buffers back to window
	for(i=(actualBufferCount - undequeueBufferCount); (i >= 0) && (i < actualBufferCount); i++) {
		err = mCameraWindow->cancel_buffer(mCameraWindow, v4L2Camera->mBufferHandle[i]);
		if(err != 0) {
			ALOGE("native_window_cancel_buffer failed: %s (%d)", strerror(-err), -err);
			if(err == ENODEV) {
				ALOGE("Preview surface abandoned!");
				mCameraWindow = NULL;
			}
			goto fail;
		}
		DBUG("cancel mBufferHandle[%d]=0x%x, mGrallocHandle[%d]=0x%x.", i, v4L2Camera->mBufferHandle[i], \
																		i, v4L2Camera->mGrallocHandle[i]);
		mapper.unlock((buffer_handle_t) v4L2Camera->mGrallocHandle[i]);
		v4L2Camera->mBufferHandle[i] = v4L2Camera->mGrallocHandle[i] = NULL;
	}

	v4L2Camera->mPreviewBufferCount = (actualBufferCount - undequeueBufferCount);
	DBUG("allocate total buffer %d.", v4L2Camera->mPreviewBufferCount);

	FUNCTION_OUT
	return v4L2Camera->mGrallocHandle;

fail:
	// need to cancel buffers if any were dequeued
	for(int start=0; (start < i) && (i > 0); start++) {
		int ret = mCameraWindow->cancel_buffer(mCameraWindow, v4L2Camera->mBufferHandle[start]);
		if(ret != 0) {
			ALOGE("native_window_cancel_buffer failed: %s (%d)", strerror(-ret), -ret);
			break;
		}
		DBUG("Cancel mBufferHandle[%d]=0x%x, mGrallocHandle[%d]=0x%x.", start, v4L2Camera->mBufferHandle[start], \
																		start, v4L2Camera->mGrallocHandle[start]);
		mapper.unlock((buffer_handle_t) v4L2Camera->mGrallocHandle[start]);
		v4L2Camera->mBufferHandle[start] = v4L2Camera->mGrallocHandle[start] = NULL;
	}

	delete v4L2Camera->mBufferHandle;
	delete v4L2Camera->mGrallocHandle;
	v4L2Camera->mBufferHandle = NULL;
	v4L2Camera->mGrallocHandle = NULL;

	FUNCTION_OUT;
	return NULL;
}

void TelechipsCameraHardware::lockCurrentPreviewBuffer(int index)
{
	//FUNCTION_IN
	GraphicBufferMapper &mapper = GraphicBufferMapper::get();
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	buffer_handle_t** hndl2hndl;
	buffer_handle_t* handle;
	int err, stride;
	Rect bounds;

	err = mCameraWindow->dequeue_buffer(mCameraWindow, (buffer_handle_t**) &hndl2hndl, &stride);
//	DBUG("dequeue buffer Index = %d, Addr = 0x%x",index,(buffer_handle_t*) hndl2hndl);
	if(err != 0) {
		ALOGE("%s:  dequeueBuffer failed: %s (%d)", __FUNCTION__, strerror(-err), -err);
		if(err == ENODEV) {
			ALOGE("%s:  Preview surface abandoned!", __FUNCTION__);
			mCameraWindow = NULL;
		}

			return;
	}

	handle = *hndl2hndl;
	v4L2Camera->mBufferHandle[index]  = (buffer_handle_t*) hndl2hndl;
	v4L2Camera->mGrallocHandle[index] = (buffer_handle_t*) handle;

	err = mCameraWindow->lock_buffer(mCameraWindow, v4L2Camera->mBufferHandle[index]);
	if(err != 0) {
		ALOGE("%s:  lockbuffer failed: %s (%d)", __FUNCTION__, strerror(-err), -err);
		if(err == ENODEV) {
			ALOGE("%s:  Preview surface abandoned!", __FUNCTION__);
			mCameraWindow = NULL;
		}
	}

	if(err != 0) {
		ALOGE("%s:  lockbuffer failed: %s (%d)", __FUNCTION__, strerror(-err), -err);
		return;
	}

	mapper.lock((buffer_handle_t) v4L2Camera->mGrallocHandle[index], TCC_CAMHAL_GRALLOC_USAGE, \
				bounds, &v4L2Camera->mCameraBufferAddr[index]);

	// send to address for kernel.
	v4L2Camera->camif_setCameraAddress(index, MODE_PREVIEW);

	//FUNCTION_OUT
}

void* TelechipsCameraHardware::freeCameraBuffers(unsigned char cameraStatus)
{
	FUNCTION_IN
	TCC_V4l2_Camera* v4L2Camera = mV4l2Camera;
	GraphicBufferMapper &mapper = GraphicBufferMapper::get();
	int i, count;

	if(cameraStatus == CAM_STATUS_CAPTURE) {
		count = v4L2Camera->mCaptureBufferCount;
	} else {
		count = v4L2Camera->mPreviewBufferCount;
	}
	DBUG("camera status = %d, buffer count = %d.", cameraStatus, count);
	ALOGD("camera status = %d, buffer count = %d.", cameraStatus, count);
	for(i=0; i < count; i++) {
		int ret = mCameraWindow->cancel_buffer(mCameraWindow, v4L2Camera->mBufferHandle[i]);
		if(ret != 0) {
			ALOGE("native_window_cancel_buffer failed: %s (%d)", strerror(-ret), -ret);
			ALOGD("native_window_cancel_buffer failed: %s (%d)", strerror(-ret), -ret);
			break;
		}
		DBUG("Cancel mBufferHandle[%d]=0x%x, mGrallocHandle[%d]=0x%x.", i, v4L2Camera->mBufferHandle[i], \
																		i, v4L2Camera->mGrallocHandle[i]);
		mapper.unlock((buffer_handle_t) v4L2Camera->mGrallocHandle[i]);
		v4L2Camera->mBufferHandle[i] = v4L2Camera->mGrallocHandle[i] = NULL;
	}

	delete v4L2Camera->mBufferHandle;
	delete v4L2Camera->mGrallocHandle;
	v4L2Camera->mBufferHandle = NULL;
	v4L2Camera->mGrallocHandle = NULL;

	FUNCTION_OUT
	return v4L2Camera->mGrallocHandle;
}

}; // namespace android



