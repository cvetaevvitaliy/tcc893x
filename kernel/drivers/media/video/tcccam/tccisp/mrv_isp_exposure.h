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


/******************************************************************************
 * Public Mean Luminance Measurement Defines and Typedefs
******************************************************************************/

#define MRV_MEAN_LUMA_ARR_SIZE_COL 5 // must be > Zero
#define MRV_MEAN_LUMA_ARR_SIZE_ROW 5 // must be > Zero
#define MRV_MEAN_LUMA_ARR_SIZE     (MRV_MEAN_LUMA_ARR_SIZE_COL*MRV_MEAN_LUMA_ARR_SIZE_ROW)

// Structure contains a 2-dim 5x5 array for mean luminance values from 5x5 MARVIN measurement grid.
typedef struct
{
    UINT8 ucMeanLumaBlock[MRV_MEAN_LUMA_ARR_SIZE_COL][MRV_MEAN_LUMA_ARR_SIZE_ROW];
} tsMrvMeanLuma;

// Structure contains bits autostop and exp_meas_mode of isp_exp_ctrl
typedef struct
{
    BOOL bAutoStop;
#if (MARVIN_FEATURE_AUTO_EXPOSURE == MARVIN_FEATURE_AUTO_EXPOSURE_V4)
    BOOL bExpMeasMode;
    BOOL bExpStart;
#endif //MARVIN_FEATURE_AUTO_EXPOSURE
} tsIspExpCtrl;

/******************************************************************************
 * Public Mean Luminance Measurement Interface
 ******************************************************************************/

RESULT MrvIspMeasExposureInitializeModule( void );

RESULT MrvIspMeasExposureSetConfig( const tsMrvWindow *ptWnd, const tsIspExpCtrl *ptIspExpCtrl );
RESULT MrvIspMeasExposureGetConfig( tsMrvWindow *ptWnd, tsIspExpCtrl *ptIspExpCtrl );

RESULT MrvIspMeasExposureGetMeanLumaValues( tsMrvMeanLuma *ptMrvMeanLuma );
RESULT MrvIspMeasExposureGetMeanLumaByNum( UINT8 BlockNum, UINT8 *pLuma );
RESULT MrvIspMeasExposureGetMeanLumaByPos( UINT8 XPos, UINT8 YPos, UINT8 *pLuma );
