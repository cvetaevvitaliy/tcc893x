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

#ifndef __DMB_PROTOCOL_H__
#define __DMB_PROTOCOL_H__

typedef void (*BB_Command_Read_T)(unsigned int, unsigned char [], unsigned char);
typedef void (*BB_Command_Write_T)(unsigned int, unsigned char);
typedef void (*BB_Command_Read_T_EX)(int, unsigned int, unsigned char [], unsigned char);
typedef void (*BB_Command_Write_T_EX)(int, unsigned int, unsigned char);

typedef void (*TimeDelayMs_T)(unsigned int);

void DMBPROTOCOL_init(BB_Command_Read_T pBB_Command_Read, BB_Command_Write_T pBB_WriteCommand, TimeDelayMs_T pBB_DelayMs);
void DMBPROTOCOL_initEx(BB_Command_Read_T_EX pBB_Command_ReadEx, BB_Command_Write_T_EX pBB_WriteCommandEx, TimeDelayMs_T pBB_DelayMs);
int DMBPROTOCOL_Set(void);

#endif	// __DMB_PROTOCOL_H__

/* end of file */

