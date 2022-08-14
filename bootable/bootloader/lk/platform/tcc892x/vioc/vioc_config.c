/******************************************************************************
*
*  (C)Copyright All Rights Reserved by Telechips Inc.
*
*  This material is confidential and shall remain as such.
*  Any unauthorized use, distribution, reproduction is strictly prohibited.
*
*******************************************************************************
*
*  File Name   : Vioc_config.c
*
*  Description :
*
*******************************************************************************
*
*  yyyy/mm/dd     ver            descriptions                Author
*	---------	--------   ---------------       -----------------
*    2011/08/08     0.1            created                      hskim
*******************************************************************************/
#include "vioc_config.h"

VIOC_CONFIG_PATH_u *VIOC_CONFIG_GetPathStruct (unsigned int nType)
{
	VIOC_CONFIG_PATH_u *pConfigPath = 0;
	volatile PVIOC_IREQ_CONFIG pIREQConfig;
	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)HwVIOC_IREQ;

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
	VIOC_CONFIG_PATH_u *pConfigPath = 0;
	unsigned int nStatus, loop = 100;

	pConfigPath = VIOC_CONFIG_GetPathStruct(nType);
	if(pConfigPath == 0) {
		printf("VIOC_CONFIG_PlugIn:  invalid path type. \n");
		return VIOC_DEVICE_INVALID;
	}

	BITCSET(pConfigPath->nREG, 0xFF, nValue);
	BITCSET(pConfigPath->nREG, (0x1<<31), (0x1<<31));

	if((pConfigPath->nREG>>18) & 0x1) {
		printf("VIOC_CONFIG_PlugIn:  path configuration error(1). device is busy. Type:%d Value:%d\n", nType, nValue);
		BITCSET(pConfigPath->nREG, (0x1<<31), (0x0<<31));
		return VIOC_DEVICE_BUSY;
	}

	while(1) {
		mdelay(1);
		loop--;
		nStatus = (pConfigPath->nREG>>16) & 0x3;
		if(nStatus == VIOC_PATH_CONNECTED) 	break;
		if(loop < 1) {
			printf("VIOC_CONFIG_PlugIn:  path configuration error(2). device is busy. Type:%d Value:%d\n", nType, nValue);
			return VIOC_DEVICE_BUSY;
		}
	}
    //printf(" @@@@@ PlugIn: device(%d) in RDMA%d \n", nType, nValue);

	return VIOC_DEVICE_CONNECTED;
}

int VIOC_CONFIG_PlugOut(unsigned int nType)
{
	VIOC_CONFIG_PATH_u *pConfigPath = 0;
	unsigned int nStatus, loop = 100;

	pConfigPath = VIOC_CONFIG_GetPathStruct(nType);
	if(pConfigPath == 0) {
		printf("VIOC_CONFIG_PlugOut:  invalid path type. \n");
		return VIOC_DEVICE_INVALID;
	}

	BITCSET(pConfigPath->nREG, (0x1<<31), (0x0<<31));

	if((pConfigPath->nREG>>18) & 0x1) {
		printf("VIOC_CONFIG_PlugOut:  path configuration error(1). device is busy. Type:%d\n", nType);
		BITCSET(pConfigPath->nREG, (0x1<<31), (0x0<<31));
		return VIOC_DEVICE_BUSY;
	}

	while(1) {
		mdelay(1);
		loop--;
		nStatus = (pConfigPath->nREG>>16) & 0x3;
		if(nStatus == VIOC_PATH_DISCONNECTED) 	break;
		if(loop < 1) {
			printf("VIOC_CONFIG_PlugOut:  path configuration error(2). device is busy. Type:%d\n", nType);
			return VIOC_DEVICE_BUSY;
		}
	}
    //printf(" @@@@@ PlugOut: device(%d) \n", nType);

	return VIOC_PATH_DISCONNECTED;
}

