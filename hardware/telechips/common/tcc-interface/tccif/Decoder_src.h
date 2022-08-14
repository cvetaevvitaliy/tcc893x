/**
 * Copyright (C) 2011, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef DECODER_SOURCE_H_

#define DECODER_SOURCE_H_

#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>
#include <utils/threads.h>
#include <utils/List.h>

using namespace android;

class DecoderSource : public MediaSource,
                  public MediaBufferObserver {
	
public:
	DecoderSource(int codec_info, int width, int height);
    virtual ~DecoderSource();

    virtual status_t start(MetaData *params = NULL);
	status_t pre_stop();
    virtual status_t stop();
	virtual sp<MetaData> getFormat();
    virtual status_t read(
            MediaBuffer **buffer, const ReadOptions *options = NULL);
	virtual void signalBufferReturned(MediaBuffer* buffer){};
	void SubmitPacket(const uint8_t *data, uint32_t len, int64_t timestamp_ms);
	
protected:
	sp<MetaData> mOutputFormat;

    Mutex mLock;
    List<MediaBuffer *> mFilledBuffers;
    Condition mBufferFilled;
	bool mStarted;
	bool mPreStopped;
	int64_t start_timestamp_ms;
	bool mSignalledEOS;
};

#endif
