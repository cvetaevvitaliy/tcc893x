/****************************************************************************
 *   FileName    : adec.h
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
#ifndef _ADEC_H_
#define _ADEC_H_

#include "audio_common.h"

typedef audio_streaminfo_t adec_input_t;
typedef audio_pcminfo_t adec_output_t;

#define		TCAS_CODEC_AMRWB		0x100
#define		TCAS_CODEC_AMRWBPLUS	(TCAS_CODEC_AMRWB+1)

#define		TCAS_CODEC_NOLFEMIX			(0)
#define		TCAS_CODEC_LFEMIX			(1)

#define		TCAS_CODEC_NODOWNMIX		(0)
#define		TCAS_CODEC_CENTER			(1)
#define		TCAS_CODEC_LR				(2)
#define		TCAS_CODEC_LtRt				(3)
#define		TCAS_CODEC_CLR				(4)
#define		TCAS_CODEC_LRCs				(5)
#define		TCAS_CODEC_CLRCs			(6)
#define		TCAS_CODEC_LRLsRs			(7)
#define		TCAS_CODEC_CLRLsRs			(8)

typedef union union_audio_codec_t 
{	
	//! Data structure for AAC/BSAC decoding
	struct 
	{
		//!     AAC Object type : (2 : AAC_LC, 4: LTP, 5: SBR, 22: BSAC, ...)
		int		m_iAACObjectType;
		//!     AAC Stream Header type : ( 0 : RAW-AAC, 1: ADTS, 2: ADIF)
		int		m_iAACHeaderType;
		//!     upsampling flag ( 0 : disable, 1: enable)
		//!     only, if( ( m_iAACForceUpsampling == 1 ) && ( samplerate <= 24000 ) ), then out_samplerate = samplerate*2
		int		m_iAACForceUpsampling;
		//!		upmix (mono to stereo) flag (0 : disable, 1: enable)
		//!     only, if( ( m_iAACForceUpmix == 1 ) && ( channel == mono ) ), then out_channel = 2;
		int		m_iAACForceUpmix;
		//!		Dynamic Range Control
		//!		Dynamic Range Control, Enable Dynamic Range Control (0 : disable (default), 1: enable)	
		int		m_iEnableDRC;
		//!		Dynamic Range Control, Scaling factor for boosting gain value, range: 0 (not apply) ~ 127 (fully apply)
		int		m_iDrcBoostFactor;
		//!		Dynamic Range Control, Scaling factor for cutting gain value, range: 0 (not apply) ~ 127 (fully apply)
		int		m_iDrcCutFactor;
		//!		Dynamic Range Control, Target reference level, range: 0 (full scale) ~ 127 (31.75 dB below full-scale)
		int		m_iDrcReferenceLevel;
		//!		Dynamic Range Control, Enable DVB specific heavy compression (aka RF mode), (0 : disable (default), 1: enable)
		int		m_iDrcHeavyCompression;
		//!		ChannelMasking
		//!		bit:  8    7    6    5    4    3    2    1    0
		//!		ch : RR | LR | CS | LFE | RS | LS | CF | RF | LF
		int		m_uiChannelMasking;
		//!		Disable HE-AAC Decoding (Decodig AAC only): 0 (enable HE-AAC Decoding (default), 1 (disable HE-AAC Decoding)
		int		m_uiDisableHEAACDecoding;
		//!		RESERVED
		int		reserved[32-11];
	} m_unAAC;

	//! Data structure for AC3 decoding
	struct 
	{
		//!		Compression mode : ( 0 = COMP_CUSTOMA_A, 1 = COMP_CUSTOM_D, 2 = COMP_LINE, 3 = COMP_RF(Default) )
		int		m_iAC3Compmode;
		//!		Dual mono reproduction mode ( 0 = stereo, 1 = Left mono, 2 = Right mono, 3 = Mixed mono(Default) )
		int		m_iAC3DualmonoMode;
		//!		Output lfe channel present ( 0 = off, 1 = on(Default) )
		int   	m_iAC3Lfe;
  		//!		Output channel configuration ( 0 = reserved, 1 = 1/0(C), 2 = 2/0(L,R), 3 = 3/0(L,C,R), 4 = 2/1(L,R,l), 5 = 3/1(L,C,R,l), 6 = 2/2(L,C,l,r), 7 = 3/2(L,C,R,l,r)(Default) )
		int   	m_iAC3Outputmode;
		//!		Stereo output mode, only effective when mode is 2 ( 0 = audo detect, 1 = Dolby Surround compatible(Lt/Rt), 2 = Stereo(Lo/Ro)(Default) )
		int   	m_iAC3Stereomode;
		//!		Dynamic range compression cut scale factor(Default 0x40000000, = Q30(1.0))
		int   	m_iAC3DynamicScaleHi;
		//!		Dynamic range compression boost scale factor(Default 0x40000000, = Q30(1.0))
		int   	m_iAC3DynamicScaleLow;
		//!		Karaoke capable reproduction mode ( 0 = no vocal, 1 = left vocal, 2 = right vocal, 3 = both vocals(Default) )
		int   	m_iAC3KaraokeMode;
		//!		Contains internal information for helping find sync word
		int		m_iAC3DepositSyncinfo;
		//!		reserved for future needs
		int		m_iAC3DitherOn;
		//!		reserved for future needs
		int		reserved[32-10];
	} m_unAC3Dec;

	//! Data structure for WMA decoding. Note. These information have to be feeded from ASF parser
	struct 
	{
		//!		Block size in bytes of the WMA stream.	  
		unsigned int	m_uiNBlockAlign;

		//!		Codec ID. 0x161 = Standard, 0x162 = Professional, 0x163 = Lossless, 0xA = Voice 
		unsigned int	m_uiWFormatTag;
		//!		reserved for future needs
		int		reserved[32-2];
	} m_unWMADec;

	//! Data structure for AMR WB (+) decoding
	struct 
	{
		//!		select decoding mode within AMRWB and AMRWB+  selected by user or application which can be TCAS_CODEC_AMRWB or TCAS_CODEC_AMRWBPLUS
		int		m_iAWPSelectCodec;
		//!		select sampling frequncy for AMRWB+ decoding mode
		int		m_iAWPOutFS;
		//!		flag for using limitter 
		int		m_iAWPUseLimitter;
		//!		flag for using stereo thru mono
		int		m_iAWPStereoThruMono;
		//!		input file name for debugging
		int		m_pcOpenFileName;
		//!		reserved for future needs
		int		reserved[32-5];
	} m_unAWP;		
		
	//! Data structure for RA G2(cook) decoding. These information are all from the RM demuxer		
	struct 
	{
		//!		number of samples per frame		
		int		m_iRAG2NSamples;
		//!		number of channel
		int		m_iRAG2NChannels;
		//!		number of region
		int		m_iRAG2NRegions;
		//!		Total number of bir per an encoded frame
		int		m_iRAG2NFrameBits;
		//!		Sampling Frequency
		int		m_iRAG2SampRate;
		//!		Starting bin coded with coupling		
		int		m_iRAG2CplStart;
		//!		allocated bit num for coupling
		int		m_iRAG2CplQbits;
		//!		reserved for future needs
		int		reserved[32-7];
	} m_unRAG2;		

	//! Data structure for DTS decoding. These are used by application layer 
	struct 
	{
	//!		Specifies the speaker out configuration(1~8)
	int		m_iDTSSetOutCh;
	//!		set	interleaving mode for output PCM  (interleaving(1)/non-onterleaving(0))
	int		m_iDTSInterleavingFlag;
	//!		Instructs the decoder to use the specified value as the target sample rate for sub audio(8000,12000..)
	int		m_iDTSUpSampleRate;
	//!		Instruct the decoder to use the specified percentage for dynamic range control(default(0), 0~100)
	int		m_iDTSDrcPercent;
	//!		Instruct the decoder to use the LFE mixing (No LFE mixing(0), LFE mixing(1))
	int		m_iDTSLFEMix;
	//!		reserved for future needs
	int		reserved[32-5];
	} m_unDTS;
		
	//! Data structure for DRA decoding
	struct 
	{
		//!		Init mode select ( 0: initialize by input channel,  1: initialize by the frist frame)
		int		m_iDRAInitMode;
		//!     byte order setting (0: Windows byte order, 1:network byte order)
		int		m_iDRAByteOrder;
		//!		Bitrate of this frame (output)
		int		m_iDRAInstantBitrate;		
		//! 	downmix mode select ( 0: disable downmix, 1: stereo to mono)
		int		m_iDRADownmixMode;
		//!		reserved for future needs
		int		reserved[32-4];
	} m_unDRA;

	//! Data structure for FLAC decoding
	struct 
	{
		//!		Init mode select ( 0: file decoding mode,  1: buffer decoding mode, 2: demuxer&decode mode)
		int		m_iFlacInitMode;
		//!		reserved for future needs
		int		reserved[32-1];
	} m_unFLAC;

	//! Data structure for VORBIS decoding
	struct 
	{
		void* (*m_pfCalloc		 ) ( unsigned int , unsigned int );					//!< cdk_ogg_malloc
		//!		reserved for future needs
		int		reserved[32-1];
	} m_unVORBIS;

	//! Data structure for WAV decoding
	struct 
	{
		//!		Format ID
		int		m_iWAVForm_TagID;
		//!		Block size in bytes of the WAV stream.	  
		unsigned int	m_uiNBlockAlign;		
		//!		Endian  ( 0 : little endian,  1: big endian )
		int		m_iEndian;	
		//!		Container type ( 0 : Audio,  1: MPG,  2 : TS)
		int		m_iContainerType;	
		//!		reserved for future needs
		int		reserved[32-4];
	} m_unWAV;
		
	//! Data structure for MP2 decoding
	struct 
	{
		//!		DAB mode select ( 0: OFF,  1: ON)
		int		m_iDABMode;
		//!		reserved for future needs
		int		reserved[32-1];
	} m_unMP2;			
	//! Data structure for DDPDec decoding
	struct 
	{
		//!		DDP stream buffer size in words
		int		m_iDDPStreamBufNwords;
		//!     floating point value of gain			
		char   *m_pcMixdataTextbuf;
		//!     dynamic range disable
		int		m_iMixdataTextNsize;
		//!     dynamic range compression mode
		int		m_iCompMode;
		//!     output LFE channel present
		int		m_iOutLfe;
		//!		output channel configuration
		int		m_iOutChanConfig;
		//!		PCM scale factor
		int		m_iPcmScale;
		//!		stereo output mode
		int		m_iStereMode;
		//!		dual mono reproduction mode
		int		m_iDualMode;
		//!		dynamic range compression cut scale factor
		int		m_iDynScaleHigh;
		//! 	dynamic range compression boost scale factor
		int		m_iDynScaleLow;
		//!		Karaoke capable mode
		int		m_iKMode;
		//!		Frame debug flags
		int		m_iFrmDebugFlags;
		//!		Decode debug flags
		int		m_iDecDebugFlags;
		//!		number of output channels set by user
		int		m_iNumOutChannels;
		//!		mix meta data write flag
		int		m_imixdataout;
		//!		[only DDP Converter library] 
		//!		not detect error
		int		m_iQuitOnErr;
		//!		[only DDP Converter library] 
		//!		use SRC mode
		int		m_iSRCMode;	
		//!		[only DDP Converter library] 
		//!		out stream mode, (0:default(PCM/DD both out), 1:DD stream don't out, 2:PCM data don't out)
		int		m_iOutPutMode;	
		//!		reserved for future needs
		int		reserved[32-19];

	} m_unDDPDec;
	
	//! Data structure for MPEG Audio Layer 3/2/1 decoding
	struct 
	{
		//!		Layer type given by parser (3 or 2 or 1)
		int		m_iLayer;
		//!		DAB mode selection for layer 2 decoding ( 0: OFF,  1: ON)
		int     m_iDABMode;
		//!		Enable only one specific layer decoding ( 0: enables decoding all layers, n(=1,2,3): enables decoding layer n only)
		int     m_iEnableLayer;
		
		//!		reserved for future needs
		int		reserved[32-3];
	} m_unMP321Dec;

}union_audio_codec_t;

//! Data structure for Audio decoder initializing on CDK platform
typedef struct adec_init_t
{
	unsigned char  *m_pucExtraData;		//!< extra data
	int 			m_iExtraDataLen;	//!< extra data length in bytes
	
	adec_input_t   *m_psAudiodecInput;
	adec_output_t  *m_psAudiodecOutput;
	
	int 			m_iDownMixMode;

	// Callback Func
	void* (*m_pfMalloc		 ) ( unsigned int );								//!< malloc
	void  (*m_pfFree		 ) ( void* );										//!< free
	void* (*m_pfMemcpy		 ) ( void*, const void*, unsigned int );			//!< memcpy
	void  (*m_pfMemset		 ) ( void*, int, unsigned int );					//!< memset
	void* (*m_pfRealloc		 ) ( void*, unsigned int );							//!< realloc
	void* (*m_pfMemmove		 ) ( void*, const void*, unsigned int );			//!< memmove

	void*		 (*m_pfFopen	) (const char *, const char *);					//!< fopen
	unsigned int (*m_pfFread	) (void*, unsigned int, unsigned int, void* );	//!< fread
	int			 (*m_pfFseek	) (void*, long, int );							//!< fseek
	long		 (*m_pfFtell	) (void* );										//!< ftell
	int			 (*m_pfFclose   ) (void *);										//!< fclose

	char		   *m_pcOpenFileName;
	
	union_audio_codec_t m_unAudioCodecParams;

} adec_init_t;


/*!
 ***********************************************************************
 * \brief
 *		TCC_DTS_DEC		: main api function of dts decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of dts decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_DTS_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_DTS_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_AWP_DEC		: main api function of amr-wb+ decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of amr-wb+ decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_AWP_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */		
int TCC_AWP_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_AAC_DEC		: main api function of aac decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of aac decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_AAC_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */	
int TCC_AAC_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_MP3_DEC		: main api function of mp3 decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of mp3 decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_MP3_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_MP3_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_AC3_DEC		: main api function of ac3 decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of ac3 decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_AC3_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_AC3_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_QCELP_DEC	: main api function of qcelp decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of qcelp decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_QCELP_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_QCELP_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_EVRC_DEC	: main api function of evrc decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of evrc decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_EVRC_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_EVRC_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_FLAC_DEC	: main api function of flac decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of flac decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_FLAC_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_FLAC_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_MP2_DEC		: main api function of mp2 decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of mp2 decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_MP2_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_MP2_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_AMRNB_DEC	: main api function of amr-nb decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of amr-nb decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_AMRNB_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_AMRNB_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_RAG2_DEC	: main api function of RA-G2 decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of RA-G2 decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_RAG2_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_RAG2_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_G722_DEC	: main api function of g.722 decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of g.722 decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_G722_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_G722_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_G729AB_DEC	: main api function of g.729ab decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of g.729ab decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_G729AB_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_G729AB_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_BSAC_DEC	: main api function of bsac decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of bsac decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_BSAC_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_BSAC_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_DRA_DEC		: main api function of dra decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of dra decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_DRA_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_DRA_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_WMA_DEC		: main api function of wma decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of wma decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_WMA_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_WMA_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_APE_DEC		: main api function of ape decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of ape decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_APE_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_APE_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_WAV_DEC		: main api function of wav decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of wav decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_WAV_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_WAV_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_VORBIS_DEC		: main api function of vorbis decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of vorbis decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_VORBIS_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_VORBIS_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_MP3HD_DEC	: main api function of mp3HD decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of mp3HD decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_MP3HD_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_MP3HD_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

/*!
 ***********************************************************************
 * \brief
 *		TCC_DDP_DEC	    : main api function of Dolby digital decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of mp3HD decoder
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_DDP_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
int TCC_DDP_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

int TCC_MP321_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );
#endif //_ADEC_H_
