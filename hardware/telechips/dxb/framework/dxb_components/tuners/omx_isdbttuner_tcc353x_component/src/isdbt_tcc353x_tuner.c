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

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/
#define LOG_TAG	"TUNER_TCC353X"
#include <utils/Log.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "tcc_tsif.h"
#include "tcc_dxb_control.h"
#include "tcc353x_api.h"
#include "tcpal_os.h"

#include "isdbt_tcc353x_tuner.h"
#include "isdbt_tuner_space.h"
#include "tcc353x_boot_xx.h"
#include "tcc353x_boot_ax.h"

static I32S g_tcc_dxb_ctrl_fd = -1;
static I32S g_curr_channel = -1;
Tcc353xOption_t Tcc353xOptions[4];

extern I32S Tcc353xI2cOpen(I32S _moduleIndex);
extern I32S Tcc353xI2cClose(I32S _moduleIndex);
extern I32S Tcc353xTccspiOpen(I32S _moduleIndex);
extern I32S Tcc353xTccspiClose(I32S _moduleIndex);

extern Tcc353xRegisterConfig_t Tcc353xSingle[3];
extern Tcc353xRegisterConfig_t Tcc353xDiversityMaster[3];
extern Tcc353xRegisterConfig_t Tcc353xDiversitySlaveLast[3];
extern Tcc353xRegisterConfig_t Tcc353xDiversitySlaveMid[3];

extern Tcc353xOption_t Tcc353xOptionSingle;
extern Tcc353xOption_t Tcc353xOption2Diversity[2];
extern Tcc353xOption_t Tcc353xOption3Diversity[3];
extern Tcc353xOption_t Tcc353xOption4Diversity[4];

extern const unsigned char
    TCC353X_BOOT_DATA_ISDBT13SEG[TCC353X_BOOT_SIZE_ISDBT13SEG];

unsigned int FrequencyInfo[4] = {0,0,0,0};

I32S CurrentNumOfDiversity = 1;
I32S Tcc353xDxbControlType = 0;	/* Master control #0 pin, SlaveN control #1 pin */

/**************************************************************************
*  FUNCTION NAME : 
*    I32S tcc353x_init(I32S , I32S )
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
I32S tcc353x_init(I32S commandInterface, I32S streamInterface)
{
	I32S iBoardType, err, ret;
	I32U deviceIdx;
	ST_CTRLINFO_ARG stCtrl;
	Tcc353xStreamFormat_t streamFormat;
	I32S chip_version = VER_TCC353X_DONT_CARE;

	int i;

#if defined (_USE_TCC3530_)
	I32U rfSwitchingGpioCtrl[4] = {(1<<GPIO_NUM_RF_SWITCHING_TCC3530),0,0,0};
	I32U rfSwitchingVhfLow[4] =  {0,0,0,0};
	I32U rfSwitchingVhfHigh[4] = {0,0,0,0};
	I32U rfSwitchingUhf[4] = {(1<<GPIO_NUM_RF_SWITCHING_TCC3530),0,0,0};
#elif defined (_USE_TCC3532_)
	I32U rfSwitchingGpioCtrl[4] = {(1<<GPIO_NUM_RF_SWITCHING_TCC3532_SINGLE),0,0,0};
	I32U rfSwitchingVhfLow[4] =  {0,0,0,0};
	I32U rfSwitchingVhfHigh[4] = {0,0,0,0};
	I32U rfSwitchingUhf[4] = {(1<<GPIO_NUM_RF_SWITCHING_TCC3532_SINGLE),0,0,0};
#elif defined (_USE_TCC353X_TRIPLE_BAND_) && defined (_USE_TCC3531_)
	I32U rfSwitchingGpioCtrl[4] = 
		{(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING1) | (1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING2) | (1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING3),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE1) | (1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE2) | (1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE3),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE1) | (1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE2) | (1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE3),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE1) | (1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE2) | (1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE3)};
	I32U rfSwitchingVhfLow[4] =  
		{(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING3),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE3),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE3),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE3)};
	I32U rfSwitchingVhfHigh[4] = 
		{(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING2),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE2),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE2),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE2)};
	I32U rfSwitchingUhf[4] = 
		{(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING1),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE1),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE1),
		(1<<GPIO_NUM_TRIPLE_BAND_RF_SWITCHING_SLAVE1)};
#else
	I32U rfSwitchingGpioCtrl[4] = {0,0,0,0};
	I32U rfSwitchingVhfLow[4] =  {0,0,0,0};
	I32U rfSwitchingVhfHigh[4] = {0,0,0,0};
	I32U rfSwitchingUhf[4] = {0,0,0,0};
#endif


#if defined (_USE_TCC353X_2DIVERSITY_)
	CurrentNumOfDiversity = 2; 
#elif defined (_USE_TCC353X_3DIVERSITY_)
	CurrentNumOfDiversity = 3; 
#elif defined (_USE_TCC353X_4DIVERSITY_)
	CurrentNumOfDiversity = 4; 
#else
	CurrentNumOfDiversity = 1; 
#endif

#if defined (_DXB_PWRCTRL_1_3_)
	/* Master control #0 pin, SlaveN control #1 pin */
	Tcc353xDxbControlType = 0;
#elif defined (_DXB_PWRCTRL_4_)
	/* All baseband control #0 pin	*/
	Tcc353xDxbControlType = 1; 
#elif defined (_DXB_PWRCTRL_2_2_)
	/* Master, Slave1 control #0 pin, Slave2~3 control  #1 pin */
	Tcc353xDxbControlType = 2; 
#else
	/* Master control #0 pin, SlaveN control #1 pin */
	Tcc353xDxbControlType = 0; 
#endif

	/* power control */
	g_tcc_dxb_ctrl_fd = open(DXB_CTRL_DEV_FILE, O_RDWR | O_NDELAY);
	if (g_tcc_dxb_ctrl_fd < 0) {
		ALOGD("CANNOT open %s :: %d\n", DXB_CTRL_DEV_FILE,
		     (int) g_tcc_dxb_ctrl_fd);
		return -1;
	}

 	/* for full module */
	/*
	iBoardType = BOARD_ISDBT_TCC353X_FSMA;
	*/
	iBoardType = BOARD_ISDBT_TCC353X;	/* for evm */

	err =
	    ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_SET_BOARD,
		  &iBoardType);
	if (err != 0) {
		close(g_tcc_dxb_ctrl_fd);
		g_tcc_dxb_ctrl_fd = -1;
		ALOGD("ioctl(0x%x) error :: %d\n", IOCTL_DXB_CTRL_SET_BOARD,
		     (int) err);
		return -1;
	}

	err =
	    ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_GET_CTLINFO, &stCtrl);
	if (err != 0) {
		close(g_tcc_dxb_ctrl_fd);
		g_tcc_dxb_ctrl_fd = -1;
		ALOGD("ioctl(0x%x) error :: %d\n",
		     IOCTL_DXB_CTRL_GET_CTLINFO, (int) err);
		return -1;
	}

	/* power off - on */
	switch(Tcc353xDxbControlType) {
	case 0 :
		/* Master control #0 pin, SlaveN control #1 pin */
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
		if(CurrentNumOfDiversity>1) {
			deviceIdx = 1;
			err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
		}
	break;
	case 1 :
		/* All baseband control #0 pin	*/
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
	break;
	case 2 : 
		/* Master, Slave1 control #0 pin, Slave2~3 control  #1 pin */
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
		if(CurrentNumOfDiversity>2) {
			deviceIdx = 1;
			err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
		}
	break;
	default :
		/* Master control #0 pin, SlaveN control #1 pin */
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
		deviceIdx = 1;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
	break;
	}
	usleep(10000);

	/* open I2C/TCCSPI Driver and Initialize */
	if (commandInterface == TCC353X_IF_TCCSPI)
		Tcc353xTccspiOpen(0);
	else if (commandInterface == TCC353X_IF_I2C)
		Tcc353xI2cOpen(0);
	else
		Tcc353xTccspiOpen(0);

	switch(Tcc353xDxbControlType) {
	case 0 :
		/* Master control #0 pin, SlaveN control #1 pin */
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_ON, &deviceIdx);
		if(CurrentNumOfDiversity>1) {
			deviceIdx = 1;
			err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_ON, &deviceIdx);
		}
	break;
	case 1 :
		/* All baseband control #0 pin	*/
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_ON, &deviceIdx);
	break;
	case 2 : 
		/* Master, Slave1 control #0 pin, Slave2~3 control  #1 pin */
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_ON, &deviceIdx);
		if(CurrentNumOfDiversity>2) {
			deviceIdx = 1;
			err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_ON, &deviceIdx);
		}
	break;
	default :
		/* Master control #0 pin, SlaveN control #1 pin */
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_ON, &deviceIdx);
		deviceIdx = 1;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_ON, &deviceIdx);
	break;
	}
	usleep(10000);

	/* reset */
	switch(Tcc353xDxbControlType) {
	case 0 :
		/* Master control #0 pin, SlaveN control #1 pin */
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_RESET, &deviceIdx);
		if(CurrentNumOfDiversity>1) {
			deviceIdx = 1;
			err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_RESET, &deviceIdx);
		}
	break;
	case 1 :
		/* All baseband control #0 pin	*/
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_RESET, &deviceIdx);
	break;
	case 2 : 
		/* Master, Slave1 control #0 pin, Slave2~3 control  #1 pin */
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_RESET, &deviceIdx);
		if(CurrentNumOfDiversity>2) {
			deviceIdx = 1;
			err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_RESET, &deviceIdx);
		}
	break;
	default :
		/* Master control #0 pin, SlaveN control #1 pin */
		deviceIdx = 0;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_RESET, &deviceIdx);
		deviceIdx = 1;
		err = ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_RESET, &deviceIdx);
	break;
	}
	usleep(10000);

	if (err != 0) {
		close(g_tcc_dxb_ctrl_fd);
		g_tcc_dxb_ctrl_fd = -1;
		ALOGD("ioctl(0x%x) error :: %d\n",
		     IOCTL_DXB_CTRL_GET_CTLINFO, (int) err);
		return -1;
	}

	/* open TCC353X Driver  */
	switch(CurrentNumOfDiversity) {
	case 1:
		TcpalPrintLog((I08S *) "[TCC353X] Single Mode \n");
		TcpalMemcpy (&Tcc353xOptions[0], &Tcc353xOptionSingle, 
		    sizeof(Tcc353xOption_t)*CurrentNumOfDiversity);
		Tcc353xOptions[0].Config =
		    (Tcc353xRegisterConfig_t *) (&Tcc353xSingle[1]);
	break;
	case 2:
		TcpalPrintLog((I08S *) "[TCC353X] 2-Diversity Mode \n");
		TcpalMemcpy (&Tcc353xOptions[0], &Tcc353xOption2Diversity[0], 
		    sizeof(Tcc353xOption_t)*CurrentNumOfDiversity);
		Tcc353xOptions[0].Config =
		    (Tcc353xRegisterConfig_t *) (&Tcc353xDiversityMaster[1]);
		Tcc353xOptions[1].Config =
		    (Tcc353xRegisterConfig_t *) (&Tcc353xDiversitySlaveLast[1]);
	break;
	case 3:
		TcpalPrintLog((I08S *) "[TCC353X] 3-Diversity Mode \n");
		TcpalMemcpy (&Tcc353xOptions[0], &Tcc353xOption3Diversity[0], 
		    sizeof(Tcc353xOption_t)*CurrentNumOfDiversity);
		Tcc353xOptions[0].Config =
		    (Tcc353xRegisterConfig_t *) (&Tcc353xDiversityMaster[1]);
		Tcc353xOptions[1].Config =
		    (Tcc353xRegisterConfig_t *) (&Tcc353xDiversitySlaveMid[1]);
		Tcc353xOptions[2].Config =
		    (Tcc353xRegisterConfig_t *) (&Tcc353xDiversitySlaveLast[1]);
	break;
	case 4:
		TcpalPrintLog((I08S *) "[TCC353X] 4-Diversity Mode \n");
		TcpalMemcpy (&Tcc353xOptions[0], &Tcc353xOption4Diversity[0], 
		    sizeof(Tcc353xOption_t)*CurrentNumOfDiversity);
		Tcc353xOptions[0].Config =
		    (Tcc353xRegisterConfig_t *) (&Tcc353xDiversityMaster[1]);
		Tcc353xOptions[1].Config =
			(Tcc353xRegisterConfig_t *) (&Tcc353xDiversitySlaveMid[1]);
		Tcc353xOptions[2].Config =
			(Tcc353xRegisterConfig_t *) (&Tcc353xDiversitySlaveMid[1]);
		Tcc353xOptions[3].Config =
		    (Tcc353xRegisterConfig_t *) (&Tcc353xDiversitySlaveLast[1]);
	break;
	default:
		TcpalPrintLog((I08S *) "[TCC353X] Single Mode \n");
		TcpalMemcpy (&Tcc353xOptions[0], &Tcc353xOptionSingle, 
		    sizeof(Tcc353xOption_t));
		Tcc353xOptions[0].Config =
		    (Tcc353xRegisterConfig_t *) (&Tcc353xSingle[1]);
	break;
	}

	for(i=0; i<CurrentNumOfDiversity; i++) {
#if defined (_USE_TCC353X_TRIPLE_BAND_)
		Tcc353xOptions[i].rfType = TCC353X_TRIPLE_BAND_RF;
#else
		Tcc353xOptions[i].rfType = TCC353X_DUAL_BAND_RF;
#endif
		Tcc353xOptions[i].commandInterface = commandInterface;
		Tcc353xOptions[i].streamInterface = streamInterface;

#if !defined (_USE_TCC353X_TRIPLE_BAND_) && defined (_USE_TCC3531_)
		/* default use options */
#else
		Tcc353xOptions[i].rfSwitchingGpioN = rfSwitchingGpioCtrl[i];
		Tcc353xOptions[i].rfSwitchingUhf = rfSwitchingUhf[i];
		Tcc353xOptions[i].rfSwitchingVhfHigh = rfSwitchingVhfHigh[i];
		Tcc353xOptions[i].rfSwitchingVhfLow= rfSwitchingVhfLow[i];
#endif

#if defined (_USE_TCC3530_)
		Tcc353xOptions[i].basebandName = BB_TCC3530;
#elif defined (_USE_TCC3532_)
		Tcc353xOptions[i].basebandName = BB_TCC3532;
#elif defined (_USE_TCC3531_)
		Tcc353xOptions[i].basebandName = BB_TCC3531;
#else
		Tcc353xOptions[i].basebandName = BB_TCC3531;
#endif
	}

	ret = Tcc353xApiOpen(0, &Tcc353xOptions[0], sizeof(Tcc353xOption_t)*CurrentNumOfDiversity);
	if(ret<TCC353X_RETURN_SUCCESS) {
		TcpalPrintLog((I08S *) "[TCC353X] Open Fail!!!\n");
		tcc353x_deinit(commandInterface, streamInterface);
		return -1;
	}
	chip_version = ret;
	streamFormat.pidFilterEnable = 0;
	streamFormat.syncByteFilterEnable = 1;
	streamFormat.tsErrorFilterEnable = 0;
	streamFormat.tsErrorInsertEnable = 1;

#if defined (_USE_TCC3530_)
	ret = Tcc353xApiInit(0, &TCC353X_BOOT_DATA_ISDBT13SEG[0],
	    TCC353X_BOOT_SIZE_ISDBT13SEG, &streamFormat);
#elif defined (_USE_TCC3532_)
	if(chip_version==VER_TCC3532_0AX) {
		ret = Tcc353xApiInit(0, &TCC353X_BOOT_DATA_ISDBT13SEG_AX[0],
		    TCC353X_BOOT_SIZE_ISDBT13SEG_AX, &streamFormat);
	} else if(chip_version==VER_TCC3532_0XX) {
		ret = Tcc353xApiInit(0, &TCC353X_BOOT_DATA_ISDBT13SEG[0],
		    TCC353X_BOOT_SIZE_ISDBT13SEG, &streamFormat);
	} else {
		ret = Tcc353xApiInit(0, &TCC353X_BOOT_DATA_ISDBT13SEG_AX[0],
		    TCC353X_BOOT_SIZE_ISDBT13SEG_AX, &streamFormat);
	}
#elif defined (_USE_TCC3531_)
	if(chip_version==VER_TCC3531_2AX) {
		ret = Tcc353xApiInit(0, &TCC353X_BOOT_DATA_ISDBT13SEG_AX[0],
		    TCC353X_BOOT_SIZE_ISDBT13SEG_AX, &streamFormat);
	} else if(chip_version==VER_TCC3531_1XX) {
		ret = Tcc353xApiInit(0, &TCC353X_BOOT_DATA_ISDBT13SEG[0],
		    TCC353X_BOOT_SIZE_ISDBT13SEG, &streamFormat);
	} else {
		ret = Tcc353xApiInit(0, &TCC353X_BOOT_DATA_ISDBT13SEG_AX[0],
		    TCC353X_BOOT_SIZE_ISDBT13SEG_AX, &streamFormat);
	}
#else
	ret = Tcc353xApiInit(0, NULL, 0, &streamFormat);
#endif

	if (ret != TCC353X_RETURN_SUCCESS) {
		TcpalPrintLog((I08S *) "[TCC353X] Init Fail!!!\n");
		tcc353x_deinit(commandInterface, streamInterface);
		return -1;
	} else {
		TcpalPrintLog((I08S *) "[TCC353X] Init Success!!!\n");
	}

	return 0;
}

/**************************************************************************
*  FUNCTION NAME : 
*    I32S tcc353x_deinit(I32S , I32S )
*  
*  DESCRIPTION : Deinit Tcc353x
*  OUTPUT:	
**************************************************************************/
I32S tcc353x_deinit(I32S commandInterface, I32S streamInterface)
{
	I32U deviceIdx;

	/* close TCC353X driver */
	Tcc353xApiClose(0);

	ALOGD("power off tcc353x!!");
	switch(Tcc353xDxbControlType) {
	case 0 :
		/* Master control #0 pin, SlaveN control #1 pin */
		deviceIdx = 0;
		ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
		if(CurrentNumOfDiversity>1) {
			deviceIdx = 1;
			ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
		}
	break;
	case 1 :
		/* All baseband control #0 pin	*/
		deviceIdx = 0;
		ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
	break;
	case 2 : 
		/* Master, Slave1 control #0 pin, Slave2~3 control  #1 pin */
		deviceIdx = 0;
		ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
		if(CurrentNumOfDiversity>2) {
			deviceIdx = 1;
			ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
		}
	break;
	default :
		/* Master control #0 pin, SlaveN control #1 pin */
		deviceIdx = 0;
		ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
		deviceIdx = 1;
		ioctl(g_tcc_dxb_ctrl_fd, IOCTL_DXB_CTRL_OFF, &deviceIdx);
	break;
	}
	close(g_tcc_dxb_ctrl_fd);

	/* close I2C/TCCSPI Driver */
	if (commandInterface == TCC353X_IF_TCCSPI)
		Tcc353xTccspiClose(0);
	else if (commandInterface == TCC353X_IF_I2C)
		Tcc353xI2cClose(0);
	else
		Tcc353xTccspiClose(0);
	return 0;
}

/**************************************************************************
*  FUNCTION NAME : 
*    I32S tcc353x_search_channel(I32S,I32S,I32S)
*  
*  DESCRIPTION : None
*  OUTPUT:	
**************************************************************************/
I32S tcc353x_search_channel(I32S searchmode, I32S channel,
			    I32S countrycode)
{
	return 0;
}

/**************************************************************************
*  FUNCTION NAME : 
*    I32S tcc353x_channel_tune(I32S)
*  
*  DESCRIPTION : Channel Tune (not check lock status)
*  OUTPUT:	
**************************************************************************/
I32S tcc353x_channel_tune(I32S channel)
{
	I32S freq;
	I32S bandwidth;
	/* I32S finetune; */
	I32S ret = TCC353X_RETURN_FAIL;
	Tcc353xTuneOptions tuneOption;
	I32S i;

	freq = GetFrequency(channel);
	bandwidth = GetBandwidth(channel);
	/* finetune=GetFinetune(channel); */

	if (0 < freq) {
		FrequencyInfo[0] = freq;
		TcpalMemset(&tuneOption, 0x00, sizeof(tuneOption));
#if defined (_USE_ONLY_1_SEGMENT_)
		tuneOption.rfIfType = TCC353X_LOW_IF;
		tuneOption.segmentType = TCC353X_ISDBT_1_OF_13SEG;
#else
		tuneOption.rfIfType = TCC353X_ZERO_IF;
		tuneOption.segmentType = TCC353X_ISDBT_13SEG;
#endif	

		if(bandwidth==8000)
			tuneOption.BandwidthMHz = 8;

		ret = Tcc353xApiChannelSelect(0, freq, &tuneOption);

		for(i=0; i<CurrentNumOfDiversity; i++)
			Tcc353xMonitoringApiInit(0, i);
	}

	ALOGD("channel set [%d]\n", ret);

	if (ret == TCC353X_RETURN_SUCCESS)
		return freq;

	return -1;
}

/**************************************************************************
*  FUNCTION NAME : 
*    I32S tcc353x_channel_set(I32S)
*  
*  DESCRIPTION : Channel tune (check lock status)
*  OUTPUT:	
**************************************************************************/
I32S tcc353x_channel_set(I32S channel)
{
	I32S freq;
	I32S bandwidth;
	/* I32S finetune; */
	I32S ret = TCC353X_RETURN_FAIL;
	Tcc353xTuneOptions tuneOption;
	I32S i;

	freq = GetFrequency(channel);
	bandwidth = GetBandwidth(channel);
	/* finetune=GetFinetune(channel); */

	if (0 < freq) {
		FrequencyInfo[0] = freq;
		TcpalMemset(&tuneOption, 0x00, sizeof(tuneOption));

#if defined (_USE_ONLY_1_SEGMENT_)
		tuneOption.rfIfType = TCC353X_LOW_IF;
		tuneOption.segmentType = TCC353X_ISDBT_1_OF_13SEG;
#else
		tuneOption.rfIfType = TCC353X_ZERO_IF;
		tuneOption.segmentType = TCC353X_ISDBT_13SEG;
#endif
		if(bandwidth==8000)
			tuneOption.BandwidthMHz = 8;

		ret = Tcc353xApiChannelSearch(0, freq, &tuneOption);
		for(i=0; i<CurrentNumOfDiversity; i++)
			Tcc353xMonitoringApiInit(0, i);

		ALOGD("channel set [%d]\n", ret);
	}

	if (ret == TCC353X_RETURN_SUCCESS)
		return freq;

	return -1;
}

/**************************************************************************
*  FUNCTION NAME : 
*    I32S tcc353x_frequency_set(I32S, I32S)
*  
*  DESCRIPTION : Channel Tune (check lock status) with frequency information
*  OUTPUT:	
**************************************************************************/
I32S tcc353x_frequency_set(I32S iFreqKhz, I32S iBWKhz)
{
	I32S ret = TCC353X_RETURN_FAIL;
	Tcc353xTuneOptions tuneOption;
	I32S i;

	if (0 < iFreqKhz) {
		FrequencyInfo[0] = iFreqKhz;
		TcpalMemset(&tuneOption, 0x00, sizeof(tuneOption));

#if defined (_USE_ONLY_1_SEGMENT_)
		tuneOption.rfIfType = TCC353X_LOW_IF;
		tuneOption.segmentType = TCC353X_ISDBT_1_OF_13SEG;
#else
		tuneOption.rfIfType = TCC353X_ZERO_IF;
		tuneOption.segmentType = TCC353X_ISDBT_13SEG;
#endif

		if(iBWKhz==8000)
			tuneOption.BandwidthMHz = 8;

		ret = Tcc353xApiChannelSearch(0, iFreqKhz, &tuneOption);
		for(i=0; i<CurrentNumOfDiversity; i++)
			Tcc353xMonitoringApiInit(0, i);
	}

	ALOGD("frequency set [%d]\n", ret);

	if (ret == TCC353X_RETURN_SUCCESS)
		return TRUE;

	return FALSE;
}

int tcc353x_frequency_get (I32S channel, I32S *pFreq)
{
	I32S	freq;

	freq = GetFrequency(channel);
	if (freq > 0) {
		*pFreq = freq;
		return 0;
	}
	else {
		*pFreq = 0;
		return -1;
	}
}

/**************************************************************************
*  FUNCTION NAME : 
*    I32S tcc353x_get_signal_strength(Tcc353xStatus_t *)
*  
*  DESCRIPTION : Get signal strength
*  OUTPUT:	
**************************************************************************/
I32S tcc353x_get_signal_strength(Tcc353xStatus_t * _monitorValues)
{
	Tcc353xStatus_t monitor[4];
	I32U structSize = 0;
	I32S ret;
	I32S i;
#if defined (DBG_MONITOR)
	/* more debugging */
	mailbox_t mailbox;
	I64S samplingfreqerr = 0;
	double dsample = 0;
	I64U L,H,temp, temp2;
#endif
	structSize = sizeof(Tcc353xStatus_t) * CurrentNumOfDiversity;

	for(i=0; i<CurrentNumOfDiversity; i++) {
		ret = Tcc353xMonitoringApiGetStatus(0, i, &monitor[i]);
		if(ret != TCC353X_RETURN_SUCCESS) {
			return -1;
		}
	}
	
	Tcc353xMonitoringApiAntennaPercentage (0, &monitor[0], structSize);
	memcpy(_monitorValues, &monitor[0], sizeof(Tcc353xStatus_t));
	Tcc353xMonitoringDiversityProc (0, &monitor[0], structSize);

#if defined USE_MONITORING_LOG
	for(i=0; i<CurrentNumOfDiversity; i++) {
		TcpalPrintStatus((I08S *)"[BB#%d] Lock [%1d%1d%1d%1d] RSSI[%2d] RF[%3d] BB[%3d]\n",
				i,
				monitor[i].status.isdbLock.AGC,
				monitor[i].status.isdbLock.CTO, 
				monitor[i].status.isdbLock.CFO, 
				monitor[i].status.isdbLock.TMCC,
				monitor[i].status.rssi.currentValue,
				monitor[i].rfLoopGain,
				monitor[i].bbLoopGain);
	}

	TcpalPrintStatus((I08S *)"\tA VBER[%3d,%3d] TSPER[%5d,%5d] MER[%2d,%2d] PCBER[%3d,%3d] Antenna[%3d]\n",
		monitor[0].status.viterbiber[0].currentValue, 
		monitor[0].status.viterbiber[0].avgValue, 
		monitor[0].status.tsper[0].currentValue, 
		monitor[0].status.tsper[0].avgValue,
		monitor[0].status.mer[0].currentValue, 
		monitor[0].status.mer[0].avgValue, 
		monitor[0].status.pcber[0].currentValue, 
		monitor[0].status.pcber[0].avgValue,
		monitor[0].antennaPercent[0]);
	TcpalPrintStatus((I08S *)"\tB VBER[%3d,%3d] TSPER[%5d,%5d] MER[%2d,%2d] PCBER[%3d,%3d] Antenna[%3d]\n",
		monitor[0].status.viterbiber[1].currentValue, 
		monitor[0].status.viterbiber[1].avgValue, 
		monitor[0].status.tsper[1].currentValue, 
		monitor[0].status.tsper[1].avgValue,
		monitor[0].status.mer[1].currentValue, 
		monitor[0].status.mer[1].avgValue, 
		monitor[0].status.pcber[1].currentValue, 
		monitor[0].status.pcber[1].avgValue,
		monitor[0].antennaPercent[1]);
	TcpalPrintStatus((I08S *)"\tC VBER[%3d,%3d] TSPER[%5d,%5d] MER[%2d,%2d] PCBER[%3d,%3d] Antenna[%3d]\n\n",
		monitor[0].status.viterbiber[2].currentValue, 
		monitor[0].status.viterbiber[2].avgValue, 
		monitor[0].status.tsper[2].currentValue, 
		monitor[0].status.tsper[2].avgValue,
		monitor[0].status.mer[2].currentValue, 
		monitor[0].status.mer[2].avgValue, 
		monitor[0].status.pcber[2].currentValue, 
		monitor[0].status.pcber[2].avgValue,
		monitor[0].antennaPercent[2]);
#endif

#if defined (DBG_MONITOR)
	/* more debugging */

	char stringsmax0[1024] = {0,};
	char stringsmax1[1024] = {0,};
	memset (&stringsmax0, 0, sizeof (stringsmax1));
	memset (&stringsmax1, 0, sizeof (stringsmax1));

	/* master */
	sprintf(stringsmax0, "[DBGM0]");
	sprintf(stringsmax0+strlen(stringsmax0),"SyncStat,A_VBER,A_TSPER,A_MER,A_PCBER,B_VBER,B_TSPER,B_MER,B_PCBER,");
	sprintf(stringsmax0+strlen(stringsmax0),"CTO_R_CNT,CFO_R_CNT,FR_SYNC_R_CNT,TMCC_R_CNT,");
	sprintf(stringsmax0+strlen(stringsmax0),"TS_A_R_CNT,TS_B_R_CNT,RSSI,RF,BB,GI_SIZE,FFT_SIZE,CFO,FFO,T_BND_MASK,SFE,");
	sprintf(stringsmax0+strlen(stringsmax0),"REG_WEIGHT,MAX_ENERGY,MAX_ID,N_DLY,P_DLY,MAX_DLY,N_OFFSET,FTO_OFFSET,FTO_OVER_CNT,");
	sprintf(stringsmax0+strlen(stringsmax0),"DFT_SRT_ON,TI_FIL_INDEX,TI_CHE_SEL,DPE_IDX_L,DPE_IDX_R,DPE_IDX_SUM,");
	sprintf(stringsmax0+strlen(stringsmax0),"DPE_FD_MAX,DOP_FREQ,DOP_EST_FD_SEL,TI_SEL_ACC,TI_SEL_DEC,TI_SEL,");
	sprintf(stringsmax0+strlen(stringsmax0),"SNR_MER,MASTER_SNR,TMCC_0,TMCC_1,TMCC_2,TMCC_3,");
	sprintf(stringsmax0+strlen(stringsmax0),"L1_DIV_COMB_PWR,L2_DIV_COMB_PWR,NOF_DIV_OP,NOF_DIV_COMB_OP\n");
	TcpalPrintStatus((I08S *)"%s",stringsmax0);

	for(i=0; i<CurrentNumOfDiversity; i++) {
		memset (&stringsmax1, 0, sizeof (stringsmax1));

		sprintf(stringsmax1, "[DBGM1_%d]", i);
		Tcc353xApiMailboxRead(0, i, ((0<<11) | 0x01), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"0x%x,",mailbox.data_array[0]);

		sprintf(stringsmax1+strlen(stringsmax1),"%d,%d,%d,%d,",
			monitor[0].status.viterbiber[0].currentValue, 
			monitor[0].status.tsper[0].currentValue, 
			monitor[0].status.mer[0].currentValue, 
			monitor[0].status.pcber[0].currentValue);

		sprintf(stringsmax1+strlen(stringsmax1),"%d,%d,%d,%d,",
			monitor[0].status.viterbiber[1].currentValue, 
			monitor[0].status.tsper[1].currentValue, 
			monitor[0].status.mer[1].currentValue, 
			monitor[0].status.pcber[1].currentValue);

		Tcc353xApiMailboxRead(0, i, ((0<<11) | 0x03), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"%d,%d,%d,%d,%d,%d,",
			(mailbox.data_array[0]&0xFFFF),
			((mailbox.data_array[0]>>16)&0xFFFF),
			(mailbox.data_array[1]&0xFFFF),
			((mailbox.data_array[1]>>16)&0xFFFF),
			((mailbox.data_array[2]>>16)&0xFFFF),
			(mailbox.data_array[3]&0xFFFF));

		sprintf(stringsmax1+strlen(stringsmax1),"%d,%d,%d,",
			monitor[0].status.rssi.currentValue,
			monitor[0].rfLoopGain,
			monitor[0].bbLoopGain);

		Tcc353xApiMailboxRead(0, i, ((0x06<<11) | 0x01), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"%d,%d,",
			(mailbox.data_array[0]&0xFFFF),
			((mailbox.data_array[0]>>16)&0xFFFF));

		Tcc353xApiMailboxRead(0, i, ((0x08<<11) | 0x01), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"%d,",
			mailbox.data_array[1]);

		Tcc353xApiMailboxRead(0, i, ((0x07<<11) | 0x01), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"%d,",
			mailbox.data_array[2]);

		Tcc353xApiMailboxRead(0, i, ((0x09<<11) | 0x01), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"%d,",
			mailbox.data_array[0]);

		Tcc353xApiMailboxRead(0, i, ((0x0A<<11) | 0x01), &mailbox);

		H = (I64U)(mailbox.data_array[1]&0xFF);
		L = (I64U)(mailbox.data_array[2]);
		temp = (H<<32);
		if(H&0x80)
			temp2 = (temp|0xFFFFFF0000000000);
		else
			temp2 = temp;
		samplingfreqerr = (I64S)(L+temp2);

		dsample = ((double)(samplingfreqerr)/1000000.0);
		sprintf(stringsmax1+strlen(stringsmax1),"%lf,",dsample);

		Tcc353xApiMailboxRead(0, i, ((0x0B<<11) | 0x01), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"0x%x,%d,%d,%d,%d,%d,%d,%d,%d,",
			mailbox.data_array[0],
			mailbox.data_array[1],
			((mailbox.data_array[2]>>16)&0xFFFF),
			(mailbox.data_array[2]&0xFFFF),
			((mailbox.data_array[3]>>16)&0xFFFF),
			(mailbox.data_array[3]&0xFFFF),
			((mailbox.data_array[4]>>16)&0xFFFF),
			(mailbox.data_array[4]&0xFFFF),
			(mailbox.data_array[5]&0xFFFF));

		Tcc353xApiMailboxRead(0, i, ((0x0C<<11) | 0x01), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"0x%x,",
			mailbox.data_array[0]);

		Tcc353xApiMailboxRead(0, i, ((0x0D<<11) | 0x00), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"%d,%d,",
			((mailbox.data_array[0]>>16)&0xFFFF),
			(mailbox.data_array[0]&0xFFFF));

		Tcc353xApiMailboxRead(0, i, ((0x0F<<11) | 0x00), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"%d,%d,%d,%d,%d,%d,",
			(mailbox.data_array[0]&0xFFFF),
			((mailbox.data_array[0]>>16)&0xFFFF),
			(mailbox.data_array[1]&0xFFFF),
			((mailbox.data_array[1]>>16)&0xFFFF),
			(mailbox.data_array[2]&0xFFFF),
			((mailbox.data_array[2]>>16)&0xFFFF));


		Tcc353xApiMailboxRead(0, i, ((0x10<<11) | 0x01), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"%d,%d,%d,",
			((mailbox.data_array[1]>>16)&0xFFFF),
			((mailbox.data_array[2]>>16)&0xFFFF),
			(mailbox.data_array[2]&0xFFFF));

		Tcc353xApiMailboxRead(0, i, ((0x11<<11) | 0x00), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"%d,%d,",
			monitor[0].status.snr.currentValue,
			mailbox.data_array[1]);

		Tcc353xApiMailboxRead(0, i, ((0x05<<11) | 0x00), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"0x%08x,0x%08x,0x%08x,0x%08x,",
			mailbox.data_array[0],
			mailbox.data_array[1],
			mailbox.data_array[2],
			mailbox.data_array[3]);

		Tcc353xApiMailboxRead(0, i, ((0x03<<11) | 0x00), &mailbox);
		sprintf(stringsmax1+strlen(stringsmax1),"%d,%d,%d,%d\n",
			mailbox.data_array[0],
			mailbox.data_array[1],
			(mailbox.data_array[3]&0xFFFF),
			((mailbox.data_array[3]>>16)&0xFFFF));

		TcpalPrintStatus((I08S *)"%s",stringsmax1);
	}

	if(CurrentNumOfDiversity>1) {
		for(i=0; i<CurrentNumOfDiversity; i++) {
			I32U EST_DOP_FREQ;
			I32U DIV_NOF_OP;
			I32U DIV_NOF_COMP_OP;
			I32U RESYNC_RESULT[7];

			TcpalPrintStatus((I08S *)"[DBGMON_] [%d]th Baseband\n",i);

			ret = Tcc353xApiMailboxRead(0, i, ((0x0F << 11) | 0x00), &mailbox);
			if(ret != TCC353X_RETURN_SUCCESS)
				return TCC353X_RETURN_FAIL;
			EST_DOP_FREQ = (mailbox.data_array[2]&0xFFFF);
			TcpalPrintStatus((I08S *)"[DBGMON0] EST_DOP_FREQ[%d]\n",EST_DOP_FREQ);

			ret = Tcc353xApiMailboxRead(0, i, ((0x03 << 11) | 0x00), &mailbox);
			if(ret != TCC353X_RETURN_SUCCESS)
				return TCC353X_RETURN_FAIL;
			DIV_NOF_OP = (mailbox.data_array[3]&0xFFFF);
			DIV_NOF_COMP_OP = ((mailbox.data_array[3]>>16)&0xFFFF);
			TcpalPrintStatus((I08S *)"[DBGMON1] DIV_NOF_OP[%d] DIV_NOF_COMP_OP\n",DIV_NOF_OP, DIV_NOF_COMP_OP);
			
			
			ret = Tcc353xApiMailboxRead(0, i,((0x00 << 11) | 0x03), &mailbox);
			if(ret != TCC353X_RETURN_SUCCESS)
				return TCC353X_RETURN_FAIL;
			TcpalMemcpy(&RESYNC_RESULT[0],
				    &mailbox.data_array[0], sizeof(I32U)*7);
			TcpalPrintStatus((I08S *)"[DBGMON2] RESYNC_RESULT_0[0x%x]\n",RESYNC_RESULT[0]);
			TcpalPrintStatus((I08S *)"[DBGMON3] RESYNC_RESULT_1[0x%x]\n",RESYNC_RESULT[1]);
			TcpalPrintStatus((I08S *)"[DBGMON4] RESYNC_RESULT_2[0x%x]\n",RESYNC_RESULT[2]);
			TcpalPrintStatus((I08S *)"[DBGMON5] RESYNC_RESULT_3[0x%x]\n",RESYNC_RESULT[3]);
			TcpalPrintStatus((I08S *)"[DBGMON6] RESYNC_RESULT_4[0x%x]\n",RESYNC_RESULT[4]);
			TcpalPrintStatus((I08S *)"[DBGMON7] RESYNC_RESULT_5[0x%x]\n",RESYNC_RESULT[5]);
			TcpalPrintStatus((I08S *)"[DBGMON8] RESYNC_RESULT_6[0x%x]\n",RESYNC_RESULT[6]);
		}
	}
#endif
	return 0;
}

/**************************************************************************
*  FUNCTION NAME : 
*    I32S tcc353x_get_ews_flag ()
*  
*  DESCRIPTION : Get ews flag
*  OUTPUT:	
**************************************************************************/
I32S tcc353x_get_ews_flag ()
{
	I32S ret;
	ret = Tcc353xApiGetEwsFlag(0, 0);
	if (ret < 0 )
		return 0;
	return ret;
}

I32S tcc353x_get_tmcc_information(tmccInfo_t *pTmccinfo)
{
	return (Tcc353xApiGetTMCCInfo(0, 0,pTmccinfo));
}
