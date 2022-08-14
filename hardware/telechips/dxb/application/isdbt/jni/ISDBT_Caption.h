/*
 * Copyright (C) 2013 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __ISDBT_CAPTION_H__
#define __ISDBT_CAPTION_H__

#define	LCD_ROT0L			0
#define	LCD_ROT90L			1
#define	LCD_ROT180L		2
#define	LCD_ROT270L		3
#define	LCD_MIR_ROT0L		10
#define	LCD_MIR_ROT90L	11
#define	LCD_MIR_ROT180L	12
#define	LCD_MIR_ROT270L	13

#define ARGB(a, r, g, b) 			((unsigned int)((a<<24)|(r<<16)|(g<<8)|(b<<0)))
#define ARGB444(a) (((a&0xf0000000)>>16)|((a&0xf00000)>>12)|((a&0xf000)>>8)|((a&0xf0)>>4))

/* ISDB-T Debugging Option Checking Macro */
#define CHECK_ISDBT_CAPTION_DEBUG(a)			((ISDBT_CAPTION_DEBUG & (a)) == (a))
#define MAX_NUM_OF_ISDBT_CAPTION_ELEMENT	(16)
#define	MAX_SIZE_OF_CAPTION_PES				(32*1024)	// However, 3 TS packets is the upper limit.

/* ISDB-T Caption Return Value */
#define ISDBT_CAPTION_GET_FRAME_DATA		(4)
#define ISDBT_CAPTION_EMPTY_FIFO_ELEMENT	(3)
#define ISDBT_CAPTION_PARSING_OK			(2)
#define ISDBT_CAPTION_SUCCESS				(1)

#define ISDBT_CAPTION_ERROR					(0)
#define ISDBT_CAPTION_PARSING_ERROR			(-1)
#define ISDBT_CAPTION_INVALID_PARAMETER		(-2)


/* --------------------------------------------------
**  ISDB-T Caption Debug Flag Difinition
** -------------------------------------------------- */
#define ISDBT_CAPTION_DEBUG_NONE				(0x00000000)
#define ISDBT_CAPTION_DEBUG_DATA_UNIT_DUMP		(0x00000001)
#define ISDBT_CAPTION_DEBUG_ADD_CHAR_CHECK		(0x00000002)


/* --------------------------------------------------
**  Closed Caption Return Type
** -------------------------------------------------- */
#define CCPARS_SUCCESS						(1)

#define CCPARS_ERROR						(0)
#define CCPARS_ERROR_BAD_DATA_SIZE			(-1000)
#define CCPARS_ERROR_FAIL_MEM_ALLOC			(-1001)


/* control function character set code table - ARIB STD-B24 - start*/
/* C0 area */
#define	ARIB_NUL		0x00
#define	ARIB_BEL		0x07
#define	ARIB_APB		0x08
#define	ARIB_APF		0x09
#define	ARIB_APD		0x0A
#define ARIB_APU		0x0B
#define	ARIB_CS			0x0C
#define	ARIB_APR		0x0D
#define	ARIB_LS1		0x0E
#define	ARIB_LS0		0x0F
#define	ARIB_PAPF		0x16
#define	ARIB_CAN		0x18
#define	ARIB_SS2		0x19
#define	ARIB_ESC		0x1B
#define	ARIB_APS		0x1C
#define	ARIB_SS3		0x1D
#define	ARIB_RS			0x1E
#define	ARIB_US			0x1F
#define	ARIB_SP			0x20

/* C1 area */
#define	ARIB_DEL	 	0x7F
#define	ARIB_BKF		0x80
#define	ARIB_RDF		0x81
#define	ARIB_GRF		0x82
#define	ARIB_YLF		0x83
#define	ARIB_BLF		0x84
#define	ARIB_MGF		0x85
#define	ARIB_CNF		0x86
#define	ARIB_WHF		0x87
#define	ARIB_SSZ		0x88
#define	ARIB_MSZ		0x89
#define	ARIB_NSZ		0x8A
#define	ARIB_SZX		0x8B
#define	ARIB_COL		0x90
#define	ARIB_FLC		0x91
#define	ARIB_CDC		0x92
#define	ARIB_POL		0x93
#define	ARIB_WMM		0x94
#define	ARIB_MACRO		0x95
#define	ARIB_HLC		0x97
#define	ARIB_RPC		0x98
#define	ARIB_SPL		0x99
#define	ARIB_STL		0x9A
#define	ARIB_CSI		0x9B
#define	ARIB_TIME		0x9D
/* control function character set code table - ARIB STD-B24 - stop*/

/* Character Size Controls - ARIB_SZX */
#define ARIB_SZX_TINY		(0x60) // Tiny Size
#define ARIB_SZX_DH			(0x41) // Double Height
#define ARIB_SZX_DW			(0x44) // Double Width
#define ARIB_SZX_DHDW		(0x45) // Double Height & Double Width
#define ARIB_SZX_S1			(0x6B) // Special 1
#define ARIB_SZX_S2			(0x64) // Special 2

/* Time Controls - ARIB_TIME */
#define ARIB_TIME_FREE		(0x40) // Free
#define ARIB_TIME_REAL		(0x41) // Real
#define ARIB_TIME_OFFSET	(0x42) // Offset
#define ARIB_TIME_UNIQUE	(0x43) // Unique

/* Maximum Numbers of ARIB_TIME Parameter */
#define MAX_NUMS_OF_ARIB_TIME_PARAMS	(4)

/* Flashing Control - ARIB_FLC */
#define ARIB_FLC_START_NORMAL		(0x40)
#define ARIB_FLC_START_INVERTED		(0x47)
#define ARIB_FLC_STOP				(0x4F)


/* ----------------------------------------------------------------------
**  ARIB Parsing Delimiter Characters
** ---------------------------------------------------------------------- */

#define ISDBT_CAP_MANAGE_DATA		(0x0)	// Management Data
#define ISDBT_CAP_1ST_STATE_DATA	(0x1)	// 1st Language Data (Statement Data)
#define ISDBT_CAP_2ND_STATE_DATA	(0x2)	// 2nd Language Data (Statement Data)
#define ISDBT_CAP_3RD_STATE_DATA	(0x3)	// 3rd Language Data (Statement Data)
#define ISDBT_CAP_4TH_STATE_DATA	(0x4)	// 4th Language Data (Statement Data)
#define ISDBT_CAP_5TH_STATE_DATA	(0x5)	// 5th Language Data (Statement Data)
#define ISDBT_CAP_6TH_STATE_DATA	(0x6)	// 6th Language Data (Statement Data)
#define ISDBT_CAP_7TH_STATE_DATA	(0x7)	// 7th Language Data (Statement Data)
#define ISDBT_CAP_8TH_STATE_DATA	(0x8)	// 8th Language Data (Statement Data)

#define	ISDBT_CAP_LANGUAGE_MASK		(0xF)	// ISDB-T Caption Language Mask
#define	MAX_NUM_OF_ISDBT_CAP_LANG	(8)

/* types of data unit of Caption data, arib std-b24 v5.0-e1 */
#define	ISDBT_DUNIT_STATEMENT_BODY		0x20
#define	ISDBT_DUNIT_GEOMETRIC 			0x28
#define	ISDBT_DUNIT_SYNTHESIZED_SOUND	0x2c
#define	ISDBT_DUNIT_1BYTE_DRCS 			0x30
#define	ISDBT_DUNIT_2BYTE_DRCS 			0x31
#define	ISDBT_DUNIT_COLOR_MAP 			0x34
#define	ISDBT_DUNIT_BIT_MAP 			0x35

/* Default Font Width & Height */
#define ISDBT_DEFAULT_FONT_WIDTH		(16)
#define ISDBT_DEFAULT_FONT_HEIGHT		(16)

/* Binary Bit Mask */
#define BIT_MASK_OF_B4_TO_B1			(0x000F)
#define BIT_MASK_OF_B6_TO_B1			(0x003F)

#define COLOR_SET_FULL					(0x10) // Full Foreground/Background Color
#define COLOR_SET_HALF					(0x20) // Half Foreground/Background Color

/* Bit Mask of Character Display Effect */
#define CHAR_DISP_MASK_HWRITE				(0x0001)
#define CHAR_DISP_MASK_VWRITE				(0x0002)
#define CHAR_DISP_MASK_UNDERLINE			(0x0004)
#define CHAR_DISP_MASK_FLASHING			(0x0008)
#define CHAR_DISP_MASK_HIGHLIGHT			(0x0010)
#define CHAR_DISP_MASK_HALF_WIDTH		(0x0020)
#define CHAR_DISP_MASK_HALF_HEIGHT		(0x0040)
#define CHAR_DISP_MASK_DOUBLE_WIDTH		(0x0080)
#define CHAR_DISP_MASK_DOUBLE_HEIGHT		(0x0100)
#define CHAR_DISP_MASK_REPEAT				(0x0200)
#define CHAR_DISP_MASK_NONSPACE			(0x0400)


/* FIFO Frame Flag Definition */
#define ISDBT_FIFO_FLAG_DEFAULT			(0x00000000)
#define ISDBT_FIFO_FLAG_1ST_FLASHING	(0x00000001) // 1st Flashing Caption
#define ISDBT_FIFO_FLAG_2ND_FLASHING	(0x00000002) // 2nd Flashing Caption

#define ISDBT_FIFO_FLAG_FLASHING		(ISDBT_FIFO_FLAG_1ST_FLASHING | ISDBT_FIFO_FLAG_2ND_FLASHING)


/* ISO 639-2 Language Code */
#ifndef LANG_CODE
#define LANG_CODE(a, b, c)				(((0xFF & (a)) << 16) | ((0xFF & (b)) << 8) | (0xFF & (c)))
#endif

#define ISO639_LANGUAGE_ENGLIASH		LANG_CODE('e','n','g')
#define ISO639_LANGUAGE_JAPANESE		LANG_CODE('j','p','n')
#define ISO639_LANGUAGE_PORTUGUESE		LANG_CODE('p','o','r')

typedef enum{
	SUB_DISP_H_320X60,
	SUB_DISP_H_960X540,
	SUB_DISP_V_960X540,
	SUB_DISP_H_720X480,
	SUB_DISP_V_720X480,

	SUB_DISP_MAX,
}SUB_DISP_FORMAT;

typedef enum{
	SUB_ARCH_UNKNOWN,
	SUB_ARCH_TCC92XX,
	SUB_ARCH_TCC93XX,
	SUB_ARCH_TCC88XX,
	SUB_ARCH_TCC892X,
	SUB_ARCH_TCC893X,

	SUB_ARCH_MAX
}SUB_ARCH_TYPE;

typedef enum
{
	SEG_UNKNOWN,
	ONESEG_ISDB_T,	
	ONESEG_SBTVD_T,
	FULLSEG_ISDB_T,
	FULLSEG_SBTVD_T,
	SEG_MAX
} E_DTV_MODE_TYPE;


/* Chracter Set Type */
typedef enum
{
	CS_NONE,

	CS_KANJI,
	CS_ALPHANUMERIC,
	CS_LATIN_EXTENSION,
	CS_SPECIAL,
	CS_HIRAGANA,
	CS_KATAKANA,
	CS_MOSAIC_A,
	CS_MOSAIC_B,
	CS_MOSAIC_C,
	CS_MOSAIC_D,
	CS_JISX0201_KATAKANA,
	CS_DRCS_0, // 2-Byte Code
	CS_DRCS_1, // 1-Byte Code
	CS_DRCS_2,
	CS_DRCS_3,
	CS_DRCS_4,
	CS_DRCS_5,
	CS_DRCS_6,
	CS_DRCS_7,
	CS_DRCS_8,
	CS_DRCS_9,
	CS_DRCS_10,
	CS_DRCS_11,
	CS_DRCS_12,
	CS_DRCS_13,
	CS_DRCS_14,
	CS_DRCS_15,
	CS_MACRO,

	CS_MAX
} E_CHARACTER_SET;
/* ====================================================================== */



/* Color Map Lower Address Type Enumeration */
typedef enum
{
	E_CMLA_0,
	E_CMLA_1,
	E_CMLA_2,
	E_CMLA_3,
	E_CMLA_4,
	E_CMLA_5,
	E_CMLA_6,
	E_CMLA_7,
	E_CMLA_8,
	E_CMLA_9,
	E_CMLA_10,
	E_CMLA_11,
	E_CMLA_12,
	E_CMLA_13,
	E_CMLA_14,
	E_CMLA_15,

	E_CMLA_MAX
} E_COLOR_MAP_LOWER_ADDRESS_TYPE;


/* Font Type Enumeration */
typedef enum
{
	E_FONT_SMALL,
	E_FONT_MIDDLE,
	E_FONT_NORMAL,

	E_FONT_MAX
} E_FONT_TYPE;


/* Font Type Enumeration */
typedef enum
{
	E_FONT_EFFECT_NONE,
	E_FONT_EFFECT_TINY_SIZE,
	E_FONT_EFFECT_DOUBLE_HEIGHT,
	E_FONT_EFFECT_DOUBLE_WIDTH,
	E_FONT_EFFECT_DOUBLE_HEIGHT_DOUBLE_WIDTH,
	E_FONT_EFFECT_SPECIAL_1,
	E_FONT_EFFECT_SPECIAL_2,

	E_FONT_EFFECT_MAX
} E_FONT_EFFECT_TYPE;

typedef enum
{
	E_POLARITY_NORMAL,
	E_INV_POLARITY_1,	
	E_INV_POLARITY_2,

	E_POLARITY_MAX
}E_POLARITY_TYPE;

/* --------------------------------------------------
**  Character Display Information Control
** -------------------------------------------------- */
#define E_INIT_DATA_HEAD_MNG			(0x00000001)
#define E_INIT_DATA_HEAD_STATE		(0x00000002)
#define E_INIT_DATA_UNIT_TEXT			(0x00000004)
#define E_INIT_DATA_UNIT_GEO			(0x00000008)
#define E_INIT_CTRL_CODE_CS			(0x00000010)
#define E_INIT_CTRL_CODE_SWF			(0x00000020)
#define E_INIT_HEAD_UNIT_CTRL			(0xffffffff)

typedef enum
{
	E_DISP_CTRL_INIT,

	/* C0/C1 Control Code */
	E_DISP_CTRL_ALERT,
	E_DISP_CTRL_SET_COLOR,
	E_DISP_CTRL_CLR_SCR,
	E_DISP_CTRL_SPACE,
	E_DISP_CTRL_DELETE,
	E_DISP_CTRL_NEW_FIFO,
	E_DISP_CTRL_CHANGE_PTS,
	E_DISP_CTRL_CLR_LINE,
	E_DISP_CTRL_CLR_REC,
	E_DISP_CTRL_SET_POLARITY,
	E_DISP_CTRL_BMP,

	E_DISP_CTRL_MAX
} E_CHAR_DISP_INFO_CONTROL_TYPE;

/* Enumeration related to E_DISP_CTRL_SET_COLOR */
typedef enum
{
	E_SET_COLOR_FORE, // Foreground Color
	E_SET_COLOR_BACK, // Background Color

	E_SET_COLOR_MAX
} E_SET_COLOR_TYPE;

/* Enumeration related to E_DISP_CTRL_CHANGE_PTS */
typedef enum
{
	E_PTS_CHANGE_INC,
	E_PTS_CHANGE_DEC,

	E_PTS_CHANGE_MAX
} E_PTS_CHANGE_TYPE;

/* Enumeration related to E_DISP_CTRL_CLR_LINE */
typedef enum
{
	E_CLR_LINE_FROM_CUR_POS,
	E_CLR_LINE_FROM_FIRST_POS,

	E_CLR_LINE_MAX
} E_CLR_LINE_TYPE;

/* Enumeration related to E_DISP_CTRL_ACTPOS_FORWARD & E_DISP_CTRL_ACTPOS_DOWN */
typedef enum
{
	E_SET_START_POS_CURRENT = 0, // O MUST be current position. 
	E_SET_START_POS_INIT,

	E_SET_START_POS_MAX
} E_SET_START_POS_TYPE;

/* Enumeration related to E_DISP_CTRL_SET_LINING */
typedef enum
{
	E_SET_LINING_STOP,
	E_SET_LINING_START,

	E_SET_LINING_MAX
} E_SET_LINING_TYPE;

/* Enumeration related to E_DISP_CTRL_SET_FLASHING */
typedef enum
{
	E_SET_FLASHING_STOP,
	E_SET_FLASHING_OPEN,
	E_SET_FLASHING_PLAY,

	E_SET_FLASHING_MAX
} E_SET_FLASHING_TYPE;

/* Enumeration related to E_DISP_CTRL_SET_REPEAT_CHAR */
typedef enum
{
	E_SET_REPEAT_CHAR_COUNT,
	E_SET_REPEAT_CHAR_TO_END_OF_LINE,

	E_SET_REPEAT_CHAR_MAX
} E_SET_REPEAT_CHAR_TYPE;

/* Enumeration related to E_DISP_CTRL_SET_SPACING */
typedef enum
{
	E_SET_SPACING_HORIZONTAL,
	E_SET_SPACING_VERTICAL,

	E_SET_SPACING_MAX
} E_SET_SPACING_TYPE;


/********************* structures ************************/
typedef struct{
	int flc_idx_num;
	char *p_flc_idx_grp;
	
	int x;
	int y;
	int size;
	char *p_data;	
}T_BITMAP_DATA_TYPE;

typedef struct{
	unsigned int mngr_grp_type;
	unsigned int mngr_grp_changed;
	unsigned int sts_grp_type;
	unsigned int sts_grp_changed;
}T_GRP_DATA_TYPE;

typedef struct
{
	unsigned int	CharSet;
	unsigned char	CharCode[4];
} T_CC_CHAR_INFO;

typedef struct _drcs_font_data
{
	unsigned char 	fontId;
	unsigned char	mode;
	unsigned int	depth;
	unsigned int	width;
	unsigned int	height;
	unsigned char 	*patternData;
	unsigned int	regionX;
	unsigned int	regionY;
	unsigned short	geometricData_length;
	unsigned char 	*geometricData;

	void 			*ptrNext;
} DRCS_FONT_DATA;

typedef struct _drcs_pattern_data
{
	unsigned int	NumberOfCode;
	unsigned short	CharacterCode;
	unsigned int	NumberOfFont;

	DRCS_FONT_DATA	*ptrFontData;
} DRCS_PATTERN_DATA;


typedef struct _isdbt_caption_drcs
{
	void			*ptrPrev;
	void			*ptrNext;

	DRCS_PATTERN_DATA	PatternHeader;
} ISDBT_CAPTION_DRCS;


typedef struct _ts_pes_caption_data_unit
{
	unsigned char 	unit_separator;
	unsigned char 	data_unit_parameter;
	unsigned int 	data_unit_size;
	unsigned char	*pData;

	void			 *ptrNextDUnit;
} TS_PES_CAP_DATAUNIT;


typedef struct _ts_pes_caption_state
{
	unsigned char 	TMD;
	unsigned int	STM_time;
	unsigned short 	STM_etc;
	unsigned int	data_unit_loop_length;

	void 		 	*DUnit;	
} TS_PES_CAP_STATEDATA;


typedef struct _ts_pes_caption_language
{
	unsigned char 	language_tag;
	unsigned char 	DMF;
	unsigned char 	DC;
	unsigned int		ISO_639_language_code;
	unsigned char 	Format;
	unsigned char 	TCS;
	unsigned char		rollup_mode;
} TS_PES_CAP_LANGUAGECODE;


typedef struct _ts_pes_caption_management
{
	unsigned char 	TMD;
	unsigned int	OTM_time;
	unsigned short	OTM_etc;
	unsigned char 	num_languages;

	TS_PES_CAP_LANGUAGECODE	LCode[MAX_NUM_OF_ISDBT_CAP_LANG];

	unsigned int	data_unit_loop_length;

	void 			*DUnit;
} TS_PES_CAP_MGEDATA;


typedef struct _ts_pes_caption_datagroup
{
	unsigned char 	data_group_id;
	unsigned char 	data_group_version;
	unsigned char 	data_group_link_number;
	unsigned char 	last_data_group_link_number;
	unsigned short 	data_group_size;
	unsigned short 	CRC_16;

	TS_PES_CAP_STATEDATA	DState;
	TS_PES_CAP_MGEDATA		DMnge;

	void 			*ptrNextDGroup;
} TS_PES_CAP_DATAGROUP;


typedef struct _ts_pes_caption_info
{
	unsigned char	data_identifier;
	unsigned char	private_stream_id;
	unsigned char	reserved_future_use;
	unsigned char	PES_data_packet_header_length;

	void 			*DGroup;
} TS_PES_CAP_PARM;

typedef struct _isdbt_caption_info
{
	E_DTV_MODE_TYPE 			isdbt_type;

	TS_PES_CAP_LANGUAGECODE 	LanguageInfo[MAX_NUM_OF_ISDBT_CAP_LANG];
	unsigned char					ucNumsOfLangCode;
	unsigned char					ucSelectLangCode;
} ISDBT_CAPTION_INFO;

typedef struct{
	unsigned int x;
	unsigned int y;
	unsigned int w;
	unsigned int h;
	int handle;
}ISDBT_BMP_DATA;

typedef struct{
	int total_bmp_num;
	ISDBT_BMP_DATA	bmp[2];
}ISDBT_BMP_INFO;
	
typedef struct _isdbt_char_disp_info
{
	E_DTV_MODE_TYPE dtv_type;
	int data_type;
	unsigned int uiCalcPos;
 	
	unsigned int uiFontType;
	unsigned int uiFontEffect;
	unsigned int uiPaletteNumber;
	
	unsigned int uiPolChanges;
	unsigned int uiOriForeColor;
	unsigned int uiOriBackColor;
	unsigned int uiOriHalfForeColor;
	unsigned int uiOriHalfBackColor;
	
	unsigned int uiForeColor;
	unsigned int uiBackColor;
	unsigned int uiHalfForeColor;
	unsigned int uiHalfBackColor;
	
	unsigned int uiRasterColor;
	unsigned int uiClearColor;
	unsigned short usDispMode;
	unsigned int uiRepeatCharCount;
	unsigned int uiCharDispCount;
	unsigned int uiPolarity;

	unsigned int sub_plane_w;
 	unsigned int sub_plane_h;

	unsigned int mngr_disp_w;
	unsigned int mngr_disp_h;

	unsigned int swf_disp_w;
	unsigned int swf_disp_h;
	
	unsigned int act_disp_w;
	unsigned int act_disp_h;
	
	unsigned int def_font_w;
	unsigned int def_font_h;
	unsigned int def_hs;
	unsigned int def_vs;
	
	unsigned int act_font_w;
	unsigned int act_font_h;
	unsigned int act_hs;
 	unsigned int act_vs;	

	unsigned int mngr_font_w;
	unsigned int mngr_font_h;
	unsigned int mngr_hs;
	unsigned int mngr_vs;

	unsigned int mngr_pos_x;
	unsigned int mngr_pos_y;
	
 	unsigned int origin_pos_x;
 	unsigned int origin_pos_y;
	
	int act_pos_x;
	int act_pos_y;

	unsigned int uiLineNum;
	unsigned int uiMaxLineNum;
	unsigned int uiCharNumInLine;
	unsigned int uiMaxCharInLine;

	double ratio_x;
	double ratio_y;

	int disp_format;
	unsigned int disp_rollup;
	unsigned int disp_flash;
	unsigned int disp_highlight;

	unsigned int  nonspace_char;

	ISDBT_BMP_INFO	bmp_info;

	double ratio_drcs_x;	//used to scale-up 1seg DRCS
	double ratio_drcs_y;
} T_CHAR_DISP_INFO;

typedef struct{
	T_CHAR_DISP_INFO 	disp_info;
	int 					disp_handle;
	int 					backup_handle;
	unsigned int 			need_new_handle;
	unsigned int 			ctrl_char_num;
	unsigned long long 		pts;
	int	total_num;
}T_SUB_CONTEXT;

extern int ISDBTCap_ControlCharDisp(T_SUB_CONTEXT *p_sub_ctx, int mngr, unsigned int ucControlType, unsigned int param1, unsigned int param2, unsigned int param3);
extern void CCPARSE_Set_FontSize(unsigned int width, unsigned int height);

extern void CCPARS_Set_Lang(int data_type, int lang_index);
extern int CCPARS_Get_Lang(int data_type);
extern void CCPARS_Init_GSet(int data_type, E_DTV_MODE_TYPE dtv_type);

extern int CCPARS_Parse_CCPES(T_SUB_CONTEXT *p_sub_ctx, void *handle, void *pRawData, int RawDataSize, TS_PES_CAP_PARM *pstCapParmStart, int *DUnitNum);
extern int CCPARS_Deallocate_CCData(void *ptr);
extern void *CCPARS_Control_Process(T_SUB_CONTEXT *p_sub_ctx, int management, void *handle, void *pRawData, T_CC_CHAR_INFO *pstCurrCap);

extern void *CCPARS_Dealloc_DRCS(void *ptr);
extern void *CCPARS_Get_DRCS(unsigned char *pCharCode, unsigned int iCharSet, unsigned int iWidth, unsigned int iHeight);
extern void *CCPARS_Control_DRCS(void *handle, void *pRawData, int RawDataSize, int ByteType);
extern int CCPARS_Check_SS(int data_type);
extern void CCPARS_Macro_Process(T_SUB_CONTEXT *p_sub_ctx, unsigned char *pCharCode);
extern int ISDBT_CC_Is_NonSpace(unsigned char *pInCharCode);
extern int CCPARS_Bitmap_Process(T_SUB_CONTEXT *p_sub_ctx, void *handle, void *pRawData, int size);

#endif /* __ISDBT_CAPTION_H__ */
