/*
 * Copyright (C) 2011 Telechips, Inc.
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
 *
 * Date: 2011/12/08
 * Android revised by ZzaU.
 */

#include <mach/tcc_grp_ioctrl.h>
#include <mach/tcc_scaler_ioctrl.h>
#include <mach/tcc_composite_ioctl.h>
#include <mach/tcc_component_ioctl.h>
#include <mach/tcc_overlay_ioctl.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_viqe_ioctl.h>
#include <mach/tcc_mem_ioctl.h>

#include <cutils/properties.h>
#include <libpmap/pmap.h>
#include <stdlib.h>
#include <hardware/hwcomposer.h>

#include <linux/fb.h>
#include <mach/tcc_video_private.h>

#include <libhdmi/libhdmi.h>
#include <libhpd/libhpd.h>
#include <libcec/libcec.h>

#define OVERLAY_0_DEVICE	"/dev/overlay"
#define OVERLAY_1_DEVICE	"/dev/overlay1"
#define GRAPHIC_DEVICE		"/dev/graphic"
#define SCALER0_DEVICE		"/dev/scaler"
#define SCALER1_DEVICE		"/dev/scaler1"
#define FB0_DEVICE			"/dev/graphics/fb0"
#define COMPOSITE_DEVICE 	"/dev/composite"
#define COMPONENT_DEVICE 	"/dev/component"
#define VIQE_DEVICE			"/dev/viqe"
#define TMEM_DEVICE			"/dev/tmem"
#define CLK_CTRL_DEVICE		"/dev/clockctrl"
#define HDMI_DRVICE			"/dev/hdmi"
#define HDMI_CEC_DEVICE		"/dev/cec"
#define HDMI_HPD_DEVICE		"/dev/hpd"

#define VIDEO_PLAY_SCALER			SCALER0_DEVICE
#define VIDEO_CAPTURE_SCALER 		SCALER1_DEVICE

#define VSYNC_OVERLAY_COUNT 6

#define HDMI_DEC_MAX_WIDTH	1920
#define HDMI_DEC_MAX_HEIGHT	1080

#if defined(_TCC8920_) || defined(_TCC8930_)
#define LCDC_VIDEO_CHANNEL		LCDC_LAYER_2
#else
#define LCDC_VIDEO_CHANNEL		LCDC_LAYER_0
#endif//

#if defined(_TCC8800_)
#define USE_LIMITATION_SCALER_RATIO
#endif

#define RET_RETRY_THIS_FRAME 10

//#ifndef USE_LCD_VIDEO_VSYNC
//#define USE_LCD_OTF_SCALING // To support On-the-fly mode Only for Progressive contents in case of no-rotation.
//#endif

#define TCC_CLOCKCTRL_HWC_DDI_CLOCK_DISABLE 116
#define TCC_CLOCKCTRL_HWC_DDI_CLOCK_ENABLE 117

typedef struct
{
	unsigned int YaddrPhy;
	unsigned int UaddrPhy;
	unsigned int VaddrPhy;
#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
	unsigned int timeStamp;
	unsigned int curTime;
	unsigned int buffer_id;
#endif
	unsigned int flags;
	unsigned int frameView;

	unsigned int MVC_Base_YaddrPhy;
	unsigned int MVC_Base_UaddrPhy;
	unsigned int MVC_Base_VaddrPhy;

}output_overlay_param_t;

typedef enum{
	OUTPUT_LCDC_0,
	OUTPUT_LCDC_1,
	OUTPUT_LCDC_MAX
}TCC_OUTPUT_LCDC_TYPE;

typedef struct
{
	unsigned char lcdc;
	unsigned char mode;
} TCC_OUTPUT_START_TYPE;

typedef struct 
{
	unsigned int sx;
	unsigned int sy;
	unsigned int width;
	unsigned int height;
	unsigned int format;
	unsigned int transform;
}pos_info_t;

typedef struct 
{
	unsigned int sx;
	unsigned int sy;
	unsigned int srcW;
	unsigned int srcH;
	unsigned int dstW;
	unsigned int dstH;
}crop_info_t;


typedef struct
{
	unsigned int left;
	unsigned int top;
	unsigned int right;
	unsigned int bottom;
}region_info_t;

typedef struct
{
	region_info_t source;
	region_info_t display;
}region_info_detail_t;

typedef struct
{
	unsigned int	mScreenModeIndex;
	bool			mScreenMode;
	int				mOutputMode; //0 : lcd , 1 : external display , 2 : dual(lcd  + external display)
	bool			mHDMIOutput;
	bool			mCompositeOutput;
	bool			mComponentOutput;
	unsigned int	mCompositeMode;
	unsigned int	mComponentMode;

	//video display mode related with vsync
    int 			mVsyncMode;
	bool			mVSyncSelected;
	unsigned int	mUnique_address;
	// video display mode check onthefly
	int				mOntheflymode;
	//video display deinterlace mode
    int 			mVideoDeinterlaceFlag;
	int             mVideoDeinterlaceScaleUpOneFieldFlag;
    int 			mVideoDeinterlaceSupport;
	int 			mVideoDeinterlaceForce;
	int 			mVideoDeinterlaceToMemory;

	//Variable
	int				mOverlay_fd;
	int				mOverlay_ch;
	int 			mRt_fd;
	int 			mM2m_fd;
	int 			mFb_fd;
	int 			mComposite_fd;
	int 			mComponent_fd;
//	int 			mJpegEnc_fd;
//	int 			mJpegDec_fd;
	int 			mViqe_fd;
	bool			bUseMemInfo;
	int				mClkctrl_fd;
	int				mTmem_fd;
	void* 			mTMapInfo;
	pmap_t			mUmpReservedPmap;

	unsigned int 	mLcd_width;
	unsigned int 	mLcd_height;

	unsigned int 	mOutput_width;
	unsigned int 	mOutput_height;

    /*for stream original aspect ratio 
     *if -1(not define), aspect ratio is ratio for image wdith and height.
     */
    int 			mStreamAspectRatio; //-1:not define 0:16:9, 1:4:3

	//Overlay
	bool			mOverlay_recheck;
	int 			mOverlayBuffer_Cnt; // Total Count!!
	int 			mOverlayBuffer_Current;
	void*			mOverlayBuffer_PhyAddr[6];
	//Cropped region info.
	bool			mCropRgn_changed;	
	bool			mCropChanged;
	bool			mAdditionalCropSource;
	region_info_detail_t		mOrigin_RegionInfo;
	crop_info_t 	mTarget_CropInfo;
	
	unsigned int	mDispWidth; // Scaler OUT!!
	unsigned int	mDispHeight;
	unsigned int	mFinal_DPWidth; //Crop OUT!!
	unsigned int	mFinal_DPHeight;
	unsigned int	mFinal_stride;
	unsigned int	mRight_truncation;

	//Overlay :: Calculated Real_target to display!!
	pos_info_t		mOrigin_DispInfo;
	pos_info_t		mTarget_DispInfo;

	pos_info_t		mScreenMode_DispInfo;

	bool			mFirst_Frame;
	bool			mParallel;
	bool			mOrder_Parallel;
	SCALER_TYPE 	mScaler_arg;
	G2D_COMMON_TYPE mGrp_arg;
	bool			mNeed_scaler;
	bool			mNeed_transform;
	bool			mNeed_rotate;
	bool			mNeed_crop; 
	unsigned int 	mTransform;

	//Capture
//	bool			mNeed_Capture;
	
	//test
	unsigned int	mFrame_cnt;
	
	pmap_t			mOverlayPmap;
	pmap_t			mOverlayRotation_1st;
	pmap_t			mOverlayRotation_2nd;
	pmap_t			mDualDisplayPmap;
	
	bool 			mExclusive_Enable;
	bool 			mExclusive_Display;
	char			*mHDMIUiSize;	
	int				mDisplayOutputSTB;	
	char			*mComponentChip;

	unsigned int 	mBackupOutputModeIndex;
	unsigned int 	mOutputModeIndex;

	VIQE_DI_TYPE	mViqe_arg;

	bool			mVIQE_onoff;
	bool			mVIQE_onthefly;
	bool			mVIQE_DI_use;
	bool			mVIQE_OddFirst;
	int 			mVIQE_firstfrm;
	int 			mVIQE_m2mDICondition;
	unsigned int	mPrevY;
	unsigned int	mPrevU;
	unsigned int	mPrevV;

//	output_overlay_param_t mLast_frame;
//	bool mApplied_Overlay;
//	bool mThreadloop;
//	nsecs_t	mLatestDisplay_ns;
	// protected by mLock
	unsigned int	mFrame_rate;
	unsigned int	mOriginalHDMIResolution;
	bool			mChangedHDMIResolution;
	bool			mMVCmode;

	unsigned int	mMode_3Dui;
	
	struct tcc_lcdc_image_update extend_display;

	bool mIgnore_ratio;
	bool bShowFps;
	bool bSupport_lastframe;

	int mIdx_codecInstance;
}render_param_t;


class HwRenderer
{
public:
    HwRenderer( gralloc_module_t const *module,
				int overlay_ch,
			    int inWidth, int inHeight, int inFormat,
			    int outLeft, int outTop, int outRight, int outBottom, unsigned int transform,
			    int creation_from,
			    int numOfExternalVideo);
	~HwRenderer();
	bool initCheck() const { return mInitCheck; }
	
	int setCropRegion(int inWidth, int inHeight, int srcLeft, int srcTop, int srcRight, int srcBottom, int outLeft, int outTop, int outRight, int outBottom, unsigned int transform);
	int render(int fd_0, char* ghandle, int inWidth, int inHeight, int outLeft, int outTop, int outRight, int outBottom, unsigned int transform);
	void setNumberOfExternalVideo( int numOfExternalVideo, bool isPrepare);
	void setFBChannel( int numOLayers, bool isReset);

protected:
	int _openOverlay(void);
	int _closeOverlay(void);
	int _ioctlOverlay(int cmd, overlay_video_buffer_t* arg);
	int initDevice(int inWidth, int inHeight);
	void closeDevice();
	int getDisplaySize(OUTPUT_SELECT Output, tcc_display_size *display_size);
	int VIQEProcess(unsigned int PA_Y, unsigned int PA_U, unsigned int PA_V, unsigned int DestAddr);

	bool (HwRenderer::*OverlayCheckDisplayFunction)(bool, int, int, int, int, unsigned int);
	bool (HwRenderer::*OverlayCheckExternalDisplayFunction)(bool, int, int, int, int, unsigned int);

    int overlayInit(SCALER_TYPE *scaler_arg, G2D_COMMON_TYPE *grp_arg, uint8_t isOutRGB);
	bool overlayCheck(bool reCheck, int outSx, int outSy, int outWidth, int outHeight, unsigned int rotation);
	int overlayResizeExternalDisplay(int flag);
	int overlayResizeGetPosition(int flag);
	bool overlayCheckExternalDisplay(bool reCheck, int outSx, int outSy, int outWidth, int outHeight, unsigned int rotation);
	bool overlayCheckDisplayScreenMode(bool reCheck, int outSx, int outSy, int outWidth, int outHeight, unsigned int rotation);
	bool overlayCheckExternalDisplayScreenMode(bool reCheck, int outSx, int outSy, int outWidth, int outHeight, unsigned int rotation);
	void overlayScreenModeCalculate(float fCurrentCheckDisp, bool bCut);
	
    int overlayProcess(SCALER_TYPE *scaler_arg, G2D_COMMON_TYPE *grp_arg, unsigned int srcY, unsigned int srcU, unsigned int srcV, uint8_t *dest);
    int overlayProcess_Parallel(SCALER_TYPE *scaler_arg, G2D_COMMON_TYPE *grp_arg, unsigned int srcY, unsigned int srcU, unsigned int srcV, uint8_t *dest);
	bool get_private_info(TCC_PLATFORM_PRIVATE_PMEM_INFO *info, char* grhandle, bool isHWaddr, int fd_val, unsigned int width, unsigned int height);
	bool output_change_check(int outSx, int outSy, int outWidth, int outHeight, unsigned int rotation, bool *overlayCheckRet, bool interlaced);
	bool output_Overlay(output_overlay_param_t * pOverlayInfo, bool *overlayCheckRet);

	void setVsync();
	void clearVsync(bool able_use_vsync, bool prev_onthefly_mode);
	void setting_mode();
	
	void Set_MVC_Mode(TCC_PLATFORM_PRIVATE_PMEM_INFO* pPrivate_data, int inWidth, int inHeight, int outWidth, int outHeight);
	void Native_FramerateMode(TCC_PLATFORM_PRIVATE_PMEM_INFO* pPrivate_data, int inWidth, int inHeight, int outWidth, int outHeight);
	int digits10(unsigned int num);
	int intToStr(int num, int sign, char *buffer);
	bool check_valid_hw_availble(int fd_val);

private:
	bool mInitCheck;
	render_param_t *pMParm;
	gralloc_module_t const *grallocModule;
    unsigned int mDecodedWidth, mDecodedHeight;
	unsigned int mOutputWidth, mOutputHeight;
	unsigned int mInputWidth, mInputHeight;
	unsigned int mBox;
	unsigned int mCheckSync;
	int mNumberOfExternalVideo;
	int mCheckEnabledTCCSurface;
	int mVideoCount;
	bool mIsFBChannelOn;
	
	int mColorFormat;
	int mGColorFormat;
	int mBufferId;
	int mNoSkipBufferId;
	
    HwRenderer(const HwRenderer &);
    HwRenderer &operator=(const HwRenderer &);
};
