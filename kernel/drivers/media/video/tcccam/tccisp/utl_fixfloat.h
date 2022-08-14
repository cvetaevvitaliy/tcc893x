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

#ifndef _UTL_FIXFLOAT_H
#define _UTL_FIXFLOAT_H

/*****************************************************************************
 * Prototypes
 *****************************************************************************/

UINT32 UtlFloatToFix_U0208( FLOAT fFloat );
FLOAT  UtlFixToFloat_U0208( UINT32 ulFix );

UINT32 UtlFloatToFix_U0408( FLOAT fFloat );
FLOAT  UtlFixToFloat_U0408( UINT32 ulFix );

UINT32 UtlFloatToFix_U0800( FLOAT fFloat );
FLOAT  UtlFixToFloat_U0800( UINT32 ulFix );

UINT32 UtlFloatToFix_U1000( FLOAT fFloat );
FLOAT  UtlFixToFloat_U1000( UINT32 ulFix );

UINT32 UtlFloatToFix_U1200( FLOAT fFloat );
FLOAT  UtlFixToFloat_U1200( UINT32 ulFix );

UINT32 UtlFloatToFix_S0207( FLOAT fFloat );
FLOAT  UtlFixToFloat_S0207( UINT32 ulFix );

UINT32 UtlFloatToFix_S0307( FLOAT fFloat );
FLOAT  UtlFixToFloat_S0307( UINT32 ulFix );

UINT32 UtlFloatToFix_S0407( FLOAT fFloat );
FLOAT  UtlFixToFloat_S0407( UINT32 ulFix );

UINT32 UtlFloatToFix_S0808( FLOAT fFloat );
FLOAT  UtlFixToFloat_S0808( UINT32 ulFix );

UINT32 UtlFloatToFix_S1200( FLOAT fFloat );
FLOAT  UtlFixToFloat_S1200( UINT32 ulFix );

#endif // _UTL_FIXFLOAT_H
