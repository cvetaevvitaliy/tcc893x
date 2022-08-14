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


#ifndef _MRV_ISP_CAC_H
#define _MRV_ISP_CAC_H


/******************************************************************************
 * Public CAC Defines and Typedefs
******************************************************************************/

//! configuration of chromatic aberration correction block (given to the
//! CAC driver)
typedef struct
{
   UINT16 usHSize;        //! size of the input image in pixels
   UINT16 usVSize;        //! 
   INT16  sHCenterOffset; //! offset between image center and optical 
   INT16  sVCenterOffset; //! center of the input image in pixels
   UINT8  ucHClipMode;    //! maximum red/blue pixel shift in horizontal
   UINT8  ucVClipMode;    //! and vertival direction, range 0..2
   UINT16 usABlue;        //! parameters for radial shift calculation,
   UINT16 usARed;         //! 9 bit twos complement with 4 fractional
   UINT16 usBBlue;        //! digits, valid range -16..15.9375
   UINT16 usBRed;
   UINT16 usCBlue;
   UINT16 usCRed;
   float  fAspectRatio;   //! 0 = square pixel sensor, all other = aspect
                          //! ratio of non-square pixel sensor
} tsMrvCacConfig;


//! register values of chromatic aberration correction block (delivered by
//! the CAC driver)
typedef struct
{
   UINT8  ucHClipMode;    //! maximum red/blue pixel shift in horizontal
   UINT8  ucVClipMode;    //! and vertival direction, range 0..2
   BOOL   bCacEnabled;    //! TRUE=enabled, FALSE=disabled
   UINT16 usHCountStart;  //! preload value of the horizontal CAC pixel
                          //! counter, range 1..4095
   UINT16 usVCountStart;  //! preload value of the vertical CAC pixel
                          //! counter, range 1..4095
   UINT16 usABlue;        //! parameters for radial shift calculation,
   UINT16 usARed;         //! 9 bit twos complement with 4 fractional
   UINT16 usBBlue;        //! digits, valid range -16..15.9375
   UINT16 usBRed;
   UINT16 usCBlue;
   UINT16 usCRed;
   UINT8  ucXNormShift;   //! horizontal normalization shift, range 0..7
   UINT8  ucXNormFactor;  //! horizontal normalization factor, range 16..31
   UINT8  ucYNormShift;   //! vertical normalization shift, range 0..7
   UINT8  ucYNormFactor;  //! vertical normalization factor, range 16..31
} tsMrvCacRegValues;

/******************************************************************************
 * Public CAC Interface
 ******************************************************************************/

RESULT MrvIspCacSetConfig( const tsMrvCacConfig *ptCacConfig );
RESULT MrvIspCacGetRegValues( tsMrvCacRegValues *ptCacRegValues );
RESULT MrvIspCacEnable( void );
RESULT MrvIspCacDisable( void );


#endif //#ifndef _MRV_ISP_CAC_H
