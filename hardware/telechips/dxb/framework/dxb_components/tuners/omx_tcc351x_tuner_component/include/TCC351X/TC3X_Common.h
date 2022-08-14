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

#ifndef __TC3X_COMMON_H__
#define __TC3X_COMMON_H__

#include "IO/TC3X_IO_UserDefine.h"

typedef enum
{
    DEVICE_BB_NONE = 0,
    DEVICE_BB_TC3X,
    DEVICE_BB_OTHER
} TCCBB_DEVICE;

typedef enum
{
    DEVICE_BB_NORMAL_BOARD = 0,
    DEVICE_BB_TCC3161_DUAL_TMSET_BOARD
} TCCBB_BoardDefine;

//---------------------------------------------------------------------------------------
//
//  TC3X Interface Form
//
typedef enum
{
    TC3X_IO_FAIL,
    TC3X_IO_SUCCESS,
    TC3X_IO_ILLEGAL_INTF                                   //illegal interface
} TC3X_IO_Err;

//---------------------------------------------------------------------------------------
//
//      Interface Structure
//
typedef enum
{
    TC3X_INT_LEVEL_FALLING = 0,
    TC3X_INT_LEVEL_RISING,
    TC3X_INT_EDGE_FALLING,
    TC3X_INT_EDGE_RISING
} TC3X_INT_TRIG_Set;

// -----------------------------------------------------------------
// IO Series Enum
//
typedef enum
{
    TC3X_IF_SRAMLIKE,
    TC3X_IF_I2C,
    TC3X_IF_CSPI,
    TC3X_IF_SDIO1BIT,
    TC3X_IF_SDIO4BIT
} TC3X_MainIOSeries;

typedef enum
{
    TC3X_STREAM_IO_MAINIO,
    TC3X_STREAM_IO_PTS,                                    // DVB-T, DVBH : EDGE Int
    TC3X_STREAM_IO_STS,                                    // DVB-T, DVBH : EDGE Int
    TC3X_STREAM_IO_SPIMS,                                  // DVB-T, DVBH : EDGE Int
    TC3X_STREAM_IO_SPISLV,                                 // DVB-T, DVBH : LEVEL Int
    TC3X_STREAM_IO_HPI_HEADERON,                           // DVB-T : LEVEL Int
    // Header Format
    // Header[31:20] : 0xa80
    // ID [19:18] : 0x01:BufferA , 0x02:BufferB, 0x03:BufferC
    // Private[17:16]
    // Length[15:0] : WordSize
    TC3X_STREAM_IO_HPI_HEADEROFF,                          // DVB-T : LEVEL Int
    TC3X_STREAM_IO_MAX
} TC3X_StreamIOSeries;

//---------------------------------------------------------------------------------------
//
//  TC3X Standards Enum
//
typedef enum
{
    TC3X_STD_DVBT = 0,
    TC3X_STD_DVBH,
    TC3X_STD_CMMB,
    TC3X_STD_DMB,
    TC3X_STD_ISDBT1SEG,
    TC3X_STD_ISDBT13SEG,
    TC3X_STD_OTHERS,
    TC3X_STD_MAX
} STD_TC3X;

//---------------------------------------------------------------------------------------
//
//  TC3X Commands Enum
//
enum
{
    CMD_BB_INT_INIT = 0,
    CMD_BB_INT_ENABLE,
    CMD_BB_INT_DISABLE,
    CMD_BB_INT_PAUSE,
    CMD_BB_INT_RESUME
};

enum
{
    CMD_SDIO_INT_INIT = 0,
    CMD_SDIO_INT_ENABLE,
    CMD_SDIO_INT_DISABLE,
    CMD_SDIO_INT_PAUSE,
    CMD_SDIO_INT_RESUME
};

enum
{
    CMD_SPI_ALLOC_BUFF = 0,
    CMD_SPI_INIT,
    CMD_SPI_START,
    CMD_SPI_STOP
};

enum
{
    CMD_BB_COLDBOOT_NORMAL = 0,
    CMD_BB_COLDBOOT_BROADCAST,
    CMD_BB_COLDBOOT_NON_WRITE
};

// -----------------------------------------------------------------
//
// User Function Msg
//
typedef enum
{
    // BB
    CMD_BB_NONE = 0,
    CMD_BB_PW_RESET,
    CMD_BB_PW_DN,
    CMD_BB_PW_ON,
    CMD_BB_PW_TSPD,
    CMD_BB_PW_TSPON,
    CMD_BB_WARMBOOT,

    CMD_BB_REG_R,
    CMD_BB_REG_W,

    CMD_BB_MAILBOX_R,
    CMD_BB_MAILBOX_W,

    CMD_BB_STREAMOUT_PAUSE,
    CMD_BB_STREAMOUT_RESTART,

    CMD_BB_CHECK_OP,

    CMD_BB_GET_CHIPID,

    CMD_BB_WRITE_MEM,
    CMD_BB_READ_MEM,

    CMD_BB_PLL_SET,
    
    CMD_BB_GET_BOOTCODE_VERSION,

#if defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
    CMD_BB_EN_PID,
    CMD_BB_DIS_PID,
    CMD_BB_PID_REG,
    CMD_BB_PID_UNREG,

    CMD_GET_DVB_LOCK,
    CMD_GET_DVB_SNR_SET,
    CMD_UPDATECMD_DVB_BERSET,
    CMD_GET_DVB_PCBER_SET,
    CMD_GET_DVB_VITERBIBER_SET,
    CMD_GET_DVB_TSPER_SET,
    CMD_GET_DVB_RSSI_SET,
#endif // #if defined (STDDEF_DVBT) || defined (STDDEF_DVBH)

#if defined (STDDEF_DVBH)
    CMD_BB_SETMPEFECPARAM,
    CMD_BB_REG_DVBH_SERVICE,
    CMD_BB_UNREG_DVBH_SERVICE,

    // CMD Ext
    SET_MPEFEC_STAT_CLR,
    GET_MPEFEC_STAT,
    GET_MPEFEC_CRSTAT,
#endif // STDDEF_DVBH

#if defined (STDDEF_CMMB)
    CMD_BB_SET_CMMB_SERVICE_INFO,
    CMD_BB_CMMB_REG,
    CMD_BB_CMMB_UNREG,

    CMD_GET_CMMB_LOCK,
    CMD_GET_CMMB_MER_SET,
    CMD_UPDATECMD_CMMB_BERSET,
    CMD_GET_CMMB_LDPC_BLER_SET,
    CMD_GET_CMMB_VITERBIBER_SET,
    CMD_GET_CMMB_FER_SET,
    CMD_GET_CMMB_RSSI_SET,
    CMD_REQ_CMMBSIZE,
#endif // STDDEF_CMMB

#if defined (STDDEF_ISDBT1SEG) || defined (STDDEF_ISDBT13SEG)
    CMD_GET_ISDBT_LOCK,
    CMD_GET_ISDBT_MER_SET,
    CMD_UPDATECMD_ISDBT_BERSET,
    CMD_GET_ISDBT_PCBER_SET,
    CMD_GET_ISDBT_VITERBIBER_SET,
    CMD_GET_ISDBT_TSPER_SET,
    CMD_GET_ISDBT_RSSI_SET,
#endif // defined (STDDEF_ISDBT1SEG) || defined (STDDEF_ISDBT13SEG)

#if defined (STDDEF_DMB)
    CMD_BB_TDMB_REG,
    CMD_BB_TDMB_UNREG,

    CMD_GET_TDMB_LOCK,
    CMD_GET_TDMB_SNR_SET,
    CMD_GET_TDMB_PCBER_SET,
    CMD_GET_TDMB_VITERBIBER_SET,
    CMD_GET_TDMB_TSPER_SET,
    CMD_GET_TDMB_RSSI_SET,
    CMD_PUSH_TDMB_STAT_DATA,
    CMD_REQUEST_PARSE_TDMB_DATA,
    CMD_REQUEST_TDMB_RESET_MONITOR_VALUE,
#endif // STDDEF_DMB

    // RF
    CMD_RF_NONE,
    CMD_RF_SET_RF_ADDR,
    CMD_RF_READ_REG_SINGLE,
    CMD_RF_WRITE_REG_SINGLE,
    CMD_RF_GET_PLL_LOCKSTAT,
    MSG_SET_FN_RF_READ,
    MSG_SET_FN_RF_WRITE,
    MSG_SET_FN_RF_WRITEEX
} ENUM_CMD_TO_BBRF;

// -----------------------------------------------------------------
//
// Frequency Setting Option
//
typedef struct
{
    unsigned int Hierarchy;
    unsigned int EnablePIDFiltering;
} tFreqSet_Option;

// -----------------------------------------------------------------
//
// Services Define
//

#if defined (STDDEF_DVBH)

typedef struct
{
    unsigned short pid[8];
    unsigned short rs_row_num[8];
    unsigned char max_burst_duration[8];
    unsigned char UseMPEFEC[8];
    unsigned char UseTimeSlice[8];
    unsigned char num;
} tMPEFEC_INFO;

// for Setting MPEFEC
typedef struct
{
    unsigned short pid[8];
    unsigned short rs_row_num[8];
    unsigned char max_burst_duration[8];
    unsigned char UseMPEFEC[8];
    unsigned char UseTimeSlice[8];
    unsigned char num;
} tMPEFEC_ServInfo;
#endif // STDDEF_DVBH

#if defined (STDDEF_CMMB)
//---------------------------------------------------------------------------------------
//      CMMB Struct for Stream Receive

#define MAX_CMMB_TS_NUM     (40)
typedef struct
{
    unsigned char MFID;                                    // MF ID
    unsigned char Scrambling;                              // 0~7 : mode0~mode7
    unsigned char InterleavingMode;                        // 0:reserved or clch, 1:mode1, 2:mode2, 3:mode3
    unsigned char RSCodeRate;                              // 0:240, 1:224, 2:192, 3:176
    unsigned char Constellation;                           // 0:bpsk, 1:qpsk, 2:16qam, 3,reserved.. 
    // BB에 넣으려면.. 0:qpsk, 1:16qam, 2:reserved, 3:bpsk
    unsigned char LDPC_CR;                                 // 0:1/2, 1:3/4
    unsigned char NumOfTS;
    unsigned char TSStart;
} CMMBService_t;

typedef struct
{
    // 2  ModeModulation [27~28]
    // 3  ModeScrambling [24~26]
    // 2  ModeByteInterleaving [22~23]
    // 6  QuantityTimeSlot [16~21]
    // 2  LDPCCodingRate [14~15]
    // 6  StartTimeSlotNum [8~13]
    // 2  RSCodeRate [6~7]
    // 6  MFID; [0~5]
    unsigned int ServicesParam[MAX_CMMB_TS_NUM];
    unsigned int ServiceNumber;
} tCMMBServicesInfo;
#endif // STDDEF_CMMB

#if defined (STDDEF_DMB)
//---------------------------------------------------------------------------------------
//  TDMB Struct for Stream Receive
typedef struct
{
    int       ServType;
    int       PType;
    int       SubChID;
    int       CUSize;
    int       StartCU;
    int       ReConfig;
    int       RS_ON;
    int       PLevel;
    int       BitRate;
} TDMBService_t;
#endif // STDDEF_DMB

#if defined(STDDEF_ISDBT1SEG)
typedef struct
{
    int       Reserved;
} ISDBT1SEGService_t;

typedef struct
{
    unsigned int idx;
    unsigned int RESERVED;
} ISDBT1SEGCtrl_t;
#endif // defined(STDDEF_ISDBT1SEG)

#if defined(STDDEF_ISDBT13SEG)
typedef struct
{
    int       Reserved;
} ISDBT13SEGService_t;

typedef struct
{
    unsigned int RESERVED;
} ISDBT13SEGCtrl_t;
#endif // defined(STDDEF_ISDBT13SEG)

#if defined (STDDEF_DMB)
// -----------------------------------------------------------------
// TDMB Service Types
//
#define SRVTYPE_NONE                0x00
#define SRVTYPE_DAB                 0x01
#define SRVTYPE_DABPLUS             0x02
#define SRVTYPE_DATA                0x03
#define SRVTYPE_DMB                 0x04
#define SRVTYPE_FIDC                0x05
#define SRVTYPE_RAWDATA             0x06
#define SRVTYPE_FIC                 0x07
#define SRVTYPE_FIC_WITH_ERR        0x08
#define EWS_ANNOUNCE_FLAG                   0x09
#define RECONFIGURATION_FLAG                0x0a
#define EWS_ANNOUNCE_RECONFIGURATION_FLAG   0x0b
#endif // STDDEF_DMB

//------------------------------------------------------------------
// Monitoring Value
//

#if defined (STDDEF_ISDBT1SEG) || defined (STDDEF_ISDBT13SEG)
typedef struct
{
    unsigned char AGC;
    unsigned char DCEC;
    unsigned char CTO;
    unsigned char CFO;
    unsigned char TMCC;

    unsigned char RESERVED0;
    unsigned char RESERVED1;
    unsigned char RESERVED2;
    unsigned char RESERVED3;
} t_ISDBTLock;
#endif

#if defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
typedef struct
{
    unsigned char AGC;
    unsigned char DCEC;
    unsigned char CTO;
    unsigned char CFO;
    unsigned char SPISCH;
    unsigned char TPS;
    unsigned char TSPER_UNDER_THR;
    unsigned char INV_SYNC_BYTE;
} t_DVBLock;
#endif

#if defined (STDDEF_CMMB)
typedef struct
{
    unsigned char AGC;
    unsigned char DCEC;
    unsigned char CTO;
    unsigned char CFO;
    unsigned char CLCH;
    unsigned char RESERVED0;
    unsigned char FTO;
    unsigned char RESERVED2;
} t_CMMBLock;
#endif

#if defined (STDDEF_DMB)
typedef struct
{
    unsigned char AGC;
    unsigned char DC;
    unsigned char CTO;
    unsigned char CFO;
    unsigned char FFO;
    unsigned char FTO;
    unsigned char SYNC_BYTE;
    unsigned char OFDM_DETECT;
} t_TDMBLock;
#endif // STDDEF_DMB

// -----------------------------------------------------------------
// Stream Type & Event Function Type
//
typedef enum
{
    CMD_STREAM_NONE = 0,
#if defined (STDDEF_CMMB)
    CMD_STREAM_CMMB,
    CMD_STREAM_CMMB_PIECE,
    CMD_STREAM_CMMB_PIECE_START,
    CMD_STREAM_CMMB_PIECE_END,
#endif // STDDEF_CMMB
#if defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
    CMD_STREAM_TS,
#endif // #if defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
#if defined (STDDEF_DVBH)
    CMD_STREAM_IP,
#endif // STDDEF_DVBH
#if defined (STDDEF_DMB)
    CMD_STREAM_TDMB,
#endif // STDDEF_DMB
#if defined(STDDEF_ISDBT1SEG)
    CMD_STREAM_ISDBT1SEG,
#endif
#if defined(STDDEF_ISDBT13SEG)
    CMD_STREAM_ISDBT13SEG,
#endif
    CMD_STREAM_STOPPED,

    // Error Handler
    TC3X_ERROR_UNKNOWN,
    TC3X_ERROR_CRITICAL_MAILBOX,

    EVENT_QUIET_DATALINE = 0xDA
} ENUM_CMD_STREAM;

// -----------------------------------------------------------------
// MailboxLog
//
typedef enum
{
    MAILBOX_LOG_NONE = 0,
    MAILBOX_LOG_FREQSET_PREPARE,
    MAILBOX_LOG_FREQSET_START,
    MAILBOX_LOG_FREQSET_START_STEP1,
    MAILBOX_LOG_FREQSET_START_STEP2,
    MAILBOX_LOG_FREQSET_STOP,
    MAILBOX_LOG_SERVICE
} ENUM_MAILBOX_LOG;

typedef struct
{
    unsigned int Frequency;
    int       SpurTap0_A;
    int       SpurTap0_B;
    int       SpurTap1_A;
    int       SpurTap1_B;
    int       SpurTap2_A;
    int       SpurTap2_B;
} tTunningForOSC;

// -----------------------------------------------------------------
// TCC35XX Setting Option
//
typedef struct
{
    unsigned char GPIOT_Strength;           // 0~3 , others : don't care (default value)
    unsigned char STS_SPI_ClkSpeed_DLR;     // 0~7 , others : don't care (default value)
    //----------------------------------------
    // Standard             | DLR 0 | DLR 1 | DLR 2 | DLR 3 | DLR 4 | DLR 5 | DLR 6 | DLR 7
    // DVBT  (KHz)          | 67200 | 33600 | 22400 | 16800 | 13440 | 11200 | 9600  | 8400
    // TDMB  (KHz)          | 19200 | 9600  | 6400  | 4800  | 3840  | 3200  | 2743  | 2400
    // CMMB  (KHz)          | 55200 | 27600 | 18400 | 13800 | 11040 | 9200  | 7886  | 6900
    // ISDB-T 1SEG (KHz)    | 43200 | 21600 | 14400 | 10800 | 8640  | 7200  | 6172  | 5400
    // ISDB-T FULLSEG(KHz)  | 60000 | 30000 | 20000 | 15000 | 12000 | 10000 | 8572  | 7500
    unsigned char Reserved[22];
} tTC3X_SettingOption;  // 24 Bytes

#endif
