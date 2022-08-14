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

#ifndef _DVB_PARSE_H_
#define _DVB_PARSE_H_


/*****************************************************************************
*  define declaration                                                                                                         *
*****************************************************************************/
#ifndef FALSE
#define FALSE	(0)
#endif
#ifndef TRUE
#define TRUE	(!FALSE)
#endif
#ifndef NULL
#define NULL	(0)
#endif

#if 1
#define	_DEF_ODD_PARITY_1_		(0x01)
#define	_DEF_ODD_PARITY_2_		(0x02)
#define	_DEF_ODD_PARITY_3_		(0x04)
#define	_DEF_ODD_PARITY_4_		(0x08)
#define	_DEF_ODD_PARITY_5_		(0x10)
#define	_DEF_ODD_PARITY_6_		(0x20)
#define	_DEF_ODD_PARITY_7_		(0x40)
#define	_DEF_ODD_PARITY_8_		(0x80)
#else
#define	_DEF_ODD_PARITY_1_		(0x80)
#define	_DEF_ODD_PARITY_2_		(0x40)
#define	_DEF_ODD_PARITY_3_		(0x20)
#define	_DEF_ODD_PARITY_4_		(0x10)
#define	_DEF_ODD_PARITY_5_		(0x08)
#define	_DEF_ODD_PARITY_6_		(0x04)
#define	_DEF_ODD_PARITY_7_		(0x02)
#define	_DEF_ODD_PARITY_8_		(0x01)
#endif

#if 1
#define	_DEF_ODD_DATA_MASK_	(0x7F)
#else
#define	_DEF_ODD_DATA_MASK_	(0xFE)
#endif
#define	_DEF_ODD_REJECT_DATA_	(0xFF)

#define	_DEF_LEN_HAMMING_8_4_	(1)
#if 1
#define	_DEF_HAMMING_8_4_A_	(0xA3)	/*	1010 0011	(1100 0101)	*/
#define	_DEF_HAMMING_8_4_B_	(0x8E)	/*	1000 1110	(0111 0001)	*/
#define	_DEF_HAMMING_8_4_C_	(0x3A)	/*	0011 1010	(0101 1100)	*/
#define	_DEF_HAMMING_8_4_D_	(0xFF)	/*	1111 1111	(1111 1111)	*/
#else
#define	_DEF_HAMMING_8_4_A_	(0xC5)	/*	1100 0101	*/
#define	_DEF_HAMMING_8_4_B_	(0x71)	/*	0111 0001	*/
#define	_DEF_HAMMING_8_4_C_	(0x5C)	/*	0101 1100	*/
#define	_DEF_HAMMING_8_4_D_	(0xFF)	/*	1111 1111	*/
#endif

#if 1
#define	_DEF_HAMMING_8_4_D1_	(0x02)	/*0000 0010*/
#define	_DEF_HAMMING_8_4_D2_	(0x08)	/*0000 1000*/
#define	_DEF_HAMMING_8_4_D3_	(0x20)	/*0010 0000*/
#define	_DEF_HAMMING_8_4_D4_	(0x80)	/*1000 0000*/
#else
#define	_DEF_HAMMING_8_4_D1_	(0x40)
#define	_DEF_HAMMING_8_4_D2_	(0x10)
#define	_DEF_HAMMING_8_4_D3_	(0x04)
#define	_DEF_HAMMING_8_4_D4_	(0x01)
#endif

#define	_DEF_LEN_HAMMING_24_18_	(3)
#if 1
#define	_DEF_HAMMING_24_18_A_	(0x55)	/*	0101 0101	(1010 1010)	*/
#define	_DEF_HAMMING_24_18_B_	(0x66)	/*	0110 0110	(0110 0110)	*/
#define	_DEF_HAMMING_24_18_C_	(0x78)	/*	0111 1000	(0001 1110)	*/
#define	_DEF_HAMMING_24_18_D_0_	(0x80)	/*	1000 0000	(0000 0001)	*/
#define	_DEF_HAMMING_24_18_D_1_	(0x7F)	/*	0111 1111	(1111 1110)	*/
#define	_DEF_HAMMING_24_18_D_2_	(0x00)	/*	0000 0000	(0000 0000)	*/
#define	_DEF_HAMMING_24_18_E_0_	(0x00)	/*	0000 0000	(0000 0000)	*/
#define	_DEF_HAMMING_24_18_E_1_	(0x80)	/*	1000 0000	(0000 0001)	*/
#define	_DEF_HAMMING_24_18_E_2_	(0x7F)	/*	0111 1111	(1111 1110)	*/
#define	_DEF_HAMMING_24_18_F_	(0xFF)	/*	1111 1111	(1111 1111)	*/
#else
#define	_DEF_HAMMING_24_18_A_		(0xAA)
#define	_DEF_HAMMING_24_18_B_		(0x66)
#define	_DEF_HAMMING_24_18_C_		(0x1E)
#define	_DEF_HAMMING_24_18_D_0_	(0x01)
#define	_DEF_HAMMING_24_18_D_1_	(0xFE)
#define	_DEF_HAMMING_24_18_D_2_	(0x00)
#define	_DEF_HAMMING_24_18_E_0_	(0x00)
#define	_DEF_HAMMING_24_18_E_1_	(0x01)
#define	_DEF_HAMMING_24_18_E_2_	(0xFE)
#define	_DEF_HAMMING_24_18_F_		(0xFF)
#endif

#if 1
#define	_DEF_HAMMING_24_18_D1_		(0x40)
#define	_DEF_HAMMING_24_18_D2_		(0x20)
#define	_DEF_HAMMING_24_18_D3_		(0x10)
#define	_DEF_HAMMING_24_18_D4_		(0x04)
#define	_DEF_HAMMING_24_18_D_BYTE_	(0x7F)
#else
#define	_DEF_HAMMING_24_18_D1_		(0x20)
#define	_DEF_HAMMING_24_18_D2_		(0x08)
#define	_DEF_HAMMING_24_18_D3_		(0x04)
#define	_DEF_HAMMING_24_18_D4_		(0x02)
#define	_DEF_HAMMING_24_18_D_BYTE_	(0x7F)
#endif

#define	_DEF_HAMMING_REJECT_BITS_	(0xFF)
#define	_DEF_HAMMING_MASK_BITS_	(0xFF)

/*****************************************************************************
*****************************************************************************/




/*****************************************************************************
*                                                                            *
*  enum declaration                                                          *
*                                                                            *
*****************************************************************************/
typedef enum
{
	E_DATA_IDENTIFIER_EBU_DATA_MIN	= 0x10,
	E_DATA_IDENTIFIER_EBU_DATA_MAX	= 0x1F

}E_DATA_IDENTIFIER_TYPE;

typedef enum
{
	E_DATA_UNIT_ID_EBU_TELETEXT_NONSUBTITLE	= 0x02,
	E_DATA_UNIT_ID_EBU_TELETEXT_SUBTITLE		= 0x03
	
}E_DATA_UNIT_ID_TYPE;

typedef enum
{
	E_TTX_DATA_IDENTIFIER_RESERVED1_MIN		= 0x00,
	E_TTX_DATA_IDENTIFIER_RESERVED1_MAX		= 0x0F,
	E_TTX_DATA_IDENTIFIER_EBUDATA_MIN		= 0x10, 
	E_TTX_DATA_IDENTIFIER_EBUDATA_MAX		= 0x1F,
	E_TTX_DATA_IDENTIFIER_RESERVED2_MIN		= 0x02,
	E_TTX_DATA_IDENTIFIER_RESERVED2_MAX		= 0x7F,
	E_TTX_DATA_IDENTIFIER_USERDEFINED_MIN	= 0x80,
	E_TTX_DATA_IDENTIFIER_USERDEFINED_MAX	= 0xFF

}E_TTX_DATA_IDENTIFIER_TYPE;

typedef enum
{
	E_TTX_DATA_UNIT_ID_RESERVED1_MIN	= 0x00,
	E_TTX_DATA_UNIT_ID_RESERVED1_MAX	= 0x01,
	E_TTX_DATA_UNIT_ID_EBUTTX_NONSUB	= 0x02,
	E_TTX_DATA_UNIT_ID_EBUTTX_SUB		= 0x03,
	E_TTX_DATA_UNIT_ID_RESERVED2_MIN	= 0x04,
	E_TTX_DATA_UNIT_ID_RESERVED2_MAX	= 0x7F,
	E_TTX_DATA_UNIT_ID_USERDEFINED_MIN	= 0x80,
	E_TTX_DATA_UNIT_ID_USERDEFINED_MAX	= 0xFE,
	E_TTX_DATA_UNIT_ID_STUFFING			= 0xFF
	
}E_TTX_DATA_UNIT_ID;

typedef enum
{
	E_TTX_PACKET_HEADER				= 0x00,
	E_TTX_PACKET_NORMAL_01				= 0x01,
	E_TTX_PACKET_NORMAL_25				= 0x19,
	E_TTX_PACKET_ENHANCEMENTDATA_26	= 0x1A,
	E_TTX_PACKET_PAGELINKING_27		= 0x1B,
	E_TTX_PACKET_ENHANCEMENTDATA_28	= 0x1C,
	E_TTX_PACKET_ENHANCEMENTDATA_29	= 0x1D,
	E_TTX_PACKET_BROADCASTSERVICE_30	= 0x1E,
	E_TTX_PACKET_GENERALCODING_31		= 0x1F
	
}E_TTX_PACKET_TYPE;

/*****************************************************************************
*****************************************************************************/




/*************************************************************************************************************
*
*  Item : PAGE_HEADER	(Format of the page header packet (X/0)
*
* Usage : Contains the Page header
*
*
*											|     Magazine	| Packet		|
*											 \___Number	| Number (0)	|
*						Bytes 1-2			3	|		4		5	|
*						--------------------------------------------
*						|Clock		| Framing	|			|		|
*						| Run-In		| Code		| Packet Address		|
*						--------------------------------------------
*				 _______/						 ___________________/
*		 _______/			 ___________________/
*		|		Bytes 1 - 5	/			6 - 13							14 - 45
*		----------------------------------------------------------------------------------------
*	X/0	|		Prefix		|	Page Address & Control Bits		|	32 Data Bytes (odd parity coded)		|
*		----------------------------------------------------------------------------------------
*					  _______/								 \______________________
*			  _______/																\_______________________
*			/	Byte 6			7			8			9			10			11			12			13	\
*			------------------------------------------------------------------------------------------------
*			| Page		| Page		| Subcode	| Subcode	| Subcode	| Subcode	| Control		| Control		|
*			| Number	| Number	| S1			| S2			| S3			| S4			| Bits		| Bits		|
*			| Units		| Tens		|			| + C4		|			| + C5, C6	| C7 - C10	| C11 - C14	|
*			------------------------------------------------------------------------------------------------
*
*
*************************************************************************************************************/
#define	_DEF_SUBCODE_S0_	(0x03)
#define	_DEF_SUBCODE_S1_	(0x0F)
#define	_DEF_SUBCODE_S2_	(0x07)
#define	_DEF_SUBCODE_S3_	(0x0F)

typedef struct
{
	unsigned char		ucPage_Number;
	unsigned char		ucSub_Code[4];
	unsigned short	usControl_Bit;
	
	unsigned char		data_block[32];
	
}PAGE_HEADER;


/*************************************************************************************************************
*
*  Item : NORMAL_PACKET		(Format of packets X/1 to X/25 for direct display)
*
* Usage : Contains the Normal packets
*
*
*	
*												|     Magazine	| Packet		|
*												 \___Number	| Number (0)	|
*							Bytes 1-2			3	|		4		5	|
*							--------------------------------------------
*							|Clock		| Framing	|			|		|
*							| Run-In		| Code		| Packet Address		|
*							--------------------------------------------
*					 _______/						 ___________________/
*			 _______/			 ___________________/
*			|		Bytes 1 - 5	/						6 - 45
*			--------------------------------------------------------------------------------
*	X/1-25	|		Prefix		|				40 Data Bytes (odd parity coded)					|
*			--------------------------------------------------------------------------------
*
*
*************************************************************************************************************/
typedef struct
{
	/*unsigned char	ucBP[4];*/
	unsigned char	data_block[40];
	
}NORMAL_PACKET;


/*************************************************************************************************************
*
*  Item : PAGE_ENHANCEMENT_DATA_PACKETS		(Format of packets X/26, X/28 and M/29)
*
* Usage : Contains the Page enhancement data packets
*
*
*	
*											|     Magazine	| Packet		|
*											 \___Number	| Number (0)	|
*						Bytes 1-2			3	|		4		5	|
*						--------------------------------------------
*						|Clock		| Framing	|			|		|
*						| Run-In		| Code		| Packet Address		|
*						--------------------------------------------
*				________/						  __________________/
*			  __/			  ___________________/
*			 /	Bytes 1 - 5	/	6		7 - 9		10 - 12											43 - 45
*	X/26	------------------------------------------------------------------------------------------------
*	X/28	|	Prefix		|	DC	   |	Triplet 1	|	Triplet2	|			|	Triplet N	|			|	Triplet 13	|
*	X/29	------------------------------------------------------------------------------------------------
*							|		   |							  ______/			 \_______
*						      /		    \					  _______/							\_____
*						    /			      \				 /										   \
*						   |	Byte 6		|				/   Byte 3N+4			3N+5			3N+6    \
*						   ---------------				--------------------------------------------
*						   |	Designation	|				| 			| 				| 				|
*						   |		Code	|				|	Triplet N	(N=1 to 13)	Hamming 24/18 coded	|
*						   ---------------				| 			|				| 				|
*														--------------------------------------------
*
*************************************************************************************************************/
#define	_DEF_LEN_TRIPLET_			(13)

#define	_DEF_ADDR_MASK_1_		(0x0F)
#define	_DEF_ADDR_MASK_2_		(0x60)

#define	_DEF_MODE_MASK_		(0x7C)

#define	_DEF_DATA_MASK_		(0x7F)

#if 0
typedef enum
{
	E_ROW_FULL_SCREEN_COLOUR			= 0,
	E_ROW_FULL_ROW_COLOUR				= 1,
	E_ROW_SET_ACTIVE_POSITION			= 4,
	E_ROW_ADDRESS_DISPLAY_ROW_0		= 7,
	E_ROW_PDC_COUNTRY					= 8,
	E_ROW_PDC_MONTH_DAY				= 9,
	E_ROW_PDC_STARTING_TIME			= 10,
	E_ROW_PDC_FINISHING_TIME			= 11,
	E_ROW_PDC_TIME_OFFSET				= 12,
	E_ROW_PDC_SERIES					= 13,
	E_ROW_ORIGIN_MODIFIER				= 16,
	E_ROW_ACTIVE_OBJECT_INVOCATION	= 17,
	E_ROW_ADAPTIVE_OBJECT_INVOCATION	= 18,
	E_ROW_PASSIVE_OBJECT_INVOCATION	= 19,
	E_ROW_ACTIVE_OBJECT_DEFINITION		= 21,
	E_ROW_ADATIVE_OBJECT_DEFINITION	= 22,
	E_ROW_PASSIVE_OBJECT_DEFINITION	= 23,
	E_ROW_DRCS_MODE					= 24,
	E_ROW_TERMINATION_MARKER			= 31

}E_ROW_ADDRESS_GROUP;

typedef enum
{
	E_COL_FOREGROUND_COLOUR								= 0,
	E_COL_BLOCK_MOSAIC_CHARACTER							= 1,
	E_COL_LINE_DRAWING_OR_SMOOTHED_MOSAIC_CHARACTER	= 2,
	E_COL_BACKGROUND_COLOUR								= 3,
	E_COL_TIME_MINUTES										= 6,
	E_COL_ADDITIONAL_FLASH_FUNCTIONS						= 7,
	E_COL_MODIFIED_CHARACTER_SET_DESIGN					= 8,
	E_COL_CHARACTER_G0_SET								= 9,
	E_COL_CHARACTER_G3_SET								= 11,
	E_COL_DISPLAY_ATTRIBUTES								= 12,
	E_COL_DRCS_CHARACTER_INVOCATION						= 13,
	E_COL_FONT_STYLE										= 14,
	E_COL_CHARACTER_G2_SET								= 15,
	E_COL_G0_CHARACTER_WITHOUT_DIACRITICAL_MARK			= 16,
	E_COL_G0_CHARACTER_WITH_DIACRITICAL_MARK				= 17
	
}E_COL_ADDRESS_GROUP;
#endif

typedef struct
{
	unsigned char	ucAddress;
	unsigned char	ucMode;
	unsigned char	ucData;

//	unsigned char	ucFullScreen_Colour;
	
}ADDRESS_TRIPLETS;

typedef struct
{
	ADDRESS_TRIPLETS	stAddress_Triplets[_DEF_LEN_TRIPLET_];

}ENHANCEMENT_X_26;

typedef struct
{
	unsigned char	pData[_DEF_LEN_TRIPLET_][3];
	
}ENHANCEMENT_TRIPLET;


/***************************************************************************************************************************************
*
*  Item : EDITORIAL_LINKING		(Format of packets X/27/0~3 for editorial links)
*
* Usage : Contains the Page linking
*
*
*		
*												|     Magazine	| Packet		|
*												 \___Number	| Number (0)	|
*							Bytes 1-2			3	|		4		5	|
*							--------------------------------------------									     -----------
*							|Clock		| Framing	|			|		|									     |	Link		  |
*							| Run-In		| Code		| Packet Address		|									     |	Control	  |
*							--------------------------------------------									     -----------
*					_______/						  __________________/									      \		 /
*				  __/			  ___________________/															|		|
*				 /	Bytes 1 - 5	/	6		7 - 12		13 - 18		19 - 24		25 - 30		31 - 36		37 - 42	|	43	|	44
*				----------------------------------------------------------------------------------------------------------------
*	X/27/0 - 3	|	Prefix		|	DC	|	Link 0	|	Link 1	|	Link 2	|	Link 3	|	Link 4	|	Link 5	|	LC	|	CRC	|
*				----------------------------------------------------------------------------------------------------------------
*								|		|_					  ______________/			 \______________
*							      /		    \		    __________/											\_____________
*							    /			      \	  /																	    \
*							   |	Byte 6		|	/  Byte 6N+7		6N + 8		6N + 9		6N + 10		6N + 11		6N + 12 \
*							   ---------------	------------------------------------------------------------------------
*							   |	Designation	|	|Page		|Page		|Subcode	|Subcode	|Subcode	|Subcode	|
*							   |	Code (0 - 3)	|	|Number		|Number		|S1			|S2			|S3			|S4			|
*							   ---------------	|Units		|Tens		|			|+ M1		|			|+ M2, M3	|
*												------------------------------------------------------------------------
*
****************************************************************************************************************************************/
#define	_DEF_MAGAZIN_M1_	(0x08)
#define	_DEF_MAGAZIN_M2_	(0x0C)
typedef struct
{
	unsigned char		ucPage_Number;
	unsigned short	usPage_SubCode[4];
	unsigned char		ucMagazine_Number;
	
}HEADER_DATA;

typedef enum
{
	E_DESIGNATION_CODE_0_	= 0,
	E_DESIGNATION_CODE_1_,
	E_DESIGNATION_CODE_2_,
	E_DESIGNATION_CODE_3_,
	E_DESIGNATION_CODE_4_,
	E_DESIGNATION_CODE_5_,
	E_DESIGNATION_CODE_6_,
	E_DESIGNATION_CODE_7_
	
}E_DESIGNATION_CODE_TYPE;

#define	_DEF_LINK_MAX_		(06)
#define	_DEF_LEN_LINK_DATA_	(06)
typedef struct
{
	HEADER_DATA	stHeader_Data[_DEF_LINK_MAX_];
	
	unsigned char		ucLink_Control;
	//unsigned char		ucCRC;
	
}EDITORIAL_LINKING;


/******************************************************************************************************************************
*
*  Item : COMPOSITIONAL_LINKING		(Format of Format 1 packets X/27/4 and X/27/ for compositional linking)
*
* Usage : Format1 - for compositional linking in presentation enhancement applications
*
*
*		
*												|     Magazine	| Packet		|
*												 \___Number	| Number (0)	|
*							Bytes 1-2			3	|		4		5	|
*							--------------------------------------------
*							|Clock		| Framing	|			|		|
*							| Run-In		| Code		| Packet Address		|
*							--------------------------------------------
*					_______/						  __________________/
*				  __/			  ___________________/					
*				 /	Bytes 1 - 5	/	6	   7 - 9	   10 - 12	   13 - 15	   16 - 18											43 - 45
*				-------------------------------------------------------- - - - - - -------------------- - - - - -----------------
*	X/27/4 - 7	|	Prefix		|	DC	|	   Link 0		|	   Link 1		|		|		Link M		|		|	Reserved		|
*				|				|		|Triplet1	| Triplet2	|Triplet3	| Triplet4	|		|Triplet N	| Triplet N+1	|		|	Triplet 13		|
*				-------------------------------------------------------- - - - - - -------------------- - - - - - - --------------
*								|		|_								_______/		 \_______
*							      /		    \					    _________/						\__________
*							    /			      \				  /											    \
*							   |	Byte 6		|				/	Byte 3N+4			3N + 5			3N + 6   \
*							   ---------------				------------------------------------------------
*							   |	Designation	|				|				|				|				|
*							   |	Code (0 - 3)	|				|	Triplet N (N = 1 to 13)	Hamming (24/18) coded	|
*							   ---------------				|				|				|				|
*															------------------------------------------------
*
*******************************************************************************************************************************/
/*enum
{
	E_STANDARD_FORMAT	= 0x00,
	E_REFORMATTED_DATA	= 0x05,
	E_FORMAT_EXTENSION	= 0x06,
	E_NO_LINKED_PAGE	= 0x3F
	
}E_PAGE_FUNCTION_TYPE;

enum
{
	E_
}E_PAGE_CODING_TYPE;*/

#define	_DEF_MASK_LINK_CONTROL_DATA_2_	(0x40)
#define	_DEF_MASK_LINK_CONTROL_DATA_3_	(0x01)

#define	_DEF_MASK_LINK_FUNCTION_			(0x03)
#define	_DEF_MASK_PAGE_VALIDITY_			(0x0C)

#define	_DEF_MASK_PAGE_UNITS_				(0x3C)
#define	_DEF_MASK_PAGE_TENS_				(0x78)

#define	_DEF_MASK_SUBCODE_FLAGS_1_		(0x0C)
#define	_DEF_MASK_SUBCODE_FLAGS_2_		(0x7F)
#define	_DEF_MASK_SUBCODE_FLAGS_3_		(0x7F)

#define	_DEF_MASK_PAGE_SUBCODE_S1_		(0x78)	/*	15-18	[2] 111 1000	*/
#define	_DEF_MASK_PAGE_SUBCODE_S2_		(0x07)	/*	12-14	[2] 000 0111	*/
#define	_DEF_MASK_PAGE_SUBCODE_S3_		(0x78)	/*	8-11	[1] 111 1000	*/
#define	_DEF_MASK_PAGE_SUBCODE_S4_		(0x06)	/*	6-7		[1] 000 0110	*/

typedef struct
{
	unsigned char		ucPage_Function;
	unsigned char		ucPage_Coding;
	unsigned char		ucLink_Type;

	unsigned char		ucRelative_Magazine_Number;
	unsigned char		ucPage_Number;
	unsigned char		ucSub_code[4];
	
}DATA_BROADCASTING;


#define	_DEF_LINK_CONTROL_DATA_RESERVED_	(0xFF)

#define	_DEF_MASK_PAGE_FUNCTION_1_			(0x0F)
#define	_DEF_MASK_PAGE_FUNCTION_2_			(0x03)

#define	_DEF_MASK_PAGE_CODING_			(0x0C)
#define	_DEF_MASK_LINK_TYPE_				(0x30)

#define	_DEF_MASK_PAGE_UNITS_1_			(0x0E)
#define	_DEF_MASK_PAGE_UNITS_2_			(0x01)

typedef struct
{
	unsigned char		ucLink_Function;
	unsigned char		ucPage_Validity;
	
	unsigned char		ucMagazine_Number;
	unsigned char		ucPage_Number;
	unsigned short	usPage_subcode_flags;
	unsigned char		ucLink_function_flags;
	
}PRESENTATION_ENHANCEMENT;

typedef union
{
	PRESENTATION_ENHANCEMENT	stPresentation_enhancement;
	DATA_BROADCASTING			stData_broadcasting;
	
}TRIPLET_DATA;

//#define	_DEF_TRIPLET_MAX_		(13)
#define	_DEF_LEN_TRIPLET_DATA_	(03)
typedef struct
{
	unsigned char		ucLinkControlData2;
	unsigned char		ucLinkControlData3;
	
	TRIPLET_DATA	unTriplet_Data[_DEF_LINK_MAX_];
	
}COMPOSITIONAL_LINKING;


/******************************************************************************************************************************
*
*  Item : BROADCAST_SERVICE		(Coding of Packet 8/30)
*
* Usage : Packet 8/30
*
*
*		
*											|     Magazine	| Packet			|
*											 \___Number	| Number (30)	|
*						Bytes 1-2			3	|	(8)	4		5		|
*						-------------------------------------------------
*						|Clock		| Framing	|			|			|
*						| Run-In		| Code		|	Packet Address		|
*						-------------------------------------------------
*				_______/							  __________________/
*			  __/			  _______________________/					
*			 /	Bytes 1 - 5	/  6			7 -12						13 - 25						26 - 45
*			---------------------------------------------------------------------------------------------
*	8/30	|	Prefix		|DC	|	Initial Page	|		Format 1  / Format 2			|	Status Display		|
*	format 1	|				|	|				|									|					|
*			---------------------------------------------------------------------------------------------
*						 ___|     /\__				 \_______________________				
*					____/	 __/	      \____								 \__________________
*			     ____/			/			\___													\______________________
*			    /		Byte 6	|				\																	     \
*			    --------------				 ------------------------------------------------------------------------
*			    |	Designation	|				 |Page		|Page		|Subcode	|Subcode	|Subcode	|Subcode	|
*			    |	Code (0, 1)	|				 |Number	|Number		|S1			|S2			|S3			|S4			|
*			    --------------				 |Units		|Tens		|			|+ M1		|			|+ M2, M3	|
*											 ------------------------------------------------------------------------
*
*******************************************************************************************************************************/

/****************************************************************************
*
*  Item : BROADCAST_SERVICE_FORMAT_1		(Coding of Packet 8/30 Format 1)
*
* Usage : Bytes 13 - 25
*
*
*	Bytes		13-14		15		16-18		19-21		22-25
*		-------------------------------------------------------------
*		|	Network	|	Time	|	MJD		|	UTC		|Reserved	|
*		|	Ident	|	Offset	|			|			|			|	
*		-------------------------------------------------------------
*
*****************************************************************************/
typedef struct
{
	unsigned short			usNetwork_ID;
	unsigned char				ucTime_Offset;
	DATE_TIME_STRUCT		stTime_Data;
	
}BS_FORMAT_1;

/****************************************************************************
*
*  Item : BROADCAST_SERVICE_FORMAT_2		(Coding of Packet 8/30 Format 2)
*
* Usage : Bytes 13 - 25
*
*
*			Bytes				13 - 25
*		-------------------------------------------------
*		|			Programme Identification Data			|
*		|												|
*		-------------------------------------------------
*
*****************************************************************************/
#define	_DEF_LEN_PROGRAMME_IDENTIFICATION_		(13)
typedef struct
{
	unsigned char	ucChannel_ID;
	unsigned char	ucUpdate_Flag;
	unsigned char	ucPrepare_Flag;

	unsigned char	ucSound_Status;
	unsigned char	ucMode_ID;

	unsigned char	ucCountry1;
	unsigned char	ucNetwork1;

	unsigned char	ucDay;
	unsigned char	ucMonth;
	unsigned char	ucHour;
	unsigned char	ucMinute;
	
	unsigned char ucCountry2;
	unsigned char	ucNetwork2;

	unsigned char	ucProgramme_type;

}BS_FORMAT_2;

typedef union
{
	BS_FORMAT_1	stBS_Format1;
	BS_FORMAT_2	stBS_Format2;
		
}BS_FORMAT_DATA;

typedef struct
{
	HEADER_DATA	stHeader_Data;
	BS_FORMAT_DATA	unFormat_Data;
	unsigned char		ucStatus_Display[20];
	
}BROADCAST_SERVICE;

/*********************************************************************
*                                                                                                                       *
*  Item : DATA_PACKET_UNION                                                                           *
*                                                                                                                       *
* Usage : Contains the union of all data_packets structs                                         *
*                                                                                                                       *
*********************************************************************/
#define	_DEF_TTX_DATA_VALID_MASK		(0x7F)

typedef union
{
	PAGE_HEADER			stPage_Header;
	NORMAL_PACKET			stNormal_Packet;
	ENHANCEMENT_X_26		stEnhancement_X_26;
	ENHANCEMENT_TRIPLET	stEnhancement_Triplet;
	EDITORIAL_LINKING		stEditorial_Linking;
	COMPOSITIONAL_LINKING	stCompositional_Linking;
	BROADCAST_SERVICE		stBroadcast_Service;
	
} DATA_PACKET_UNION;


/*****************************************************************************
*
*  Item : EBU_TELETEXT
*
* Usage : Contains the Data_field for EBU Teletext
*
*
* 			MPEG Definition
*		================
*
* 		Syntax						No. of bits	Mnemonic
*	----------------------------	--------	--------
*	data_field(){
*		reserved_future_use				   2		bslbf
*		field_parity						   1		bslbf
*		line_offset						   5		uimsbf
*		framing_code						   8		bslbf
*		magazine_and_packet_address	 	  16		bslbf
*		data_block()						320		bslbf
*	}
*
*
*****************************************************************************/
#define	_DEF_TTX_FRAMING_CODE_			(0xE4)

#define	_DEF_TTX_MAGAZINE_MASK_		(0x07)
#define	_DEF_TTX_PACKET_ADDRESS_MASK	(0x01)

typedef struct
{
	unsigned char			ucResult;
	unsigned char			ucField_Parity;
	unsigned char			ucLine_Offset;
	unsigned char			ucMagazine;
	E_TTX_PACKET_TYPE	ucPacket_Address;

	unsigned char			ucDesignation_Code;
	
	DATA_PACKET_UNION	unData_Block;
	
}  EBU_TELETEXT;


/*********************************************************************
*                                                                                                                       *
*  Item : DATA_FIELD_STRUCT                                                                            *
*                                                                                                                       *
* Usage : Contains the union of all data_field structs                                              *
*                                                                                                                       *
*********************************************************************/
typedef union
{
	EBU_TELETEXT		stTTX;
	
} DATA_FIELD_UNION;


/*****************************************************************************
*
*	Item : PES_DATA_FIELD_STRUCT
*
*	Usage : Contains the PES data field Table
*
*
*			MPEG Definition
*		================
*
* 		Syntax						No. of bits	Mnemonic
*	----------------------------	--------	--------
*	PES_data_field(){
*		data_identifier						   8		uimsbf
*			for (i=0;i<N;i++){
*				data_unit_id				   8		uimsbf
*				data_unit_length			   8		uimsbf
*				data_field()
*			}
*		}
*	}
*
*  NOTE : The descriptors are chained in the order that they are found
*         in the raw MPEG data.
*
*****************************************************************************/
#define	_DEF_LEN_TTX_UNIT_			(0x2C)	/*	(8+8=16/8=2)+(2+1+5+8=16/8=2)+(320/8=40) = 44 = 0x2C	*/
typedef struct _pes_data_unit
{
	E_DATA_UNIT_ID_TYPE	ucData_Unit_ID;
	DATA_FIELD_UNION	unData_Field;

}PES_DATA_UNIT_STRUCT;

typedef struct	_pes_data_field_struct
{
	unsigned char				ucData_Identifier;
	unsigned char				ucCount_Unit;
	
	PES_DATA_UNIT_STRUCT	*pstPES_Data_Unit;

}  PES_DATA_FIELD_STRUCT;




extern int MPEGPARS_ParsePES_DataField(void *handle, void *pRawData, int uiLenData, PES_DATA_FIELD_STRUCT** ppTTX_Struct);
extern void MPEGPARS_PES_Free_DataField(PES_DATA_FIELD_STRUCT** ppFree_DataField);

extern int Get_Teletext_Packet_PageHeader(unsigned char *pData_reverse, PAGE_HEADER* strPage_Header);
extern int Get_Teletext_Packet_Normal(unsigned char *pData_reverse, NORMAL_PACKET* strNormal_Packet);
extern int Get_Teletext_Packet_PageEnhancementData(unsigned char *pData_reverse, EBU_TELETEXT *pEBU_Teletext);
extern int Get_Teletext_Packet_PageLinking(unsigned char *pData_reverse, EBU_TELETEXT *pEBU_Teletext);
extern int Get_Teletext_Packet_BroadcastService(unsigned char *pData_reverse, EBU_TELETEXT *pEBU_Teletext);
extern BOOL Get_Odd_Parity(unsigned char *pData, unsigned char ucLen);
extern unsigned char Get_Odd_Parity_Data(unsigned char ucData);
extern unsigned char Get_Hamming_8_4(unsigned char ucData);
extern void Get_Hamming_24_18(unsigned char *pData, unsigned char *pAccept_Data);




#endif	//_DVB_PARSE_H_


