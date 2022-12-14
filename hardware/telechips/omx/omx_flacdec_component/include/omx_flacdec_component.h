/**

  @file omx_flacdec_component.h

  This file is header of FLAC decoder component.

  Copyright (C) 2007-2008  STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

#ifndef _OMX_FLACDEC_COMPONENT_H_
#define _OMX_FLACDEC_COMPONENT_H_

#include <omx_audiodec_component.h>

/* Component private entry points declaration */
OMX_ERRORTYPE omx_flacdec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName, OMX_HANDLE_FUNC pHandleFunc);

static OMX_S32 OpenFLACDec(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer);
static OMX_S32 DecodeFLAC(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer);
static OMX_S32 FlushFLACDec(OMX_COMPONENTTYPE* openmaxStandComp,  OMX_BUFFERHEADERTYPE* inputbuffer);
static OMX_S32 CloseFLACDec(OMX_COMPONENTTYPE* openmaxStandComp);

#endif /* _OMX_FLACDEC_COMPONENT_H_ */

