/*****************************************************************************************
 *
 * FILE NAME          : MxL101SF_PhyDefs.h
 * 
 * AUTHOR             : Brenndon Lee, Mahendra Kondur
 *
 * DATE CREATED       : 5/18/2009
 *                      3/31/2010 - Support for Multiple MxL101SF devices
 *                      6/3/2010 - Support for MP version silicon
 *
 * DESCRIPTION        : This file contains MxL101SF common control register 
 *                      definitions
 *
 *****************************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ****************************************************************************************/

#ifndef __MXL101SF_PHY_DEFS_H__
#define __MXL101SF_PHY_DEFS_H__

/*****************************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
*****************************************************************************************/
#include "MaxLinearDataTypes.h"
#include "MxL101SF_PhyCtrlApi.h"

/*****************************************************************************************
    Macros
*****************************************************************************************/

#define CHIP_ID_REG                 0xFC     
#define TOP_CHIP_REV_ID_REG         0xFA     
#define MXL101SF_DEV_REVISION_MASK  0x0F

#define SNR_RB_LSB_REG            0x27
#define SNR_RB_MSB_REG            0x28

#define N_ACCUMULATE_REG          0x11    
#define RS_AVG_ERRORS_LSB_REG     0x2C    
#define RS_AVG_ERRORS_MSB_REG     0x2D    

#define IRQ_STATUS_REG            0x24
#define IRQ_MASK_FEC_LOCK         0x10

#define SYNC_LOCK_REG             0x28
#define SYNC_LOCK_MASK            0x10

#define RS_LOCK_REG               0x28
#define RS_LOCK_DET_MASK          0x08

#define INITACQ_NODETECT_REG      0x20 
#define FORCE_NFFT_CPSIZE_REG     0x20

#define CODE_RATE_TPS_REG         0x29
#define CODE_RATE_TPS_MASK        0x07

#define CP_LOCK_REG               0x28
#define CP_LOCK_DET_MASK          0x04

#define TPS_HIERACHY_REG          0x29
#define TPS_HIERARCHY_INFO_MASK   0x40

#define TPS_HPORLP_REG            0x20
#define TPS_HPORLP_INFO_MASK      0x80

#define MODORDER_TPS_REG          0x2A
#define PARAM_CONSTELLATION_MASK  0x30

#define TPS_ALPHA_REG             0x2A
#define TPS_ALPHA_MASK            0x03

#define MODE_TPS_REG              0x2A
#define PARAM_FFT_MODE_MASK       0x0C

#define CP_TPS_REG                0x29
#define PARAM_GI_MASK             0x30

#define AGC_LOCK_REG              0x26
#define AGC_LOCK_MASK             0x20

#define TPS_LOCK_REG              0x2A
#define PARAM_TPS_LOCK_MASK       0x40

#define FEC_PER_COUNT_REG         0x2E
#define FEC_PER_SCALE_REG         0x2B
#define FEC_PER_SCALE_MASK        0x03

#define PIN_MUX_MODE_REG          0x1B    
#define ENABLE_PIN_MUX            0x1E

#define MPEG_OUT_CLK_INV_REG      0x17    
#define MPEG_OUT_CTRL_REG         0x18    

#define INVERTED_CLK_PHASE        0x20
#define MPEG_DATA_PARALLEL        0x01
#define MPEG_DATA_SERIAL          0x02

#define INVERTED_MPEG_SYNC        0x04
#define INVERTED_MPEG_VALID       0x08

#define MPEG_BIT_ORDER_REG          0x19
#define MPEG_SER_MSB                0x80
#define MPEG_SER_MSB_FIRST_ENABLED  0x01

#define TUNER_LOOP_THRU_CONTROL_REG     0x09
#define ENABLE_LOOP_THRU                0x01

#define TOTAL_NUM_IF_OUTPUT_FREQ        16

#define TUNER_NORMAL_IF_SPECTRUM        0x0
#define TUNER_INVERT_IF_SPECTRUM        0x10

#define TUNER_IF_SEL_REG          0x06
#define TUNER_IF_FCW_REG          0x3C
#define TUNER_IF_FCW_BYP_REG      0x3D
#define RF_LOCK_STATUS_REG        0x23

#define DIG_CLK_FREQ_SEL_REG      0x07
#define REF_SYNTH_INT_REG         0x5C
#define REF_SYNTH_REMAIN_REG      0x58
#define DIG_RFREFSELECT_REG       0x32
#define XTAL_CLK_OUT_GAIN_REG     0x31
#define TUNER_LOOP_THRU_CTRL_REG  0x09
#define DIG_XTAL_ENABLE_REG       0x06
#define DIG_XTAL_BIAS_REG         0x66
#define XTAL_CAP_REG              0x08

#define GPO_CTRL_REG              0x18
#define GPO_0_MASK                0x10
#define GPO_1_MASK                0x20

#define MXL_MODE_REG              0x03
#define MXL_TUNE_REG              0x1C

#define DIG_RF_PWR_LSB_REG        0x46
#define DIG_RF_PWR_MSB_REG        0x47

#define AGC_LOCK_REG              0x26
#define AGC_LOCK_MASK             0x20
#define AGC_GAIN_LSB_REG          0x25
#define AGC_GAIN_MSB_REG          0x26

#define TPS_CELL_ID_LSB_REG       0x98
#define TPS_CELL_ID_MSB_REG       0x99

#define FREQ_OFFSET_LSB_REG       0x9D
#define FREQ_OFFSET_MSB_REG       0x9E

#define SPECTRUM_CTRL_REG         0x8D
#define SPECTRUM_MASK             0x01

#define PLL_CTRL_REG              0xF1

#define MPEG_DRIVE_STRENGTH_REG   0x04
#define MPEG_DRIVE_STRENGTH_MASK  0x01

#define MPEG_OUT_ENABLE_MASK      0xC0
#define CLEAR_MPEG_SETTINGS_MASK  0xF0

#define MXL_PAGE_REG              0x00
#define MXL_REG_PAGE_0            0x00
#define MXL_REG_PAGE_1            0x01
#define MXL_REG_PAGE_2            0x02

#define TPS_CELL_ID_LSB_REG       0x98
#define TPS_CELL_ID_MSB_REG       0x99

#define CELL_ID_CP_LOCK_TO1       1024
#define CELL_ID_CP_LOCK_TO2       2560
#define CELL_ID_CP_LOCK_TO        350 

#define AGC_NO_CHANNEL_THRESHOLD  6112

#define CH_SCAN_CP_LOCK_TO1       512
#define CH_SCAN_CP_LOCK_TO2       1536
#define CH_SCAN_CP_LOCK_TO3       2048
#define CH_SCAN_CP_LOCK_TO4       2560
#define CH_SCAN_TPS_LOCK_TO1       1024
#define CH_SCAN_TPS_LOCK_TO2       2048
#define CH_SCAN_TPS_LOCK_TO3       2560
#define CH_SCAN_TPS_LOCK_TO4       3072
#define CH_SCAN_RS_LOCK_TO        5000

#define FREQ_OFFSET_SRCH_RANGE_REG  0xEA

/*****************************************************************************************
    User-Defined Types (Typedefs)
*****************************************************************************************/

typedef struct
{
  UINT8 regAddr;
  UINT8 mask;
  UINT8 data;
} REG_CTRL_INFO_T, *PREG_CTRL_INFO_T;

/*****************************************************************************************
    Global Variable Declarations
*****************************************************************************************/

extern REG_CTRL_INFO_T MxL_OverwriteDefault[]; 
extern REG_CTRL_INFO_T MxL_SuspendMode[];
extern REG_CTRL_INFO_T MxL_WakeUp[];
extern REG_CTRL_INFO_T MxL_StandbyMode[];
extern REG_CTRL_INFO_T MxL_MpegTsON[];
extern REG_CTRL_INFO_T MxL_MpegTsOFF[];
extern REG_CTRL_INFO_T MxL_PhySoftReset[];     
extern REG_CTRL_INFO_T MxL_TunerDemodMode[];
extern REG_CTRL_INFO_T MxL_TunerMode[];
extern REG_CTRL_INFO_T MxL_TopMasterEnable[];
extern REG_CTRL_INFO_T MxL_TopMasterDisable[];
extern REG_CTRL_INFO_T MxL_IrqClear[];
extern REG_CTRL_INFO_T MxL_ResetPerCount[];
extern REG_CTRL_INFO_T MxL_CableSettings[];
extern REG_CTRL_INFO_T MxL_EnableCellId[];
extern REG_CTRL_INFO_T MxL_DisableCellId[];
extern REG_CTRL_INFO_T MxL_SetSpurFilt[];
extern REG_CTRL_INFO_T MxL_SetSpurNotch[];
extern REG_CTRL_INFO_T MxL_SetSpurNotch2[];
extern REG_CTRL_INFO_T MxL_SpurCoeff6MHz[];
extern REG_CTRL_INFO_T MxL_SpurCoeff7MHz[];
extern REG_CTRL_INFO_T MxL_SpurCoeff8MHz[];
extern REG_CTRL_INFO_T MxL_ClearSpurFilt[];
extern REG_CTRL_INFO_T MxL_EnableExtTuneCfg[];
extern REG_CTRL_INFO_T MxL_DisableExtTuneCfg[];

/*****************************************************************************************
    Prototypes
*****************************************************************************************/

void Ctrl_PhyTune(UINT32 freqInHz, UINT8 bwInMHz, MXL_DEV_MODE_E tunerMode, REG_CTRL_INFO_T * arrayPtr);
MXL_DEV_MODE_E Ctrl_ReadCurrentDeviceMode(UINT8 i2cSlvAddr, void * privDataPtr);
MXL_STATUS Ctrl_ConfigSpurCoeff(PMXL_RF_TUNE_CFG_T ChanTuneParamPtr);
MXL_STATUS Ctrl_ProgramRegisters(UINT8 I2cSlaveAddr, 
                                 PREG_CTRL_INFO_T ctrlRegInfoPtr, 
                                 void *PrivateDataPtr);
MXL_STATUS Ctrl_ReadAgcGain(UINT8 i2cSlvAddr, void * privDataPtr, UINT16 * agcGainPtr);

// OEM specific APIs
MXL_STATUS Ctrl_ReadRegister(UINT8 I2cSlaveAddr, 
                             UINT8 regAddr, 
                             UINT8 *dataPtr, 
                             void *PrivateDataPtr);
MXL_STATUS Ctrl_WriteRegister(UINT8 I2cSlaveAddr, 
                              UINT8 regAddr, 
                              UINT8 regData, 
                              void *PrivateDataPtr);

#endif /* __MXL101SF_PHY_CFG_H__*/




