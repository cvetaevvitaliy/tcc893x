#include <stdio.h>
#include "MxL603_TunerApi.h"
#include "MxL603_OEM_Drv.h"

#define EXAMPLE_DEV_MAX 2
#define MXL603_I2C_ADDR 0x60

/* Example of OEM Data, customers should have
below data structure declared at their appropriate 
places as per their software design 

typedef struct
{
  UINT8   i2c_address;
  UINT8   i2c_bus;
  sem_type_t sem;
  UINT16  i2c_cnt;
} user_data_t;  

user_data_t device_context[EXAMPLE_DEV_MAX];

*/

int main(void)
{
  MXL_STATUS status; 
  UINT8 devId;
  MXL_BOOL singleSupply_3_3V;
  MXL603_XTAL_SET_CFG_T xtalCfg;
  MXL603_IF_OUT_CFG_T ifOutCfg;
  MXL603_AGC_CFG_T agcCfg;
  MXL603_TUNER_MODE_CFG_T tunerModeCfg;
  MXL603_CHAN_TUNE_CFG_T chanTuneCfg;
  MXL_BOOL refLockPtr = MXL_UNLOCKED;
  MXL_BOOL rfLockPtr = MXL_UNLOCKED;		
  MXL603_VER_INFO_T mxlDevVerInfoPtr;

/* If OEM data is implemented, customer needs to use OEM data structure  
   related operation. Following code should be used as a reference. 
   For more information refer to sections 2.5 & 2.6 of 
   MxL603_mxLWare_API_UserGuide document.

  for (devId = 0; devId < EXAMPLE_DEV_MAX; devId++)
  {
    // assigning i2c address for  the device
    device_context[devId].i2c_address = GET_FROM_FILE_I2C_ADDRESS(devId);     

    // assigning i2c bus for  the device
    device_context[devId].i2c_bus = GET_FROM_FILE_I2C_BUS(devId);                      

    // create semaphore if necessary
    device_context[devId].sem = CREATE_SEM();                                                           

    // sample stat counter
    device_context[devId].i2c_cnt = 0;                                                                               

    status = MxLWare603_API_CfgDrvInit(devId, (void *) &device_context[devId]);  

    // if you don’t want to pass any oem data, just use NULL as a parameter:
    // status = MxLWare603_API_CfgDrvInit(devId, NULL);  
  }

*/

  /* If OEM data is not required, customer should treat devId as 
     I2C slave Address */
  devId = MXL603_I2C_ADDR;
  Ctrl_I2cConnect(devId);
  //Step 1 : Soft Reset MxL603
  status = MxLWare603_API_CfgDevSoftReset(devId);
  if (status != MXL_SUCCESS)
  {
    printf("Error! MxLWare603_API_CfgDevSoftReset\n");    
  }
    
  //Step 2 : Overwrite Default
  singleSupply_3_3V = MXL_DISABLE;
  status = MxLWare603_API_CfgDevOverwriteDefaults(devId, singleSupply_3_3V);
  if (status != MXL_SUCCESS)
  {
    printf("Error! MxLWare603_API_CfgDevOverwriteDefaults\n");    
  }

  MxLWare603_API_ReqDevVersionInfo(devId ,&mxlDevVerInfoPtr);
  printf("ChipID : %d, ChipVersion : %d\n",mxlDevVerInfoPtr.chipId, mxlDevVerInfoPtr.chipVersion);

  //Step 3 : XTAL Setting
  xtalCfg.xtalFreqSel = MXL603_XTAL_16MHz;
  xtalCfg.xtalCap = 12;
  xtalCfg.clkOutEnable = MXL_ENABLE;
  xtalCfg.clkOutDiv = MXL_DISABLE;
  xtalCfg.clkOutExt = MXL_DISABLE;
  xtalCfg.singleSupply_3_3V = MXL_DISABLE;
  xtalCfg.XtalSharingMode = MXL_DISABLE;
  status = MxLWare603_API_CfgDevXtal(devId, xtalCfg);
  if (status != MXL_SUCCESS)
  {
    printf("Error! MxLWare603_API_CfgDevXtal\n");    
  }

  //Step 4 : IF Out setting
  ifOutCfg.ifOutFreq = MXL603_IF_4_1MHz;
  ifOutCfg.ifInversion = MXL_DISABLE;
  ifOutCfg.gainLevel = 11;
  ifOutCfg.manualFreqSet = MXL_DISABLE;
  ifOutCfg.manualIFOutFreqInKHz = 0;
  status = MxLWare603_API_CfgTunerIFOutParam(devId, ifOutCfg);
  if (status != MXL_SUCCESS)
  {
    printf("Error! MxLWare603_API_CfgTunerIFOutParam\n");    
  }

  //Step 5 : AGC Setting
  agcCfg.agcType = MXL603_AGC_EXTERNAL;
  agcCfg.setPoint = 66;
  agcCfg.agcPolarityInverstion = MXL_DISABLE;
  status = MxLWare603_API_CfgTunerAGC(devId, agcCfg);
  if (status != MXL_SUCCESS)
  {
    printf("Error! MxLWare603_API_CfgTunerAGC\n");    
  }

  //Step 6 : Application Mode setting
  tunerModeCfg.signalMode = MXL603_DIG_DVB_T_DTMB;
  tunerModeCfg.ifOutFreqinKHz = 4100;
  tunerModeCfg.xtalFreqSel = MXL603_XTAL_16MHz;
  tunerModeCfg.ifOutGainLevel = 11;
  status = MxLWare603_API_CfgTunerMode(devId, tunerModeCfg);
  if (status != MXL_SUCCESS)
  {
    printf("Error! MxLWare603_API_CfgTunerMode\n");    
  }

  //Step 7 : Channel frequency & bandwidth setting
  chanTuneCfg.bandWidth = MXL603_TERR_BW_8MHz;
  chanTuneCfg.freqInHz = 506000000;
  chanTuneCfg.signalMode = MXL603_DIG_DVB_T_DTMB;
  chanTuneCfg.xtalFreqSel = MXL603_XTAL_16MHz;
  chanTuneCfg.startTune = MXL_START_TUNE;
  status = MxLWare603_API_CfgTunerChanTune(devId, chanTuneCfg);
  if (status != MXL_SUCCESS)
  {
    printf("Error! MxLWare603_API_CfgTunerChanTune\n");    
  }

  // Wait 15 ms 
  MxLWare603_OEM_Sleep(15);

  // Read back Tuner lock status
  status = MxLWare603_API_ReqTunerLockStatus(devId, &rfLockPtr, &refLockPtr);
  if (status == MXL_TRUE)
  {
    if (MXL_LOCKED == rfLockPtr && MXL_LOCKED == refLockPtr)
    {
      printf("Tuner locked\n");
    }
    else
      printf("Tuner unlocked\n");
  }

  // To Change Channel, GOTO Step #7

  // To change Application mode settings, GOTO Step #6
  Ctrl_I2cDisConnect();
  return 0;
}
