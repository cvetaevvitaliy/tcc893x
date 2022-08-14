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

#include "TC3X_IO.h"
#include "TC3X_IO_UTIL.h"
#include "../TC3X_Common.h"

#ifdef TCC78X
#include "tc_file.h"
#else
#include <string.h>
#endif

#ifndef NULL
#define NULL 0
#endif

#define	CRC_CHECK_BY_TABLE

#ifdef	CRC_CHECK_BY_TABLE
const unsigned char sdio_crc7_table[256] = {
    0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f,
    0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
    0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34, 0x2f, 0x26,
    0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e,
    0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
    0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45,
    0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d, 0x14,
    0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c,
    0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b,
    0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
    0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
    0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a,
    0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
    0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21,
    0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70,
    0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38,
    0x41, 0x48, 0x53, 0x5a, 0x65, 0x6c, 0x77, 0x7e,
    0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f, 0x36,
    0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67,
    0x10, 0x19, 0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f,
    0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
    0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
    0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55,
    0x22, 0x2b, 0x30, 0x39, 0x06, 0x0f, 0x14, 0x1d,
    0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08, 0x13, 0x1a,
    0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52,
    0x3c, 0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03,
    0x74, 0x7d, 0x66, 0x6f, 0x50, 0x59, 0x42, 0x4b,
    0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21, 0x28,
    0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60,
    0x0e, 0x07, 0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31,
    0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79
};

/**********************************************************
  Polynomial = 0x11021 (x^16 + x^12 + x^5 + 1)
  width      = 16 bit  (CCITT-CRC)
  INIT_CCITT_CRC	0xffffU
**********************************************************/
const unsigned short sdio_crc16_table[256] = {
    0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50a5U, 0x60c6U, 0x70e7U,
    0x8108U, 0x9129U, 0xa14aU, 0xb16bU, 0xc18cU, 0xd1adU, 0xe1ceU, 0xf1efU,
    0x1231U, 0x0210U, 0x3273U, 0x2252U, 0x52b5U, 0x4294U, 0x72f7U, 0x62d6U,
    0x9339U, 0x8318U, 0xb37bU, 0xa35aU, 0xd3bdU, 0xc39cU, 0xf3ffU, 0xe3deU,
    0x2462U, 0x3443U, 0x0420U, 0x1401U, 0x64e6U, 0x74c7U, 0x44a4U, 0x5485U,
    0xa56aU, 0xb54bU, 0x8528U, 0x9509U, 0xe5eeU, 0xf5cfU, 0xc5acU, 0xd58dU,
    0x3653U, 0x2672U, 0x1611U, 0x0630U, 0x76d7U, 0x66f6U, 0x5695U, 0x46b4U,
    0xb75bU, 0xa77aU, 0x9719U, 0x8738U, 0xf7dfU, 0xe7feU, 0xd79dU, 0xc7bcU,
    0x48c4U, 0x58e5U, 0x6886U, 0x78a7U, 0x0840U, 0x1861U, 0x2802U, 0x3823U,
    0xc9ccU, 0xd9edU, 0xe98eU, 0xf9afU, 0x8948U, 0x9969U, 0xa90aU, 0xb92bU,
    0x5af5U, 0x4ad4U, 0x7ab7U, 0x6a96U, 0x1a71U, 0x0a50U, 0x3a33U, 0x2a12U,
    0xdbfdU, 0xcbdcU, 0xfbbfU, 0xeb9eU, 0x9b79U, 0x8b58U, 0xbb3bU, 0xab1aU,
    0x6ca6U, 0x7c87U, 0x4ce4U, 0x5cc5U, 0x2c22U, 0x3c03U, 0x0c60U, 0x1c41U,
    0xedaeU, 0xfd8fU, 0xcdecU, 0xddcdU, 0xad2aU, 0xbd0bU, 0x8d68U, 0x9d49U,
    0x7e97U, 0x6eb6U, 0x5ed5U, 0x4ef4U, 0x3e13U, 0x2e32U, 0x1e51U, 0x0e70U,
    0xff9fU, 0xefbeU, 0xdfddU, 0xcffcU, 0xbf1bU, 0xaf3aU, 0x9f59U, 0x8f78U,
    0x9188U, 0x81a9U, 0xb1caU, 0xa1ebU, 0xd10cU, 0xc12dU, 0xf14eU, 0xe16fU,
    0x1080U, 0x00a1U, 0x30c2U, 0x20e3U, 0x5004U, 0x4025U, 0x7046U, 0x6067U,
    0x83b9U, 0x9398U, 0xa3fbU, 0xb3daU, 0xc33dU, 0xd31cU, 0xe37fU, 0xf35eU,
    0x02b1U, 0x1290U, 0x22f3U, 0x32d2U, 0x4235U, 0x5214U, 0x6277U, 0x7256U,
    0xb5eaU, 0xa5cbU, 0x95a8U, 0x8589U, 0xf56eU, 0xe54fU, 0xd52cU, 0xc50dU,
    0x34e2U, 0x24c3U, 0x14a0U, 0x0481U, 0x7466U, 0x6447U, 0x5424U, 0x4405U,
    0xa7dbU, 0xb7faU, 0x8799U, 0x97b8U, 0xe75fU, 0xf77eU, 0xc71dU, 0xd73cU,
    0x26d3U, 0x36f2U, 0x0691U, 0x16b0U, 0x6657U, 0x7676U, 0x4615U, 0x5634U,
    0xd94cU, 0xc96dU, 0xf90eU, 0xe92fU, 0x99c8U, 0x89e9U, 0xb98aU, 0xa9abU,
    0x5844U, 0x4865U, 0x7806U, 0x6827U, 0x18c0U, 0x08e1U, 0x3882U, 0x28a3U,
    0xcb7dU, 0xdb5cU, 0xeb3fU, 0xfb1eU, 0x8bf9U, 0x9bd8U, 0xabbbU, 0xbb9aU,
    0x4a75U, 0x5a54U, 0x6a37U, 0x7a16U, 0x0af1U, 0x1ad0U, 0x2ab3U, 0x3a92U,
    0xfd2eU, 0xed0fU, 0xdd6cU, 0xcd4dU, 0xbdaaU, 0xad8bU, 0x9de8U, 0x8dc9U,
    0x7c26U, 0x6c07U, 0x5c64U, 0x4c45U, 0x3ca2U, 0x2c83U, 0x1ce0U, 0x0cc1U,
    0xef1fU, 0xff3eU, 0xcf5dU, 0xdf7cU, 0xaf9bU, 0xbfbaU, 0x8fd9U, 0x9ff8U,
    0x6e17U, 0x7e36U, 0x4e55U, 0x5e74U, 0x2e93U, 0x3eb2U, 0x0ed1U, 0x1ef0U
};
#endif

//---------------------------------------------------------------------------------------
//
//  CRC
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_CRC7
//    Description : CRC Check
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
unsigned char TC3X_IO_UTIL_CRC7 (unsigned char *data, unsigned int len)
{
#ifndef CRC_CHECK_BY_TABLE
    unsigned short masking, carry;
    unsigned short crc;
    unsigned int i;

    crc = 0x0000;;

    for (i = 0; i < len; i++)
    {
        masking = 1 << 8;
        while ((masking >>= 1))
        {
            carry = crc & 0x40;
            crc <<= 1;
            if ((!carry) ^ (!(*data & masking)))
                crc ^= 0x9;
            crc &= 0x7f;
        }
        data++;
    }

    return (unsigned char) crc;
#else
    unsigned char crc7_accum = 0;
    unsigned int i;

    for (i = 0; i < len; i++)
    {
        crc7_accum = sdio_crc7_table[(crc7_accum << 1) ^ data[i]];
    }
    return crc7_accum;
#endif
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_CRC16
//    Description : CRC Check
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
unsigned short TC3X_IO_UTIL_CRC16 (unsigned char *data, unsigned short init, unsigned int len)
{
#ifndef	CRC_CHECK_BY_TABLE
    unsigned int masking, carry;
    unsigned short crc;
    unsigned int i;

    crc = init;

    for (i = 0; i < len; i++)
    {
        masking = 1 << 8;
        while ((masking >>= 1))
        {
            carry = crc & 0x8000;
            crc <<= 1;
            if ((!carry) ^ (!(*data & masking)))
                crc ^= 0x1021;

            crc &= 0xffff;
        }
        data++;
    }
    return crc;
#else
    register int bTemp;
    unsigned int i;
    unsigned short crc;

    crc = init;
    for (i = 0; i < len; i++)
    {
        bTemp = *data++;
        crc = sdio_crc16_table[((crc >> 8) ^ bTemp) & 0xff] ^ (crc << 8);
    }
    return crc;
#endif
}


void     *TC3X_IO_UTIL_fopen (char *filename, char *opt)
{
    FILE     *fp;
    char      re_opt[10];

    if (opt[0] == 'r' || opt[0] == 'R')
    {
        re_opt[0] = 'r';
        re_opt[1] = 'b';
        re_opt[2] = '\0';
    }
    else if (opt[0] == 'w' || opt[0] == 'W')
    {
        re_opt[0] = 'w';
        re_opt[1] = 'b';
        re_opt[2] = '\0';
    }

    fp = fopen (filename, re_opt);
    if (fp == 0)
        return NULL;
    return fp;
}

int TC3X_IO_UTIL_fclose (void *fp)
{
    return fclose ((FILE *) fp);
}

int TC3X_IO_UTIL_fwrite (void *buf, int unit, int count, void *fp)
{
    return fwrite (buf, unit, count, (FILE *) fp);
}

int TC3X_IO_UTIL_fread (void *buf, int unit, int count, void *fp)
{
    return fread (buf, unit, count, (FILE *) fp);
}

int TC3X_IO_UTIL_fseek (void *fp, int offset, int flag)
{
    return fseek ((FILE *) fp, offset, flag);
}


//---------------------------------------------------------------------------------------
//
//  ColdBoot Util
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_ColdBootParser
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_UTIL_ColdBootParser (unsigned char *pData, unsigned int size, TC3X_BOOTBIN * pBOOTBin)
{
    unsigned int idx;
    unsigned int length;
    unsigned char *pBin = NULL;
    unsigned char *pDAGU = NULL;
    unsigned char *pDINT = NULL;
    unsigned char *pRAND = NULL;
    unsigned char *pcol_order = NULL;
    unsigned int BootSize[5];

    //coldboot      0x00000001
    //dagu          0x00000002
    //dint          0x00000003
    //rand          0x00000004
    //col_order: 0x00000005

    //sizebyte      4byte
    //data          nbyte

    TC3X_IO_memset (BootSize, 0, sizeof (unsigned int) * 5);

    // cold boot
    idx = 0;
    if (pData[idx + 3] != 0x01)
    {
        PRINTF_LV_0 ("# Coldboot_preset_Error\n");
        return 0;
    }
    idx += 4;
    length = (pData[idx] << 24) + (pData[idx + 1] << 16) + (pData[idx + 2] << 8) + (pData[idx + 3]);
    idx += 4;

    BootSize[0] = length;
    pBin = &pData[idx];
    idx += length;
    size -= (length + 8);

    // dagu
    if (pData[idx + 3] != 0x02)
    {
        PRINTF_LV_0 ("# Coldboot_preset_Error\n");
        return 0;
    }
    idx += 4;
    length = (pData[idx] << 24) + (pData[idx + 1] << 16) + (pData[idx + 2] << 8) + (pData[idx + 3]);
    idx += 4;

    if (length)
    {
        pDAGU = &pData[idx];
        BootSize[1] = length;
        idx += length;
    }
    else
    {
        BootSize[1] = 0;
    }
    size -= (length + 8);

    // dint
    if (pData[idx + 3] != 0x03)
    {
        PRINTF_LV_0 ("# Coldboot_preset_Error\n");
        return 0;
    }
    idx += 4;
    length = (pData[idx] << 24) + (pData[idx + 1] << 16) + (pData[idx + 2] << 8) + (pData[idx + 3]);
    idx += 4;

    if (length)
    {
        pDINT = &pData[idx];
        BootSize[2] = length;
        idx += length;
    }
    else
    {
        pDINT = NULL;
        BootSize[2] = 0;
    }
    size -= (length + 8);

    // rand
    if (pData[idx + 3] != 0x04)
    {
        PRINTF_LV_0 ("# Coldboot_preset_Error\n");
        return 0;
    }

    idx += 4;
    length = (pData[idx] << 24) + (pData[idx + 1] << 16) + (pData[idx + 2] << 8) + (pData[idx + 3]);
    idx += 4;

    if (length)
    {
        pRAND = &pData[idx];
        BootSize[3] = length;
        idx += length;
    }
    else
    {
        pRAND = NULL;
        BootSize[3] = 0;
    }
    size -= (length + 8);

    if (size >= 8)
    {
        // for cmmb

        // col_order
        if (pData[idx + 3] != 0x05)
        {
            PRINTF_LV_0 ("# Coldboot_preset_Error\n");
            return 0;
        }

        idx += 4;
        length = (pData[idx] << 24) + (pData[idx + 1] << 16) + (pData[idx + 2] << 8) + (pData[idx + 3]);
        idx += 4;

        PRINTF_LV_2 ("# COL_ORDER_EXIST[%d]\n", length);

        if (length)
        {
            pcol_order = &pData[idx];
            BootSize[4] = length;
            idx += length;
        }
        else
        {
            pcol_order = NULL;
            BootSize[4] = 0;
        }
        size -= (length + 8);
    }

    pBOOTBin->pCOLDBOOT = pBin;
    pBOOTBin->COLDBOOTSize = BootSize[0];
    pBOOTBin->pDAGU = pDAGU;
    pBOOTBin->DAGUSize = BootSize[1];
    pBOOTBin->pDINT = pDINT;
    pBOOTBin->DINTSize = BootSize[2];
    pBOOTBin->pRAND = pRAND;
    pBOOTBin->RANDSize = BootSize[3];
    pBOOTBin->pCOL_ORDER = pcol_order;
    pBOOTBin->COL_ORDERSize = BootSize[4];

    return 1;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_dummy_function0
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_UTIL_dummy_function0 (void)
{
    PRINTF_LV_2 ("# undefined a function of TC3X sdk\n");
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_dummy_function1
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_UTIL_dummy_function1 (void *d1)
{
    return;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_dummy_function2
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_UTIL_dummy_function2 (int d2)
{
    return;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_dummy_function3
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_UTIL_dummy_function3 (int i0, TC3X_DNUM handle, int i1)
{
    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_dummy_function4
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_UTIL_dummy_function4 (int i0, TC3X_DNUM handle)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_dummy_function5
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_UTIL_dummy_function5 (int i0, TC3X_DNUM handle, int i1, int i2, int i3)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_dummy_function6
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_UTIL_dummy_function6 (int i0, TC3X_DNUM handle, int i1)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_dummy_function7
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_UTIL_dummy_function7 (int i0, TC3X_DNUM handle, int i1, void *p1, void *p2, void *p3, void *p4)
{
    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_dummy_function8
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_UTIL_dummy_function8 (TC3X_DNUM handle, int i1, void *p1, void *p2, void *p3, void *p4)
{

}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_dummy_function9
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_UTIL_dummy_function9 (int i0, TC3X_DNUM handle, int i1, void *p1, void *p2, void *p3, void *p4)
{

}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_UTIL_dummy_function10
//    Description : 
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_UTIL_dummy_function10 (int i0, int i1)
{

}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_Util_GET_MAXBB
//    Description : get max baseband number
//    Input : 
//    
//    Output : 
//      int
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_Util_GET_MAXBB(void)
{
    int ret;
    ret = MAX_TCC3X;
    return ret;
}

int TC3X_IO_Util_Get_BoardDefine(void)
{
    int definedboard = 0;

    #ifdef TCCX_NORMAL_BOARD
    definedboard = DEVICE_BB_NORMAL_BOARD;
    #endif
    
    #ifdef TCC3161_DUAL_TMSET_BOARD
    definedboard = DEVICE_BB_TCC3161_DUAL_TMSET_BOARD;
    #endif
    
    return definedboard;
}

#if defined(USE_ANDROID) || defined(USE_LINUX)
long long TC3X_IO_Util_GetTickCnt (void)
{
    return (TC3X_IO_GET_TIMECNT_ms ());
}

unsigned int TC3X_IO_Util_GetInterval (long long startTick)
{
    unsigned int Cnt;

    if (TC3X_IO_Util_GetTickCnt () > startTick)
        Cnt = (unsigned int) ((TC3X_IO_Util_GetTickCnt () - startTick) * 1000);
    else
        Cnt = (unsigned int) (((0xFFFFFFFFFFFFFFFFLL - startTick) + TC3X_IO_Util_GetTickCnt () + 1) * 1000);

    return Cnt;
}
#else
unsigned int TC3X_IO_Util_GetTickCnt (void)
{
    return (TC3X_IO_GET_TIMECNT_us ());
}

unsigned int TC3X_IO_Util_GetInterval (unsigned int startTick)
{
    unsigned int Cnt;

    if (TC3X_IO_Util_GetTickCnt () > startTick)
        Cnt = TC3X_IO_Util_GetTickCnt () - startTick;
    else
        Cnt = ((0xFFFFFFFF - startTick) + TC3X_IO_Util_GetTickCnt () + 1);
    return Cnt;
}
#endif // USE_ANDROID || USE_LINUX

