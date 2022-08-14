/****************************************************************************
 *   FileName    : nand_list.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall 
constitute any express or implied warranty of any kind, including without limitation, any warranty 
of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright 
or other third party intellectual property right. No warranty is made, express or implied, 
regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of 
or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
 *
 ****************************************************************************/
#ifndef __NAND_LIST_H
#define __NAND_LIST_H

#include "nand_io_v8.h"

#define _ANY						((unsigned short)-2)

#if defined(NFC_VER_300)
#define A_ECC_40b_PER_1KB			(A_ECC_40BIT|A_1KB)
#elif defined(NFC_VER_200)
#define A_ECC_40b_PER_1KB			(A_ECC_24BIT|A_512B)
#elif defined(NFC_VER_50)
#define A_ECC_40b_PER_1KB			(A_ECC_12BIT|A_512B)
#else
#error
#endif

static const unsigned char s_ucLowerPageTable_128PpB_MLC[64] =
{
	0x00,
	0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F,
	0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F,
	0x21, 0x23, 0x25, 0x27, 0x29, 0x2B, 0x2D, 0x2F,
	0x31, 0x33, 0x35, 0x37, 0x39, 0x3B, 0x3D, 0x3F,
	0x41, 0x43, 0x45, 0x47, 0x49, 0x4B, 0x4D, 0x4F,
	0x51, 0x53, 0x55, 0x57, 0x59, 0x5B, 0x5D, 0x5F,
	0x61, 0x63, 0x65, 0x67, 0x69, 0x6B, 0x6D, 0x6F,
	0x71, 0x73, 0x75, 0x77, 0x79, 0x7B, 0x7D
//or
//	  0,
//	  1,   3,   5,   7,   9,
//	 11,  13,  15,  17,  19,
//	 21,  23,  25,  27,  29,
//	 31,  33,  35,  37,  39,
//	 41,  43,  45,  47,  49,
//	 51,  53,  55,  57,  59,
//	 61,  63,  65,  67,  69,
//	 71,  73,  75,  77,  79,
//	 81,  83,  85,  87,  89,
//	 91,  93,  95,  97,  99,
//	101, 103, 105, 107, 109,
//	111, 113, 115, 117, 119,
//	121, 123, 125
};

static const unsigned char s_ucLowerPageTable_256PpB_MLC[128] =
{
	0x00, 0x01,
	0x02, 0x03, 0x06, 0x07, 0x0a, 0x0b, 0x0e, 0x0f,
	0x12, 0x13, 0x16, 0x17, 0x1a, 0x1b, 0x1e, 0x1f,
	0x22, 0x23, 0x26, 0x27, 0x2a, 0x2b, 0x2e, 0x2f,
	0x32, 0x33, 0x36, 0x37, 0x3a, 0x3b, 0x3e, 0x3f,
	0x42, 0x43, 0x46, 0x47, 0x4a, 0x4b, 0x4e, 0x4f,
	0x52, 0x53, 0x56, 0x57, 0x5a, 0x5b, 0x5e, 0x5f,
	0x62, 0x63, 0x66, 0x67, 0x6a, 0x6b, 0x6e, 0x6f,
	0x72, 0x73, 0x76, 0x77, 0x7a, 0x7b, 0x7e, 0x7f,
	0x82, 0x83, 0x86, 0x87, 0x8a, 0x8b, 0x8e, 0x8f,
	0x92, 0x93, 0x96, 0x97, 0x9a, 0x9b, 0x9e, 0x9f,
	0xa2, 0xa3, 0xa6, 0xa7, 0xaa, 0xab, 0xae, 0xaf,
	0xb2, 0xb3, 0xb6, 0xb7, 0xba, 0xbb, 0xbe, 0xbf,
	0xc2, 0xc3, 0xc6, 0xc7, 0xca, 0xcb, 0xce, 0xcf,
	0xd2, 0xd3, 0xd6, 0xd7, 0xda, 0xdb, 0xde, 0xdf,
	0xe2, 0xe3, 0xe6, 0xe7, 0xea, 0xeb, 0xee, 0xef,
	0xf2, 0xf3, 0xf6, 0xf7, 0xfa, 0xfb
//or
//	  0,   1,
//	  2,   3,   6,   7,  10,  11,  14,  15,  18,  19,
//	 22,  23,  26,  27,  30,  31,  34,  35,  38,  39,
//	 42,  43,  46,  47,  50,  51,  54,  55,  58,  59,
//	 62,  63,  66,  67,  70,  71,  74,  75,  78,  79,
//	 82,  83,  86,  87,  90,  91,  94,  95,  98,  99,
//	102, 103, 106, 107, 110, 111, 114, 115, 118, 119,
//	122, 123, 126, 127, 130, 131, 134, 135, 138, 139,
//	142, 143, 146, 147, 150, 151, 154, 155, 158, 159,
//	162, 163, 166, 167, 170, 171, 174, 175, 178, 179,
//	182, 183, 186, 187, 190, 191, 194, 195, 198, 199,
//	202, 203, 206, 207, 210, 211, 214, 215, 218, 219,
//	222, 223, 226, 227, 230, 231, 234, 235, 238, 239,
//	242, 243, 246, 247, 250, 251
};

static const unsigned char s_ucLowerPageTable_256PpB_MLC_Type2[128] =
{
	  0,   1,
	  2,   3,   4,   5,   8,   9,  12,  13,  16,  17,
	 20,  21,  24,  25,  28,  29,  32,  33,  36,  37,
	 40,  41,  44,  45,  48,  49,  52,  53,  56,  57,
	 60,  61,  64,  65,  68,  69,  72,  73,  76,  77,
	 80,  81,  84,  85,  88,  89,  92,  93,  96,  97,
	100, 101, 104, 105, 108, 109, 112, 113, 116, 117,
	120, 121, 124, 125, 128, 129, 132, 133, 136, 137,
	140, 141, 144, 145, 148, 149, 152, 153, 156, 157,
	160, 161, 164, 165, 168, 169, 172, 173, 176, 177,
	180, 181, 184, 185, 188, 189, 192, 193, 196, 197,
	200, 201, 204, 205, 208, 209, 212, 213, 216, 217,
	220, 221, 224, 225, 228, 229, 232, 233, 236, 237,
	240, 241, 244, 245, 248, 249/*, 252, 253,
	254, 255*/
};

static const NAND_IO_FEATURE TOSHIBA_NAND_DevInfo[] =
{
	//*=================================================================================================================================================================
	//*[           DEVICE CODE               ][              SIZE               ][               Cycle                  ][                  ATTRIBUTE                  ]
	//*-----------------------------------------------------------------------------------------------------------------------------------------------------------------
	//*  1st,  2nd,  3rd,  4th,  5th,  6th,     BpD,DDP,BBpD, PpB,  Page,Spare,  Col,Row, tWC,tWP,tWH, tRC,tREA,tRP,tREH,  MediaType, UsableFunc, LSBPageTable
	//*=================================================================================================================================================================
	{ {"TC58NVG0S3HTAI0"/*128MB*/, NULL, NULL},
	  {{0x98, 0xF1, 0x80, 0x15, 0x72, 0x16}},  1024,  0,  20,  64,  2048,  128,    2,  2,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_8BIT|A_512B), (F_CP|F_CR), NULL },
	{ {"TC58NVG1S3ETAI0"/*256MB*/, NULL, NULL},
	  {{0x98, 0xDA, 0x90, 0x15, 0x76, 0x14}},  2048,  0,  40,  64,  2048,   64,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_CP|F_MP|F_CR), NULL },
	{ {"TC58NVG1S3HTAI0"/*256MB*/, NULL, NULL},
	  {{0x98, 0xDA, 0x90, 0x15, 0x76, 0x16}},  2048,  0,  40,  64,  2048,  128,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_8BIT|A_512B), (F_CP|F_MP|F_CR), NULL },
	{ {"TC58NVG3S0F"/*1GB*/, "TH58NVG4S0F"/*2GB*/, NULL},
	  {{0x98, 0xD3, 0x90, 0x26, 0x76, _ANY}},  4096,  0,  80,  64,  4096,  232,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_CP|F_MP|F_CR|F_EDO), NULL },
	{ {"TC58NVG4D2H"/*2GB*/, NULL, NULL},
	  {{0x98, 0xD5, 0x84, 0x32, 0x72, 0x56}},  2056,  0,  58, 128,  8192,  640,    2,  3,  20, 10,  7,  20,  16, 10,   7, (A_08BIT|A_ECC_24BIT|A_1KB|A_RAND_IO), (F_CP|F_CR|F_EDO), s_ucLowerPageTable_128PpB_MLC },
	{ {"TC58NVG5D2H"/*4GB*/, NULL, NULL},
	  {{0x98, 0xD7, 0x94, 0x32, 0x76, 0x56}},  4116,  0,  80, 128,  8192,  640,    2,  3,  20, 10,  7,  20,  16, 10,   7, (A_08BIT|A_ECC_24BIT|A_1KB|A_RAND_IO), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_128PpB_MLC },
	{ {"TC58NVG6D2G"/*8GB*/, NULL, NULL},
	  {{0x98, 0xDE, 0x94, 0x82, 0x76, 0x56}},  4124,  0, 128, 256,  8192,  640,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_24BIT|A_1KB|A_RAND_IO), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC },
	  // Toggle DDR NAND
	{ {"TC58TEG5DCJ"/*4GB*/, NULL, NULL},
	  {{0x98, 0xD7, 0x84, 0x93, 0x72, 0x57}},  1060,  0,  59, 256, 16384, 1280,    2,  3,  20, 10, 17,  20,  16, 10,   7, (A_08BIT|A_ECC_40BIT|A_1KB|A_RAND_IO), (F_CP|F_CR), s_ucLowerPageTable_256PpB_MLC },
	{ {NULL, "TH58TEG7DCJ"/*16GB*/,NULL},
	  {{0x98, 0xDE, 0x84, 0x93, 0x72, 0x57}},  2092,  0, 148, 256, 16384, 1280,    2,  3,  25, 11, 11,  20,  16, 10,   7, (A_08BIT|A_ECC_40BIT|A_1KB|A_RAND_IO), (F_CP|F_CR), s_ucLowerPageTable_256PpB_MLC },	  	

	// SMART NAND
	{ {"THGVR1G5D1H"/*4GB*/, NULL, NULL},
	  {{0x98, 0xD7, 0x94, 0x32, 0xF4, 0x56}},  4116,  0,  90, 128,  8192,   32,    2,  3,  30, 12, 10,  30,  25, 12,  10, (A_08BIT|A_ECC_EMBEDDED), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_128PpB_MLC },
	{ {"THGVR1G6D1G"/*8GB*/, NULL, NULL},
	  {{0x98, 0xDE, 0x94, 0x82, 0xF4, 0x56}},  4124,  0, 138, 256,  8192,   32,    2,  3,  30, 12, 10,  30,  25, 12,  10, (A_08BIT|A_ECC_EMBEDDED), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC },
};

static const NAND_IO_FEATURE HYNIX_NAND_DevInfo[] =
{
	//*=================================================================================================================================================================
	//*[           DEVICE CODE               ][              SIZE               ][               Cycle                  ][                  ATTRIBUTE                  ]
	//*-----------------------------------------------------------------------------------------------------------------------------------------------------------------
	//*  1st,  2nd,  3rd,  4th,  5th,  6th,     BpD,DDP,BBpD, PpB,  Page,Spare,  Col,Row, tWC,tWP,tWH, tRC,tREA,tRP,tREH,  MediaType, UsableFunc, LSBPageTable
	//*=================================================================================================================================================================
	{ {"H27U1G8F2B"/*128MB*/, NULL, NULL},
	  {{0xAD, 0xF1, 0x00, 0x1D, _ANY, _ANY}},  1024,  0,  20,  64,  2048,   64,    2,  2,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_CR), NULL },
	{ {"H27U2G8F2C"/*256MB*/, NULL, NULL},
	  {{0xAD, 0xDA, 0x90, 0x95, 0x44, _ANY}},  2048,  0,  40,  64,  2048,   64,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_MP|F_CR), NULL },
	{ {"H27U4G8F2D"/*512MB*/, NULL, NULL},
	  {{0xAD, 0xDC, 0x90, 0x95, 0x54, _ANY}},  4096,  0,  80,  64,  2048,   64,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_MP|F_CP|F_CR|F_EDO), NULL },
	{ {"H27UAG8T2B"/*2GB*/, NULL, NULL},
	  {{0xAD, 0xD5, 0x94, 0x9A, 0x74, 0x42}},  1024,  0,  25, 256,  8192,  448,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_24BIT|A_1KB), (F_MP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC },
	{ {"H27UBG8T2A"/*4GB*/, "H27UCG8U5A"/*8GB*/, NULL},
	  {{0xAD, 0xD7, 0x94, 0x9A, 0x74, 0x42}},  2048,  0,  50, 256,  8192,  448,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_24BIT|A_1KB), (F_MP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC },
	{ {"H27UBG8T2B"/*4GB*/, NULL, NULL}, // 26nm 
	  {{0xAD, 0xD7, 0x94, 0xDA, 0x74, 0xC3}},  2048,  0,  48, 256,  8192,  640,    2,  3,  20,  8, 10,  20,  16, 10,  10, (A_08BIT|A_ECC_24BIT|A_1KB|A_RAND_IO|A_RR_H_26NM), (F_MP|F_CP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC },
  	{ {"H27UCG8T2A"/*8GB*/, NULL, NULL}, //20nm
	  {{0xAD, 0xDE, 0x94, 0xDA, 0x74, 0xC4}},  4180,  0,  120, 256,  8192,  640,    2,  3,  20,  12, 10,  20,  16, 12,  12, (A_08BIT|A_ECC_40BIT|A_1KB|A_ECC_SPARE_24BIT|A_RAND_IO|A_RR_H_20NM), (F_MP|F_CP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC },	
	{ {"H27UBG8T2C"/*4GB*/, NULL, NULL}, //20nm
	  {{0xAD, 0xD7, 0x94, 0x91, 0x60, 0x44}},  2092,  0,   68, 256,  8192,  640,    2,  3,  20,  12, 10,  20,  16, 12,  12, (A_08BIT|A_ECC_40BIT|A_1KB|A_ECC_SPARE_24BIT|A_RAND_IO|A_RR_H_20NM), (F_MP|F_CP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC },	  	
};

static const NAND_IO_FEATURE SAMSUNG_NAND_DevInfo[] =
{
	//*=================================================================================================================================================================
	//*[           DEVICE CODE               ][              SIZE               ][               Cycle                  ][                  ATTRIBUTE                  ]
	//*-----------------------------------------------------------------------------------------------------------------------------------------------------------------
	//*  1st,  2nd,  3rd,  4th,  5th,  6th,     BpD,DDP,BBpD, PpB,  Page,Spare,  Col,Row, tWC,tWP,tWH, tRC,tREA,tRP,tREH,  MediaType, UsableFunc, LSBPageTable
	//*=================================================================================================================================================================
	{ {"K9F1G08U0D"/*128MB*/, NULL, NULL},
	  {{0xEC, 0xF1, 0x00, 0x95, 0x40, _ANY}},  1024,  0,  20,  64,  2048,   64,    2,  2,  30, 15, 10,  30,  20, 15,  10, (A_08BIT|A_ECC_4BIT|A_512B), (0), NULL },
	{ {"K9F2G08U0C"/*256MB*/, NULL, NULL},
	  {{0xEC, 0xDA, 0x10, 0x95, 0x44, _ANY}},  2048,  0,  40,  64,  2048,   64,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (0), NULL },
	{ {"K9F4G08U0D"/*512MB*/, NULL, NULL},
	  {{0xEC, 0xDC, 0x10, 0x95, 0x54, _ANY}},  4096,  0,  80,  64,  2048,   64,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_MP), NULL },
	{ {"K9GAG08U0E"/*2GB*/, NULL, NULL},
	  {{0xEC, 0xD5, 0x84, 0x72, 0x50, 0x42}},  2076,  0,  58, 128,  8192,  436,    2,  3,  30, 15, 10,  30,  25, 15,  10, (A_08BIT|A_ECC_24BIT|A_1KB), (F_CP|F_CR|F_EDO), s_ucLowerPageTable_128PpB_MLC },
	{ {"K9GAG08U0F"/*2GB*/, NULL, NULL},
	  {{0xEC, 0xD5, 0x94, 0x76, 0x54, 0x43}},  2076,  0,  58, 128,  8192,  512,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_24BIT|A_1KB), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_128PpB_MLC },
	{ {"K9GAG08U0M"/*2GB*/, NULL, NULL},
	  {{0xEC, 0xD5, 0x14, 0xB6, 0x74, _ANY}},  4096,  0, 100, 128,  4096,  128,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_MP|F_EDO), NULL },
	{ {"K9GBG08U0A"/*4GB*/, NULL, NULL},
	  {{0xEC, 0xD7, 0x94, 0x7A, 0x54, 0x43}},  4152,  0, 116, 128,  8192,  640,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_24BIT|A_1KB|A_RAND_IO), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_128PpB_MLC },
	{ {"K9GBG08U0B"/*4GB*/, NULL, NULL},
	  {{0xEC, 0xD7, 0x94, 0x7E, 0x64, 0x44}},  4096,  0, 102, 128,  8192, 1024,    2,  3,  25, 11, 11,  25,  20, 11,  11, (A_08BIT|A_ECC_40b_PER_1KB|A_RAND_IO|A_RR_S), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_128PpB_MLC },
	{ {"K9LCG08U0A"/*8GB*/, NULL, NULL},
	  {{0xEC, 0xDE, 0xD5, 0x7A, 0x58, 0x43}},  4152, 20, 116, 128,  8192,  640,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_24BIT|A_1KB|A_RAND_IO), (F_CP|F_MP|F_CR|F_EDO|F_IL), s_ucLowerPageTable_128PpB_MLC },
	{ {"K9LCG08U0B"/*8GB*/, NULL, NULL},
	  {{0xEC, 0xDE, 0xD5, 0x7E, 0x68, 0x44}},  8192,  0, 204, 128,  8192, 1024,    2,  3,  25, 12, 11,  25,  20, 12,  11, (A_08BIT|A_ECC_40b_PER_1KB|A_RAND_IO|A_RR_S), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_128PpB_MLC },
};

static const NAND_IO_FEATURE MICRON_NAND_DevInfo[] =
{
	//*=================================================================================================================================================================
	//*[           DEVICE CODE               ][              SIZE               ][               Cycle                  ][                  ATTRIBUTE                  ]
	//*-----------------------------------------------------------------------------------------------------------------------------------------------------------------
	//*  1st,  2nd,  3rd,  4th,  5th,  6th,     BpD,DDP,BBpD, PpB,  Page,Spare,  Col,Row, tWC,tWP,tWH, tRC,tREA,tRP,tREH,  MediaType, UsableFunc, LSBPageTable
	//*=================================================================================================================================================================
	{ {"MT29F4G08ABADA"/*512MB*/, NULL, NULL},
      {{0x2C, 0xDC, 0x90, 0x95, 0x56, _ANY}},  4096,  0,  80,  64,  2048,   64,    2,  3,  20, 10,  7,  20,  16, 10,   7, (A_08BIT|A_ECC_4BIT|A_512B), (F_CP|F_MP|F_CR|F_EDO), NULL },
	{ {"MT29F8G08ABABA"/*1GB*/, NULL, NULL},
      {{0x2C, 0x38, 0x00, 0x26, 0x85, _ANY}},  2048,  0,  40, 128,  4096,  224,    2,  3,  25, 15, 10,  25,  20, 15,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_CP|F_MP|F_CR|F_EDO), NULL },
	{ {"MT29F16G08CBACA"/*2GB*/, NULL, NULL},
	  {{0x2C, 0x48, 0x04, 0x4A, 0xA5, 0x00}},  2048,  0,  50, 256,  4096,  224,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_24BIT|A_1KB|A_ECC_SPARE_16BIT), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC },
	{ {"MT29F32G08CBACA"/*4GB*/, NULL, NULL},
	  {{0x2C, 0x68, 0x04, 0x4A, 0xA9, 0x00}},  4096,  0, 100, 256,  4096,  224,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_24BIT|A_1KB|A_ECC_SPARE_16BIT), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC },
	{ {"MT29F32G08CBADA"/*4GB*/, NULL, NULL},
	  {{0x2C, 0x44, 0x44, 0x4B, 0xA9, 0x00}},  2128,  0,  74, 256,  8192,  744,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_40b_PER_1KB|A_RR_M), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC_Type2 },
	{ {"MT29F64G08CBAAA"/*8GB*/, NULL, NULL},
	  {{0x2C, 0x88, 0x04, 0x4B, 0xA9, 0x00}},  4096,  0, 100, 256,  8192,  448,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_24BIT|A_1KB), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC },
	{ {"MT29F64G08CBABA"/*8GB*/, NULL, NULL},
	  {{0x2C, 0x64, 0x44, 0x4B, 0xA9, 0x00}},  4096,  0, 100, 256,  8192,  744,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_40b_PER_1KB|A_RR_M), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC_Type2 },
	{ {NULL, "MT29F256G08CJAAA"/*32GB*/, NULL},
	  {{0x2C, 0xA8, 0x05, 0xCB, 0xA9, 0x00}},  8192,  0, 200, 256,  8192,  448,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_24BIT|A_1KB), (F_CP|F_MP|F_CR|F_EDO), s_ucLowerPageTable_256PpB_MLC },
};

static const NAND_IO_FEATURE SPANSION_NAND_DevInfo[] =
{
	//*=================================================================================================================================================================
	//*[           DEVICE CODE               ][              SIZE               ][               Cycle                  ][                  ATTRIBUTE                  ]
	//*-----------------------------------------------------------------------------------------------------------------------------------------------------------------
	//*  1st,  2nd,  3rd,  4th,  5th,  6th,     BpD,DDP,BBpD, PpB,  Page,Spare,  Col,Row, tWC,tWP,tWH, tRC,tREA,tRP,tREH,  MediaType, UsableFunc, LSBPageTable
	//*=================================================================================================================================================================
	{ {"S34ML01G1"/*128MB*/, NULL, NULL},
	  {{0x01, 0xF1, 0x00, 0x1D, _ANY, _ANY}},  1024,  0, 20, 64,  2048,  64,    2,  2,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_CR), NULL },
	{ {"S34ML02G1"/*256MB*/, NULL, NULL},
	  {{0x01, 0xDA, 0x90, 0x95, 0x44, _ANY}},  2048,  0, 40, 64,  2048,  64,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_CP|F_MP|F_CR), NULL },
  	{ {"S34ML04G1"/*512MB*/, NULL, NULL},
	  {{0x01, 0xDC, 0x90, 0x95, 0x54, _ANY}},  4096,  0, 80, 64,  2048,  64,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_CP|F_MP|F_CR), NULL },
};

static const NAND_IO_FEATURE FIDELIX_NAND_DevInfo[] =
{
	//*=================================================================================================================================================================
	//*[           DEVICE CODE               ][              SIZE               ][               Cycle                  ][                  ATTRIBUTE                  ]
	//*-----------------------------------------------------------------------------------------------------------------------------------------------------------------
	//*  1st,  2nd,  3rd,  4th,  5th,  6th,     BpD,DDP,BBpD, PpB,  Page,Spare,  Col,Row, tWC,tWP,tWH, tRC,tREA,tRP,tREH,  MediaType, UsableFunc, LSBPageTable
	//*=================================================================================================================================================================
	{ {"FMND1G08U3A"/*128MB*/, NULL, NULL},
	  {{0xF8, 0xF1, 0x80, 0x95, 0x40, _ANY}},  1024,  0, 20, 64,  2048,  64,    2,  2,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_CP), NULL },
};

const NAND_IO_FEATURE   MACRONIX_NAND_DevInfo[] =
{
   //*=======================================================================================================================================================
    //*[        DEVICE CODE       ][           SIZE            ][               Cycle                     ][                         ATTRIBUTE                 ]
    //*-------------------------------------------------------------------------------------------------------------------------------------------------------
    //*  1st,  2nd,  3rd,  4th,  5th,  6th,     BpD,DDP,BBpD, PpB,  Page,Spare,  Col,Row, tWC,tWP,tWH, tRC,tREA,tRP,tREH,  MediaType, UsableFunc, LSBPageTable
    //*=======================================================================================================================================================
    { {"MX30LF1G08AA"/*128MB*/, NULL, NULL},
     {{0xC2, 0xF1, 0x80, 0x1D, _ANY, _ANY}}, 	1024,  0, 20,  64,   2048, 64,    2,   2,   30, 15, 10,  30, 20, 15, 10, (A_08BIT|A_ECC_4BIT|A_512B), (F_CP), NULL },

};

const NAND_IO_FEATURE   EON_NAND_DevInfo[] =
{
	//*=================================================================================================================================================================
	//*[           DEVICE CODE               ][              SIZE               ][               Cycle                  ][                  ATTRIBUTE                  ]
	//*-----------------------------------------------------------------------------------------------------------------------------------------------------------------
	//*  1st,  2nd,  3rd,  4th,  5th,  6th,     BpD,DDP,BBpD, PpB,  Page,Spare,  Col,Row, tWC,tWP,tWH, tRC,tREA,tRP,tREH,  MediaType, UsableFunc, LSBPageTable
	//*=================================================================================================================================================================
	{ {"EN27LN1G08/F59L1G81A"/*128MB*/, NULL, NULL},
	  {{0x92, 0xF1, 0x80, 0x95, 0x40, _ANY}},  1024,  0, 20, 64,  2048,  64,    2,  2,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_CP), NULL }, 	/* 6th ID 도 0x7F로 동일함 */
};

const NAND_IO_FEATURE   EMST_NAND_DevInfo[] =
{
	//*=================================================================================================================================================================
	//*[           DEVICE CODE               ][              SIZE               ][               Cycle                  ][                  ATTRIBUTE                  ]
	//*-----------------------------------------------------------------------------------------------------------------------------------------------------------------
	//*  1st,  2nd,  3rd,  4th,  5th,  6th,     BpD,DDP,BBpD, PpB,  Page,Spare,  Col,Row, tWC,tWP,tWH, tRC,tREA,tRP,tREH,  MediaType, UsableFunc, LSBPageTable
	//*=================================================================================================================================================================
	{ {"F59L2G81A"/*256MB*/, NULL, NULL},
	  {{0xC8, 0xDA, 0x90, 0x95, 0x44, _ANY}},  2048,  0, 40,  64,   2048,  64,    2,  3,  25, 12, 10,  25,  20, 12,  10, (A_08BIT|A_ECC_4BIT|A_512B), (F_CP|F_CR|F_MP), NULL },  	
};

#endif	/* __NAND_LIST_H */
