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

#ifndef __TC3X_IO_I2C_H__
#define __TC3X_IO_I2C_H__

#include "../TC3X_Common.h"

#if defined (USE_IF_I2C)
void      TC3X_IO_I2C_Init (int moduleidx);
TC3X_IO_Err TC3X_IO_I2C_Close(int moduleidx);
void      TC3X_IO_I2C_Variables_Init (int moduleidx);
TC3X_IO_Err TC3X_IO_I2C_Reg_Write (int moduleidx, int ChipAddr, int RegAddr, unsigned int data);
TC3X_IO_Err TC3X_IO_I2C_Reg_Write_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned int data);
unsigned int TC3X_IO_I2C_Reg_Read (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError);
unsigned int TC3X_IO_I2C_Reg_Read_InnerSem (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError);
TC3X_IO_Err TC3X_IO_I2C_Reg_WriteEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *cData, int iSize);
TC3X_IO_Err TC3X_IO_I2C_Reg_WriteEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *cData, int iSize);
TC3X_IO_Err TC3X_IO_I2C_Reg_ReadEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *cData, int iSize);
TC3X_IO_Err TC3X_IO_I2C_Reg_ReadEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *cData, int iSize);

void TC3X_IO_I2C_Reg_Read_Wrap(int iHandleNum, unsigned int Addr, unsigned char *data, unsigned char bytesize);
void TC3X_IO_I2C_Reg_Write_Wrap(int iHandleNum, unsigned int Addr, unsigned char data);
#endif // USE_IF_I2C

#endif
