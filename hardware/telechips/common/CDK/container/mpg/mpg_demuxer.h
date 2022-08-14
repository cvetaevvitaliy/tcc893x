/*
 * ________  _____           _____   _____           ____  ____   ____		
 *    /     /       /       /       /       /     /   /    /   \ /			
 *   /     /___    /       /___    /       /____ /   /    /____/ \___			
 *  /     /       /       /       /       /     /   /    /           \		
 * /     /_____  /_____  /_____  /_____  /     / _ /_  _/_      _____/ 		
 *  																				
 * Copyright (c) 2008 Telechips Inc.
 * Korad Bldg, 1000-12 Daechi-dong, Kangnam-Ku, Seoul, Korea					
 *
 *
 *
 * TCCXXX_MPG_Parser.h
 *
 * This file is api of libmpg.
 */
#ifndef _MPG_H_
#define _MPG_H_

#include "../av_common_types.h"

#define MPG_BASIC_FILECACHE_SIZE	(1 * 1024)

//Operation Type
#define MPG_OP_INIT			0
#define MPG_OP_CLOSE			1
#define MPG_OP_SUBTITLE		2//Decode RLE bitmap for subtitle
#define MPG_OP_SEEK			3
#define MPG_OP_GETSTREAM		4	//this means decoding
#define MPG_OP_SELSTREAM		5
#define MPG_OP_SCAN			7

//Operation Mode
	//* In case of Sequential Mode, parser doesn't consider the type of current packet(Video or Audio).
	//Parser just decode packets in order.

	//* Interleaved mode could be used in case that multiplexing between video and audio is very bad.
	//In that case, Video FIFO or Audio FIFO could be overflowed to keep precise PTS.
	//If interleaved mode is used, user can choose the type of data(Audio or Video) in every decoding point.
	//In other word, if user wants video data, parser would decode next video data, and vice versa.
#define MPG_MODE_SEQUENTIAL		0x0
#define MPG_MODE_INTERLEAVED		0x1

typedef int				mpg_handle_t;
typedef int				mpg_result_t;

typedef struct mpg_video_info_t
{
	unsigned int m_Index;
	unsigned int m_FourCC;
	unsigned int m_Width;
	unsigned int m_Height;
	unsigned int m_AspectRatio;//(height / width) * 10000
	unsigned int m_FrameRate; // fremarate * 1000;
	unsigned int m_BitRate;

	int m_StartPTS;
	int m_EndPTS;
} mpg_video_info_t;

typedef struct mpg_audio_info_t
{
	unsigned int m_Index;
	unsigned int m_FormatId;
	unsigned int m_SamplePerSec;
	unsigned int m_BitsPerSample;
	unsigned int m_BitsRate;
	unsigned int m_Channels;
	unsigned int m_SyncWord;

	unsigned int m_ChunkSize;

	int m_StartPTS;
	int m_EndPTS;
} mpg_audio_info_t;

typedef struct mpg_dec_info_t
{
	/* file information */
	U64 m_FileSize;
	unsigned int m_Runningtime;// in milli-second

	unsigned int				m_uiTotalVideoNum;
	int									m_uiDefaultVideo;
	mpg_video_info_t *	m_pszVideoInfo;

	unsigned int				m_uiTotalAudioNum;
	int									m_uiDefaultAudio;
	mpg_audio_info_t *	m_pszAudioInfo;
} mpg_dec_info_t;

typedef struct mpg_callback_func_t
{
	void* (*m_pMalloc		 ) ( unsigned int );					//!< malloc
	void* (*m_pNonCacheMalloc) ( unsigned int );					//!< non-cacheable malloc 
	void  (*m_pFree			 ) ( void* );							//!< free
	void  (*m_pNonCacheFree	 ) ( void* );							//!< non-cacheable free
	void* (*m_pMemcpy		 ) ( void*, const void*, unsigned int );//!< memcpy
	void* (*m_pMemset		 ) ( void*, int, unsigned int );		//!< memset
	void* (*m_pRealloc		 ) ( void*, unsigned int );				//!< realloc
	void* (*m_pMemmove		 ) ( void*, const void*, unsigned int );//!< memmove

	void*		 (*m_pFopen	) (const char *, const char *);						//!< fopen
	unsigned int (*m_pFread	) (void*, unsigned int, unsigned int, void* );		//!< fread
	unsigned int (*m_pCRCFread	) (void*, unsigned int, unsigned int, void* );	//!< fread
	int			 (*m_pFseek	) (void*, long, int );								//!< fseek
	long		 (*m_pFtell	) (void* );											//!< ftell
	int			 (*m_pFseek64) ( void*, S64, int );								//!< fseek
	S64		 (*m_pFtell64) ( void* );	
	unsigned int (*m_pFwrite) (const void*, unsigned int, unsigned int, void*);	//!< fwrite
	int			 (*m_pFclose) (void *);											//!< fclose
	int			 (*m_pUnlink) ( const char *);									//!< _unlink
	unsigned int (*m_pFeof  ) (void *);											//!< feof
	unsigned int (*m_pFflush) (void *);											//!< fflush

} mpg_callback_func_t;

//! Structure for Initialization
typedef struct mpg_dec_init_t
{
	void *				m_pOpenFileName;	//!< This field should be specified in cases of both Initialization stage and Refresh Stage
	mpg_callback_func_t		m_CallbackFunc;	//!< Functions for Memory or File IO
	int				m_GetStreamMode;		//!< Sequential Mode or Interleaved Mode
	int				m_FileCacheSize;		//!< The limitation size of file cache(n case of interleave mode, parser consumes double of this size)
	int				m_ReturnDuration;		//!< If this field is set, parser returns duration(In this case, initialization stage takes more time)
	int				m_CheckRealFPS;			//!< Sometimes FPS info in the sequence header is not correct. So calculate real fps by checking PTS between two sequence header

	//Channel Selection Info
	int				m_UseCaptionInfo;		//!< If this field is set, parser returns decoded sub-picture(Caption) which has index of "m_CaptionIdx"
	int				m_CaptionIdx;			//!< This field should be specified as "0" even though "m_UseCaptionInfo" is "0".

	int				m_UseNavigationInfo;	//!< If this field is set, parser returns Navigation Data through output structure
} mpg_dec_init_t;

typedef struct mpg_dec_input_t
{
	int				m_iSeekTimeMSec;		//!< Seek Time(second) from Current Time
	int				m_iSeekMode;			//!< Seek mode(Reset, Forward, Backward)
	int				m_iSeekComponent;		//!<Video only, Audio only, Both of them

	int				m_iGetStreamComponent;	//!< This field is used in Interleaved mode.(Select Video or Audio)
	int				m_iInterestingCaptionIdx;	//!< Through this field, user can choose caption.(Alternation is available)
} mpg_dec_input_t;

typedef struct mpg_caption_input_t
{
	unsigned char *			m_pData;
	unsigned int			m_iDataLength;
	unsigned int			m_bCropEnable;
} mpg_caption_input_t;

// FIXME : output
typedef struct mpg_caption_output_t
{	
	unsigned short *			m_pCaption;	//!<pointer to Caption image buffer
	unsigned int			m_iCaptionPTS;

	unsigned int			m_iWidth;		//!<width of Caption image buffer
	unsigned int			m_iHeight;		//!<height of Caption image buffer

	unsigned int			m_iOffsetX;
	unsigned int			m_iOffsetY;

	unsigned int			m_iDisplayStart;	//like PTS
	unsigned int			m_iDisplayEnd;	//like PTS
} mpg_caption_output_t;

// FIXME : output
typedef struct mpg_dec_output_t
{
	unsigned char *			m_pData;	//!< pointer to data
	int				m_iDataLength;

	int				m_iType;			//!< Data Type(Video or Audio)
	unsigned int			m_iRefTime;	//!< PTS.(sometimes this field could be '0' when there's no PTS info in packet)
	unsigned int			m_iLastSCR;
} mpg_dec_output_t;

typedef struct mpg_dec_sel_stream_t
{
	unsigned long	 m_ulSelType;		//!< [in] DMXTYPE_VIDEO | DMXTYPE_AUDIO | DMXTYPE_SUBTITLE
	unsigned long	 m_ulVideoID;		//!< [in] Video ID (if m_ulSelType has DMXTYPE_VIDEO)
	unsigned long	 m_ulAudioID;		//!< [in] Audio ID (if m_ulSelType has DMXTYPE_AUDIO)
	unsigned long	 m_ulSubtitleID;	//!< [in] Subtitle ID (if m_ulSelType has DMXTYPE_SUBTITLE)
} mpg_dec_sel_stream_t;

//! Select stream - output
typedef struct mpg_dec_sel_info_t
{
	mpg_video_info_t *	m_pstVideoInfo;	//!< [out] pointer to video information
	mpg_audio_info_t *	m_pstAudioInfo;	//!< [out] pointer to audio information
} mpg_dec_sel_info_t;

/************************************************************************

	Common defines

************************************************************************/

/* Seek Mode */
#define MPG_SEEK_RESET			0x00000000
#define MPG_SEEK_DIR_FWD			0x00000001
#define MPG_SEEK_DIR_BWD			0x00000002

#define MPG_SEEK_BASE_BOTH	              	0x00000000
#define MPG_SEEK_BASE_VIDEO              	0x00000010
#define MPG_SEEK_BASE_AUDIO		0x00000020

#define MPG_SEEK_CHNK_DIR_FWD		0x00000000
#define MPG_SEEK_CHNK_DIR_BWD		0x00000100

#define MPG_SEEK_MODE_RELATIVE		0x00000000
#define MPG_SEEK_MODE_ABSOLUTE		0x00001000

//Component Set
#define MPG_PACKET_NONE			0
#define MPG_PACKET_VIDEO			1
#define MPG_PACKET_AUDIO			2
#define MPG_PACKET_SUBTITLE			4
#define MPG_PACKET_PCI			5
#define MPG_PACKET_DSI			6
#define MPG_PACKET_NAVI			7

//Error Code
#define MPG_ERR_NONE			0
#define MPG_ERR_END_OF_FILE		-1
#define MPG_ERR_OUT_OF_MEMORY		-2
#define MPG_ERR_INVALID_DATA		-3
#define MPG_ERR_INVALID_PARAM		-4
#define MPG_ERR_NOT_SUPPORTED		-5
#define MPG_ERR_NEED_MORE_DATA		-6
#define MPG_ERR_BUFFER_FULL		-7

#define MPG_ERR_FILE_NOT_FOUND		-8
#define MPG_ERR_DEVICE_ERROR		-10
#define MPG_ERR_SYNCED			-11
#define MPG_ERR_DATA_NOT_FOUND		-12
#define MPG_ERR_MIME_NOT_FOUND		-13
#define MPG_ERR_NOT_DIRECTORY		-14
#define MPG_ERR_NOT_COMPATIBLE		-15
#define MPG_ERR_CONNECT_FAILED		-16
#define MPG_ERR_DROPPING			-17
#define MPG_ERR_STOPPED			-18
#define MPG_ERR_UNAUTHORIZED		-19
#define MPG_ERR_LOADING_HEADER		-20

#define MPG_ERR_STREAM_DOES_NOT_EXIST	-21

#define MPG_ERR_FILE_OPEN			-30
#define MPG_ERR_FILE_WRITE			-31
#define MPG_ERR_NEED_CHCAE_MEM		-32
#define MPG_ERR_FILE_SEEK			-33
#define MPG_ERR_FILE_READ			-34

/*!
 ***********************************************************************
 * \brief
 *		TCC_MP4_DMX		: main api function of MPG demuxer
 * \param
 *		[in]Op			: demuxer operation 
 * \param
 *		[in,out]pHandle	: handle of MPG demuxer
 * \param
 *		[in]pParam1		: input parameter(Refer to Test Code)
 * \param
 *		[in]pParam2		: extension parameter(Refer to Test Code)
 * \return
 *		If successful, TCC_MPG_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */


mpg_result_t 
TCC_MPG_DEC(int Op, mpg_handle_t* pHandle, void* pParam1, void* pParam2);

#endif/*_MPG_H_*/
