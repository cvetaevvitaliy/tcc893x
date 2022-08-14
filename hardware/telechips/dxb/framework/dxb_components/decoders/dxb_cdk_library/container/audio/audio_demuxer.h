/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/

#ifndef __AUDIO_DEMUXER_H__
#define __AUDIO_DEMUXER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../common.h"

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Definitions
//
typedef int audio_handle_t;

// opCode
#define AUDIO_DMX_OPEN			0
#define AUDIO_DMX_GETINFO		1
#define AUDIO_DMX_GETFRAME		3
#define AUDIO_DMX_CLOSE			4
#define AUDIO_DMX_SEEK			5

// Return value
#define ERR_AUDIO_DMX_NONE				 0
#define ERR_AUDIO_DMX_END_OF_FILE		-1

#define CONTAINER_TYPE_AUDIO_AAC	0
#define CONTAINER_TYPE_AUDIO_MP3	1
#define CONTAINER_TYPE_AUDIO_MP2	2
#define CONTAINER_TYPE_AUDIO_VORBIS	3
#define CONTAINER_TYPE_AUDIO_FLAC	4
#define CONTAINER_TYPE_AUDIO_APE	5
#define CONTAINER_TYPE_AUDIO_WAV	6
#define CONTAINER_TYPE_AUDIO_MP3HD	7

#define SEEK_RELATIVE	0
#define SEEK_ABSOLUTE	1

#ifndef NULL
#define NULL 0
#endif

/*
============================================
  Demuxer Initialize (AVIOP_OPEN) : Input
============================================
*/

//! common callback function
typedef struct audio_callback_func_t
{
	void* (*m_pMalloc		 ) ( unsigned int );					//!< malloc
	void* (*m_pNonCacheMalloc) ( unsigned int );					//!< non-cacheable malloc 
	void  (*m_pFree			 ) ( void* );							//!< free
	void  (*m_pNonCacheFree	 ) ( void* );							//!< non-cacheable free
	void* (*m_pMemcpy		 ) ( void*, const void*, unsigned int );//!< memcpy
	void* (*m_pMemset		 ) ( void*, int, unsigned int );		//!< memset
	void* (*m_pRealloc		 ) ( void*, unsigned int );				//!< realloc
	void* (*m_pMemmove		 ) ( void*, const void*, unsigned int );//!< memmove
	int   (*m_pStrncmp       ) ( const char*, const char*, unsigned int ); //!< strncmp

	void*		 (*m_pFopen	) (const char *, const char *);						//!< fopen
	unsigned int (*m_pFread	) (void*, unsigned int, unsigned int, void* );		//!< fread
	int			 (*m_pFseek	) (void*, long, int );								//!< fseek 32bit io
	long		 (*m_pFtell	) (void* );											//!< ftell 32 bit io
	unsigned int (*m_pFwrite) (const void*, unsigned int, unsigned int, void*);	//!< fwrite
	int			 (*m_pFclose) (void *);											//!< fclose
	int			 (*m_pUnlink) ( const char *);									//!< _unlink
	unsigned int (*m_pFeof  ) (void *);											//!< feof
	unsigned int (*m_pFflush) (void *);											//!< fflush

	int			 (*m_pFseek64) ( void*, i64_t, int );							//!< fseek 64bit io
	i64_t		 (*m_pFtell64) ( void* );										//!< ftell 64bit io

} audio_callback_func_t;

typedef struct audio_dmx_init_t 
{
	void*						m_pOpenFileName;	//!< Open file name
	audio_callback_func_t		m_sCallbackFunc;	//!< System callback function
	int							m_iIoBlockSize;		//!< File read cache size
	unsigned int				m_uiOption;			//!< Demuxing options
	unsigned int				m_uiDmxType;		//!< Demuxer Type
	audio_handle_t				m_iDmxHandle;		//!< Demuxer Handle
	unsigned int				m_uiId3TagOffset;
	void						*m_pvExtraInfo;
} audio_dmx_init_t;

//! common demuxer information
typedef struct audio_dmx_info_t
{
	//! file information : 128 Bytes
	struct 
	{
		/* common */
		char* m_pszOpenFileName;	//!< open file name
		int m_iFileSize;			//!< total file size
		int m_iRunningtime;			//!< runing time * 1000

		int m_Reserved[32-3];
	} m_sFileInfo;

	//! video information : 128 Bytes
	struct 
	{
		/* common */
		int m_iTotalNumber;			//!< total audio number
		int m_iSamplePerSec;		//!< samples/sec
		int m_iBitsPerSample;		//!< bits/sample
		int m_iChannels;			//!< channels
		int m_iFormatId;			//!< format id

		/* extra info (common) */
		unsigned char* m_pExtraData;//!< extra data
		int m_iExtraDataLen;		//!< extra data length
		
		int m_iMinDataSize;			//!< minimum data size

		/* aac dec */
		int m_iAacHeaderType;		//!< aac header type ( 0 : RAW-AAC, 1: ADTS, 2: ADIF)
		int m_iAacBSAC;				//!< m_iAacBSAC: ( 0 : aac, 1 : bsac )
		
		int m_iBitRate;				//!< bitrate
		int m_iBlockAlign;			//!< block align
		
		int m_Reserved[32-12];			
	} m_sAudioInfo;

} audio_dmx_info_t;

//! input parameter
typedef struct audio_dmx_input_t
{
	unsigned char* m_pPacketBuff;		//!< [in] allocated packet(video or audio) buffer pointer
	int m_iPacketBuffSize;				//!< [in] allocated packet(video or audio) buffer size
	int m_iPacketType;					//!< [in] PACKET_NONE or PACKET_VIDEO or PACKET_AUDIO
										// if m_pPacketBuffer is zero, mp4 parser's internal memory will be used.
	int m_iUsedBytes;
	int	m_Reserved[8-4];
} audio_dmx_input_t;

//! output parameter
typedef struct audio_dmx_output_t
{
	unsigned char* m_pPacketData;	//!< pointer of output data
	int m_iPacketSize;				//!< length of output data
	int m_iPacketType;				//!< packet type of output data
	int m_iTimeStamp;				//!< timestamp(msec) of output data
	int m_iEndOfFile;				//!< end of file
	int m_iDecodedSamples;			//!< number of frame in the m_iPacketSize
	unsigned int m_uiUseCodecSpecific; //!< variable use in several codec
	int m_Reserved[32-7];			//!< reserved...
} audio_dmx_output_t;

//! seek parameter
typedef struct audio_dmx_seek_t
{
	int m_iSeekTimeMSec;	//!< seek time(unit : second) from current time
	int m_iSeekMode;		//!< Seek mode : 0(default video) 1(video) 2(audio)
} audio_dmx_seek_t;

typedef enum
{	
	// LATM/LOAS (Low Overhead Audio Stream): LATM with sync information
	TF_AAC_LOAS			= 0,	// default value

	// LATM (Low-overhead MPEG-4 Audio Transport Multiplex), without LOAS Sync-Layer, No random access is possible
	TF_AAC_LATM_MCP1	= 1,	// LATM wiht muxConfigPresent = 1
	TF_AAC_LATM_MCP0	= 2,	// LATM wiht muxConfigPresent = 0

	// ADTS (Audio Data Transport Stream)
	TF_AAC_ADTS			= 3,	

	// ADIF (Audio Data Interchange Format)
	TF_AAC_ADIF			= 4,	// not supported

	TF_UNKNOWN			= 0x7FFFFFFF	// Unknown format
}TransportFormat;

int is_mpeg_audio(audio_dmx_init_t *p_dmx_init, void *pvFileHandle );
int TCC_AAC_DMX( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );
//int TCC_MP3_DMX( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );
int TCC_APE_DMX( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );
//int TCC_MP2_DMX( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );
int TCC_FLAC_DMX( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );
int TCC_WAV_DMX( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );
int TCC_MP3HD_DMX( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );
int TCC_MP23_DMX( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

void* latm_parser_init( unsigned char *pucPacketDataPtr, unsigned int piDataLength, int *piSamplingFrequency, int *piChannelNumber, void *psCallback, TransportFormat eFormat);
int latm_parser_close( void *pLatm );
int latm_parser_get_frame( void *pLatmHandle, unsigned char *pucPacketDataPtr, int iStreamLength, unsigned char **pucAACRawDataPtr, int *piAACRawDataLength, unsigned int uiInitFlag );
int latm_parser_get_header_type( void *pLatmHandle );

int check_mpeg_audio_get_layer(int iHandle, unsigned char *pPacketBuff, int iPacketBuffSize );
int check_mpeg_audio_init(void);
int check_mpeg_audio_close(int iHandle );
#ifdef __cplusplus
}
#endif
#endif //__AUDIO_DEMUXER_H__
