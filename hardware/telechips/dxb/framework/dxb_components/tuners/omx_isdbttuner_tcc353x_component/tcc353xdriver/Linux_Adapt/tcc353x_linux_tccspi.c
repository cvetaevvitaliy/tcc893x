/****************************************************************************
 *   FileName    : tcc353x_linux_tccspi.c
 *   Description : tcc353x tcspi function for linux
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

#include "tcc353x_common.h"
#include "tcpal_os.h"
#include "SPI.h"
#include "spidev.h"

I32S Tcc353xTccspiClose(I32S _moduleIndex);

#define TCCSPI_SPEED	5000 * 1000//(20000 * 1000)

I32U gTccSpiHanleInit0 = 0;
I32U gTccSpiHanleInit1 = 0;
I32U gTccSpiHanleInited = 0;
I08U gTccSpiChipAddr[4];
I32S ghSPI[2] = { 0, };

static void Tcc353xTccspiSetup(I32S _moduleIndex)
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
		TcpalPrintStatus("[TCC353X] tccspi mode:%d\n", spi_mode);
		/*spi_mode |= SPI_MODE_3; */
		spi_mode = SPI_MODE_3;
		ioctl(ghSPI[0], SPI_IOC_WR_MODE, &spi_mode);


		spi_max_clock = TCCSPI_SPEED;
		ioctl(ghSPI[0], SPI_IOC_WR_MAX_SPEED_HZ, &spi_max_clock);
		ioctl(ghSPI[0], SPI_IOC_RD_MAX_SPEED_HZ, &spi_max_clock);
		TcpalPrintStatus("[TCC353X] spi_max_clock:%d\n",
				 spi_max_clock);
		TcpalPrintStatus((I08S *)
				 "[TCC353X] TccSpi open Success\n");
	}
	Tcc353xTccspiInit();
}

I32S Tcc353xTccspiOpen(I32S _moduleIndex)
{
	/* exception handling */
	if (_moduleIndex == 0) {
		if (gTccSpiHanleInit0 != 0 && gTccSpiHanleInit1 == 0)
			Tcc353xTccspiClose(_moduleIndex);
	} else {
		if (gTccSpiHanleInit1 != 0 && gTccSpiHanleInit0 == 0)
			Tcc353xTccspiClose(_moduleIndex);
	}

	/* normal process */
	if (_moduleIndex == 0)
		gTccSpiHanleInit0 = 1;
	else
		gTccSpiHanleInit1 = 1;

	if (gTccSpiHanleInited != 0) {
		return TCC353X_RETURN_SUCCESS;
	}

	gTccSpiHanleInited = 1;

	TcpalMemset(&gTccSpiChipAddr[_moduleIndex], 0x00, 4);

	Tcc353xTccspiSetup(_moduleIndex);

	/* need reset */

	return TCC353X_RETURN_SUCCESS;
}

I32S Tcc353xTccspiClose(I32S _moduleIndex)
{
	if (_moduleIndex == 0)
		gTccSpiHanleInit0 = 0;
	else
		gTccSpiHanleInit1 = 0;

	if (gTccSpiHanleInit0 == 0 && gTccSpiHanleInit1 == 0) {
		gTccSpiHanleInited = 0;

		TcpalPrintLog((I08S *) "[%d] Tcc353xTccspiClose \n",
			      _moduleIndex);
		close(ghSPI[0]);
	}

	return TCC353X_RETURN_SUCCESS;
}

I32S Tcc353xAdaptSpiReadWrite (I32S _moduleIndex, I08U * _bufferIn, I08U * _bufferOut,
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
		return TCC353X_RETURN_FAIL;
	}
	return TCC353X_RETURN_SUCCESS;
}
