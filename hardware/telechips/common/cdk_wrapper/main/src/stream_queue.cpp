/* ------------------------------------------------------------------
 * Copyright (C) 2009-2010 Telechips 
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
#ifndef PROFILE_MEDIA_STREAMING
#define PROFILE_MEDIA_STREAMING
#endif

#include "tcc_cdk_wrapper.h"
#include "utils/Log.h"
#include "stream_queue.h"
#include <cutils/properties.h>

#undef	LOG_TAG
#define LOG_TAG	"TCCStreamQueue"

//#define DEBUG
#ifdef DEBUG
#define LOGINFO(x...)   LOGI(x)
#define LOGERR(x...)	LOGE(x)
#else
#define LOGINFO(x...)
#define LOGERR(x...) //LOGE(x)
#endif

StreamQueue::StreamQueue(uint32_t nPacketType, uint32_t nQueueBufferSize, uint32_t nBufferElementSize)
	: m_nMaxBufferSize(nQueueBufferSize)
	, m_nMaxElementSize(nBufferElementSize)
	, m_nReadBufPtr(0)
	, m_nWriteBufPtr(0)
	, m_nReadFrmIdx(0)
	, m_nWriteFrmIdx(0)
	, m_nIdxNum(0)
	, m_nfState(STREAM_Q_EMPTY)
	, m_nPacketType(nPacketType)
	, m_nState(START)
	, m_nStreamError(0)
    , m_nGetFifoFailCount(0)
	, m_QueueThresholdMgmtType(THRESHOLD_TYPE_NONE)
	, m_MaxBufferingThreshold(0)
	, m_MinBufferingThreshold(0) 
	, m_RemainSize(0)
	, m_lastMaxWriteOffset(0)
	, m_WFDStream(0)
{
	m_pBuffer = new uint8_t[nQueueBufferSize];
	m_nReadBufPtr = 0;
	m_nWriteBufPtr = 0;
	m_nReadFrmIdx = 0;
	m_nWriteFrmIdx = 0;
    m_nGetFifoFailCount = 0;

    char value[PROPERTY_VALUE_MAX];
    property_get("tcc.demuxer.max.frame.num", value, "0");
    m_nIdxNum = atoi(value);
    if (m_nIdxNum == 0) {
        m_nIdxNum = STREAM_FRAME_NUM;
    }

    property_get("tcc.media.wfd.sink.run", value, "0");
    m_WFDStream = atoi(value);

    stStreamInfo = new _STREAM_S_[m_nIdxNum];

	InitBuffer();
}

StreamQueue::~StreamQueue()
{
	if (NULL != m_pBuffer)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}

    if (NULL != stStreamInfo)
	{
		delete[] stStreamInfo;
		stStreamInfo = NULL;
	}
}

void StreamQueue::InitBufferSimple()
{
	m_nReadBufPtr = 0; 
	m_nWriteBufPtr = 0; 
	m_nReadFrmIdx = 0; 
	m_nWriteFrmIdx = 0; 
 	m_nfState = STREAM_Q_EMPTY;
}

void StreamQueue::InitBuffer()
{
	InitBufferSimple();
	memset(m_pBuffer, 0, m_nMaxBufferSize);
	
	for(int i = 0 ; i < m_nIdxNum; i++)
	{
		memset( &stStreamInfo[i], 0, sizeof(_STREAM_S_) );
	}
}

uint32_t StreamQueue::SkipAudioStream()
{
    if(m_nfState != STREAM_Q_EMPTY)
    {
        unsigned int	uiTail, uiHead;
        unsigned int	uiFirstPTS = 0, uiLastPTS = 0, uiCurPTS = 0;

        uiTail = m_nReadFrmIdx;
        uiHead = m_nWriteFrmIdx;

        if(uiHead == 0)
            uiHead = m_nIdxNum-1;
        else
            uiHead--;

        if(uiHead == uiTail)
            return 0;

        while(uiLastPTS == 0)
        {
            uiLastPTS = stStreamInfo[uiHead].timestamp;
            if(uiLastPTS != 0)
                break;
            if(uiHead == 0)
                uiHead = m_nIdxNum;

            uiHead--;

            if(uiHead == uiTail)
                break;
        }

        while(1)
        {
            uiCurPTS = stStreamInfo[uiHead].timestamp;

            if(60 < (uiLastPTS - uiCurPTS)){
                m_nReadFrmIdx = uiHead;
                break;
            }

            if(uiHead == 0)
                uiHead = m_nIdxNum;

            uiHead--;

            if(uiHead == uiTail)
                break;
        }
    } 
    return 0;
}
void StreamQueue::SetBufferingThreshold(ManagedStreamType eManagedStream, BufferingThresholdType nThresholdType, uint32_t nMinValue, uint32_t nMaxValue)
{
	if(eManagedStream == VIDEO)
	{
		if(nThresholdType == THRESHOLD_TYPE_TIME)
		{
			if(nMinValue == 0) nMinValue = VIDEO_MIN_DURATION; 
			if(nMaxValue == 0) nMaxValue = VIDEO_MAX_DURATION; 
		}
		else 
		{
			if(nMinValue == 0) nMinValue = VIDEO_MIN_FRAME_NUM; 
			if(nMaxValue == 0) nMaxValue = VIDEO_MAX_FRAME_NUM; 
		}
	}
	else if(eManagedStream == AUDIO)
	{
		if(nThresholdType == THRESHOLD_TYPE_TIME)
		{
			if(nMinValue == 0) nMinValue = AUDIO_MIN_DURATION; 
			if(nMaxValue == 0) nMaxValue = AUDIO_MAX_DURATION; 
		}
		else
		{
			if(nMinValue == 0) nMinValue = AUDIO_MIN_FRAME_NUM; 
			if(nMaxValue == 0) nMaxValue = AUDIO_MAX_FRAME_NUM; 
		}
	}
	else {
		if(nThresholdType == THRESHOLD_TYPE_TIME)
		{
			if(nMinValue == 0) nMinValue = SUBTITLE_MIN_DURATION; 
			if(nMaxValue == 0) nMaxValue = SUBTITLE_MAX_DURATION; 
		}
		else
		{
			if(nMinValue == 0) nMinValue = SUBTITLE_MIN_FRAME_NUM; 
			if(nMaxValue == 0) nMaxValue = SUBTITLE_MAX_FRAME_NUM; 
		}
	}

	m_QueueThresholdMgmtType = nThresholdType;
	SetMaxBufferingThreshold(nMaxValue);
	SetMinBufferingThreshold(nMinValue);
}

uint32_t StreamQueue::GetCurrBufferingAmount(BufferingThresholdType nType)
{
	if(nType == THRESHOLD_TYPE_NONE) nType = m_QueueThresholdMgmtType;

	return (nType == StreamQueue::THRESHOLD_TYPE_TIME) 
		? GetBufferedTime() : GetLength();
}

uint32_t StreamQueue::GetQueuedState()
{	
	return m_nfState;	
}

int32_t StreamQueue::GetOneFifo(uint32_t ReqSize)
{
	uint32_t	uiHead;
	uint32_t	ReadBufPtr, WriteBufPtr;
	uint32_t 	lRemainSize = 0;
	
	LOGINFO("[%s] in.", __func__);

	if(m_nfState == STREAM_Q_FULL)
	{
		LOGERR("[%s]:[%5d] m_nfState = [%d].", __func__, __LINE__, m_nfState);
#ifdef PROFILE_MEDIA_STREAMING
        m_RemainSize = lRemainSize;
#endif
		return 0;
	}
	
	ReadBufPtr = m_nReadBufPtr;
	WriteBufPtr = m_nWriteBufPtr;
	
	uiHead = m_nWriteFrmIdx;
	
	if( ReadBufPtr > WriteBufPtr ) // Tail < Head
	{
		lRemainSize = ReadBufPtr -  WriteBufPtr;
		if( lRemainSize <= ReqSize )
		{
			LOGERR("[%s]:[%5d] ReqSize[%d] / lRemainSize [%d] 1.", __func__, __LINE__, ReqSize, lRemainSize);
			m_nfState = STREAM_Q_FULL;
#ifdef PROFILE_MEDIA_STREAMING
            m_RemainSize = lRemainSize;
#endif

			return 0;
		}
	}
	else
	{
		 lRemainSize = m_nMaxBufferSize - WriteBufPtr;
		 if( ReqSize >= lRemainSize )
		 {
		    m_lastMaxWriteOffset = WriteBufPtr;
		 	WriteBufPtr = 0;
		 	lRemainSize = ReadBufPtr;
			if( lRemainSize <= ReqSize )
			{
				LOGERR("[%s]:[%5d] ReqSize[%d] / lRemainSize [%d] 2.", __func__, __LINE__, ReqSize, lRemainSize);
				m_nfState = STREAM_Q_FULL;
#ifdef PROFILE_MEDIA_STREAMING
                m_RemainSize = lRemainSize;
#endif
				return 0;
			}
		 }
	}
	
#if 1
	stStreamInfo[uiHead].offset = WriteBufPtr;
	stStreamInfo[uiHead].size = ReqSize;
	stStreamInfo[uiHead].timestamp = 0;
	stStreamInfo[uiHead].result = 0;

    if( m_WFDStream == 1)
        stStreamInfo[uiHead].mPacketDiscont = 0;
#endif
	
	m_nWriteBufPtr = WriteBufPtr;
	
	LOGINFO("[%s] : Line[%d] - uiHead[%d], offset[%d], size[%d]", __func__, __LINE__, uiHead, WriteBufPtr, ReqSize);
#ifdef PROFILE_MEDIA_STREAMING
    m_RemainSize = lRemainSize;
#endif
	return 1;
}

int32_t StreamQueue::GetAvailBytes(uint32_t &WriteBufPtr,  uint32_t &ReadBufPtr)
{
	uint32_t 	lAvailSize = 0;
   	ReadBufPtr = m_nReadBufPtr;
	WriteBufPtr = m_nWriteBufPtr;

	if( ReadBufPtr > WriteBufPtr ) // Tail < Head
	{
		lAvailSize = (m_lastMaxWriteOffset - ReadBufPtr) +  WriteBufPtr;
	}
	else
	{
		lAvailSize = WriteBufPtr - ReadBufPtr;
	}
    
	return lAvailSize;
}

uint32_t StreamQueue::GetBufferedTime()
{
	unsigned int timeRange = 0;
	unsigned int	uiTail, uiHead;
	unsigned int	uiFirstPTS = 0, uiLastPTS = 0;

	LOGINFO("[%s] in.", __func__);
	
	if(m_nfState != STREAM_Q_EMPTY)
	{
		uiTail = m_nReadFrmIdx;
		uiHead = m_nWriteFrmIdx;

		if(uiHead == 0)
			uiHead = m_nIdxNum-1;
		else
			uiHead--;

		if(uiHead == uiTail)
			return 0;

		while(uiFirstPTS == 0)
		{
			uiFirstPTS = stStreamInfo[uiTail].timestamp;
			if(uiFirstPTS != 0)
				break;
			uiTail++;
			if(uiTail == m_nIdxNum)
				uiTail = 0;

			if(uiTail == uiHead)
				break;
		}

		while(uiLastPTS == 0)
		{
			uiLastPTS = stStreamInfo[uiHead].timestamp;
			if(uiLastPTS != 0)
				break;
			if(uiHead == 0)
				uiHead = m_nIdxNum;
			uiHead--;

			if(uiHead == uiTail)
				break;
		}

		if (!(uiHead == uiTail || uiFirstPTS == 0 || uiLastPTS == 0))
		{
			if(uiLastPTS >= uiFirstPTS)
			{
				timeRange = (uiLastPTS-uiFirstPTS)%((unsigned long)(0x100000000LL/45LL));
			}
		}

		LOGINFO("GetBufferedTime() : Tail/Head(%d:%d), uiFirstPTS/uiLastPTS(%d:%d)", uiHead, uiTail, uiFirstPTS, uiLastPTS);
	}
	
	LOGINFO("[%s] [%d] out, return range : [%d].", __func__, __LINE__, timeRange);
	return timeRange;
}

int32_t StreamQueue::GetLength()
{
	int lRemainSize = 0;
	unsigned int	uiTail, uiHead;

	LOGINFO("[%s] in.", __func__);

	uiTail = m_nReadFrmIdx;
	uiHead = m_nWriteFrmIdx;
	
	if(m_nfState != STREAM_Q_EMPTY)
	{
		if( uiTail < uiHead ) // Tail < Head
		{
			 lRemainSize = uiHead - uiTail;
		}
		else
		{
			lRemainSize = (m_nIdxNum - uiTail) +  uiHead;
		}
		LOGINFO("[%s] [%d] out, return size : [%d].", __func__, __LINE__, lRemainSize);
		return lRemainSize;
	}
	
	LOGINFO("[%s] [%d] out, return size : [%d].", __func__, __LINE__, lRemainSize);
	return lRemainSize;
}

uint8_t* StreamQueue::GetOneStream(uint32_t *Len, uint32_t *TimeStamp, secure_info_t *pstSecureInfo, int32_t* PacketDiscont, int32_t* Result)
{
	uint32_t	uiTail;
	uint8_t	*ptrData;

	LOGINFO("[%s] in.", __func__);
	
	if(m_nfState == STREAM_Q_EMPTY)
	{
		LOGERR("[%s]:[%5d] m_nfState = [%d].", __func__, __LINE__, m_nfState);
		*Len = 0;
		*Result = 0;
		return 0;
	}
		
	uiTail = m_nReadFrmIdx;

	ptrData = m_pBuffer + stStreamInfo[uiTail].offset;

	*Len = stStreamInfo[uiTail].size;
	*TimeStamp = stStreamInfo[uiTail].timestamp;
	*Result = stStreamInfo[uiTail].result;

	if( pstSecureInfo )
		*pstSecureInfo = stStreamInfo[uiTail].secure_info;

	if( PacketDiscont && m_WFDStream == 1)
		*PacketDiscont = stStreamInfo[uiTail].mPacketDiscont;
	
	LOGINFO("[%s] [%d] out, return ptr : [%x].", __func__, __LINE__, ptrData);
	
	return ptrData;
}

uint8_t* StreamQueue::GetWriteAddress()
{
	return m_pBuffer + m_nWriteBufPtr;
}

int32_t StreamQueue::WriteFifo(write_info_t *pstWriteInfo, secure_info_t *pstSecureInfo, int32_t result)
{
	uint32_t	WriteBufPtr;
	uint8_t*  ptrWrite = NULL;
	uint32_t	MaxSize;
	uint32_t	uiHead = m_nWriteFrmIdx;
	uint8_t    *ptrData   = NULL;
	uint32_t    ReqSize   = 0;
	uint32_t    TimeStamp = 0;
	int32_t     PacketDiscont = 0;

	if( pstWriteInfo ) {
		ptrData   = pstWriteInfo->pbyDataBuffer;
		ReqSize   = pstWriteInfo->ulDataSize;
		TimeStamp = pstWriteInfo->ulTimestamp;
		if(m_WFDStream) {
			PacketDiscont = pstWriteInfo->mPacketDiscont;
		} else {
			PacketDiscont = 0;
		}
	}

	LOGINFO("[%s] in.", __func__);

	if(m_nfState == STREAM_Q_FULL)
	{
		LOGERR("[%s]:[%5d] m_nfState = [%d].", __func__, __LINE__, m_nfState);
		return -1;
	}
	
	MaxSize =  m_nMaxBufferSize;
	WriteBufPtr = m_nWriteBufPtr;
	
	stStreamInfo[uiHead].offset = WriteBufPtr;
	stStreamInfo[uiHead].size = ReqSize;
	stStreamInfo[uiHead].timestamp = TimeStamp;
	stStreamInfo[uiHead].result = result;

	if( m_WFDStream == 1)
		stStreamInfo[uiHead].mPacketDiscont = PacketDiscont;

	if( pstSecureInfo )
		stStreamInfo[uiHead].secure_info = *pstSecureInfo;
	else
		stStreamInfo[uiHead].secure_info.ulSecureType = 0;

	if(ptrData)
	{
		ptrWrite = m_pBuffer + WriteBufPtr;
		if(ReqSize) memcpy((void *)ptrWrite, (void *)ptrData, ReqSize);
	}
	LOGINFO("[%s] WritePointer[%x] Size[%d]", __func__, ptrWrite, ReqSize);

	m_nWriteBufPtr = WriteBufPtr + (((ReqSize+15) >> 4) << 4);

	LOGINFO("[%s] out.", __func__);
	
	return ReqSize;	
}

int32_t StreamQueue::Push()
{
	unsigned int	uiTail, uiHead;

	LOGINFO("[%s] in.", __func__);

	if(m_nfState == STREAM_Q_FULL)
	{
		LOGERR("[%s]:[%5d] m_nfState = [%d].", __func__, __LINE__, m_nfState);
		return 0;
	}
	
	uiTail = m_nReadFrmIdx;
	uiHead = m_nWriteFrmIdx;
	uiHead++;
	
	if( uiHead == m_nIdxNum )
		uiHead = 0;

	if(uiTail  == uiHead)
		m_nfState = STREAM_Q_FULL;
	else
		m_nfState = STREAM_Q_NORMAL;

	m_nWriteFrmIdx = uiHead;

	LOGINFO("[%s] out.", __func__);
	
	return 1;
}


int32_t StreamQueue::Pop()
{
	unsigned int	uiTail, uiHead;
	unsigned int	ReadBufPtr;

	LOGINFO("[%s] in.", __func__);
	
	if(m_nfState == STREAM_Q_EMPTY)
	{
		LOGERR("[%s]:[%5d] m_nfState = [%d].", __func__, __LINE__, m_nfState);
		return 0;
	}
	
	uiTail = m_nReadFrmIdx;
	uiHead = m_nWriteFrmIdx;

	ReadBufPtr = stStreamInfo[uiTail].offset + (((stStreamInfo[uiTail].size+15) >> 4) << 4);
	
	uiTail++;
	if( uiTail == m_nIdxNum )
		uiTail = 0;

	if(uiTail  == uiHead)
		m_nfState = STREAM_Q_EMPTY;
	else
		m_nfState = STREAM_Q_NORMAL;

	m_nReadFrmIdx = uiTail;
	m_nReadBufPtr = ReadBufPtr;
	
	LOGINFO("[%s] out.", __func__);
	
	return 1;
}
