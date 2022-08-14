/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

#ifndef _MPEG_TS_H_
#define _MPEG_TS_H_

typedef enum{
	SYSTEM_14496_1_SIMPLE_SCENE = 0x01,
	SYSTEM_14496_1_2D_SCENE,
	SYSTEM_14496_1_VRML_SCENE,
	SYSTEM_14496_1_AUDIO_SCENE,
	SYSTEM_14496_1_COMPLETE_SCENE,

	VISUAL_14496_2_SIMPLE = 0x20,
	VISUAL_14496_2_CORE,
	VISUAL_14496_2_MAIN,
	VISUAL_14496_2_SIMPLE_SCALE,
	VISUAL_14496_2_12BIT,
	VISUAL_14496_2_BASIC_ANIM,
	VISUAL_14496_2_ANIM,
	VISUAL_14496_2_SIMPLE_FACE,
	VISUAL_14496_2_SIMPLE_SCAL_TEXT,
	VISUAL_14496_2_CORE_SCAL_TEXT,

	AUDIO_14496_3_AAC_MAIN = 0x40,
	AUDIO_14496_3_AAC_LC,
	AUDIO_14496_3_TF,
	AUDIO_14496_3_TF_MAIN_SCALE,
	AUDIO_14496_3_TF_LC_SCALE,
	AUDIO_14496_3_TWIN_VQ_CORE,
	AUDIO_14496_3_CELP,
	AUDIO_14496_3_HVXC,
	AUDIO_14496_3_HILN,
	AUDIO_14496_3_TTSI,
	AUDIO_14496_3_MAIN_SYNTHETIC,
	AUDIO_14496_3_WAVETABLE_SYNTHETIC,

	VISUAL_13818_2_SIMPLE = 0x60,
	VISUAL_13818_2_MAIN,
	VISUAL_13818_2_SNR,
	VISUAL_13818_2_SPATIAL,
	VISUAL_13818_2_HIGH,
	VISUAL_13818_2_422,
	AUDIO_13818_7,
	AUDIO_13818_3,
	VISUAL_11172_2,
	AUDIO_11172_3

} OBJECT_PROFILE_INDICATION_ID;


typedef enum{
	OBJECT_DESCRIPTOR_STREAM = 0x01,
	CLOCK_REFERENCE_STREAM,
	SCENE_DESCRIPTION_STREAM,
	VISUAL_STREAM,
	AUDIO_STREAM,
	MPEG7_STREAM,
	OBJECT_CONTENT_INFO_STREAM = 0x0A
} OD_STEAM_TYPE;


/*****************************************************************************
*
*  Item : Object Description TAG
*
* Usage :
*
* Where :
*
*****************************************************************************/
enum{
	OBJECT_DESCRIPTOR_TAG = 0x01,
	INITIAL_OBJECT_DESCRIPTOR_TAG,
	ES_DESCRIPTOR_TAG,
	DECODER_CONFIG_DESCRIPTOR_TAG,
	DECODER_SPECIFIC_INFO_TAG,
	SL_CONFIG_DESCRIPTOR_TAG
};
/*****************************************************************************
*
*  Item : PES Flag Mask bit
*
* Usage :
*
* Where :
*
*****************************************************************************/
#define PES_PRIORITY			0x08
#define PES_DATA_ALIGN_INDICAT	0x04
#define PES_COPYRIGHT			0x02
#define PES_ORG_OR_COPY			0x01

#define PES_PTS_FLAG			0x80
#define PES_DTS_FLAG			0x40
#define PES_ESCR_FLAG			0x20
#define PES_ES_RATE_FLAG		0x10
#define PES_DSM_TRICK_MODE		0x08
#define PES_ADD_COPY_INFO		0x04
#define PES_CRC_FLAG			0x02
#define PES_EXTENSION_FLAG		0x01


/*****************************************************************************
*
*  Item : PES Stream ID
*
* Usage :
*
* Where :
*
*****************************************************************************/
enum{
	program_stream_map = 0xBC,
	private_stream_1,
	padding_stream,
	private_stream_2,
	ECM_stream = 0xF0,
	EMM_stream,
	DSMCC_stream,
	ISOIEC_13522_stream,
	ITU_T_Rec_H_222_1_type_A,
	ITU_T_Rec_H_222_1_type_B,
	ITU_T_Rec_H_222_1_type_C,
	ITU_T_Rec_H_222_1_type_D,
	ITU_T_Rec_H_222_1_type_E,
	ancillary_stream,
	program_stream_directory = 0xFF
};


/*****************************************************************************
*
*  Item : PES_HEADER_INFO_STRUCT
*
* Usage : 
*
* Where :
*
*****************************************************************************/
typedef struct	_sl_header_info_struct
{
	unsigned char			ucAccessUnitStartFlag;
	unsigned char			ucAccessUnitEndFlag;
	unsigned char			ucRandomAccessPointFlag;
	unsigned char			ucOCRFlag;
	unsigned char			ucIdleFlag;
	unsigned char			ucPaddingFlag;
	unsigned char			ucPaddingBits;

	unsigned int			uiSeguenceNumber;
	unsigned int			uiOCR;

	unsigned char			ucDecodingTimeStampFlag;
	unsigned char			ucCompositionTimeStampFlag;
//	unsigned char			ucWallClockTimeStampFlag;

	unsigned char			ucInstantBitrateFlag;

	unsigned int			uiDecodingTimeStamp;
	unsigned int			uiCompositionTimeStamp;
	unsigned int			uiAccessUnitLength;
	unsigned int			uiInstantBitrate;
	unsigned int			uiDegradationPriority;
	unsigned int			uiAU_SequenceNumber;
}  SL_HEADER_INFO_STRUCT;

typedef struct _pes_header_info_struct
{
	unsigned char			ucStreamID;
	unsigned char			header_flag1;
	unsigned char			header_flag2;
	unsigned char			PTS_DTS_flags;
	
	unsigned short			uiPacketLength;

	unsigned short			uiESCR_Ext;
	unsigned int			uiESCR_Base;
	unsigned int			uiPTS;
	unsigned int			uiDTS;
	unsigned int			uiES_Rate;
	unsigned int			uiPrevCRC;

	SL_HEADER_INFO_STRUCT	stSLConfig;
} PES_HEADER_INFO_STRUCT;



/*****************************************************************************
*
*  Item : ADAPTATION_INFO_STRUCT
*
* Usage :
*
* Where :
*
*****************************************************************************/
typedef struct _adaptation_field_struct
{
	unsigned char			discontinuity_indicator;
	unsigned char			elementary_stream_priority_indicator;
	unsigned char			random_access_indicator;
	unsigned char			PCR_flag;
	
	unsigned char			OPCR_flag;
	unsigned char			splicing_point_flag;
	unsigned char			splice_countdown;
	
	unsigned int			uiPCRBase;
	unsigned char			ucPCRBaseLSBit;
	unsigned char			ucPCRBaseLastIndex;
	unsigned short			usPCRExt;
	
	unsigned int			uiOPCRBase;
	unsigned char			ucOPCRBaseLSBit;
	unsigned char			ucOPCRBaseLastIndex;
	unsigned short			usOPCRExt;
} ADAPTATION_INFO_STRUCT;


/*****************************************************************************
*
*  Item : TS_HEADER_INFO_STRUCT
*
* Usage : 
*
* Where :
*
*****************************************************************************/

typedef struct _ts_header_info_struct
{
	unsigned short			uiPID;
	unsigned char			fErrorIndicator;
	unsigned char			fPayloadUnitStart;
	
	unsigned char			ucScramblingControl;
	unsigned char			fAdaptField;
	unsigned char			ucAdaptFieldLength;
	unsigned char			ucContinuityCount;
	unsigned char			ucTmp;
	
	ADAPTATION_INFO_STRUCT	stAdaptInfo;
} TS_HEADER_INFO_STRUCT;



#endif        /* _MPEG_TS_H_ */

/* end of file */

