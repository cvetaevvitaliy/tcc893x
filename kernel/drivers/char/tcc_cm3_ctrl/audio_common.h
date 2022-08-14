/****************************************************************************
One line to give the program's name and a brief idea of what it does.
Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
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

//! All informaiton of PCM (input/output)
typedef struct audio_pcminfo_t
{
	TCASVoid* m_pvChannel[TCAS_MAX_CHANNEL];		//!< pointers addressing each channel PCM data
	TCAS_S32 m_iNchannelOffsets[TCAS_MAX_CHANNEL]; 	//!< offset value of each channel from pChannel when interleaving
	TCAS_S32 m_iNchannelOrder[TCAS_MAX_CHANNEL];	//!< channel order of each channel, ( L,C,R,Lr,Rr,Ls,Rs,LFE )
	
	enum_samplerate_t m_eSampleRate;				//!< Sampling Frequncy
	enum_channel_type_t m_eChannelType;				//!< channel type containing speacker configuration
	enum_function_switch_t	m_ePcmInterLeaved;		//!< flag for PCM interleaving
	TCAS_U32 m_uiNumberOfChannel;					//!< Total number of channel
	TCAS_U32 m_uiSamplesPerChannel;					//!< number of sample per channel
	TCAS_U32 m_uiBitsPerSample;						//!< bit resolution of a sample
	
	TCASVoid* m_pvExtraInfo;						//!< pointer for extra data
} audio_pcminfo_t;

//! data structure for encoded stream
typedef struct audio_streaminfo_t
{
	TCAS_S8* m_pcStream;							//!< pointer holding address of encoded data
	TCAS_S32 m_iStreamLength;						//!< valid size of pStream
	
	enum_samplerate_t m_eSampleRate;				//!< sampling frequency
	TCAS_U32 m_uiNumberOfChannel;					//!< number of channel
	TCAS_U32 m_uiBitRates;							//!< bit rata of this stream,  ex)128000 
	TCAS_U32 m_uiSamplesPerChannel;					//!< number of samples per channel, frame length
	
	TCASVoid* m_pvExtraInfo;						//!< pointer for extra data
} audio_streaminfo_t;

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
	
	TCAS_ERROR_XX									= 0x7fffffff,
} TCAS_ERROR_TYPE;

#endif /* _AUDIO_COMMON_H_ */
