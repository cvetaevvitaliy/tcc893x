/*
 *           Copyright 2012 Availink, Inc.
 *
 *  This software contains Availink proprietary information and
 *  its use and disclosure are restricted solely to the terms in
 *  the corresponding written license agreement. It shall not be 
 *  disclosed to anyone other than valid licensees without
 *  written permission of Availink, Inc.
 *
 */


///$Date: 2012-3-8 21:47 $
///
#ifndef AV2011_h_h
	#define AV2011_h_h

	#include "II2CRepeater.h"
	#include "ITuner.h"

	#ifdef AVL_CPLUSPLUS
extern "C" {
	#endif
	typedef  unsigned char UINT8;

	typedef  short AVL_int16;
	typedef  unsigned short UINT16;

	typedef  int INT32;
	typedef  unsigned int UINT32;

	#define AV2011_ENABLE_TIMEOUT
	#define AV2011_TIMEOUT 100

	/// AV2011_Setting: Structure for AV2011 special setting
	struct AV2011_Setting{
		AVL_uchar AutoGain;
		AVL_uchar Single_End;
		AVL_uchar Auto_Scan;
		AVL_uchar RF_Loop;

	};

	enum AutoGain
	{
		AV2011_AutoGain_OFF = 0, //FT_EN 0, fix LNA gain 
		AV2011_AutoGain_ON = 1 //FT_EN 0, LNA gain Auto control
	};

	enum Single_End
	{
		AV2011_Differential = 0,  //IQ Differential mode
		AV2011_SingleEnd = 1    //IQ Single end mode
	};

	enum Auto_Scan
	{
		Auto_Scan_OFF = 0,  //0 = normal manual lock mode
		Auto_Scan_ON = 1 //1 = blind scan search mode
	};

	enum RF_Loop
	{
		RF_Loop_OFF = 0,  //0 = RF loop through off
		RF_Loop_ON = 1 //1 = RF loop through on
	};

	// time delay function ( minisecond )
	AVL_DVBSx_ErrorCode AVL_DVBSx_ExtAV2011_Initialize(struct AVL_Tuner * pTuner);
	AVL_DVBSx_ErrorCode AVL_DVBSx_ExtAV2011_GetLockStatus(struct AVL_Tuner * pTuner);
	AVL_DVBSx_ErrorCode AVL_DVBSx_ExtAV2011_Lock( struct AVL_Tuner * pTuner);
	AVL_DVBSx_ErrorCode AVL_DVBSx_ExtAV2011_Check_I2C_Address(AVL_puint16 Address, struct AVL_DVBSx_Chip * pAVLChip);

	#ifdef AVL_CPLUSPLUS
}
	#endif
#endif
