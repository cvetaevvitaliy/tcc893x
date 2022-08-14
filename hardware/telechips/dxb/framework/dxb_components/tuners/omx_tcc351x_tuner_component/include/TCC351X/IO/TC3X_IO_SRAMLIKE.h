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

#ifndef __TC3X_IO_SRAMLIKE_h__
#define __TC3X_IO_SRAMLIKE_h__

#include "../TC3X_Common.h"

#if defined (USE_IF_SRAMLIKE)
// SRAMLIKE IF Setting
#define	TC3X_HwEBIBASE					(0x40000000)
#define	TC3X_HwEBIADDR					*(volatile unsigned short *)(TC3X_HwEBIBASE + 0x00)
#define	TC3X_HwEBIDATA					*(volatile unsigned char *) (TC3X_HwEBIBASE + 0x04)

void      TC3X_IO_SRAMLIKE_Init (int moduleidx);
TC3X_IO_Err TC3X_IO_SRAMLIKE_Close(int moduleidx);
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_Write (int moduleidx, int ChipAddr, int RegAddr, unsigned int data);
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_WriteEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize);
unsigned int TC3X_IO_SRAMLIKE_Reg_Read (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError);
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_ReadEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize);
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_Write_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned int data);
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_WriteEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize);
unsigned int TC3X_IO_SRAMLIKE_Reg_Read_InnerSem (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError);
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_ReadEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize);
TC3X_IO_Err TC3X_IO_SRAMLIKE_Reg_ReadEx_InnerSem_DMA (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize);
#endif // USE_IF_SRAMLIKE

#endif
