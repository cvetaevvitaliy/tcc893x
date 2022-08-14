#ifndef __TCCCAMERA_H__
#define __TCCCAMERA_H__

#include <camera/ICamera.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>
#include <utils/List.h>
#include <utils/threads.h>
#include <gui/Surface.h>
#include <libpmap/pmap.h>

using namespace android;

#define MAX_CLOCK_V2IP_ENABLE 105
#define MAX_CLOCK_V2IP_DISABLE 104

#define MAX_FPS_COUNT (30 + 1)


class CameraCtrl;

class CameraCtrlListener : public CameraListener
{
public:
	CameraCtrlListener(CameraCtrl *input): mCameraCtrl(input) {}
    virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2) {};
    virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata) {};
    virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr);
	void release()
	{
		mCameraCtrl = NULL;
	}

protected:
	~CameraCtrlListener() {};
	
private:
	CameraCtrl *mCameraCtrl;
};

class CameraCtrl : public MediaSource, public MediaBufferObserver 
{
public:
	CameraCtrl();
	~CameraCtrl();

	status_t configure_camera(const sp<IGraphicBufferProducer> pSurface, int frame_width, int frame_height, int default_framerate, int no_preview, const String16& clientName);
    virtual status_t start(MetaData *params = NULL);
	virtual sp<MetaData> getFormat();

	int stop();
	virtual status_t read(MediaBuffer **buffer, const ReadOptions *options = NULL);
	virtual void signalBufferReturned(MediaBuffer* buffer);
	status_t setFramerate_camera(int fps);
	int getFramerate_camera();
	status_t get_YUV420p_Frame(uint8_t *frame, uint32_t len, uint8_t *width, uint8_t *height);
	status_t replace_YUV420p_Frame(uint8_t *frame, uint32_t len, bool enable);

private:
   	friend class CameraCtrlListener;
	
	sp<android::Camera> pCamera;
	sp<MetaData> pMeta;
	sp<CameraCtrlListener> pListener;
	CameraParameters *mCameraParams;
	int mSupported_Hdmi_forCamera;

    Mutex mLock;
    Condition mFrameAvailableCondition;
    Condition mFrameCompleteCondition;
    List<sp<IMemory> > mFramesReceived;
    List<sp<IMemory> > mFramesBeingEncoded;
	List<int64_t> mFrameTimes;

	int64_t mStartTimeUs;
	int32_t mNumFramesReceived;
	int64_t mFirstFrameTimeUs;
	int64_t mLastFrameTimestampUs;
	int32_t mNumFramesEncoded;
	int32_t mNumFramesDropped;

	bool mStarted;
	int mFrameRate;
	int mAvailableFps[MAX_FPS_COUNT];

	int  mColorFormat;
	uint  mFrameWidth;
	uint  mFrameHeight;
	bool mEnabled_FrmReplacement;
	int mBeforeFps;
	int mAfterFps;
	char *mBuffer_FrmReplacement;

	bool mEnabled_FrmCapture;
	bool mCaptured_Frm;
	bool mFailed_Capture_Frm;
	char *mBuffer_FrmCapture;

    int mDev_fd;
    void* mMap_addr;
    pmap_t mCamPmap;

	bool mem_prepare();
	void mem_destroy();
	unsigned int mem_get_virtAddr(unsigned int phyAddr);

	void dataCallbackTimestamp(int64_t timestampUs, int32_t msgType, const sp<IMemory> &data);
	void releaseQueuedFrames();
	void releaseOneRecordingFrame(const sp<IMemory>& frame);
  
};

#endif
