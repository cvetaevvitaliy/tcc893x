/**

  @file omx_mp3dec_component.h

  This file is header of MP3 decoder component.

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

#ifndef _OMX_MP3DEC_COMPONENT_H_
#define _OMX_MP3DEC_COMPONENT_H_

#include <omx_audiodec_component.h>

/* Component private entry points declaration */
OMX_ERRORTYPE omx_mp3dec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName, OMX_HANDLE_FUNC pHandleFunc);

static OMX_S32 OpenMP3Dec(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer);
static OMX_S32 DecodeMP3(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer);
static OMX_S32 FlushMP3Dec(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer);
static OMX_S32 CloseMP3Dec(OMX_COMPONENTTYPE* openmaxStandComp);

#endif /* _OMX_MP3DEC_COMPONENT_H_ */

