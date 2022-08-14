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


///$Date: 2012-2-9 17:36 $
///

#ifndef AVL6211_API_h_h
    #define AVL6211_API_h_h

    #include "include/avl_dvbsx.h"

    #ifdef AVL_CPLUSPLUS
extern "C" {
	#endif

    struct Signal_Level
    {
	    AVL_uint16 SignalLevel;
        AVL_int16 SignalDBM;
    };

	void AVL_DVBSx_Error_Dispose(AVL_DVBSx_ErrorCode r);
	void AVL_Set_LPF(struct AVL_Tuner * pTuner, AVL_uint32 m_uiSymbolRate_Hz);

	AVL_DVBSx_ErrorCode AVL6211_Initialize(struct AVL_DVBSx_Chip * pAVLChip,struct AVL_Tuner * pTuner);
	AVL_DVBSx_ErrorCode CPU_Halt(struct AVL_DVBSx_Chip * pAVLChip);
	AVL_DVBSx_ErrorCode AVL6211_LockChannel(struct AVL_DVBSx_Chip * pAVLChip, struct AVL_Tuner * pTuner, AVL_uint32 tuner_freq, AVL_uint32 signal_symbolrate);

	AVL_int16 AVL_Get_Level_Percent(struct AVL_DVBSx_Chip * pAVLChip);
	int AVL_Get_Quality_Percent(struct AVL_DVBSx_Chip * pAVLChip);

	AVL_uint32 AVL6211_GETLockStatus(struct AVL_DVBSx_Chip * pAVLChip);   
	AVL_uint32 AVL6211_GETBer(struct AVL_DVBSx_Chip * pAVLChip);
	AVL_uint32 AVL6211_GETPer(struct AVL_DVBSx_Chip * pAVLChip);
	AVL_uint32 AVL6211_GETSnr(struct AVL_DVBSx_Chip * pAVLChip);
	AVL_uint32 AVL6211_GETSignalLevel(struct AVL_DVBSx_Chip * pAVLChip);

	AVL_uint32 AVL6211_BlindScan_Reset(struct AVL_DVBSx_Chip * pAVLChip, struct AVL_DVBSx_BlindScanAPI_Setting *pBSsetting);

	#ifdef AVL_CPLUSPLUS
}
	#endif

#endif
