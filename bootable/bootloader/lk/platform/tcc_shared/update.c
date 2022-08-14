/***************************************************************************************
*	FileName    : update.c
*	Description : firmware update 
****************************************************************************************
*
*	TCC Board Support Package
*	Copyright (c) Telechips, Inc.
*	ALL RIGHTS RESERVED
*
****************************************************************************************/

//#include <common.h>
#include <debug.h>
#include <err.h>
#include <reg.h>
#include <string.h>
#include <platform.h>
//#include <def_tcc.h>
#include <i2c.h>
#include <platform/globals.h>
#include <platform/reg_physical.h>

#ifndef _TODO_
//#include <i2c.h>
//#include <clock.h>
#include <fwdn/Disk.h>
#include <fwdn/file.h>
#include <fwdn/FSAPP.h>
#include <sdmmc/sd_memory.h>

int gSDUpgrading = 0;

extern unsigned char pca9539u3_data[2];
extern int FWUG_MainFunc(int hFile, int iFileSize);
extern int VerifyROMFile(unsigned int *pBuffer,unsigned int size );

void sd_deinit(void)
{
	#if defined (PLATFORM_TCC892X) || defined(PLATFORM_TCC893X)
	unsigned int i2c_ch = I2C_CH_SMU;
	#else
	unsigned int i2c_ch = SMU_I2C_CH0;
	#endif
	unsigned char DestAddress;
	unsigned char i2cData_mode[3] = {0,0,0};
	unsigned char i2cData_data[3] = {0,0,0};
	//PPIC pPIC = (PIC *)&HwPIC_BASE;

	/* disable clock & io bus */
	//tca_ckc_setperi(PERI_SDMMC0, DISABLE, 120000);
	//tca_ckc_setiopwdn(RB_SDMMCCONTROLLER, 1);

	/* reset peri block */
	tca_ckc_setioswreset(RB_SDMMC0CONTROLLER, 1);
	tca_ckc_setioswreset(RB_SDMMC0CONTROLLER, 0);
	
	/* slot0,1 power down */
	DestAddress = 0xE8;
	i2cData_mode[0] = 6;
	i2cData_data[0] = 2;
	BITCLR(pca9539u3_data[0], Hw6|Hw7);
	i2cData_data[1] = pca9539u3_data[0];
	i2cData_data[2] = pca9539u3_data[1];
	i2c_xfer(DestAddress, 3, i2cData_mode, 0, 0, i2c_ch);
	i2c_xfer(DestAddress, 3, i2cData_data, 0, 0, i2c_ch);

	/* clear interrupt */
	//BITSET(pPIC->CLR1, Hw12|Hw13);
}
#endif

/************* end of file *************************************************************/
