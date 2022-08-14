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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#ifdef HAVE_ANDROID_OS
#define LOG_TAG	"TCC317X_PAL_I2C"
#include <utils/Log.h>
#else
#include <stdio.h>
#define	LOGD		printf
#define	LOGE		printf
#define	LOGI		printf
#endif

#include "tcc317x_common.h"
#include "tcpal_os.h"

#define         MAX_I2C_BURST   512
#define         I2C_SLAVE       0x0703	/* Change slave address         */

I32U gI2CHanleInit0 = 0;
I32U gI2CHanleInit1 = 0;
I32U gI2CHanleInited = 0;
I08U gI2CChipAddr = 0x00;

I32S ghI2C[2] = { 0, };

I08U TempBuff[MAX_I2C_BURST + 4];

static void Tcc317xI2cSetup(I32S _moduleIndex)
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
		TcpalPrintStatus((I08S *) "# [TCC317X] I2C open Success\n");
	}
}

I32S Tcc317xI2cOpen(I32S _moduleIndex)
{

	if (_moduleIndex == 0)
		gI2CHanleInit0 = 1;
	else
		gI2CHanleInit1 = 1;

	if (gI2CHanleInited != 0) {
		return TCC317X_RETURN_SUCCESS;
	}

	gI2CHanleInited = 1;
	gI2CChipAddr = 0x00;

	Tcc317xI2cSetup(_moduleIndex);

	/* need reset */

	return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xI2cClose(I32S _moduleIndex)
{
	if (_moduleIndex == 0)
		gI2CHanleInit0 = 0;
	else
		gI2CHanleInit1 = 0;

	if (gI2CHanleInit0 == 0 && gI2CHanleInit1 == 0) {
		gI2CHanleInited = 0;

		TcpalPrintLog((I08S *) "[%d] TC317x_IO_I2C_Close \n",
			      _moduleIndex);
		close(ghI2C[0]);
	}

	return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xAdaptI2CReadEx(I32S _moduleIndex, I32S _chipAddress,
		    I08U _registerAddr, I08U * _outData, I32S _size)
{
	I08U buf[2];
	I32S cMax, remain;
	I32S berr, err;
	I32S i;

	berr = 0;
	cMax = _size / MAX_I2C_BURST;
	remain = _size % MAX_I2C_BURST;

	if (gI2CChipAddr != _chipAddress) {
		gI2CChipAddr = _chipAddress;
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
		return TCC317X_RETURN_FAIL;

	return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xAdaptI2CWriteEx(I32S _moduleIndex, I32S _chipAddress,
		     I08U _registerAddr, I08U * _inputData, I32S _size)
{
	I08U buf[2];
	I32S err, berr;
	I32S cMax, remain;
	I32S i;

	berr = 0;
	cMax = _size / MAX_I2C_BURST;
	remain = _size % MAX_I2C_BURST;

	if (gI2CChipAddr != _chipAddress) {
		gI2CChipAddr = _chipAddress;
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
		return TCC317X_RETURN_FAIL;
	return TCC317X_RETURN_SUCCESS;
}
