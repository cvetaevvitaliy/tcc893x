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

#ifndef TRUE
#define TRUE                1
#endif
	
#ifndef FALSE
#define FALSE               0
#endif

/* STS */
#define BROADCAST_TCC317X_I2C_STS_TUNER_NAME "OMX.tcc.broadcast.tcc317x.i2c.sts"
#define BROADCAST_TCC317X_CSPI_STS_TUNER_NAME "OMX.tcc.broadcast.tcc317x.cspi.sts"

/* SPI Master */
#define BROADCAST_TCC317X_I2C_SPIMS_NAME "OMX.tcc.broadcast.tcc317x.i2c.spims"
#define BROADCAST_TCC317X_CSPI_SPIMS_NAME "OMX.tcc.broadcast.tcc317x.cspi.spims"

typedef struct
{
	int EWSMessageID;
	int EWSMessageLen;
	unsigned char *EWSMessage;
}st_EWS_MESSAGE;

DERIVEDCLASS(omx_tcc317x_tuner_component_PrivateType, omx_base_source_PrivateType)
#define omx_tcc317x_tuner_component_PrivateType_FIELDS omx_base_source_PrivateType_FIELDS \
	st_EWS_MESSAGE *pEWS_Message; \
	pthread_t ChannelSearchThread; \
	int		countrycode; 
ENDCLASS(omx_tcc317x_tuner_component_PrivateType)

void omx_tcc317x_tuner_component_data_ch_notify(OMX_S32 moduleidx, OMX_PTR pucData, OMX_U32 uiSize);
void omx_tcc317x_tuner_component_ews_notify(OMX_S32 moduleidx, void *pst_EWS);
void omx_tcc317x_tuner_component_set_channel_notify(void *pResult);
void omx_tcc317x_tuner_component_searched_notify(void *pChannelInfo, int searchedCount);
void omx_tcc317x_tuner_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* outputbuffer);

OMX_ERRORTYPE omx_tcc317x_I2C_STS_tuner_component_Init (OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_tcc317x_I2C_SPIMS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_tcc317x_CSPI_STS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_tcc317x_CSPI_SPIMS_tuner_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_tcc317x_tuner_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);

OMX_ERRORTYPE omx_tcc317x_tuner_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_tcc317x_tuner_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName);
OMX_ERRORTYPE omx_tcc317x_tuner_component_GetExtensionIndex(  OMX_IN  OMX_HANDLETYPE hComponent, OMX_IN  OMX_STRING cParameterName,  OMX_OUT OMX_INDEXTYPE* pIndexType);
OMX_ERRORTYPE omx_tcc317x_tuner_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp, internalRequestMessageType *message);

OMX_ERRORTYPE omx_tcc317x_tuner_component_SetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure);

OMX_ERRORTYPE omx_tcc317x_tuner_component_GetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure);

OMX_ERRORTYPE omx_tcc317x_tuner_component_GetParameter(
	OMX_IN  OMX_HANDLETYPE hComponent,
	OMX_IN  OMX_INDEXTYPE nParamIndex,
	OMX_INOUT OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_tcc317x_tuner_component_SetParameter(
	OMX_IN  OMX_HANDLETYPE hComponent,
	OMX_IN  OMX_INDEXTYPE nParamIndex,
	OMX_IN  OMX_PTR ComponentParameterStructure);
