#include <stdio.h>
#include "MxL101SF_PhyCtrlApi.h"
#include "MxL101SF_OEM_Drv.h"

int main(void)
{
  UINT8 mxlI2cSlvAddr = 0x60;
  UINT32 loop;
  MXL_RESET_CFG_T mxlSoftReset;
  MXL_OVERWRITE_DEFAULT_CFG_T mxlInitSoc;
  MXL_DEV_INFO_T mxlDevInfo;
  MXL_XTAL_CFG_T mxlXtalCfg;
  MXL_DEV_MODE_CFG_T mxlDevMode;
  MXL_MPEG_CFG_T mxlMpegOutCfg;
  MXL_TS_CTRL_T mxlTsConfig;
  MXL_TOP_MASTER_CFG_T mxlTopMasterCfg;
  MXL_RF_TUNE_CFG_T mxlChanCfg;
  MXL_DEMOD_LOCK_STATUS_T rsLockStatus;
  MXL_DEMOD_SNR_INFO_T snr;
  MXL_DEMOD_BER_INFO_T ber;
  MXL_SIGNAL_STATS_T sigStrength;
  MXL_DEMOD_CELL_ID_INFO_T tpsCellIdInfo;
  MXL_DEMOD_SPECTRUM_CFG_T mxlFreqSpectrumCfg;

  
  // Open LPT driver for I2C communcation
  Ctrl_I2cConnect(mxlI2cSlvAddr);

  // 1. Do SW Reset
  mxlSoftReset.I2cSlaveAddr = mxlI2cSlvAddr;
  MxLWare_API_ConfigDevice(MXL_DEV_SOFT_RESET_CFG, &mxlSoftReset);
  
  // 2. Read Back chip id and version
  //    Expecting CHIP ID = 0x61, Version = 0x6
  mxlDevInfo.I2cSlaveAddr = mxlI2cSlvAddr;
  MxLWare_API_GetDeviceStatus(MXL_DEV_ID_VERSION_REQ, &mxlDevInfo);
  printf("MxL101SF : ChipId = 0x%x, Version = 0x%x\n", mxlDevInfo.DevId, 
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
  // with XTAL datasheet’s requirement.
  mxlXtalCfg.I2cSlaveAddr = mxlI2cSlvAddr;
  mxlXtalCfg.XtalFreq = XTAL_24MHz;
  mxlXtalCfg.LoopThruEnable = MXL_DISABLE;
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

printf("MxL101SF : Scanning Frequencies ....\n");  
for(int i=0; i< 48;i++)
{ 
  // 10. Tune RF with channel frequency and bandwidth
  mxlChanCfg.I2cSlaveAddr = mxlI2cSlvAddr;
  mxlChanCfg.Bandwidth = 8;
  mxlChanCfg.Frequency = 474000000 + i*8000000;
  mxlChanCfg.TpsCellIdRbCtrl = MXL_ENABLE;  // Enable TPS Cell ID feature
  MxLWare_API_ConfigTuner(MXL_TUNER_CHAN_TUNE_CFG, &mxlChanCfg);

  // Wait 200ms
  Ctrl_Sleep(200);

  // 11. Read back statistics like SNR, BER, and demod status
  
  loop = 3; // This experimental number to monitor statistics.
  while (loop--)
  {
    // Read back RS Lock Status
    rsLockStatus.I2cSlaveAddr = mxlI2cSlvAddr;
    MxLWare_API_GetDemodStatus(MXL_DEMOD_RS_LOCK_REQ, &rsLockStatus);
    printf("MxL101SF : RS Lock Status = 0x%x@[%d]\n", rsLockStatus.Status, mxlChanCfg.Frequency);

    // Read back SNR
    snr.I2cSlaveAddr = mxlI2cSlvAddr;
    MxLWare_API_GetDemodStatus(MXL_DEMOD_SNR_REQ, &snr);
    printf("MxL101SF : Demod SNR = %f dB\n", (REAL32)snr.SNR/10000);
    if((REAL32)snr.SNR/10000 > 200. && rsLockStatus.Status == 0)
    {
        printf("No signal @ frequency\n");
        break;
    }

    // Read back BER
    ber.I2cSlaveAddr = mxlI2cSlvAddr;
    MxLWare_API_GetDemodStatus(MXL_DEMOD_BER_REQ, &ber);
    printf("MxL101SF : Demod BER = %f\n", (REAL32)ber.BER/1e10);

    // Read back Signal Strength
    sigStrength.I2cSlaveAddr = mxlI2cSlvAddr;
    MxLWare_API_GetTunerStatus(MXL_TUNER_SIGNAL_STRENGTH_REQ, &sigStrength);
    printf("MxL101SF : Signal Strength = %f dBm\n", sigStrength.SignalStrength);
    
    // Read back TPS Cell ID
    tpsCellIdInfo.I2cSlaveAddr = mxlI2cSlvAddr;
    MxLWare_API_GetDemodStatus(MXL_DEMOD_TPS_CELL_ID_REQ, &tpsCellIdInfo);
    printf("MxL101SF : TPS Cell ID = %04X\n", tpsCellIdInfo.TpsCellId);

    Ctrl_Sleep(100); // This delay is also experimental, it can be anything.
  }
}
  // Want to change TV channel
  // go to Step 10
  Ctrl_I2cDisConnect();
  return 0;
}
