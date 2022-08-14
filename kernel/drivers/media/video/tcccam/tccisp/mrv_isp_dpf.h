/****************************************************************************
One line to give the program's name and a brief idea of what it does.
Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/


#ifndef _MRV_ISP_DPF_H
#define _MRV_ISP_DPF_H

#if( MARVIN_FEATURE_DPF == MARVIN_FEATURE_DPF_V1 )


/********************************
 * public variables
 ********************************/
//[DPF_CONFIG]

//[DPF_SPATIAL]
typedef struct
{
    UINT8 ucWeightCoeff1;
    UINT8 ucWeightCoeff2;
    UINT8 ucWeightCoeff3;
    UINT8 ucWeightCoeff4;
    UINT8 ucWeightCoeff5;
    UINT8 ucWeightCoeff6;
}tsDpfSpatial;

//[DPF_MODE]
typedef struct
{
    UINT32 ulDpfNlGainEnable;
    UINT32 ulDpfLscGainComp;
    UINT32 ulDpfAwbGainComp;
    UINT32 ulRBFilterSize;
    UINT32 ulRFilterOff;
    UINT32 ulGRFilterOff;
    UINT32 ulGBFilterOff;
    UINT32 ulBFilterOff;
    UINT32 ulDpfEnable;    
}tsDpfMode;

typedef struct
{
    tsDpfMode           tDpfMode;         
    tsDpfSpatial        tDpfSpatialG;   
    tsDpfSpatial        tDpfSpatialRB;      
}tsMrvDpfCfg;

typedef struct
{
    UINT8 ucInvWeightR;
    UINT8 ucInvWeightG;
    UINT8 ucInvWeightB;
}tsMrvDpfInvStrength;

/********************************
 * public functions
 ********************************/
RESULT MrvIspDpf_Enable( void );
RESULT MrvIspDpf_Disable( void );
RESULT MrvIspDpf_SetConfig (const tsMrvDpfCfg *ptMrvDpfCfg );
RESULT MrvIspDpf_GetConfig ( tsMrvDpfCfg *ptMrvDpfCfg );
RESULT MrvIspDpf_SetNoiseLevel (const tsMrvDpfNLLElem *ptMrvDpfNLLElem);
RESULT MrvIspDpf_GetNoiseLevel ( tsMrvDpfNLLElem *ptMrvDpfNLLElem);
RESULT MrvIspDpf_SetNLGain (const tsMrvDpfNLGain *ptMrvDpfNLGain );
RESULT MrvIspDpf_GetNLGain ( tsMrvDpfNLGain *ptMrvDpfNLGain );
RESULT MrvIspDpf_SetStrength (const tsMrvDpfInvStrength *ptMrvDpfInvStrength);
RESULT MrvIspDpf_GetStrength (tsMrvDpfInvStrength *ptMrvDpfInvStrength);

#elif ( MARVIN_FEATURE_DPF != MARVIN_FEATURE_EXIST_NOT )
#error unknown value for MARVIN_FEATURE_DPF
#endif  // MARVIN_FEATURE_DPF

#endif // _MRV_ISP_DPF_H

