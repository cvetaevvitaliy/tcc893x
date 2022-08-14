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
#define LOG_TAG	"MN88472_DVBT"
#include <utils/Log.h>

#include <fcntl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/ioctl.h>
#include "MN88472_dvbt.h"
#include "tcc_dxb_control.h"
#include "MN_DMD_driver.h"
#include "MN_DMD_common.h"
#include "MN88472_UserFunction.h"

static unsigned int guiNumberOfBB = 1;
static int giPrevFreqKhz = 0, giPrevBWKhz = 0, giPrevSystem = 0;
#define DXB_CTRL_DEV_FILE		"/dev/tcc_dxb_ctrl"
static int giTccDXBCtrlFD = -1, giActiveFreq = -1;
static DMD_PARAMETER_t	gParam;	
static int giCtrlMode = 2; // 0: off, 1: on, 2: auto
static int Power_On(void)
{
    unsigned int deviceIdx, iBoardType;
    giTccDXBCtrlFD = open(DXB_CTRL_DEV_FILE, O_RDWR | O_NDELAY);
	if(giTccDXBCtrlFD<0)
	{
		ALOGD ("%s CANNOT open %s :: %d\n", __func__, DXB_CTRL_DEV_FILE, giTccDXBCtrlFD);
		return 1;
	}
    deviceIdx = 0;
	ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_SET_CTRLMODE, &giCtrlMode);
	ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_ON, &deviceIdx);
	iBoardType = BOARD_DVBT2_MN88472_YJ;
    ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_SET_BOARD, &iBoardType);
    return 0;
}

static int Power_Off(void)
{
    unsigned int deviceIdx;
    deviceIdx = 0;
    ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_OFF, &deviceIdx);

    close(giTccDXBCtrlFD);	
	giTccDXBCtrlFD = -1;	
    return 0;
}

int SetAntennaPower(int arg)
{
	if (arg == giCtrlMode)
		return 0;

	giCtrlMode = arg;
	if (giTccDXBCtrlFD >= 0)
	{
		Power_Off();
		Power_On();
	}
	return 0;
}

int MN88472DVBT_GetStatus(DVBT_TUNE_STATUS *pStatus)
{
    unsigned int	i;
    char	buf[10];
    DMD_get_info( &gParam, DMD_E_INFO_ALL );
#if 0
    for(i=0;i<gParam.info[DMD_E_INFO_ALL];i++)
    {
        ALOGD("%25s:%15s:%d" , DMD_info_title( gParam.system , i ),  DMD_value_text( &gParam, i ), gParam.info[i] );
    }
#endif
    pStatus->uiFrequency = gParam.freq;
    switch(gParam.bw)
    {
        case DMD_E_BW_6MHZ:
            pStatus->uiBandwidth = 6000;
            break;
        case DMD_E_BW_7MHZ:
            pStatus->uiBandwidth = 7000;
            break;
        case DMD_E_BW_8MHZ:
        default:
            pStatus->uiBandwidth = 8000;
            break;
    }
    if(gParam.system == DMD_E_DVBT)
    {
        pStatus->uiDVBSystem = 0; //DVBT
        pStatus->uiFECLength = -1;
        pStatus->uiRotation = -1;
        pStatus->uiPLPNum = -1;
        pStatus->uiPilotPP = -1;
        pStatus->uiConstellation = gParam.info[DMD_E_INFO_DVBT_CONSTELLATION];
        pStatus->uiGuardInterval = gParam.info[DMD_E_INFO_DVBT_GI];
        pStatus->uiCodeRate = gParam.info[DMD_E_INFO_DVBT_HP_CODERATE];
        pStatus->uiFFT = gParam.info[DMD_E_INFO_DVBT_MODE];
    }
    else
    {
        pStatus->uiDVBSystem = 1; //DVBT2
        pStatus->uiRotation = gParam.info[DMD_E_INFO_DVBT2_DAT_PLP_ROTATION];
        pStatus->uiConstellation = gParam.info[DMD_E_INFO_DVBT2_DAT_PLP_MOD];
        pStatus->uiFECLength = gParam.info[DMD_E_INFO_DVBT2_DAT_PLP_FEC_TYPE];
        pStatus->uiGuardInterval = gParam.info[DMD_E_INFO_DVBT2_GI];
        pStatus->uiCodeRate = gParam.info[DMD_E_INFO_DVBT2_DAT_PLP_COD];
        pStatus->uiFFT = gParam.info[DMD_E_INFO_DVBT2_MODE];
        pStatus->uiPLPNum = gParam.info[DMD_E_INFO_DVBT2_NUM_PLP];
        pStatus->uiPilotPP = gParam.info[DMD_E_INFO_DVBT2_PILOT_PATTERN];
    }
    return 0;
}

void MM88472DVBT_DisplayTuneStatus(void)
{
    const char *pstDVBSystem[] = {"DVBT",	"DVBT2"};

    //For DVBT2
    const char *pstConstellation_DVBT2[] = {"QPSK",	"16QAM", "64QAM", "256QAM"};
    const char *pstFECLength_DVBT2[] = {"16k LDPC", "64k LDPC"};
    const char *pstGuardInterval_DVBT2[] = {"1/32","1/16","1/8","1/4","1/128","19/128","19/256"};
    const char *pstCodeRate_DVBT2[] = {"1/2","3/5",	"2/3",	"3/4","4/5","5/6"};
    const char *pstFFT_DVBT2[] = {"1k","2k","4k","8k","16k","32k"};
    const char *pstPilotPP_DVBT2[] = {"PP1","PP2","PP3","PP4","PP5","PP6","PP7","PP8"};

    //For DVBT
    const char *pstConstellation_DVBT[] = { "QPSK","16QAM","64QAM"};
    const char *pstGuardInterval_DVBT[] = {"1/32","1/16","1/8","1/4"};
    const char *pstCodeRate_DVBT[] = {"1/2","2/3","3/4","5/6","7/8"};
    const char *pstFFT_DVBT[] = {"2k","8k","4k"};

    DVBT_TUNE_STATUS TuneStatus;
    MN88472DVBT_GetStatus(&TuneStatus);

    ALOGD("[TUNE_STATUS]Frequency :%dKhz, BandWidth:%dKhz @ %s", TuneStatus.uiFrequency, TuneStatus.uiBandwidth, pstDVBSystem[TuneStatus.uiDVBSystem]);
    if(TuneStatus.uiDVBSystem == 0)
    {
        //DVBT
        ALOGD("[TUNE_STATUS]uiConstellation = %s", 	pstConstellation_DVBT[TuneStatus.uiConstellation]);
        ALOGD("[TUNE_STATUS]uiGuardInterval = %s", 	pstGuardInterval_DVBT[TuneStatus.uiGuardInterval]);
        ALOGD("[TUNE_STATUS]uiCodeRate = %s", 		pstCodeRate_DVBT[TuneStatus.uiCodeRate]);
        ALOGD("[TUNE_STATUS]uiFFT = %s", 			pstFFT_DVBT[TuneStatus.uiFFT]);
    }
    else
    {
        //DVBT2
	ALOGD("[TUNE_STATUS]uiRotation = %d [1:Yes, 0:No]", 	TuneStatus.uiRotation);
        ALOGD("[TUNE_STATUS]uiConstellation = %s", 	pstConstellation_DVBT2[TuneStatus.uiConstellation]);
        ALOGD("[TUNE_STATUS]uiFECLength = %s", 		pstFECLength_DVBT2[TuneStatus.uiFECLength]);
        ALOGD("[TUNE_STATUS]uiGuardInterval = %s", 	pstGuardInterval_DVBT2[TuneStatus.uiGuardInterval]);
        ALOGD("[TUNE_STATUS]uiCodeRate = %s", 		pstCodeRate_DVBT2[TuneStatus.uiCodeRate]);
        ALOGD("[TUNE_STATUS]uiFFT = %s", 			pstFFT_DVBT2[TuneStatus.uiFFT]);
        ALOGD("[TUNE_STATUS]uiPLPNum = %d", 		TuneStatus.uiPLPNum);
        ALOGD("[TUNE_STATUS]uiPilotPP = %s", 		pstPilotPP_DVBT2[TuneStatus.uiPilotPP]);
    }
}

int MN88472DVBT_Open(int iCountryCode, int iSystem)
{
    giPrevFreqKhz = 0, giPrevBWKhz = 0, giPrevSystem = 0;
    Power_On();  
    DMD_open( &gParam );
    DMD_init( &gParam );

    if(iSystem == 0)
        gParam.system = DMD_E_DVBT;
    else	    
        gParam.system = DMD_E_DVBT2;
    gParam.bw = DMD_E_BW_8MHZ;
    DMD_set_system( &gParam );
    return 0;
}

int MN88472DVBT_Close(void)
{ 
    Power_Off();
	return 0;
}

int MN88472DVBT_ChannelSet(int iChannel, int bLockOn)
{
	return 0;
}

int MN88472DVBT_FrequencySet(int iFreqKhz, int iBWKhz, int iSystem, int bLockOn)
{	    
	int old_bw = gParam.bw;
    ALOGD("%s:[freq:%dkhz][bw:%dkhz][sys:%d][lock:%d]",__func__, iFreqKhz, iBWKhz, iSystem, bLockOn);

    if(giPrevFreqKhz == iFreqKhz && giPrevBWKhz == iBWKhz && giPrevSystem == iSystem)    
    {
        ALOGD("%s: It is same frequency",__func__);
        if (bLockOn)
        {
	        if( DMD_scan( &gParam ) != DMD_E_OK )
	        {
	       	    return 1;
	        }
       	    ALOGD("%s: Found!!![%dKhz][%dKhz]",__func__, iFreqKhz, iBWKhz);
        }
        return 0;
    }

    gParam.freq = iFreqKhz;
	gParam.funit = DMD_E_KHZ;
	switch(iBWKhz)
    {
    case 6000:
        gParam.bw = DMD_E_BW_6MHZ;
        break;
    case 7000:
        gParam.bw = DMD_E_BW_7MHZ;
        break;
    case 8000:
        gParam.bw = DMD_E_BW_8MHZ;
        break;
    default:
        return 1;
    }
    
    //iSystem : 0:DVBT, 1:DVBT2
    if(iSystem == 0)
    {
        // DVBT
        if(gParam.system == DMD_E_DVBT2 || old_bw != gParam.bw)
        {
           	gParam.system = DMD_E_DVBT;
           	DMD_set_system( &gParam );
        }
    }
    else
    {
        //DVBT2
        if(gParam.system == DMD_E_DVBT || old_bw != gParam.bw)
        {
           	gParam.system = DMD_E_DVBT2;
           	DMD_set_system( &gParam );
        }    
    }

    
    if(bLockOn)
    {
        if( DMD_scan( &gParam ) == DMD_E_OK )
        {
            giPrevFreqKhz = iFreqKhz;
            giPrevBWKhz = iBWKhz;
            giPrevSystem = iSystem;
       	    ALOGD("%s: Found!!![%dKhz][%dKhz]",__func__, iFreqKhz, iBWKhz);
       	    return 0;
        }
        giPrevFreqKhz = 0;
        giPrevBWKhz = 0;
        giPrevSystem = 0;
        return 1;
    }
    else
    {
        giPrevFreqKhz = iFreqKhz;
        giPrevBWKhz = iBWKhz;
        giPrevSystem = iSystem;
        DMD_tune(  &gParam );	
        return 0;
    }
}

int MN88472DVBT_GetSignalStrength(void)
{
    int snr_i, retLevel = 4, cnr_int, cnr_dec;
    DMD_get_info( &gParam , DMD_E_INFO_CNR_INT );	
    cnr_int = gParam.info[DMD_E_INFO_CNR_INT];
    //ALOGD("%s :CNR_INT %d",__func__, cnr_int  );     
    
    DMD_get_info( &gParam , DMD_E_INFO_CNR_DEC );	
    cnr_dec = gParam.info[DMD_E_INFO_CNR_DEC];
    //ALOGD("%s :CNR_DEC %d",__func__, cnr_dec  ); 

    //MM88472DVBT_DisplayTuneStatus();
    if (cnr_int > 20)       cnr_int = 20;
    retLevel = cnr_int * 5;
    return retLevel;
}

int MN88472DVBT_GetSignalQuality(void)
{
    return 100 - MN88472DVBT_GetBER();
}

int MN88472DVBT_GetSNR(void)
{
	int iSNR = 0;

    DMD_get_info( &gParam , DMD_E_INFO_CNR_INT );
    iSNR = gParam.info[DMD_E_INFO_CNR_INT] * 4;

	if (iSNR > 100) iSNR = 100;

	return iSNR;
}

int MN88472DVBT_GetBER(void)
{
    int iBER = 0;

    DMD_get_info( &gParam , DMD_E_INFO_BERRNUM );
    if(gParam.info[DMD_E_INFO_BITNUM] == 0 )
    {
        iBER = 100;
    }
    else
    {
        iBER = (int)(2500 * gParam.info[DMD_E_INFO_BERRNUM] / gParam.info[DMD_E_INFO_BITNUM]);
    }

	if (iBER > 100) iBER = 100;

	return iBER;
}

int MN88472DVBT_GetDataPLPs(int *piPLPIds, int *piPLPNum)
{
    if(gParam.system == DMD_E_DVBT2)
    {
        //Check MPLP information
        int i;
        unsigned char	pPLPIds[256];
        unsigned char	pNumPLPs;

        if( DMD_get_dataPLPs( pPLPIds, &pNumPLPs , &gParam ) != DMD_E_OK )
        {
            *piPLPNum = 0;
            return 1;
        }

        for(i=0;i<pNumPLPs;i++)
        {
             ALOGD("%s : No.%d, ID:%04x ",__func__, i, pPLPIds[i]);
             piPLPIds[i] = pPLPIds[i];
        } 
        *piPLPNum = pNumPLPs;
    }
    else
        *piPLPNum = 0;
    return 0;
}

int MN88472DVBT_SetDataPLP(int iPLPId)
{
    if(gParam.system == DMD_E_DVBT2)
    {
        ALOGD("%s : ID:%04x ",__func__, iPLPId);
        if( DMD_set_info( &gParam , DMD_E_INFO_DVBT2_SELECTED_PLP , iPLPId ) != DMD_E_OK)
            return 1;
    }
    return 0;
}
