#ifndef _STREAM_QUEUE_H_
#define _STREAM_QUEUE_H_

#define AUDIO_MIN_FRAME_NUM   20
#define VIDEO_MIN_FRAME_NUM   20
#define SUBTITLE_MIN_FRAME_NUM 1
#define AUDIO_MAX_FRAME_NUM   30
#define VIDEO_MAX_FRAME_NUM   30
#define SUBTITLE_MAX_FRAME_NUM 2

#if 0
#define AUDIO_MIN_DURATION    2000
#define VIDEO_MIN_DURATION    2000
#define AUDIO_MAX_DURATION    3000
#define VIDEO_MAX_DURATION    3000
#else
#define AUDIO_MIN_DURATION    1000
#define VIDEO_MIN_DURATION    1000
#define SUBTITLE_MIN_DURATION 1000
#define AUDIO_MAX_DURATION    5000
#define VIDEO_MAX_DURATION    5000
#define SUBTITLE_MAX_DURATION 1000
#endif

#define STREAM_FRAME_NUM      10240//2048//1024

typedef struct secure_info_t {
	uint32_t ulSecureType;
	union {
		struct {
			unsigned long		ulStreamCounter;
			unsigned long long	ullInputCounter;
		} stHdcpInfo;
	} enSecureInfo;
} secure_info_t;

typedef struct write_info_t {
	uint8_t       *pbyDataBuffer;
	uint32_t       ulDataSize;
	uint32_t       ulTimestamp;
	int32_t        mPacketDiscont;
};

typedef struct _STREAM_S_ {
    uint32_t  offset;
    uint32_t  size;
    uint32_t  timestamp;
    int32_t   result;
    int32_t   mPacketDiscont;
	secure_info_t  secure_info;
} _STREAM_S_;

class StreamQueue {
	public:
		enum ManagedStreamType { VIDEO = 1, AUDIO = 2, SUBTITLE = 4 };

		enum QueuedState {
			STREAM_Q_EMPTY = 1,
			STREAM_Q_FULL, 
			STREAM_Q_NORMAL
		};

		enum QueueState { START, STOPPING, STOPPED };

		enum BufferingThresholdType {
			THRESHOLD_TYPE_NONE,
			THRESHOLD_TYPE_TIME,
			THRESHOLD_TYPE_COUNT,
			THRESHOLD_TYPE_CLIP_TIME,
			THRESHOLD_TYPE_CLIP_COUNT
		};
		
		StreamQueue(uint32_t nPacketType, uint32_t nQueueBufferSize, uint32_t nBufferElementSize);
		~StreamQueue();

		void      SetBufferingThreshold(ManagedStreamType eManagedStream, BufferingThresholdType nThresholdType, 
				                        uint32_t nMinValue = 0, uint32_t nMaxValue = 0);
		uint32_t  GetCurrBufferingAmount(BufferingThresholdType nType = THRESHOLD_TYPE_NONE);
		void      InitBufferSimple();
		void      InitBuffer();
		uint32_t  SkipAudioStream();
		uint32_t  GetQueuedState();

		int32_t   GetOneFifo(uint32_t ReqSize);    // Get One empty FIFO
		uint8_t*  GetWriteAddress();

		uint32_t  GetBufferedTime();
		int32_t   GetLength();
        int32_t   GetAvailBytes(uint32_t &w_off, uint32_t &r_off);
		uint8_t*  GetOneStream(uint32_t *Len, uint32_t *TimeStamp, secure_info_t *pstSecureInfo, int32_t* PacketDiscont, int32_t* Result);
		int32_t   WriteFifo(write_info_t *pstWriteInfo, secure_info_t *pstSecureInfo = NULL, int32_t result = 0);
		int32_t   Push();
		int32_t   Pop();

		inline uint32_t  GetPacketType() const { return m_nPacketType; };
		inline void SetPacketType(uint32_t nType) { m_nPacketType = nType; };
	
		inline uint32_t  GetState() const { return m_nState; };
		inline void SetState(uint32_t nState) { m_nState = nState; };

		inline uint32_t  GetStreamErrorCode() const { return m_nStreamError; };
		inline void SetStreamErrorCode(int32_t nCode) { m_nStreamError = nCode; };

		inline uint32_t  GetMaxBufferingThreshold() const { return m_MaxBufferingThreshold; };
		inline void SetMaxBufferingThreshold(int32_t nThreshold) { m_MaxBufferingThreshold = nThreshold; };

		inline uint32_t  GetMinBufferingThreshold() const { return m_MinBufferingThreshold; };
		inline void SetMinBufferingThreshold(int32_t nThreshold) { m_MinBufferingThreshold = nThreshold; };

		inline uint32_t  GetElementSize() const { return m_nMaxElementSize; };
		inline void SetElementSize(int32_t nCapacity) { m_nMaxElementSize = nCapacity; };

		inline uint32_t  GetFifoFailCount() const { return m_nGetFifoFailCount; };
		inline void SetFifoFailCountZero() { m_nGetFifoFailCount = 0; };
		inline void IncreaseFifoFailCount() { m_nGetFifoFailCount++; };
        inline uint32_t getRemainSize() { return m_RemainSize;}

	private:
		_STREAM_S_ *stStreamInfo;
		uint8_t   *m_pBuffer;
		uint32_t  m_nMaxBufferSize;
		uint32_t  m_nMaxElementSize;
		uint32_t  m_nReadBufPtr;
		uint32_t  m_nWriteBufPtr;
		uint32_t  m_nReadFrmIdx;
		uint32_t  m_nWriteFrmIdx;
		uint32_t  m_nIdxNum;
		uint32_t  m_nfState;
		uint32_t  m_nPacketType;

		uint32_t  m_nState;
		int32_t  m_nStreamError;
        
		uint32_t  m_nGetFifoFailCount;
		BufferingThresholdType m_QueueThresholdMgmtType;

		uint32_t  m_MaxBufferingThreshold;
		uint32_t  m_MinBufferingThreshold; 
        uint32_t  m_RemainSize;
        uint32_t  m_lastMaxWriteOffset;
        uint32_t  m_WFDStream;
};

#endif // _STREAM_QUEUE_H_

