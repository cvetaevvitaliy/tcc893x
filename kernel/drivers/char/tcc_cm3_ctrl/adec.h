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

#ifndef _ADEC_H_
#define _ADEC_H_

#include "audio_common.h"

typedef audio_streaminfo_t adec_input_t;
typedef audio_pcminfo_t adec_output_t;

#define		TCAS_CODEC_AMRWB		0x100
#define		TCAS_CODEC_AMRWBPLUS	(TCAS_CODEC_AMRWB+1)

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
		//!		RESERVED
		int		reserved[32-4];
	} m_unAAC;

	//! Data structure for AC3 decoding
	struct 
	{
		//!		Compression mode : ( 0 = COMP_CUSTOMA_A, 1 = COMP_CUSTOM_D, 2 = COMP_LINE, 3 = COMP_RF(Default) )
		int		m_iAC3Compmode;
		//!   Dual mono reproduction mode ( 0 = stereo, 1 = Left mono, 2 = Right mono, 3 = Mixed mono(Default) )
		int		m_iAC3DualmonoMode;
		//!   Output lfe channel present ( 0 = off, 1 = on(Default) )
		int   	m_iAC3Lfe;
  		//!   Output channel configuration ( 0 = reserved, 1 = 1/0(C), 2 = 2/0(L,R), 3 = 3/0(L,C,R), 4 = 2/1(L,R,l), 
    	//!   5 = 3/1(L,C,R,l), 6 = 2/2(L,C,l,r), 7 = 3/2(L,C,R,l,r)(Default) )
		int   	m_iAC3Outputmode;
		//!   Stereo output mode, only effective when mode is 2 ( 0 = audo detect, 1 = Dolby Surround compatible(Lt/Rt), 2 = Stereo(Lo/Ro)(Default) )
		int   	m_iAC3Stereomode;
		//!   Dynamic range compression cut scale factor(Default 0x40000000, = Q30(1.0))
		int   	m_iAC3DynamicScaleHi;
		//!   Dynamic range compression boost scale factor(Default 0x40000000, = Q30(1.0))
		int   	m_iAC3DynamicScaleLow;
		//!		Karaoke capable reproduction mode ( 0 = no vocal, 1 = left vocal, 2 = right vocal, 3 = both vocals(Default) )
		int   	m_iAC3KaraokeMode;
		//!   Contains internal information for helping find sync word
		int		m_iAC3DepositSyncinfo;
		//!   reserved for future needs
		int		reserved[32-9];
	} m_unAC3Dec;

	//!/ Data structure for WMA decoding
	struct 
	{
		//!   Note. below informations have to be feeded from ASF parser			
		//!   block size in bytes of the WMA stream.	  
		unsigned int	m_uiNBlockAlign;
		//!   Codec ID 
		//!   0x161 = Standard, 0x162 = Professional, 0x163 = Lossless, 0xA = Voice
		unsigned int	m_uiWFormatTag;
		//!   reserved for future needs
		int		reserved[32-2];
	} m_unWMADec;

	//! Data structure for AMR WB (+) decoding
	struct 
	{
		//!		select decoding mode within AMRWB and AMRWB+  selected by user or application 
		//!		which can be TCAS_CODEC_AMRWB or TCAS_CODEC_AMRWBPLUS
		int		m_iAWPSelectCodec;
		//!		select sampling frequncy for AMRWB+ decoding mode
		int		m_iAWPOutFS;
		//!		flag for using limitter 
		int		m_iAWPUseLimitter;
		//!		flag for using stereo thru mono
		int		m_iAWPStereoThruMono;
		//!		input file name for debugging
		int		m_pcOpenFileName;
		//!		RESERVED
		int		reserved[32-5];
	} m_unAWP;		
		
	//! Data structure for RA G2(cook) decoding
	//!	These are all from the RM demuxer		
	struct 
	{
		//!			number of samples per frame		
		int			m_iRAG2NSamples;
		//!			number of channel
		int			m_iRAG2NChannels;
		//!			number of region
		int			m_iRAG2NRegions;
		//!			Total number of bir per an encoded frame
		int			m_iRAG2NFrameBits;
		//!			Sampling Frequency
		int			m_iRAG2SampRate;
		//!			Starting bin coded with coupling		
		int			m_iRAG2CplStart;
		//!			allocated bit num for coupling
		int			m_iRAG2CplQbits;
		//!			RESERVED	
		int			reserved[32-7];
	} m_unRAG2;		

	//! Data structure for DTS decoding
	//!	These are used by application layer
	struct 
	{
		//!		set output channel using TCAS_ChannelType
		int		m_iDTSSetOutCh;
		//!		set	interleaving mode for output PCM  interleaving(1)/non-onterleaving(0) 
		int		m_iDTSInterleavingFlag;
		//!		RESERVED
		int		reserved[32-2];
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
		//!		RESERBED
		int		reserved[32-4];
	} m_unDRA;

	//! Data structure for FLAC decoding
	struct 
	{
		//!		Init mode select ( 0: file decoding mode,  1: buffer decoding mode, 2: demuxer&decode mode)
		int		m_iFlacInitMode;
		int		reserved[32-1];
	} m_unFLAC;

	//! Data structure for VORBIS decoding
	struct 
	{
		//!		Init mode select ( 0: file decoding mode,  1: buffer decoding mode, 2: demuxer&decode mode)
		void* (*m_pfCalloc		 ) ( unsigned int , unsigned int );					//!< cdk_ogg_malloc
		int		reserved[32-1];
	} m_unVORBIS;

	//! Data structure for WAV decoding
	struct 
	{
		//!		Init mode select ( 0: file decoding mode,  1: buffer decoding mode, 2: demuxer&decode mode)
		int		m_iWAVForm_TagID;
		int		reserved[32-1];
	} m_unWAV;
		
	//! Data structure for liba52 decoding
	struct 
	{
		//!     To disable adjust level set it to 1, else set to 0
		int		m_iDisableAdjust;
		//!     floating point value of gain			
		float	m_fGain;
		//!     dynamic range disable
		int		m_iDisableDynrng;
		//!     level setting
		float	m_fLevel;
		//!     bias value
		float	m_fBias;
		//!		RESERVED
		int		reserved[32-5];
	} m_unLiba52;
			
} union_audio_codec_t;
	
//! data structure for init Audio decoder of CDK platform
typedef struct adec_init_t
{
	unsigned char  *m_pucExtraData;		//!< extra data
	int 			m_iExtraDataLen;	//!< extra data length
	
	adec_input_t   *m_psAudiodecInput;
	adec_output_t  *m_psAudiodecOutput;
	
	int 			m_iDownMixMode;

	//! Callback Func
	void* (*m_pfMalloc		 ) ( unsigned int );					//!< malloc
	void  (*m_pfFree		 ) ( void* );							//!< free
	void* (*m_pfMemcpy		 ) ( void*, const void*, unsigned int );//!< memcpy
	void  (*m_pfMemset		 ) ( void*, int, unsigned int );		//!< memset
	void* (*m_pfRealloc		 ) ( void*, unsigned int );				//!< realloc
	void* (*m_pfMemmove		 ) ( void*, const void*, unsigned int );//!< memmove

	void*		 (*m_pfFopen	) (const char *, const char *);						//!< fopen
	unsigned int (*m_pfFread	) (void*, unsigned int, unsigned int, void* );		//!< fread
	int			 (*m_pfFseek	) (void*, long, int );								//!< fseek
	long		 (*m_pfFtell	) (void* );											//!< ftell
	int			 (*m_pfFclose   ) (void *);											//!< fclose

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
 *		TCC_WAV_DEC		: main api function of ape decoder
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of ape decoder
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
 *		TCC_A52_DEC		: main api function of vorbis decoder
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
int TCC_A52_DEC( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );


#endif //_ADEC_H_
