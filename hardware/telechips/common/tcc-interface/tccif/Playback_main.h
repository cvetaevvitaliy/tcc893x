/**
 * Copyright (C) 2011, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef PLAYBACK_MAIN_H_

#define PLAYBACK_MAIN_H_

#include <binder/ProcessState.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/CameraSource.h>
#include <media/stagefright/OMXClient.h>
#include <utils/List.h>
#include <common.h>
#include "Decoder_ctrl.h"
#include "Decoder_src.h"

using namespace android;

class PlaybackMain {
public:
	PlaybackMain();
    ~PlaybackMain();

	/**
     * The method to receive packet-frame from RTP.
     *
     * Caution : This frame has to be depacketized data.
     *
     * @return negative value on error.
     */
	status_t receivedPacket(const uint8_t *data, uint32_t len, int64_t timestamp_ms);

	/**
     * The method to set Surface which Renderer will use.
	 *
     * bAsync_mode		: Camera preview and videos are rate-limited on the producer side.
     *		    		  If enabled, we use async mode to always show the most recent frame at the cost of requiring an additional buffer.
	 *
     */
	status_t set_Surface(sp<IGraphicBufferProducer> &surf, bool bASync_mode = true);

	/**
     * The method to configure parameters which Decoder will use.
     *
     * This function create instance related with Decoder and Renderer.
     *
     * @return NULL on error.
     */
	status_t configure_playback(int codec_info, int frame_width, int frame_height);

	/**
     * The method to start Decoder and Renderer.
     *
     * @return android::OK if no error.
     */
	status_t start_playback();

	/**
     * The method to stop Decoder and Renderer.
     *
     * @return android::OK if no error.
     */
    status_t stop_playback();

	/**
     * The method to get YUV data form Decoder.
     *
     * @return android::OK if no error.
     */
	status_t get_YUV420p_Frame(uint8_t *frame, int32_t len, uint8_t *width, uint8_t *height);

protected:
	sp<IOMX> pIOMX;
	OMXClient mClient;
	sp<IGraphicBufferProducer> pVideoSurface;
	DecoderCtrl *mDecoder_Controller;
	DecoderSource *mDecoderSource;
	sp<MediaSource> pDecoderSource;

	bool mStarted;
	bool mASyncMode;
};

#endif
