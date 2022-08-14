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
#define	FOURCC_h264 FOURCC('h','2','6','4') 
#define	FOURCC_H264 FOURCC('H','2','6','4') //0x34363248
#define	FOURCC_x264 FOURCC('x','2','6','4') 
#define	FOURCC_X264 FOURCC('X','2','6','4') 
#define	FOURCC_vssh FOURCC('v','s','s','h') 
#define	FOURCC_VSSH FOURCC('V','S','S','H') 
// MPEG-4 Family
#define	FOURCC_mp4v FOURCC('m','p','4','v') //0x7634706d
#define	FOURCC_MP4V FOURCC('M','P','4','V') //0x5634504d
#define	FOURCC_xvid FOURCC('x','v','i','d')
#define	FOURCC_XVID FOURCC('X','V','I','D')
#define	FOURCC_divx FOURCC('d','i','v','x')
#define	FOURCC_DIVX FOURCC('D','I','V','X')
#define	FOURCC_MP43 FOURCC('M','P','4','3')
#define	FOURCC_mp43 FOURCC('m','p','4','3')
#define	FOURCC_DIV4 FOURCC('D','I','V','4')
#define	FOURCC_div4 FOURCC('d','i','v','4')
#define FOURCC_DVX1 FOURCC('D','V','X','1')
#define FOURCC_dvx1 FOURCC('d','v','x','1')

// H.263 Family
#define	FOURCC_h263 FOURCC('h','2','6','3')
#define	FOURCC_s263 FOURCC('s','2','6','3') //0x33363273 ITU H.263 video (3GPP format)
#define	FOURCC_S263 FOURCC('S','2','6','3') //0x33363253
#define	FOURCC_H263 FOURCC('H','2','6','3') //0x33363248

// VC-1 Family
#define	FOURCC_WVC1 FOURCC('W','V','C','1')
#define	FOURCC_wvc1 FOURCC('w','v','c','1')


#define	FOURCC_MJPG FOURCC('M','J','P','G')
#define	FOURCC_mjpg FOURCC('m','j','p','g')

#define	FOURCC_mjpa FOURCC('m','j','p','a') //0x61706a6d
#define	FOURCC_mjpb FOURCC('m','j','p','b')
#define	FOURCC_3IV1 FOURCC('3','I','V','1')
#define	FOURCC_3IV2 FOURCC('3','I','V','2')
#define	FOURCC_SVQ1 FOURCC('S','V','Q','1')
#define	FOURCC_SVQ3 FOURCC('S','V','Q','3')

#define	FOURCC_VP31 FOURCC('V','P','3','1')
#define	FOURCC_cvid FOURCC('c','v','i','d')

#define FOURCC_MPEG FOURCC('M','P','E','G')
#define FOURCC_mpeg FOURCC('m','p','e','g')

#define FOURCC_div3 FOURCC('d','i','v','3')
#define FOURCC_DIV3 FOURCC('D','I','V','3')

#define FOURCC_rv10 FOURCC('r','v','1','0')
#define FOURCC_RV10 FOURCC('R','V','1','0')
#define FOURCC_rv20 FOURCC('r','v','2','0')
#define FOURCC_RV20 FOURCC('R','V','2','0')
#define FOURCC_rv30 FOURCC('r','v','3','0')
#define FOURCC_RV30 FOURCC('R','V','3','0')
#define FOURCC_rv40 FOURCC('r','v','4','0')
#define FOURCC_RV40 FOURCC('R','V','4','0')

#define FOURCC_MP4S FOURCC('M','P','4','S')
#define FOURCC_mp4s FOURCC('m','p','4','s')

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
#define AV_AUDIO_MS_PCM				0x0001
#define AV_AUDIO_MS_ADPCM			0x0002
#define AV_AUDIO_IEEE_FLOAT			0x0003
#define AV_AUDIO_MS_ALAW			0x0006
#define AV_AUDIO_MS_ULAW			0x0007
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

/************************************************************************

	Image defines (not used)

************************************************************************/
#define	FOURCC_AVDJ FOURCC('A','V','D','J')
#define	FOURCC_jpeg FOURCC('j','p','e','g')

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
