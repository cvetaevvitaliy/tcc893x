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

#include "tcc317x_common.h"
#include "tcpal_os.h"
#include "SPI.h"
#include "spidev.h"

I32S Tcc317xTccspiClose(I32S _moduleIndex);

#define TCCSPI_SPEED	5000 * 1000//(20000 * 1000)

I32U gTccSpiHanleInit0 = 0;
I32U gTccSpiHanleInit1 = 0;
I32U gTccSpiHanleInited = 0;
I08U gTccSpiChipAddr[4];
I32S ghSPI[2] = { 0, };

static void Tcc317xTccspiSetup(I32S _moduleIndex)
{
	unsigned char spi_mode = 0;
	unsigned int spi_max_clock;

	if (_moduleIndex >= 2) {
		TcpalPrintErr((I08S *) "Not supported, moduleidx=%d\n",
			      _moduleIndex);
		return;
	}

	ghSPI[0] = open("/dev/spidev0.0", O_RDWR | O_NDELAY);

	if (ghSPI[0] < 0) {
		TcpalPrintErr((I08S *) "ghSPI INVALID_HANDLE_VALUE");
	} else {
		//SET SPI using IOCTL
		spi_mode = SPI_MODE_3;
		ioctl(ghSPI[0], SPI_IOC_RD_MODE, &spi_mode);
		TcpalPrintStatus("[TCC317X] tccspi mode:%d\n", spi_mode);
		/*spi_mode |= SPI_MODE_3; */
		spi_mode = SPI_MODE_3;
		ioctl(ghSPI[0], SPI_IOC_WR_MODE, &spi_mode);


		spi_max_clock = TCCSPI_SPEED;
		ioctl(ghSPI[0], SPI_IOC_WR_MAX_SPEED_HZ, &spi_max_clock);
		ioctl(ghSPI[0], SPI_IOC_RD_MAX_SPEED_HZ, &spi_max_clock);
		TcpalPrintStatus("[TCC317X] spi_max_clock:%d\n",
				 spi_max_clock);
		TcpalPrintStatus((I08S *)
				 "[TCC317X] TccSpi open Success\n");
	}
	Tcc317xTccspiInit();
}

I32S Tcc317xTccspiOpen(I32S _moduleIndex)
{
	/* exception handling */
	if (_moduleIndex == 0) {
		if (gTccSpiHanleInit0 != 0 && gTccSpiHanleInit1 == 0)
			Tcc317xTccspiClose(_moduleIndex);
	} else {
		if (gTccSpiHanleInit1 != 0 && gTccSpiHanleInit0 == 0)
			Tcc317xTccspiClose(_moduleIndex);
	}

	/* normal process */
	if (_moduleIndex == 0)
		gTccSpiHanleInit0 = 1;
	else
		gTccSpiHanleInit1 = 1;

	if (gTccSpiHanleInited != 0) {
		return TCC317X_RETURN_SUCCESS;
	}

	gTccSpiHanleInited = 1;

	TcpalMemset(&gTccSpiChipAddr[_moduleIndex], 0x00, 4);

	Tcc317xTccspiSetup(_moduleIndex);

	/* need reset */

	return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xTccspiClose(I32S _moduleIndex)
{
	if (_moduleIndex == 0)
		gTccSpiHanleInit0 = 0;
	else
		gTccSpiHanleInit1 = 0;

	if (gTccSpiHanleInit0 == 0 && gTccSpiHanleInit1 == 0) {
		gTccSpiHanleInited = 0;

		TcpalPrintLog((I08S *) "[%d] Tcc317xTccspiClose \n",
			      _moduleIndex);
		close(ghSPI[0]);
	}

	return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xAdaptSpiReadWrite (I32S _moduleIndex, I08U * _bufferIn, I08U * _bufferOut,
		  I32S _size, I08U _reservedOption)
{
	I32S bret;
	struct spi_ioc_transfer msg;

	bret = 0;
	TcpalMemset(&msg, 0x00, sizeof(msg));
	msg.tx_buf = _bufferIn;
	msg.rx_buf = _bufferOut;
	msg.len = _size;
	msg.speed_hz = TCCSPI_SPEED;
	msg.bits_per_word = 8;

	bret = ioctl(ghSPI[0], SPI_IOC_MESSAGE(1), &msg);
	if ((unsigned int) bret != _size) {
		TcpalPrintErr("[SPI.c] Error SpiWrite1(size:%d, ret:%d)\n",
			      _size, bret);
		return TCC317X_RETURN_FAIL;
	}
	return TCC317X_RETURN_SUCCESS;
}
