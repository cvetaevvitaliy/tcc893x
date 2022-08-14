/****************************************************************************
 *   FileName    : tcc317x_rf.c
 *   Description : Rf Functions
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

#include "tcc317x_rf.h"
#include "tcc317x_command_control.h"

I32U Tcc317x_RF_REG_DAB_VER = 0x00010001;
I32U Tcc317x_RF_REG_DMB_VER = 0x00010000;

/* extern function */
I64U Tcc317xDiv64 (I64U x, I64U y);
extern void TcpalSleepUs (I32S _us);

#define SCALE       22
#define FIXED(x)    (x<<SCALE)
#define MUL(A,B)    ((A*B)>>SCALE)
#define DIV(A,B)    (Tcc317xDiv64((A<<SCALE), B))

#ifndef Bit0
#define Bit31       0x80000000
#define Bit30       0x40000000
#define Bit29       0x20000000
#define Bit28       0x10000000
#define Bit27       0x08000000
#define Bit26       0x04000000
#define Bit25       0x02000000
#define Bit24       0x01000000
#define Bit23       0x00800000
#define Bit22       0x00400000
#define Bit21       0x00200000
#define Bit20       0x00100000
#define Bit19       0x00080000
#define Bit18       0x00040000
#define Bit17       0x00020000
#define Bit16       0x00010000
#define Bit15       0x00008000
#define Bit14       0x00004000
#define Bit13       0x00002000
#define Bit12       0x00001000
#define Bit11       0x00000800
#define Bit10       0x00000400
#define Bit9        0x00000200
#define Bit8        0x00000100
#define Bit7        0x00000080
#define Bit6        0x00000040
#define Bit5        0x00000020
#define Bit4        0x00000010
#define Bit3        0x00000008
#define Bit2        0x00000004
#define Bit1        0x00000002
#define Bit0        0x00000001
#define BitNONE     0x00000000
#endif

#ifndef BITSET
#define BITSET(X, MASK)             ( (X) |= (I32U)(MASK) )
#endif
#ifndef BITCLR
#define BITCLR(X, MASK)             ( (X) &= ~((I32U)(MASK)) )
#endif

extern Tcc317xHandle_t Tcc317xHandle[TCC317X_MAX][TCC317X_DIVERSITY_MAX];

/* RF RegMap */
/* rf_config0(0x04)(32bits) register bit map - Power down
| 31 - 21  |   20  |   19  |  18  |  17  |   16  |
| RESERVED |PD_XTAL|PD_XBUF|PD_CP2|PD_AFT|PD_RSSI|
|   15	   |  14 |  13 |   12	  |  11  |  10	|   9	|   8  |  7   |   6  |	5   |	4  |   3    |	2   |	1  |   0  |
|PD_RF_BIAS|PD_VL|PD_LL|PD_BB_BIAS|PD_LPF|PD_PGA|PD_DCOC|PD_HPF|PD_BGR|PD_VCO|PD_PLL|PD_SDM|PD_LOGEN|PD_MTIA|PD_MLB|PD_MVB|
*****************************************************************************************************************************/

/* rf_config0(0x05)(32bits) register bit map - VCO / PLL
|   31 - 30   |    29-28    |   27-26    |    25-24   |    23-22     |    21-20    |    19-18    |    17-16    |
|CP2_BIAS<1:0>|CP1_BIAS<1:0>|VCO_DUM<1:0>|VCO_COR<1:0>|XBUF_BIAS<1:0>|LOL_DIV2<1:0>|LOV_DIV8<1:0>|LOV_DIV4<1:0>|
|   15-8   |    7   |    6   |   5    |   4     |   3    |   2    |   1    |  0     |
|AFCNT<7:0>|AFCNT<8>|AFC_VDCS|AFC_OSEL|VCON_OPEN|RESERVED|NCNT_SDC|RESERVED|AUTO_RST|
*****************************************************************************************************************************/

/* rf_config0(0x06)(32bits) register bit map - VCO / PLL
|  31	|   30	|   29	 |   28   |   27-24  |	 23-20	|   19-16  |
|PLL_MUX|EN_TVCO|RESERVED|RCNT_RDC|LF_C3<3:0>|LF_R2<3:0>|LF_C2<3:0>|
|   15-12  |   11-8   |      7-6     |	 5-4	 |    3   |    2-0     |
|LF_R1<3:0>|LF_C1<3:0>|TVCO_BIAS<1:0>|CP_RST<1:0>|RESERVED|ICP_CON<2:0>|
*****************************************************************************************************************************/

/* rf_config0(0x07)(32bits) register bit map
| 31-14  |   13-8    |	 7-5  |   4   |   3-0  |
|RESERVED|AFCODE<5:0>|RESERVED|PFD_POL|RESERVED|
*****************************************************************************************************************************/	   

/* rf_config0(0x08)(32bits) register bit map
|   31	 |  30	 |  29-8 |  7-0 |
|DIV_MODE|RST_PLL|F<21:0>|N<7:0>|
*****************************************************************************************************************************/	      

/* rf_config0(0x09)(32bits) register bit map
| 31-22  |  21-20 |	19-16	 |  15	 |  14	 |   13-8    |	    7-4      |	    3-0        |
|RESERVED|LFS<1:0>|MIX_TIAIC<3:0>|MI_TEST|MQ_TEST|MIX_GC<5:0>|VHF_LNA_CT<3:0>|LBAND_LNA_CT<3:0>|
*****************************************************************************************************************************/	      

/* rf_config0(0x0A)(32bits) register bit map
|     31-28	 |    27   |   26-24   |     23-21    |   20-16    |
|LPF1ST_ICON<3:0>|HPF_DM_PD|CMR_FT<2:0>|REC_ICONR<2:0>|CMR2_GT<4:0>|
|    15-13     |    12-8    |	   7-4	   |	 3-0	  |
|REC_ICONL<2:0>|CMR1_GT<4:0>|RSSI_ICON<3:0>|COMP_ICON<3:0>|
*****************************************************************************************************************************/

/* rf_config0(0x0B)(32bits) register bit map
|   31	 |    30-29   |    28-24    | 23-14  |	 13  |	 12-8	  |    7-4	|     3-1     |   0  |
|RESERVED|DCOC_FT<1:0>|DCOC_PDS<4:0>|RESERVED|PD_INTR|FTCTRIM<4:0>|LPF_ICON<3:0>|CLK_MODE<2:0>|GFT_EN|
*****************************************************************************************************************************/

/* rf_config0(0x0C)(32bits) register bit map - PGA
|   31-24   |	23-20  |  19-16   |    15-12	 |  11-8  |	7-0	|
|IBU_TU<7:0>|VRG2B<3:0>|VRG1B<3:0>|DCOC_ICON<3:0>|reserved|PGA_ICON<7:0>|
*****************************************************************************************************************************/

/* rf_config0(0x0D)(32bits) register bit map - LPF2
| 31-24  |    23-16   |  15-13 |    12-10   |	9-8  |	 7   |	6-5   |   4-3  |  2-0	|  
|Reserved|LPF_3DB<7:0>|Reserved|LPF_HSM<2:0>|C6M<1:0>|LPF_HLM|C4M<1:0>|C2M<1:0>|C2D<2:0>|
*****************************************************************************************************************************/

/* rf_config0(0x0E)(32bits) register bit map - GAIN
|   31-24   |   23   |   22-16   |  15-11   |   10   |    9-8    |    7-4   |    3   |    2   |    1-0    |  
|DPGAGS<7:0>|Reserved|AFT_GS<6:0>|LL_GS<4:0>|LLNA_LOS|LL_LGS<1:0>|VL_GS<3:0>|Reserved|VLNA_LOS|VL_LGS<1:0>|
****************************************************************************************************************************/

I32S Tcc317xRfInit (I32S _moduleIndex, I32S _diversityIndex, I32S _band)
{
    if (_band == 0)     /* band III */
    {
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x02, 0x00030000);
        
	if(Tcc317xHandle[_moduleIndex][_diversityIndex].options.useLBAND ==0)
	        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x04, 0x00002002);  /*<13>,<1>*/
	else
		Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x04, 0x00000002);  /*<13>,<1>*/
		
        /* modified 20130927
		Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x05, 0x5500640b);
		*/
		Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x05, 0x5f04640b);   /* freq value : 176.640 */
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x06, 0x1AADA754);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x07, 0x00002000);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x08, 0x555555c7);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x09, 0x000D2299);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x0a, 0xD87060DD);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x0b, 0x00000e72);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x0c, 0x02AE7077);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x0d, 0x00240376);
	if(Tcc317xHandle[_moduleIndex][_diversityIndex].options.useLBAND ==0)
	        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x0e, 0x7F7F0722); /* Vout, V=LNA mid, L=LNA mid */
	else
	        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x0e, 0x7F7F2522); /* Vout, V=LNA mid, L=LNA mid */
    }
    else        /* LBAND */
    {
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x02, 0x00030000);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x04, 0x00004001); /*<14>,<0>*/
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x05, 0x0F00640B);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x06, 0x0AADA754);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x07, 0x00002000);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x08, 0xE9F92C4C);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x09, 0x000d2193);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x0a, 0xD87060DD);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x0b, 0x00000e72);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x0c, 0x02FE7077);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x0d, 0x00240376);
        Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x0e, 0x7F7F1207); /* Lout, V=LNA mid, L=LNA mid */
    }

    return TCC317X_RETURN_SUCCESS;
}

void Tcc317xRfTune (void * _handle, I32S _moduleIndex, I32S _diversityIndex, I32S _freq_khz, I32S _bw_khz, I32S _oscClk)
{
    I64U      N, F;
    I64U      Flo, VCO_DIV, FOffset, Fvco, FR, PLL_MODE;
    I64U      N_int, intF, intVCO_DIV;
    I64U      fXtal, fpfd, f_freq_khz;
    I64U      Temp, Temp1, Temp2;
    I32U      OSCCLK;
    I32U      DIV_MODE;
    I32S      RST_PLL = 1;
    I32S      band = 0;
    I32U      temp32;
    Tcc317xHandle_t *t317xhandle;

    t317xhandle = (Tcc317xHandle_t *)(_handle);

    FOffset = 0;

    if (_freq_khz > 1000000)    /* l-band */
        band = 1;

    /* band III && TCC3170 */
    if(t317xhandle->options.chipSelection == TCC317X_TCC3170 && band == 0)
    {
        if (_freq_khz <= 190736)
            Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x09, 0x003d2299);
        else
            Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x09, 0x001d2299);
    }

    Tcc317xRfRegisterRead (_moduleIndex, _diversityIndex, 0x06, &temp32);
    Temp = temp32;
    if (((Temp >> 28) & 0x01) == 0)
        FR = 0;
    else
        FR = 1;

    if (((Temp >> 31) & 0x01) == 0)
        PLL_MODE = 2;
    else
        PLL_MODE = 4;

    OSCCLK = _oscClk;
    fXtal = OSCCLK;
    f_freq_khz = _freq_khz;

    /* Calculate PLL */
    if (f_freq_khz < 250000)
    {
        /* VHF */
        DIV_MODE = 0x00;
        fpfd = fXtal >> FR;
        VCO_DIV = 16;

        Flo = f_freq_khz - FOffset;
        Fvco = Flo * VCO_DIV;

        Temp1 = Fvco << FR;
        Temp2 = PLL_MODE * fXtal;
        N = DIV (Temp1, Temp2);
        N_int = (N >> SCALE) << SCALE;
        F = ((N - N_int) * (2 << 21)) >> SCALE;

        if (N_int < (19 << SCALE))
        {
            FR = 1;
            fpfd = fXtal >> FR;
            VCO_DIV = 16;
            Flo = f_freq_khz - FOffset;
            Fvco = Flo * VCO_DIV;

            Temp1 = Fvco * FR;
            Temp2 = PLL_MODE * fXtal;
            N = DIV (Temp1, Temp2);
            N_int = (N >> SCALE) << SCALE;
            F = ((N - N_int) * (2 << 21)) >> SCALE;
        }
        intF = F;
        intVCO_DIV = VCO_DIV;
    }
    else
    {
        /* LBAND */
        DIV_MODE = 0x01;
        fpfd = fXtal >> FR;
        VCO_DIV = 2;

        Flo = f_freq_khz - FOffset;
        Fvco = Flo * VCO_DIV;

        Temp1 = Fvco << FR;
        Temp2 = PLL_MODE * fXtal;
        N = DIV (Temp1, Temp2);
        N_int = (N >> SCALE) << SCALE;
        F = ((N - N_int) * (2 << 21)) >> SCALE;

        if (N_int < (19 << SCALE))
        {
            FR = 1;

            VCO_DIV = 2;
            Flo = f_freq_khz - FOffset;
            Fvco = Flo * VCO_DIV;

            Temp1 = Fvco << FR;
            Temp2 = PLL_MODE * fXtal;
            N = DIV (Temp1, Temp2);
            N_int = (N >> SCALE) << SCALE;
            F = ((N - N_int) * (2 << 21)) >> SCALE;
        }
        intF = F;
        intVCO_DIV = VCO_DIV;
    }

    Temp = ((N_int >> SCALE) & 0xFF);
    Temp |= ((intF & 0x3FFFFF) << 8);
    Temp |= ((RST_PLL & 0x01) << 30);
    Temp |= ((DIV_MODE & 0x01) << 31);
    Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x08, (I32U)Temp);

    Tcc317xRfRegisterRead (_moduleIndex, _diversityIndex, 0x06, &temp32);
    Temp = temp32;

    if (FR == 0)
        BITCLR (Temp, Bit28);
    else
        BITSET (Temp, Bit28);

    temp32 = (I32U)((Temp & 0xFFFFFFFF));
    Tcc317xRfRegisterWrite (_moduleIndex, _diversityIndex, 0x06, temp32);
}

I64U Tcc317xDiv64 (I64U x, I64U y)
{
    I64U      a, b, q, counter;

    q = 0;
    if (y != 0)
    {
        while (x >= y)
        {
            a = x >> 1;
            b = y;
            counter = 1;
            while (a >= b)
            {
                b <<= 1;
                counter <<= 1;
            }
            x -= b;
            q += counter;
        }
    }
    return q;
}
