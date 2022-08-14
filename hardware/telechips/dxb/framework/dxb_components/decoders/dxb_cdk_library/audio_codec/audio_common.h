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
 
#ifndef _AUDIO_COMMON_H_
#define _AUDIO_COMMON_H_

typedef signed char 		TCAS_S8;
typedef unsigned char 		TCAS_U8;
typedef signed short int 	TCAS_S16;
typedef unsigned short int 	TCAS_U16;
typedef signed int 			TCAS_S32;
typedef unsigned int 		TCAS_U32;

#if defined (__GNUC__)
typedef signed long long 	TCAS_S64;
typedef unsigned long long 	TCAS_U64;
#else
typedef signed __int64 		TCAS_S64;
typedef unsigned __int64 	TCAS_U64;
#endif

typedef float 				TCAS_F32;
typedef double 				TCAS_F64;

typedef void 				TCASVoid;

//! Enumeration type for Sampling Frequency 
typedef enum
{
	TCAS_SR_6000	= 6000,
	TCAS_SR_8000	= 8000,
	TCAS_SR_11025	= 11025,
	TCAS_SR_12000	= 12000,
	TCAS_SR_16000	= 16000,
	TCAS_SR_22050	= 22050,
	TCAS_SR_24000	= 24000,
	TCAS_SR_32000	= 32000,
	TCAS_SR_44100	= 44100,
	TCAS_SR_48000	= 48000,
	TCAS_SR_64000	= 64000,
	TCAS_SR_88200	= 88200,
	TCAS_SR_96000	= 96000,
} enum_samplerate_t;

//! Enumeration type for Channel information 
typedef enum
{
	TCAS_CH_UNKNOWN	= 0x000,
	TCAS_CH_MONO	= 0x001,
	TCAS_CH_STEREO	= 0x002,
	TCAS_CH_2F1R	= 0x004,
	TCAS_CH_2F2R	= 0x006,
	TCAS_CH_3F		= 0x003,
	TCAS_CH_3F1R	= 0x005,
	TCAS_CH_3F2R	= 0x007,
	TCAS_CH_LFE		= 0x100,
	TCAS_CH_DUAL	= 0x010,
	TCAS_CH_XX		= 0x7fffffff
} enum_channel_type_t;

//! Enumeration type for Flag
typedef enum
{
	TCAS_DISABLE	= 0x000,
	TCAS_ENABLE		= 0x001,
	TCAS_UNKNOWN	= 0x7fffffff
} enum_function_switch_t;

#define TCAS_MAX_CHANNEL	8	/* up to 7.1 channel */

#define In
#define Out
#define InOut

#define AUDIO_INIT			0
#define	AUDIO_DECODE      	1
#define	AUDIO_ENCODE     	2
#define AUDIO_CLOSE			3
#define AUDIO_FLUSH			4

#define CH_LEFT_FRONT		0
#define CH_CENTER			1
#define CH_RIGHT_FRONT		2
#define CH_LEFT_REAR		3
#define CH_RIGHT_REAR		4
#define CH_LEFT_SIDE		5
#define CH_RIGHT_SIDE		6
#define	CH_LFE				7

//! All information of PCM (input/output)
typedef struct audio_pcminfo_t
{
	TCASVoid* m_pvChannel[TCAS_MAX_CHANNEL];		//!< This pointer array holds the address of each PCM data channel
	TCAS_S32 m_iNchannelOffsets[TCAS_MAX_CHANNEL]; 	//!< offset value of each channel from pChannel when interleaving
	TCAS_S32 m_iNchannelOrder[TCAS_MAX_CHANNEL];	//!< channel order of each channel, ( L,C,R,Lr,Rr,Ls,Rs,LFE )
	
	enum_samplerate_t m_eSampleRate;				//!< Sampling Frequency
	enum_channel_type_t m_eChannelType;				//!< channel type containing speaker configuration
	enum_function_switch_t	m_ePcmInterLeaved;		//!< This flag indicates that PCM data are interleaved or not, 
	TCAS_U32 m_uiNumberOfChannel;					//!< Total number of channel
	TCAS_U32 m_uiSamplesPerChannel;					//!< Samples per channel
	TCAS_U32 m_uiBitsPerSample;						//!< Bits/sample
	
	TCASVoid* m_pvExtraInfo;						//!< The pointer for extra information data
} audio_pcminfo_t;

//! Data structure for encoded stream
typedef struct audio_streaminfo_t
{
	TCAS_S8* m_pcStream;							//!< This pointer holds the address of encoded stream buffer
	TCAS_S32 m_iStreamLength;						//!< Valid size of pStream (bytes)
	
	enum_samplerate_t m_eSampleRate;				//!< Sampling Frequency
	TCAS_U32 m_uiNumberOfChannel;					//!< Number of channel
	TCAS_U32 m_uiBitRates;							//!< Bit rata of this stream,  ex)128000 
	TCAS_U32 m_uiSamplesPerChannel;					//!< Number of samples per channel, frame length
	
	TCASVoid* m_pvExtraInfo;						//!< The pointer for extra information data
} audio_streaminfo_t;

//! Enumeration type for Error code
typedef enum
{
	TCAS_SUCCESS									=  0,
	TCAS_ERROR_NULL_INSTANCE						= -1,
	TCAS_ERROR_INVALID_OPINFO						= -2,
	TCAS_ERROR_NOT_SUPPORT_FORMAT					= -3,
	TCAS_ERROR_INVALID_BUFSTATE						= -4,
	TCAS_ERROR_INVALID_DATA							= -5,
	TCAS_ERROR_NULL_PCMBUFF							= -6,
	TCAS_ERROR_DECODE								= -7,
	TCAS_ERROR_ENCODE								= -8,
	TCAS_ERROR_SEEK_FAIL							= -9,
	TCAS_ERROR_INVALID_POS							= -10,
	TCAS_ERROR_OPEN_FAIL							= -11,
	TCAS_ERROR_CLOSE_FAIL							= -12,
	TCAS_ERROR_MORE_DATA							= -13,
	TCAS_ERROR_SKIP_FRAME							= -14,
	TCAS_ERROR_CONCEALMENT_APPLIED					= -15,
	//! Codec specified error code is defined here. 
	//! ex)1xx = 100, 121,...
	TCAS_ERROR_CODEC_SPECIFY						= -100,
	TCAS_ERROR_DDP_NOT_SUPPORT_FRAME              	= -101,
	
	TCAS_ERROR_XX									= 0x7fffffff,
} TCAS_ERROR_TYPE;

#endif /* _AUDIO_COMMON_H_ */
