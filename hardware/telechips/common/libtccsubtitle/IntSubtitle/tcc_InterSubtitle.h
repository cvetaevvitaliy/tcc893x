#ifndef __TCC_INTER_SUBTITLE_H__
#define __TCC_INTER_SUBTITLE_H__

#include "TCC_GS_DECODER.h"
#include <utils/threads.h>
#include <libpmap/pmap.h>
#include <string.h>
#include <stdio.h>
#include "tcc_resizer.h"

#define	SUBTITLE_RINGBUF_SIZE		(128*1024)
#define	SUBTITLE_ONE_ELEMENT_MAX 	(64*2*5)		// max line 4 and 1 margin line
#define	SUBTITLE_MAX_FIFO_NUM		(SUBTITLE_RINGBUF_SIZE/SUBTITLE_ONE_ELEMENT_MAX)

#define FBDEV0  			"/dev/graphics/fb0"
#define OVERLAY1			"dev/overlay1"
#define M2M4PGS 	 		"/dev/scaler"
#define COMPOSITE_DEVICE 	"/dev/composite"
#define COMPONENT_DEVICE 	"/dev/component"

//#define PGS_DECOUT_DUMP
//#define GFIFO_USE_LOCK
using namespace android;
/*For M2TS PGS Stream*/
#define PGS_QUEUE_MAX_SIZE	10 

#ifdef GFIFO_USE_LOCK
#include "oscl_semaphore.h"
#endif
class tccGFifo
{
public:
	tccGFifo(void);
	~tccGFifo(void);


	typedef struct{
	int iRead;
	int iWrite;
	int iCurEleNum;
	int iMaxEleNum;	
	
#ifdef GFIFO_USE_LOCK		
	OsclSemaphore *semaLock ; // semaphore locking
#endif		
	int *pFifo;		// baseaddress of frame
	}	GFIFO_OBJECT;

	#ifdef GFIFO_USE_LOCK
	// it should be used for a/v buffering.
	static void GFIFO_WaitAndLock(GFIFO_OBJECT * pGFIFO)			
	{
		if( pGFIFO->semaLock )
			pGFIFO->semaLock->Wait() ;	
	}

	static void GFIFO_Unlock(GFIFO_OBJECT * pGFIFO)					
	{
		if( pGFIFO->semaLock )
			pGFIFO->semaLock->Signal() ;	
	}
	#else
	#define	GFIFO_WaitAndLock(handle)
	#define	GFIFO_Unlock(handle)
	#endif

public:
	unsigned int GFIFO_Need_MemSize( unsigned int element_num )
	{
		return (sizeof(GFIFO_OBJECT) + element_num*sizeof(int*) ) ;
	}


	int GetEmptyEle(void* handle)
	{
		GFIFO_OBJECT *pGFIFO;
		int iElePos;

		pGFIFO = (GFIFO_OBJECT*)handle;

		
		if( pGFIFO->iCurEleNum < pGFIFO->iMaxEleNum )
		{
			iElePos = pGFIFO->iWrite;
		
			return iElePos;	
		}
		
		return -1;
	} ;

	int GetEmptyEleNum(void* handle)
	{
		GFIFO_OBJECT *pGFIFO;

		pGFIFO = (GFIFO_OBJECT*)handle;

		
		return pGFIFO->iMaxEleNum - pGFIFO->iCurEleNum;
	} ;

	int GetValidEle(void* handle, int iEleIdx)
	{
		GFIFO_OBJECT *pGFIFO;

		pGFIFO = (GFIFO_OBJECT*)handle;

		
		if( iEleIdx >= pGFIFO->iCurEleNum )
			return NULL;

		iEleIdx += pGFIFO->iRead;

		if( iEleIdx >= pGFIFO->iMaxEleNum )
			iEleIdx -= pGFIFO->iMaxEleNum;

		return pGFIFO->pFifo[iEleIdx];
	} ;

	int GetValidEleNum(void* handle)
	{
		GFIFO_OBJECT *pGFIFO;

		pGFIFO = (GFIFO_OBJECT*)handle;

		
		return pGFIFO->iCurEleNum;
	}
	
	int Push(void* handle, int iEleValue)
	{
		GFIFO_OBJECT *pGFIFO;
		int iElePos;

		pGFIFO = (GFIFO_OBJECT*)handle;

		
		if( pGFIFO->iCurEleNum < pGFIFO->iMaxEleNum )
		{
			pGFIFO->pFifo[pGFIFO->iWrite] = iEleValue;	

			iElePos = pGFIFO->iWrite;

			GFIFO_WaitAndLock(pGFIFO);
			pGFIFO->iCurEleNum++;
			pGFIFO->iWrite++;
			if( pGFIFO->iWrite >= pGFIFO->iMaxEleNum )
				pGFIFO->iWrite = 0;

			GFIFO_Unlock(pGFIFO);			
			return iElePos;	
		}

		return -1;
	} ;

	int Pop(void* handle)
	{
		GFIFO_OBJECT *pGFIFO;
		int iEleValue;

		pGFIFO = (GFIFO_OBJECT*)handle;

		
		if( pGFIFO->iCurEleNum > 0 )
		{
			iEleValue = pGFIFO->pFifo[pGFIFO->iRead];	

			GFIFO_WaitAndLock(pGFIFO);
			pGFIFO->iCurEleNum--;
			pGFIFO->iRead++;
			if( pGFIFO->iRead >= pGFIFO->iMaxEleNum )
				pGFIFO->iRead = 0;

			GFIFO_Unlock(pGFIFO);			
			return iEleValue;	
		}

		return -1;
	};

	
	int GetFirst(void* handle, int* pEleValue)
	{
		GFIFO_OBJECT *pGFIFO;

		pGFIFO = (GFIFO_OBJECT*)handle;

		
		if( pGFIFO->iCurEleNum > 0 )
		{
			*pEleValue = pGFIFO->pFifo[pGFIFO->iRead];

			return pGFIFO->iRead;
		}

		*pEleValue = -1;

		return -1;
	} ;

	void Flush(void* handle)
	{
		GFIFO_OBJECT *pGFIFO;

		pGFIFO = (GFIFO_OBJECT*)handle;

		
		GFIFO_WaitAndLock(pGFIFO);			

		pGFIFO->iRead = pGFIFO->iWrite;		
		pGFIFO->iCurEleNum = 0;	

		GFIFO_Unlock(pGFIFO);			
	} ;

	// except iEleIdx element, the other will be flushed.	
	void FlushEx(void* handle, int iEleIdx )
	{
		GFIFO_OBJECT *pGFIFO;

		pGFIFO = (GFIFO_OBJECT*)handle;

		

		if( iEleIdx >= pGFIFO->iCurEleNum )
			return ;

		GFIFO_WaitAndLock(pGFIFO);			

		iEleIdx += pGFIFO->iRead;

		if( iEleIdx >= pGFIFO->iMaxEleNum )
			iEleIdx -= pGFIFO->iMaxEleNum;

		if( iEleIdx+1 >= pGFIFO->iMaxEleNum )
			pGFIFO->iWrite = 0 ;
		else
			pGFIFO->iWrite = iEleIdx+1 ;

		pGFIFO->iRead = iEleIdx ;
		pGFIFO->iCurEleNum = 1 ;

		GFIFO_Unlock(pGFIFO);			
	} ;


	
	void* Create(char* pWorkingBuf, unsigned int uiWorkingBufSize, unsigned int uiNumOfElement)
	{
		GFIFO_OBJECT *pGFIFO;

				
		if( uiWorkingBufSize < GFIFO_Need_MemSize(uiNumOfElement) )
			return 0;

		pGFIFO = (GFIFO_OBJECT *)pWorkingBuf;

		memset( (void*)pGFIFO, 0x00, sizeof(GFIFO_OBJECT));

		pGFIFO->pFifo= (int *)(pWorkingBuf + sizeof(GFIFO_OBJECT)) ;
		pGFIFO->iMaxEleNum = uiNumOfElement;
		
#ifdef GFIFO_USE_LOCK
		pGFIFO->semaLock = new OsclSemaphore() ;
#endif
		return (void*)pGFIFO;
	};

	void Close(void* handle)
	{
		GFIFO_OBJECT *pGFIFO;

		pGFIFO = (GFIFO_OBJECT*)handle;
				
		Flush(handle);
		
#ifdef GFIFO_USE_LOCK	
		if( pGFIFO->semaLock )
		{
			pGFIFO->semaLock->Close() ;
			delete pGFIFO->semaLock ;
		}
#endif
	}	;
};

typedef struct
{
	unsigned uStartTm ;
	unsigned uEndTm ;
	unsigned uDataSize ;
	unsigned char *pData ;	
} TCC_SUBTITLE_ELEMENT ;

typedef struct TCC_PGS_STREAM
{
	/*for image Subtitle*/
	unsigned int 	m_uiSrcWidth;//width of Caption image buffer
	unsigned int	m_uiSrcHeight;//height of Caption image buffer
	unsigned int 	m_uiDstWidth;//width of Caption image buffer
	unsigned int	m_uiDstHeight;//height of Caption image buffer
	unsigned int	m_uiOffsetX;
	unsigned int	m_uiOffsetY;
	unsigned long	m_ulStartTimeStamp;			//!< [out] the timestamp of outstream
	unsigned long	m_ulEndTimeStamp;		//!< [out] the end timestamp of outstream: This is not mandatory except in the case of text-subtitle
	unsigned long	m_ulStreamDataSize;		//!< [out] the size to outstream data
	unsigned char  *m_pbyStreamData;		//!< [out] the pointer to outstream data
	unsigned long	m_ulSubtitleFormat;		//!Subtitle Format
	unsigned int	isStreamEnqueued; 	
	unsigned int	isReadyToDisplay; 	
	unsigned short *m_pCaption;			//pointer to Caption image buffer
	void		   *m_pSpecificData;		//!< [out] the pointer to specific output of demuxer(if demuxer has a specific data)
} TCC_PGS_STREAM;

typedef struct
{
	unsigned head ;
	unsigned tail ;
	unsigned ringBufSize ;
	unsigned unitBufSize ;
	void *pGFifo ;	
	unsigned char *pRingBuf ;	
} TCC_RINGBUF_MAN ;

typedef enum
{
	SUBTITLETYPE_SRT = 0x0,
	SUBTITLETYPE_SSA,		
	SUBTITLETYPE_USF,		
	SUBTITLETYPE_BMP,		
	SUBTITLETYPE_VOBSUB,	
	SUBTITLETYPE_KATE,
	SUBTITLETYPE_MAX,

} SUBTITLE_TYPE ;


typedef enum
{
	OUTPUT_DEVICE_NONE = 0x0,
	OUTPUT_DEVICE_LCD,
	OUTPUT_DEVICE_EXTEND_DEVICE,		

} OUTPUT_DEVICE_TYPE ;

class tccInterSubtitle
{
public :
	tccInterSubtitle(void) ;
	~tccInterSubtitle(void) ;

private :
	tccGFifo m_clsGFifo ;
	TCC_RINGBUF_MAN m_stBufMan ;

	TCC_SUBTITLE_ELEMENT m_eles[100] ;
	int m_tail, m_head ;
	
	//For PGS
	gsdhandle_t	hGsd;
	unsigned int gsd_output_len;
	unsigned char	*pGSOut;			// original output size (no rescaled)

public :
	void Create( unsigned int iFifoNum, unsigned int iEleBufSize, unsigned int iOneEleMaxSize ) ;
	void Destroy() ;

	int SetSubtitle2( SUBTITLE_TYPE eSubType, unsigned int uStartTime, unsigned int uEndTime, unsigned int uDataSize, void *pSubData ) ;
	int SetSubtitle( SUBTITLE_TYPE eSubType, unsigned int uStartTime, unsigned int uEndTime, unsigned int uDataSize, void *pSubData ) ;
	int GetSubtitle( TCC_SUBTITLE_ELEMENT **pEle ) ;
	void PopSubtitle(int nDataSize) ;
	int GetSubtitleNum() ;
	void ResetSubtitle() ;
	int SetSubtitleSeek(int msec) ;
	int ParserForSSA( TCC_SUBTITLE_ELEMENT *pEle, void *pSubData ) ;
	int ParserForSRT( TCC_SUBTITLE_ELEMENT *pEle, void *pSubData ) ;
	void gs_decode_dump(unsigned char* RGB, int width, int height);
	int gs_decoder_open(long lVideoWidth,long lVideoHeight) ;
	gsdresult_t  gs_decoder_close(void) ;
	gsdresult_t  gs_decode(int uStartTime,unsigned int uDataSize, void *pSubData);
	gsdresult_t  gs_decoder_reset(void) ;

	class CaptionThread:public Thread
	{
		tccInterSubtitle *mCaptionDispThread;

		public :
			CaptionThread(tccInterSubtitle *hw )
			: Thread(false),mCaptionDispThread(hw){ }
			
		  virtual bool threadLoop()
		  {
			mCaptionDispThread->CaptionDisplayProcess();
			return false;
		  }
	};

    // protected by mLock 
    sp<CaptionThread>   mCaptionDispThread;
    mutable Mutex       mLock;
    bool Threadloop;
	
	TCC_PGS_STREAM  PGS_QUEUE[PGS_QUEUE_MAX_SIZE];


	int mM2m_fd;
	int mFb_fd;
	int mOverlay_fd;
	int mComposite_fd;
	int mComponent_fd;

	unsigned int OutputMode;
	unsigned int CompositeMode;
	unsigned int ComponentMode;
	
	unsigned int toggle_dualbuf;
	
	char* Input_PhyAddr;
	char* Prior_Output_PhyAddr;
	char* Output_PhyAddr;

	unsigned char *Input_VirAddr;
	unsigned char* Prior_Output_VirAddr;
	unsigned char *Output_VirAddr;

	pmap_t mTempPmap;
	pmap_t mPriorTempPmap;

	unsigned int HorizontalMediaSize;
	unsigned int VerticalMediaSize;
	unsigned int HorizontalTargetSize;
	unsigned int VerticalTargetSize;
	
	int PgsQueueReadIdx;
	int PgsQueueWriteIdx;
	int CurrentPlaybackPosition;
	unsigned int PGSDisplayOnOFF;
	
	void InitializePGSQueue();
	void ClearPGSQueue();
	
    TCC_PGS_STREAM* GetDecodedPGS( void );

	int  InitOverlayForCaptionDisplay();
	void DeinitOverlayForCaptionDisplay();

	int GetDisplaySize(unsigned int OutPut_Mode, unsigned int *Display_Width, unsigned int *Display_Height);
	int ResizeCaptionImage(TCC_PGS_STREAM *PGSINFO);
	int ResizeCaptionImage(TCC_PGS_STREAM *PGSINFO,unsigned int* dst_x, unsigned int* dst_y, unsigned int* dst_w, unsigned int* dst_h);
	int DisplayCaption(TCC_PGS_STREAM *PGSINFO);

	int SetCurrentPlaybackPosition(int msec);
	int GetCurrentPlaybackPosition();

	int SetPGSOnOff(int OnOff);

	int CreateCaptionDisplayThread();
	void DestoryCaptionDisplayThread();

	int CaptionDisplayProcess();
};
#endif /* __TCC_INTER_SUBTITLE_H__ */
