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



#ifndef _DATAPATH_INIT_H
#define _DATAPATH_INIT_H

extern void   DpInitBasicLcd( const tsPlSystemConfig *ptSysConf, int iContrast );

extern void   DpInitLcd( const tsPlSystemConfig *ptSysConf, int iContrast );
extern void   DpInitIsi( tsIsiSensorConfig *ptIsiConfig, tsIsiSensorCaps *ptIsiCaps );
extern RESULT DpInitChangeIsiConfig( const tsIsiSensorConfig *ptIsiConfig, tsIsiSensorAwbProfile *ptIsiSensorAwbProfile );

extern RESULT DpInitSetupViewfinderPath( const tsPlSystemConfig *ptSysConf, int iZoom, BOOL bInitBuffers );

extern void DpInitCamParams( const tsPlSystemConfig *ptSysConf);

// marvin related
extern RESULT DpInitMrv( tsPlSystemConfig *ptSysConf);
// some of its individual blocks
extern RESULT DpInitMrvImageEffects( const tsPlSystemConfig *ptSysConf, BOOL bEnable );
extern RESULT DpInitMrvIspFilter( const tsPlSystemConfig *ptSysConf, BOOL bEnable );
extern RESULT DpInitMrvIspCAC( const tsPlSystemConfig *ptSysConf, BOOL bEnable );
extern RESULT DpInitMrvIspLensShade( const tsPlSystemConfig *ptSysConf, BOOL bEnable );
extern RESULT DpInitMrvIspBadPixel( const tsPlSystemConfig *ptSysConf, BOOL bEnable );
extern RESULT DpInitBadPixelGenerator( const tsPlSystemConfig *ptSysConf );
extern RESULT DpInitBls( const tsPlSystemConfig *ptSysConf );
extern void MrvSls_WbProcess(tsConfig *ptConfig);



#endif //#ifndef _MISCELLANEOUS_H
