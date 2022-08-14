#ifndef __TCCCAMERA_H__
#define __TCCCAMERA_H__

#include <camera/ICamera.h>
#include <camera/Camera.h>
#include <surfaceflinger/Surface.h>
#include <camera/CameraParameters.h>
#include <media/stagefright/CameraSource.h>
#include <common.h>

using namespace android;

#define MAX_CLOCK_V2IP_ENABLE 105
#define MAX_CLOCK_V2IP_DISABLE 104

#define MAX_FPS_COUNT (30 + 1)

class CameraCtrl
{
public:
	CameraCtrl();
	~CameraCtrl();

	status_t configure_camera(const sp<Surface> pSurface, int frame_width, int frame_height, int default_framerate);
	status_t stop_camera();
	status_t setFramerate_camera(int fps);
	int getFramerate_camera();
	
	sp<CameraSource> pCameraMedia;

private:
	sp<Camera> pCamera;
	CameraParameters *mCameraParams;

	bool mStarted;	
	int mFrameRate;
	int mAvailableFps[MAX_FPS_COUNT];
};

#endif
