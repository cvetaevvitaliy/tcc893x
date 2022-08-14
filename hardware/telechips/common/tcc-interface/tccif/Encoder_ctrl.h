/**
 * Copyright (C) 2011, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef ENCODER_CTRL_H_

#define ENCODER_CTRL_H_

#include <binder/ProcessState.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/CameraSource.h>
#include <media/stagefright/OMXClient.h>
#include <utils/List.h>
#include <utils/threads.h>
#include <common.h>

using namespace android;

class EncoderCtrl {
public:
	EncoderCtrl();
    ~EncoderCtrl();

	status_t configure_encoder(sp<IOMX> & IOMX, sp<MediaSource> cameraSource, int codec_info, int frame_width, int frame_height, int bitrate_Kbps, int framerate, int keyFrame_Interval_seconds);
    status_t stop_encoder();
	status_t request_intraFrame();
	status_t set_bitrate(int Kbps);
	status_t set_framerate(int fps);
		
    sp<MediaSource> pEncoderMedia;
	bool bRTPworking;
	
protected:
	sp<IOMX> pIOMX;
	IOMX::node_id mNode;

	bool mStarted;
};

#endif
