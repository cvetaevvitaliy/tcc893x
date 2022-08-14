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
#define LOG_TAG	"MXL101SF_DVBT"
#include <utils/Log.h>

#include <fcntl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>
#include <sys/ioctl.h>
#include "MxL101SF_dvbt.h"
#include "MxL101SF_PhyCtrlApi.h"
#include "MxL101SF_OEM_Drv.h"
#include "tcc_dxb_control.h"

static unsigned int guiNumberOfBB = 1;

#define DXB_CTRL_DEV_FILE		"/dev/tcc_dxb_ctrl"
static int giTccDXBCtrlFD = -1, giActiveFreq = -1;
static UINT8 mxlI2cSlvAddr = 0x60;
static int giCtrlMode = 2; // 0: off, 1: on, 2: auto

int Power_On(void)
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
	iBoardType = BOARD_DVBT_MXL101SF_YJ;
    ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_SET_BOARD, &iBoardType);
    return 0;
}

int Power_Off(void)
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

int MxL101SFDVBT_Open(int iCountryCode, int iCmdInterface, int iDataInterface, int iNumberOfBB)
{ 
    MXL_RESET_CFG_T mxlSoftReset;
    MXL_OVERWRITE_DEFAULT_CFG_T mxlInitSoc;
    MXL_DEV_INFO_T mxlDevInfo;
    MXL_XTAL_CFG_T mxlXtalCfg;
    MXL_DEV_MODE_CFG_T mxlDevMode;
    MXL_MPEG_CFG_T mxlMpegOutCfg;
    MXL_TS_CTRL_T mxlTsConfig;
    MXL_TOP_MASTER_CFG_T mxlTopMasterCfg;
    MXL_DEMOD_SPECTRUM_CFG_T mxlFreqSpectrumCfg;
    Power_On();
    // Open LPT driver for I2C communcation
    Ctrl_I2cConnect(mxlI2cSlvAddr);

    // 1. Do SW Reset
    mxlSoftReset.I2cSlaveAddr = mxlI2cSlvAddr;
    MxLWare_API_ConfigDevice(MXL_DEV_SOFT_RESET_CFG, &mxlSoftReset);

    // 2. Read Back chip id and version
    //    Expecting CHIP ID = 0x61, Version = 0x6
    mxlDevInfo.I2cSlaveAddr = mxlI2cSlvAddr;
    MxLWare_API_GetDeviceStatus(MXL_DEV_ID_VERSION_REQ, &mxlDevInfo);
    ALOGD("MxL101SF : ChipId = 0x%x, Version = 0x%x\n", mxlDevInfo.DevId, 
                   mxlDevInfo.DevVer);

    // 3. Init Tuner and Demod
    mxlInitSoc.I2cSlaveAddr = mxlI2cSlvAddr;
    MxLWare_API_ConfigDevice(MXL_DEV_101SF_OVERWRITE_DEFAULTS_CFG, &mxlInitSoc);

    // Step 4
    // Enable Loop Through if needed
    // Enable Clock Out and configure Clock Out gain if needed
    // Configure MxL101SF XTAL frequency
    // Configure XTAL Bias value if needed

    // Xtal Capacitance value must be configured in accordance 
    // with XTAL datasheetë­?requirement.
    mxlXtalCfg.I2cSlaveAddr = mxlI2cSlvAddr;
    mxlXtalCfg.XtalFreq = XTAL_24MHz;
    mxlXtalCfg.LoopThruEnable = MXL_ENABLE;
    mxlXtalCfg.XtalBiasCurrent = XTAL_BIAS_NA;
    mxlXtalCfg.XtalCap = 0x12; 
    mxlXtalCfg.XtalClkOutEnable = MXL_DISABLE;
    mxlXtalCfg.XtalClkOutGain = CLK_OUT_NA;
    MxLWare_API_ConfigDevice(MXL_DEV_XTAL_SETTINGS_CFG, &mxlXtalCfg);

    // 5. Set Baseband mode, SOC or Tuner only mode
    mxlDevMode.I2cSlaveAddr = mxlI2cSlvAddr;
    mxlDevMode.DeviceMode = (MXL_DEV_MODE_E)MXL_SOC_MODE;
    MxLWare_API_ConfigDevice(MXL_DEV_OPERATIONAL_MODE_CFG, &mxlDevMode);

    // 6. Configure MPEG Out 
    // CLK, MPEG_CLK_INV, Polarity of MPEG Valid/MPEG Sync
    mxlMpegOutCfg.I2cSlaveAddr = mxlI2cSlvAddr;
    //mxlMpegOutCfg.MpegClkFreq = MPEG_CLOCK_36_571429MHz;
    mxlMpegOutCfg.MpegClkFreq = MPEG_CLOCK_9_142857MHz;

    //mxlMpegOutCfg.MpegClkPhase = MPEG_CLK_IN_PHASE;
    mxlMpegOutCfg.MpegClkPhase = MPEG_CLK_INVERTED;

    mxlMpegOutCfg.MpegSyncPol = MPEG_CLK_IN_PHASE;
    mxlMpegOutCfg.MpegValidPol = MPEG_CLK_IN_PHASE;
    mxlMpegOutCfg.SerialOrPar = MPEG_DATA_PARALLEL;
    MxLWare_API_ConfigDevice(MXL_DEV_MPEG_OUT_CFG, &mxlMpegOutCfg);

    // 7. Turn ON TS 
    mxlTsConfig.I2cSlaveAddr = mxlI2cSlvAddr;
    mxlTsConfig.TsCtrl = MXL_ON;
    MxLWare_API_ConfigDevice(MXL_DEV_TS_CTRL_CFG, &mxlTsConfig);

    // 8. Enable Top Master Control
    mxlTopMasterCfg.I2cSlaveAddr = mxlI2cSlvAddr;
    mxlTopMasterCfg.TopMasterEnable = MXL_ENABLE;
    MxLWare_API_ConfigTuner(MXL_TUNER_TOP_MASTER_CFG, &mxlTopMasterCfg);
    // 9. Disable processing of Inverted Spectrum signals 
    mxlFreqSpectrumCfg.SpectrumCfg = MXL_OFF;
    mxlFreqSpectrumCfg.I2cSlaveAddr = mxlI2cSlvAddr;
    MxLWare_API_ConfigDemod(MXL_DEMOD_SPECTRUM_CFG, &mxlFreqSpectrumCfg);

    return 0;
}

int MxL101SFDVBT_Close(void)
{
    Ctrl_I2cDisConnect();
    Power_Off();
	return 0;
}

int MxL101SFDVBT_ChannelSet(int iChannel, int bLockOn)
{
	return 0;
}

int MxL101SFDVBT_FrequencySet(int iFreqKhz, int iBWKhz, int bLockOn, int *iSNR)
{	
    int ret = 0, i;
    MXL_RF_TUNE_CFG_T mxlChanCfg;
    MXL_DEMOD_LOCK_STATUS_T rsLockStatus;
    MXL_DEMOD_SNR_INFO_T snr;
    MXL_DEMOD_BER_INFO_T ber;
    MXL_SIGNAL_STATS_T sigStrength;
    MXL_DEMOD_CELL_ID_INFO_T tpsCellIdInfo;
    // 10. Tune RF with channel frequency and bandwidth
    mxlChanCfg.I2cSlaveAddr = mxlI2cSlvAddr;
    mxlChanCfg.Bandwidth = iBWKhz/1000;
    mxlChanCfg.Frequency = iFreqKhz*1000;
    giActiveFreq = -1;
    mxlChanCfg.TpsCellIdRbCtrl = MXL_ENABLE;  // Enable TPS Cell ID feature
    MxLWare_API_ConfigTuner(MXL_TUNER_CHAN_TUNE_CFG, &mxlChanCfg);

    if(bLockOn)
    {
        // Wait 200ms
        Ctrl_Sleep(200);
        for(i=0; i< 3; i++)
        {
            ret = 1;
            // Read back RS Lock Status
            rsLockStatus.I2cSlaveAddr = mxlI2cSlvAddr;
            MxLWare_API_GetDemodStatus(MXL_DEMOD_RS_LOCK_REQ, &rsLockStatus);
            ALOGD("MxL101SF : RS Lock Status = 0x%x@[%d]\n", rsLockStatus.Status, mxlChanCfg.Frequency);

            // Read back SNR
            snr.I2cSlaveAddr = mxlI2cSlvAddr;
            MxLWare_API_GetDemodStatus(MXL_DEMOD_SNR_REQ, &snr);
            ALOGD("MxL101SF : Demod SNR = %f dB\n", (REAL32)snr.SNR/10000);
#if 0            
            // Read back BER
            ber.I2cSlaveAddr = mxlI2cSlvAddr;
            MxLWare_API_GetDemodStatus(MXL_DEMOD_BER_REQ, &ber);
            ALOGD("MxL101SF : Demod BER = %f\n", (REAL32)ber.BER/1e10);

            // Read back Signal Strength
            sigStrength.I2cSlaveAddr = mxlI2cSlvAddr;
            MxLWare_API_GetTunerStatus(MXL_TUNER_SIGNAL_STRENGTH_REQ, &sigStrength);
            ALOGD("MxL101SF : Signal Strength = %f dBm\n", sigStrength.SignalStrength);

            // Read back TPS Cell ID
            tpsCellIdInfo.I2cSlaveAddr = mxlI2cSlvAddr;
            MxLWare_API_GetDemodStatus(MXL_DEMOD_TPS_CELL_ID_REQ, &tpsCellIdInfo);
            ALOGD("MxL101SF : TPS Cell ID = %04X\n", tpsCellIdInfo.TpsCellId);
#endif

            if((REAL32)snr.SNR/10000 > 200. && rsLockStatus.Status == 0)
            {
                ALOGD("No signal @ frequency\n");
                ret = 1;
                break;
            }
            else if((REAL32)snr.SNR/10000 < 200. && rsLockStatus.Status == 1)
            {
                ALOGD("Valid signal @ frequency\n");
                ret = 0;
                if(iSNR){
                    i = (int )(snr.SNR/10000);
                    if (i > 200)
                        i =0;
                    *iSNR =i;
                }
                break;
            }
            Ctrl_Sleep(100); // This delay is also experimental, it can be anything.
        }
    }

    giActiveFreq = iFreqKhz*1000;

    return ret;
}

int MxL101SFDVBT_GetSignalStrength(void)
{
    int percentage = 0;

    if (giActiveFreq < 0)
        return 0;

#if 0
    int snr_i = MxL101SFDVBT_GetSNR();

    percentage = (snr_i > 200) ? 0 : snr_i * 5;
#else
    MXL_SIGNAL_STATS_T sigStrength; 

    sigStrength.I2cSlaveAddr = mxlI2cSlvAddr;
    sigStrength.Frequency=giActiveFreq;
    MxLWare_API_GetTunerStatus(MXL_TUNER_SIGNAL_STRENGTH_REQ, &sigStrength);
    //ALOGD("MxL101SF : Signal Strength = %d dBm\n", sigStrength.SignalStrength);

    percentage = sigStrength.SignalStrength + 100;
#endif

    if (percentage < 0)
        return 0;
    if (percentage > 100)
        return 100;

    return percentage;
}

int MxL101SFDVBT_GetSNR(void)
{
    MXL_DEMOD_SNR_INFO_T snr;

    if (giActiveFreq < 0)
        return 0;

    snr.I2cSlaveAddr = mxlI2cSlvAddr;
    MxLWare_API_GetDemodStatus(MXL_DEMOD_SNR_REQ, &snr);

    return (int)(snr.SNR/10000);
}

int MxL101SFDVBT_GetBER(void)
{
    MXL_DEMOD_BER_INFO_T ber;

    if (giActiveFreq < 0)
        return 0;

    ber.I2cSlaveAddr = mxlI2cSlvAddr;
    MxLWare_API_GetDemodStatus(MXL_DEMOD_BER_REQ, &ber);

    return (int)(ber.BER/100);
}

int MxL101SFDVBT_GetSignalQuality(void)
{
    int percentage = 0;

    if (giActiveFreq < 0)
        return 0;

    if (MxL101SFDVBT_GetSNR() < 400000)
    {
#if 0
		percentage = 100 - MxL101SFDVBT_GetBER();
#else
        MXL_DEMOD_BER_INFO_T ber;

	    ber.I2cSlaveAddr = mxlI2cSlvAddr;
	    MxLWare_API_GetDemodStatus(MXL_DEMOD_BER_REQ, &ber);

	    float quality = (ber.BER == 0) ? 1 : pow(2.73, -200*ber.BER/1e10);

	    percentage = (int)(100 * quality);
#endif
    }

    if (percentage > 100)
        return 100;
    if (percentage < 0)
        return 0;

    return percentage;
}

