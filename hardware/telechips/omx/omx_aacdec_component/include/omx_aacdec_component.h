/**

  @file omx_aacdec_component.h

  This file is header of AAC decoder component.

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

#ifndef _OMX_AACDEC_COMPONENT_H_
#define _OMX_AACDEC_COMPONENT_H_

#include <omx_audiodec_component.h>

/* Component private entry points declaration */
OMX_ERRORTYPE omx_aacdec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName, OMX_HANDLE_FUNC pHandleFunc);

OMX_ERRORTYPE omx_aacdec_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_aacdec_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure);

static OMX_S32 OpenAACDec(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer);
static OMX_S32 DecodeAAC(OMX_COMPONENTTYPE* openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer);
static OMX_S32 FlushAACDec(OMX_COMPONENTTYPE* openmaxStandComp);
static OMX_S32 CloseAACDec(OMX_COMPONENTTYPE* openmaxStandComp);

// for LATM Demuxer
typedef enum
{	
	// LATM/LOAS (Low Overhead Audio Stream): LATM with sync information
	TF_AAC_LOAS			= 0,	// default value

	// LATM (Low-overhead MPEG-4 Audio Transport Multiplex), without LOAS Sync-Layer, No random access is possible
	TF_AAC_LATM_MCP1	= 1,	// LATM wiht muxConfigPresent = 1
	TF_AAC_LATM_MCP0	= 2,	// LATM wiht muxConfigPresent = 0

	// ADTS (Audio Data Transport Stream)
	TF_AAC_ADTS			= 3,	

	// ADIF (Audio Data Interchange Format)
	TF_AAC_ADIF			= 4,	// not supported

	TF_UNKNOWN			= 0x7FFFFFFF	// Unknown format
}TransportFormat;

#endif /* _OMX_AACDEC_COMPONENT_H_ */

