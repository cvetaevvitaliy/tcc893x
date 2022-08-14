#ifndef TCC_MPEG2_TS_EXTRACTOR_H_

#define TCC_MPEG2_TS_EXTRACTOR_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/MediaExtractor.h>
#include <utils/threads.h>
#include <utils/Vector.h>

namespace android {

struct AMessage;
struct TCCAnotherPacketSource;
struct TCCATSParser;
struct DataSource;
struct TCCMPEG2TSSource;
struct ALooper;
struct String8;
struct SfLiveSource;

struct TCCMPEG2TSExtractor : public MediaExtractor {
    TCCMPEG2TSExtractor(const sp<DataSource> &source);

    enum {
        FEEDMODE_FEED_ONCE = 1,
        FEEDMODE_FEED_BULK = 2,
        FEEDMODE_PEEK_ONLY = 4,
        FEEDMODE_POST_INIT = 8,
    };

    virtual size_t countTracks();
    virtual sp<MediaSource> getTrack(size_t index);
    virtual sp<MetaData> getTrackMetaData(size_t index, uint32_t flags);

    virtual sp<MetaData> getMetaData();

    virtual uint32_t flags() const;
    virtual uint32_t getMyType() const;

    virtual void signalUnderrun(bool underrun, int64_t thresholdUs = 0);
	virtual bool checkQueueDurationUs(
                     int64_t *lastPacketTimeUs, 
                     int64_t *durationUs,
                     int64_t videoThresholdUs = 1000000ll, 
                     int64_t audioThresholdUs = 1000000ll);
	bool checkQueuePacketCount(int32_t videoThresholdNum = 100, 
		                                    int32_t audioThresholdNum = 100);

    const sp<DataSource>& getDataSource();
    const sp<SfLiveSource>& getLiveSource();

    void init(); 
    status_t init_l(int32_t mode = 0); 
    status_t initCheck() const;

    void startFeeder(); 
    void stopFeeder(); 
    void pauseFeeder(); 
    void resumeFeeder(); 
    bool isFeederStopped(); 

    void partial_reset(); 

	bool isEOSReached() { return mReachedEOS; }

    void setLiveSource(const sp<SfLiveSource> &liveSource);
    status_t seekTo(int64_t& seekTimeUs);

	void onMessageReceived(const sp<AMessage> &msg);

protected:
    virtual ~TCCMPEG2TSExtractor();

private:
    enum {
        kWhatInit      = 'iniT',
        kWhatInitDone  = 'inDo',
	};

    friend struct TCCMPEG2TSSource;
    struct ATSPacketFeeder;

    mutable Mutex mLock;
    sp<ALooper> mLooper;
    sp<AHandlerReflector<TCCMPEG2TSExtractor> > mReflector;

    sp<DataSource> mDataSource;
    sp<SfLiveSource> mLiveSource;
	sp<ATSPacketFeeder> mTSPacketFeeder;

    sp<TCCATSParser> mParser;

    Vector<sp<TCCAnotherPacketSource> > mSourceImpls;

	uint32_t mVideoPacketIndex;
	uint32_t mAudioPacketIndex;

	bool mReachedEOS;
	status_t mInitCheck;

    off64_t mOffset;

    bool mHaveAudio;
    bool mHaveVideo;

    size_t mMaxNumOfTSPacketToInit;
    bool mTraceESQTime;

    status_t feedMore(int32_t mode = 0, int64_t durationUs = -1);

    DISALLOW_EVIL_CONSTRUCTORS(TCCMPEG2TSExtractor);
};

bool SniffM3U8(
        const sp<DataSource> &source, String8 *mimeType, float *confidence,
        sp<AMessage> *);

}  // namespace android

#endif  // TCC_MPEG2_TS_EXTRACTOR_H_
