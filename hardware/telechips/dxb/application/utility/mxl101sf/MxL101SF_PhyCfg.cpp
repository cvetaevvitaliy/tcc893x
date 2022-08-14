/*****************************************************************************************
 *
 * FILE NAME          : MxL101SF_PhyCfg.cpp
 * 
 * AUTHOR             : Brenndon Lee, Mahendra Kondur
 *
 * DATE CREATED       : 1/22/2008
 *                      6/3/2010 - Support for MP version silicon
 *
 * DESCRIPTION        : This file contains control parameters for only MxL101SF 
 *                             
 *****************************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ****************************************************************************************/

#include "MxL101SF_PhyDefs.h"

REG_CTRL_INFO_T MxL_OverwriteDefault[] = 
{
  {0x42, 0xFF, 0x33},  
  {0x71, 0xFF, 0x66},  
  {0x72, 0xFF, 0x01},  
  {0x73, 0xFF, 0x00},  
  {0x74, 0xFF, 0xC2},  
  {0x71, 0xFF, 0xE6},  
  {0x6F, 0xFF, 0xB7},  
  {0x98, 0xFF, 0x40},  
  {0x99, 0xFF, 0x18},
  {0xE0, 0xFF, 0x00},  
  {0xE1, 0xFF, 0x10},  
  {0xE2, 0xFF, 0x91},  
  {0xE4, 0xFF, 0x3F},  
  {0xB1, 0xFF, 0x00},  
  {0xB2, 0xFF, 0x09},  
  {0xB3, 0xFF, 0x1F},  
  {0xB4, 0xFF, 0x1F},  
  {0x00, 0xFF, 0x01},
  {0xE2, 0xFF, 0x02},
  {0x81, 0xFF, 0x04},  
  {0xF4, 0xFF, 0x07},  
  {0xB8, 0xFF, 0x42},  
  {0xBF, 0xFF, 0x1C},  
  {0x82, 0xFF, 0xC6},  
  {0x87, 0xFF, 0x01}, // added for optimization 
  {0x81, 0x1C, 0x04},  
  {0x83, 0xFF, 0xCF},  
  {0x85, 0xFF, 0x02},  
	
  {0x00, 0xFF, 0x02},
  {0x27, 0xFF, 0x20},
	{0x00, 0xFF, 0x00},
        
  {0, 0, 0}
};

REG_CTRL_INFO_T MxL_SuspendMode[] =
{
  {0x1C, 0xFF, 0x00},   //START_TUNE=0
  {0x04, 0xFF, 0x02},   //DRIVE_STRENGTH=2 (GO_SLEEP)
  {0x02, 0xFF, 0x00},   //SOFT_RST_N=0
  {0x01, 0xFF, 0x00},   //TOP_MASTER_ENABLE=0
  {0x06, 0xDF, 0x00},   //XTAL_EN_CLKOUT=0
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_WakeUp[] =
{
  {0x04, 0xFF, 0x00},
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_StandbyMode[] =
{
  {0x01, 0xFF, 0x00},   //TOP_MASTER_ENABLE=0
  {0x1C, 0xFF, 0x01},   //START_TUNE=1
  {0x1C, 0xFF, 0x00},   //START_TUNE=0
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_MpegTsON[] =
{
  {0x17, 0xC0, 0xC0},
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_MpegTsOFF[] =
{
  {0x17, 0xC0, 0x40},
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_PhySoftReset[] =     
{
  {0xFF, 0xFF, 0x00}, 
  {0x02, 0xFF, 0x01}, 
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_TunerDemodMode[] =
{
  {0x03, 0xFF, 0x01},  
  {0x7D, 0x40, 0x00},  
  {0x3B, 0x20, 0x20},
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_TunerMode[] =
{
  {0x03, 0xFF, 0x00}, 
  {0x7D, 0x40, 0x40},
  {0x3B, 0x20, 0x00},  
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_TopMasterEnable[] =
{
  {0x01, 0xFF, 0x01}, 
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_TopMasterDisable[] =
{
  {0x01, 0xFF, 0x00}, 
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_PhyTuneRF[] = 
{
  {0x1D, 0x7F, 0x00},  
  {0x1E, 0xFF, 0x00},  
  {0x1F, 0xFF, 0x00},  
  {0,    0,    0}
};

#if 0
REG_CTRL_INFO_T MxL_SetSpurFilt[] = 
{
  {0x00, 0xFF, 0x01},  
  {0x81, 0x1C, 0x04},  
  {0x83, 0xFF, 0xCF},  
  {0x85, 0xFF, 0x02},  
  {0x00, 0xFF, 0x00},  
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_ClearSpurFilt[] = 
{
  {0x00, 0xFF, 0x01},  
  {0x81, 0x1C, 0x00},  
  {0x00, 0xFF, 0x00},  
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_SetSpurNotch[] = 
{
  {0x00, 0xFF, 0x01},  
  {0x81, 0x1C, 0x00},  
  {0x83, 0xFF, 0xDF},  
  {0x85, 0xFF, 0x06},  
  {0x00, 0xFF, 0x00},  
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_SetSpurNotch2[] = 
{
  {0x00, 0xFF, 0x01},  
  {0x81, 0xFF, 0x04},  
  {0x83, 0xFF, 0xDF},  
  {0x85, 0xFF, 0x06},  
  {0x00, 0xFF, 0x00},  
  {0,    0,    0}
};
#endif

REG_CTRL_INFO_T MxL_SpurCoeff6MHz[] =
{
  {0x00, 0xFF, 0x01},  
//  {0xE2, 0xFF, 0x02}, // optimization
//  {0x81, 0xFF, 0x04},    
//  {0xF4, 0xFF, 0x07},  // optimization
//  {0xB8, 0xFF, 0x42},  // optimization
//  {0xBF, 0xFF, 0x1C},  // optimization
//  {0x82, 0xFF, 0xC6},  // optimization
  {0x84, 0xFF, 0xA1}, 
//  {0x85, 0xFF, 0x02},
  {0x86, 0xFF, 0x9B},
//  {0x87, 0xFF, 0x01},  // optimization
//  {0x00, 0xFF, 0x00},  // optimization  
  {0x00, 0xFF, 0x02}, 
  {0xDD, 0xFF, 0x14},
  {0xDE, 0xFF, 0x0D},
  {0x00, 0xFF, 0x00},

  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_SpurCoeff7MHz[] =
{
  {0x00, 0xFF, 0x01},   //Change to page 1  
//  {0xE2, 0xFF, 0x02},  // optimization
//  {0x81, 0xFF, 0x04},   
//  {0xF4, 0xFF, 0x07},  // optimization
//  {0xB8, 0xFF, 0x42},  // optimization
//  {0xBF, 0xFF, 0x1C},  // optimization
//  {0x82, 0xFF, 0xC6},  // optimization
  {0x84, 0xFF, 0x96}, 
//  {0x85, 0xFF, 0x02},
  {0x86, 0xFF, 0xA0},
//  {0x87, 0xFF, 0x01},  // optimization
//  {0x00, 0xFF, 0x00},  // optimization.
  {0x00, 0xFF, 0x02}, 
  {0xDD, 0xFF, 0x42},
  {0xDE, 0xFF, 0x0F},
  {0x00, 0xFF, 0x00},

  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_SpurCoeff8MHz[] =
{
  {0x00, 0xFF, 0x01},   //Change to page 1  
//  {0xE2, 0xFF, 0x02},  // optimization
//  {0x81, 0xFF, 0x04},  
//  {0xF4, 0xFF, 0x07},  // optimization
//  {0xB8, 0xFF, 0x42},  // optimization
//  {0xBF, 0xFF, 0x1C},  // optimization
//  {0x82, 0xFF, 0xC6},  // optimization
  {0x84, 0xFF, 0x8C}, 
//  {0x85, 0xFF, 0x02},
  {0x86, 0xFF, 0x6C},
//  {0x87, 0xFF, 0x01},  // optimization
//  {0x00, 0xFF, 0x00},  // optimization.
  {0x00, 0xFF, 0x02},
  {0xDD, 0xFF, 0x70},
  {0xDE, 0xFF, 0x11},
  {0x00, 0xFF, 0x00},
  
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_IrqClear[] =
{
  {0x0E, 0xFF, 0xFF}, 
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_ResetPerCount[] =
{
  {0x20, 0x01, 0x01}, 
  {0x20, 0x01, 0x00}, 
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_CableSettings[] =
{
  {0x0A, 0x10, 0x10}, 
  {0x0A, 0x0F, 0x04}, 
  {0x3F, 0x3F, 0x01}, 
  {0x44, 0xF0, 0x60}, 
  {0x46, 0xF0, 0x10}, 
  {0x48, 0xF0, 0x70}, 
  {0x48, 0x0F, 0x0C}, 
  {0x0D, 0x03, 0x02}, 
  {0x4D, 0xFF, 0x40}, 
  {0x69, 0x01, 0x01}, 
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_EnableCellId[] =
{
  {0x00, 0xFF, 0x01}, 
  {0x7E, 0xFF, 0x05}, 
  {0x00, 0xFF, 0x00}, 
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_DisableCellId[] =
{
  {0x00, 0xFF, 0x01}, 
  {0x7E, 0xFF, 0x04}, 
  {0x00, 0xFF, 0x00}, 
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_EnableExtTuneCfg[] =
{
  {0x00, 0xFF, 0x01}, 
  {0x6C, 0xFF, 0xAC}, 
  {0x00, 0xFF, 0x00}, 
  {0,    0,    0}
}; 

REG_CTRL_INFO_T MxL_DisableExtTuneCfg[] =
{
  {0x00, 0xFF, 0x01}, 
  {0x6C, 0xFF, 0x2C}, 
  {0x00, 0xFF, 0x00}, 
  {0,    0,    0}
}; 


/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_ProgramRegisters
--|
--| AUTHOR        : Brenndon Lee, Mahendra Kondur, Mariusz Murawski
--|
--| DATE CREATED  : 1/15/2008
--|                 3/31/2010(Mahee) - Support for multiple MxL101SF devices
--|                 12/27/2011(Mariusz) - bug fixed (tmp initialized to 0)
--|
--| DESCRIPTION   : This function writes multiple registers with provided data array.
--|
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS Ctrl_ProgramRegisters(UINT8 I2cSlaveAddr, 
                                 PREG_CTRL_INFO_T ctrlRegInfoPtr, 
                                 void *PrivateDataPtr)
{
  UINT8 status = MXL_TRUE;
  UINT16 i;
  UINT8 tmp = 0;
  
  for (i = 0; 
       ctrlRegInfoPtr[i].regAddr|ctrlRegInfoPtr[i].mask|ctrlRegInfoPtr[i].data; 
       i++)
  {
    /* Check if partial bits of register were updated */
    if (ctrlRegInfoPtr[i].mask != 0xFF)  
    {
      status = Ctrl_ReadRegister(I2cSlaveAddr, 
                                 ctrlRegInfoPtr[i].regAddr, 
                                 &tmp, 
                                 PrivateDataPtr);
      if (status != MXL_TRUE) break;;
    }
    
    tmp &= ~ctrlRegInfoPtr[i].mask;
    tmp |= ctrlRegInfoPtr[i].data;

    status |= Ctrl_WriteRegister(I2cSlaveAddr, 
                                 ctrlRegInfoPtr[i].regAddr, 
                                 tmp, 
                                 PrivateDataPtr);
    if (status != MXL_TRUE) break;
  }

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_PhyTune
--| 
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/15/2008
--|                 3/31/2010(Mahee) - Support for multiple MxL101SF devices
--|                 5/4/2010(Mahee) - Calculation for improved frequency resolution 
--|                 12/27/2010(Mariusz) - Function made re-entrant, tunerMode added 
--|
--| DESCRIPTION   : Tune to the specified RF frequency with bandwidth
--|                 
--|                 The provided input frequency and bandwidth's unit is MHz. 
--|                 Changed to increase frequency resolution.
--|
--| RETURN VALUE  : void, arrayPtr returned by reference
--|
--|-------------------------------------------------------------------------------------*/

void Ctrl_PhyTune(UINT32 freqInHz, UINT8 bwInMHz, MXL_DEV_MODE_E tunerMode, REG_CTRL_INFO_T * arrayPtr)
{
  UINT32 freq; 
  UINT8 filtBw;

  /* Set Channel Bandwidth */
  switch (bwInMHz)
  {
    case 6:
      filtBw = (tunerMode == MXL_DEV_TUNER_DEMOD_MODE)?21:69;
      break;

    case 7:
      filtBw = (tunerMode == MXL_DEV_TUNER_DEMOD_MODE)?42:90;
      break;

    default:
    case 8:
      filtBw = (tunerMode == MXL_DEV_TUNER_DEMOD_MODE)?63:111;
      break;
  }
  
  /* Calculate RF Channel */
  freq = (UINT16)(freqInHz/15625);
  arrayPtr[0].regAddr = MxL_PhyTuneRF[0].regAddr;
  arrayPtr[0].mask = MxL_PhyTuneRF[0].mask;
  arrayPtr[0].data = filtBw;

  arrayPtr[1].regAddr = MxL_PhyTuneRF[1].regAddr;
  arrayPtr[1].mask = MxL_PhyTuneRF[1].mask;
  arrayPtr[1].data = (freq & 0xFF);

  arrayPtr[2].regAddr = MxL_PhyTuneRF[2].regAddr;
  arrayPtr[2].mask = MxL_PhyTuneRF[2].mask;
  arrayPtr[2].data = (freq >> 8 ) & 0xFF; 

  arrayPtr[3].regAddr = 0;
  arrayPtr[3].mask = 0;
  arrayPtr[3].data = 0;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_ConfigSpurCoeff
--| 
--| AUTHOR        : Bob Yu
--|
--| DATE CREATED  : 10/28/2011
--|
--| DESCRIPTION   : Config spur coefficiency according to specified bandwidth
--|                 
--| RETURN VALUE  : MXL_STATUS
--|
--|-------------------------------------------------------------------------------------*/

MXL_STATUS Ctrl_ConfigSpurCoeff(PMXL_RF_TUNE_CFG_T ChanTuneParamPtr)
{
  UINT8 status;
	UINT8 bwInMHz = ChanTuneParamPtr->Bandwidth;
	UINT8 i2cSlvAddr = ChanTuneParamPtr->I2cSlaveAddr;
  void *privDataPtr = ChanTuneParamPtr->PrivateDataPtr;
  /* Set spur coefficiency according to Bandwidth */
  switch (bwInMHz)
  {
    case 6:
      status = Ctrl_ProgramRegisters(i2cSlvAddr, MxL_SpurCoeff6MHz, privDataPtr); 
      break;

    case 7:
      status = Ctrl_ProgramRegisters(i2cSlvAddr, MxL_SpurCoeff7MHz, privDataPtr);  
      break;

    case 8:
      status = Ctrl_ProgramRegisters(i2cSlvAddr, MxL_SpurCoeff8MHz, privDataPtr);  
      break;
  
    default:
	  status = MXL_FALSE;
      break;
  }

  return (MXL_STATUS)status;
}

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_ReadCurrentDeviceMode
--|    
--| AUTHOR        : Mariusz Murawski
--|
--| DATE CREATED  : 11/21/2011
--|
--| DESCRIPTION   : Read currently device mode
--|                 
--| RETURN VALUE  : MXL_DEV_TUNER_MODE, MXL_DEV_TUNER_DEMOD_MODE
--|
--|-------------------------------------------------------------------------------------*/
 
MXL_DEV_MODE_E Ctrl_ReadCurrentDeviceMode(UINT8 i2cSlvAddr, void * privDataPtr)
{  
  UINT8 value;
      
  Ctrl_ReadRegister(i2cSlvAddr, MXL_MODE_REG, &value, privDataPtr);
  value = ((value & MXL_DEV_MODE_MASK) == MXL_DEV_TUNER_DEMOD_MODE)?MXL_DEV_TUNER_DEMOD_MODE:MXL_DEV_TUNER_MODE;
 
  return (MXL_DEV_MODE_E) value;
}


MXL_STATUS Ctrl_ReadAgcGain(UINT8 i2cSlvAddr, void * privDataPtr, UINT16 * agcGainPtr)
{
  MXL_STATUS status;
  UINT8 regData = 0;
  UINT16 agcGainRb = 0;

  status = Ctrl_ReadRegister(i2cSlvAddr, AGC_GAIN_LSB_REG, &regData, privDataPtr);
  agcGainRb = regData;
  regData = 0;
  if (status == MXL_TRUE)
  {
	status = Ctrl_ReadRegister(i2cSlvAddr, AGC_GAIN_MSB_REG, &regData, privDataPtr);
	agcGainRb |= (regData & 0x1F) << 8;
  }

  if (agcGainPtr) *agcGainPtr = agcGainRb;

  return status;
}

