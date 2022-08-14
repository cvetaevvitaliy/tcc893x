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
   																				
#ifndef _AV_COMMON_DMX_H_
#define _AV_COMMON_DMX_H_

#include "av_common_fcc_tag.h"
#include "av_common_types.h"

typedef av_handle_t	av_dmx_handle_t; //!< The type of a demuxer's handle
typedef av_result_t	av_dmx_result_t; //!< The type of a demuxer's result

/*
 ============================================================
 *
 *	Constant Definition
 *
 ============================================================
*/

// Demuxer OP Code
#define DMXOP_OPEN					 0 //!< demuxer open (initialize)
#define DMXOP_CLOSE					 1 //!< demuxer close
#define DMXOP_GET_STREAM			 2 //!< get stream
#define DMXOP_SEEK					 3 //!< seek stream
#define DMXOP_SELECT_STREAM			 4 //!< select stream(for multistream)
#define DMXOP_GET_LASTERROR			 5 //!< get last error
#define DMXOP_GET_METADATA			 6 //!< get meta data

// Demuxer Return Value
// 0 ~ 999

#define DMXRET_SUCCESS				 0 //!< success(0 or plus)
#define DMXRET_FAILED				-1 //!< general failed code
#define DMXRET_END_OF_STREAM		-2 //!< the end of stream 
#define DMXRET_INVALID_HANDLE		-3 //!< invalid handle
#define DMXRET_FILE_OPEN_FAILED		-4 //!< open file failure

// Stream Type

#define DMXTYPE_NONE				 0 //!< get auto selected stream
#define DMXTYPE_VIDEO				 1 //!< get video stream
#define DMXTYPE_AUDIO				 2 //!< get audio stream
#define DMXTYPE_SUBTITLE			 4 //!< get subtitle stream
#define DMXTYPE_GRAPHICS			 8 //!< get graphics stream

// Metadata Structure
#define META_ID_DEFAULT				 0 //!< user defined ID
#define	META_ID_COPYRIGHT			 1 //!< CopyRight [string]
#define META_ID_CREATION_TIME		 2 //!< Creation Time [string]
#define META_ID_PROFILE_LEVEL		 3 //!< Profile Level [string]
#define META_ID_CODEC_NAME			 4 //!< Codec Name [string]
#define META_ID_MANUFACTURER		 5 //!< Manufacturer [string]
#define META_ID_LANGUAGE			 6 //!< Language [string]
#define META_ID_ARTIST				 7 //!< Artist [string]
#define META_ID_ALBUM_TITLE			 8 //!< Album Title [string]
#define META_ID_VENDOR_NAME			 9 //!< Vendor Name [string]
#define META_ID_VKEY_TOTAL			10 //!< Total Key frames 
#define META_ID_VKEY_LAST_TIMESTAMP	11 //!< Last Key frame timestamp
#define META_ID_DISPLAY_WIDTH		12 //!< Display width
#define META_ID_DISPLAY_HEIGHT		13 //!< Display height

#define META_TYPE_SIGNED_LONG		 1
#define META_TYPE_UNSIGNED_LONG		 2
#define META_TYPE_FLOAT				 3
#define META_TYPE_STRING			 4
#define META_TYPE_DATA				 5

typedef struct av_metadata_t
{
	unsigned long	m_ulDataID;
	unsigned long	m_ulDataType;
	unsigned long	m_ulDataLength;	//!< [out] for META_TYPE_STRING or META_TYPE_DATA

	union {
		long			lValue;		//!< [out] META_TYPE_SIGNED
		unsigned long	ulValue;	//!< [out] META_TYPE_UNSIGNED
		float			fValue;		//!< [out] META_TYPE_FLOAT
		char		   *pszString;	//!< [out] META_TYPE_STRING
		unsigned char  *pbyData;	//!< [out] META_TYPE_DATA
	} m_unData;
} av_metadata_t;


/*
 ============================================================
 *
 *	Demuxer Operation Usage and Structure Definitions
 *
 ============================================================
*/

/*
 ============================================================
 *	Operation: Demuxer Open
 ============================================================
 *	PARAMS:
 *		- opCode:	DMXOP_OPEN
 *		- handle:	(av_dmx_handle_t*)pHandle
 *		- param1:	<demuxer specific init. structure>in (xxx_dmx_init_t*)
 *		- param2:	<demuxer specific info. structure>out (xxx_dmx_info_t*)
 *	RETURNS:
 *		- DMXRET_SUCCESS / DMXRET_FAILED
 ============================================================
*/

/*
 ============================================================
 *	Operation: Demuxer Close
 ============================================================
 *	PARAMS:
 *		- opCode:	DMXOP_CLOSE
 *		- handle:	(av_dmx_handle_t*)pHandle
 *		- param1:	NULL
 *		- param2:	NULL
 *	RETURNS:
 *		- DMXRET_SUCCESS / DMXRET_FAILED
 ============================================================
 */


/*============================================================
 *	Operation: Get Stream
 =============================================================
 *	PARAMS:
 *		- opCode:	DMXOP_GET_STREAM
 *		- handle:	(av_dmx_handle_t*)pHandle
 *		- param1:	(av_dmx_getstream_t*)in
 *		- param2:	(av_dmx_outstream_t*)out
 *	RETURNS:
 *		- DMXRET_SUCCESS / DMXRET_FAILED / DMXERR_END_OF_STREAM
 ============================================================*/

//! Get stream structure
#if 0
typedef struct av_dmx_getstream_t
{
	unsigned char  *m_pbyStreamBuff;		//!< [in] the pointer to getstream buffer
	unsigned long	m_ulStreamBuffSize;		//!< [in] the size of getstream buffer
	unsigned long	m_ulStreamType;			//!< [in] the type of getstream
	void		   *m_pSpecificData;		//!< [in] the pointer to specific input of demuxer(if demuxer has a specific data)
} av_dmx_getstream_t;

typedef struct av_dmx_outstream_t
{
	unsigned char  *m_pbyStreamData;		//!< [out] the pointer to outstream data
	unsigned long	m_ulStreamDataSize;		//!< [out] the size to outstream data
	unsigned long	m_ulStreamType;			//!< [out] the type of outstream
	unsigned long	m_ulTimeStamp;			//!< [out] the timestamp of outstream
	unsigned long	m_ulEndTimeStamp;		//!< [out] the end timestamp of outstream: This is not mandatory except in the case of text-subtitle
	void		   *m_pSpecificData;		//!< [out] the pointer to specific output of demuxer(if demuxer has a specific data)
} av_dmx_outstream_t;
#else
typedef struct av_dmx_getstream_t
{
	unsigned char  *m_pbyStreamBuff;		//!< [in] the pointer to getstream buffer
	long			m_lStreamBuffSize;		//!< [in] the size of getstream buffer
	long			m_lStreamType;			//!< [in] the type of getstream
	void		   *m_pSpecificData;		//!< [in] the pointer to specific input of demuxer(if demuxer has a specific data)
} av_dmx_getstream_t;

typedef struct av_dmx_outstream_t
{
	unsigned char  *m_pbyStreamData;		//!< [out] the pointer to outstream data
	long			m_lStreamDataSize;		//!< [out] the size to outstream data
	long			m_lStreamType;			//!< [out] the type of outstream
	long			m_lTimeStamp;			//!< [out] the timestamp of outstream
	long			m_lEndTimeStamp;		//!< [out] the end timestamp of outstream: This is not mandatory except in the case of text-subtitle
	void		   *m_pSpecificData;		//!< [out] the pointer to specific output of demuxer(if demuxer has a specific data)
} av_dmx_outstream_t;
#endif


/*
 ============================================================
 *	Operation: Select Stream
 ============================================================
 *	PARAMS:
 *		- opCode:	DMXOP_SELECT_CHANNEL
 *		- handle:	(av_dmx_handle_t*)pHandle
 *		- param1:	<demuxer specific select stream structure>in (xxx_dmx_sel_stream_t*)
 *		- param2:	<demuxer specific select info. structure>out (xxx_dmx_sel_info_t*)
 *	RETURNS:
 *		- DMXRET_SUCCESS / DMXRET_FAILED / DMXERR_END_OF_STREAM
 ============================================================
*/



/*
 ============================================================
 *	Operation: Frame Seek
 ============================================================
 *	PARAMS:
 *		- opCode:	DMXOP_SEEK
 *		- handle:	(DMXHANDLE_t*)pHandle
 *		- param1:	(av_dmx_seek_t*)in
 *		- param2:	(av_dmx_seek_t*)out
 *	RETURNS:
 *		- DMXRET_SUCCESS / DMXRET_FAILED / DMXERR_END_OF_STREAM
 ============================================================
*/

/*
 *-----------------------------------------------------------
 *	Seek Mode Flags (av_dmx_seek_t.m_ulSeekMode)
 *-----------------------------------------------------------
*/

//! default seek mode : it is used absolute_time, based-on video_timestamp
#define DMXSEEK_DEFAULT				0x0000	
//! seek options
#define DMXSEEK_RELATIVE_TIME		0x0010


typedef struct av_dmx_seek_t
{
	long			m_lSeekTime;	//!< millisecond unit
	unsigned long	m_ulSeekMode;	//!< mode flags
} av_dmx_seek_t;


/*!
 ============================================================
 *	Operation: Get Last Error
 ============================================================
 *	PARAMS:
 *		- opCode:	DMXOP_GET_LASTERROR
 *		- handle:	(av_dmx_handle_t*)pHandle
 *		- param1:	NULL
 *		- param2:	NULL
 *	RETURNS:
 *		- <Demuxer Specific Last Error Code>
 ============================================================
 */


#endif//_AV_COMMON_DMX_H_
