/*
 *           Copyright 2012 Availink, Inc.
 *
 *  This software contains Availink proprietary information and
 *  its use and disclosure are restricted solely to the terms in
 *  the corresponding written license agreement. It shall not be 
 *  disclosed to anyone other than valid licensees without
 *  written permission of Availink, Inc.
 *
 */


///$Date: 2012-3-8 21:47 $
///
#include "../include/ExtAV2011.h"
#include "../include/IBSP.h"
#include "../include/IBase.h"
#include "../include/II2C.h"
#define AV2011_R0_PLL_LOCK 0x01
#define AV2011_Tuner

#ifdef  TELECHIPS_CODE
typedef struct AVL_DVBSx_Chip AVL_DVBSx_Chip;
typedef struct AV2011_Setting AV2011_Setting;
typedef struct AVL_Tuner      AVL_Tuner;
#endif//TELECHIPS_CODE

static UINT16 tuner_crystal = 27; //unit is MHz
//static AVL_uchar auto_scan=0; //0 for normal mode, 1 for Blindscan mode
static void Time_DELAY_MS(UINT32 ms);
 //Main function at tuner control
static AVL_DVBSx_ErrorCode Tuner_control(UINT32 channel_freq, UINT32 bb_sym,AVL_DVBSx_Chip * pAVLChip,struct AV2011_Setting * AV2011_Configure);
// I2C write function ( start register, register array pointer, register length)
static AVL_DVBSx_ErrorCode AV2011_I2C_write(UINT8 reg_start,UINT8* buff,UINT8 len,AVL_DVBSx_Chip * pAVLChip);
AVL_DVBSx_ErrorCode ExtAV2011_RegInit(struct AVL_Tuner * pTuner,AVL_DVBSx_Chip * pAVLChip);
static AVL_uint16 AV2011_address = 0;


AVL_DVBSx_ErrorCode AVL_DVBSx_ExtAV2011_GetLockStatus(struct AVL_Tuner * pTuner)
{
	AVL_DVBSx_ErrorCode r;
	AVL_uchar uilock;
	//Send register address
	uilock = 0x0b;
	r = AVL_DVBSx_II2CRepeater_SendData((AVL_uchar)(pTuner->m_uiSlaveAddress), &uilock, 1, pTuner->m_pAVLChip );
	if( AVL_DVBSx_EC_OK != r ) 
	{
		return(r);
	}
	r = AVL_DVBSx_II2CRepeater_ReadData((AVL_uchar)(pTuner->m_uiSlaveAddress), &uilock, 1, pTuner->m_pAVLChip );
	//r = AVL_DVBSx_II2CRepeater_ReadData_Multi(  pTuner->m_uiSlaveAddress, &uilock, 0x0B,(AVL_uint16)(0x1), pTuner->m_pAVLChip );
	if( AVL_DVBSx_EC_OK == r ) 
	{
		if( 0 == (uilock & AV2011_R0_PLL_LOCK) ) 
		{
			r = AVL_DVBSx_EC_GeneralFail;
		}
	}
	return(r);
}

AVL_DVBSx_ErrorCode AVL_DVBSx_ExtAV2011_Initialize(struct AVL_Tuner * pTuner)
{
	AVL_DVBSx_ErrorCode r;
	AV2011_address = pTuner->m_uiSlaveAddress;
	r = AVL_DVBSx_II2C_Write16(pTuner->m_pAVLChip, rc_tuner_slave_addr_addr, pTuner->m_uiSlaveAddress);
	r |= AVL_DVBSx_II2C_Write16(pTuner->m_pAVLChip, rc_tuner_use_internal_control_addr, 0);
	r |= AVL_DVBSx_II2C_Write16(pTuner->m_pAVLChip, rc_tuner_LPF_margin_100kHz_addr, 0);	//clean up the LPF margin for blind scan. for external driver, this must be zero.
	r |= AVL_DVBSx_II2C_Write16(pTuner->m_pAVLChip, rc_tuner_max_LPF_100kHz_addr, 400);	//set up the right LPF for blind scan to regulate the freq_step. This field should corresponding the flat response part of the LPF.

	r |= AVL_DVBSx_II2CRepeater_Initialize(pTuner->m_uiI2CBusClock_kHz, pTuner->m_pAVLChip);

	r |= ExtAV2011_RegInit(pTuner,pTuner->m_pAVLChip); //init all tuner register
	return(r);	
}


AVL_DVBSx_ErrorCode ExtAV2011_RegInit(struct AVL_Tuner * pTuner,AVL_DVBSx_Chip * pAVLChip)
{
	UINT8 reg[50];
	AVL_DVBSx_ErrorCode r;
	
		//Tuner initial registers R0~R41
		reg[0]=(char) (0x38);
		reg[1]=(char) (0x00);
		reg[2]=(char) (0x00);
		reg[3]=(char) (0x50);
		reg[4]=(char) (0x1f);
		reg[5]=(char) (0xa3);
		reg[6]=(char) (0xfd);
		reg[7]=(char) (0x58);
		reg[8]=(char) (0x46); // 0x0e
		reg[9]=(char) (0x82);
		reg[10]=(char) (0x88);
		reg[11]=(char) (0xb4);
		reg[12]=(char) (0xd6); //RFLP=ON at Power on initial
		reg[13]=(char) (0x40);
#ifdef AV2011_Tuner
		reg[14]=(char) (0x94);
		reg[15]=(char) (0x4a);
#else
		reg[14]=(char) (0x5b);
		reg[15]=(char) (0x6a);
#endif
		reg[16]=(char) (0x66);
		reg[17]=(char) (0x40);
		reg[18]=(char) (0x80);
		reg[19]=(char) (0x2b);
		reg[20]=(char) (0x6a);
		reg[21]=(char) (0x50);
		reg[22]=(char) (0x91);
		reg[23]=(char) (0x27);
		reg[24]=(char) (0x8f);
		reg[25]=(char) (0xcc);
		reg[26]=(char) (0x21);
		reg[27]=(char) (0x10);
		reg[28]=(char) (0x80);
		reg[29]=(char) (0x02);
		reg[30]=(char) (0xf5);
		reg[31]=(char) (0x7f);
		reg[32]=(char) (0x4a);
		reg[33]=(char) (0x9b);
		reg[34]=(char) (0xe0);
		reg[35]=(char) (0xe0);
		reg[36]=(char) (0x36);
		reg[37]=(char) (0x00); //Disable FT-function at Power on initial
		reg[38]=(char) (0xab);
		reg[39]=(char) (0x97);
		reg[40]=(char) (0xc5);
		reg[41]=(char) (0xa8);
	
		// Sequence 1
		// Send Reg0 ->Reg11
		r =  AV2011_I2C_write(0,reg,12,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK){
			return(r);
		}
		// Time delay 1ms
		Time_DELAY_MS(1);
		// Sequence 2
		// Send Reg13 ->Reg24
		r = AV2011_I2C_write(13,reg+13,12,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK){
			return(r);
		}
		// Send Reg25 ->Reg35
		r = AV2011_I2C_write(25,reg+25,11,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK){
			return(r);
		}
		// Send Reg36 ->Reg41
		r = AV2011_I2C_write(36,reg+36,6,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK){
			return(r);
		}
		// Time delay 1ms
		Time_DELAY_MS(1);
		// Sequence 3
		// send reg12
		r = AV2011_I2C_write(12,reg+12,1,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK){
			return(r);
		}
		//monsen 20081125, Wait 100 ms
		Time_DELAY_MS(10);
		//monsen 20081030, re initial again
		{
			// Sequence 1
			// Send Reg0 ->Reg11
			r = AV2011_I2C_write(0,reg,12,pAVLChip);
			if(r!=AVL_DVBSx_EC_OK){
			return(r);
			}
			// Time delay 1ms
			Time_DELAY_MS(1);
			// Sequence 2
			// Send Reg13 ->Reg24		
			r = AV2011_I2C_write(13,reg+13,12,pAVLChip);
			if(r!=AVL_DVBSx_EC_OK){
			return(r);
		    }
			// Send Reg25 ->Reg35
			r = AV2011_I2C_write(25,reg+25,11,pAVLChip);
			if(r!=AVL_DVBSx_EC_OK){
			return(r);
		    }
			// Send Reg36 ->Reg41
			r = AV2011_I2C_write(36,reg+36,6,pAVLChip);
			if(r!=AVL_DVBSx_EC_OK){
			return(r);
			}
			// Time delay 1ms
			Time_DELAY_MS(1);
			// Sequence 3
			// send reg12
			r = AV2011_I2C_write(12,reg+12,1,pAVLChip);
			if(r!=AVL_DVBSx_EC_OK){
			return(r);
			}
			Time_DELAY_MS(5);
			return(r);
	    }
}

/********************************************************************************
* AVL_DVBSx_ErrorCode Tuner_control(UINT32 channel_freq, UINT32 bb_sym, AVL_DVBSx_Chip * pAVLChip,struct AV2011_Setting * AV2011_Configure)
*
* Arguments:
* Parameter1: UINT32 channel_freq : Channel frequency (Unit: MHz)
* Parameter2: UINT32 bb_sym : Baseband Symbol Rate (Unit: KHz)
* Paramiter3: AVL_DVBSx_Chip * pAVLChip : AVLChip structure
* Paramiter4: struct AV2011_Setting * AV2011_Configure : Special configuration for AV2011(IQ mode, Auto_Gain, Scan mode, RF_LP)
* Return Value: INT32 : Result
*****************************************************************************/
AVL_DVBSx_ErrorCode Tuner_control(UINT32 channel_freq, UINT32 tuner_lpf, AVL_DVBSx_Chip * pAVLChip,struct AV2011_Setting * AV2011_Configure)
{
	UINT8 reg[50];
	UINT32 fracN;
	UINT32 BW;
	UINT32 BF;
	AVL_DVBSx_ErrorCode r;
	Time_DELAY_MS(50);
	fracN = (channel_freq + tuner_crystal/2)/tuner_crystal;
	if(fracN > 0xff)
	fracN = 0xff;
	reg[0]=(char) (fracN & 0xff);
	fracN = (channel_freq<<17)/tuner_crystal;
	fracN = fracN & 0x1ffff;
	reg[1]=(char) ((fracN>>9)&0xff);
	reg[2]=(char) ((fracN>>1)&0xff);
	// reg[3]_D7 is frac<0>, D6~D0 is 0x50
#ifdef AV2011_Tuner
	//AV2011 IQ Single_end/Differential mode at bit2
	reg[3]=(char) (((fracN<<7)&0x80) | 0x50 |(AV2011_Configure->Single_End)<<2);
#else
    //AV2020 no IQ mode
   	reg[3]=(char) (((fracN<<7)&0x80) | 0x50 );
#endif
	// Channel Filter Bandwidth Setting.
	//"sym" unit is Hz;
	if(AV2011_Configure->Auto_Scan)//auto scan requested by BB
	{
		reg[5] = 0xA3; //BW=27MHz
		// Sequence 4
		// Send Reg0 ->Reg4
		r = AV2011_I2C_write(0,reg,4,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK)
		{
			return(r);
		}
		Time_DELAY_MS(5);

		// Sequence 5
		// Send Reg5
		r = AV2011_I2C_write(5, reg+5, 1,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK)
		{
			return(r);
		}
		Time_DELAY_MS(5);

		// Fine-tune Function Control
		// Auto-scan mode 
		// FT_block=1, FT_EN=0, FT_hold=1
		reg[37] = 0x05;
		r = AV2011_I2C_write(37, reg+37, 1,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK)
		{
			return(r);
		}
		// Time delay 4ms
		Time_DELAY_MS(4);
	}
	else
	{
		BW = tuner_lpf;
		// Bandwidth can be tuned from 4M to 40M
		if( BW< 4000)
		{
			BW = 4000;
		}
		if( BW> 40000)
		{
			BW = 40000;
		}
		BF = (BW*127 + 21100/2) / (21100); 
		reg[5] = (UINT8)BF;

		// Sequence 4
		// Send Reg0 ->Reg4
		Time_DELAY_MS(5);
		r = AV2011_I2C_write(0,reg,4,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK)
		{
			return(r);
		}
		Time_DELAY_MS(5);
		r = AV2011_I2C_write(0,reg,4,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK)
		{
			return(r);
		}
		Time_DELAY_MS(5);
		// Sequence 5
		// Send Reg5
		r = AV2011_I2C_write(5, reg+5, 1,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK)
		{
			return(r);
		}
		Time_DELAY_MS(5);
		
		// Fine-tune Function Control
		// not auto-scan case. enable block function.
		// FT_block=1, FT_EN=1, FT_hold=0
		reg[37] = 0x04 | ((AV2011_Configure->AutoGain)<<1);
		r = AV2011_I2C_write(37, reg+37, 1,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK)
		{
		return(r);
		}
		Time_DELAY_MS(5);
		//Disable RFLP at Lock Channel sequence after reg[37]
		//RFLP=OFF at Lock Channel sequence
		// RFLP can be Turned OFF, only at Receiving mode.
		reg[12] = 0x96 + ((AV2011_Configure->RF_Loop)<<6);
		r = AV2011_I2C_write(12, reg+12, 1,pAVLChip);
		if(r!=AVL_DVBSx_EC_OK)
		{
			return(r);
			Time_DELAY_MS(5);
		}
	}
	return r;
}

static void Time_DELAY_MS(UINT32 ms)
{
	AVL_DVBSx_IBSP_Delay(ms);
}

static AVL_DVBSx_ErrorCode AV2011_I2C_write(UINT8 reg_start,UINT8* buff,UINT8 len,AVL_DVBSx_Chip * pAVLChip)
{
	AVL_DVBSx_ErrorCode r=0;
	AVL_uint16 uiTimeOut = 0;
	AVL_uchar ucTemp[50];
	int i;
	AVL_DVBSx_IBSP_Delay(5);
	ucTemp[0] = reg_start;
	r = AVL_DVBSx_II2CRepeater_GetOPStatus( pAVLChip );
	while( r != AVL_DVBSx_EC_OK)
	{
#ifdef AV2011_ENABLE_TIMEOUT
			if( uiTimeOut++>AV2011_TIMEOUT ) 
			{
				return(AVL_DVBSx_EC_TimeOut);
			}
#endif
			AVL_DVBSx_IBSP_Delay(1);
			r = AVL_DVBSx_II2CRepeater_GetOPStatus( pAVLChip );
	}
	for(i=1;i<len+1;i++)
	{
	   // ucTemp[i++]=AVL_uchar(reg_start+i);
		ucTemp[i]=*(buff+i-1);
	}
	r = AVL_DVBSx_II2CRepeater_SendData((AVL_uchar) AV2011_address,ucTemp, len+1, pAVLChip );
	if(r != AVL_DVBSx_EC_OK){
		return(r);
	}
	AVL_DVBSx_IBSP_Delay(1);
	  return(r);
}

static AVL_DVBSx_ErrorCode Frequency_LPF_Adjustment(struct AVL_Tuner * pTuner,AVL_uint16 *uiAdjustFreq)
{
	AVL_DVBSx_ErrorCode r;
	AVL_uint32 uitemp1;
	AVL_uint16 uitemp2;
	AVL_uint16 minimum_LPF_100kHz;
	AVL_uint16 carrierFrequency_100kHz;

	r = AVL_DVBSx_II2C_Read32(pTuner->m_pAVLChip, 0x263E, &uitemp1);
	r |= AVL_DVBSx_II2C_Read16(pTuner->m_pAVLChip, 0x2642, &uitemp2);
	if(r != AVL_DVBSx_EC_OK)
	{
		*uiAdjustFreq = pTuner->m_uiFrequency_100kHz;
		return r;
	}

	if(pTuner->m_uiSymbolRate_Hz <= uitemp1)
	{
		carrierFrequency_100kHz =(AVL_uint16 )((uitemp2/10)+ pTuner->m_uiFrequency_100kHz);

		minimum_LPF_100kHz = (pTuner->m_uiSymbolRate_Hz/100000 )*135/200 +  (uitemp2/10) + 50;
		if(pTuner->m_uiLPF_100kHz < minimum_LPF_100kHz)
		{
			pTuner->m_uiLPF_100kHz = (AVL_uint16 )(minimum_LPF_100kHz);
		}
	}
	else
	{
		carrierFrequency_100kHz = pTuner->m_uiFrequency_100kHz;
	}

	*uiAdjustFreq = carrierFrequency_100kHz;

	return AVL_DVBSx_EC_OK;

}

AVL_DVBSx_ErrorCode AVL_DVBSx_ExtAV2011_Lock(  struct AVL_Tuner * pTuner)
{
	AVL_DVBSx_ErrorCode r;
	AV2011_Setting AV2011_Set;
	AVL_uint16 carrierFrequency_100kHz;

	Frequency_LPF_Adjustment(pTuner, &carrierFrequency_100kHz);

	//pTuner->m_pParameters is used store four special setting for AV2011:IQ mode, auto Gain setting, Work mode, RF loop through
	if(pTuner->m_pParameters == 0)  //if none
	{
		AV2011_Set.Auto_Scan = 0;  //Default Setting: Normal lock mode
		AV2011_Set.AutoGain = 1;   //Default Setting: Auto Gain control on
		AV2011_Set.Single_End = 1; //Default setting: IQ Differential mode
		AV2011_Set.RF_Loop = 1;    //Default setting: open RF loop through
		//(struct AV2011_Setting)(pTuner->m_pParameters) = &AV2011_Set;  //use default setting if not set it in Struct pTuner->m_pParameters.
		r = Tuner_control((UINT32)((carrierFrequency_100kHz)/10), (UINT32)((pTuner->m_uiLPF_100kHz)*100), pTuner->m_pAVLChip, &AV2011_Set);
	}
	else  //set AV2011 tuner special setting according to pTuner->m_pParameter
	{
		r = Tuner_control((UINT32)((carrierFrequency_100kHz)/10), (UINT32)((pTuner->m_uiLPF_100kHz)*100), pTuner->m_pAVLChip, (AV2011_Setting *)(pTuner->m_pParameters));
	}
	return(r);
}

AVL_DVBSx_ErrorCode AVL_DVBSx_ExtAV2011_Check_I2C_Address(AVL_puint16 Address, struct AVL_DVBSx_Chip * pAVLChip)
{
	AVL_DVBSx_ErrorCode r;
	AVL_Tuner Tuner;
	AVL_Tuner * pTuner = &Tuner;
	AVL_uint16 Addr;
	AVL_uchar pTemp[23] = {0};

	ChunkAddr(i2cm_rsp_addr, pTemp);

	pTuner->m_uiFrequency_100kHz = 15500;
	pTuner->m_uiI2CBusClock_kHz = 100;
	pTuner->m_uiLPF_100kHz = 320;
	pTuner->m_uiSymbolRate_Hz = 30000000;
	pTuner->m_pAVLChip = pAVLChip;
	pTuner->m_pInitializeFunc = &AVL_DVBSx_ExtAV2011_Initialize;
	pTuner->m_pLockFunc = &AVL_DVBSx_ExtAV2011_Lock;
	pTuner->m_pGetLockStatusFunc = &AVL_DVBSx_ExtAV2011_GetLockStatus;

	r = AVL_DVBSx_II2CRepeater_Initialize(pTuner->m_uiI2CBusClock_kHz, pTuner->m_pAVLChip);
	if (r != AVL_DVBSx_EC_OK)
	{
		return r;
	}

	for (Addr = 0; Addr < 256; Addr++)
	{
		r = AVL_DVBSx_II2C_Write(pTuner->m_pAVLChip, pTemp, 23);		//erase the buffer i2cm_rsp_addr content 
		if (AVL_DVBSx_EC_OK != r)
		{
			return r;
		}
		pTuner->m_uiSlaveAddress = Addr;

		r = pTuner->m_pInitializeFunc(pTuner);
		if (r != AVL_DVBSx_EC_OK)
		{
			continue;
		}
		AVL_DVBSx_IBSP_Delay(1);
		r = pTuner->m_pLockFunc(pTuner);
		if (r != AVL_DVBSx_EC_OK)
		{
			continue;
		}
		AVL_DVBSx_IBSP_Delay(50);		//Wait a while for tuner to lock in certain frequency.
		r = pTuner->m_pGetLockStatusFunc(pTuner);
		if (r != AVL_DVBSx_EC_OK)
		{
			continue;
		}
		break;
	}
	if (Addr >= 256)
	{
		return AVL_DVBSx_EC_GeneralFail;
	}
	else
	{
		*Address = pTuner->m_uiSlaveAddress;
	}
	return AVL_DVBSx_EC_OK;
}
