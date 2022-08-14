/**
 * Copyright (C) 2013, Telechips Limited
 * by ZzaU(B070371)
 */
 
#define LOG_NDEBUG 0
#define LOG_TAG "Modified_SF_Sample"

#include <media/stagefright/MediaSource.h>
#include <tccif_modified_stagefright_sample.h>
#include "utils/Log.h"
#include <cutils/properties.h>
#include <fcntl.h>

StageFrightIf_Ex::StageFrightIf_Ex()
{
    ALOGV("Create StageFrightIf_Ex");
}

StageFrightIf_Ex::~StageFrightIf_Ex()
{
    ALOGV("Destory StageFrightIf_Ex");
}

void StageFrightIf_Ex::interface_init()
{
    status_t result = OK;
    
    ALOGV("interface_init");
    
    // Connect to the OMX client
    result = mClient.connect();
    CHECK(OK == result);
    mIOMX = mClient.interface();

    pCameraCtrl     = new CameraCtrl();    
    mEncoderCtrl    = new EncoderCtrl();
    mDecoderCtrl    = new DecoderCtrl();
}

void StageFrightIf_Ex::interface_deinit()
{
    ALOGV("interface_deinit");

    delete mEncoderCtrl;
    mEncoderCtrl = NULL;
    delete mDecoderCtrl;
    mDecoderCtrl = NULL;
    mClient.disconnect();
    mIOMX = NULL;
}

void StageFrightIf_Ex::set_CameraSurface(sp<IGraphicBufferProducer> &surf, int w, int h, int bitrate, int framerate)
{
    ALOGV("set_CameraSurface");

    if( surf == NULL )
    {
        ALOGE("bufferProducer for video is NULL.");
        return;
    }

    mWidth  = w;
    mHeight = h;
    mBps    = mCurr_Bps = bitrate;
    mFps    = mCurr_Fps = framerate;
    pCameraSurface = surf;
}

void StageFrightIf_Ex::set_VideoSurface(sp<IGraphicBufferProducer> &surf)
{
    ALOGV("set_VideoSurface");

    if( surf == NULL )
    {
        ALOGE("bufferProducer for video is NULL.");
        return;
    }

    pVideoSurface = surf;
}

void StageFrightIf_Ex::start(const String16& clientName)
{
    ALOGV("start");

    if( pCameraSurface == NULL || pVideoSurface == NULL)
    {
        ALOGE("Not ready to start operation. please check Surface Pointer");
        return;
    }

    property_set("tcc.all.video.call.enable", "1");

    // Try to set the clock higher
    // To enable max-clock.
    int clockctrlFD = open("/dev/clockctrl", O_WRONLY);

    if(clockctrlFD > 0)
    {
        ioctl(clockctrlFD, MAX_CLOCK_V2IP_ENABLE, 0);
        ALOGI("Enable clock speed to maximum for V2IP");
        close(clockctrlFD);
    }
    
    interface_init();
    pCameraCtrl->configure_camera(pCameraSurface, mWidth, mHeight, mFps, 0, clientName);
    mEncoderCtrl->configure_encoder(mIOMX, pCameraCtrl, CODEC_FORMAT_H264, mWidth, mHeight, mBps, mFps, 5);
    mDecoderCtrl->setSurfaceTexture(pVideoSurface, 1);
    mDecoderCtrl->configure_decoder(mIOMX, mEncoderCtrl->pEncoderMedia);
    mDecoderCtrl->start_RenderThread();

    mStarted = true;
}

void StageFrightIf_Ex::stop()
{
    if( !mStarted )
        return;

    ALOGV("stop");

    mDecoderCtrl->stop_RenderThread(NULL);
    mDecoderCtrl->stop_decoder();
    mEncoderCtrl->stop_encoder();
    
    interface_deinit();
    property_set("tcc.all.video.call.enable", "0");

    int clockctrlFD = open("/dev/clockctrl", O_WRONLY);

    if(clockctrlFD > 0)
    {
        ioctl(clockctrlFD, MAX_CLOCK_V2IP_DISABLE, 0);
        ALOGI("Disable clock speed to maximum for V2IP");
        close(clockctrlFD);
    }

    mStarted = false;
}


void StageFrightIf_Ex::change_framerate(int fps)
{
    int final_fps;
    
    ALOGV("change_framerate");

    if( fps == 0)
    {
        if( mCurr_Fps == mFps )
            mCurr_Fps = mFps/2;
        else if( mCurr_Fps == mFps/2 )
            mCurr_Fps = mFps/3;
        else 
            mCurr_Fps = mFps;
    }
    else
        mCurr_Fps = fps;

    pCameraCtrl->setFramerate_camera(mCurr_Fps);
    final_fps = pCameraCtrl->getFramerate_camera();
    mCurr_Fps = final_fps;
    mEncoderCtrl->set_framerate(mCurr_Fps);
    
    ALOGI("%d fps", mCurr_Fps);
}

void StageFrightIf_Ex::change_bitrate(int Kbps)
{
    ALOGV("change_bitrate");
    
    if( Kbps == 0)
    {
        if( mCurr_Bps == mBps )
            mCurr_Bps = mBps/2;
        else if( mCurr_Bps == mBps/2 )
            mCurr_Bps = mBps/3;
        else 
            mCurr_Bps = mBps;
    }
    else
        mCurr_Bps = Kbps;

    mEncoderCtrl->set_bitrate(mCurr_Bps);
    
    ALOGI("%d bps", mCurr_Bps);
}

void StageFrightIf_Ex::request_intraframe()
{
    ALOGV("request_intraframe");

    mEncoderCtrl->request_intraFrame();

}
