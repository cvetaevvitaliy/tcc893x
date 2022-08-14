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


#ifndef _MRV_SMIA_MIPI_H
#define _MRV_SMIA_MIPI_H


/******************************************************************************
 * Defines
 ******************************************************************************/


#if (MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_MIPI_EMP)
#define SMIA_INTRPT_FIFO_FILL_LEVEL    0x00000020  // mask for SMIA fifo fill level interrupt
#define SMIA_INTRPT_EMB_DATA_OVFLW     0x00000002  // mask for SMIA embedded data overflow interrupt
#define SMIA_INTRPT_FRAME_END          0x00000001  // mask for SMIA frame end interrupt
#define SMIA_INTRPT_ALL                0x00000023  // mask for all SMIA interrupts
#endif //#if( MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_MIPI_EMP )



#if (MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_MIPI_EMP)
#define SMIA_PIC_FORMAT_RAW_12               0x0000000C  // mask for SMIA 
#define SMIA_PIC_FORMAT_RAW_8_TO_10_DECOMP   0x0000000D  // mask for SMIA
#define SMIA_PIC_FORMAT_RAW_10               0x0000000B  // mask for SMIA  
#define SMIA_PIC_FORMAT_RAW_8                0x0000000A  // mask for SMIA
#define SMIA_PIC_FORMAT_YUV_422              0x00000000  // mask for SMIA 
#endif //#if( MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_MIPI_EMP )



/******************************************************************************
 * Public SMIA/MIPI Interface
 ******************************************************************************/

#if (MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_MIPI_EMP)
RESULT MrvSmiaMipiInitializeModule( void );
RESULT MrvSmiaMipiGetControlRegisterStatus( UINT32 *puSmiaControlRegisterStatus );
RESULT MrvSmiaMipiSetControlRegisterStatus( const UINT32 uSmiaControlRegisterStatus );
RESULT MrvSmiaMipiActivateModule( const BOOL bModuleActive );
RESULT MrvSmiaMipiActivateCfgUpdateSignal( const BOOL bCfgUpdateSignalActive );
RESULT MrvSmiaMipiResetInterrups( void );
RESULT MrvSmiaMipiFreeEmbDataFifo( void );
RESULT MrvSmiaMipiEmbDataAvailabe( BOOL *pbEmbDataAvail );
RESULT MrvSmiaMipiActivateInterrupts( const UINT32 uActivatedInterrupt );
RESULT MrvSmiaMipiGetImscRegister( UINT32 *puActivatedInterrupt );
RESULT MrvSmiaMipiGetGeneralInterruptState( UINT32 *puGeneralInterruptStatus );
RESULT MrvSmiaMipiGetActivatedInterruptState( UINT32 *puActivatedInterruptStatus );
RESULT MrvSmiaMipiSetInterruptRegister( const UINT32 uInterruptRegisterMask, const UINT32 uNewInterruptRegisterValue );
RESULT MrvSmiaMipiSetPictureFormat( const UINT32 uPictureFormat );
RESULT MrvSmiaMipiGetPictureFormat( UINT32 *puPictureFormat );
RESULT MrvSmiaMipiSetFrameLines( const UINT32 uNumOfEmbDataLines, const UINT32 uNumOfPicDataLines );
RESULT MrvSmiaMipiGetFrameLines( UINT32 *puNumOfEmbDataLines, UINT32 *puNumOfPicDataLines );
RESULT MrvSmiaMipiSetEmbDataStorage( const tsMrvWindow *ptsEmbDataWindow1, const tsMrvWindow *ptsEmbDataWindow2 );
RESULT MrvSmiaMipiGetEmbDataStorage( tsMrvWindow *ptsEmbDataWindow1, tsMrvWindow *ptsEmbDataWindow2 );
RESULT MrvSmiaMipiGetEmbData(UINT8 *puDestEmbDataBuffer, const UINT32 ulDestEmbDataBufSize, UINT32 *puNumOfEmbDataBytes);
RESULT MrvSmiaMipiSetEmbDataFillLevel( const UINT32 ulFillLevel );
RESULT MrvSmiaMipiGetEmbDataFillLevel( UINT32 *pulFillLevel );
#endif //#if (MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_MIPI_EMP)


#endif //#ifndef _MRV_SMIA_MIPI_H
