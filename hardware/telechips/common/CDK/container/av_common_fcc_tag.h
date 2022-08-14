/****************************************************************************
 *   FileName    : av_common_fcc_tag.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information¡¯s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#ifndef _AV_COMMON_FCC_TAG_H_
#define _AV_COMMON_FCC_TAG_H_

#ifndef FOURCC
#define FOURCC(ch0, ch1, ch2, ch3)                              \
			 ((unsigned long)(unsigned char)(ch0) | ((unsigned long)(unsigned char)(ch1) << 8) |   \
			 ((unsigned long)(unsigned char)(ch2) << 16) | ((unsigned long)(unsigned char)(ch3) << 24 ))
#endif
/************************************************************************

	Video Four Character Code

************************************************************************/
#define FOURCC_vide FOURCC('v','i','d','e') // type

// H.264 Family
#define	FOURCC_avc1 FOURCC('a','v','c','1') //0x31637661
#define	FOURCC_AVC1 FOURCC('A','V','C','1') //0x31435641
#define	FOURCC_h264 FOURCC('h','2','6','4') //0x34363268
#define	FOURCC_H264 FOURCC('H','2','6','4') //0x34363248
#define	FOURCC_x264 FOURCC('x','2','6','4') //0x34363278
#define	FOURCC_X264 FOURCC('X','2','6','4') //0x34363258
#define	FOURCC_vssh FOURCC('v','s','s','h') 
#define	FOURCC_VSSH FOURCC('V','S','S','H') 
#define	FOURCC_z264 FOURCC('z','2','6','4') //0x3436327A
#define	FOURCC_Z264 FOURCC('Z','2','6','4') //0x3436325A
// MVC
#define	FOURCC_mvc 	FOURCC('m','v','c',' ') 
#define	FOURCC_MVC 	FOURCC('M','V','C',' ') 

// MPEG-4 Family
#define	FOURCC_mp4v FOURCC('m','p','4','v') //0x7634706d
#define	FOURCC_MP4V FOURCC('M','P','4','V') //0x5634504d

#define	FOURCC_xvid FOURCC('x','v','i','d')
#define	FOURCC_XVID FOURCC('X','V','I','D') // http://www.xvidmovies.com/codec/

#define	FOURCC_3iv2 FOURCC('3','i','v','2') // 3ivx, MPEG4-based codec. To be used for "3ivx Delta 4.0."
#define	FOURCC_3IV2 FOURCC('3','I','V','2') 

#define FOURCC_SEDG FOURCC('S','E','D','G') //0x47444553: Samsung MPEG-4 (Part 2)
#define FOURCC_RMP4 FOURCC('R','M','P','4') //0x34504d52: According to fourcc.org, RMP4 is MPEG-4 ASP from Sigma Designs' "REALmagic" encoder.

#define	FOURCC_MP4S FOURCC('M','P','4','S') // The first ISO standard codec for use with the Sharp digital camera implementing a restricted feature set of MPEG4.
#define	FOURCC_mp4s FOURCC('m','p','4','s')

#define	FOURCC_M4S2 FOURCC('M','4','S','2') // Microsoft MPEG-4 version2 simple profile
#define	FOURCC_m4s2 FOURCC('m','4','s','2')

#define FOURCC_DVX1 FOURCC('D','V','X','1') // Lusent
#define FOURCC_dvx1 FOURCC('d','v','x','1')

#define	FOURCC_FMD4 FOURCC('F','M','D','4')
#define	FOURCC_fmd4 FOURCC('f','m','d','4')

#define	FOURCC_FVFW FOURCC('F','V','F','W')
#define	FOURCC_fvfw FOURCC('f','v','f','w')

#define	FOURCC_RMP4 FOURCC('R','M','P','4')
#define	FOURCC_rmp4 FOURCC('r','m','p','4')

#define FOURCC_UMP4 FOURCC('U','M','P','4')
#define FOURCC_SMP4 FOURCC('S','M','P','4')
#define FOURCC_WV1F FOURCC('W','V','1','F')
#define FOURCC_EM4A FOURCC('E','M','4','A')
#define FOURCC_DIGI FOURCC('D','I','G','I')
// DIVX-5
#define	FOURCC_dx50 FOURCC('d','x','5','0')
#define	FOURCC_DX50 FOURCC('D','X','5','0')
#define	FOURCC_divx FOURCC('d','i','v','x')
#define	FOURCC_DIVX FOURCC('D','I','V','X')
#define	FOURCC_DivX FOURCC('D','i','v','X')

// DIVX-3
// DivX 3 exists in two versions: 
// a low-motion version (fourcc DIV3), 
// and a fast-motion version (fourcc DIV4 - NOT to be confused with DivX 4). 
#define	FOURCC_MP43 FOURCC('M','P','4','3')
#define	FOURCC_mp43 FOURCC('m','p','4','3')
#define FOURCC_div3 FOURCC('d','i','v','3')
#define FOURCC_DIV3 FOURCC('D','I','V','3')
#define	FOURCC_DIV4 FOURCC('D','I','V','4')
#define	FOURCC_div4 FOURCC('d','i','v','4')

// H.263
#define	FOURCC_h263 FOURCC('h','2','6','3')
#define	FOURCC_H263 FOURCC('H','2','6','3') //0x33363248
#define	FOURCC_s263 FOURCC('s','2','6','3') //0x33363273 ITU H.263 video (3GPP format)
#define	FOURCC_S263 FOURCC('S','2','6','3') //0x33363253

// Sorenson Media Video
#define	FOURCC_flv1 FOURCC('f','l','v','1') // sorenson h.263 or Sorenson Spark 
#define	FOURCC_FLV1 FOURCC('F','L','V','1')
#define	FOURCC_SVQ1 FOURCC('S','V','Q','1') // Sorenson Video 1
#define	FOURCC_SVQ3 FOURCC('S','V','Q','3') // Sorenson Video 3: a tweaked version of H.264

// Windows Media Video
// VC-1
#define FOURCC_WMV3 FOURCC('W','M','V','3') // Complex profile is not covered by VC-1 standard and may occur in old WMV3 files where it was called "advanced profile". 
#define FOURCC_wmv3 FOURCC('w','m','v','3')
#define FOURCC_WMV9 FOURCC('W','M','V','9')
#define FOURCC_wmv9 FOURCC('w','m','v','9')
#define FOURCC_WMVA FOURCC('W','M','V','A') // WMVA(WMV advanced profile old version), Windows Media Player 10
#define FOURCC_wmva FOURCC('w','m','v','a')
#define FOURCC_WVC1 FOURCC('W','V','C','1') // WVC1(WMV advanced profile current version), Windows Media Player 11
#define FOURCC_wvc1 FOURCC('w','v','c','1')
#define FOURCC_WMVP FOURCC('W','M','V','P')
#define FOURCC_wmvp FOURCC('w','m','v','p')
#define FOURCC_WMVR FOURCC('W','M','V','R')
#define FOURCC_wmvr FOURCC('w','m','v','r')
#define FOURCC_VC1	FOURCC('V','C','1',' ')
#define FOURCC_vc1	FOURCC('v','c','1',' ')
// wmv7/8
#define FOURCC_WMV2 FOURCC('W','M','V','2')
#define FOURCC_wmv2 FOURCC('w','m','v','2')
#define FOURCC_WMV1 FOURCC('W','M','V','1')
#define FOURCC_wmv1 FOURCC('w','m','v','1')

// MotionJPEG
#define	FOURCC_MJPG FOURCC('M','J','P','G')
#define	FOURCC_mjpg FOURCC('m','j','p','g')
#define	FOURCC_IJPG FOURCC('I','J','P','G') //it works
 
#define	FOURCC_AVRn FOURCC('A','V','R','n') //it works (2011/04/22)
#define	FOURCC_AVRN FOURCC('A','V','R','N') 
#define	FOURCC_mjpa FOURCC('m','j','p','a') //0x61706a6d
#define	FOURCC_mjpb FOURCC('m','j','p','b')

#define	FOURCC_3IV1 FOURCC('3','I','V','1')
#define	FOURCC_3IV2 FOURCC('3','I','V','2')


#define	FOURCC_VP31 FOURCC('V','P','3','1')
#define	FOURCC_cvid FOURCC('c','v','i','d')

// MPEG-1/2
#define FOURCC_MPEG FOURCC('M','P','E','G')
#define FOURCC_mpeg FOURCC('m','p','e','g')
#define FOURCC_MPG2 FOURCC('M','P','G','2')
#define FOURCC_mpg2 FOURCC('m','p','g','2')
#define FOURCC_MP2V FOURCC('M','P','2','V')
#define FOURCC_mp2v FOURCC('m','p','2','v')
#define FOURCC_mpg1 FOURCC('m','p','g','1')
#define FOURCC_MPG1 FOURCC('M','P','G','1')
#define	FOURCC_m1v1 FOURCC('m','1','v','1')
#define	FOURCC_M1V1 FOURCC('M','1','V','1')
#define	FOURCC_m2v1 FOURCC('m','2','v','1')
#define	FOURCC_M2V1 FOURCC('M','2','V','1')
#define	FOURCC_hdv1 FOURCC('h','d','v','1')
#define	FOURCC_HDV1 FOURCC('H','D','V','1')
#define	FOURCC_hdv2 FOURCC('h','d','v','2')
#define	FOURCC_HDV2 FOURCC('H','D','V','2')

// RealVideo
#define FOURCC_rv10 FOURCC('r','v','1','0')
#define FOURCC_RV10 FOURCC('R','V','1','0')
#define FOURCC_rv20 FOURCC('r','v','2','0')
#define FOURCC_RV20 FOURCC('R','V','2','0')
#define FOURCC_rv30 FOURCC('r','v','3','0')
#define FOURCC_RV30 FOURCC('R','V','3','0')
#define FOURCC_rv40 FOURCC('r','v','4','0')
#define FOURCC_RV40 FOURCC('R','V','4','0')
#define FOURCC_RV89COMBO FOURCC('T','R','O','M')

// AVS
#define FOURCC_AVS  FOURCC('A','V','S',' ')
#define FOURCC_avs  FOURCC('a','v','s',' ')
#define FOURCC_cavs FOURCC('c','a','v','s')
#define FOURCC_CAVS FOURCC('C','A','V','S')

// VPX Family
#define FOURCC_V_VP  FOURCC('V','_','V','P')
#define FOURCC_v_vp  FOURCC('v','_','v','p')
// VP8
#define FOURCC_VP80  FOURCC('V','P','8','0')
#define FOURCC_vp80  FOURCC('v','p','8','0')
// VP9
#define FOURCC_VP90  FOURCC('V','P','9','0')
#define FOURCC_vp90  FOURCC('v','p','9','0')

/************************************************************************

	Audio defines

************************************************************************/
#define FOURCC_soun FOURCC('s','o','u','n') // type

#define FOURCC_sowt FOURCC('s','o','w','t')
#define FOURCC_twos FOURCC('t','w','o','s')
#define FOURCC_raw  FOURCC('r','a','w',' ')
#define FOURCC_NONE FOURCC('N','O','N','E')
#define FOURCC_ulaw FOURCC('u','l','a','w')
#define FOURCC_alaw FOURCC('a','l','a','w')
#define FOURCC_QDM2 FOURCC('Q','D','M','2')
#define FOURCC_ima4 FOURCC('i','m','a','4')
#define FOURCC_mp4a FOURCC('m','p','4','a')	
#define FOURCC_samr FOURCC('s','a','m','r') // Narrowband AMR voice(3GPP)
#define FOURCC_sawb FOURCC('s','a','w','b') // Wideband AMR voice(3GPP)
#define FOURCC_sawp FOURCC('s','a','w','p') // Extended AMR-WB (AMR-WB+) (3GPP)
#define FOURCC_sqcp FOURCC('s','q','c','p') // 13K(QCELP) Voice(3GPP2) 0xE1 
#define FOURCC_QLCM FOURCC('Q','L','C','M')
#define FOURCC_sevc FOURCC('s','e','v','c') // EVRC Voice(3GPP2) 0xA0
#define FOURCC_EVRC FOURCC('E','V','R','C')
#define FOURCC_secb FOURCC('s','e','c','b') // EVRC-B Voice(3GPP2) 
#define FOURCC_secw FOURCC('s','e','c','w') // EVRC-WB Voice(3GPP2) 
#define FOURCC_ssmv FOURCC('s','s','m','v') // SMV Voice(3GPP2) 0xA1
#define FOURCC_svmr FOURCC('s','v','m','r') // VMR Voice(3GPP2)
#define FOURCC_ac3  FOURCC('a','c','-','3') // ac-3


// rfc2361
// RIFF AudioFormat Tags
// http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/RIFF.html
#define AV_AUDIO_UNKNOWN			0x0000
#define AV_AUDIO_MS_PCM				0x0001		// Little endian LPCM	
#define AV_AUDIO_MS_ADPCM			0x0002
#define AV_AUDIO_IEEE_FLOAT			0x0003
#define AV_AUDIO_MS_ALAW			0x0006
#define AV_AUDIO_MS_ULAW			0x0007
#define AV_AUDIO_MS_PCM_SWAP		0x0008		// Big endian LPCM	
#define AV_AUDIO_INTEL_DVI_ADPCM 	0x0011
#define AV_AUDIO_G723_ADPCM 		0x0014

#define AV_AUDIO_DOLBY_AC2	 		0x0030
#define AV_AUDIO_G721_ADPCM 		0x0040
#define AV_AUDIO_G728_CELP	 		0x0041
#define AV_AUDIO_MS_G723	 		0x0042
#define AV_AUDIO_ITUT_G726	 		0x0045
#define AV_AUDIO_MP2		 		0x0050 //MPEG 1 Layer 2 audio codec
#define AV_AUDIO_MP3		 		0x0055 //MPEG-1 Layer 3 (MP3) audio codec
#define AV_AUDIO_APICOM_G726_ADPCM	0x0064
#define AV_AUDIO_APICOM_G722_ADPCM	0x0065
#define AV_AUDIO_ATNT_G729A	 		0x0083
#define AV_AUDIO_SIEMENS_SBC24 		0x0091
#define AV_AUDIO_DOLBY_AC3_SPDIF 	0x0092
#define AV_AUDIO_MS_AAC				0x00ff //AAC
#define AV_AUDIO_MS_AUDIO1		 	0x0160
#define AV_AUDIO_QDESIGN2	 		0x0450 //QDesign Music
#define AV_AUDIO_AC3 				0x2000 //AC3
#define AV_AUDIO_DTS				0x2001 //DTS
#define AV_AUDIO_RA_1_2_144			0x2002 //RealAudio 1 / 2 14.4
#define AV_AUDIO_RA_1_2_288			0x2003 //RealAudio 1 / 2 28.8
#define AV_AUDIO_RA_G2_8_COOK		0x2004 //RealAudio G2 / 8 Cook (low bitrate)
#define AV_AUDIO_RA_3_4_5_MUSIC		0x2005 //RealAudio 3 / 4 / 5 Music (DNET)
#define AV_AUDIO_RA_10_AAC			0x2006 //RealAudio 10 AAC (RAAC)
#define AV_AUDIO_RA_10_AACP			0x2007 //RealAudio 10 AAC+ (RACP)
#define AV_AUDIO_OGG_VORBIS     	0x566f //Ogg Vorbis (mode 1) 
#define AV_AUDIO_OGG_VORBIS_MODE1	0x674f //Ogg Vorbis (mode 1) 
#define AV_AUDIO_OGG_VORBIS_MODE2	0x6750 //Ogg Vorbis (mode 2) 
#define AV_AUDIO_OGG_VORBIS_MODE3	0x6751 //Ogg Vorbis (mode 3) 
#define AV_AUDIO_OGG_VORBIS_MODE1P	0x676f //Ogg Vorbis (mode 1+) 
#define AV_AUDIO_OGG_VORBIS_MODE2P	0x6770 //Ogg Vorbis (mode 2+) 
#define AV_AUDIO_OGG_VORBIS_MODE3P	0x6771 //Ogg Vorbis (mode 3+) 
#define AV_AUDIO_FAAD_AAC			0x706d //FAAD AAC 
#define AV_AUDIO_FLAC				0xf1ac //Free Lossless Audio Codec FLAC

//http://msdn.microsoft.com/en-us/library/dd443195(VS.85).aspx
#define AV_AUDIO_WMA_STANDARD		0x0161 //Windows Media Audio Standard encoded content
#define AV_AUDIO_WMA_PRO			0x0162 //Windows Media Audio Professional encoded content
#define AV_AUDIO_WMA_LOSSLESS		0x0163 //Windows Media Audio Lossless encoded content
#define AV_AUDIO_WMA_VOICE			0x000a //Windows Media Audio Voice encoded content

// http://www.matroska.org/technical/specs/codecid/index.html
#define AV_AUDIO_TTA1				0x77a1 //The True Audio lossles audio compressor

//Mplayer/etc/codecs.conf
#define AV_AUDIO_AAC				0xAAC0 //Borgtech nonsense tag 

// The others
#define AV_AUDIO_SPEEX				0xA109 //Speex ACM Codec 
#define AV_AUDIO_AMR_WP				0x0056
#define AV_AUDIO_AMR_NB				0x0057
#define AV_AUDIO_AMR_WB				0x0058
#define AV_AUDIO_AAC_706D       	0x0000706d // AAC
#define AV_AUDIO_RAAAC				0x72616163
#define AV_AUDIO_RACP				0x72616370
#define AV_AUDIO_QCELP				0xCC00
#define AV_AUDIO_EVRC				0xCD00
#define AV_AUDIO_EVRC_B				0xCD01
#define AV_AUDIO_EVRC_WB			0xCD02
#define AV_AUDIO_VMR				0xCD10
#define AV_AUDIO_SMV				0xCD20
#define AV_AUDIO_MP3HD 				0xCD40
#define AV_AUDIO_MP1 				0xCC01
#define AV_AUDIO_BSAC_MP4			0x0016
#define AV_AUDIO_BSAC_CDK			0xAAC1
#define AV_AUDIO_COOK				0x636F6F6B
#define AV_AUDIO_cook 				0x434F4F4B 
#define AV_AUDIO_kooc 				0x4B4F4F43 
#define AV_AUDIO_APE				0x0a9e
#define AV_AUDIO_QT_IMA_ADPCM 		0x10011
#define AV_AUDIO_OGG_VORBIS_PACKET 			0x2674F
#define AV_AUDIO_OGG_VORBIS_INTERNAL_VIDEO 	0x1674F
#define AV_AUDIO_OGG_VORBIS_INTERNAL_AUDIO 	0x3674F

#define AV_AUDIO_EAC3				0x2008 //EAC3 Dolby Digital Plus
#define AV_AUDIO_FORCC_AC3 			0x30303032	//0x2000 //AC3

/************************************************************************

	Image defines (not used)

************************************************************************/
#define	FOURCC_AVDJ FOURCC('A','V','D','J')
#define	FOURCC_jpeg FOURCC('j','p','e','g')
#define	FOURCC_JPEG FOURCC('J','P','E','G')

//#define	FOURCC_png$ FOURCC('p','n','g',' ')
#define	FOURCC_png  FOURCC('p','n','g',' ')
#define	FOURCC_tiff FOURCC('t','i','f','f')




/************************************************************************

	Subtitle defines (not used)

************************************************************************/
/*
#define FOURCC_sdsm		FOURCC('s','d','s','m') // type
#define FOURCC_ASCII	FOURCC('A','S','C',' ')
#define FOURCC_OEM		FOURCC('O','E','M',' ')
#define FOURCC_UTF8		FOURCC('U','T','F','8')
#define FOURCC_UTF16	FOURCC('U','T','F','1')
#define FOURCC_SSA		FOURCC('S','S','A',' ')
#define FOURCC_ASS		FOURCC('A','S','S',' ')
#define FOURCC_USF		FOURCC('U','S','F',' ')
#define FOURCC_TEXT		FOURCC('T','E','X','T')
*/
#endif//_AV_COMMON_FCC_TAG_H_
