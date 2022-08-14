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


#ifndef _MRV_ISP_HIST_CALC_H
#define _MRV_ISP_HIST_CALC_H


/******************************************************************************
 * Public Histogram Calculation Defines and Typedefs
******************************************************************************/

#define MRV_HIST_DATA_BIN_ARR_SIZE 16
    typedef struct
    {
        UINT16 usHistBin[MRV_HIST_DATA_BIN_ARR_SIZE];
    } tsMrvHistDataBin;
/******************************************************************************
 * Public Histogram Calculation Interface
 ******************************************************************************/
RESULT MrvHistCalcInit( void );

RESULT MrvHistCalcSetMode( 
   const UINT8 uHistogramMode
   );
   
RESULT MrvHistCalcGetMode( 
   UINT8 *puHistogramMode
   );
   
RESULT MrvHistCalcSetStepSize( 
   const UINT8 uHistogramStepSize
   );

RESULT MrvHistCalcGetStepSize( 
   UINT8 *puHistogramStepSize
   );
RESULT MrvHistCalcSetWeightMatrix(
   const tsMrvHistMatrix *ptMrvHistMatrix
   );
RESULT MrvHistCalcGetWeightMatrix(
   tsMrvHistMatrix *ptMrvHistMatrix
   );

RESULT MrvHistCalcSetHistogramWindow( 
   const tsMrvWindow *ptWnd
   );
   
RESULT MrvHistCalcGetHistogramWindow( 
   tsMrvWindow *ptWnd
   );
   
RESULT MrvHistCalcGetOneMeasureResultDataBin(
   UINT8   ucDataBinIndex,
   UINT16 *pusMeasureResultDataBin
   );
   
RESULT MrvHistCalcGetAllMeasureResultDataBin( 
   tsMrvHistDataBin *psHistDataBin
   );

#endif //#ifndef _MRV_ISP_HIST_CALC_H
