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


#ifndef _MRV_SMIA_H
#define _MRV_SMIA_H


/******************************************************************************
 * Defines
 ******************************************************************************/

// SMIA interrup mask defines
#if ( MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_COMPLETE )
#define SMIA_INTRPT_FIFO_FILL_LEVEL    0x00000020  // mask for SMIA fifo fill level interrupt
#define SMIA_INTRPT_SYNC_FIFO_OVFLW    0x00000010  // to be documented
#define SMIA_INTRPT_ERR_CS             0x00000008  // to be documented
#define SMIA_INTRPT_ERR_PROTOCOL       0x00000004  // to be documented
#define SMIA_INTRPT_EMB_DATA_OVFLW     0x00000002  // mask for SMIA embedded data overflow interrupt
#define SMIA_INTRPT_FRAME_END          0x00000001  // mask for SMIA frame end interrupt
#define SMIA_INTRPT_ALL                0x0000003F  // mask for all SMIA interrupts
#endif //#if( MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_COMPLETE )

#if ( MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_EMP) 
#define SMIA_INTRPT_EMB_DATA_OVFLW     0x00000002  // mask for SMIA embedded data overflow interrupt
#define SMIA_INTRPT_FRAME_END          0x00000001  // mask for SMIA frame end interrupt
#define SMIA_INTRPT_ALL                0x00000003  // mask for all SMIA interrupts
#endif //#if( MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_EMP )

// SMIA output picture format definitions
#if ( MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_COMPLETE )
#define SMIA_PIC_FORMAT_COMPRESSED           0x0000000F  // mask for SMIA 
#define SMIA_PIC_FORMAT_RAW_8_TO_10_DECOMP   0x0000000D  // mask for SMIA 
#define SMIA_PIC_FORMAT_RAW_12               0x0000000C  // mask for SMIA 
#define SMIA_PIC_FORMAT_RAW_10               0x0000000B  // mask for SMIA 
#define SMIA_PIC_FORMAT_RAW_8                0x0000000A  // mask for SMIA 
#define SMIA_PIC_FORMAT_RAW_7                0x00000009  // mask for SMIA 
#define SMIA_PIC_FORMAT_RAW_6                0x00000008  // mask for SMIA 
#define SMIA_PIC_FORMAT_RGB_888              0x00000006  // mask for SMIA
#define SMIA_PIC_FORMAT_RGB_565              0x00000005  // mask for SMIA
#define SMIA_PIC_FORMAT_RGB_444              0x00000004  // mask for SMIA
#define SMIA_PIC_FORMAT_YUV_420              0x00000001  // mask for SMIA
#define SMIA_PIC_FORMAT_YUV_422              0x00000000  // mask for SMIA 
#endif //#if( MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_COMPLETE )


#if ( MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_EMP)
#define SMIA_PIC_FORMAT_RAW_8_TO_10_DECOMP   0x0000000D  // mask for SMIA
#define SMIA_PIC_FORMAT_RAW_10               0x0000000B  // mask for SMIA  
#define SMIA_PIC_FORMAT_RAW_8                0x0000000A  // mask for SMIA
#endif //#if( MARVIN_FEATURE_SMIA == MARVIN_FEATURE_SMIA_EMP )


/******************************************************************************
 * Public SMIA Interface
 ******************************************************************************/

RESULT MrvSmiaInitializeModule( void );
RESULT MrvSmiaGetControlRegisterStatus( UINT32 *puSmiaControlRegisterStatus );
RESULT MrvSmiaSetControlRegisterStatus( const UINT32 uSmiaControlRegisterStatus );
RESULT MrvSmiaActivateModule( const BOOL bModuleActive );
RESULT MrvSmiaSetDmaChannel( const UINT8 uDmaChannel );
RESULT MrvSmiaGetDmaChannel( UINT8 *puDmaChannel );
RESULT MrvSmiaSetShutdownLane( const BOOL bShutdownLane );
RESULT MrvSmiaGetShutdownLane( BOOL *pbShutdownLane );
RESULT MrvSmiaActivateCfgUpdateSignal( const BOOL bCfgUpdateSignalActive );
RESULT MrvSmiaResetInterrups( void );
RESULT MrvSmiaFreeEmbDataFifo( void );
RESULT MrvSmiaGetCurrentDmaChannel( BOOL *puDmaChannel );
RESULT MrvSmiaEmbDataAvailabe( BOOL *pbEmbDataAvail );
RESULT MrvSmiaActivateInterrupts( const UINT32 uActivatedInterrupt );
RESULT MrvSmiaGetImscRegister( UINT32 *puActivatedInterrupt );
RESULT MrvSmiaGetGeneralInterruptState( UINT32 *puGeneralInterruptStatus );
RESULT MrvSmiaGetActivatedInterruptState( UINT32 *puActivatedInterruptStatus );
RESULT MrvSmiaSetInterruptRegister( const UINT32 uInterruptRegisterMask, const UINT32 uNewInterruptRegisterValue );
RESULT MrvSmiaSetPictureFormat( const UINT32 uPictureFormat );
RESULT MrvSmiaGetPictureFormat( UINT32 *puPictureFormat );
RESULT MrvSmiaSetFrameLines( const UINT32 uNumOfEmbDataLines, const UINT32 uNumOfPicDataLines );
RESULT MrvSmiaGetFrameLines( UINT32 *puNumOfEmbDataLines, UINT32 *puNumOfPicDataLines );
RESULT MrvSmiaSetEmbDataStorage( const UINT32 uEmbDataLine, const UINT32 uEmbDataPos, const UINT32 uNumOfEmbDataBytes );
RESULT MrvSmiaGetEmbDataStorage( UINT32 *puEmbDataLine, UINT32 *puEmbDataPos, UINT32 *puNumOfEmbDataBytes );
RESULT MrvSmiaGetEmbData(UINT8 *puDestEmbDataBuffer, const UINT32 ulDestEmbDataBufSize, UINT32 *puNumOfEmbDataBytes);
RESULT MrvSmiaSetEmbDataFillLevel( const UINT32 ulFillLevel );
RESULT MrvSmiaGetEmbDataFillLevel( UINT32 *pulFillLevel );


#endif //#ifndef _MRV_SMIA_H
