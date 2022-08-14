/**
 * Copyright (C) 2011, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef DECODER_CTRL_H_

#define DECODER_CTRL_H_

#include <binder/ProcessState.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/CameraSource.h>
#include <media/stagefright/OMXClient.h>
#include <gui/Surface.h>
#include <gui/IGraphicBufferProducer.h>

#include <utils/List.h>
#include <utils/threads.h>
#include <common.h>
#include "Decoder_src.h"
#include <libpmap/pmap.h>
#include <hardware/gralloc.h>
#include <hardware/hardware.h>


using namespace android;

class DecoderCtrl {
public:
	DecoderCtrl();
    ~DecoderCtrl();

	status_t setSurfaceTexture(const sp<IGraphicBufferProducer> &bufferProducer, bool bASync_mode);
	status_t configure_decoder(sp<IOMX> & IOMX, sp<MediaSource> trackSource);
	status_t stop_decoder();
	status_t start_RenderThread();
	status_t stop_RenderThread(DecoderSource *src);
	status_t get_YUV420p_Frame(uint8_t *frame, int32_t len, uint8_t *width, uint8_t *height);
	
    sp<MediaSource> pDecoderMedia;
	sp<ANativeWindow> pNativeWindow;
	bool bRENDERworking;

private:
	class RenderThread : public Thread {
        DecoderCtrl* mDecoder;
    public:
        RenderThread(DecoderCtrl* dec)
            : Thread(false), mDecoder(dec) { }

        virtual bool threadLoop() {
            mDecoder->renderThread();
            // loop until we need to quit
            return false;//true;
        }
    };
	void render(MediaBuffer *buffer);
	int renderThread();
	sp<RenderThread>  	pRenderThread;
	bool mThreadloop;
	
protected:
	sp<IOMX> pIOMX;
	mutable Mutex mLock;
	
	bool mStarted;
	bool mSignalledEOS;

	int32_t  mFrameWidth;
	int32_t  mFrameHeight;

	bool mEnabled_FrmCapture;
	bool mCaptured_Frm;
	char *mBuffer_FrmCapture;
	bool mFailed_Capture_Frm;

    int mDev_fd;
    void* mGMap_addr;
	void* mPMap_addr;
    pmap_t mUmpPmap;
	gralloc_module_t const *mGrallocModule;

	bool mem_prepare();
	void mem_destroy();
	unsigned int mem_get_privateAddr(buffer_handle_t* gHandle, bool bEnd);
	unsigned int mem_get_virtAddr(unsigned int phyAddr, unsigned int len, bool bEnd);
	
};

#endif

