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

#ifndef __TC3X_IO_CSPI_H__
#define __TC3X_IO_CSPI_H__

#include "../TC3X_Common.h"
#include "TC3X_IO.h"

TC3X_IO_Err TC3X_IO_CSPI_Init (int moduleidx);
TC3X_IO_Err TC3X_IO_CSPI_Close (int moduleidx);
void      TC3X_IO_CKC_SetSPISClock2 (int iCh, int iFreq);
void      TC3X_IO_CSPI_GPSB_Init (int moduleidx, int channel);
void      TC3X_IO_CSPI_SetGPSBCh (int moduleidx, int ch);
int       TC3X_IO_CSPI_GetGPSBCh (int moduleidx);
unsigned int TC3X_IO_CSPI_Reg_Read (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError);    // use 52cmd
TC3X_IO_Err TC3X_IO_CSPI_Reg_Write (int moduleidx, int ChipAddr, int RegAddr, unsigned int data);       // use 52cmd
TC3X_IO_Err TC3X_IO_CSPI_Reg_ReadEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize); // use 53cmd
TC3X_IO_Err TC3X_IO_CSPI_Reg_WriteEx (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize); // use 53cmd
unsigned int TC3X_IO_CSPI_Reg_Read_InnerSem (int moduleidx, int ChipAddr, int RegAddr, TC3X_IO_Err * pError);
TC3X_IO_Err TC3X_IO_CSPI_Reg_Write_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned int data);
TC3X_IO_Err TC3X_IO_CSPI_Reg_ReadEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *data, int iSize);
TC3X_IO_Err TC3X_IO_CSPI_Reg_WriteEx_InnerSem (int moduleidx, int ChipAddr, int RegAddr, unsigned char *pCh, int iSize);
int       TC3X_IO_CSPI_CMD0 (int moduleidx);               // R1
int       TC3X_IO_CSPI_CMD3 (int moduleidx, unsigned char *outbuff);
int       TC3X_IO_CSPI_CMD5 (int moduleidx, unsigned char *outbuff);    // Return R4
int       TC3X_IO_CSPI_CMD52 (int moduleidx, unsigned char write_flag,  // write flag  0: read   1: write
                              unsigned short reg_addr,     // SDIO register address
                              unsigned char *buf);         // data buffer to be read and write
int       TC3X_IO_CSPI_CMD53 (int moduleidx, unsigned char write_flag,  // write flag  0: read   1: write
                              unsigned int reg_addr,       // SDIO register address
                              unsigned char *buf,          // data buffer to be read and write
                              unsigned short len, int fixedmode);

void      TC3X_IO_CSPI_SINGLE_ERR_INIT (int moduleidx);
void      TC3X_IO_CSPI_MULTI_ERR_INIT (int moduleidx);
void      TC3X_IO_CSPI_Reg_Read_Wrap(int iHandleNum, unsigned int Addr, unsigned char *data, unsigned char bytesize);
void      TC3X_IO_CSPI_Reg_Write_Wrap(int iHandleNum, unsigned int Addr, unsigned char data);

#endif
