/**
 * Copyright (C) 2011, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef RECORD_MAIN_H_

#define RECORD_MAIN_H_

#include <binder/ProcessState.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/CameraSource.h>
#include <media/stagefright/OMXClient.h>
#include <utils/List.h>
#include <utils/threads.h>
#include <common.h>
#include "Camera_ctrl.h"
#include "Encoder_ctrl.h"

using namespace android;

// Callback class used to pass encoded frames to the caller(RTP Sender or Packetizer).
class EncodedFrameCallBack
{
public:
    virtual void EncodedFrameToCaller(uint8_t *data, uint32_t len, int64_t timestamp_ms) = 0;
    virtual ~EncodedFrameCallBack() {}	
};

class RecordMain
{
public:

	/**
     * pCallback given by external will be called whenever Encoder have output frame to send after record start.
     *
     * @return NULL on error.
     */
	RecordMain(EncodedFrameCallBack *callback);
    ~RecordMain();

	/**
     * The method to set Surface which Camera will use.
     */
	status_t set_Surface(const sp<IGraphicBufferProducer>& surf);

	/**
     * The method to configure parameters which Camera and Encoder will use.
     *
     * This function create instance related with Camera and Encoder.
     *
     * @return NULL on error.
     */
	status_t configure_record(int codec_info, int frame_width, int frame_height, int bitrate_Kbps, int framerate, int keyFrame_Interval_seconds, int no_preview, const String16& clientName);

	/**
     * The method to start a Camera and Encoder.
     * Once this is called, fCb_EncodeDone will be called whenever there is encoded frame.
     *
     * @return android::OK if no error.
     */
	status_t start_record();

	/**
     * The method to stop a Camera and Encoder.
     *
     * @return android::OK if no error.
     */
    status_t stop_record();

	/**
     * The method to get YUV data form Camera.
     *
     * @return android::OK if no error.
     */
	status_t get_YUV420p_Frame(uint8_t *frame, uint32_t len, uint8_t *width, uint8_t *height);

	/**
     * The method to replace Camera frame into other image(frame) provided by external because of security reason.
     *
     * @return android::OK if no error.
     */
	status_t replace_YUV420p_Frame(uint8_t *frame, uint32_t len, bool enable);

	/**
     * The method to request I-Frame to Encoder.
     *
     * @return android::OK if no error.
     */
	status_t request_intraFrame();

	/**
     * The method to change bitrate of the Camera and Encoder.
     *
     * @return android::OK if no error.
     */
	status_t set_bitrate(int Kbps);

	/**
     * The method to change framerate of the Camera and Encoder.
     *
     * @return framerate value applied. (because the Camera can't support all kinds of framerate.)
     */
	int set_framerate(int fps);

private:
	class SenderThread : public Thread {
        RecordMain* mRecoder;
    public:
        SenderThread(RecordMain* rec)
            : Thread(false), mRecoder(rec) { }

        virtual bool threadLoop() {
            mRecoder->senderThread();
            // loop until we need to quit
            return false;//true;
        }
    };

	int senderThread();
	bool bRTPworking;
	sp<SenderThread>  	pSenderThread;
	bool mThreadloop;

protected:
	sp<IOMX> pIOMX;
	OMXClient mClient;
	sp<IGraphicBufferProducer> pCameraSurface;
	sp<CameraCtrl> pCamera_Controller;
	EncoderCtrl *mEncoder_Controller;
	int mCurr_Kbps, mCurr_Fps;
	int mFrame_Width, mFrame_Height;
	EncodedFrameCallBack *fCb_EncodeDone;
	
	bool mStarted;
	bool mConfigured;
	bool mSignalledEOS;
};

#endif
