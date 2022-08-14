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


#ifndef _MRV_SLS_WB_PRIV_H
#define _MRV_SLS_WB_PRIV_H

/*****************************************************************************
 * DEFINES
 *****************************************************************************/
//! abbreviations for local debug control ( level | module )
#define DERR  ( DBG_ERR  | DBG_MRV )    //!< errors
#define DWARN ( DBG_WARN | DBG_MRV )    //!< warnings
#define DINFO ( DBG_INFO | DBG_MRV )    //!< information

//! limits for out of range tests.
#define DIVMIN     0.00001f  //!< lowest denominator for divisions
#define LOGMIN     0.0001f   //!< lowest value for logarithmic calculations

//! value to initialize the gexp prior filter
#define AWB_GEXP_PRIOR_MIDDLE 0.5f
//! max test cases.
#define AWB_MAX_TESTSTEPS 4
//! max aberration between the calculated and the reference values to set the result to success.
#define AWB_MAX_ABERRATION   0.0001f //  0.000001f
//! the max light profiles the function can handle.
#define AWB_MAX_PROFILECOUNT   10
//! square root of 2 pi
#define SQR2PI    2.506628274631000502415765284811f
//! Max number of addressable profiles
#define MAX_NUMBER_OF_EXISTING_PROFILES 32
//! Array size of crosstalk matrix
#define MATRIX_SIZE 9
//! Array size of color components
#define COMP_MATRIX_SIZE 3
//! Number of profil classes
#define NUMBER_OF_PROFIL_CLASS 3
//! Number of RGB planes
#define NUMBER_OF_RGB_GAIN_PLANES 3
//! Number of red plane in RGB array
#define RGB_GAIN_PLANE_R  0
//! Number of green plane in RGB array
#define RGB_GAIN_PLANE_G  1
//! Number of blue plane in RGB array
#define RGB_GAIN_PLANE_B  2
//! Max length of profil names
#define MAX_NAME_LENGHT 23
//! For initialization purposes
#define GAIN_NORMAL 1

//! Initialization values for near white parameters
#define REF_CR_MAX_R_INIT 128;
#define REF_CB_MAX_B_INIT 128;
#define MIN_C_INIT        20;
#define MAX_C_SUM_INIT    20;
#define MAX_Y_INIT        200;
#define MIN_Y_MAX_G_INIT  30;

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
//! structure for awb calculation
typedef struct tsMrvWbValues_
{
    tsIsiAwbFloatMean     tMrvAwbMean;                    //!< YCbCr mean values
    tsIsiCompGain         tMrvAwbMeanRgb;                 //!< RGB values converted from YCbCr
    tsIsi3x3FloatMatrix   tMrvXtalkMatrix;                //!< xtalk matrix
    tsIsi3x3FloatMatrix   tMrvInvXtalkMatrix;             //!< inverse xtalk matrix
    tsIsiXTalkFloatOffset tMrvXtalkOffset;                //!< xtalk offset
    tsIsiCompGain         tMrvAwbMeans;                   //!< reconverted mean values
    tsIsiCompGain         tMrvAwbGains;                   //!< gains from awb register
    UINT16                usMrvAwbPcaFilterSizeMax;       //!< max damping count chosed from menu
    UINT16                usMrvAwbPriorExpFilterSizeMax;  //!< max damping count chosed from menu
    FLOAT                 fExpPriorOut;                   //!< transfers fExpPriorOut from MrvSls_WbCalcRgbGains to MrvSls_WbProcess
}tsMrvWbValues;

//! structure for print purposes and In/Out classification
typedef struct tsMrvProfileInfo_
{
    char cProfileClass[NUMBER_OF_PROFIL_CLASS]; //!< Profile class: I = Input profile, O = Output profile
    char cProfileText[MAX_NAME_LENGHT]; //!< Profile names for print purposes

}tsMrvProfileInfo;

//! context structure for function MrvSls_Wb_Interp1
typedef struct tsMrvWbInterp1Ctx_
{
    FLOAT*  paf_x;       //!< x-array, lookup table
    FLOAT*  paf_Y;       //!< Y-array
    UINT16 usArraySize; //!< size of above arrays
    FLOAT  f_xi;        //!< value to find matching interval for in x-array
    FLOAT  f_Yi;        //!< result calculated from f_xi, paf_x and paf_Y
}tsMrvWbInterp1Ctx;

/*****************************************************************************
 * GLOBALS
 *****************************************************************************/
//! the distance of a calculated value to the center is assigned to one of three regions
enum RegionType
{
    eRegionA, //!< Only one lightsource selected
    eRegionB, //!< Interpolation of region A and Region B selected
    eRegionC  //!< Selection of a mixture of 3 or 4 lightsources
};

 //! data for print purposes and In/Out classification
tsMrvProfileInfo tMrvProfileInfos[MAX_NUMBER_OF_EXISTING_PROFILES] =
{
    "I","Inprof  CIE_A         ",
    "I","Inprof  CIE_B         ",
    "I","Inprof  CIE_C         ",
    "O","Outprof CIE_D50       ",
    "O","Outprof CIE_D55       ",
    "I","Inprof CIE_D65        ",
    "O","Outprof CIE_D75       ",
    "O","Outprof CIE_E         ",
    "I","not defined           ",
    "I","not defined           ",
    "I","not defined           ",
    "O","Outprof Twilight      ",
    "I","not defined           ",
    "I","not defined           ",
    "O","Outprof Shade         ",
    "O","Outprof Day           ",
    "I","Inprof  CIE_F1        ",
    "I","Inprof  CIE_F2  (CWF) ",
    "I","Inprof  CIE_F3        ",
    "I","Inprof  CIE_F4        ",
    "I","Inprof  CIE_F5        ",
    "I","Inprof  CIE_F6        ",
    "I","Inprof  CIE_F7        ",
    "I","Inprof  CIE_F8        ",
    "I","Inprof  CIE_F9        ",
    "I","Inprof  CIE_F10 (TL85)",
    "I","Inprof  CIE_F11 (TL84)",
    "I","Inprof  CIE_F12 (TL83)",
    "O","Outprof Cloudy        ",
    "O","Outprof Sunny         ",
    "I","not defined           ",
    "I","Default Profile       ",
};

 //! constants for conversion from Y,Cb,Cr to R,G,B
FLOAT YuvRgbCoeff[]  =
{
    1.1636025f,  - 0.0622839f,    1.6007823f,
    1.1636025f,  - 0.4045270f,  - 0.7949191f,
    1.1636025f,    1.9911744f,  - 0.0250092f
};

//! flag is true if the push button is pressed.
BOOL   bTriggerPushWb;

/* for the basic version the numbers of available sensor profiles and the filter size
 * are fixed. In later versions the space should be allocated dynamic.
 */
//! counter for prior exp damping filter
UINT16 usAwbPriorExpFilterCount;
//! coefficient for AWB damping filter
FLOAT fAwbIIRDampCoef;
//! damped gain values
tsIsiCompGain tAwbGainDamped;
//! damped matrix coefficients
FLOAT  fMrvMatrixCoeffDamped[MATRIX_SIZE];
//! damped gain offsets
tsIsiXTalkFloatOffset tMrvOffsetDamped;
//! damping filter for fGExp prior values
FLOAT  fExpPriorInFilter[AWB_DAMPING_FILTER_SIZE_MAX];
//! structure for data transfer AE -> SLS support
tsMrvAEtoSLS tMrvAEtoSLS;

//! testcase counter
UINT16 usTCCount;
//! testcase error counter
UINT16 usTCErrCount;
/*****************************************************************************
 * PRIVATE PROTOTYPES
 *****************************************************************************/
RESULT MrvSls_WbProcess(const tsConfig *ptConfig);

#endif //_MRV_SLS_WB_PRIV_H
