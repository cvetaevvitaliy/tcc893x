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

#ifndef __TC3X_USERDEFINE_h__
#define __TC3X_USERDEFINE_h__

//---------------------------
// Special Platform Define
//
#define USE_ANDROID
//#define USE_LINUX

#if defined(USE_ANDROID)
#define LOG_TAG "TCC35XX"
#include <utils/Log.h>
#endif
//---------------------------
// Board Define
//

//---------------------------
// #1 Dual Board Option
//
#define TCCX_NORMAL_BOARD
//#define TCC3161_DUAL_TMSET_BOARD   // TCC3161&2BND_SV0.1

#if defined (TCCX_NORMAL_BOARD) && defined (TCC3161_DUAL_TMSET_BOARD)
    # error. please select only one board define
#endif

#if !defined (TCCX_NORMAL_BOARD) && !defined (TCC3161_DUAL_TMSET_BOARD)
    # error. please select only one board define
#endif

//#define TESTOSCSUPPORT
#if defined (TESTOSCSUPPORT)
    #define OSC19200
#endif // TESTOSCSUPPORT 

//---------------------------
// #2 GPIO-C Antenna Band Switching
//
#define USE_BAND_SWITCH_GPIOC_06    // Band Switch using GPIOC 06 // Default : Enable

//---------------------------
#define DUAL_BB_AVAIL

#ifdef DUAL_BB_AVAIL
#define MAX_TCC3X           2
#define TCC3X_DEVICE0
#define TCC3X_DEVICE1
#else
#define MAX_TCC3X           1
#define TCC3X_DEVICE0
#endif

//---------------------------

#define TC3X_TDMB_SUPPORT
#define TC3X_DVBT_SUPPORT
#define TC3X_CMMB_SUPPORT
#define TC3X_ISDBT1SEG_SUPPORT

#define STDDEF_DVBT
#define STDDEF_CMMB
#define STDDEF_DMB
#define STDDEF_ISDBT1SEG
// Interface Define
#define USE_IF_CSPI
#define USE_IF_I2C

//
//------------------------------------------------------------------

#if defined(USE_ANDROID)
#define PRINTF_LV_0 ALOGD
#define PRINTF_LV_1 //ALOGD
#define PRINTF_LV_2 //ALOGD
#elif defined (USE_LINUX)
#define PRINTF_LV_0 printf
#define PRINTF_LV_1
#define PRINTF_LV_2
#else
#define PRINTF_LV_0
#define PRINTF_LV_1
#define PRINTF_LV_2
#endif

//----------------------------------------------------------
// Packet size & Packet Num Define (Recommand)
//
#if defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
// DVB-T 
#define TC3X_DVBT_PKTSIZE           (188)
#define TC3X_DVBT_PKTNUM_SPI        (32)
#define TC3X_DVBT_PKTNUM_STS        (8)
#define TC3X_DVBT_PKTNUM_CMDIO      (40)   // Not Recommand!!! USE SPI or STS!!!

// DVB-T / DVB-H TSBuff Num
#define DEF_TS_BUFF_NUM             90
#endif // defined (STDDEF_DVBT) || defined (STDDEF_DVBH)

#if defined (STDDEF_DVBH)
// DVB-H
#define TC3X_DVBH_PKTSIZE           (188)
#define TC3X_DVBH_PKTNUM_SPI        (32)
#define TC3X_DVBH_PKTNUM_STS        (32)
#define TC3X_DVBH_PKTNUM_CMDIO      (32)
#endif // STDDEF_DVBH

#if defined (STDDEF_CMMB)
#define TC3X_CMMB_PKTSIZE           (188)
#define TC3X_CMMB_PKTNUM            (64)
#define TC3X_CMMB_PKTSIZE_TS        (188)
#define TC3X_CMMB_PKTNUM_TS         (32)

// CMMB Buffer size
#define DEF_CMMB_BUFF_SIZE          (300*1024)
#endif // STDDEF_CMMB

#if defined (STDDEF_DMB)
    //#define TC3X_TDMB_PKTSIZE                     (188*2) // for fifo
    #define TC3X_TDMB_PKTSIZE               (188)   // for fifo
    #define TC3X_TDMB_PKTNUM                (20)
    #define TC3X_TDMB_PKTNUM_CMDIO          (20)

    // DAB Buffer size
    #define DEF_TDMB_BUFF_SIZE              0x4000
#endif // STDDEF_DMB

#if defined(STDDEF_ISDBT1SEG) || defined(STDDEF_ISDBT13SEG)
    #define TC3X_ISDBT1SEG_PKTSIZE          (188)
    #define TC3X_ISDBT1SEG_PKTNUM_SPI       (32)
    #define TC3X_ISDBT1SEG_PKTNUM_STS       (32)
    #define TC3X_ISDBT1SEG_PKTNUM_CMDIO     (32)

    #define DEF_ISDBT_TS_BUFF_NUM           192
#endif // defined(STDDEF_ISDBT1SEG)

#if defined(STDDEF_ISDBT13SEG)
    #define TC3X_ISDBT13SEG_PKTSIZE         (188)
    #define TC3X_ISDBT13SEG_PKTNUM_SPI      (32)
    #define TC3X_ISDBT13SEG_PKTNUM_STS      (32)
    #define TC3X_ISDBT13SEG_PKTNUM_CMDIO    (32)
#endif // defined(STDDEF_ISDBT13SEG)

//--------------------------------------------------------------------------
// Lock Time Defines
//
#if defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
#define DVB_AGC_LOCK           (50)
#define DVB_CTO_LOCK           (20)     // 13.8
#define DVB_CTO_RETRY          (3)
#define DVB_CFO_LOCK           (20)     // 12.6
#define DVB_CFO_RETRY          (3)
#define DVB_TPS_LOCK           (180)    // 171.36
#define DVB_TPS_LOCK_FAIL      (90)     // 85.68
#define DVB_TPS_RETRY          (3)
#define DVB_SYNC_INV_LOCK      (200)
#define DVB_SYNC_TSPER_UNDER_THRP_LOCK      (1000)
#define DVB_SYNC_TSPER_UNDER_THRP_RETRY     (1)
// invalid min : 230
// valid min : 270
// worst : 680+1200
#endif

#if defined (STDDEF_CMMB)
    #define CMMB_AGC_LOCK           (50)
    #define CMMB_CTO_BEACON_LOCK    (30)
    #define CMMB_CTO_BEACON_RETRY   (7)
    #define CMMB_CFO_LOCK           (10)    // 0.5
    #define CMMB_CFO_RETRY          (3)
    #define CMMB_CLCH_LOCK          (1000)  // 85.68
    #define CMMB_CLCH_RETRY         (2)
    // invalid min : 650
    // valid min : 1160
    // worst : 2650
#endif

#if defined (STDDEF_ISDBT1SEG) || defined (STDDEF_ISDBT13SEG)
    #define ISDBT_AGC_LOCK          (50)
    #define ISDBT_CTO_LOCK          (20)   // 13.8
    #define ISDBT_CTO_RETRY         (3)
    #define ISDBT_CFO_LOCK          (20)   // 12.6
    #define ISDBT_CFO_RETRY         (3)
    #define ISDBT_TMCC_LOCK         (520)  // 514.08
    #define ISDBT_TMCC_LOCK_FAIL    (260)  // 257.04
    #define ISDBT_TMCC_RETRY        (3)
    // invalid min : 230
    // valid min : 610
    // worst : 1530
#endif

#if defined (STDDEF_DMB)
    #define TDMB_OFDMDETECT_LOCK    (100)
    #define TDMB_OFDMDETECT_RETRY   (1)
    #define TDMB_CTO_LOCK           (100)
    #define TDMB_CTO_RETRY          (3)
    #define TDMB_CFO_LOCK           (20)
    #define TDMB_CFO_RETRY          (3)
    // invalid min : 100
    // valid min : 220
    // worst : 760
#endif

//--------------------------------------------------------------------------
// Antenna Bar Recommendations
//
// 0 : TS_PER (FER) > 5x10-1
// 1 : 5x10-2 < TS_PER < 5x10-1
// 2 : 5x10-3 < TS_PER < 5x10-2
// 3 : 5x10-4 < TS_PER < 5x10-3
// 4 : 5x10-5 < TS_PER < 5x10-4
// 5 : TS_PER < 5x10-5
#endif
