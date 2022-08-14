#ifndef __TCCCAMERA_H__
#define __TCCCAMERA_H__

#include <camera/ICamera.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>
#include <gui/Surface.h>
#include <libpmap/pmap.h>
#include "common_if.h"

using namespace android;

#define MAX_CLOCK_V2IP_ENABLE 105
#define MAX_CLOCK_V2IP_DISABLE 104

#define MAX_FPS_COUNT (30 + 1)


class CameraIf;

class CameraIfListener : public CameraListener
{
public:
	CameraIfListener(CameraIf *input): mCameraIf(input) {}
    virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2) {};
    virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata) {};
    virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr);
	void release()
	{
		mCameraIf = NULL;
	}

protected:
	~CameraIfListener() {};
	
private:
	CameraIf *mCameraIf;
};

// Callback class used to pass camera frames to the caller.
class CameraFrameCallBack
{
public:
    virtual void CameraFrameToCaller(sp<IMemory> dataPtr, int64_t timestamp_us) = 0;
    virtual ~CameraFrameCallBack() {}	
};

class CameraIf
{
public:
/*!
 *************************************************************************************************
 * \brief
 *		CameraIf/~CameraIf	: Constructor / Destructor of Renderer
 * \param 0
 *		[in] callback		: The callback provided by external will be called whenever having frame after Camera start.
 *************************************************************************************************
 */
	CameraIf(CameraFrameCallBack *callback);
	~CameraIf();

/*!
 *************************************************************************************************
 * \brief
 *		Init	: to initialize Camera.
 * \param 0
 *		[in] pSurface			: *This surface pointer has to be provided although the frame from Camera don't want to be displayed by itself
 *                                If the frame from Camera don't want to be displayed by itself, call Start() function with no_preview, "1".
 *		[in] frame_width		: Camera Frame Width 
 *		[in] frame_height		: Camera Frame Height 
 *		[in] default_framerate	: Camera Frame Rate
 *		[in] no_preview			: If the frame from Camera don't want to display by itself, call Start() function with no_preview, "1".
 *									Otherwise, set the value into "0".
 *		[in] clientName		: package name
 * \return
 *		If successful, Init() returns 0 or plus. Otherwise, it returns a minus value.
 *************************************************************************************************
 */
	int Init(const sp<IGraphicBufferProducer> pSurface, int frame_width, int frame_height, int default_framerate, int no_preview, const String16& clientName);

/*!
 *************************************************************************************************
 * \brief
 *		Start	: to start Camera.
 * \return
 *		If successful, Start() returns 0 or plus. Otherwise, it returns a minus value.
 *************************************************************************************************
 */
    int Start();

/*!
 *************************************************************************************************
 * \brief
 *		Stop	: to stop Camera.
 * \return
 *		If successful, Stop() returns 0 or plus. Otherwise, it returns a minus value.
 *************************************************************************************************
 */
	int Stop();

/*!
 *************************************************************************************************
 * \brief
 *		ReleaseFrame	: to release it after frame from Camera is encoded.
 * \param 0
 *		[in] frame	: This pointer is same with received one from callback provided when calling constuctor of this class.
 *************************************************************************************************
 */
	void ReleaseFrame(const sp<IMemory>& frame);

/*!
 *************************************************************************************************
 * \brief
 *		SetFramerate	: to change framerate
 * \param 0
 *		[in] fps	: framerate value to change.
 * \return
 *		If successful, return fps value applied actually.
 *			- (The applied fps can be differed with given one because the Camera can't support all kinds of framerate.)
 *		Use this returned value for encoder because same fps value have to set between camera and encoder.
 *************************************************************************************************
 */
	int SetFramerate(int fps);

/*!
 *************************************************************************************************
 * \brief
 *		GetVirtual_Address	: to get virtual address that can be got from memory pointer given by Callback like below:
 *		-	enc_metadata_t* pEncMetadataInfo = (enc_metadata_t*)dataPtr->pointer();
 *			unsigned int phyAddr = (unsigned int)pEncMetadataInfo->fd_0;
 *			Above phyAddr can be used for Encoder and Mixer.
 * \param 0
 *		[in] phyAddr	: This address is what get from given frame's pointer by callback function when calling constructor of this class.
 * \return
 *		If successful, return virtual address.
 *************************************************************************************************
 */
	unsigned int GetVirtual_Address(unsigned int phyAddr);

/*!
 *************************************************************************************************
 * \brief
 *		GetFrame_Format	: to get frame format from camera.
 * \return
 *		If successful, return format value.
 *************************************************************************************************
 */
	tFRAME_BUF_FORMAT GetFrame_Format(void);

private:
   	friend class CameraIfListener;

	CameraFrameCallBack *fCb_CamFrameReady;
	
	sp<android::Camera> mCamera;
	sp<CameraIfListener> mListener;
	CameraParameters *mCameraParams;

    Mutex mLock;

	bool mStarted;
	int mFrameRate;
	int mAvailableFps[MAX_FPS_COUNT];
	int mSupported_Hdmi_forCamera;
	
	tFRAME_BUF_FORMAT  mColorFormat;
	uint  mFrameWidth;
	uint  mFrameHeight;

    int mDev_fd;
    void* mMap_addr;
    pmap_t mCamPmap;

	bool _mem_prepare();
	void _mem_destroy();
	tFRAME_BUF_FORMAT _getColorFormat(const char* colorFormat);
	void _dataCallbackTimestamp(int64_t timestampUs, const sp<IMemory> &data);  
};

#endif
