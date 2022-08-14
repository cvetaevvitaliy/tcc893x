/****************************************************************************
 *   FileName    : tcc_wav_mux.h
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
#ifndef _TCC_WAV_MUXER_H_
#define _TCC_WAV_MUXER_H_

#include "../common.h"

#define TCC_WAV_MAX_PCM_LENGTH				4096

#define WAV_LPCM_HEADER_SIZE				44
#define WAV_MSADPCM_HEADER_SIZE				90
#define WAV_A_MU_LAW_HEADER_SIZE			46

#pragma pack(1)

typedef struct lpcm_wav_header_t
{
	//"RIFF" chunk descriptor
	char			m_cChunkId[4];		//"RIFF"
	unsigned int	m_uiChunkSize;
	char			m_cFormat[4];		//"WAVE"
	
	//"fmt " sub-chunk
	char            m_cFmtChunkID[4];	//"fmt "
	unsigned int    m_uiFmtChunkSize;
	unsigned short  m_usAudioFormat;
	unsigned short  m_usNumChannels;
	unsigned int	m_uiSampleRate;
	unsigned int	m_uiByteRate;
	unsigned short  m_usBlockAlign;
    unsigned short  m_usBitsPerSample;

	//"data" sub-chunk
	char			m_cDataChunkId[4];	//"data"
	unsigned int	m_uiDataChunkSize;
	//data

} lpcm_wav_header_t;

typedef struct ms_adpcm_coefset_t
{
    short m_sCoef1;
    short m_sCoef2;
} ms_adpcm_coefset_t;

typedef struct ms_adpcm_wav_header_t
{
	//"RIFF" chunk descriptor
	char			m_cChunkId[4];		//"RIFF"
	unsigned int	m_uiChunkSize;
	char			m_cFormat[4];		//"WAVE"
	
	//"fmt " sub-chunk
	char            m_cFmtChunkID[4];	//"fmt "
	unsigned int    m_uiFmtChunkSize;

	unsigned short  m_usAudioFormat;
	unsigned short  m_usNumChannels;
	unsigned int	m_uiSampleRate;
	unsigned int	m_uiAvgBytesPerSec;
	unsigned short  m_usBlockAlign;
	unsigned short  m_usBitsPerSample;

	unsigned short  m_usCbSize;
	unsigned short  m_usSamplesPerBlock;
	unsigned short  m_usNumCoef;  //42
	
	ms_adpcm_coefset_t	m_sCoef[7]; //28

	//"fact" sub-chunk
	char            m_cFactChunkID[4];	//"fact"
	unsigned int    m_uiFactChunkSize;
	unsigned int    m_uiNumberOfSamples;

	//"data" sub-chunk
	char			m_cDataChunkId[4];	//"data"
	unsigned int	m_uiDataChunkSize;
	//data

} ms_adpcm_wav_header_t;

typedef struct mu_a_law_wav_header_t
{
	//"RIFF" chunk descriptor
	char			m_cChunkId[4];		//"RIFF"
	unsigned int	m_uiChunkSize;
	char			m_cFormat[4];		//"WAVE"
	
	//"fmt " sub-chunk
	char            m_cFmtChunkID[4];	//"fmt "
	unsigned int    m_uiFmtChunkSize;

	unsigned short  m_usAudioFormat;
	unsigned short  m_usNumChannels;
	unsigned int	m_uiSampleRate;
	unsigned int	m_uiAvgBytesPerSec;
	unsigned short  m_usBlockAlign;
	unsigned short  m_usBitsPerSample;
	unsigned short  m_usCbSize;

	//"data" sub-chunk
	char			m_cDataChunkId[4];	//"data"
	unsigned int	m_uiDataChunkSize;
	//data

} mu_a_law_wav_header_t;

#pragma pack()

typedef struct wav_enc_init_extra_info_t //tcc_wav_enc extrainfo
{
	unsigned int            m_uiWaveSamplesPerBlock;
	unsigned int            m_uiWaveBytesPerBlock;
	unsigned int			m_uiWaveByteRate;
} wav_enc_init_extra_info_t;

typedef struct wav_mux_info_t
{
	void					*m_pWavefile;

	void					*m_psWaveHdr;

	unsigned int			m_uiWaveFormatId;
	unsigned int			m_uiWaveSamplerate;
	unsigned int			m_uiWaveChannels;
	unsigned int			m_uiWaveBitsPerSample;
	unsigned int			m_uiWaveDataSize;
	unsigned int            m_uiWaveBytesPerBlock;
	unsigned int            m_uiWaveSamplesPerBlock;
	unsigned int			m_uiWaveByteRate;

	unsigned int			m_uiNumberOfSample;
	unsigned int			m_uiInitOk;
	//unsigned int			m_uiFrameCount;

	wav_enc_init_extra_info_t		m_sEncSetInfo;

} wav_mux_info_t;

/*!
 ***********************************************************************
 * \brief
 *		cmux_wav		: main api function of cmux_wav
 * \param
 *		[in]Op			: muxer operation 
 * \param
 *		[in,out]pHandle	: cmux_wav handle
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, cmux_wav returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
common_result_t 
cmux_wav( int Op, common_handle_t* pWavHandle, void* pParam1, void* pParam2 );

#endif//_TCC_WAV_MUXER_H_
