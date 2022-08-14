/****************************************************************************
 *   FileName    : tcc353x_pal_i2c.c
 *   Description : i2c Platform adaptive layer
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

#include "tcc353x_common.h"
#include "tcpal_os.h"

extern int Tcc353xAdaptI2CReadEx(int moduleidx, int ChipAddr, int RegAddr,
		     unsigned char *cData, int iSize);
extern int Tcc353xAdaptI2CWriteEx(int moduleidx, int ChipAddr, int RegAddr,
		      unsigned char *cData, int iSize);

I32S Tcc353xI2cRead(I32S _moduleIndex, I32S _chipAddress,
		    I08U _registerAddr, I08U * _outData, I32S _size)
{
	I32S ret;
	ret = Tcc353xAdaptI2CReadEx (_moduleIndex, _chipAddress, _registerAddr,
				 _outData, _size);
	return ret;
}

I32S Tcc353xI2cWrite(I32S _moduleIndex, I32S _chipAddress,
		     I08U _registerAddr, I08U * _inputData, I32S _size)
{
	I32S ret;
	ret = Tcc353xAdaptI2CWriteEx(_moduleIndex, _chipAddress,
				  _registerAddr, _inputData, _size);
	return ret;
}
