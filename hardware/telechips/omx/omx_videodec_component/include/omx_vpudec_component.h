/**
  @file src/components/ffmpeg/omx_videodec_component.h
  
  This component implements All Video decoder. (MPEG4/AVC/H.263/WMV/RV)
  The H.264 / MPEG-4 AVC Video decoder is based on the FFmpeg software library.

  Copyright (C) 2007-2008 STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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

  $Date: 2009/03/10 13:33:29 $
  Revision $Rev: 554 $
  Author $Author: B060934 $
  Android revised by ZzaU.
*/

#ifndef _OMX_VPUDEC_COMPONENT_H_
#define _OMX_VPUDEC_COMPONENT_H_

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Video.h>
#include <OMX_TCC_Index.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <omx_base_filter.h>

#include <string.h>
#include "TCCMemory.h"

#include <tcc_video_common.h>

#include <vdec.h>

/** Video Decoder component private structure.
  */
/**
 * Pixel format. Notes:
 *
 * PIX_FMT_RGB32 is handled in an endian-specific manner. A RGBA
 * color is put together as:
 *  (A << 24) | (R << 16) | (G << 8) | B
 * This is stored as BGRA on little endian CPU architectures and ARGB on
 * big endian CPUs.
 *
 * When the pixel format is palettized RGB (PIX_FMT_PAL8), the palettized
 * image data is stored in AVFrame.data[0]. The palette is transported in
 * AVFrame.data[1] and, is 1024 bytes long (256 4-byte entries) and is
 * formatted the same as in PIX_FMT_RGB32 described above (i.e., it is
 * also endian-specific). Note also that the individual RGB palette
 * components stored in AVFrame.data[1] should be in the range 0..255.
 * This is important as many custom PAL8 video codecs that were designed
 * to run on the IBM VGA graphics adapter use 6-bit palette components.
 */
 
/* 
   VPU frame skip (or search) mode definitions
*/
#define SKIPMODE_NONE               0
#define SKIPMODE_I_SEARCH			1
#define SKIPMODE_B_SKIP				2
#define SKIPMODE_EXCEPT_I_SKIP		3
#define SKIPMODE_NEXT_B_SKIP        4
#define SKIPMODE_NEXT_STEP          10

#define SKIPMODE_IDR_SEARCH         21	// (SKIPMODE_I_SEARCH)
#define SKIPMODE_NONIDR_SEARCH      22	// (SKIPMODE_I_SEARCH)

/* 
   VPU ring-buffer state structures
*/
typedef struct ringbuff_state_t {
	OMX_PTR   pVpuInstance;
	OMX_U8   *pRingBuffBase[2];
	OMX_U8   *pRingBuffEnd[2];
	OMX_U8   *pReadPtr;
	OMX_U8   *pWritePtr;
	OMX_U8   *pPrevReadPtr;
	OMX_U8   *pPrevWritePtr;

	OMX_U8   *pBackupBuffer;  // for resolution change
	OMX_U8   *pPrevRingBuffBase;
	OMX_U8   *pPrevRingBuffEnd;
	OMX_S32   lBackupStartOffset;
	OMX_S32   lBackupLength;

	OMX_S32   lRingBuffSize;
	OMX_S32   lWrittenBytes;
	OMX_S32   lEmptySpace;
	OMX_S32   lUsedByte;
	OMX_BOOL  bFlushRing;
} ringbuff_state_t;

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
	OMX_U8        *pBasePtr; //for debugging
} input_queue_t;

/* 
   Output buffer index queue structures for vsync mode
*/
#define REFERENCE_BUFFER_MAX 32
typedef struct dispidx_queue_t {
	OMX_U32  aulDispIdxRing[REFERENCE_BUFFER_MAX];
	OMX_U32  aulBuffIdRing[REFERENCE_BUFFER_MAX];
	OMX_S32  lQueCount;
	OMX_S32  lStartPos;
	OMX_S32  lEndPos;
} dispidx_queue_t;

/* 
   Output buffer index list structures for non-vsync mode
*/
typedef struct dispbuff_state_t {
	OMX_PTR  pOmxOutBuff;
	OMX_BOOL bIsCleared;
	OMX_S32  alKeepedIndex[2];
	OMX_S32  alDisplayIndex[2];
} dispbuff_state_t;

typedef struct dispidx_list_t {
	dispbuff_state_t  astDispBuffState[REFERENCE_BUFFER_MAX];
	OMX_S32           lUsedCount;
	OMX_S32           lMaxCount;
	OMX_PTR           pVpuInstance;
	OMX_S32           hFBDevice;
} dispidx_list_t;

/* 
   Display information manager structures
*/
typedef struct disp_info_t {
	OMX_S32      lFrameType;           // Frame type
	OMX_TICKS    llTimestamp;          // Timestamp (us)
	OMX_TICKS    llPrevTimestamp;      // Timestamp (us) for error control
	OMX_S32      lRvTimestamp;         // TR fo RealVideo timestamp
	OMX_S32      lPicStructure;        // PictureStructure
	OMX_S32      lM2vFieldSequence;    // Field sequence(MPEG2) 
	OMX_S32      lFrameDurationFactor; // MPEG2 frame duration factor
	OMX_S32      lFrameSize;           // Frame size
	OMX_U32      ulBufferFlags;        // Output buffer flags
	OMX_BOOL     bIsMvcDependent;
	OMX_BOOL     bPtsNotExist;
	OMX_BOOL     bIsValid;
} disp_info_t;

typedef struct dispinfo_manager_t {
	disp_info_t  astDispInfoList[REFERENCE_BUFFER_MAX];
	OMX_S32      lStreamType;          // stream type (STD_XXX)
	OMX_S32      lFrameRate;           // frame-rate (fps*1000)
	OMX_S32      lFrameDurationFactor; // MPEG2 frame duration factor
	OMX_TICKS    llFrameDuration;      // frame-duration (us, 1000000/fps)
	OMX_TICKS    llLastBaseTimestamp;  // Last base timestamp
	OMX_TICKS    llLastOutTimestamp;   // Last output timestamp
	OMX_TICKS    llLastDecTimestamp;   // Last decoded timestamp
	OMX_TICKS    llShowedTimestamp;    // Last showed timestamp
	OMX_BOOL     bBackwardPlayback;    // playback direction

	OMX_TICKS    lRvTimestamp[2];
	OMX_S32      lRvTimeReference[2];
	OMX_S32      lRvReferenceIdx;
} dispinfo_manager_t;


/* 
   Gralloc mode structures
*/
//typedef struct PORT_TYPE
//{
//	BUFFER_TYPE BufferType;   	/*Used when buffer pointers which come from the normal virtual space */
//	OMX_U32 IsBuffer2D;   		/*Used when buffer pointers which come from Gralloc allocations */
//} PORT_TYPE;

//typedef enum COPY_MODE {
//	COPY_NONE,
//	COPY_START,
//	COPY_DONE,
//	COPY_FAILED,
//	COPY_PRIV_DATA
//} COPY_MODE;

typedef struct gralloc_info_t {
#ifdef ANDROID_USE_GRALLOC_BUFFER
	gralloc_module_t const *pstGrallocModule;
#endif
	OMX_S32      hCopyDev;
	OMX_PTR      pCopyParam;
	OMX_S32      lStrideY;
	OMX_S32      lStrideC;
	OMX_S32      lFrameSizeY;
	OMX_S32      lFrameSizeC;
	OMX_S32      lFrameSize;
	OMX_U8      *pDispOut[2][3];	//!< physical address of Y, Cb, Cr component
	OMX_BOOL     bBufferLocked;
} gralloc_info_t;


/* 
   Structures etc
*/
typedef struct output_info_t {
	OMX_S32      lFrameType;           // Frame type
	OMX_S32      lPicStructure;        // PictureStructure
	OMX_TICKS    llTimestamp;          // Timestamp (us)
	OMX_U32      ulBufferFlags;        // Output buffer flags
	OMX_BOOL     bIsMvcDependent;
} output_info_t;



DERIVEDCLASS(omx_vpudec_component_PrivateType, omx_base_filter_PrivateType)
#define omx_vpudec_component_PrivateType_FIELDS omx_base_filter_PrivateType_FIELDS \
  /** @param VPU input/output parameters */ \
  OMX_PTR             pVpuInstance; \
  vdec_init_t         stVpuInit; \
  vdec_input_t        stVpuInput; \
  vdec_output_t       stVpuOutput; \
  vdec_user_info_t    stVpuUserInfo; \
  /** @param video metadata from Telechips extractor */ \
  cdmx_info_t        *pstCdmxInfo;\
  /** @param helper for VPU ring-buffer operation */ \
  ringbuff_state_t    stRingBuffState; \
  input_queue_t       stInputInfoQueue; \
  dispidx_queue_t     stDispIdxQueue; \
  dispidx_list_t      stDispBuffList; \
  dispinfo_manager_t  stDispInfoMgr; \
  /** @param helper for VPU ring-buffer operation */ \
  gralloc_info_t      stGrallocInfo; \
  /** @param resolution change */ \
  OMX_CONFIG_RECTTYPE stCropRectParam; \
  /** @param Aspect Ratio change */ \
  OMX_CONFIG_SCALEFACTORTYPE stScaleFactor; \
  /** @param extra data */ \
  OMX_U8       *pbyExtraDataBuff; \
  OMX_S32       lExtraDataLength; \
  /** @param private flags */ \
  OMX_U32       ulFlags; \
  /** @param Component unique ID for platform private data */ \
  OMX_U32       ulComponentUID; \
  /** @param Semaphore for waiting LibInit() */ \
  tsem_t*       pstCodecSyncSem; \
  /** @param VPU initialization state */ \
  OMX_BOOL     *pbIsVpuClosed; \
  OMX_S32      *plVpuPreOpenFD; \
  /** @param extractor/container type from extractor */ \
  OMX_U32       ulExtractorType; \
  OMX_U32       ulContainerType; \
  /** @param one input per one frame mode */ \
  OMX_U32       bSingleFrameInput; \
  /** @param VPU error occurrance checking */ \
  OMX_S32       lSeqInitCount; \
  /** @param VPU reference frame count */ \
  OMX_S32       lRefBuffCount; \
  /** @param input buffer counting */ \
  OMX_S32       lInputCount; \
  /** @param decoding/output fail counting */ \
  OMX_S32       lThumbFailCount; \
  OMX_S32       lOutputFailCount; \
  OMX_S32       lOutputFailMax; \
  /** @param decode only mode */\
  OMX_S32       lDecFrameSkipCount; \
  OMX_S32       lDecFrameSkipMax; \
  OMX_S32       lFrameSkipErrorCount; \
  OMX_S32       lIdrSliceScanCount; \
  /** @param VPU frame skip(or search) mode */ \
  OMX_S32       lFrameSkipMode; \
  /** @param about input data feeding (buffering) */ \
  OMX_TICKS     llQueuedStartTime; \
  OMX_TICKS     llFeedMaxTimeDiff; \
  OMX_TICKS     llFeedMinTimeDiff; \
  /** @param frame-rate update */ \
  OMX_S32       lFrameSuccessCount; \
  OMX_S32       lNewFrameRate; \
  /** @param sequence header backup */ \
  OMX_U8       *pSequenceHeader; \
  OMX_S32       lSeqHeaderLength; \
  /** @param thumbnail output buffer */\
  OMX_U8       *pThumbnailBuff; \
  OMX_S32       lThumbnailLength; \
  /** @param VSync buffer id generation */\
  OMX_U32       nVSyncBuffId; \
  /** @param framebuffer device handle for VSync mode */ \
  OMX_S32       hFBDevice; \
  /** @param tmem device handle for system buffers etc */ \
  OMX_S32       hTMemDevice; \
  /** @param UMP base for gralloc mode */ \
  OMX_S32       lUMPMapSize; \
  OMX_PTR       pUMPBase[2]; \
  OMX_S32       lTryGrallocCopyCount; \
  /** @param Sequence header buffer for hdcp source mode */ \
  OMX_S32       lSeqBuffMapSize; \
  OMX_PTR       pSeqBuffBase[2]; \
  /** @param Platforma private to serve display device */ \
  OMX_PTR       pOutputPortPrivate; \
  /** @param MVC output */ \
  OMX_S32       lMVCBaseViewIndex; \
  OMX_PTR       pMVCBaseView[3]; \
  
ENDCLASS(omx_vpudec_component_PrivateType)

OMX_BOOL
omx_vpudec_component_InitVpuRingbufferCallback(
	OMX_COMPONENTTYPE   *openmaxStandComp,
	OMX_BOOL             bModeChange
	);

void
omx_vpudec_component_DeinitVpuRingbufferCallback(
	OMX_COMPONENTTYPE   *openmaxStandComp
	);

void
omx_vpudec_component_SetParameter(
	OMX_COMPONENTTYPE   *openmaxStandComp,
	OMX_INDEXTYPE        nParamIndex,
	OMX_PTR              pComponentParameterStructure
	);

void
omx_vpudec_component_SetConfig(
	OMX_COMPONENTTYPE   *openmaxStandComp,
	OMX_INDEXTYPE        nConfigIndex,
	OMX_PTR              pComponentConfigStructure	
	);

void
omx_vpudec_component_GetConfig(
	OMX_COMPONENTTYPE   *openmaxStandComp,
	OMX_INDEXTYPE        nIndex,
	OMX_PTR              pComponentConfigStructure
	);

void 
omx_vpudec_component_BufferMgmtCallback(
	OMX_COMPONENTTYPE		*openmaxStandComp, 
	OMX_BUFFERHEADERTYPE	*pInputBuffer, 
	OMX_BUFFERHEADERTYPE	*pOutputBuffer
	);

#endif //_OMX_VPUDEC_COMPONENT_H_

