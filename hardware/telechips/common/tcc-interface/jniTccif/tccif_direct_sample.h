/**
 * Copyright (C) 2013, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef __TCC_DIRECT_SAMPLE_H__
#define __TCC_DIRECT_SAMPLE_H__

#include "Camera_if.h"
#include "Encoder_if.h"
#include "Decoder_if.h"
#include "Renderer_if.h"

#include <gui/Surface.h>
#include <gui/IGraphicBufferProducer.h>
#include <binder/IMemory.h>

class DirectIf_Ex : public CameraFrameCallBack {
public:
	DirectIf_Ex();
    ~DirectIf_Ex();

	void CameraFrameToCaller(sp<IMemory> dataPtr, int64_t timestamp_us);
	void set_CameraSurface(sp<IGraphicBufferProducer> &surf, int w, int h, int bitrate, int framerate);
	void set_VideoSurface(sp<IGraphicBufferProducer> &surf);
	void start(const String16& clientName);
	void stop();

	void change_framerate(int fps);
	void change_bitrate(int Kbps);
	void request_intraframe();

private:
	int Proc(sp<IMemory> dataPtr, int64_t timestamp_ms);

	class LoopBack_TestThread : public Thread {
        DirectIf_Ex* mDirectIf;
    public:
        LoopBack_TestThread(DirectIf_Ex* dIf)
            : Thread(false), mDirectIf(dIf) { }

        virtual bool threadLoop() {
            mDirectIf->mainThread();
            // loop until we need to quit
            return false;//true;
        }
    };
	int mainThread();
	sp<LoopBack_TestThread>  	mMainThread;
	bool mStarted;
	bool bThreadWorking;

	Mutex mLock;
    Condition mFrameAvailableCondition;
    List<sp<IMemory> > mFramesReceived;
    List<sp<IMemory> > mFramesBeingEncoded;
    List<int64_t> mFrameTimes;
	int mFramesReceivedCount;
	int mFramesReleasedCount;
	int64_t mStartTimeUs;
	
	CameraIf *pCamera;
	EncoderIf*pEncoder;
	DecoderIf*pDecoder;
	RendererIf *pRenderer;

	sp<IGraphicBufferProducer> pCameraSurface;
	sp<IGraphicBufferProducer> pVideoSurface;
	int mWidth;
	int mHeight;
	int mBps;
	int mFps;
	int mCurr_Bps;
	int mCurr_Fps;
	int mRequested_IFrame;

};

#endif

