#define LOG_TAG	"MXL603"
//#include <utils/Log.h>


//#include <stdio.h>
//#include <fcntl.h>
//#include <sys/time.h>
//#include <sys/mman.h>
//#include <string.h>
//#include <sys/ioctl.h>
#include <linux/kernel.h>
#include "MxL603_TunerApi.h"
#include "MxL603_OEM_Drv.h"
//#include "tcc_dxb_control.h"

#define SUPPORT_BOARD_MAX603_V1_1
#define EXAMPLE_DEV_MAX 2
#define MXL603_I2C_ADDR 0x60
#define PRINTF printk

//#define	BUILD_EXCUTABLE

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

static UINT8 g_devId = MXL603_I2C_ADDR;

#ifdef		BUILD_EXCUTABLE
#define DXB_CTRL_DEV_FILE		"/dev/tcc_dxb_ctrl"
static int giTccDXBCtrlFD = -1;
int Power_On(void)
{
    unsigned int deviceIdx;
    giTccDXBCtrlFD = open(DXB_CTRL_DEV_FILE, O_RDWR | O_NDELAY);
	if(giTccDXBCtrlFD<0)
	{
		PRINTF ("%s CANNOT open %s :: %d\n", __func__, DXB_CTRL_DEV_FILE, giTccDXBCtrlFD);
		return 1;
	}	

    deviceIdx = 0;
	ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_ON, &deviceIdx);
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

int main_test(void)
{
    MXL_STATUS status; 
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

       for (g_devId = 0; g_devId < EXAMPLE_DEV_MAX; g_devId++)
       {
    // assigning i2c address for  the device
    device_context[g_devId].i2c_address = GET_FROM_FILE_I2C_ADDRESS(g_devId);     

    // assigning i2c bus for  the device
    device_context[g_devId].i2c_bus = GET_FROM_FILE_I2C_BUS(g_devId);                      

    // create semaphore if necessary
    device_context[g_devId].sem = CREATE_SEM();                                                           

    // sample stat counter
    device_context[g_devId].i2c_cnt = 0;                                                                               

    status = MxLWare603_API_CfgDrvInit(g_devId, (void *) &device_context[g_devId]);  

    // if you don’t want to pass any oem data, just use NULL as a parameter:
    // status = MxLWare603_API_CfgDrvInit(g_devId, NULL);  
    }

*/

    /* If OEM data is not required, customer should treat g_devId as 
       I2C slave Address */
    g_devId = MXL603_I2C_ADDR;
    Ctrl_I2cConnect(g_devId);
    //Step 1 : Soft Reset MxL603
    status = MxLWare603_API_CfgDevSoftReset(g_devId);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgDevSoftReset\n");    
    }

    //Step 2 : Overwrite Default
#ifdef      SUPPORT_BOARD_MAX603_V1_1
    singleSupply_3_3V = MXL_ENABLE;
#else    
    singleSupply_3_3V = MXL_DISABLE;
#endif

    status = MxLWare603_API_CfgDevOverwriteDefaults(g_devId, singleSupply_3_3V);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgDevOverwriteDefaults\n");    
    }

    MxLWare603_API_ReqDevVersionInfo(g_devId ,&mxlDevVerInfoPtr);
    PRINTF("ChipID : %d, ChipVersion : %d\n",mxlDevVerInfoPtr.chipId, mxlDevVerInfoPtr.chipVersion);

    //Step 3 : XTAL Setting
    xtalCfg.xtalFreqSel = MXL603_XTAL_16MHz;
    xtalCfg.xtalCap = 25; //12;
    xtalCfg.clkOutEnable = MXL_ENABLE;
    xtalCfg.clkOutDiv = MXL_DISABLE;
    xtalCfg.clkOutExt = MXL_DISABLE;
#ifdef      SUPPORT_BOARD_MAX603_V1_1
    xtalCfg.singleSupply_3_3V = MXL_ENABLE;
#else    
    xtalCfg.singleSupply_3_3V = MXL_DISABLE;
#endif    
    xtalCfg.XtalSharingMode = MXL_DISABLE;
    status = MxLWare603_API_CfgDevXtal(g_devId, xtalCfg);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgDevXtal\n");    
    }

    //Step 4 : IF Out setting
    ifOutCfg.ifOutFreq = MXL603_IF_5MHz; //MXL603_IF_4_1MHz;
    ifOutCfg.ifInversion = MXL_DISABLE;
    ifOutCfg.gainLevel = 11;
    ifOutCfg.manualFreqSet = MXL_DISABLE;
    ifOutCfg.manualIFOutFreqInKHz = 0;
    status = MxLWare603_API_CfgTunerIFOutParam(g_devId, ifOutCfg);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgTunerIFOutParam\n");    
    }

    //Step 5 : AGC Setting
    agcCfg.agcType = MXL603_AGC_EXTERNAL;
    agcCfg.setPoint = 66;
    agcCfg.agcPolarityInverstion = MXL_DISABLE;
    status = MxLWare603_API_CfgTunerAGC(g_devId, agcCfg);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgTunerAGC\n");    
    }

    //Step 6 : Application Mode setting
    tunerModeCfg.signalMode = MXL603_DIG_DVB_T_DTMB;
    tunerModeCfg.ifOutFreqinKHz = 5000; //4100;
    tunerModeCfg.xtalFreqSel = MXL603_XTAL_16MHz;
    tunerModeCfg.ifOutGainLevel = 11;
    status = MxLWare603_API_CfgTunerMode(g_devId, tunerModeCfg);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgTunerMode\n");    
    }

    //Step 7 : Channel frequency & bandwidth setting
    chanTuneCfg.bandWidth = MXL603_TERR_BW_8MHz;
    chanTuneCfg.freqInHz = 474000000;
    chanTuneCfg.signalMode = MXL603_DIG_DVB_T_DTMB;
    chanTuneCfg.xtalFreqSel = MXL603_XTAL_16MHz;
    chanTuneCfg.startTune = MXL_START_TUNE;
    status = MxLWare603_API_CfgTunerChanTune(g_devId, chanTuneCfg);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgTunerChanTune\n");    
    }

    // Wait 15 ms 
    MxLWare603_OEM_Sleep(15);

    // Read back Tuner lock status
    status = MxLWare603_API_ReqTunerLockStatus(g_devId, &rfLockPtr, &refLockPtr);
    if (status == MXL_TRUE)
    {
        if (MXL_LOCKED == rfLockPtr && MXL_LOCKED == refLockPtr)
        {
            PRINTF("Tuner locked\n");
        }
        else
            PRINTF("Tuner unlocked\n");
    }

    // To Change Channel, GOTO Step #7

    // To change Application mode settings, GOTO Step #6
    Ctrl_I2cDisConnect();
    return 0;
}
#endif

int MxL603_OEM_Init(void)
{
    MXL_STATUS status; 
    MXL_BOOL singleSupply_3_3V;
    MXL603_XTAL_SET_CFG_T xtalCfg;
    MXL603_IF_OUT_CFG_T ifOutCfg;
    MXL603_AGC_CFG_T agcCfg;
    MXL603_TUNER_MODE_CFG_T tunerModeCfg;
    MXL603_VER_INFO_T mxlDevVerInfoPtr;

#ifdef		BUILD_EXCUTABLE    
    Power_On();
    usleep(1000);
#endif    
    /* If OEM data is not required, customer should treat g_devId as 
       I2C slave Address */
    g_devId = MXL603_I2C_ADDR;
    Ctrl_I2cConnect(g_devId);
    //Step 1 : Soft Reset MxL603
    status = MxLWare603_API_CfgDevSoftReset(g_devId);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgDevSoftReset\n");    
    }

    //Step 2 : Overwrite Default
#ifdef      SUPPORT_BOARD_MAX603_V1_1
    singleSupply_3_3V = MXL_ENABLE;
#else    
    singleSupply_3_3V = MXL_DISABLE;
#endif
    status = MxLWare603_API_CfgDevOverwriteDefaults(g_devId, singleSupply_3_3V);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgDevOverwriteDefaults\n");    
    }

    MxLWare603_API_ReqDevVersionInfo(g_devId ,&mxlDevVerInfoPtr);
    PRINTF("MxL603 ChipID : %d, ChipVersion : %d\n",mxlDevVerInfoPtr.chipId, mxlDevVerInfoPtr.chipVersion);

    //Step 3 : XTAL Setting
    xtalCfg.xtalFreqSel = MXL603_XTAL_16MHz;
    xtalCfg.xtalCap = 25; //12;
    xtalCfg.clkOutEnable = MXL_ENABLE;
    xtalCfg.clkOutDiv = MXL_DISABLE;
    xtalCfg.clkOutExt = MXL_DISABLE;
#ifdef      SUPPORT_BOARD_MAX603_V1_1
    xtalCfg.singleSupply_3_3V = MXL_ENABLE;
#else    
    xtalCfg.singleSupply_3_3V = MXL_DISABLE;
#endif    
    xtalCfg.XtalSharingMode = MXL_DISABLE;
    status = MxLWare603_API_CfgDevXtal(g_devId, xtalCfg);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgDevXtal\n");    
    }

    //Step 4 : IF Out setting
    ifOutCfg.ifOutFreq = MXL603_IF_5MHz; //MXL603_IF_4_1MHz;
    ifOutCfg.ifInversion = MXL_DISABLE;
    ifOutCfg.gainLevel = 11;
    ifOutCfg.manualFreqSet = MXL_DISABLE;
    ifOutCfg.manualIFOutFreqInKHz = 0;
    status = MxLWare603_API_CfgTunerIFOutParam(g_devId, ifOutCfg);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgTunerIFOutParam\n");    
    }

    //Step 5 : AGC Setting
    agcCfg.agcType = MXL603_AGC_EXTERNAL;
    agcCfg.setPoint = 66;
    agcCfg.agcPolarityInverstion = MXL_DISABLE;
    status = MxLWare603_API_CfgTunerAGC(g_devId, agcCfg);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgTunerAGC\n");    
    }

    //Step 6 : Application Mode setting
    tunerModeCfg.signalMode = MXL603_DIG_DVB_T_DTMB;
    tunerModeCfg.ifOutFreqinKHz = 5000; //4100;
    tunerModeCfg.xtalFreqSel = MXL603_XTAL_16MHz;
    tunerModeCfg.ifOutGainLevel = 11;
    status = MxLWare603_API_CfgTunerMode(g_devId, tunerModeCfg);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgTunerMode\n");    
    }

    return 0;
}

int MxL603_OEM_Tune(int freqInKhz, int bandwidthInKhz)
{
    MXL_STATUS status; 
    MXL603_CHAN_TUNE_CFG_T chanTuneCfg;
    MXL_BOOL refLockPtr = MXL_UNLOCKED;
    MXL_BOOL rfLockPtr = MXL_UNLOCKED;		
    //Step 7 : Channel frequency & bandwidth setting
    int bandWidth = MXL603_TERR_BW_8MHz;
    switch(bandwidthInKhz)
    {
    case 6000:
        bandWidth = MXL603_TERR_BW_6MHz;
        break;
    case 7000:
        bandWidth = MXL603_TERR_BW_7MHz;
        break;
    case 8000:
        bandWidth = MXL603_TERR_BW_8MHz;
        break;
    default:
        return 1;
    }

    chanTuneCfg.bandWidth = bandWidth;
    chanTuneCfg.freqInHz = freqInKhz*1000;
    chanTuneCfg.signalMode = MXL603_DIG_DVB_T_DTMB;
    chanTuneCfg.xtalFreqSel = MXL603_XTAL_16MHz;
    chanTuneCfg.startTune = MXL_START_TUNE;
    status = MxLWare603_API_CfgTunerChanTune(g_devId, chanTuneCfg);
    if (status != MXL_SUCCESS)
    {
        PRINTF("Error! MxLWare603_API_CfgTunerChanTune\n");    
        return 1;
    }

    MxLWare603_OEM_Sleep(15);
    // Read back Tuner lock status
    status = MxLWare603_API_ReqTunerLockStatus(g_devId, &rfLockPtr, &refLockPtr);
    if (status == MXL_TRUE)
    {
        if (MXL_LOCKED == rfLockPtr && MXL_LOCKED == refLockPtr)
        {
            PRINTF("Tuner locked\n");
        }
        else
            PRINTF("Tuner unlocked\n");
    }

    return 0;
}

int MxL603_OEM_DeInit(void)
{
#ifdef		BUILD_EXCUTABLE	
    Power_Off();
#endif    
    return 0;
}

int MxL603_OEM_SetSystem(int system)
{
    return 0;
}

