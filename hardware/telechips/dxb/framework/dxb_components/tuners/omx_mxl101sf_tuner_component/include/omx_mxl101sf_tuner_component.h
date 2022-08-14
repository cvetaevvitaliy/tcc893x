/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Audio.h>
#include <pthread.h>
#include <omx_base_source.h>
#include <omx_base_audio_port.h> 
#include "TCCFile.h"
#include "TCCMemory.h"


/** Alsasrcport component private structure.
 * see the define above
 */

#ifndef TRUE
#define TRUE                1
#endif
	
#ifndef FALSE
#define FALSE               0
#endif


DERIVEDCLASS(omx_mxl101sf_tuner_component_PrivateType, omx_base_source_PrivateType)
#define omx_mxl101sf_tuner_component_PrivateType_FIELDS omx_base_source_PrivateType_FIELDS \
  int		countrycode; \
  int		dxb_standard; //broadcasting standard, DVBT, ISDBT, TDMB
ENDCLASS(omx_mxl101sf_tuner_component_PrivateType)


/* Component private entry points declaration */
OMX_ERRORTYPE omx_mxl101sf_tuner_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName);
OMX_ERRORTYPE omx_mxl101sf_tuner_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp);

void omx_mxl101sf_tuner_component_BufferMgmtCallback(
	OMX_COMPONENTTYPE *openmaxStandComp,
	OMX_BUFFERHEADERTYPE* inputbuffer);

OMX_ERRORTYPE omx_mxl101sf_tuner_component_GetParameter(
	OMX_IN  OMX_HANDLETYPE hComponent,
	OMX_IN  OMX_INDEXTYPE nParamIndex,
	OMX_INOUT OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_mxl101sf_tuner_component_SetParameter(
	OMX_IN  OMX_HANDLETYPE hComponent,
	OMX_IN  OMX_INDEXTYPE nParamIndex,
	OMX_IN  OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_mxl101sf_tuner_component_MessageHandler(
	OMX_COMPONENTTYPE* openmaxStandComp, 
	internalRequestMessageType *message);


OMX_ERRORTYPE omx_mxl101sf_tuner_component_GetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure);

OMX_ERRORTYPE omx_mxl101sf_tuner_component_SetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure);

OMX_ERRORTYPE omx_mxl101sf_tuner_component_GetExtensionIndex(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,
  OMX_OUT OMX_INDEXTYPE* pIndexType);

OMX_ERRORTYPE omx_mxl101sf_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_mxl101sf_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);



