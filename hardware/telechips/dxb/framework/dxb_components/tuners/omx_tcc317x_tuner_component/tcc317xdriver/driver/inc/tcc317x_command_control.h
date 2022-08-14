/****************************************************************************
 *   FileName    : tcc317x_command_control.h
 *   Description : control command Function
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall 
constitute any express or implied warranty of any kind, including without limitation, any warranty 
of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright 
or other third party intellectual property right. No warranty is made, express or implied, 
regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of 
or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************/

#ifndef __TCC317X_COMMAND_CONTROL_H__
#define __TCC317X_COMMAND_CONTROL_H__

#include "tcc317x_core.h"

I32S Tcc317xMemoryRead(I32S _moduleIndex, I32S _diversityIndex, I32U _address, I08U *_data, I32U _size);
I32S Tcc317xMemoryWrite(I32S _moduleIndex, I32S _diversityIndex, I32U _address, I08U *_data, I32U _size);
I32S Tcc317xStreamRead(I32S _moduleIndex, I08U *_data, I32S _size);
I32S Tcc317xDspTableWrite (Tcc317xHandle_t * _handle, I32U _addr, I08U *_data, I32S _size);
I32S Tcc317xDspAsmWrite (Tcc317xHandle_t * _handle, I08U *_data, I32S _size);
I32S ReadProcess(Tcc317xHandle_t *_handle, I08U _registerAddr, I32S _size, I08U *_outData, I08U _unlock);
I32S WriteProcess(Tcc317xHandle_t *_handle, I08U _registerAddr, I08U *_inputData, I32S _size, I08U _unlock);
I32S Tcc317xRead(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size);
I32S Tcc317xWrite(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I08U *_data, I32U _size);
I32S Tcc317xReadIrqError(I32S _moduleIndex, I08U *_data);
I32S Tcc317xIrqClear(I32S _moduleIndex);
I32S Tcc317xRfRegisterRead(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I32U *_data);
I32S Tcc317xRfRegisterWrite(I32S _moduleIndex, I32S _diversityIndex, I08U _address, I32U _data);

#endif
