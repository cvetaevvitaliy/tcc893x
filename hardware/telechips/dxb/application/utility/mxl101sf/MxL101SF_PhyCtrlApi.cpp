/*****************************************************************************************
 *
 * FILE NAME          : MxL101SF_PhyCtrlApi.cpp
 * 
 * AUTHOR             : Brenndon Lee, Mahendra Kondur, Mariusz Murawski
 *
 * DATE CREATED       : 1/22/2008
 *                      3/31/2010(MK) - Support for multiple MxL101SF devices.
 *                      6/03/2010(MK) - Support for MP version chip.
 *                      1/2/2011(MM)  - Locking time optimization
 *
 * DESCRIPTION        : This file contains control APIs that configure MxL101SF 
 *                      and read back the statistics through I2C interface.
 *                             
 *****************************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ****************************************************************************************/

#include "MxL101SF_PhyCtrlApi.h"
#include "MxL101SF_PhyDefs.h"
#include "MxL101SF_OEM_Drv.h"

const UINT8 MxLWareVersion[] = {9, 2, 6, 2};

#ifdef MXL101_LOCKING_TIME_OPTIMIZATION
#warning Locking optimization enabled
#define MXL101_MAX_DEVICES 4
#define MXL101_MAX_DEVICES_MASK 0x03
static struct
{
  UINT8 last_bw;
  MXL_DEV_MODE_E mode;
} mxl101_ctx[MXL101_MAX_DEVICES];  
#endif

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevSoftReset
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 9/8/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|                 12/27/2011(MM)- Locking time optimization initialization added
--|
--| DESCRIPTION   : By writing any value into address 0xFF (AIC), all control 
--|                 registers are initialized to the default value.
--|                 AIC - Address Initiated Command
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevSoftReset(PMXL_RESET_CFG_T SoftResetPtr)
{
#ifdef MXL101_LOCKING_TIME_OPTIMIZATION
  mxl101_ctx[SoftResetPtr->I2cSlaveAddr & MXL101_MAX_DEVICES_MASK].last_bw = 0;
  mxl101_ctx[SoftResetPtr->I2cSlaveAddr & MXL101_MAX_DEVICES_MASK].mode = MXL_DEV_TUNER_DEMOD_MODE;
#endif
  /* Program software reset */
  return Ctrl_ProgramRegisters(SoftResetPtr->I2cSlaveAddr,  
                               MxL_PhySoftReset,            
                               SoftResetPtr->PrivateDataPtr);
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevGetChipInfo
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 2/7/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.

--| DESCRIPTION   : This function returns MxL101SF Chip Id and version information.
--|                 Chip Id of MxL101SF is 0x61
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevGetChipInfo(PMXL_DEV_INFO_T DevInfoPtr)
{
  UINT8 status;
  
  /* Read MxL101SF SoC chip id */
  status = Ctrl_ReadRegister(DevInfoPtr->I2cSlaveAddr,    /* I2C Address of MxL101SF */
                             CHIP_ID_REG,                 /* Register Address */
                             &DevInfoPtr->DevId,          /* Pointer to Data container */ 
                             DevInfoPtr->PrivateDataPtr); /* Pointer for private data */

  /* Read MxL101SF SoC revision id */
  status |= Ctrl_ReadRegister(DevInfoPtr->I2cSlaveAddr, 
                              TOP_CHIP_REV_ID_REG, 
                              &DevInfoPtr->DevVer, 
                              DevInfoPtr->PrivateDataPtr);

  DevInfoPtr->DevVer &= MXL101SF_DEV_REVISION_MASK;
  
  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevConfigMxLOperationalMode
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur, Mariusz Murawski
--|
--| DATE CREATED  : 1/15/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|                 6/3/2010(MK) - Code optimization.
--|                 12/27/2011(MM) - DevModePtr->DeviceMode changed to enum
--|                                - Locking time optimization added as conditional 
--|                                  compilation block
--|
--| DESCRIPTION   : MxL101SF has a built-in RF Tuner in addtion to DVB-T BB block
--|                 In tuner only mode, Digial BB will be disabled.
--|                 MXL_DEV_TUNER_MODE : Tuner mode, 
--|                 MXL_DEV_TUNER_DEMOD_MODE : SOC mode
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevConfigMxLOperationalMode(PMXL_DEV_MODE_CFG_T DevModePtr)
{
  PREG_CTRL_INFO_T mxlDevModeCtrlPtr;

#ifdef MXL101_LOCKING_TIME_OPTIMIZATION
  mxl101_ctx[DevModePtr->I2cSlaveAddr & MXL101_MAX_DEVICES_MASK].mode = DevModePtr->DeviceMode;
#endif
  if (DevModePtr->DeviceMode == MXL_DEV_TUNER_MODE) mxlDevModeCtrlPtr = MxL_TunerMode;
  else mxlDevModeCtrlPtr = MxL_TunerDemodMode;
  
  /* Program MxL101SF device mode registers sequence */
  return Ctrl_ProgramRegisters(DevModePtr->I2cSlaveAddr, 
                               mxlDevModeCtrlPtr, 
                               DevModePtr->PrivateDataPtr);
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerConfigTopMaster
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 4/17/2009
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|                 6/3/2010(MK) - Code optimization.
--|
--| DESCRIPTION   : This function will control MxL101SF tuner block.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerConfigTopMaster(PMXL_TOP_MASTER_CFG_T TopMasterCtrlPtr)
{
  PREG_CTRL_INFO_T ctrlRegPtr;
   
  if (TopMasterCtrlPtr->TopMasterEnable == MXL_DISABLE) ctrlRegPtr = MxL_TopMasterDisable;
  else ctrlRegPtr = MxL_TopMasterEnable;
  
  /* Program top master control registers sequence */
  return Ctrl_ProgramRegisters(TopMasterCtrlPtr->I2cSlaveAddr, 
                               ctrlRegPtr, 
                               TopMasterCtrlPtr->PrivateDataPtr);
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevInitTunerDemod
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 7/30/2009
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : Initializing Tuner and Demod block of MxL101SF by overwriting
--|                 power on default values.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevInitTunerDemod(PMXL_OVERWRITE_DEFAULT_CFG_T InitSocPtr)
{
  /* Program initilization settings for MxL101SF */
  return Ctrl_ProgramRegisters(InitSocPtr->I2cSlaveAddr, 
                               MxL_OverwriteDefault, 
                               InitSocPtr->PrivateDataPtr);
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevControlMpegOut
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 05/22/2010
--|
--| DESCRIPTION   : This function will turn ON or OFF MPEG-2 transport stream and
--|                 also control MPEG drive strength.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevControlMpegOut(PMXL_TS_CTRL_T TsOutCtrlPtr)
{
  UINT8 status;
  UINT8 regData = 0;
  PREG_CTRL_INFO_T tsCtrlPtr;

  // Rename MPEG-ON/OFF
  if (TsOutCtrlPtr->TsCtrl == MXL_ON) tsCtrlPtr = MxL_MpegTsON;
  else tsCtrlPtr = MxL_MpegTsOFF;

  /* Program transport stream control sequence */
  status = Ctrl_ProgramRegisters(TsOutCtrlPtr->I2cSlaveAddr, 
                                 tsCtrlPtr, 
                                 TsOutCtrlPtr->PrivateDataPtr);

  /* Read MPEG-2 TS drive strength register */
  status |= Ctrl_ReadRegister(TsOutCtrlPtr->I2cSlaveAddr, 
                              MPEG_DRIVE_STRENGTH_REG, 
                              &regData, 
                              TsOutCtrlPtr->PrivateDataPtr);

  /* Apply required register setting for MPEG-2 drive strength */
  if (TsOutCtrlPtr->TsDriveStrength == TS_DRIVE_STRENGTH_2X) 
    regData |= MPEG_DRIVE_STRENGTH_MASK;
  else 
    regData &= ~MPEG_DRIVE_STRENGTH_MASK;
    
  status |= Ctrl_WriteRegister(TsOutCtrlPtr->I2cSlaveAddr, 
                               MPEG_DRIVE_STRENGTH_REG, 
                               regData, 
                               TsOutCtrlPtr->PrivateDataPtr);
  
  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevConfigMpegOut
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 1/15/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|                 6/3/2010(MK) - Code optimization.
--|
--| DESCRIPTION   : This function will configure MPEG-2 output through ASI(MPEG TS) 
--|                 interface the following parameters.
--|                  TS output mode : Seral or Parallel
--|                  MPEG CLK Phase : Normal or Inverted
--|                  MPEG Valid Polarity : Low or High
--|                  MPEG Sync Polarity  : Low or High
--|                  MPEG CLK Frequency  : 0 - 36.571429MHz
--|                                      : 1 - 2.285714MHz
--|                                      : 2 - 4.571429MHz
--|                                      : 3 - 6.857143MHz
--|                                      : 4 - 9.142857MHz
--|                                      : 5 - 13.714286MHz
--|                                      : 6 - 18.285714MHz
--|                                      : 7 - 27.428571MHz
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevConfigMpegOut(PMXL_MPEG_CFG_T MpegOutCfgPtr)
{
  UINT8 status;
  UINT8 mpegMode;
  UINT8 regData;
  UINT8 i2cSlvAddr = MpegOutCfgPtr->I2cSlaveAddr;
  void *privDataPtr = MpegOutCfgPtr->PrivateDataPtr;

  /* Always enable MPEG OUT setting for MxL101SF */
  mpegMode = MPEG_OUT_ENABLE_MASK; 

  /* Configure MPEG clock phase */
  if (MpegOutCfgPtr->MpegClkPhase == MPEG_CLK_IN_PHASE) mpegMode &= ~INVERTED_CLK_PHASE;
  else mpegMode |= INVERTED_CLK_PHASE;;

  /* Configure MPEG clock frequency */
  if (MpegOutCfgPtr->MpegClkFreq > MPEG_CLOCK_27_428571MHz) 
    MpegOutCfgPtr->MpegClkFreq = MPEG_CLOCK_27_428571MHz;
  
  mpegMode |= (((UINT8)MpegOutCfgPtr->MpegClkFreq) << 2);

  /* Write MPEG clock frequency & clock phase register */
  status = Ctrl_WriteRegister(i2cSlvAddr, MPEG_OUT_CLK_INV_REG, mpegMode, privDataPtr);

  /* Read current MPEG setting's from device */
  status |= Ctrl_ReadRegister(i2cSlvAddr, MPEG_OUT_CTRL_REG, &mpegMode, privDataPtr);
  
  /* Clear current MPEG settings */
  mpegMode &= CLEAR_MPEG_SETTINGS_MASK;

  /* Configure new MPEG settings */
  if (MpegOutCfgPtr->SerialOrPar == MPEG_DATA_PARALLEL) mpegMode |= MPEG_DATA_PARALLEL;
  else 
  {
    mpegMode |= MPEG_DATA_SERIAL;

    /* If serial interface is selected, configure MSB or LSB order in transmission */
    status |= Ctrl_ReadRegister(i2cSlvAddr, MPEG_BIT_ORDER_REG, &regData, privDataPtr);

    /* Configure MPEG start byte order for serial mode */
    if (MpegOutCfgPtr->LsbOrMsbFirst == MPEG_SERIAL_MSB_1ST) regData |= MPEG_SER_MSB;  
    else regData &= ~MPEG_SER_MSB; 

    status |= Ctrl_WriteRegister(i2cSlvAddr, MPEG_BIT_ORDER_REG, regData, privDataPtr);
  }
  
  /* Configure MPEG Sync polarity */
  if (MpegOutCfgPtr->MpegSyncPol == MPEG_CLK_IN_PHASE) mpegMode &= ~INVERTED_MPEG_SYNC;
  else mpegMode |= INVERTED_MPEG_SYNC;

  /* configure MPEG Valid polarity */
  if (MpegOutCfgPtr->MpegValidPol == MPEG_CLK_IN_PHASE) mpegMode &= ~INVERTED_MPEG_VALID; 
  else mpegMode |= INVERTED_MPEG_VALID;

  status |= Ctrl_WriteRegister(i2cSlvAddr, MPEG_OUT_CTRL_REG, mpegMode, privDataPtr);

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevConfigPowerSavingMode
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 10/20/2009
--|                 3/31/2010 - Support for multiple MxL101SF devices.
--|                 6/29/2010 - Support for standby mode.
--|
--| DESCRIPTION   : This function configures standby mode and sleep mode of MxL101SF
--|                 to control power consumption.
--|
--| RETURN VALUE  : True or False
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevConfigPowerSavingMode(PMXL_PWR_MODE_CFG_T PwrModePtr)
{
  UINT8 status = MXL_FALSE;
  MXL_RESET_CFG_T mxlReset;
  MXL_OVERWRITE_DEFAULT_CFG_T initTunerDemod;
  
  switch (PwrModePtr->PowerMode)
  {
    case STANDBY_ON:
      /* Program MxL101SF standby mode registers sequence */
      status = Ctrl_ProgramRegisters(PwrModePtr->I2cSlaveAddr, 
                                      MxL_StandbyMode, 
                                      PwrModePtr->PrivateDataPtr);
      break;

    case SLEEP_ON:
      /* Program MxL101SF suspend mode registers sequence */
      status = Ctrl_ProgramRegisters(PwrModePtr->I2cSlaveAddr, 
                                     MxL_SuspendMode, 
                                     PwrModePtr->PrivateDataPtr);
      break;

    case STANDBY_OFF:
      /* Program MxL101SF wake up registers sequence, 
         when device is in standby mode */
      status = Ctrl_ProgramRegisters(PwrModePtr->I2cSlaveAddr, 
                                     MxL_TopMasterEnable, 
                                     PwrModePtr->PrivateDataPtr);
      
      status |= Ctrl_WriteRegister(PwrModePtr->I2cSlaveAddr, 
                                  MXL_TUNE_REG, 
                                  0x01, 
                                  PwrModePtr->PrivateDataPtr);
      break;

    case SLEEP_OFF:
      /* Program MxL101SF wake up registers sequence, 
         when device is in suspend mode */
      status = Ctrl_ProgramRegisters(PwrModePtr->I2cSlaveAddr, 
                                     MxL_WakeUp, 
                                     PwrModePtr->PrivateDataPtr);
      
      mxlReset.I2cSlaveAddr = PwrModePtr->I2cSlaveAddr;
      mxlReset.PrivateDataPtr = PwrModePtr->PrivateDataPtr;

      initTunerDemod.I2cSlaveAddr = PwrModePtr->I2cSlaveAddr;
      initTunerDemod.PrivateDataPtr = PwrModePtr->PrivateDataPtr;
      
      /* When MxL101SF is woken up from suspend mode, 
         program S/W reset and initilize the device */
      status |= MxL101SF_API_DevSoftReset(&mxlReset);   
      status |= MxL101SF_API_DevInitTunerDemod(&initTunerDemod);
      break;

    default:
      break;
  }

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerChanTune
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur, Mariusz Murawski
--|
--| DATE CREATED  : 1/15/2008
--|                 10/23/2009(MK) - Removed S/W fix for DFT bug.
--|                 11/20/2009(MK) - Settings for TPS Cell ID.
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|                 6/3/2010(MK) - Code optimization.
--|                 12/27/2010(MM) - More optimizations
--|                                - Locking time optimization added as conditional 
--|                                  compilation block
--|
--| DESCRIPTION   : This function will tune MxL101SF device with a given RF value, 
--|                 control TPS cell id feature and also optimize MxL101SF device.
--|                 When channel frequency has to be changed this function should 
--|                 be called.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerChanTune(PMXL_RF_TUNE_CFG_T ChanTuneParamPtr)
{
  UINT8 status;
  MXL_DEV_MODE_E mxl_mode;
  UINT8 regData = 0;
  MXL_BOOL cpLockStatus = MXL_OFF;
  UINT8 i2cSlvAddr = ChanTuneParamPtr->I2cSlaveAddr;
  void *privDataPtr = ChanTuneParamPtr->PrivateDataPtr;
  REG_CTRL_INFO_T ctrlArrayPtr[4];
  
  /* Stop tune */
  status = Ctrl_WriteRegister(i2cSlvAddr, MXL_TUNE_REG, 0, privDataPtr);

#ifdef MXL101_LOCKING_TIME_OPTIMIZATION
  mxl_mode = mxl101_ctx[i2cSlvAddr & MXL101_MAX_DEVICES_MASK].mode;
#else
  /* Check device operational mode */
  mxl_mode = Ctrl_ReadCurrentDeviceMode(i2cSlvAddr, privDataPtr);
#endif  

#ifdef MXL101_LOCKING_TIME_OPTIMIZATION
  if (ChanTuneParamPtr->Bandwidth != mxl101_ctx[i2cSlvAddr & MXL101_MAX_DEVICES_MASK].last_bw)
  {
#endif    
    /* Configure spur coefficiency according to specified bandwidth, see overwriteDefault section in PG */
    status |= Ctrl_ConfigSpurCoeff(ChanTuneParamPtr);

    /* Fill out registers required for channel tune */
    Ctrl_PhyTune(ChanTuneParamPtr->Frequency, ChanTuneParamPtr->Bandwidth, mxl_mode, ctrlArrayPtr);
  
    /* Program channel tune registers sequence */
    status |= Ctrl_ProgramRegisters(i2cSlvAddr, ctrlArrayPtr, privDataPtr);
#ifdef MXL101_LOCKING_TIME_OPTIMIZATION
    mxl101_ctx[i2cSlvAddr & MXL101_MAX_DEVICES_MASK].last_bw = ChanTuneParamPtr->Bandwidth;
  }
#endif

  /* Enable TPS cell id feature, if requested by the application */
  if (ChanTuneParamPtr->TpsCellIdRbCtrl == MXL_ENABLE)
    status |= Ctrl_ProgramRegisters(i2cSlvAddr, MxL_EnableCellId, privDataPtr); 

  /* Start tune */
  status |= Ctrl_WriteRegister(i2cSlvAddr, MXL_TUNE_REG, 1, privDataPtr);

  /* When MxL101SF operating in SoC mode, device has to be configure for
     optimal performance during channel tuning */

  if ((mxl_mode == MXL_DEV_TUNER_DEMOD_MODE) && (ChanTuneParamPtr->TpsCellIdRbCtrl == MXL_ENABLE))
  {
    UINT16 agcGain = 0;

    UINT32 timeNow = 0;
    UINT32 cellIdStartTime;
    UINT32 timeout;
    UINT32 cellIdEndTime;

    /* Get timestamps */
    Ctrl_GetTime(&timeNow);

    cellIdStartTime = timeNow;
    timeout = cellIdStartTime + CELL_ID_CP_LOCK_TO2;    
    cellIdEndTime = 0;
    
    /* Loop until timeout period is not expired or CP lock is not acquired */
    while ((timeNow < timeout) && ((timeNow < cellIdEndTime) || (cellIdEndTime == 0)))
    {
      Ctrl_Sleep(1);
      /* Check for CP lock */
      status |= Ctrl_ReadRegister(i2cSlvAddr, CP_LOCK_REG, &regData, privDataPtr);
      cpLockStatus = ((regData & CP_LOCK_DET_MASK) >> 2) ? MXL_ON : MXL_OFF;

      if (cpLockStatus == MXL_ON)
      {
        if (cellIdEndTime == 0) 
        {
          cellIdEndTime = timeNow + CELL_ID_CP_LOCK_TO;
        }
      } 
      else cellIdEndTime = 0;

      
      Ctrl_GetTime(&timeNow);
      if (((cellIdStartTime + CELL_ID_CP_LOCK_TO) < timeNow) && (agcGain == 0))
      {
        Ctrl_ReadAgcGain(i2cSlvAddr, privDataPtr, &agcGain);
        timeout = cellIdStartTime + ((agcGain < AGC_NO_CHANNEL_THRESHOLD)?CELL_ID_CP_LOCK_TO2:CELL_ID_CP_LOCK_TO1);    
      }
    } 
    status |= Ctrl_ProgramRegisters(i2cSlvAddr, MxL_DisableCellId, privDataPtr); 
  }
  

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerChanTuneExt
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 8/09/2010
--|
--| DESCRIPTION   : Optional Tune RF API for better performance under higher 
--|                 temperature range.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerChanTuneExt(PMXL_RF_TUNE_CFG_T ChanTuneParamPtr)
{
  REG_CTRL_INFO_T ctrlArrayPtr[4];
  UINT8 status;
  MXL_DEV_MODE_E mxl_mode;
  UINT8 regData = 0;
  MXL_BOOL cpLockStatus = MXL_OFF;
  UINT8 i2cSlvAddr = ChanTuneParamPtr->I2cSlaveAddr;
  void *privDataPtr = ChanTuneParamPtr->PrivateDataPtr;
  
  /* Stop tune */
  status = Ctrl_WriteRegister(i2cSlvAddr, MXL_TUNE_REG, 0, privDataPtr);

  /* Check device operational mode */
  mxl_mode = Ctrl_ReadCurrentDeviceMode(i2cSlvAddr, privDataPtr);

  /* Fill out registers required for channel tune */
  Ctrl_PhyTune(ChanTuneParamPtr->Frequency, ChanTuneParamPtr->Bandwidth, mxl_mode, ctrlArrayPtr);
  
  /* Program channel tune registers sequence */
  status |= Ctrl_ProgramRegisters(i2cSlvAddr, ctrlArrayPtr, privDataPtr);

  /* Enable TPS cell id feature, if requested by the application */
  if (ChanTuneParamPtr->TpsCellIdRbCtrl == MXL_ENABLE)
    status |= Ctrl_ProgramRegisters(i2cSlvAddr, MxL_EnableCellId, privDataPtr); 

  status |= Ctrl_ProgramRegisters(i2cSlvAddr, MxL_EnableExtTuneCfg, privDataPtr);

  /* Start tune */
  status |= Ctrl_WriteRegister(i2cSlvAddr, MXL_TUNE_REG, 1, privDataPtr);

  /* Sleep for 32 msec's */
  Ctrl_Sleep(32);
  status |= Ctrl_ProgramRegisters(i2cSlvAddr, MxL_DisableExtTuneCfg, privDataPtr);

  /* When MxL101SF operating in SoC mode, device has to be configure for
     optimal performance during channel tuning */
  if ((mxl_mode == MXL_DEV_TUNER_DEMOD_MODE) && (ChanTuneParamPtr->TpsCellIdRbCtrl == MXL_ENABLE))
  {
    UINT16 agcGain = 0;

    UINT32 timeNow = 0;
    UINT32 cellIdStartTime;
    UINT32 timeout;
    UINT32 cellIdEndTime;

    /* Get timestamps */
    Ctrl_GetTime(&timeNow);

    cellIdStartTime = timeNow;
    timeout = cellIdStartTime + CELL_ID_CP_LOCK_TO2;    
    cellIdEndTime = 0;
    
    /* Loop until timeout period is not expired or CP lock is not acquired */
    while ((timeNow < timeout) && ((timeNow < cellIdEndTime) || (cellIdEndTime == 0)))
    {
      Ctrl_Sleep(1);
      /* Check for CP lock */
      status |= Ctrl_ReadRegister(i2cSlvAddr, CP_LOCK_REG, &regData, privDataPtr);
      cpLockStatus = ((regData & CP_LOCK_DET_MASK) >> 2) ? MXL_ON : MXL_OFF;

      if (cpLockStatus == MXL_ON)
      {
        if (cellIdEndTime == 0) 
        {
          cellIdEndTime = timeNow + CELL_ID_CP_LOCK_TO;
        }
      } 
      else cellIdEndTime = 0;

      
      Ctrl_GetTime(&timeNow);
      if (((cellIdStartTime + CELL_ID_CP_LOCK_TO) < timeNow) && (agcGain == 0))
      {
        Ctrl_ReadAgcGain(i2cSlvAddr, privDataPtr, &agcGain);
        timeout = cellIdStartTime + ((agcGain < AGC_NO_CHANNEL_THRESHOLD)?CELL_ID_CP_LOCK_TO2:CELL_ID_CP_LOCK_TO1);    
      }
    } 
    status |= Ctrl_ProgramRegisters(i2cSlvAddr, MxL_DisableCellId, privDataPtr); 
  }
  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_ConfigFreqOffsetSearchRange
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 8/31/2010
--|
--| DESCRIPTION   : This function increases frequency offset search  range 
--|                 during channel scan operation.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_ConfigFreqOffsetSearchRange(PMXL_TUNER_FREQ_OFFSET_CFG_T FreqOffsetRangeCfgPtr)
{
  UINT8 Status = MXL_FALSE;
  UINT8 rawData = 0;

		Status = Ctrl_ReadRegister(FreqOffsetRangeCfgPtr->I2cSlaveAddr, 
			                          FREQ_OFFSET_SRCH_RANGE_REG,
																													&rawData,
																													FreqOffsetRangeCfgPtr->PrivateDataPtr);

  if (FreqOffsetRangeCfgPtr->MaxFreqOffsetRangeCfg == MXL_ENABLE) rawData |= 0x07;
  else rawData &= ~0x07;

  Status |= Ctrl_WriteRegister(FreqOffsetRangeCfgPtr->I2cSlaveAddr,
			                            FREQ_OFFSET_SRCH_RANGE_REG,
																															rawData,
																															FreqOffsetRangeCfgPtr->PrivateDataPtr);

  return (MXL_STATUS)Status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetSNR
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur  
--|
--| DATE CREATED  : 1/24/2008
--|                 2/12/2010(MK) - Support for Interger calculation of SNR.
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices 
--|
--| DESCRIPTION   : This function returns SNR(Signal to Noise Ratio).
--|                 SNR is calculated as follows after reading 10bit register
--|                 Integer calculation: 
--|                  10000 x SNR = 1563 x SNR_REG_VALUE - 25000  dB 
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetSNR(PMXL_DEMOD_SNR_INFO_T SnrPtr)
{
  UINT8 status = MXL_FALSE;
  UINT8 rawData;

  /* Read <7:0> bits of SNR data */
  status = Ctrl_ReadRegister(SnrPtr->I2cSlaveAddr, 
                             SNR_RB_LSB_REG, 
                             &rawData, 
                             SnrPtr->PrivateDataPtr);
  SnrPtr->SNR = rawData;

  /* Read <10:8> bits of SNR data */
  status |= Ctrl_ReadRegister(SnrPtr->I2cSlaveAddr, 
                              SNR_RB_MSB_REG, 
                              &rawData, 
                              SnrPtr->PrivateDataPtr);

  SnrPtr->SNR |= (rawData & 0x03) << 8;
  
  /* Calculate SNR value */
  if (status == MXL_TRUE) SnrPtr->SNR =  CALCULATE_SNR(SnrPtr->SNR); 
  
  return (MXL_STATUS)status;       
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetBER
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 1/24/2008
--|                 2/24/2010(MK) - Support for Interger calculation of BER.
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function returns BER(Bit Error Ratio).
--|                 Integer calculation
--|                  1e10 x BER = (AVG_ERRORS x 4 x 3247)
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetBER(PMXL_DEMOD_BER_INFO_T BerInfoPtr)
{
  UINT8 status;
  UINT8 rawData;
  
  /* Read <7:0> bits of BER data */
  status = Ctrl_ReadRegister(BerInfoPtr->I2cSlaveAddr, 
                             RS_AVG_ERRORS_LSB_REG, 
                             &rawData, 
                             BerInfoPtr->PrivateDataPtr);
  BerInfoPtr->BER = rawData;

  /* Read <15:8> bits of BER data */
  status |= Ctrl_ReadRegister(BerInfoPtr->I2cSlaveAddr, 
                              RS_AVG_ERRORS_MSB_REG, 
                              &rawData, 
                              BerInfoPtr->PrivateDataPtr);

  BerInfoPtr->BER |= (rawData << 8);

  /* Calculate BER value */
  if (status == MXL_TRUE) BerInfoPtr->BER = CALCULATE_BER(BerInfoPtr->BER);

  return (MXL_STATUS)status;       
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetSyncLockStatus
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 5/6/2009
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function return MPEG-2 SYNC_LOCK Status of MxL101SF.
--|                 MXL_LOCKED, if locked else MXL_UNLOCKED
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetSyncLockStatus(PMXL_DEMOD_LOCK_STATUS_T SyncLockPtr)
{
  MXL_STATUS status;
  UINT8 rawData;
  
  /* Read MPEG SYNC lock status */
  status = Ctrl_ReadRegister(SyncLockPtr->I2cSlaveAddr, 
                             SYNC_LOCK_REG, 
                             &rawData, 
                             SyncLockPtr->PrivateDataPtr);

  if (status == MXL_TRUE)
  { 
    if ((rawData & SYNC_LOCK_MASK) >> 4) SyncLockPtr->Status = MXL_LOCKED;
    else SyncLockPtr->Status = MXL_UNLOCKED;
  }

  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetRsLockStatus
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 4/30/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function returns RS Lock status.
--|                 MXL_LOCKED, if locked else MXL_UNLOCKED
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetRsLockStatus(PMXL_DEMOD_LOCK_STATUS_T RsLockPtr)
{
  MXL_STATUS status;
  UINT8 rawData;

  /* Read demod RS lock status */
  status = Ctrl_ReadRegister(RsLockPtr->I2cSlaveAddr, 
                             RS_LOCK_REG, 
                             &rawData, 
                             RsLockPtr->PrivateDataPtr);

  if (status == MXL_TRUE)
  {
    if ((rawData & RS_LOCK_DET_MASK) >> 3) RsLockPtr->Status = MXL_LOCKED;
    else RsLockPtr->Status = MXL_UNLOCKED;
  }
 
  return status;       
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetCpLockStatus
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 4/30/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function returns CP Lock status.
--|                 MXL_LOCKED, if locked else MXL_UNLOCKED
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetCpLockStatus(PMXL_DEMOD_LOCK_STATUS_T CpLockPtr)
{
  MXL_STATUS status;
  UINT8 rawData;

  /* Read demod CP lock status */
  status = Ctrl_ReadRegister(CpLockPtr->I2cSlaveAddr, 
                             CP_LOCK_REG, 
                             &rawData, 
                             CpLockPtr->PrivateDataPtr);

  if (status == MXL_TRUE)
  {
    if((rawData & CP_LOCK_DET_MASK) >> 2) CpLockPtr->Status = MXL_LOCKED;
    else CpLockPtr->Status = MXL_UNLOCKED;
  }
 
  return status;       
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodResetIrqStatus
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 1/24/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.s
--|
--| DESCRIPTION   : This function clears IRQ status registers.
--|                 Writing 0xFF to this register will clear previous status.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodResetIrqStatus(PMXL_RESET_IRQ_CFG_T ResetIrqPtr)
{
  /* Reset demod IRQ status */
  return Ctrl_ProgramRegisters(ResetIrqPtr->I2cSlaveAddr, 
                               MxL_IrqClear, 
                               ResetIrqPtr->PrivateDataPtr);
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsCodeRate
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 1/24/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function returns code rate of TPS parameters.
--|                 bit<2:0> - 000:1/2, 001:2/3, 010:3/4, 011:5/6, 100:7/8
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsCodeRate(PMXL_DEMOD_TPS_INFO_T TpsCodeRatePtr)
{
  MXL_STATUS status;
  
  /* Read TPS Code Rate information */
  status = Ctrl_ReadRegister(TpsCodeRatePtr->I2cSlaveAddr, 
                             CODE_RATE_TPS_REG, 
                             &TpsCodeRatePtr->TpsInfo, 
                             TpsCodeRatePtr->PrivateDataPtr);

  if (status == MXL_TRUE) TpsCodeRatePtr->TpsInfo &= CODE_RATE_TPS_MASK;
  
  return status;       
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsHierarchy
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 1/24/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function returns Hiearchy information of TPS parameters.
--|                 bit<6:4> - 000:Non hierarchy, 001:1, 010:2, 011:4
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsHierarchy(PMXL_DEMOD_TPS_INFO_T TpsHierarchyPtr)
{
  MXL_STATUS status;
  
  /* Read TPS Hierarchy information */
  status = Ctrl_ReadRegister(TpsHierarchyPtr->I2cSlaveAddr, 
                             TPS_HIERACHY_REG, 
                             &TpsHierarchyPtr->TpsInfo, 
                             TpsHierarchyPtr->PrivateDataPtr);
  
  TpsHierarchyPtr->TpsInfo &= TPS_HIERARCHY_INFO_MASK;

  if (status == MXL_TRUE) TpsHierarchyPtr->TpsInfo >>= 6;
  
  return status;       
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodConfigStreamPriority
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 1/14/2010
--|                 3/31/2010 - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This API will config High priority or Low priority stream.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodConfigStreamPriority(PMXL_DEMOD_TS_PRIORITY_CFG_T TsCfgPtr)
{
  UINT8 status;
  UINT8 rawData;

  status = Ctrl_ReadRegister(TsCfgPtr->I2cSlaveAddr, 
                             TPS_HPORLP_REG, 
                             &rawData, 
                             TsCfgPtr->PrivateDataPtr);

  if (status == MXL_TRUE)
  {
    /* Configure TPS stream priority information */  
    if (TsCfgPtr->StreamPriority == LP_STREAM) rawData = (rawData | 0x80);
    else rawData = (rawData & 0x7F);

    status |= Ctrl_WriteRegister(TsCfgPtr->I2cSlaveAddr, 
                                 TPS_HPORLP_REG, 
                                 rawData, 
                                 TsCfgPtr->PrivateDataPtr);
  }
    
  return (MXL_STATUS)status;       
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetStreamPriority
--| 
--| AUTHOR        : Mariusz Murawski
--|
--| DATE CREATED  : 1/05/2011
--|
--| DESCRIPTION   : This API reads signal data and returns currently set 
--|                 stream priority.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetStreamPriority(PMXL_DEMOD_TS_PRIORITY_CFG_T TsCfgPtr)
{
  UINT8 status;
  UINT8 rawData = 0;

  status = Ctrl_ReadRegister(TsCfgPtr->I2cSlaveAddr, 
                             TPS_HPORLP_REG, 
                             &rawData, 
                             TsCfgPtr->PrivateDataPtr);

  if (status == MXL_TRUE)
  {
    TsCfgPtr->StreamPriority = (rawData & 0x80)?LP_STREAM:HP_STREAM;
  }
    
  return (MXL_STATUS)status;       
}


/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetHierarchicalAlphaValue
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 1/14/2010
--|                 3/31/2010 - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function returns Hierarchical Alpha of TPS parameters.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetHierarchicalAlphaValue(PMXL_DEMOD_TPS_INFO_T TpsAlphaPtr)
{
  MXL_STATUS status;
  
  /* Read TPS Hierarchical Alpha information */
  status = Ctrl_ReadRegister(TpsAlphaPtr->I2cSlaveAddr, 
                             TPS_ALPHA_REG, 
                             &TpsAlphaPtr->TpsInfo, 
                             TpsAlphaPtr->PrivateDataPtr);

  if (status == MXL_TRUE) TpsAlphaPtr->TpsInfo &= TPS_ALPHA_MASK;
  
  return status;       
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsConstellation
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 1/24/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function return Constellation status bit information.
--|                 Constellation, 00 : QPSK, 01 : 16QAM, 10:64QAM   
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsConstellation(PMXL_DEMOD_TPS_INFO_T TpsConstPtr)
{
  MXL_STATUS status;
  
  /* Read TPS constellation information */
  status = Ctrl_ReadRegister(TpsConstPtr->I2cSlaveAddr, 
                             MODORDER_TPS_REG, 
                             &TpsConstPtr->TpsInfo, 
                             TpsConstPtr->PrivateDataPtr);
 
  TpsConstPtr->TpsInfo &= PARAM_CONSTELLATION_MASK;

  if (status == MXL_TRUE) TpsConstPtr->TpsInfo >>= 4;
  
  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsFftMode
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 1/24/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function return FFT Mode status bit information.
--|                 FFT Mode, 00:2K, 01:8K, 10:4K
--| 
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsFftMode(PMXL_DEMOD_TPS_INFO_T TpsFftModePtr)
{
  MXL_STATUS status;
  
  /* Read TPS FFT mode information */
  status = Ctrl_ReadRegister(TpsFftModePtr->I2cSlaveAddr, 
                             MODE_TPS_REG, 
                             &TpsFftModePtr->TpsInfo, 
                             TpsFftModePtr->PrivateDataPtr);

  TpsFftModePtr->TpsInfo &= PARAM_FFT_MODE_MASK;

  if (status == MXL_TRUE) TpsFftModePtr->TpsInfo >>= 2;
  
  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsGuardInterval
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 1/24/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function return GI status bit information.
--|                 00:1/32, 01:1/16, 10:1/8, 11:1/4
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsGuardInterval(PMXL_DEMOD_TPS_INFO_T TpsGIPtr)
{
  MXL_STATUS status;
  
  /* Read TPS Guard Interval information */
  status = Ctrl_ReadRegister(TpsGIPtr->I2cSlaveAddr, 
                             CP_TPS_REG, 
                             &TpsGIPtr->TpsInfo, 
                             TpsGIPtr->PrivateDataPtr);

  TpsGIPtr->TpsInfo &= PARAM_GI_MASK;
  
  if (status == MXL_TRUE) TpsGIPtr->TpsInfo >>= 4 ;
  
  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetTpsLock
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 7/29/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function returns TPS lock status bit information.
--|                 MXL_LOCKED, if locked else MXL_UNLOCKED
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetTpsLock(PMXL_DEMOD_LOCK_STATUS_T TpsLockPtr)
{
  MXL_STATUS status;
  UINT8 tpsParams;
  
  /* Read demod TPS lock satus */
  status = Ctrl_ReadRegister(TpsLockPtr->I2cSlaveAddr, 
                             TPS_LOCK_REG, 
                             &tpsParams, 
                             TpsLockPtr->PrivateDataPtr);
 
  if (status == MXL_TRUE)
  {
    if((tpsParams & PARAM_TPS_LOCK_MASK) >> 6) TpsLockPtr->Status = MXL_LOCKED;
    else TpsLockPtr->Status = MXL_UNLOCKED;
  }

  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetPacketErrorCount
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 3/13/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function returns TS Packet error count.
--|                 
--|                  PER Count = FEC_PER_COUNT * (2 ** (FEC_PER_SCALE * 4))
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetPacketErrorCount(PMXL_DEMOD_PEC_INFO_T PecInfoPtr)
{
  UINT8 status;
  UINT32 fec_per_count, fec_per_scale;
  UINT8 rawData;
  
  /* Read FEC_PER_COUNT Register */
  status = Ctrl_ReadRegister(PecInfoPtr->I2cSlaveAddr, 
                             FEC_PER_COUNT_REG, 
                             &rawData, 
                             PecInfoPtr->PrivateDataPtr);
  fec_per_count = rawData;
 
  /* Read FEC_PER_SCALE Register */
  status |= Ctrl_ReadRegister(PecInfoPtr->I2cSlaveAddr, 
                              FEC_PER_SCALE_REG, 
                              &rawData, 
                              PecInfoPtr->PrivateDataPtr);
  
  rawData &= FEC_PER_SCALE_MASK;
  rawData *= 4;
  
  /* Calaculate packet error count */
  fec_per_scale = 1 << rawData;
  fec_per_count *= fec_per_scale;
  PecInfoPtr->PEC = fec_per_count;
  
  return (MXL_STATUS)status;       
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodResetPacketErrorCount
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 3/13/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function resets TS Packet error count.
--|                 Setting 7th bit of V5_PER_COUNT_RESET_REG will reset PEC to 0.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodResetPacketErrorCount(PMXL_RESET_PEC_CFG_T ResetPecPtr)
{
  /* Reset PEC counter */
  return Ctrl_ProgramRegisters(ResetPecPtr->I2cSlaveAddr, 
                               MxL_ResetPerCount, 
                               ResetPecPtr->PrivateDataPtr);
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerGetLockStatus
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 7/3/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function provides RF synthesizer lock status of MxL101SF
--|                 tuner block.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerGetLockStatus(PMXL_TUNER_LOCK_STATUS_T TunerLockStatusPtr)
{
  MXL_STATUS status;
  UINT8 data;
  
  /* Read tuner lock status register */
  status = Ctrl_ReadRegister(TunerLockStatusPtr->I2cSlaveAddr, 
                             RF_LOCK_STATUS_REG, 
                             &data, 
                             TunerLockStatusPtr->PrivateDataPtr);

  if (status == MXL_TRUE)
  {
    /* Extract REF_SYNTH_LOCK status */
    if ((data & 0x03) == 0x03) TunerLockStatusPtr->RefSynthLock = MXL_LOCKED;
    else TunerLockStatusPtr->RefSynthLock = MXL_UNLOCKED;

    /* Extract RF_SYNTH_LOCK status */
    if ((data & 0x0C) == 0x0C) TunerLockStatusPtr->RfSynthLock = MXL_LOCKED;
    else TunerLockStatusPtr->RfSynthLock = MXL_UNLOCKED;
  }

  return status;       
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerSetIFOutputFreq
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : In Tuner only mode of MxL101SF, this function configures.
--|                 Support for IF Frequency in integer format.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerSetIFOutputFreq(PMXL_TUNER_IF_FREQ_T IfFreqParamPtr)
{
  UINT16 IFFCW;
  UINT8 status;
  UINT8 control;
  
  /* Configure IF polarity */ 
  if (IfFreqParamPtr->IF_Polarity == TUNER_NORMAL_IF_SPECTRUM) 
    control = TUNER_NORMAL_IF_SPECTRUM;
  else 
    control = TUNER_INVERT_IF_SPECTRUM;

  /* Configure predefined IF frequency */
  if (IfFreqParamPtr->IF_Index <= IF_OTHER_35MHZ_45MHZ) 
    control |= (UINT8)IfFreqParamPtr->IF_Index;
  
  status = Ctrl_WriteRegister(IfFreqParamPtr->I2cSlaveAddr, 
                              TUNER_IF_SEL_REG, 
                              control, 
                              IfFreqParamPtr->PrivateDataPtr);

  /* Configure custom IF frequency */
  if ((IfFreqParamPtr->IF_Index == IF_OTHER_12MHZ) || 
      (IfFreqParamPtr->IF_Index == IF_OTHER_35MHZ_45MHZ)) 
  {
    if (IfFreqParamPtr->IF_Index == IF_OTHER_12MHZ) 
      IfFreqParamPtr->IF_Freq = (IfFreqParamPtr->IF_Freq/108) * 4096;
    else
      IfFreqParamPtr->IF_Freq = (IfFreqParamPtr->IF_Freq/216) * 4096;

    control = 0x08;
    IfFreqParamPtr->IF_Freq += 500000;
    IfFreqParamPtr->IF_Freq /= 1000000;
    IFFCW = (UINT16)(IfFreqParamPtr->IF_Freq);  
  }
  else
  {
    control = 0;
    IFFCW = 0;
  }

  control |= (IFFCW >> 8);
  status |= Ctrl_WriteRegister(IfFreqParamPtr->I2cSlaveAddr, 
                               TUNER_IF_FCW_BYP_REG, 
                               control, 
                               IfFreqParamPtr->PrivateDataPtr);

  control = IFFCW & 0x00FF;
  status |= Ctrl_WriteRegister(IfFreqParamPtr->I2cSlaveAddr, 
                               TUNER_IF_FCW_REG, 
                               control, 
                               IfFreqParamPtr->PrivateDataPtr);

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerConfigLoopThru
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices
--|
--| DESCRIPTION   : If loop through mode is enabled, RF signal from the antenna
--|                 is looped through to an external demodulator.
--|                  0 : Disable, 1: Enable, 
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerConfigLoopThru(UINT8 I2CSlaveAddr, 
                                            MXL_BOOL EnableDisable, 
                                            void *PrivateDataPtr)
{
  /* Configure loop thru control feature */
  return Ctrl_WriteRegister(I2CSlaveAddr, 
                            TUNER_LOOP_THRU_CTRL_REG, 
                            (UINT8)EnableDisable, 
                            PrivateDataPtr);
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevConfigXtalSelect
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : Select XTAL frequency of CLK out 
--|                 4 : 24MHz, 8 : 28.8MHz, 7 : 27 MHz, 12 : 48 MHz
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevConfigXtalSelect(UINT8 I2CSlaveAddr, 
                                            MXL_XTAL_FREQ_E XtalFreq, 
                                            void *PrivateDataPtr)
{
  UINT8 status = MXL_TRUE;
  
  /* Configure XTAL frequency */
  status = Ctrl_WriteRegister(I2CSlaveAddr, 
                              DIG_CLK_FREQ_SEL_REG, 
                              (UINT8)XtalFreq, 
                              PrivateDataPtr);

  status |= Ctrl_WriteRegister(I2CSlaveAddr, 
                               DIG_RFREFSELECT_REG, 
                               ((UINT8)XtalFreq)|0xA0, 
                               PrivateDataPtr);

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevConfigXtalClkOutGain
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : If Xtal Clock out is enabled, then a valid clk out gain value
--|                 should be programmed to the chip.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevConfigXtalClkOutGain(UINT8 I2CSlaveAddr, 
                                                MXL_XTAL_CLK_OUT_GAIN_E ClkOutGain, 
                                                void *PrivateDataPtr)
{
  /* Configure XTAL clock out gain value */
  if (ClkOutGain > CLK_OUT_18_75dB) ClkOutGain = CLK_OUT_18_75dB;

  return Ctrl_WriteRegister(I2CSlaveAddr, 
                            XTAL_CLK_OUT_GAIN_REG, 
                            (UINT8)ClkOutGain, 
                            PrivateDataPtr);
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevConfigXtalClkOutControl
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : XTAL Clock out control
--|                  0 : Disable, 1: Enable,
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevConfigXtalClkOutControl(UINT8 I2CSlaveAddr, 
                                                   MXL_BOOL EnableDisable, 
                                                   void *PrivateDataPtr)
{
  UINT8 status = MXL_TRUE;
  UINT8 control;

  status = Ctrl_ReadRegister(I2CSlaveAddr, 
                             DIG_XTAL_ENABLE_REG, 
                             &control, 
                             PrivateDataPtr);

  /* Configure XTAL clocl out feature */
  if (EnableDisable == MXL_DISABLE) control &= 0xDF; 
  else control |= 0x20;             

  status |= Ctrl_WriteRegister(I2CSlaveAddr, 
                               DIG_XTAL_ENABLE_REG, 
                               control, 
                               PrivateDataPtr);

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevConfigXtalBiasControl
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : 0 : 200uA, 1 : 575 uA, ...
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevConfigXtalBiasControl(UINT8 I2CSlaveAddr, 
                                                 MXL_XTAL_BIAS_E XtalBias, 
                                                 void *PrivateDataPtr)
{
  MXL_STATUS status = MXL_TRUE;
  UINT8 control;

  if (XtalBias > I894_2uA) XtalBias = I894_2uA;

  /* Configure XTAL bias value */
  control = (UINT8)XtalBias | 0x10; 
  status = Ctrl_WriteRegister(I2CSlaveAddr, 
                              DIG_XTAL_BIAS_REG, 
                              control, 
                              PrivateDataPtr);

  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevConfigXtalCapControl
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 7/7/2008
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : XTAL Clock Cap control
--|                 0 : 10pF, 1 : 1 pF, ...
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevConfigXtalCapControl(UINT8 I2cSlaveAddr, 
                                                UINT8 XtalCapacitor, 
                                                void *PrivateDataPtr)
{
  MXL_STATUS status = MXL_FALSE;

  /* Configure XTAL capacitance value */
  if ((XtalCapacitor < 26) || (XtalCapacitor == 0x3F)) 
    status = Ctrl_WriteRegister(I2cSlaveAddr, 
                                XTAL_CAP_REG, 
                                XtalCapacitor, 
                                PrivateDataPtr);

  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevConfigXtalSettings
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 8/4/2009
--|                 3/31/2010 - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This API shall configure XTAL & loop thru settings 
--|                 for MxL101SF.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevConfigXtalSettings(PMXL_XTAL_CFG_T XtalCfgPtr)
{
  UINT8 status;
  
  /* Configure loop thru feature */
  status = MxL101SF_API_TunerConfigLoopThru(XtalCfgPtr->I2cSlaveAddr, 
                                            XtalCfgPtr->LoopThruEnable, 
                                            XtalCfgPtr->PrivateDataPtr);

  /* Configure XTAL frequency */
  status |= MxL101SF_API_DevConfigXtalSelect(XtalCfgPtr->I2cSlaveAddr, 
                                             XtalCfgPtr->XtalFreq, 
                                             XtalCfgPtr->PrivateDataPtr);

  /* Configure XTAL clock out gain */
  status |= MxL101SF_API_DevConfigXtalClkOutGain(XtalCfgPtr->I2cSlaveAddr, 
                                                 XtalCfgPtr->XtalClkOutGain, 
                                                 XtalCfgPtr->PrivateDataPtr);

  /* Enabel or Disable XTAL clock out module of MxL101SF */
  status |= MxL101SF_API_DevConfigXtalClkOutControl(XtalCfgPtr->I2cSlaveAddr, 
                                                    XtalCfgPtr->XtalClkOutEnable, 
                                                    XtalCfgPtr->PrivateDataPtr);

  /* Configure XTAL bias value */
  status |= MxL101SF_API_DevConfigXtalBiasControl(XtalCfgPtr->I2cSlaveAddr, 
                                                  XtalCfgPtr->XtalBiasCurrent, 
                                                  XtalCfgPtr->PrivateDataPtr);

  /* Configure XTAL capacitance value */
  status |= MxL101SF_API_DevConfigXtalCapControl(XtalCfgPtr->I2cSlaveAddr, 
                                                 XtalCfgPtr->XtalCap, 
                                                 XtalCfgPtr->PrivateDataPtr);

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerGetSignalStrength
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 10/20/2009
--|                 3/31/2010 - Support for multiple MxL101SF devices & applying
--|                             adjustment value based of tuned frequency.
--|
--| DESCRIPTION   : This API shall give MxL101SF's RF Signal Strength in dBm unit        
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerGetSignalStrength(PMXL_SIGNAL_STATS_T SigQualityPtr)
{
  SINT16 rxPwr = 0;
  UINT8 status;
  UINT8 regData;
  
  status = Ctrl_WriteRegister(SigQualityPtr->I2cSlaveAddr, 
                              MXL_PAGE_REG, 
                              MXL_REG_PAGE_2, 
                              SigQualityPtr->PrivateDataPtr);
  
  /* Read <7:0> bits of signal strength data */
  status |= Ctrl_ReadRegister(SigQualityPtr->I2cSlaveAddr, 
                              DIG_RF_PWR_LSB_REG, 
                              &regData, 
                              SigQualityPtr->PrivateDataPtr);
  rxPwr = regData;

  /* Read <10:8> bits of signal strength data */
  status |= Ctrl_ReadRegister(SigQualityPtr->I2cSlaveAddr, 
                              DIG_RF_PWR_MSB_REG, 
                              &regData, 
                              SigQualityPtr->PrivateDataPtr);

  status |= Ctrl_WriteRegister(SigQualityPtr->I2cSlaveAddr, 
                             MXL_PAGE_REG, 
                             MXL_REG_PAGE_0, 
                             SigQualityPtr->PrivateDataPtr);

  rxPwr |= (regData & 0x07) << 8;
  SigQualityPtr->Frequency = SigQualityPtr->Frequency/1000000;
  
  /* Apply signal strength adjustment value based of current frequency tuned */
  if (SigQualityPtr->Frequency <= 131) rxPwr -= 4;
  else if (SigQualityPtr->Frequency <= 143) rxPwr += 40;
  else if (SigQualityPtr->Frequency <= 296) rxPwr += 36;
  else if (SigQualityPtr->Frequency <= 308) rxPwr += 41;
  else if (SigQualityPtr->Frequency <= 320) rxPwr += 44;
  else if (SigQualityPtr->Frequency <= 332) rxPwr += 52;
  else if (SigQualityPtr->Frequency <= 422) rxPwr += 39;       
  else if (SigQualityPtr->Frequency <= 506) rxPwr += 33;
  else if (SigQualityPtr->Frequency <= 566) rxPwr += 25;
  else if (SigQualityPtr->Frequency <= 650) rxPwr += 20;        
  else if (SigQualityPtr->Frequency <= 800) rxPwr += 14;
  else if (SigQualityPtr->Frequency <= 860) rxPwr += 21;            
  else if (SigQualityPtr->Frequency > 860) rxPwr += 29;

  /* Calculate signal strength value */
  SigQualityPtr->SignalStrength = (rxPwr/8) - 119;

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevConfigCableSettings
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 11/05/2009
--|                 3/31/2010 - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This API shall configure DVBC settings on MxL101SF.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevConfigCableSettings(PMXL_CABLE_CFG_T CableCfgPtr)
{
  /* Configure cable settings */
  return Ctrl_ProgramRegisters(CableCfgPtr->I2cSlaveAddr, 
                               MxL_CableSettings, 
                               CableCfgPtr->PrivateDataPtr);
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DevConfigGPOPins
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 10/02/2009
--|                 3/31/2010 - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : API to confugure GPIO Pins of MxL101SF.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DevConfigGPOPins(PMXL_DEV_GPO_CFG_T GpoPinCfgPtr)
{
  UINT8 status = MXL_FALSE;
  UINT8 regData = 0;
  UINT8 gpoMask = 0;

  status = Ctrl_ReadRegister(GpoPinCfgPtr->I2cSlaveAddr, 
                             GPO_CTRL_REG, 
                             &regData, 
                             GpoPinCfgPtr->PrivateDataPtr); 
  
  switch(GpoPinCfgPtr->GpoPinId)
  {
    default:
    case MXL_GPO_0:
      gpoMask = GPO_0_MASK;
      break;

    case MXL_GPO_1:
      gpoMask = GPO_1_MASK;
      break;
  }

  /* Configure GPO settings */
  if (GpoPinCfgPtr->GpoPinCtrl) regData |= gpoMask;
  else regData &= ~gpoMask;

  status |= Ctrl_WriteRegister(GpoPinCfgPtr->I2cSlaveAddr, 
                               GPO_CTRL_REG, 
                               regData, 
                               GpoPinCfgPtr->PrivateDataPtr);  
  
  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetCellId
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 11/19/2009
--|                 3/31/2010 - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This API returns TPS cell id value captured by demod.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetCellId(PMXL_DEMOD_CELL_ID_INFO_T TpsCellIdPtr)
{
  UINT8 status;
  UINT8 regData;
  
  status = Ctrl_WriteRegister(TpsCellIdPtr->I2cSlaveAddr, 
                              MXL_PAGE_REG, 
                              MXL_REG_PAGE_2, 
                              TpsCellIdPtr->PrivateDataPtr);
  
  /* Read <7:0> bits of TPS cell id value */
  status |= Ctrl_ReadRegister(TpsCellIdPtr->I2cSlaveAddr, 
                              TPS_CELL_ID_LSB_REG, 
                              &regData, 
                              TpsCellIdPtr->PrivateDataPtr);

  TpsCellIdPtr->TpsCellId = regData;

  /* Read <15:8> bits of TPS cell id value */
  status |= Ctrl_ReadRegister(TpsCellIdPtr->I2cSlaveAddr, 
                              TPS_CELL_ID_MSB_REG, 
                              &regData, 
                              TpsCellIdPtr->PrivateDataPtr);

  TpsCellIdPtr->TpsCellId |= regData << 8;

  status |= Ctrl_WriteRegister(TpsCellIdPtr->I2cSlaveAddr, 
                               MXL_PAGE_REG, 
                               MXL_REG_PAGE_0, 
                               TpsCellIdPtr->PrivateDataPtr);

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetAGCLock
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 1/15/2010
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function return AGC_LOCK status bit information.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetAGCLock(PMXL_DEMOD_LOCK_STATUS_T AgcLockPtr)
{
  MXL_STATUS status;
  UINT8 rawData;
  
  /* Read AGC lock status */
  status = Ctrl_ReadRegister(AgcLockPtr->I2cSlaveAddr, 
                             AGC_LOCK_REG, 
                             &rawData, 
                             AgcLockPtr->PrivateDataPtr);

  if (status == MXL_TRUE)
  {
    if((rawData & AGC_LOCK_MASK) >> 5) AgcLockPtr->Status = MXL_LOCKED;
    else AgcLockPtr->Status = MXL_UNLOCKED;
  }

  return status;
}
/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodGetFECLock
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 1/15/2010
--|                 3/31/2010 - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function return FEC_LOCK status bit information.
--|                 if set to 1, FEC_LOCK
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodGetFECLock(PMXL_DEMOD_LOCK_STATUS_T FecLockPtr)
{
  MXL_STATUS status;
  UINT8 rawData;
  
  /* Read FEC lock status */
  status = Ctrl_ReadRegister(FecLockPtr->I2cSlaveAddr, 
                             IRQ_STATUS_REG, 
                             &rawData, 
                             FecLockPtr->PrivateDataPtr);

  if (status == MXL_TRUE)
  {
    if((rawData & IRQ_MASK_FEC_LOCK) >> 4) FecLockPtr->Status = MXL_LOCKED;
    else FecLockPtr->Status = MXL_UNLOCKED;
  }

  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_DemodConfigSpectrum
--| 
--| AUTHOR        : Bob Yu
--|
--| DATE CREATED  : 4/19/2010
--|                 3/31/2010 - Support for multiple MxL101SF devices.
--|                 7/05/2010 - Configure PLL for frequency offset compensation
--|
--| DESCRIPTION   : This function will configures MxL101SF to process inverted
--|                 RF signal.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_DemodConfigSpectrum(PMXL_DEMOD_SPECTRUM_CFG_T SpectrumCfgPtr)
{
  UINT8 status;
  UINT8 specRawData;
  UINT8 pllRawData;
  
  
  /* Configure demod spectrum inversion setting */  
  if(SpectrumCfgPtr->SpectrumCfg == MXL_ENABLE)
  {
    specRawData = 0x00;
	pllRawData = 0x00;
  }
  else
  {
	specRawData = 0x01;
	pllRawData = 0x10;
  }

  status = Ctrl_WriteRegister(SpectrumCfgPtr->I2cSlaveAddr, 
                              SPECTRUM_CTRL_REG, 
                              specRawData, 
                              SpectrumCfgPtr->PrivateDataPtr);

  status |= Ctrl_WriteRegister(SpectrumCfgPtr->I2cSlaveAddr, 
								PLL_CTRL_REG, 
								pllRawData, 
								SpectrumCfgPtr->PrivateDataPtr);

  
  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerGetChannelOffset
--| 
--| AUTHOR        : Brenndon Lee, Mahendra Kondur
--|
--| DATE CREATED  : 2/22/2010
--|                 3/31/2010(MK) - Support for multiple MxL101SF devices.
--|
--| DESCRIPTION   : This function returns frequency offset for a channel. 
--|                 For valid Frequency offset, FEC lock has to be achieved.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerGetChannelOffset(PMXL_TUNER_CHAN_OFFSET_T ChanOffsetPtr)
{
  SINT16 freqOffset;
  UINT8 status;
  UINT8 rawData;
  
  status = Ctrl_WriteRegister(ChanOffsetPtr->I2cSlaveAddr, 
                              MXL_PAGE_REG, 
                              MXL_REG_PAGE_2, 
                              ChanOffsetPtr->PrivateDataPtr); 

  /* Read 0x9B registe first, otherwise 
     FREQ_OFFSET_LSB_REG/FREQ_OFFSET_LSB_REG will return 0 */
  status |= Ctrl_ReadRegister(ChanOffsetPtr->I2cSlaveAddr, 
                              0x9B, 
                              &rawData, 
                              ChanOffsetPtr->PrivateDataPtr); 

  /* Read FREQ_OFFSET_LSB_REG data */
  status |= Ctrl_ReadRegister(ChanOffsetPtr->I2cSlaveAddr, 
                              FREQ_OFFSET_LSB_REG, 
                              &rawData, 
                              ChanOffsetPtr->PrivateDataPtr);
  
  freqOffset = rawData;

  /* Read FREQ_OFFSET_LSB_REG data */
  status |= Ctrl_ReadRegister(ChanOffsetPtr->I2cSlaveAddr, 
                              FREQ_OFFSET_MSB_REG, 
                              &rawData, 
                              ChanOffsetPtr->PrivateDataPtr);

  status |= Ctrl_WriteRegister(ChanOffsetPtr->I2cSlaveAddr, 
                               MXL_PAGE_REG, 
                               MXL_REG_PAGE_0, 
                               ChanOffsetPtr->PrivateDataPtr); 

  freqOffset |= (rawData << 8);
  freqOffset &= 0x3FF;

  /* Check the sign bit, if negative number, extend sign bit */
  if (freqOffset & 0x0200) freqOffset |= 0xFC00;
  
  /* Calculate frequency offset value in Hz */
  ChanOffsetPtr->ChanOffset = (freqOffset * 2048) + 2048;

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxL101SF_API_TunerCheckChannel
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 02/22/2010
--|                 3/31/2010 - Support for multiple MxL101SF devices.
--|                 05/01/2010 - Removed checking for FEC lock status.
--|                 6/3/2010 - Code optimization.
--|                 8/9/2010 - Support for Extended Chan Tune function.
--|                 1/9/2012(MM) - New programming guide
--|
--| DESCRIPTION   : This API will check for presence of singnal at different lock
--|                 levels (i.e CP, TPS, RS) for a given frequency.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxL101SF_API_TunerCheckChannel(PMXL_TUNER_CHECK_CHAN_REQ_T ChanScanCtrlPtr)
{
  UINT32 chScanStartTime;
  UINT32 timeNow = 0;
  UINT16 agcGainRb = 0;
  UINT32 timeout;
  UINT8 status;
  UINT8 cpLockStatus = 0, tpsLockStatus = 0, fecLockStatus = 0;
  UINT8 i2cSlvAddr = ChanScanCtrlPtr->I2cSlaveAddr;
  void *privDataPtr = ChanScanCtrlPtr->PrivateDataPtr;  
  
  ChanScanCtrlPtr->ChanTuneCfg.I2cSlaveAddr = ChanScanCtrlPtr->I2cSlaveAddr;
  ChanScanCtrlPtr->ChanTuneCfg.PrivateDataPtr = ChanScanCtrlPtr->PrivateDataPtr;

  /* Tune channel RF */
  if (ChanScanCtrlPtr->UseExtendedChanTune)
    status = MxL101SF_API_TunerChanTuneExt(&ChanScanCtrlPtr->ChanTuneCfg);
  else
    status = MxL101SF_API_TunerChanTune(&ChanScanCtrlPtr->ChanTuneCfg);

  /* Record timestamps */
  Ctrl_GetTime(&timeNow);

  chScanStartTime = timeNow;

  /* Reset IRQ status */
  Ctrl_ProgramRegisters(i2cSlvAddr, MxL_IrqClear, privDataPtr);
  
  ChanScanCtrlPtr->ChanPresent = MXL_FALSE;

  if (ChanScanCtrlPtr->ChanTuneCfg.TpsCellIdRbCtrl == MXL_DISABLE)
  {
    // If Cell id disabled, wait for certain time before AGC settling
    // If Cell id enabled, this is not necessary because that time was already spend in TunerChanTune
    Ctrl_Sleep(50);
    Ctrl_ReadAgcGain(i2cSlvAddr, privDataPtr, &agcGainRb);
  }

  // If cell_id enabled -> Read CP lock just once, because we already spent whole timeout in TunerChanTune
  // If cell_id disabled -> Wait for CP lock requied timeout dependent on agc_gain 

  timeout = chScanStartTime + ((agcGainRb <= AGC_NO_CHANNEL_THRESHOLD)?CH_SCAN_CP_LOCK_TO3:CH_SCAN_CP_LOCK_TO1);
  do
  {
    Ctrl_Sleep(1);
    status |= Ctrl_ReadRegister(i2cSlvAddr, CP_LOCK_REG, &cpLockStatus, privDataPtr);
    cpLockStatus = (cpLockStatus & CP_LOCK_DET_MASK) >> 2;
    Ctrl_GetTime(&timeNow);
  } while ((timeNow < timeout) && (cpLockStatus == 0) && (ChanScanCtrlPtr->ChanTuneCfg.TpsCellIdRbCtrl == MXL_DISABLE) && (status == MXL_TRUE));

  if (ChanScanCtrlPtr->ChanScanCtrl != MXL_BREAK_AT_CP_LOCK)
  {
    if (cpLockStatus)
    {
      if (ChanScanCtrlPtr->ChanTuneCfg.TpsCellIdRbCtrl == MXL_ENABLE)
      {
        // Read AGC gain only if cell id enabled. If cell id disabled, AGC gain was already read before
        Ctrl_ReadAgcGain(i2cSlvAddr, privDataPtr, &agcGainRb);
        timeout = chScanStartTime + ((agcGainRb <= AGC_NO_CHANNEL_THRESHOLD)?CH_SCAN_TPS_LOCK_TO4:CH_SCAN_TPS_LOCK_TO2);
      }        
      else
        timeout = chScanStartTime + ((agcGainRb <= AGC_NO_CHANNEL_THRESHOLD)?CH_SCAN_TPS_LOCK_TO3:CH_SCAN_TPS_LOCK_TO1);

      // Wait for TPS lock
      do
      {
        Ctrl_Sleep(1);
        status |= Ctrl_ReadRegister(i2cSlvAddr, TPS_LOCK_REG, &tpsLockStatus, privDataPtr);
        tpsLockStatus = (tpsLockStatus & PARAM_TPS_LOCK_MASK) >> 6;
        Ctrl_GetTime(&timeNow);
      } while ((timeNow < timeout) && (tpsLockStatus == 0) && (status == MXL_TRUE));

      if (ChanScanCtrlPtr->ChanScanCtrl != MXL_BREAK_AT_TPS_LOCK)
      {
        // Read FEC lock status
  
        timeout = chScanStartTime + CH_SCAN_RS_LOCK_TO;
  
        do
        {
          Ctrl_Sleep(1);
          status |= Ctrl_ReadRegister(i2cSlvAddr, RS_LOCK_REG, &fecLockStatus, privDataPtr);
          fecLockStatus = (fecLockStatus & RS_LOCK_DET_MASK) >> 3;
          Ctrl_GetTime(&timeNow);
        } while ((timeNow < timeout) && (fecLockStatus == 0) && (status == MXL_TRUE));

        if (fecLockStatus)
        {
          ChanScanCtrlPtr->ChanPresent = MXL_TRUE;
        }

      }
      else
      {
        ChanScanCtrlPtr->ChanPresent = (tpsLockStatus)?MXL_TRUE:MXL_FALSE;
      }
    }
  }
  else
  {
     /* Break channel scan process, if interested to check only CP Lock */
    ChanScanCtrlPtr->ChanPresent = (cpLockStatus)?MXL_TRUE:MXL_FALSE;
  }

  return (MXL_STATUS)status;
}
/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_ConfigDevice
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 08/08/2009
--|
--| DESCRIPTION   : The general device configuration shall be handled 
--|                 through this API.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_ConfigDevice(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;

  switch (CmdType)
  {
    case MXL_DEV_SOFT_RESET_CFG:
      status = MxL101SF_API_DevSoftReset((PMXL_RESET_CFG_T)ParamPtr);
      break;

    case MXL_DEV_OPERATIONAL_MODE_CFG:
      status = MxL101SF_API_DevConfigMxLOperationalMode((PMXL_DEV_MODE_CFG_T)ParamPtr);
      break;

    case MXL_DEV_XTAL_SETTINGS_CFG:
      status = MxL101SF_API_DevConfigXtalSettings((PMXL_XTAL_CFG_T)ParamPtr);
      break;

    case MXL_DEV_101SF_OVERWRITE_DEFAULTS_CFG:
      status = MxL101SF_API_DevInitTunerDemod((PMXL_OVERWRITE_DEFAULT_CFG_T)ParamPtr);
      break;

    case MXL_DEV_101SF_POWER_MODE_CFG:
      status = MxL101SF_API_DevConfigPowerSavingMode((PMXL_PWR_MODE_CFG_T)ParamPtr);
      break;

    case MXL_DEV_MPEG_OUT_CFG:
      status = MxL101SF_API_DevConfigMpegOut((PMXL_MPEG_CFG_T)ParamPtr);
      break;
    
    case MXL_DEV_TS_CTRL_CFG:
      status = MxL101SF_API_DevControlMpegOut((PMXL_TS_CTRL_T)ParamPtr);
      break;

    case MXL_DEV_GPO_PINS_CFG:
      status = MxL101SF_API_DevConfigGPOPins((PMXL_DEV_GPO_CFG_T)ParamPtr);
      break;

    case MXL_DEV_CABLE_CFG:
      status = MxL101SF_API_DevConfigCableSettings((PMXL_CABLE_CFG_T)ParamPtr);
      break;

    default:

      break;
  }

  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_GetDeviceStatus
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 08/08/2009
--|
--| DESCRIPTION   : The general device inquiries shall be handled
--|                 through this API.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_GetDeviceStatus(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;

  switch (CmdType)
  {
    case MXL_DEV_ID_VERSION_REQ:
      status = MxL101SF_API_DevGetChipInfo((PMXL_DEV_INFO_T)ParamPtr);
      break;

    default:
      break;
  }

  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_GetDemodStatus
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 08/08/2009
--|
--| DESCRIPTION   : The demod specific inquiries shall be handled 
--|                 through this API.
--|                 - Support for MXL_DEMOD_TPS_HIERARCHICAL_ALPHA_REQ
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_GetDemodStatus(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;

  switch (CmdType)
  {
    case MXL_DEMOD_SNR_REQ:
      status = MxL101SF_API_DemodGetSNR((PMXL_DEMOD_SNR_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_BER_REQ:
      status = MxL101SF_API_DemodGetBER((PMXL_DEMOD_BER_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_CODE_RATE_REQ:
      status = MxL101SF_API_DemodGetTpsCodeRate((PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_HIERARCHY_REQ:
      status = MxL101SF_API_DemodGetTpsHierarchy((PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;
      
    case MXL_DEMOD_TPS_CONSTELLATION_REQ:
      status = MxL101SF_API_DemodGetTpsConstellation((PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;
    
    case MXL_DEMOD_TS_PRIORITY_REQ:
      status = MxL101SF_API_DemodGetStreamPriority((PMXL_DEMOD_TS_PRIORITY_CFG_T) ParamPtr);
      break;

    case MXL_DEMOD_TPS_FFT_MODE_REQ:
      status = MxL101SF_API_DemodGetTpsFftMode((PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;
  
    case MXL_DEMOD_TPS_HIERARCHICAL_ALPHA_REQ:
      status = MxL101SF_API_DemodGetHierarchicalAlphaValue((PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_GUARD_INTERVAL_REQ:
      status = MxL101SF_API_DemodGetTpsGuardInterval((PMXL_DEMOD_TPS_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_LOCK_REQ:
      status = MxL101SF_API_DemodGetTpsLock((PMXL_DEMOD_LOCK_STATUS_T)ParamPtr);
      break;

    case MXL_DEMOD_TPS_CELL_ID_REQ:
      status = MxL101SF_API_DemodGetCellId((PMXL_DEMOD_CELL_ID_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_PACKET_ERROR_COUNT_REQ:
      status = MxL101SF_API_DemodGetPacketErrorCount((PMXL_DEMOD_PEC_INFO_T)ParamPtr);
      break;

    case MXL_DEMOD_SYNC_LOCK_REQ:
      status = MxL101SF_API_DemodGetSyncLockStatus((PMXL_DEMOD_LOCK_STATUS_T)ParamPtr);
      break;
      
    case MXL_DEMOD_RS_LOCK_REQ:
      status = MxL101SF_API_DemodGetRsLockStatus((PMXL_DEMOD_LOCK_STATUS_T)ParamPtr);
      break;

    case MXL_DEMOD_CP_LOCK_REQ:
      status = MxL101SF_API_DemodGetCpLockStatus((PMXL_DEMOD_LOCK_STATUS_T)ParamPtr);
      break;

    case MXL_DEMOD_FEC_LOCK_REQ:
      status = MxL101SF_API_DemodGetFECLock((PMXL_DEMOD_LOCK_STATUS_T)ParamPtr);
      break;

    default:
      break;
  }

  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_ConfigDemod
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 08/08/2009
--|                 03/12/2010
--|                 04/20/2010
--|
--| DESCRIPTION   : The demod block specific configuration shall be handled 
--|                 through this API.
--|                 - Support for MXL_DEMOD_STREAM_PRIORITY_CFG
--|                 - Support for MXL_DEMOD_SPECTRUM_CFG
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_ConfigDemod(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;

  switch (CmdType)
  {
    case MXL_DEMOD_RESET_IRQ_CFG:
      status = MxL101SF_API_DemodResetIrqStatus((PMXL_RESET_IRQ_CFG_T)ParamPtr);
      break;

    case MXL_DEMOD_RESET_PEC_CFG:  
      status = MxL101SF_API_DemodResetPacketErrorCount((PMXL_RESET_PEC_CFG_T)ParamPtr);
      break;

    case MXL_DEMOD_TS_PRIORITY_CFG:
      status = MxL101SF_API_DemodConfigStreamPriority(
                                                  (PMXL_DEMOD_TS_PRIORITY_CFG_T)ParamPtr);
      break;

    case MXL_DEMOD_SPECTRUM_CFG:
      status = MxL101SF_API_DemodConfigSpectrum((PMXL_DEMOD_SPECTRUM_CFG_T)ParamPtr);
      break;

    default:
      break;
  }

  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_ConfigTuner
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 08/08/2009
--|                 08/09/2010
--|
--| DESCRIPTION   : The tuner block specific configuration shall be handled 
--|                 through this API.
--|                 - Support for MXL_TUNER_CHAN_TUNE_EXT_CFG
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_ConfigTuner(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;
  
  switch (CmdType)
  {
    case MXL_TUNER_TOP_MASTER_CFG:
      status = MxL101SF_API_TunerConfigTopMaster((PMXL_TOP_MASTER_CFG_T)ParamPtr);
      break;

    case MXL_TUNER_CHAN_TUNE_CFG:
      status = MxL101SF_API_TunerChanTune((PMXL_RF_TUNE_CFG_T)ParamPtr);
      break;

    case MXL_TUNER_CHAN_TUNE_EXT_CFG:
      status = MxL101SF_API_TunerChanTuneExt((PMXL_RF_TUNE_CFG_T)ParamPtr);
      break;

    case MXL_TUNER_IF_OUTPUT_FREQ_CFG: 
      status = MxL101SF_API_TunerSetIFOutputFreq((PMXL_TUNER_IF_FREQ_T)ParamPtr);
      break;

				case MXL_TUNER_FREQ_OFFSET_SRCH_RANGE_CFG:
      status = MxL101SF_API_ConfigFreqOffsetSearchRange((PMXL_TUNER_FREQ_OFFSET_CFG_T)ParamPtr);
      break;

    default:
      break;
  }
    
  return status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare_API_GetTunerStatus
--| 
--| AUTHOR        : Mahendra Kondur
--|
--| DATE CREATED  : 08/08/2009
--|                 02/22/2010
--|
--| DESCRIPTION   : The tuner specific inquiries shall be handled 
--|                 through this API.
--|                 Support for MXL_TUNER_CHAN_OFFSET_REQ &
--|                 MXL_TUNER_CHECK_CHAN_REQ
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxLWare_API_GetTunerStatus(MXL_CMD_TYPE_E CmdType, void *ParamPtr)
{
  MXL_STATUS status = MXL_FALSE;

  switch (CmdType)
  {
    case MXL_TUNER_LOCK_STATUS_REQ:
      status = MxL101SF_API_TunerGetLockStatus((PMXL_TUNER_LOCK_STATUS_T)ParamPtr);
      break;

    case MXL_TUNER_SIGNAL_STRENGTH_REQ:
      status = MxL101SF_API_TunerGetSignalStrength((PMXL_SIGNAL_STATS_T)ParamPtr);
      break;

    case MXL_TUNER_CHAN_OFFSET_REQ:
      status = MxL101SF_API_TunerGetChannelOffset((PMXL_TUNER_CHAN_OFFSET_T)ParamPtr);
      break;      

   case MXL_TUNER_CHECK_CHAN_STATUS_REQ:
      status = MxL101SF_API_TunerCheckChannel((PMXL_TUNER_CHECK_CHAN_REQ_T)ParamPtr);
      break;

   default:
      break;
  }

  return status;
}

