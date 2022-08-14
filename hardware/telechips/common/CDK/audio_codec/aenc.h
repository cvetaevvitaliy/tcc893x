/****************************************************************************
 *   FileName    : aenc.h
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
#ifndef _AENC_H_
#define _AENC_H_

#include "audio_common.h"

typedef audio_pcminfo_t aenc_input_t;
typedef audio_streaminfo_t aenc_output_t;

//! Callback Func
typedef struct aenc_callback_func_t
{
	TCASVoid* (*m_pfMalloc  ) ( TCAS_U32 );									//!< malloc
	TCASVoid  (*m_pfFree	) ( TCASVoid* );								//!< free
	TCASVoid* (*m_pfMemcpy  ) ( TCASVoid*, const TCASVoid*, TCAS_U32 );		//!< memcpy
	TCASVoid  (*m_pfMemset	) ( TCASVoid*, TCAS_S32, TCAS_U32 );			//!< memset
	TCASVoid* (*m_pfRealloc ) ( TCASVoid*, TCAS_U32 );						//!< realloc
	TCASVoid* (*m_pfMemmove ) ( TCASVoid*, const TCASVoid*, TCAS_U32 );		//!< memmove	
}aenc_callback_func_t;

//! Specific Information structure for each Audio encoding
typedef union codec_specific_info_t
{
	//! Data structure for AAC encoding
	struct 
	{
		//!   AAC Stream Header type : ( 0 : RAW-AAC, 1: ADTS, 2: ADIF)
		TCAS_S32		m_iAACHeaderType;
		//!   On/Off PNS Processing : ( 0 : off, 1: on)
		TCAS_S32		m_iAACTurnOnPns;
		//!   On/Off TNS Processing : ( 0 : off, 1: on)
		TCAS_S32		m_iAACTurnOnTns;
		//!   Select MPGE version : ( 0 : MPEG4, 1: MPEG2)
		TCAS_S32		m_iVersionInfo;

		//!	  RESERVED
		TCAS_S32		reserved[32-4];
	} m_unAAC;

	//! Data structure for EVRC encoding
	struct 
	{
		//!	  noise suppression selection ( 0: noise suppression off (default), 1: noise suppression on)
		TCAS_S32		m_iUseNoiseSuppression;
		//!   Maximum Bitrate selection : ( 1: 0.8 kbps, 3: 4.0 kbps, 4: 8.55 kbps (default) )
		TCAS_S32		m_iMaxRate;	
		//!   Minimum Bitrate selection : ( 1: 0.8 kbps, 3: 4.0 kbps, 4: 8.55 kbps (default) )
		TCAS_S32		m_iMinRate;
		//!	RESERVED			
		TCAS_S32		reserved[32-3];
	} m_unEVRC;

	//! Data structure for WMA encoding	
	struct 
	{
		//!	Flag for writing ASF header information per Packet.  '0'- don't write, '1'- write ASF header per packet.
		TCAS_U32		m_uiHeaderPerPacket;
		//! CDsync structure pointer
		TCASVoid 		*m_pvCdsync;
		//!	RESERVED			
		TCAS_S32		reserved[32-2];

	}m_unWMA;

	//! Data structure for MP3 encoding
	struct 
	{
		//! Indicates the MPEG Version . '0'-MPEG2 '1'-MPEG1     
		TCAS_U32		m_uiVersion;
		//! Indicates the bitrate index according to the ISO/IEC 11172-3.    
		TCAS_U32		m_uiBitrateIndex;

		//! Indicates the sampling frequency index. '0' - 44100 / 22050, '1'- 48000/24000, '2'- 32000/16000 Hz     
		TCAS_U32		m_uiSamplingFreqIndex;
		//!	Indicates the number of channel(s). '1'- 1 channel, '2'-channels.	
		TCAS_S32		m_iChannelMode;
		
		//! The flag for cutting frequency components off.
		TCAS_U32		m_uiClipEnable;			

		//! Not Used		
		TCAS_U32		m_uiFirstFrameFlag;			
		//! Indicates the stereo mode according to the ISO/IEC 11172-3.    
		TCAS_U32		m_uiStereoMode;			

		//! Not used
		TCAS_U32		m_uiSamplesPerFrame;			

		//! This variable represents bitrate. Refer to the ISO/IEC 11172-3 Bitrate Table.
		TCAS_S32		m_iBitrate;			

		//! Sampling frequency.  Ex) 44100, 48000, 32000
		TCAS_S32		m_iSamplingFreq;			

		//! Indicates the frequency limits. Adjust this value from test.( Do not exceed 80)
		TCAS_S32		m_iCutoffFreq;			

		//! Indicates the written size (bytes) in the encoded bitstream buffer.
		TCAS_U32		m_iBuffWriteIndex;			
		
		//! Encoded bitstream Buffer pointer
		TCAS_U8			*m_pucBitStreamBuff;		
		
		//! CDsync structure pointer
		TCASVoid 		*m_pvCdsync;

		//! RESERVED
		TCAS_S32		reserved[32-14];
	} m_unMP3ENC;

	//! Data structure for WAV encoding
	struct 
	{
		//!   wav format type : ( 1 : pcm, 2: ms-adpcm, 6: a-law, 7: mu-law, 11: ima-adpcm )
		TCAS_S32		m_iWaveFormat;
		//!	  RESERVED
		TCAS_S32		reserved[32-1];
	} m_unWAV;

	//! Data structure for QCELP encoding
	struct 
	{
		//!   QCELP encoding mode selection support ( 4 : full rate, 3 : half rate  )
		TCAS_S32		m_iMode;
		//!	  RESERVED
		TCAS_S32		reserved[32-1];
	} m_unQCELP;
} codec_specific_info_t;
	
//! Data structure for Audio encoder initializing on CDK platform.
typedef struct aenc_init_t
{

	//! Audio encoder common information structure
	struct 
	{
		aenc_input_t	*m_psAudioEncInput;		//!< Input stream information 
		aenc_output_t	*m_psAudioEncOutput;	//!< Output stream information 

		TCAS_U8			*m_pucExtraData;		//!< Extra data for encoding
		TCAS_S32		m_iExtraDataLen;		//!< Extra data length ( bytes )
		
		TCAS_S8			*m_pcOpenFileName;		//!< Encoded stream file name.

		TCAS_S32		m_iMaxStreamLength;		//!< Encoded stream buffer size (bytes)
		TCAS_S32		m_iPcmLength;			//!< PCM data buffer size (samples * ch * sizeof(short))

		//! Callback Func
		aenc_callback_func_t *m_psAudioCallback;

		//! RESERVED
		TCAS_S32		m_iReserved[32-8];
	} m_sCommonInfo;
	
	codec_specific_info_t m_unAudioEncodeParams;	//!< Specific information structure

} aenc_init_t;

//! Data structure for cdsync_info block
typedef struct cdsync_info
{
	TCAS_U32	CDsyncLevel;					//!< The reference power level.
	TCAS_U32	CDsyncTime;						//!< Reference silence time.
	TCAS_U32 	CDsyncSilenceCnt;				//!< The counter of silence frame. 
	TCAS_U32	CDsyncEnable;					//!< Enable CDsync Block. '0' - disable, '1'- enable
	TCAS_U32	CDsyncDetect;					//!< Indicates the CDSync status.  '0' - non-sync status, '1' - sync status
												
	TCAS_U32 	SoundLevel;						//!< Internally used.

} TCAS_CDSyncInfo;

/*!
 ***********************************************************************
 * \brief
 *		TCC_AAC_ENC		: main api function of aac encoder
 * \param
 *		[in]Op			: encoder operation 
 * \param
 *		[in,out]pHandle	: handle of aac encoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_AAC_ENC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
TCAS_S32 TCC_AAC_ENC( TCAS_S32 iOpCode, TCAS_S32* pHandle, TCASVoid* pParam1, TCASVoid* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_EVRC_ENC	: main api function of evrc encoder
 * \param
 *		[in]Op			: encoder operation 
 * \param
 *		[in,out]pHandle	: handle of evrc encoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_EVRC_ENC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
TCAS_S32 TCC_EVRC_ENC( TCAS_S32 iOpCode, TCAS_S32* pHandle, TCASVoid* pParam1, TCASVoid* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_WMA_ENC	: main api function of wma encoder
 * \param
 *		[in]Op			: encoder operation 
 * \param
 *		[in,out]pHandle	: handle of wma encoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_WMA_ENC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
TCAS_S32 TCC_WMA_ENC( TCAS_S32 iOpCode, TCAS_S32* pHandle, TCASVoid* pParam1, TCASVoid* pParam2 );


/*!
 ***********************************************************************
 * \brief
 *		TCC_MP3_ENC	: main api function of mp3 encoder
 * \param
 *		[in]Op			: encoder operation 
 * \param
 *		[in,out]pHandle	: handle of mp3 encoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_MP3_ENC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
TCAS_S32 TCC_MP3_ENC( TCAS_S32 iOpCode, TCAS_S32* pHandle, TCASVoid* pParam1, TCASVoid* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_WAV_ENC	: main api function of wav encoder
 * \param
 *		[in]Op			: encoder operation 
 * \param
 *		[in,out]pHandle	: handle of wav encoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_WAV_ENC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
TCAS_S32 TCC_WAV_ENC( TCAS_S32 iOpCode, TCAS_S32* pHandle, TCASVoid* pParam1, TCASVoid* pParam2 );


/*!
 ***********************************************************************
 * \brief
 *		TCC_QCELP_ENC	: main api function of qcelp encoder
 * \param
 *		[in]Op			: encoder operation 
 * \param
 *		[in,out]pHandle	: handle of qcelp encoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_QCELP_ENC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */

TCAS_S32 TCC_QCELP_ENC( TCAS_S32 iOpCode, TCAS_S32* pHandle, TCASVoid* pParam1, TCASVoid* pParam2 );
#endif //_AENC_H_
