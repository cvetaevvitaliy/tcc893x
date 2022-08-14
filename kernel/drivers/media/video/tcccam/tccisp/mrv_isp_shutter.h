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
 * TYPEDEFS
 ******************************************************************************/
//! mechanical shutter configuration
typedef struct
{
    BITFIELD       bIsLowActive     :1;      //! shutter_open is low  active
    BITFIELD       bUsePositiveEdge :1;      //! use positive edge of trigger signal
    BITFIELD       bUseTrigger      :1;      //! use "shutter_trig" for trigger event(positive edge)
    BITFIELD       bUseRepetition   :1;      //! shutter is opened with the repedition rate of the trigger signal
    BITFIELD       bEnable          :1;      //! enable the mechanical shutter
} tsMrvMechShutterConfig;


/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/
RESULT MrvIspMechShutterInitializeModule( void );
RESULT MrvIspMechShutterSetConfig( const tsMrvMechShutterConfig *ptShutterConfig );
RESULT MrvIspMechShutterGetConfig( tsMrvMechShutterConfig *ptShutterConfig );

RESULT MrvIspMechShutterSetOpenPolarity( const BOOL bIsLowActive );
RESULT MrvIspMechShutterGetOpenPolarity( BOOL *pbIsLowActive );
RESULT MrvIspMechShutterSetInputTriggerPolarity( const BOOL bUsePositiveEdge );
RESULT MrvIspMechShutterGetInputTriggerPolarity( BOOL *pbUsePositiveEdge );
RESULT MrvIspMechShutterSetTriggerSource( const BOOL bUseTrigger );
RESULT MrvIspMechShutterGetTriggerSource( BOOL *pbUseTrigger );
RESULT MrvIspMechShutterSetRepetition( const BOOL bUseRepetition );
RESULT MrvIspMechShutterGetRepetition( BOOL *pbUseRepetition );
RESULT MrvIspMechShutterEnable( const BOOL bEnable );
RESULT MrvIspMechShutterIsEnabled( BOOL *pbEnable );

RESULT MrvIspMechShutterGetPreDivider( UINT16 *puPreDivider );
RESULT MrvIspMechShutterSetPreDivider( const UINT16 uPreDivider );
RESULT MrvIspMechShutterGetDelay( UINT32 *puDelay );
RESULT MrvIspMechShutterSetDelay( const UINT32 uDelay );
RESULT MrvIspMechShutterGetTime( UINT32 *puTime );
RESULT MrvIspMechShutterSetTime( const UINT32 uTime );
