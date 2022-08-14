/*
 * linux/arch/arm/mach-tcc892x/vioc_config.c
 * Author:  <linux@telechips.com>
 * Created: June 10, 2008
 * Description: TCC VIOC h/w block 
 *
 * Copyright (C) 2008-2009 Telechips
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <linux/kernel.h>
#include <linux/delay.h>
#include <mach/bsp.h>
#include <mach/io.h>
#include <linux/module.h>

#include <mach/vioc_plugin_tcc892x.h>
#include <mach/vioc_config.h>

#define dprintk(msg...)	if (0) { printk( "Vioc_config: " msg); }

#if 0
VIOC_CONFIG *gpConfig = (VIOC_CONFIG *)HwVIOC_PCONFIG;
void VIOC_CONFIG_SetWindowMixerPath(unsigned int nChannel, unsigned int nSub, unsigned int nValue)
{
	if(nSub == 0) {
		switch(nChannel) {
			case (VIOC_CONFIG_WMIX0) :
				gpConfig->uMISC.bReg.WMIX0_0 = nValue;
				gpConfig->uMISC.bReg.WMIX0_1 = nValue;
				break;
			case (VIOC_CONFIG_WMIX1) :
				gpConfig->uMISC.bReg.WMIX1_0 = nValue;
				gpConfig->uMISC.bReg.WMIX1_1 = nValue;
				break;
			case (VIOC_CONFIG_WMIX2) :
				gpConfig->uMISC.bReg.WMIX2_0 = nValue;
				gpConfig->uMISC.bReg.WMIX2_1 = nValue;
				break;
			default:
				break;
		}
	} else if (nSub == 1) {
		switch(nChannel) {
			case (VIOC_CONFIG_WMIX0) :
				gpConfig->uMISC.bReg.RDMA03 = nValue;
				break;
			default:
				break;
		}
	}
}
#endif

VIOC_CONFIG_PATH_u *VIOC_CONFIG_GetPathStruct (unsigned int nType)
{
	VIOC_CONFIG_PATH_u *pConfigPath = NULL;
	volatile PVIOC_IREQ_CONFIG pIREQConfig;
	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

	switch(nType) {
		case (VIOC_SC0):
			pConfigPath = (VIOC_CONFIG_PATH_u *)&pIREQConfig->uSC0.nREG;
			break;
		case (VIOC_SC1):
			pConfigPath = (VIOC_CONFIG_PATH_u *)&pIREQConfig->uSC1.nREG;
			break;
		case (VIOC_SC2):
			pConfigPath = (VIOC_CONFIG_PATH_u *)&pIREQConfig->uSC2.nREG;
			break;
		case (VIOC_SC3):
			pConfigPath = (VIOC_CONFIG_PATH_u *)&pIREQConfig->uSC3.nREG;
			break;
		case (VIOC_VIQE):
			pConfigPath = (VIOC_CONFIG_PATH_u *)&pIREQConfig->uVIQE.nREG;
			break;
		case (VIOC_DEINTLS):
			pConfigPath = (VIOC_CONFIG_PATH_u *)&pIREQConfig->uDEINTLS.nREG;
			break;
		//case (VIOC_VIQE):
		//case (VIOC_DEINTLS):
		//case (VIOC_FILT2D   ) : return (&gpConfig->uFILT2D.bReg);
		//case (VIOC_FCDEC0   ) : return (&gpConfig->uFCDEC0.bReg);
		//case (VIOC_FCDEC1   ) : return (&gpConfig->uFCDEC1.bReg);
		//case (VIOC_FCDEC2   ) : return (&gpConfig->uFCDEC2.bReg);
		//case (VIOC_FCDEC3   ) : return (&gpConfig->uFCDEC3.bReg);
		//case (VIOC_FCENC0   ) : return (&gpConfig->uFCENC0.bReg);
		//case (VIOC_FCENC1   ) : return (&gpConfig->uFCENC1.bReg);
		//case (VIOC_FDELAY0  ) : return (&gpConfig->uFDELAY0.bReg);
		//case (VIOC_FDELAY1  ) : return (&gpConfig->uFDELAY1.bReg);
		//case (VIOC_DEBLOCK  ) : return (&gpConfig->uDEBLOCK.bReg);
		default:
			break;
	}

	return pConfigPath;
}

int VIOC_CONFIG_PlugIn (unsigned int nType, unsigned int nValue)
{
	VIOC_CONFIG_PATH_u *pConfigPath = NULL;
	unsigned int nStatus, nSelect, loop = 100;

	pConfigPath = VIOC_CONFIG_GetPathStruct(nType);
	if(pConfigPath == NULL) {
		printk("VIOC_CONFIG_PlugIn:  invalid path type. \n");
		return VIOC_DEVICE_INVALID;
	}

	nStatus = (pConfigPath->nREG>>16) & 0x3;
	nSelect = pConfigPath->nREG & 0xFF;
	if((nStatus == VIOC_PATH_CONNECTED) && (nSelect != nValue))
	{
		printk("%s, Type(%d) is plugged-out by force!!\n", __func__, nType);
		VIOC_CONFIG_PlugOut(nType);
	}
		
	BITCSET(pConfigPath->nREG, 0xFF, nValue);
	BITCSET(pConfigPath->nREG, (0x1<<31), (0x1<<31));

	if((pConfigPath->nREG>>18) & 0x1) {
		printk("VIOC_CONFIG_PlugIn:  path configuration error(1). device is busy. Type:%d Value:%d\n", nType, nValue);
		BITCSET(pConfigPath->nREG, (0x1<<31), (0x0<<31));
		return VIOC_DEVICE_BUSY;
	}

	while(1) {
		mdelay(1);
		loop--;
		nStatus = (pConfigPath->nREG>>16) & 0x3;
		if(nStatus == VIOC_PATH_CONNECTED) 	break;
		if(loop < 1) {
			printk("VIOC_CONFIG_PlugIn:  path configuration error(2). device is busy. Type:%d Value:%d\n", nType, nValue);
			return VIOC_DEVICE_BUSY;
		}
	}
    dprintk(" @@@@@ PlugIn: device(%d) in RDMA%d \n", nType, nValue);

	return VIOC_DEVICE_CONNECTED;
}EXPORT_SYMBOL(VIOC_CONFIG_PlugIn);

int VIOC_CONFIG_PlugOut(unsigned int nType)
{
	VIOC_CONFIG_PATH_u *pConfigPath = NULL;
	unsigned int nStatus, loop = 100;

	pConfigPath = VIOC_CONFIG_GetPathStruct(nType);
	if(pConfigPath == NULL) {
		printk("VIOC_CONFIG_PlugOut:  invalid path type. \n");
		return VIOC_DEVICE_INVALID;
	}

	nStatus = (pConfigPath->nREG>>16) & 0x3;
	if(nStatus == VIOC_PATH_DISCONNECTED)
	{
		printk("%s, Type(%d) was already plugged-out!!\n", __func__, nType);
		return VIOC_PATH_DISCONNECTED;
	}

	BITCSET(pConfigPath->nREG, (0x1<<31), (0x0<<31));

	if((pConfigPath->nREG>>18) & 0x1) {
		printk("VIOC_CONFIG_PlugOut:  path configuration error(1). device is busy. Type:%d\n", nType);
		BITCSET(pConfigPath->nREG, (0x1<<31), (0x0<<31));
		return VIOC_DEVICE_BUSY;
	}

	while(1) {
		mdelay(1);
		loop--;
		nStatus = (pConfigPath->nREG>>16) & 0x3;
		if(nStatus == VIOC_PATH_DISCONNECTED) 	break;
		if(loop < 1) {
			printk("VIOC_CONFIG_PlugOut:  path configuration error(2). device is busy. Type:%d\n", nType);
			return VIOC_DEVICE_BUSY;
		}
	}
    dprintk(" @@@@@ PlugOut: device(%d) \n", nType);

	return VIOC_PATH_DISCONNECTED;
}EXPORT_SYMBOL(VIOC_CONFIG_PlugOut);

void VIOC_CONFIG_RDMA12PathCtrl(unsigned int Path)
{
	/* Path - 0:  RDMA12 PATH , 	1:  VIDEOIN2 PATH */
	volatile PVIOC_IREQ_CONFIG pRDMAPath = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_CONFIG);
	//pRDMAPath->RDMA12 = Path;
	BITCSET(pRDMAPath->uMISC.nREG, (0x1<<30), (Path<<30));
}

void VIOC_CONFIG_RDMA14PathCtrl(unsigned int Path)
{
	/* Path - 0:  RDMA14 PATH , 	1:  VIDEOIN3 PATH */
	volatile PVIOC_IREQ_CONFIG pRDMAPath = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_CONFIG);
	//pRDMAPath->RDMA14 = Path;
	BITCSET(pRDMAPath->uMISC.nREG, (0x1<<31), (Path<<31));
}

void VIOC_CONFIG_WMIXPath(unsigned int Path, unsigned int Mode)
{
	/* Mode - 0: BY-PSSS PATH , 	1:  WMIX PATH */
	volatile PVIOC_IREQ_CONFIG pWMIXPath = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_CONFIG);

	switch(Path) {
		case WMIX00:
			//pWMIXPath->WMIX0_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<16), (Mode<<16));
			break;
		case WMIX03:
			//pWMIXPath->WMIX0_1 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<17), (Mode<<17));
			break;
		case WMIX10:
			//pWMIXPath->WMIX1_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<18), (Mode<<18));
			break;
		case WMIX13:
			//pWMIXPath->WMIX1_1 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<19), (Mode<<19));
			break;
		case WMIX30:
			//pWMIXPath->WMIX3_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<22), (Mode<<22));
			break;
		case WMIX40:
			//pWMIXPath->WMIX4_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<24), (Mode<<24));
			break;
		case WMIX50:
			//pWMIXPath->WMIX5_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<26), (Mode<<26));
			break;
		case WMIX60:
			//pWMIXPath->WMIX6_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<28), (Mode<<28));
			break;
	}
}
EXPORT_SYMBOL(VIOC_CONFIG_WMIXPath);

int VIOC_CONFIG_CheckPlugInOut(unsigned int nDevice, unsigned int *nPath)
{
	VIOC_CONFIG_PATH_u *pConfigPath = NULL;
	unsigned int nStatus = VIOC_PATH_DISCONNECTED;
	//unsigned int tmp;
	//int ret = -1;

	pConfigPath = VIOC_CONFIG_GetPathStruct(nDevice);
	if(pConfigPath == NULL) {
		printk("Invalid Path Type. \n");
		return (VIOC_DEVICE_INVALID);
	}

	mdelay(1);
	*nPath = pConfigPath->nREG & 0xFF;
	nStatus = (pConfigPath->nREG>>16) & 0x3;

	return nStatus;
}

/*
VIOC_CONFIG_Device_PlugState 
Check PlugInOut status of VIOC SCALER, VIQE, DEINTLS.
nDevice : VIOC_SC0, VIOC_SC1, VIOC_SC2, VIOC_VIQE, VIOC_DEINTLS
pDstatus : Pointer of status value.
return value : Device name of Plug in.
*/
int VIOC_CONFIG_Device_PlugState(unsigned int nDevice, VIOC_PlugInOutCheck *VIOC_PlugIn)
{
	VIOC_CONFIG_PATH_u *pConfigPath = NULL;
	//unsigned int nStatus = VIOC_PATH_DISCONNECTED;
	//unsigned int tmp;
	//int ret = -1;

	pConfigPath = VIOC_CONFIG_GetPathStruct(nDevice);
	if(pConfigPath == NULL)	{
		return (VIOC_DEVICE_INVALID);
	}
	
	VIOC_PlugIn->enable = pConfigPath->bREG.EN;
	VIOC_PlugIn->connect_device = pConfigPath->bREG.SELECT;
	VIOC_PlugIn->connect_statue = pConfigPath->bREG.STATUS;

	return VIOC_DEVICE_CONNECTED;
}

