/**
 * Copyright (C) 2013, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef __TCC_MODIFIED_STAGEFRIGHT_SAMPLE_H__
#define __TCC_MODIFIED_STAGEFRIGHT_SAMPLE_H__

#include <Camera_ctrl.h>
#include <Encoder_ctrl.h>
#include <Decoder_ctrl.h>


class StageFrightIf_Ex {
public:
	StageFrightIf_Ex();
    ~StageFrightIf_Ex();

	void interface_init();
	void interface_deinit();
	void set_CameraSurface(sp<IGraphicBufferProducer> &surf, int w, int h, int bitrate, int framerate);
	void set_VideoSurface(sp<IGraphicBufferProducer> &surf);
	void start(const String16& clientName);
	void stop();

	void change_framerate(int fps);
	void change_bitrate(int Kbps);
	void request_intraframe();

private:
	sp<CameraCtrl> pCameraCtrl;
	EncoderCtrl *mEncoderCtrl;
	DecoderCtrl *mDecoderCtrl;
	sp<IGraphicBufferProducer> pCameraSurface;
	sp<IGraphicBufferProducer> pVideoSurface;
	OMXClient mClient;
	sp<IOMX> mIOMX;
	int mWidth;
	int mHeight;
	int mBps;
	int mFps;
	int mCurr_Bps;
	int mCurr_Fps;
	bool mStarted;
};
#endif

