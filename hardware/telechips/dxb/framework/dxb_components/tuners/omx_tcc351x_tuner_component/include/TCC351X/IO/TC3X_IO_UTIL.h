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

#ifndef __TC3X_IO_UTIL_H__
#define __TC3X_IO_UTIL_H__

#include "TC3X_IO_UserDefine.h"

typedef int TC3X_DNUM;

typedef void (*FN_v_v) (void);
typedef void (*FN_v_i) (int);
typedef void (*FN_v_ii) (int, int);
typedef void (*FN_v_h) (TC3X_DNUM);
typedef void (*FN_v_ih) (int, TC3X_DNUM);
typedef void (*FN_v_hv) (TC3X_DNUM, void *);
typedef int (*FN_i_v) (void *);
typedef int (*FN_i_ivi) (int, void *, int);
typedef int (*FN_i_i) (int);
typedef int (*FN_i_hi) (TC3X_DNUM, int);
typedef int (*FN_i_ihi) (int, TC3X_DNUM, int);
typedef void (*FN_v_hi) (TC3X_DNUM, int);

typedef void (*FN_v_ihv) (int, TC3X_DNUM, void *);
typedef int (*FN_i_iv) (int, void *);

typedef void (*FN_v_ihi) (int, TC3X_DNUM, int);
typedef void (*FN_v_hii) (TC3X_DNUM, int, int);
typedef void (*FN_v_hiPci) (TC3X_DNUM, int, unsigned char *, int);
typedef void (*FN_v_ihiPci) (int, TC3X_DNUM, int, unsigned char *, int);
typedef int (*FN_i_hiii) (TC3X_DNUM, int, int, int);
typedef int (*FN_i_hiiv) (TC3X_DNUM, int, int, void *);
typedef int (*FN_i_ihiiv) (int, TC3X_DNUM, int, int, void *);
typedef int (*FN_i_hiiii) (TC3X_DNUM, int, int, int, int);
typedef int (*FN_i_hPcUiii) (TC3X_DNUM, unsigned char *, unsigned int, int, int);
typedef void (*FN_v_hiiiiPci) (TC3X_DNUM, int, int, int, int, unsigned char *, int);
typedef int (*FN_i_Pciii) (unsigned char *, int, int, int);
typedef void (*FN_v_hiii) (TC3X_DNUM, int, int, int);
typedef void (*FN_v_ihiii) (int, TC3X_DNUM, int, int, int);
typedef void (*FN_v_hivvvv) (TC3X_DNUM, int, void *, void *, void *, void *);
typedef void (*FN_v_ihivvvv) (int, TC3X_DNUM, int, void *, void *, void *, void *);
typedef int (*FN_i_hivvvv) (TC3X_DNUM, int, void *, void *, void *, void *);
typedef int (*FN_i_ihivvvv) (int, TC3X_DNUM, int, void *, void *, void *, void *);

//--------------------------------------
// Structures
typedef struct
{
    unsigned char *pCOLDBOOT;
    unsigned int COLDBOOTSize;
    unsigned char *pDAGU;
    unsigned int DAGUSize;
    unsigned char *pDINT;
    unsigned int DINTSize;
    unsigned char *pRAND;
    unsigned int RANDSize;
    unsigned char *pCOL_ORDER;
    unsigned int COL_ORDERSize;
} TC3X_BOOTBIN;

typedef struct
{
    int       BBType;
    int       NumOfDemodule;
    int       MainIO;
    int       StreamIO;
    int       Standard;

    // stream related
    int       StreamPktSize;
    int       StreamPktThrNum;

    int       UsePIDFiltering;
} tSTDInfo;

typedef struct
{
    int       BSetted;
    int       BInited;

    int       bbType;
    TC3X_DNUM hBB;

    FN_v_ihv  BB_PreInit;
    FN_i_ivi   BB_ColdBoot;
    FN_v_ihi  BB_Init;

    FN_v_ih   BB_Close;
    FN_i_ihiiv BB_SetFrequency;
    FN_v_ihiPci BB_StartService;
    FN_v_ihi  BB_StopService;
    FN_v_ihi  BB_CtrlPower;
    FN_i_ihivvvv BB_UserFunction;
    FN_v_ihivvvv STD_OccurEvent;
} tSTDCtrl;

typedef struct
{
    int       Standard;
    int       MainIO;
    int       StreamIO;
    int       PktSize;
    int       PktThrNum;
} tSTDIOSet;

unsigned char TC3X_IO_UTIL_CRC7 (unsigned char *data, unsigned int len);
unsigned short TC3X_IO_UTIL_CRC16 (unsigned char *data, unsigned short init, unsigned int len);

void     *TC3X_IO_UTIL_fopen (char *filename, char *opt);
int       TC3X_IO_UTIL_fclose (void *fp);
int       TC3X_IO_UTIL_fwrite (void *buf, int unit, int count, void *fp);
int       TC3X_IO_UTIL_fread (void *buf, int unit, int count, void *fp);
int       TC3X_IO_UTIL_fseek (void *fp, int offset, int flag);

void      TC3X_IO_UTIL_dummy_function0 (void);
void      TC3X_IO_UTIL_dummy_function1 (void *d1);
void      TC3X_IO_UTIL_dummy_function2 (int d2);
int       TC3X_IO_UTIL_dummy_function3 (int i0, TC3X_DNUM handle, int i1);
void      TC3X_IO_UTIL_dummy_function4 (int i0, TC3X_DNUM handle);
void      TC3X_IO_UTIL_dummy_function5 (int i0, TC3X_DNUM handle, int i1, int i2, int i3);
void      TC3X_IO_UTIL_dummy_function6 (int i0, TC3X_DNUM handle, int i1);
int       TC3X_IO_UTIL_dummy_function7 (int i0, TC3X_DNUM handle, int i1, void *p1, void *p2, void *p3, void *p4);
void      TC3X_IO_UTIL_dummy_function8 (TC3X_DNUM handle, int i1, void *p1, void *p2, void *p3, void *p4);
void      TC3X_IO_UTIL_dummy_function9 (int i0, TC3X_DNUM handle, int i1, void *p1, void *p2, void *p3, void *p4);
void      TC3X_IO_UTIL_dummy_function10 (int i0, int i1);
int       TC3X_IO_UTIL_ColdBootParser (unsigned char *pData, unsigned int size, TC3X_BOOTBIN * pBOOTBin);
int       TC3X_IO_Util_GET_MAXBB(void);
int       TC3X_IO_Util_Get_BoardDefine(void);
#if defined(USE_ANDROID) || defined(USE_LINUX)
long long TC3X_IO_Util_GetTickCnt (void);
unsigned int TC3X_IO_Util_GetInterval (long long startTick);
#else
unsigned int TC3X_IO_Util_GetTickCnt (void);
unsigned int TC3X_IO_Util_GetInterval (unsigned int startTick);
#endif // USE_ANDROID || USE_LINUX
#endif
