/**

  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

/****************************************************************************
 *   FileName    : vpu_ringbuffer_struct.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#ifndef _OMX_VPU_RINGBUFFER_STRUCT_H_
#define _OMX_VPU_RINGBUFFER_STRUCT_H_

#include <OMX_Types.h>
#include <vdec.h>

#define CODETYPE_NONE		(0x00000000)
#define CODETYPE_HEADER		(0x00000001)
#define CODETYPE_PICTURE	(0x00000002)
#define CODETYPE_ALL		(CODETYPE_HEADER | CODETYPE_PICTURE)

#define EMPTY_SIZE_MIN                  1024*2   // VPU ring-buffer margine between read ptr and write ptr

#define FEED_LIMIT_TIMEDIFF_INIT        70000 // init as 70ms
#define ROLLBACK_CHECKOUT_COUNT         10

#define INPUT_INFO_QUEUE_INIT_SIZE		64

/* build options */
#define AVC_DECODE_ONLY_MODE_ENABLE           1
#define FLUSH_RING_BEFORE_UPDATE_TIME         0
#define UPDATE_WRITE_PTR_WITH_ALIGNED_LENGTH  1
#define WRITE_PTR_ALIGN_BYTES                 512
#define USE_AVAILABLE_SIZE_FROM_VPU           0
#define RINGBUFF_UPDATE_AT_DECODING_TIME      0

#define FEED_COMPLETE	0
#define NEED_MORE_DATA	1
#define FEED_FAILED		2

#define ERROR(...)  ALOGE(__VA_ARGS__)

#define ASSERT(cond) \
	if(!(cond)){\
		ALOGE("ASSERTION FAILED!"); \
		ALOGE("  - Position: %s (%d line) : %s()", __FILE__, __LINE__, __FUNCTION__); \
		ALOGE("  - Expression: " #cond); \
	}

/* 
   VPU ring-buffer state structures
*/
typedef struct ringbuff_info_t {
	OMX_PTR   pVpuInstance;
	OMX_U8   *pRingBuffBase[2];
	OMX_U8   *pRingBuffEnd[2];
	OMX_U8   *pReadPtr;
	OMX_U8   *pWritePtr;
	OMX_U8   *pPrevReadPtr;
	OMX_U8   *pPrevWritePtr;
	OMX_S32   lRingBuffSize;
	OMX_S32   lWrittenBytes;
	OMX_S32   lEmptySpace;
	OMX_S32   lUsedByte;
	OMX_BOOL  bFlushRing;
} ringbuff_info_t;

/* 
   Input buffer information queue structures
*/
typedef struct input_info_t {
	OMX_TICKS  llTimestamp;
	OMX_S32    lFilledLen;
	OMX_U8    *pStartPtr;
	OMX_U8    *pEndPtr;
} input_info_t;

typedef struct input_queue_t {
	input_info_t  *pstInfoRing;
	OMX_S32        lQueSize;
	OMX_S32        lQueCount;
	OMX_S32        lStartPos;
	OMX_S32        lEndPos;
} input_queue_t;

typedef struct ringbuff_state_t {
	/** @param helper for VPU ring-buffer operation */
	ringbuff_info_t		 stRingBuffInfo;
	input_queue_t		 stInputInfoQueue;
	/** @param input buffer counting */
	OMX_S32				 lInputCount;
	OMX_S32				 lRollbackCheckCount;
	/** @param about input data feeding (buffering) */
	OMX_TICKS			 llQueuedStartTime;
	OMX_TICKS			 llFeedLimitTimeDiff;
	/** @param frame-rate update */
	OMX_S32				 lFrameSuccessCount;
	OMX_S32				 lNewFrameRate;

	vdec_input_t		*pstVDecInput;
	vdec_output_t		*pstVDecOutput;
	OMX_BOOL			*pisVPUClosed;
	OMX_BOOL			*pavcodecInited;
} ringbuff_state_t;

OMX_BOOL InitInputQueue(input_queue_t *pstQue, OMX_S32 lInitSize);
void DeinitInputQueue(input_queue_t *pstQue);
void ClearInputQueue(input_queue_t *pstQue);
OMX_S32 GetInputInfo(input_queue_t *pstQue, input_info_t *pstInput);
OMX_S32 ShowInputInfo(input_queue_t *pstQue, input_info_t *pstInput);
OMX_S32 ShowLastInputInfo(input_queue_t *pstQue, input_info_t *pstInput);
OMX_S32 ClearFirstInputInfo(input_queue_t *pstQue);

OMX_BOOL InitRingBuffer(ringbuff_info_t *pstRingInfo, OMX_PTR pVpuInstance);
OMX_S32 UpdateRingBuffer(ringbuff_info_t *pstRingInfo, OMX_BOOL bFromVpu);
OMX_S32 PushInputInfo(input_queue_t *pstQue, const input_info_t *pstInput);
OMX_BOOL FillRingBuffer(ringbuff_info_t *pstRingInfo, OMX_U8 *pInputBuff, OMX_S32 lInputLength);
OMX_S32 ResetRingBuffer(ringbuff_info_t *pstRingInfo);
OMX_S32 FeedVpuDecoder(ringbuff_state_t *pstRingBuffState, OMX_BUFFERHEADERTYPE *pInputBuffer, OMX_U8 *pBuffer, OMX_U32 nFilledLen, OMX_TICKS llTimeStamp, OMX_BOOL bFeedFlag);
OMX_TICKS GetCurrTimestamp(ringbuff_state_t *pstRingBuffState, OMX_TICKS *pllTimestamp, OMX_BOOL bDecodeSuccess, OMX_BOOL bInterlaced);

OMX_U32 ScanSequenceHeader(OMX_U32 video_coding_type, OMX_U8* inputCurrBuffer, OMX_U32 inputCurrLength, OMX_U32 *sequence_offset, OMX_U32 search_option, OMX_U8 uiDecoderID);
void SetRingBuffStateInstance(OMX_PTR pInst, OMX_U32 uiDecoderID);
OMX_PTR GetRingBuffStateInstance(OMX_U32 uiDecoderID);
#endif
