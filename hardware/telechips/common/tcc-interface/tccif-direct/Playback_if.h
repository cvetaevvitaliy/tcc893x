/**
 * Copyright (C) 2013, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef __TCC_PLAYBACK_INTERFACE_H__
#define __TCC_PLAYBACK_INTERFACE_H__

#ifndef int64_t
typedef signed long long int64_t;
#endif

#include "Decoder_if.h"
#include "Renderer_if.h"

#include <fcntl.h>
#include <utils/threads.h>
#include <stdlib.h>

using namespace android;
using android::status_t;

//#define USE_RENDER_THREAD

typedef struct decoded_frame_info {
    tDEC_FRAME_OUTPUT DecOutput;
    tDEC_RESULT DecResult;
} tDEC_FRAME_INFO;

class PlaybackIf {
public:
	PlaybackIf();
    ~PlaybackIf();

	status_t set_Surface(sp<IGraphicBufferProducer> &surf, bool bASync_mode);
	status_t receivedPacket(const uint8_t *data, uint32_t len, int64_t timestamp_ms);
	status_t configure_playback(int codec_info, int frame_width, int frame_height);
	status_t start_playback();
	status_t stop_playback();
	status_t stop_renderer();

private:
	int _proc(void *data, uint32_t len, uint32_t timestamp_ms, tDEC_FRAME_OUTPUT *DecOutput, tDEC_RESULT *DecResult);
	int _manage_render(tDEC_FRAME_OUTPUT *DecOutput, tDEC_RESULT *DecResult);
#ifdef USE_RENDER_THREAD
	status_t _sendPacket(const uint8_t *data, int len);
#endif

// Decoder Thread ////////////////////////////////
	class DecThread : public Thread {
        PlaybackIf* mPlayback;
    public:
        DecThread(PlaybackIf* dIf)
            : Thread(false), mPlayback(dIf) { }

        virtual bool threadLoop() {
            mPlayback->_decoder_thread();
            // loop until we need to quit
            return false;//true;
        }
    };

	int _decoder_thread();
	sp<DecThread>  	mDecThread;
	bool bDecThWorking;

	Mutex mDecLock;
    Condition mDecFrameAvailableCondition;
    List<MediaBuffer *> mDecFramesReceived;
//////////////////////////////////////////////////

#ifdef USE_RENDER_THREAD
// Renderer Thread ///////////////////////////////
	class RenderThread : public Thread {
        PlaybackIf* mPlayback;
    public:
        RenderThread(PlaybackIf* dIf)
            : Thread(false), mPlayback(dIf) { }

        virtual bool threadLoop() {
            mPlayback->_renderer_thread();
            // loop until we need to quit
            return false;//true;
        }
    };

	int _renderer_thread();
	sp<RenderThread>  	mRenderThread;
	bool bRenderThWorking;

	Mutex mRenderLock;
    Condition mRenderFrameAvailableCondition;
    List<MediaBuffer *> mRenderFramesReceived;
	uint32_t mkeptframeCount;
/////////////////////////////////////////////////
#endif

	DecoderIf *pDecoder;
	RendererIf *pRenderer;

	bool mStarted;

	sp<IGraphicBufferProducer> pVideoSurface;
	bool mASyncMode;
	int mCodecType;
	int mWidth;
	int mHeight;

	uint32_t mRenderedCnt;
};

#endif

