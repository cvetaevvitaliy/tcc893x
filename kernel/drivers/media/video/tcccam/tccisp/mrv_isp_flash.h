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


#ifndef _MRV_FLASH_LIGHT_H
#define _MRV_FLASH_LIGHT_H


/******************************************************************************
 * Typedefs
 ******************************************************************************/

typedef struct
{
    BOOL bPreflashOn;
    BOOL bFlashOn;
    BOOL bPrelightOn;
} tsMrvFlashCmd;


typedef struct
{
    BOOL  bPrelightOffAtEndOfFlash;
    BOOL  bVsyncEdgePositive;
    BOOL  bOutputPolarityLowActive;
    BOOL  bUseExternalTrigger;
    UINT8 ucCaptureDelay;
} tsMrvFlashConfig;


/******************************************************************************
 * Public Flash Light Interface
 ******************************************************************************/
RESULT MrvFlashLightInit( void );
RESULT MrvFlashSetCmdRegister( const tsMrvFlashCmd *ptMrvFlashCmd );
RESULT MrvFlashLightSetConfigRegister( const tsMrvFlashConfig *tMrvFlashConfig );
RESULT MrvFlashLightGetConfigRegister( tsMrvFlashConfig *tMrvFlashConfig );
RESULT MrvFlashLightSetPrelightModeConfig( const BOOL bPrelightModeConfig );
RESULT MrvFlashLightGetPrelightModeConfig( BOOL *pbPrelightModeConfig );
RESULT MrvFlashLightSetVsyncTriggerEdge( const BOOL bVsyncTriggerEdge );
RESULT MrvFlashLightGetVsyncTriggerEdge( BOOL *pbVsyncTriggerEdge );
RESULT MrvFlashLightSetOutSignalPolarity( const BOOL bOutputSignalPolarity );
RESULT MrvFlashLightGetOutSignalPolarity( BOOL *pbOutputSignalPolarity );
RESULT MrvFlashLightSetTriggerSource( const BOOL bTriggerSource );
RESULT MrvFlashLightGetTriggerSource( BOOL *pbTriggerSource );
RESULT MrvFlashLightSetCaptureDelayFrameNum( const UINT8 uFrameNum );
RESULT MrvFlashLightGetCaptureDelayFrameNum( UINT8 *puFrameNum );
RESULT MrvFlashLightSetMaxFlashTime( const UINT16 uMaxFlashTime );
RESULT MrvFlashLightGetMaxFlashTime( UINT16 *puMaxFlashTime );
RESULT MrvFlashLightSetFlashTime( const UINT32 uFlashTime );
RESULT MrvFlashLightGetFlashTime( UINT32 *puFlashTime );
RESULT MrvFlashLightSetFlashDelay( const UINT32 uFlashDelay );
RESULT MrvFlashLightGetFlashDelay( UINT32 *puFlashDelay );
RESULT MrvFlashLightSetDelayPreDevider( const UINT16 uFlashDelayPreDevider );
RESULT MrvFlashLightGetDelayPreDevider( UINT16 *puFlashDelayPreDevider );


#endif //#ifndef _MRV_FLASH_LIGHT_H
