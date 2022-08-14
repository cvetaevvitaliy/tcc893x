/****************************************************************************
 *   FileName    : tcc317x_main.c
 *   Description : Sample source main
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

#include "tcc317x_common.h"
#include "tcc317x_api.h"
#include "tcc317x_monitoring.h"
#include "tcc317x_stream_process.h"
#if defined (_INCLUDED_ASM_FILE_)
#include "TCC317X_BOOT_TDMB.h"
#endif

#define TCC317X_STREAM_THRESHOLD_WH             ((TCC317X_STREAM_THRESHOLD>>10)&0xFF)
#define TCC317X_STREAM_THRESHOLD_WL             ((TCC317X_STREAM_THRESHOLD>>2)&0xFF) 
#define TCC317X_DLR                             4 /*1 : 9MHz, 4: 3.xxMhz */ 
#define TCC317X_STREAM_THRESHOLD_SPI            ((188)>>2) 

#define OTHER_CHIP_BUFF_THR (188*4)
#define OTHER_CHIP_BUFF_THR_H (((188*4)>>10)&0xFF)
#define OTHER_CHIP_BUFF_THR_L (((188*4)>>2)&0xFF)

/*
	CHAIN_MODE
	: bb controls other bb's chipsel pin.

	INDIVIDUAL_MODE
	: bb work individually
*/

/*  [0] Tccspi [1] sts [2] spims */
Tcc317xRegisterConfig_t Tcc317xSingle[3] =
{
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF, TCC317X_STREAM_THRESHOLD_WH, TCC317X_STREAM_THRESHOLD_WL, 0x12,
        0,0x3C,0,0,0,0,0,0x3C,0,0,0,
        0x4,
        0,0,0,0,0,0,
        0xF0,0x0F,TCC317X_STREAM_THRESHOLD_WH,TCC317X_STREAM_THRESHOLD_WL,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0,0x3F,0,0,0,0,0,0x3C,0,0,0,
        0,
        0x21,0x30 | TCC317X_DLR ,0x12 /*or 0xF0*/,0x40,0,
        0,
        0xF0,0x0F,0,0,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0,0x3F,0,0,0,0,0,0x3C,0,0,0,
        0,
        0x11,TCC317X_DLR<<2,0x10,0,0,
        0,
        0xF0,0x0F,0,0,
    },
};

/*  [0] Tccspi [1] sts [2] spims */
Tcc317xRegisterConfig_t Tcc317x_FirstBB_ChainMode[3] =
{
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF, TCC317X_STREAM_THRESHOLD_WH, TCC317X_STREAM_THRESHOLD_WL, 0x12,
        0,0x3C,0,0,0,0,0,0x3C,0,0,0,
        0x4,
        0,0,0,0,0,0,
        0xF0,0x0F,TCC317X_STREAM_THRESHOLD_WH,TCC317X_STREAM_THRESHOLD_WL,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0,0x3F,0,0,0,0,0,0x3C,0,0,0,
        0,
        0x21,0x30 | TCC317X_DLR ,0x12 /*or 0xF0*/,0x40,0,
        0,
        0xF0,0x0F,0,0,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0,0x3F,0,0,0,0,0,0x3C,0,0,0,
        0,
        0x11,TCC317X_DLR<<2,0x10,0,0,
        0,
        0xF0,0x0F,0,0,
    },
};

/*  [0] Tccspi [1] sts [2] spims */
Tcc317xRegisterConfig_t Tcc317x_MiddleBB_ChainMode[3]  =
{
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF, TCC317X_STREAM_THRESHOLD_WH, TCC317X_STREAM_THRESHOLD_WL, 0x12,
        0,0x3C,0x10,0,0x10,0,0,0x3C,0,0,0,
        0x4,
        0,0,0,0,0,0,
        0xF0,0x0F,TCC317X_STREAM_THRESHOLD_WH,TCC317X_STREAM_THRESHOLD_WL,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0,0x3F,0x10,0,0x10,0,0,0x3C,0,0,0,
        0,
        0x21,0x30 | TCC317X_DLR ,0x72 /*or 0xF0*/,0x40,0,
        0,
        0xF0,0x0F,0,0,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0,0x3F,0x10,0,0x10,0,0,0x3C,0,0,0,
        0,
        0x11,TCC317X_DLR<<2,0x10,0,0,
        0,
        0xF0,0x0F,0,0,
    },
};

/*  [0] Tccspi [1] sts [2] spims */
Tcc317xRegisterConfig_t Tcc317x_LastBB_ChainMode[3] =
{
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF, TCC317X_STREAM_THRESHOLD_WH, TCC317X_STREAM_THRESHOLD_WL, 0x12,
        0,0x3C,0x10,0,0x10,0,0,0x3C,0,0,0,
        0x4,
        0,0,0,0,0,0,
        0xF0,0x0F,TCC317X_STREAM_THRESHOLD_WH,TCC317X_STREAM_THRESHOLD_WL,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0,0x3F,0x10,0,0x10,0,0,0x3C,0,0,0,
        0,
        0x21,0x30 | TCC317X_DLR ,0x12 /*or 0xF0*/,0x40,0,
        0,
        0xF0,0x0F,0,0,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0,0x3F,0x10,0,0x10,0,0,0x3C,0,0,0,
        0,
        0x11,TCC317X_DLR<<2,0x10,0,0,
        0,
        0xF0,0x0F,0,0,
    },
};

/*  [0] Tccspi [1] sts [2] spims */
Tcc317xRegisterConfig_t Tcc317x_FirstDiversity[3] =
{
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,TCC317X_STREAM_THRESHOLD_WH,TCC317X_STREAM_THRESHOLD_WL, 0x12,
        0x0E,0x3C,0,0,0,0,0,0x3C,0,0,0x2,
        0x4,
        0,0,0,0,0,
        0,
        0xF0,0x0F,TCC317X_STREAM_THRESHOLD_WH,TCC317X_STREAM_THRESHOLD_WL,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0x0E,0x3F,0,0,0,0,0,0x3C,0,0,0x2,
        0,
        0x21,0x30 | TCC317X_DLR ,0x72 /*or 0xF0*/,0x40,0,
        0,
        0xF0,0x0F,0,0,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0x0E,0x3F,0,0,0,0,0,0x3C,0,0,0x2,
        0,
        0x11,TCC317X_DLR<<2,0x10,0,0,
        0,
        0xF0,0x0F,0,0,
    },
};

/*  [0] Tccspi [1] sts [2] spims */
Tcc317xRegisterConfig_t Tcc317x_MiddleDiversity[3]  =
{
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,TCC317X_STREAM_THRESHOLD_WH,TCC317X_STREAM_THRESHOLD_WL,0,
        0x0F,0xFC,0x10,0,0x10,0,0,0,0,0,0x3,
        0,
        0,0,0,0,0,
        0,
        0xF0,0x0F,TCC317X_STREAM_THRESHOLD_WH,TCC317X_STREAM_THRESHOLD_WL,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0x0F,0xFF,0x10,0,0x10,0,0,0,0,0,0x3,
        0,
        0,0,0,0,0,
        0,
        0xF0,0x0F,0,0,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0x0F,0xFF,0x10,0,0x10,0,0,0,0,0,0x3,
        0,
        0,0,0,0,0,
        0,
        0xF0,0x0F,0,0,
    },
};

/*  [0] Tccspi [1] sts [2] spims */
Tcc317xRegisterConfig_t Tcc317x_LastDiversity[3] =
{
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,TCC317X_STREAM_THRESHOLD_WH,TCC317X_STREAM_THRESHOLD_WL,0,
        0x1,0xFC,0x10,0,0x10,0,0,0,0,0,0x1,
        0,
        0,0,0,0,0,
        0,
        0xF0,0x0F,TCC317X_STREAM_THRESHOLD_WH,TCC317X_STREAM_THRESHOLD_WL,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0x1,0xFF,0x10,0,0x10,0,0,0,0,0,0x1,
        0,
        0,0,0,0,0,
        0,
        0xF0,0x0F,0,0,
    },
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0x1,0xFF,0x10,0,0x10,0,0,0,0,0,0x1,
        0,
        0,0,0,0,0,
        0,
        0xF0,0x0F,0,0,
    },
};


Tcc317xOption_t Tcc317xOptionSingle =
{
    /* chip selection */
    TCC317X_TCC3171,
    /* board type */
    TCC317X_BOARD_SINGLE,
    /* select command interface */
    TCC317X_IF_I2C,
    /* select stream interface */
    TCC317X_STREAM_IO_SPIMS,
    /* changed i2c/tccspi address 0xa0-> 0xXX*/
    0xA2,
    /* pll option - 0x0F06 */
    0x0F06,
    /* osc clk*/
    19200,

    0,

    0,
    TCC317X_DIVERSITY_NONE,
    0, /* useLBAND */
    {
        /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
        /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
        /* irqMode */
        /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
        /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
        0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
        0,0x3F,0,0,0,0,0x00,0x3C,0,0,0,
        0,
        0x11,TCC317X_DLR<<2,0x10,0,0,
        0,
        0xF0,0x0F,0,0,
    },
};

Tcc317xOption_t Tcc317xOptionDualChainMode[2] =
{
    {
        /* chip selection */
        TCC317X_TCC3171,
        /* board type */
        TCC317X_BOARD_DUAL_CHAINMODE,
        /* select command interface */
        TCC317X_IF_I2C,
        /* select stream interface */
        TCC317X_STREAM_IO_SPIMS,
        /* changed i2c/tccspi address 0xa0-> 0xXX*/
        0xA2,
        /* pll option - 0x0F06 */
        0x0F06,
        /* osc clk*/
        19200,

        0,

        0,
        TCC317X_DIVERSITY_NONE,
	0, /* useLBAND */
        {
            /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
            /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
            /* irqMode */
            /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
            /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
            0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
            0,0x3F,0,0,0,0,0,0x3C,0,0,0,
            0,
            0x11,TCC317X_DLR<<2,0x10,0,0,
            0,
            0xF0,0x0F,0,0,
        },
    },
    {
        /* chip selection */
        TCC317X_TCC3171,
        /* board type */
        TCC317X_BOARD_DUAL_CHAINMODE,
        /* select command interface */
        TCC317X_IF_I2C,
        /* select stream interface */
        TCC317X_STREAM_IO_SPIMS,
        /* changed i2c/tccspi address 0xa0-> 0xXX*/
        0xA4,
        /* pll option - 0x0F06 */
        0x0F06,
        /* osc clk*/
        19200,

        0,

        0,
        TCC317X_DIVERSITY_NONE,
	0, /* useLBAND */
        {
            /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
            /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
            /* irqMode */
            /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
            /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
            0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
            0,0x3F,0x10,0,0x10,0,0,0x3C,0,0,0,
            0,
            0x11,TCC317X_DLR<<2,0x10,0,0,
            0,
            0xF0,0x0F,0,0,
        },
    }
};

Tcc317xOption_t Tcc317xOption2Diversity[2] =
{
    {
        /* chip selection */
        TCC317X_TCC3171,
        /* board type */
        TCC317X_BOARD_2DIVERSITY,
        /* select command interface */
        TCC317X_IF_I2C,
        /* select stream interface */
        TCC317X_STREAM_IO_SPIMS,
        /* changed i2c/tccspi address 0xa0-> 0xXX*/
        0xA2,
        /* pll option - 0x0F06 */
        0x0F06,
        /* osc clk*/
        19200,

        0,

        0,
        TCC317X_DIVERSITY_MASTER,
	0, /* useLBAND */
        {
            /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
            /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
            /* irqMode */
            /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
            /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
            0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
            0x0E,0x3F,0,0,0,0,0,0,0,0,0x2,
            0,
            0x11,TCC317X_DLR<<2,0x10,0,0,
            0,
            0xF0,0x0F,0,0,
        },
    },
    {
        /* chip selection */
        TCC317X_TCC3171,
        /* board type */
        TCC317X_BOARD_2DIVERSITY,
        /* select command interface */
        TCC317X_IF_I2C,
        /* select stream interface */
        TCC317X_STREAM_IO_SPIMS,
        /* changed i2c/tccspi address 0xa0-> 0xXX*/
        0xA4,
        /* pll option - 0x0F06 */
        0x0F06,
        /* osc clk*/
        19200,

        0,

        0,
        TCC317X_DIVERSITY_LAST,
	0, /* useLBAND */
        {
            /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
            /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
            /* irqMode */
            /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
            /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
            0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
            0x1,0xFF,0x10,0,0x10,0,0,0,0,0,0x1,
            0,
            0,0,0,0,0,
            0,
            0xF0,0x0F,0,0,
        },
    }
};

/* 3Diversity Support */
Tcc317xOption_t Tcc317xOption3Diversity[3] =
{
    {
        /* chip selection */
        TCC317X_TCC3171,
        /* board type */
        TCC317X_BOARD_3DIVERSITY,
        /* select command interface */
        TCC317X_IF_I2C,
        /* select stream interface */
        TCC317X_STREAM_IO_SPIMS,
        /* changed i2c/tccspi address 0xa0-> 0xXX*/
        0xA2,
        /* pll option - 0x0F06 */
        0x0F06,
        /* osc clk*/
        19200,

        0,
        0,
        TCC317X_DIVERSITY_MASTER,
        0, /* useLBAND */
        {
            /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
            /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
            /* irqMode */
            /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
            /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
            0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
            0x0E,0x3F,0,0,0,0,0,0,0,0,0x2,
            0,
            0x11,TCC317X_DLR<<2,0x10,0,0,
            0,
            0xF0,0x0F,0,0,
        },
    },
    {
        /* chip selection */
        TCC317X_TCC3171,
        /* board type */
        TCC317X_BOARD_3DIVERSITY,
        /* select command interface */
        TCC317X_IF_I2C,
        /* select stream interface */
        TCC317X_STREAM_IO_SPIMS,
        /* changed i2c/tccspi address 0xa0-> 0xXX*/
        0xA4,
        /* pll option - 0x0F06 */
        0x0F06,
        /* osc clk*/
        19200,

        0,
        0,
        TCC317X_DIVERSITY_MID,
        0, /* useLBAND */
        {
            /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
            /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
            /* irqMode */
            /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
            /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
            0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
            0x0F,0xFF,0x10,0,0x10,0,0,0,0,0,0x3,
            0,
            0,0,0,0,0,
            0,
            0xF0,0x0F,0,0,
        },
    },
    {
        /* chip selection */
        TCC317X_TCC3171,
        /* board type */
        TCC317X_BOARD_3DIVERSITY,
        /* select command interface */
        TCC317X_IF_I2C,
        /* select stream interface */
        TCC317X_STREAM_IO_SPIMS,
        /* changed i2c/tccspi address 0xa0-> 0xXX*/
        0xA6,
        /* pll option - 0x0F06 */
        0x0F06,
        /* osc clk*/
        19200,

        0,
        0,
        TCC317X_DIVERSITY_LAST,
        0, /* useLBAND */
        {
            /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
            /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
            /* irqMode */
            /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
            /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
            0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
            0x1,0xFF,0x10,0,0x10,0,0,0,0,0,0x1,
            0,
            0,0,0,0,0,
            0,
            0xF0,0x0F,0,0,
        },
    },
};

/* 3Diversity Support */
Tcc317xOption_t Tcc317xOption4Diversity[4] =
{
    {
        /* chip selection */
        TCC317X_TCC3171,
        /* board type */
        TCC317X_BOARD_4DIVERSITY,
        /* select command interface */
        TCC317X_IF_I2C,
        /* select stream interface */
        TCC317X_STREAM_IO_SPIMS,
        /* changed i2c/tccspi address 0xa0-> 0xXX*/
        0xA2,
        /* pll option - 0x0F06 */
        0x0F06,
        /* osc clk*/
        19200,

        0,
        0,
        TCC317X_DIVERSITY_MASTER,
        0, /* useLBAND */
        {
            /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
            /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
            /* irqMode */
            /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
            /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
            0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
            0x0E,0x3F,0,0,0,0,0,0,0,0,0x2,
            0,
            0x11,TCC317X_DLR<<2,0x10,0,0,
            0,
            0xF0,0x0F,0,0,
        },
    },
    {
        /* chip selection */
        TCC317X_TCC3171,
        /* board type */
        TCC317X_BOARD_4DIVERSITY,
        /* select command interface */
        TCC317X_IF_I2C,
        /* select stream interface */
        TCC317X_STREAM_IO_SPIMS,
        /* changed i2c/tccspi address 0xa0-> 0xXX*/
        0xA4,
        /* pll option - 0x0F06 */
        0x0F06,
        /* osc clk*/
        19200,

        0,
        0,
        TCC317X_DIVERSITY_MID,
        0, /* useLBAND */
        {
            /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
            /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
            /* irqMode */
            /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
            /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
            0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
            0x0F,0xFF,0x10,0,0x10,0,0,0,0,0,0x3,
            0,
            0,0,0,0,0,
            0,
            0xF0,0x0F,0,0,
        },
    },
    {
        /* chip selection */
        TCC317X_TCC3171,
        /* board type */
        TCC317X_BOARD_4DIVERSITY,
        /* select command interface */
        TCC317X_IF_I2C,
        /* select stream interface */
        TCC317X_STREAM_IO_SPIMS,
        /* changed i2c/tccspi address 0xa0-> 0xXX*/
        0xA6,
        /* pll option - 0x0F06 */
        0x0F06,
        /* osc clk*/
        19200,

        0,
        0,
        TCC317X_DIVERSITY_MID,
        0, /* useLBAND */
        {
            /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
            /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
            /* irqMode */
            /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
            /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
            0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
            0x0F,0xFF,0x10,0,0x10,0,0,0,0,0,0x3,
            0,
            0,0,0,0,0,
            0,
            0xF0,0x0F,0,0,
        },
    },
    {
        /* chip selection */
        TCC317X_TCC3171,
        /* board type */
        TCC317X_BOARD_4DIVERSITY,
        /* select command interface */
        TCC317X_IF_I2C,
        /* select stream interface */
        TCC317X_STREAM_IO_SPIMS,
        /* changed i2c/tccspi address 0xa0-> 0xXX*/
        0xA8,
        /* pll option - 0x0F06 */
        0x0F06,
        /* osc clk*/
        19200,

        0,
        0,
        TCC317X_DIVERSITY_LAST,
        0, /* useLBAND */
        {
            /* streamDataConfig0 , streamDataConfig1, streamDataConfig2, streamDataConfig3 */
            /* gpioAltH, gpioAltL, gpioDrH, gpioDrL, gpioLrH, gpioLrL, gpioDrvH, gpioDrvL, gpiopeH, gpiopeL, divio */
            /* irqMode */
            /* periConfig0, periConfig1, periConfig2, periConfig3, periConfig4, smxConfig0 */
            /* bufferConfig0, bufferConfig1, bufferConfig30, bufferConfig31 */
            0xFF,0,TCC317X_STREAM_THRESHOLD_SPI,0,
            0x1,0xFF,0x10,0,0x10,0,0,0,0,0,0x1,
            0,
            0,0,0,0,0,
            0,
            0xF0,0x0F,0,0,
        },
    },
};

#define SAMPLE_SINGLE
/*
#define SAMPLE_DUAL
#define SAMPLE_2DIVERSITY
*/

#if 0
void Tcc317xSampleMain ()
{
    /* example source */
    Tcc317xService service;
    I32U freqTable[4] = {181280, 183008, 205280, 207008};
    I32S i;
    I08U ASMDATA[8];    /* only example*/

    service.bitrate = 0;
    service.serviceType = SRVTYPE_DMB;
    service.pType = 1;
    service.identifier = 12;
    service.cuSize = 5;
    service.startCu = 0;
    service.reConfig = 2;
    service.rsOn = 1;
    service.pLevel = 1;
    service.bitrate =1;
    
#if defined (SAMPLE_SINGLE)
    Tcc317xApiOpen (0, &Tcc317xOptionSingle, sizeof(Tcc317xOptionSingle));
    #if defined (_INCLUDED_ASM_FILE_)
    Tcc317xApiInit (0, &TCC317X_BOOT_DATA_TDMB_LBAND[0], TCC317X_BOOT_SIZE_TDMB_LBAND);
    #else
    Tcc317xApiInit (0, &ASMDATA[0], 8);
    #endif // _INCLUDED_ASM_FILE_

    for(i=0; i<4; i++)
    {
        Tcc317xApiStreamStop(0);
        Tcc317xMonitoringInit(0, 0);
        Tcc317xStreamParserInit(0);
        Tcc317xApiChannelSearch(0, freqTable[i], NULL);
    }

    Tcc317xApiStreamStop(0);
    Tcc317xMonitoringInit(0, 0);
    Tcc317xStreamParserInit(0);
    Tcc317xApiChannelSelect(0, freqTable[2], NULL);
    Tcc317xApiAddService(0, &service);

    Tcc317xApiStreamStop(0);
    Tcc317xMonitoringInit(0, 0);
    Tcc317xStreamParserInit(0);
    Tcc317xApiChannelSelect(0, freqTable[1], NULL);
    Tcc317xApiAddService(0, &service);

    Tcc317xApiStreamStop(0);
    Tcc317xApiClose(0);
#elif defined (SAMPLE_DUAL)
    Tcc317xApiOpen (0, &Tcc317xOptionDualChainMode, sizeof(Tcc317xOption_t));
    Tcc317xApiOpen (1, &Tcc317xOptionDualChainMode, sizeof(Tcc317xOption_t));

    #if defined (_INCLUDED_ASM_FILE_)
    Tcc317xApiInit (0, &TCC317X_BOOT_DATA_TDMB_LBAND[0], TCC317X_BOOT_SIZE_TDMB_LBAND);
    Tcc317xApiInit (1, &TCC317X_BOOT_DATA_TDMB_LBAND[0], TCC317X_BOOT_SIZE_TDMB_LBAND);
    #else
    Tcc317xApiInit (0, &ASMDATA[0], 8);
    Tcc317xApiInit (1, &ASMDATA[0], 8);
    #endif // _INCLUDED_ASM_FILE_
    
    for(i=0; i<4; i++)
    {
        Tcc317xApiStreamStop(0);
        Tcc317xMonitoringInit(0, 0);
        Tcc317xStreamParserInit(0);
        Tcc317xApiChannelSearch(0, freqTable[i], NULL);
    }
    Tcc317xApiStreamStop(0);
    Tcc317xMonitoringInit(0, 0);
    Tcc317xStreamParserInit(0);
    Tcc317xApiChannelSelect(0, freqTable[2], NULL);
    Tcc317xApiAddService(0, &service);

    Tcc317xApiStreamStop(0);
    Tcc317xMonitoringInit(0, 0);
    Tcc317xStreamParserInit(0);
    Tcc317xApiChannelSelect(1, freqTable[3], NULL);
    Tcc317xApiAddService(1, &service);

    Tcc317xApiStreamStop(0);
    Tcc317xApiClose(0);
#elif defined (SAMPLE_2DIVERSITY)
    Tcc317xApiOpen (0, &Tcc317xOption2Diversity, sizeof(Tcc317xOption2Diversity));
    #if defined (_INCLUDED_ASM_FILE_)
    Tcc317xApiInit (0, &TCC317X_BOOT_DATA_TDMB_LBAND[0], TCC317X_BOOT_SIZE_TDMB_LBAND);
    #else
    Tcc317xApiInit (0, &ASMDATA[0], 8);
    #endif // _INCLUDED_ASM_FILE_

    for(i=0; i<4; i++)
    {
        Tcc317xApiStreamStop(0);
        Tcc317xMonitoringInit(0, 0);
        Tcc317xStreamParserInit(0);
        Tcc317xApiChannelSearch(0, freqTable[i], NULL);
    }
    Tcc317xApiStreamStop(0);
    Tcc317xMonitoringInit(0, 0);
    Tcc317xStreamParserInit(0);
    Tcc317xApiChannelSelect(0, freqTable[2], NULL);
    Tcc317xApiAddService(0, &service);

    Tcc317xApiStreamStop(0);
    Tcc317xMonitoringInit(0, 0);
    Tcc317xStreamParserInit(0);
    Tcc317xApiChannelSelect(0, freqTable[1], NULL);
    Tcc317xApiAddService(0, &service);

    Tcc317xApiStreamStop(0);
    Tcc317xApiClose(0);
#endif 
}
#if 0
#include "tcc317x_boot.h"
void Tcc317xSampleMain_Single ()
{
    /* example source */
    Tcc317xService service;
    Tcc317xStatus_t SigStat;
    Tcc317xOption_t Tcc317xOption[4];
    
    /* host - [BB#0] PWR ON */
    /* host - Command Interface open */
    /* host - Stream Interface Initialize */
    /* host - [BB#0] RST HIGH-LOW-HIGH */

    TcpalMemcpy (&Tcc317xOption[0], &Tcc317xOptionSingle, sizeof (Tcc317xOption_t));
    Tcc317xOption[0].useLBAND = 1;
    
    Tcc317xApiOpen (0, &Tcc317xOption[0], sizeof(Tcc317xOption_t));
    Tcc317xApiInit (0, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);

    /* host - Stream Interface Stop */
    Tcc317xStreamParserInit(0);
    Tcc317xApiChannelSearch(0, 181280, NULL);
    Tcc317xMonitoringInit(0, 0);
    /* host - Stream Interface Start */
    Tcc317xApiAddService(0, &service);

    while(1) {
	    Tcc317xGetSignalState(0, 0, &SigStat);
	    TcpalSleep (1000);
    }

    /* host - Stream Interface Stop */
    Tcc317xApiStreamStop(0);
    Tcc317xApiClose(0);
    /* host - [BB#0] PWR DN*/
    /* host - [BB#0] RST LOW */
    /* host - Command Interface close */
    /* host - Stream Interface close */
}

void Tcc317xSampleMain_Dual_Normal ()
{
    /* example source */
    Tcc317xService service;
    Tcc317xStatus_t SigStat;
    Tcc317xOption_t Tcc317xOption[4];
    int bbIndex = 0;

    /* host - [BB#0] PWR ON */
    /* host - Command Interface open */
    /* host - Stream Interface Initialize */
    /* host - [BB#0] RST HIGH-LOW-HIGH */
    /* host - [BB#1] RST LOW (Disable BB#1) */

    TcpalMemcpy (&Tcc317xOption[0], &Tcc317xOptionSingle, sizeof (Tcc317xOption_t));
    TcpalMemcpy (&Tcc317xOption[1], &Tcc317xOptionSingle, sizeof (Tcc317xOption_t));

    Tcc317xOption[0].useLBAND = 1;
    Tcc317xOption[1].useLBAND = 1;

    Tcc317xOption[0].address = 0xA2;
    Tcc317xOption[1].address = 0xA4;

    Tcc317xApiOpen (0, &Tcc317xOption[0], sizeof(Tcc317xOption_t));
    Tcc317xApiInit (0, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);

    /* host - [BB#1] PWR ON */
    /* host - [BB#1] RST HIGH-LOW-HIGH */
    Tcc317xApiOpen (1, &Tcc317xOption[1], sizeof(Tcc317xOption_t));
    Tcc317xApiInit (1, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);

    /* host - Stream Interface Stop */
    Tcc317xStreamParserInit(bbIndex);
    Tcc317xApiChannelSearch(bbIndex, 181280, NULL);
    Tcc317xMonitoringInit(bbIndex, 0);
    /* host - Stream Interface Start */
    Tcc317xApiAddService(bbIndex, &service);

    while(1) {
	    Tcc317xGetSignalState(bbIndex, 0, &SigStat);
	    TcpalSleep (1000);
    }

    /* host - Stream Interface Stop */
    Tcc317xApiStreamStop(0);
    Tcc317xApiClose(0);
    /* host - [BB#0] PWR DN*/
    /* host - [BB#0] RST LOW */
    
    Tcc317xApiStreamStop(1);
    Tcc317xApiClose(1);
    /* host - [BB#1] PWR DN*/
    /* host - [BB#1] RST LOW */

    /* host - Command Interface close */
    /* host - Stream Interface close */
}

void Tcc317xSampleMain_Dual_ChainMode ()
{
    /* example source */
    Tcc317xService service;
    Tcc317xStatus_t SigStat;
    Tcc317xOption_t Tcc317xOption[4];
    int bbIndex = 0;

    /* host - [BB#0,BB#1] PWR ON */
    /* host - Command Interface open */
    /* host - Stream Interface Initialize */
    /* host - [BB#0, BB#1] RST HIGH-LOW-HIGH */
    TcpalMemcpy (&Tcc317xOption[0], &Tcc317xOptionDualChainMode[0], sizeof (Tcc317xOption_t));
    TcpalMemcpy (&Tcc317xOption[1], &Tcc317xOptionDualChainMode[1], sizeof (Tcc317xOption_t));

    Tcc317xOption[0].useLBAND = 1;
    Tcc317xOption[1].useLBAND = 1;

    Tcc317xApiOpen (0, &Tcc317xOption[0], sizeof(Tcc317xOption_t));
    Tcc317xApiOpen (1, &Tcc317xOption[1], sizeof(Tcc317xOption_t));

    Tcc317xApiInit (0, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);
    Tcc317xApiInit (1, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);

    /* host - Stream Interface Stop */
    Tcc317xStreamParserInit(bbIndex);
    Tcc317xApiChannelSearch(bbIndex, 181280, NULL);
    Tcc317xMonitoringInit(bbIndex, 0);
    /* host - Stream Interface Start */
    Tcc317xApiAddService(bbIndex, &service);

    while(1) {
	    Tcc317xGetSignalState(bbIndex, 0, &SigStat);
	    TcpalSleep (1000);
    }

    /* host - Stream Interface Stop */
    Tcc317xApiStreamStop(0);
    Tcc317xApiStreamStop(1);

    Tcc317xApiClose(0);
    Tcc317xApiClose(1);
    /* host - [BB#0,BB#1] PWR DN*/
    /* host - [BB#0,BB#1] RST LOW */

    /* host - Command Interface close */
    /* host - Stream Interface close */
}

void Tcc317xSampleMain_2Diversity ()
{
    /* example source */
    Tcc317xService service;
    Tcc317xStatus_t SigStat;
    Tcc317xOption_t Tcc317xOption[4];

    /* host - [BB#0,BB#1] PWR ON */
    /* host - Command Interface open */
    /* host - Stream Interface Initialize */
    /* host - [BB#0, BB#1] RST HIGH-LOW-HIGH */
    TcpalMemcpy (&Tcc317xOption[0], &Tcc317xOption2Diversity[0], sizeof (Tcc317xOption_t)*2);
    Tcc317xOption[0].useLBAND = 1;
    Tcc317xOption[1].useLBAND = 1;

    Tcc317xApiOpen (0, &Tcc317xOption[0], sizeof(Tcc317xOption_t)*2);
    Tcc317xApiInit (0, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);

    /* host - Stream Interface Stop */
    Tcc317xStreamParserInit(0);
    Tcc317xApiChannelSearch(0, 181280, NULL);
    Tcc317xMonitoringInit(0, 0);
    /* host - Stream Interface Start */
    Tcc317xApiAddService(0, &service);

    while(1) {
	    Tcc317xGetSignalState(0, 0, &SigStat);
	    Tcc317xGetSignalState(0, 1, &SigStat);
	    TcpalSleep (1000);
    }

    /* host - Stream Interface Stop */
    Tcc317xApiStreamStop(0);
    Tcc317xApiClose(0);
    /* host - [BB#0,BB#1] PWR DN*/
    /* host - [BB#0,BB#1] RST LOW */

    /* host - Command Interface close */
    /* host - Stream Interface close */
}

void Tcc317xSampleMain_3Diversity ()
{
    /* example source */
    Tcc317xService service;
    Tcc317xStatus_t SigStat;
    Tcc317xOption_t Tcc317xOption[4];

    /* host - [BB#0,BB#1,BB#2] PWR ON */
    /* host - Command Interface open */
    /* host - Stream Interface Initialize */
    /* host - [BB#0, BB#1,BB#2] RST HIGH-LOW-HIGH */
    TcpalMemcpy (&Tcc317xOption[0], &Tcc317xOption3Diversity[0], sizeof (Tcc317xOption_t)*3);
    Tcc317xOption[0].useLBAND = 1;
    Tcc317xOption[1].useLBAND = 1;
    Tcc317xOption[2].useLBAND = 1;

    Tcc317xApiOpen (0, &Tcc317xOption[0], sizeof(Tcc317xOption_t)*3);
    Tcc317xApiInit (0, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);

    /* host - Stream Interface Stop */
    Tcc317xStreamParserInit(0);
    Tcc317xApiChannelSearch(0, 181280, NULL);
    Tcc317xMonitoringInit(0, 0);
    /* host - Stream Interface Start */
    Tcc317xApiAddService(0, &service);

    while(1) {
	    Tcc317xGetSignalState(0, 0, &SigStat);
	    Tcc317xGetSignalState(0, 1, &SigStat);
	    Tcc317xGetSignalState(0, 2, &SigStat);
	    TcpalSleep (1000);
    }

    /* host - Stream Interface Stop */
    Tcc317xApiStreamStop(0);
    Tcc317xApiClose(0);
    /* host - [BB#0,BB#1,BB#2] PWR DN*/
    /* host - [BB#0,BB#1,BB#2] RST LOW */

    /* host - Command Interface close */
    /* host - Stream Interface close */
}

void Tcc317xSampleMain_3Diversity_NotChainMode ()
{
    /* example source */
    Tcc317xService service;
    Tcc317xStatus_t SigStat;
    Tcc317xOption_t Tcc317xOption[4];

    /* host - [BB#0] PWR ON */
    /* host - Command Interface open */
    /* host - Stream Interface Initialize */
    /* host - [BB#0] RST HIGH-LOW-HIGH */
    /* host - [BB#1] RST LOW (Disable BB#1) */
    /* host - [BB#2] RST LOW (Disable BB#2) */

    TcpalMemcpy (&Tcc317xOption[0], &Tcc317xOption3Diversity[0], sizeof (Tcc317xOption_t)*3);
    Tcc317xOption[0].useLBAND = 1;
    Tcc317xOption[1].useLBAND = 1;
    Tcc317xOption[2].useLBAND = 1;

    Tcc317xApiOpen (0, &Tcc317xOption[0], sizeof(Tcc317xOption_t)*3);
    Tcc317xApiInit_DiversityNormalMode(0, 0, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);

    /* host - [BB#1] PWR ON */
    /* host - [BB#1] RST HIGH-LOW-HIGH */
    Tcc317xApiInit_DiversityNormalMode(0, 1, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);

    /* host - [BB#2] PWR ON */
    /* host - [BB#2] RST HIGH-LOW-HIGH */
    Tcc317xApiInit_DiversityNormalMode(0, 2, &TCC317X_BOOT_DATA_TDMB[0], TCC317X_BOOT_SIZE_TDMB);

    /* host - Stream Interface Stop */
    Tcc317xStreamParserInit(0);
    Tcc317xApiChannelSearch(0, 181280, NULL);
    Tcc317xMonitoringInit(0, 0);
    /* host - Stream Interface Start */
    Tcc317xApiAddService(0, &service);

    while(1) {
	    Tcc317xGetSignalState(0, 0, &SigStat);
	    Tcc317xGetSignalState(0, 1, &SigStat);
	    Tcc317xGetSignalState(0, 2, &SigStat);
	    TcpalSleep (1000);
    }

    /* host - Stream Interface Stop */
    Tcc317xApiStreamStop(0);
    Tcc317xApiClose(0);
    /* host - [BB#0,BB#1,BB#2] PWR DN*/
    /* host - [BB#0,BB#1,BB#2] RST LOW */

    /* host - Command Interface close */
    /* host - Stream Interface close */
}

#endif
#endif // 0
