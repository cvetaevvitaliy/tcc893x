/*****************************************************************************************
 *
 * FILE NAME          : MxL101SF_OEM_Drv.h
 * 
 * AUTHOR             : Brenndon Lee
 *
 * DATE CREATED       : Jan/22/2008
 *
 * DESCRIPTION        : This file contains I2C-related constants 
 *
 *****************************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ****************************************************************************************/

#ifndef __MXL101SF_OEM_DRV_H__
#define __MXL101SF_OEM_DRV_H__

/*****************************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
*****************************************************************************************/

#include "MaxLinearDataTypes.h"

/*****************************************************************************************
    Macros
*****************************************************************************************/

#define MxL_DLL_DEBUG0  printf

/*****************************************************************************************
    User-Defined Types (Typedefs)
*****************************************************************************************/

/*****************************************************************************************
    Global Variable Declarations
*****************************************************************************************/

/*****************************************************************************************
    Prototypes
*****************************************************************************************/
MXL_STATUS Ctrl_I2cConnect(UINT8 I2CSalveAddr);
MXL_STATUS Ctrl_I2cDisConnect();

MXL_STATUS Ctrl_ReadRegister(UINT8 I2CSlaveAddr, 
                             UINT8 RegAddr, 
                             UINT8 *DataPtr, 
                             void *PrivateDataPtr);
MXL_STATUS Ctrl_WriteRegister(UINT8 I2CSlaveAddr, 
                              UINT8 RegAddr, 
                              UINT8 RegData, 
                              void *PrivateDataPtr);
MXL_STATUS Ctrl_Sleep(UINT16 TimeinMilliseconds);
MXL_STATUS Ctrl_GetTime(UINT32 *TimeinMillisecondsPtr);

#endif /* __MXL101SF_OEM_DRV_H__ */

