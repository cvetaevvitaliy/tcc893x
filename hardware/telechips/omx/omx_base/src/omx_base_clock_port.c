/**
  @file src/base/omx_base_clock_port.c

  Base Clock Port class for OpenMAX clock ports to be used in derived components.

  Copyright (C) 2007-2008 STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).

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

  $Date: 2008/09/11 01:53:02 $
  Revision $Rev: 560 $
  Author $Author: B060232 $
*/
#define LOG_TAG	"OMX_CLOCK_PORT"
#include <utils/Log.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omxcore.h>
#include <OMX_Core.h>
#include <OMX_Component.h>
#include "omx_base_component.h"
#include "omx_base_clock_port.h"

/** 
  * @brief the base contructor for the generic openmax ST Clock port
  * 
  * This function is executed by the component that uses a port.
  * The parameter contains the info about the component.
  * It takes care of constructing the instance of the port and 
  * every object needed by the base port.
  *
  * @param openmaxStandComp pointer to the Handle of the component
  * @param openmaxStandPort the ST port to be initialized
  * @param nPortIndex Index of the port to be constructed
  * @param isInput specifices if the port is an input or an output
  * 
  * @return OMX_ErrorInsufficientResources if a memory allocation fails
  */

OMX_ERRORTYPE base_clock_port_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,omx_base_PortType **openmaxStandPort,OMX_U32 nPortIndex, OMX_BOOL isInput) {
  
  omx_base_clock_PortType *omx_base_clock_Port;

  if (!(*openmaxStandPort)) {
    *openmaxStandPort = TCC_calloc(1,sizeof (omx_base_clock_PortType));
  }

  if (!(*openmaxStandPort)) {
    return OMX_ErrorInsufficientResources;
  }

  base_port_Constructor(openmaxStandComp,openmaxStandPort,nPortIndex, isInput);

  omx_base_clock_Port = (omx_base_clock_PortType *)*openmaxStandPort;

  setHeader(&omx_base_clock_Port->sOtherParam, sizeof(OMX_OTHER_PARAM_PORTFORMATTYPE));
  omx_base_clock_Port->sOtherParam.nPortIndex = nPortIndex;
  omx_base_clock_Port->sOtherParam.nIndex = 0;
  omx_base_clock_Port->sOtherParam.eFormat = OMX_OTHER_FormatTime;
  
  omx_base_clock_Port->sPortParam.eDomain              = OMX_PortDomainOther;
  omx_base_clock_Port->sPortParam.format.other.eFormat = OMX_OTHER_FormatTime;
  omx_base_clock_Port->sPortParam.nBufferSize          = sizeof(OMX_TIME_MEDIATIMETYPE) ;
  omx_base_clock_Port->sPortParam.nBufferCountActual   = 1;
  omx_base_clock_Port->sPortParam.nBufferCountMin      = 1;
  omx_base_clock_Port->sPortParam.format.other.eFormat = OMX_OTHER_FormatTime;

  setHeader(&omx_base_clock_Port->sTimeStamp, sizeof(OMX_TIME_CONFIG_TIMESTAMPTYPE));
  omx_base_clock_Port->sTimeStamp.nPortIndex = nPortIndex;
  omx_base_clock_Port->sTimeStamp.nTimestamp = 0x00;
  

  setHeader(&omx_base_clock_Port->sMediaTime, sizeof(OMX_TIME_MEDIATIMETYPE));
  omx_base_clock_Port->sMediaTime.nClientPrivate = 0;
  omx_base_clock_Port->sMediaTime.nOffset = 0x0; 
  omx_base_clock_Port->sMediaTime.xScale = 1; 

  setHeader(&omx_base_clock_Port->sMediaTimeRequest, sizeof(OMX_TIME_MEDIATIMETYPE));
  omx_base_clock_Port->sMediaTimeRequest.nPortIndex = nPortIndex;
  omx_base_clock_Port->sMediaTimeRequest.pClientPrivate = NULL; 
  omx_base_clock_Port->sMediaTimeRequest.nOffset = 0x0; 

  omx_base_clock_Port->Port_SendBufferFunction = &base_clock_port_SendBufferFunction;
  omx_base_clock_Port->PortDestructor = &base_clock_port_Destructor;

  return OMX_ErrorNone;
}

/** 
  * @brief the base clock port destructor for the generic openmax ST clock port
  * 
  * This function is executed by the component that uses a port.
  * The parameter contains the info about the port.
  * It takes care of destructing the instance of the port
  * 
  * @param openmaxStandPort the ST port to be destructed
  * 
  * @return OMX_ErrorNone 
  */

OMX_ERRORTYPE base_clock_port_Destructor(omx_base_PortType *openmaxStandPort){

  base_port_Destructor(openmaxStandPort);

  return OMX_ErrorNone;
}

/** @brief the entry point for sending buffers to the port
 * 
 * This function can be called by the EmptyThisBuffer or FillThisBuffer. It depends on
 * the nature of the port, that can be an input or output port.
 */
OMX_ERRORTYPE base_clock_port_SendBufferFunction(
  omx_base_PortType *openmaxStandPort,
  OMX_BUFFERHEADERTYPE* pBuffer) {

  OMX_ERRORTYPE err;
  OMX_U32 portIndex;
  OMX_COMPONENTTYPE* omxComponent = openmaxStandPort->standCompContainer;
  omx_base_component_PrivateType* omx_base_component_Private = (omx_base_component_PrivateType*)omxComponent->pComponentPrivate;
#if NO_GST_OMX_PATCH
  unsigned int i;
#endif
  portIndex = (openmaxStandPort->sPortParam.eDir == OMX_DirInput)?pBuffer->nInputPortIndex:pBuffer->nOutputPortIndex;
  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s portIndex %lu\n", __func__, portIndex);

  if (portIndex != openmaxStandPort->sPortParam.nPortIndex) {
    DEBUG(DEB_LEV_ERR, "In %s: wrong port for this operation portIndex=%d port->portIndex=%d\n", __func__, (int)portIndex, (int)openmaxStandPort->sPortParam.nPortIndex);
    return OMX_ErrorBadPortIndex;
  }

#ifdef OPENMAX1_2
  if(omx_base_component_Private->state == OMX_StateReserved_0x00000000) {
    DEBUG(DEB_LEV_ERR, "In %s: we are in OMX_StateReserved_0x00000000\n", __func__);
    return OMX_ErrorReserved_0x8000100A;
  }
#else
  if(omx_base_component_Private->state == OMX_StateInvalid) {
    DEBUG(DEB_LEV_ERR, "In %s: we are in OMX_StateInvalid\n", __func__);
    return OMX_ErrorInvalidState;
  }
#endif

  if(omx_base_component_Private->state != OMX_StateExecuting &&
    omx_base_component_Private->state != OMX_StatePause &&
    omx_base_component_Private->state != OMX_StateIdle) {
    DEBUG(DEB_LEV_ERR, "In %s: we are not in executing/paused/idle state, but in %d\n", __func__, omx_base_component_Private->state);
    return OMX_ErrorIncorrectStateOperation;
  }
  if (!PORT_IS_ENABLED(openmaxStandPort) || (PORT_IS_BEING_DISABLED(openmaxStandPort) && !PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(openmaxStandPort)) ||
      (omx_base_component_Private->transientState == OMX_TransStateExecutingToIdle && 
      (PORT_IS_TUNNELED(openmaxStandPort) && !PORT_IS_BUFFER_SUPPLIER(openmaxStandPort)))) {
    DEBUG(DEB_LEV_ERR, "In %s: Port %d is disabled comp = %s \n", __func__, (int)portIndex,omx_base_component_Private->name);
    return OMX_ErrorIncorrectStateOperation;
  }

  /* Temporarily disable this check for gst-openmax */
#if NO_GST_OMX_PATCH
  {
  OMX_BOOL foundBuffer = OMX_FALSE;
  if(pBuffer!=NULL && pBuffer->pBuffer!=NULL) {
    for(i=0; i < openmaxStandPort->sPortParam.nBufferCountActual; i++){
    if (pBuffer->pBuffer == openmaxStandPort->pInternalBufferStorage[i]->pBuffer) {
      foundBuffer = OMX_TRUE;
      break;
    }
    }
  }
  if (!foundBuffer) {
    return OMX_ErrorBadParameter;
  }
  }
#endif

  if ((err = checkHeader(pBuffer, sizeof(OMX_BUFFERHEADERTYPE))) != OMX_ErrorNone) {
    DEBUG(DEB_LEV_ERR, "In %s: received wrong buffer header on input port\n", __func__);
    return err;
  }
  /*If port is not tunneled then simply return the buffer except paused state*/
  if (!PORT_IS_TUNNELED(openmaxStandPort) && (omx_base_component_Private->state != OMX_StatePause)) {
    openmaxStandPort->ReturnBufferFunction(openmaxStandPort,pBuffer);
    return OMX_ErrorNone;
  }
  
  /* And notify the buffer management thread we have a fresh new buffer to manage */
  if(!PORT_IS_BEING_FLUSHED(openmaxStandPort) && !(PORT_IS_BEING_DISABLED(openmaxStandPort) && PORT_IS_TUNNELED_N_BUFFER_SUPPLIER(openmaxStandPort))){
    queue(openmaxStandPort->pBufferQueue, pBuffer);
    tsem_up(openmaxStandPort->pBufferSem);
    DEBUG(DEB_LEV_PARAMS, "In %s Signalling bMgmtSem Port Index=%d\n",__func__, (int)portIndex);
    tsem_up(omx_base_component_Private->bMgmtSem);
  }else if(PORT_IS_BUFFER_SUPPLIER(openmaxStandPort)){
    DEBUG(DEB_LEV_FULL_SEQ, "In %s: Comp %s received io:%d buffer\n", 
        __func__,omx_base_component_Private->name,(int)openmaxStandPort->sPortParam.nPortIndex);
    queue(openmaxStandPort->pBufferQueue, pBuffer);
    tsem_up(openmaxStandPort->pBufferSem);
  }
  else { // If port being flushed and not tunneled then return error
    DEBUG(DEB_LEV_FULL_SEQ, "In %s \n", __func__);
    return OMX_ErrorIncorrectStateOperation;
  }
  return OMX_ErrorNone;
}
