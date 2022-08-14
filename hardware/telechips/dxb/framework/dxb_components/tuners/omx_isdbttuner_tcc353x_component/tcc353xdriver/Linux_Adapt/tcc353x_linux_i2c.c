/****************************************************************************
 *   FileName    : tcc353x_linux_i2c.c
 *   Description : tcc353x i2c function for linux
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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#ifdef HAVE_ANDROID_OS
#define LOG_TAG	"TCC353X_PAL_I2C"
#include <utils/Log.h>
#else
#include <stdio.h>
#define	LOGD		printf
#define	LOGE		printf
#define	LOGI		printf
#endif

#include "tcc353x_common.h"
#include "tcpal_os.h"
#if defined (USE_TELECHIPS_TCC79_MUSE_NUCLEUS_ADAPT)
#include "tcc353x_muse_i2c.h"
#endif

#define         MAX_I2C_BURST   512
#define         I2C_SLAVE       0x0703	/* Change slave address         */

I32U gI2CHanleInit0 = 0;
I32U gI2CHanleInit1 = 0;
I32U gI2CHanleInited = 0;
I08U gI2CChipAddr[4];

I32S ghI2C[2] = { 0, };

I08U TempBuff[MAX_I2C_BURST + 4];

static void Tcc353xI2cSetup(I32S _moduleIndex)
{
	if (_moduleIndex >= 2) {
		TcpalPrintErr((I08S *) "Not supported, moduleidx=%d\n",
			      _moduleIndex);
		return;
	}

	ghI2C[0] = open("/dev/i2c-1", O_RDWR);
	if (ghI2C[0] < 0) {
		TcpalPrintErr((I08S *) "ghI2C INVALID_HANDLE_VALUE");
	} else {
		TcpalPrintStatus((I08S *) "[TCC353X] I2C open Success\n");
	}
}

I32S Tcc353xI2cOpen(I32S _moduleIndex)
{

	if (_moduleIndex == 0)
		gI2CHanleInit0 = 1;
	else
		gI2CHanleInit1 = 1;

	if (gI2CHanleInited != 0) {
		return TCC353X_RETURN_SUCCESS;
	}

	gI2CHanleInited = 1;

	TcpalMemset(&gI2CChipAddr[_moduleIndex], 0x00, 4);

	Tcc353xI2cSetup(_moduleIndex);

	/* need reset */

	return TCC353X_RETURN_SUCCESS;
}

I32S Tcc353xI2cClose(I32S _moduleIndex)
{
	if (_moduleIndex == 0)
		gI2CHanleInit0 = 0;
	else
		gI2CHanleInit1 = 0;

	if (gI2CHanleInit0 == 0 && gI2CHanleInit1 == 0) {
		gI2CHanleInited = 0;

		TcpalPrintLog((I08S *) "[%d] TC3X_IO_I2C_Close \n",
			      _moduleIndex);
		close(ghI2C[0]);
	}

	return TCC353X_RETURN_SUCCESS;
}

I32S Tcc353xAdaptI2CReadEx(I32S _moduleIndex, I32S _chipAddress,
		    I08U _registerAddr, I08U * _outData, I32S _size)
{
	I08U buf[2];
	I32S cMax, remain;
	I32S berr, err;
	I32S i;

	berr = 0;
	cMax = _size / MAX_I2C_BURST;
	remain = _size % MAX_I2C_BURST;

	if (gI2CChipAddr[_moduleIndex] != _chipAddress) {
		gI2CChipAddr[_moduleIndex] = _chipAddress;
		ioctl(ghI2C[0], I2C_SLAVE, _chipAddress >> 1);
	}

	for (i = 0; i < cMax; i++) {
		buf[0] = _registerAddr;
		err = write(ghI2C[0], &buf[0], 1);

		if (err < 1) {
			TcpalPrintErr((I08S *)
				      "I2C Multi read 8 Addr Error!!\n");
			berr = 1;
			break;
		}

		err =
		    read(ghI2C[0], &_outData[i * MAX_I2C_BURST],
			 MAX_I2C_BURST);

		if (err < 0) {
			TcpalPrintErr((I08S *)
				      "I2C Multi read 8 data Error!!\n");
			berr = 1;
			break;
		}
	}

	if (remain && !berr) {
		buf[0] = _registerAddr;
		err = write(ghI2C[0], &buf[0], 1);

		if (err < 1) {
			TcpalPrintErr((I08S *)
				      "I2C Multi read 8 Addr Error!!\n");
			berr = 1;
		}

		err =
		    read(ghI2C[0], &_outData[cMax * MAX_I2C_BURST],
			 remain);

		if (err < 0) {
			TcpalPrintErr((I08S *)
				      "I2C Multi read 8 data Error!!\n");
			berr = 1;
		}
	}

	if (berr == 1)
		return TCC353X_RETURN_FAIL;

	return TCC353X_RETURN_SUCCESS;
}

I32S Tcc353xAdaptI2CWriteEx(I32S _moduleIndex, I32S _chipAddress,
		     I08U _registerAddr, I08U * _inputData, I32S _size)
{
	I08U buf[2];
	I32S err, berr;
	I32S cMax, remain;
	I32S i;

	berr = 0;
	cMax = _size / MAX_I2C_BURST;
	remain = _size % MAX_I2C_BURST;

	if (gI2CChipAddr[_moduleIndex] != _chipAddress) {
		gI2CChipAddr[_moduleIndex] = _chipAddress;
		ioctl(ghI2C[0], I2C_SLAVE, _chipAddress >> 1);
	}

	TempBuff[0] = (unsigned char) (_registerAddr);
	for (i = 0; i < cMax; i++) {
		TcpalMemcpy(&TempBuff[1], &_inputData[i * MAX_I2C_BURST],
			    MAX_I2C_BURST);
		err = write(ghI2C[0], &TempBuff[0], MAX_I2C_BURST + 1);

		if (err < 1) {
			berr = 1;
			TcpalPrintErr((I08S *)
				      "I2C Multi write 8 Error!!\n");
			break;
		}
	}

	if (remain && !berr) {
		TcpalMemcpy(&TempBuff[1],
			    &_inputData[cMax * MAX_I2C_BURST], remain);
		err = write(ghI2C[0], &TempBuff[0], remain + 1);

		if (err < 1) {
			TcpalPrintErr((I08S *)
				      "I2C Multi write 8 Error!!\n");
		}
	}

	if (berr)
		return TCC353X_RETURN_FAIL;
	return TCC353X_RETURN_SUCCESS;
}
