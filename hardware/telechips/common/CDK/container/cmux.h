/****************************************************************************
 *   FileName    : cmux.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¡°AS IS¡± and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information¡¯s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#ifndef _CMUX_H_
#define _CMUX_H_


#include "../cdk/cdk_pre_define.h"
#include "../cdk/cdk_port.h"
#ifdef INCLUDE_MP4_MUX
	#include "mp4/mp4_muxer.h"
#endif
	#include "audio/tcc_wav_mux.h"

#define CMUX_INIT			0
#define CMUX_CLOSE			1
#define CMUX_PUT_STREAM		2

typedef struct cmux_init_t
{
	void*	m_pCallbackFunc;	//!< callback function ptr
	void*	m_pszOutFileName;	//!< output file name
	int		m_iStreamType;		//!< ( PACKET_VIDEO | PACKET_AUDIO )
} cmux_init_t;

typedef struct cmux_input_t
{
	unsigned char* m_pData;		//!< [in] the pointer to the packet data
	int m_iDataSize;			//!< [in] the size of the packet data
	int m_Dts;					//!< [in] decoding timestamp(msec)
	int m_Pts;					//!< [in] presentation timestamp(msec)
	int m_iFlags;				//!< [in] the flag of the key frame
	int m_iType;				//!< [in] the type of the packet data
	int64_t m_lEstiFileSize;		//!< [out] the size of the estimated whole mp4 file ( when a file is closed after current muxing )
	int m_Reserved[12-8];
} cmux_input_t;


struct cmux_creation_time_t
{
	int m_iYear;			//! year
	int m_iMonth;			//! months since January	- [0,11]
	int m_iDayOfMonth;		//! day of the month		- [1,31]
	int m_iDayOfWeek;		//! days since Sunday		- [0, 6]
	int m_iHour;			//! hours since midnight	- [0,23]
	int m_iMin;				//! minutes after the hour	- [0,59]
	int m_iSec;				//! seconds after the minute- [0,59]
};

typedef struct cmux_info_t
{
	//! file information : 128 Bytes
	struct 
	{
		struct cmux_creation_time_t m_CTime; //! creation time
		int m_Reserved[32-7];		
	} m_sFileInfo;

	//! video information : 128 Bytes
	struct 
	{
		int m_iWidth;					//!< 1.width
		int m_iHeight;					//!< 2.height
		int m_iFrameRate;				//!< 3.framerate * 1000;
		int m_iFourCC;					//!< 4.fourcc
		unsigned char* m_pExtraData;	//!< 5.extra data
		int m_iExtraDataLen;			//!< 6.extra data length
		int m_bAvcC;					//!< 7.avcC flag for H264 in mp4
		int m_Reserved[32-7];
	} m_sVideoInfo;

	//! audio information : 128 Bytes
	struct 
	{
		int m_iSamplePerSec;			//!< 1.samples/sec
		int m_iBitsPerSample;			//!< 2.bits/sample
		int m_iChannels;				//!< 3.channels
		int m_iFormatId;				//!< 4.format id
		unsigned char* m_pExtraData;	//!< 5.extra data
		int m_iExtraDataLen;			//!< 6.extra data length
		int m_Reserved[32-6];
	} m_sAudioInfo;

} cmux_info_t;

#ifdef INCLUDE_MKV_DMX
int cmux_mp4( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );
#endif
int cmux_raw( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );


#endif //_CMUX_H_
