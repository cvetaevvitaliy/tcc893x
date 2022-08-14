/**
 * Copyright (C) 2013, Telechips Limited
 * by ZzaU(B070371)
 */
 
//#define LOG_NDEBUG 0
#define LOG_TAG "TccIf-Direct-Sample"

#include <tccif_direct_sample.h>
#include "utils/Log.h"
#include <utils/List.h>
#include <utils/threads.h>
#include <cutils/properties.h>
#include <fcntl.h>

DirectIf_Ex::DirectIf_Ex()
{
    ALOGV("Create DirectIf_Ex");
}

DirectIf_Ex::~DirectIf_Ex()
{
    ALOGV("Destory DirectIf_Ex");
}

int DirectIf_Ex::Proc(sp<IMemory> dataPtr, int64_t timestamp_ms)
{
    tENC_FRAME_INPUT EncInput;
    tDEC_FRAME_INPUT DecInput;
    
    tENC_FRAME_OUTPUT EncOutput;
    tDEC_FRAME_OUTPUT DecOutput;
    tDEC_RESULT DecResult;

    enc_metadata_t* pEncMetadataInfo = (enc_metadata_t*)dataPtr->pointer();

    if(1)
    {
        EncInput.frameFormat = FRAME_BUF_FORMAT_YVU420P;
        EncInput.inputStreamAddr = (unsigned char*)pEncMetadataInfo->fd_0;
        EncInput.noIncludePhyAddr = 0;
        EncInput.inputStreamSize = mWidth*mHeight*3/2;
        EncInput.nTimeStamp = (int)timestamp_ms;
        EncInput.isForceIFrame = mRequested_IFrame;
        EncInput.isSkipFrame = 0;
        EncInput.frameRate = mCurr_Fps;
        EncInput.targetKbps = mCurr_Bps;
        EncInput.Qp = 0;
        
        if( pEncoder->Encode(&EncInput, &EncOutput) < 0)
            return -1;
        mRequested_IFrame = 0;
    }

    if(1)
    {
        DecInput.inputStreamAddr = EncOutput.outputStreamVirtAddr;
        DecInput.inputStreamSize = EncOutput.frameLen > 0 ? EncOutput.frameLen : EncOutput.headerLen;
        DecInput.seek = 0;
        DecInput.nTimeStamp = EncOutput.nTimeStamp;

        if( pDecoder->Decode(&DecInput, &DecOutput, &DecResult) < 0 )
            return -1;
    }

    if(1)
    {
        if(!DecResult.no_frame_output)
            pRenderer->Render(DecOutput.bufPhyAddr[0], DecOutput.bufPhyAddr[1], DecOutput.bufPhyAddr[2], 1, DecOutput.nTimeStamp, &DecResult.plat_priv);
    }
    else
    {
        // In order to test displaying camera's output directly.
        if(0)
        {
            unsigned int Y, U, V;
            unsigned int stride;
    		unsigned int framesize_Y, framesize_C, framesize_total;

    	 	stride = ALIGNED_BUFF(mWidth, 16);
    		framesize_Y = stride * mHeight;
    		framesize_C = ALIGNED_BUFF(stride/2, 16) * mHeight/2;
    		framesize_total = framesize_Y + (framesize_C * 2);
            
            Y = (unsigned int)pEncMetadataInfo->fd_0;
        	V = Y + framesize_Y;
        	U = V + framesize_C;
            pRenderer->Render(Y, U, V, 1, timestamp_ms, NULL);
        }
    }

    return 0;
}

int DirectIf_Ex::mainThread()
{
	ALOGI("--Entering loopback_TestThread loop");

    sp<IMemory> frame;
    int64_t frameTime;
    int64_t frameTimeStamp_ms;

    bThreadWorking = true;
    
	while ( mStarted ) 
    {
        {
            Mutex::Autolock autoLock(mLock);

            while(mFramesReceived.empty() && mStarted )
            {
                if( NO_ERROR != mFrameAvailableCondition.waitRelative(mLock, 1000000000LL)) 
                {
                    ALOGW("Timed out :: There is no frame from camera for 1 seconds. Frame count: %d/%d", mFramesReleasedCount, mFramesReceivedCount);
                }
            }

            frame = NULL;
            if( mStarted )
            {
                frame = *mFramesReceived.begin();
                if( frame != NULL )
                {
                    mFramesReceived.erase(mFramesReceived.begin());

                    frameTime = *mFrameTimes.begin();
                    mFrameTimes.erase(mFrameTimes.begin());
                }
            }
        }

        if( mStarted && frame != NULL )
        {
            if( mStartTimeUs == -1)
                mStartTimeUs = frameTime;

            if( frameTime > mStartTimeUs )
            {
                frameTimeStamp_ms = (frameTime - mStartTimeUs)/1000;                
                Proc(frame, frameTimeStamp_ms);
            }

            pCamera->ReleaseFrame(frame);
    	    mFramesReleasedCount++;
        }

	}

    ALOGI("--Exiting loopback_TestThread");    

    bThreadWorking = false;    
    
	return false;
}

void DirectIf_Ex::CameraFrameToCaller(sp<IMemory> dataPtr, int64_t timestamp_us)
{
    Mutex::Autolock autoLock(mLock);

    if( !mStarted )
        return;

	mFramesReceivedCount++;

    mFramesReceived.push_back(dataPtr);
    mFrameTimes.push_back(timestamp_us);
    mFrameAvailableCondition.signal();
}

void DirectIf_Ex::set_CameraSurface(sp<IGraphicBufferProducer> &surf, int w, int h, int bitrate, int framerate)
{
    ALOGV("set_CameraSurface");

    if( surf == NULL )
    {
        ALOGE("bufferProducer for camera is NULL.");
        return;
    }

    mWidth  = w;
    mHeight = h;
    mBps    = mCurr_Bps = bitrate;
    mFps    = mCurr_Fps = framerate;
    pCameraSurface = surf;
}

void DirectIf_Ex::set_VideoSurface(sp<IGraphicBufferProducer> &surf)
{
    ALOGV("set_VideoSurface");

    if( surf == NULL )
    {
        ALOGE("bufferProducer for video is NULL.");
        return;
    }

    pVideoSurface = surf;
}

void DirectIf_Ex::start(const String16& clientName)
{
    ALOGV("Start");

    tENC_INIT_PARAMS EncInit;
    tDEC_INIT_PARAMS DecInit;

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
    
    mFramesReceivedCount = mFramesReleasedCount = 0;
    mStartTimeUs = -1;

    mRequested_IFrame = 0;
    memset(&EncInit, 0x00, sizeof(tENC_INIT_PARAMS));
    memset(&DecInit, 0x00, sizeof(tDEC_INIT_PARAMS));
    
    if( pCameraSurface == NULL || pVideoSurface == NULL)
    {
        ALOGE("Not ready to start operation. please check Surface Pointer");
        return;
    }

    pCamera = new CameraIf((CameraFrameCallBack*)this);
    pEncoder = new EncoderIf();
    pDecoder = new DecoderIf();
    pRenderer = new RendererIf (pVideoSurface, NULL, mWidth, mHeight, FRAME_BUF_FORMAT_YUV420I);

    if(false == pRenderer->IsInit_OK())
        return;

    {
        EncInit.codecFormat = CODEC_FORMAT_H264;
        EncInit.picWidth = mWidth;
        EncInit.picHeight = mHeight;
        EncInit.frameRate = mFps;
        EncInit.targetKbps = mBps;
        EncInit.keyFrameInterval = mFps * 2;
        EncInit.use_NalStartCode= 1;

        pEncoder->Init(&EncInit);
    }

    {
        DecInit.codecFormat = CODEC_FORMAT_H264;
        DecInit.frameFormat = FRAME_BUF_FORMAT_YUV420I;
        DecInit.picWidth = mWidth;
        DecInit.picHeight = mHeight;

        pDecoder->Init(&DecInit);
    }

    pCamera->Init(pCameraSurface, mWidth, mHeight, mFps, 0, clientName);
    pCamera->Start();

    mStarted = true;
    mMainThread = new LoopBack_TestThread(this);
	mMainThread->run("DirectIf_Tester");

}

void DirectIf_Ex::stop()
{
    ALOGV("Stop");

    if( !mStarted )
        return;

//    Mutex::Autolock autoLock(mLock);

    // don't hold the lock while waiting for the thread to quit
    if( mMainThread != NULL)
    {        
        sp<LoopBack_TestThread> tempTh = mMainThread;
        tempTh->requestExit();
        mStarted = false;
        mFrameAvailableCondition.signal();
        while( bThreadWorking )
        {
            usleep(10*1000); //10ms
        }

        ALOGD("--Stopped Thread.");
        
        mMainThread.clear();
        mMainThread = NULL;
    }

    pCamera->Stop();
    pEncoder->Close();
    pDecoder->Close();

    delete pCamera;
    delete pEncoder;
    delete pDecoder;
    delete pRenderer;

    {
        ALOGV("Frame count: %d/%d", mFramesReleasedCount, mFramesReceivedCount);
        while (!mFramesReceived.empty()) {
            mFramesReceived.erase(mFramesReceived.begin());
            mFrameTimes.erase(mFrameTimes.begin());
        }
    }

    property_set("tcc.all.video.call.enable", "0");

    int clockctrlFD = open("/dev/clockctrl", O_WRONLY);

    if(clockctrlFD > 0)
    {
        ioctl(clockctrlFD, MAX_CLOCK_V2IP_DISABLE, 0);
        ALOGI("Disable clock speed to maximum for V2IP");
        close(clockctrlFD);
    }

}


void DirectIf_Ex::change_framerate(int fps)
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

    final_fps = pCamera->SetFramerate(mCurr_Fps);
    mCurr_Fps = final_fps;
    
    ALOGI("%d fps", mCurr_Fps);
}

void DirectIf_Ex::change_bitrate(int Kbps)
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
    
    ALOGI("%d Kbps", mCurr_Bps);
}

void DirectIf_Ex::request_intraframe()
{
    ALOGV("request_intraframe");

    mRequested_IFrame = 1;
}











