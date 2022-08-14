/****************************************************************************
 *   FileName    : tcc317x_core.c
 *   Description : core Function
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall 
constitute any express or implied warranty of any kind, including without limitation, any warranty 
of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright 
or other third party intellectual property right. No warranty is made, express or implied, 
regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of 
or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************/

#include "tcpal_os.h"
#include "tcpal_i2c.h"
#include "tcpal_spi.h"
#include "tcc317x_mailbox.h"
#include "tcc317x_core.h"
#include "tcc317x_register_control.h"
#include "tcc317x_rf.h"
#include "tcc317x_tdmb.h"
#include "tcc317x_command_control.h"


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*          Device driver version information (8bit.8bit.8bit)              */

I32U Tcc317xCoreVersion = ((0<<16) | (2<<8) | (38));
extern I32U Tcc317x_RF_REG_DAB_VER;
extern I32U Tcc317x_RF_REG_DMB_VER;

/*                                                                          */
/*--------------------------------------------------------------------------*/


char *MailSemaphoreName[4][4] = { 
	{"MailboxSemaphore00", "MailboxSemaphore01",
	"MailboxSemaphore02", "MailboxSemaphore03"},
	{"MailboxSemaphore10", "MailboxSemaphore11",
	"MailboxSemaphore12", "MailboxSemaphore13"},
	{"MailboxSemaphore20", "MailboxSemaphore21",
	"MailboxSemaphore22", "MailboxSemaphore23"},
	{"MailboxSemaphore30", "MailboxSemaphore31",
	"MailboxSemaphore32", "MailboxSemaphore33"}
};

char *OPMailboxSemaphoreName[4][4] = { 
	{"OpMailboxSemaphore00", "OpMailboxSemaphore01",
	"OpMailboxSemaphore02", "OpMailboxSemaphore03"},
	{"OpMailboxSemaphore10", "OpMailboxSemaphore11",
	"OpMailboxSemaphore12", "OpMailboxSemaphore13"},
	{"OpMailboxSemaphore20", "OpMailboxSemaphore21",
	"OpMailboxSemaphore22", "OpMailboxSemaphore23"},
	{"OpMailboxSemaphore30", "OpMailboxSemaphore31",
	"OpMailboxSemaphore32", "OpMailboxSemaphore33"}
};


/* static functions */
static I32S Tcc317xColdbootParserUtil(I08U * pData, I32U size,
				      Tcc317xBoot_t * pBOOTBin);
static void Tcc317xSetGpio(Tcc317xHandle_t * _handle);
static I32S Tcc317xSetPll(Tcc317xHandle_t * _handle);
static void Tcc317xSaveOption(I32S _moduleIndex,
			      Tcc317xOption_t * _Tcc317xOption);
static void Tcc317xSetInterruptControl(Tcc317xHandle_t * _handle);
static void Tcc317xOutBufferConfig(Tcc317xHandle_t * _handle);
static void Tcc317xSetStreamControl(Tcc317xHandle_t * _handle);
static I32S Tcc317xCodeDownload(I32S _moduleIndex, I32S _diversityIndex, 
				I08U * _coldbootData, I32S _codeSize, 
				I32S _separateFlag);
static I32S Tcc317xCrcCheckAddressing(I32S _moduleIndex, I32S _diversityIndex,
			      I08U * _coldbootData, I32S _codeSize,
			      Tcc317xBoot_t * _boot, I32S _separateFlag);

#ifndef BITSET
#define	BITSET(X, MASK)				( (X) |= (I32U)(MASK) )
#endif
#ifndef BITCLR
#define	BITCLR(X, MASK)				( (X) &= ~((I32U)(MASK)) )
#endif

#define TC317X_AGC_IN_BOOT

struct tcbd_spur_data {
	I32S freq_khz;
	I32S num_data;
	I32S data[7];
};

Tcc317xHandle_t Tcc317xHandle[TCC317X_MAX][TCC317X_DIVERSITY_MAX];

TcpalSemaphore_t Tcc317xInterfaceSema;
I32U *pTcc317xInterfaceSema = NULL;
I32U Tcc317xUseInterfaceCount = 0;

TcpalSemaphore_t Tcc317xMailboxSema[TCC317X_MAX][TCC317X_DIVERSITY_MAX];
TcpalSemaphore_t Tcc317xOpMailboxSema[TCC317X_MAX][TCC317X_DIVERSITY_MAX];

/* number of devices with EnumTcc317xBoardType */
/*
	[0] : TCC317X_BOARD_SINGLE = 0,
	[1] : TCC317X_BOARD_DUAL_CHAINMODE,
	[2] : TCC317X_BOARD_TRIPLE_CHAINMODE,
	[3] : TCC317X_BOARD_2DIVERSITY,
	[4] : TCC317X_BOARD_3DIVERSITY,
	[5] : TCC317X_BOARD_4DIVERSITY,
	[6] : TCC317X_BOARD_MAX
*/

I32S Tcc317xCurrBoardMode[4] = { 0, 0, 0, 0 };
I08S Tcc317xCurrtModuleCount[4] = { 1, 1, 1, 1 };
I08S Tcc317xCurrDiversityCount[4] = { 1, 1, 1, 1 };

/* AutoSerach */
unsigned int TCC317X_LBAND_TABLE[] = {
	1452960, 1454672, 1456384, 1458096,	//LA~LD
	1459808, 1461520, 1463232, 1464944,	//LE~LH
	1466656, 1468368, 1470080, 1471792,	//LI~LL
	1473504, 1475216, 1476928, 1478640,	//LM~LP
	1480352, 1482064, 1483776, 1485488,	//LQ~LT
	1487200, 1488912, 1490624,	//LU~LW
};

unsigned int TCC317X_BANDIII_TABLE[] = {
	174928, 176640, 178352, 180064,	// 5A~ 5D
	181936, 183648, 185360, 187072,	// 6A~ 6D
	188928, 190640, 192352, 194064,	// 7A~ 7D
	195936, 197648, 199360, 201072,	// 8A~ 8D
	202928, 204640, 206352, 208064,	// 9A~ 9D
	209936, 211648, 213360, 215072, 210096,	//10A~10D, 10N
	216928, 218640, 220352, 222064, 217088,	//11A~11D, 11N
	223936, 225648, 227360, 229072, 224096,	//12A~12D, 12N
	230784, 232496, 234208, 235776, 237488, 239200,	//13A~13F
};

unsigned int TCC317X_KOREA_BAND_TABLE[] = {
	175280, 177008, 178736,	//7A ~ 7C
	181280, 183008, 184736,	//8A ~ 8C
	187280, 189008, 190736,	//9A ~ 9C
	193280, 195008, 196736,	//10A~10C
	199280, 201008, 202736,	//11A~11C
	205280, 207008, 208736,	//12A~12C
	211280, 213008, 214736,	//13A~13C
};

static struct tcbd_spur_data spur_table_default[] = {
	{ 181936, 2, {(0x035C<<16)|(0x0342), (0x0359<<16)|(0x033e)} },
	{ 183008, 2, {(0x00CA<<16)|(0x036B), (0x00CE<<16)|(0x0368)} },
	{ 192352, 2, {(0x0081<<16)|(0x0328), (0x0084<<16)|(0x0324)} },
	{ 201008, 2, {(0x033A<<16)|(0x0366), (0x0336<<16)|(0x0363)} },
	{ 201072, 2, {(0x034B<<16)|(0x0352), (0x0347<<16)|(0x034F)} },
	{ 211280, 2, {(0x001E<<16)|(0x0307), (0x001F<<16)|(0x0302)} },
	{ 211648, 2, {(0x009F<<16)|(0x033E), (0x00A2<<16)|(0x033A)} },
	{ 220352, 2, {(0x0361<<16)|(0x033E), (0x035E<<16)|(0x033A)} },
	{ 230784, 2, {(0x008B<<16)|(0x032F), (0x008E<<16)|(0x032B)} },
	{1459808, 2, {(0x00CA<<16)|(0x036B), (0x00CE<<16)|(0x0368)} },
	{1468368, 2, {(0x0366<<16)|(0x033A), (0x0363<<16)|(0x0336)} },
	{1478640, 2, {(0x005A<<16)|(0x0316), (0x005C<<16)|(0x0311)} }
};


static struct tcbd_spur_data spur_table_24576[] = {
	{ 180064, 2, {(0x03C3<<16)|(0x030D), (0x03C2<<16)|(0x0308)} },
	{ 184736, 2, {(0x0092<<16)|(0x033A), (0x0098<<16)|(0x0332)} },
	{ 188928, 2, {(0x00B1<<16)|(0x034F), (0x00B5<<16)|(0x034B)} },
	{ 189008, 2, {(0x00C6<<16)|(0x0366), (0x00CA<<16)|(0x0363)} },
	{ 195936, 2, {(0x0328<<16)|(0x037F), (0x0324<<16)|(0x037C)} },
	{ 196736, 2, {(0x0030<<16)|(0x030F), (0x0032<<16)|(0x0305)} },
	{ 199280, 2, {(0x000B<<16)|(0x030A), (0x000B<<16)|(0x0300)} },
	{ 204640, 2, {(0x03C3<<16)|(0x030D), (0x03C2<<16)|(0x0308)} },
	{ 205280, 2, {(0x039F<<16)|(0x031E), (0x039B<<16)|(0x0315)} },
	{ 208736, 2, {(0x03C4<<16)|(0x0312), (0x03C2<<16)|(0x0308)} },
	{ 213008, 2, {(0x0006<<16)|(0x0305), (0x0006<<16)|(0x0300)} },
	{ 213360, 2, {(0x0086<<16)|(0x032C), (0x0089<<16)|(0x0328)} },
	{ 229072, 2, {(0x038F<<16)|(0x031F), (0x038D<<16)|(0x031B)} },
	{ 237488, 2, {(0x03E2<<16)|(0x0307), (0x03E1<<16)|(0x0302)} },
	{ 1458096, 2, {(0x03E2<<16)|(0x0307), (0x03E1<<16)|(0x0302)} },
	{ 1466656, 2, {(0x006B<<16)|(0x031E), (0x006D<<16)|(0x0319)} },
	{ 1475216, 2, {(0x00D4<<16)|(0x037A), (0x00D8<<16)|(0x0377)} },
	{ 1482064, 2, {(0x0325<<16)|(0x0384), (0x0321<<16)|(0x0382)} },
	{ 1490624, 2, {(0x0389<<16)|(0x0322), (0x0387<<16)|(0x031E)} },
};

#if defined(TC317X_AGC_IN_BOOT)
static mailbox_t agc_data_table_lband[] = {
	{MBCMD_AGC_DAB_JAM, 3, 0,
	 {0x0, (195 << 16) + (23 << 8) + 24, 0x001C0012}},
};

static mailbox_t agc_table_data_vhf[] = {
	{MBCMD_AGC_DAB_JAM, 3, 0,
	 {0x0, (195 << 16) + (16 << 8) + 17, 0x005a0043}},
};
#else
static mailbox_t agc_data_table_lband[] = {
	{MBCMD_AGC_DAB_CFG, 3, 0, {0xC0ACFF44, 0x031EFF53, 0x011EFF07}},
	{MBCMD_AGC_DAB_JAM, 3, 0,
	 {0x00091223, (195 << 16) + (23 << 8) + 24, 0x001C0012}},
	{MBCMD_AGC_DAB_3, 7, 0,
	 {0x52120223, 0x58120823, 0x5c121223,
	  0x60121c23, 0x62122023, 0x68122423, 0x6a122a23}},
	{MBCMD_AGC_DAB_4, 7, 0,
	 {0x70123223, 0x30093a23, 0x32094023, 0x3a094823,
	  0x3c095223, 0x40095823, 0x42095c23}},
	{MBCMD_AGC_DAB_5, 7, 0,
	 {0x48096023, 0x4a096223, 0x52096423,
	  0x4e096823, 0x58096a23, 0x5a097023, 0x5e090812}},
	{MBCMD_AGC_DAB_6, 7, 0,
	 {0x62091212, 0x68090e12, 0x70091c12,
	  0x72092012, 0x78092412, 0x7a093012, 0x7c093812}},
	{MBCMD_AGC_DAB_7, 4, 0,
	 {0x7e093a12, 0x6f094012, 0x73094212, 0x7f094a12}},
	{MBCMD_AGC_DAB_8, 5, 0,
	 {0x30093a23, 0x32094023, 0x3a094823, 0x3c095223, 0x40095823}},
	{MBCMD_AGC_DAB_9, 5, 0,
	 {0x72123a23, 0x78124023, 0x7a124823, 0x7e125223, 0x73125823}},
	{MBCMD_AGC_DAB_A, 5, 0,
	 {0x5e090812, 0x62091212, 0x68090e12, 0x70091c12, 0x72092012}},
	{MBCMD_AGC_DAB_B, 5, 0,
	 {0x5e097223, 0x62097823, 0x68097a23, 0x70097e23, 0x72097323}},
	{MBCMD_AGC_DAB_C, 5, 0, {0x0, 0x0, 0x0, 0x0, 0x0}},
	{MBCMD_AGC_DAB_D, 5, 0, {0x0, 0x0, 0x0, 0x0, 0x0}},
	{MBCMD_AGC_DAB_E, 5, 0, {0x0, 0x0, 0x0, 0x0, 0x0}},
	{MBCMD_AGC_DAB_F, 5, 0, {0x0, 0x0, 0x0, 0x0, 0x0}},
};

static mailbox_t agc_table_data_vhf[] = {
	{MBCMD_AGC_DAB_CFG, 3, 0,
	 {0xC0ACFF44, 0x031EFF53, 0x011EFF07}},
	{MBCMD_AGC_DAB_JAM, 3, 0,
	 {0x16112243, (195 << 16) + (16 << 8) + 17, 0x005a0043}},
	{MBCMD_AGC_DAB_3, 7, 0,
	 {0x5a220243, 0x5e220843, 0x60221043,
	  0x62221443, 0x64221c43, 0x68222043, 0x6c222243}},
	{MBCMD_AGC_DAB_4, 7, 0,
	 {0x70222843, 0x2a112c43, 0x30113443, 0x34113a43,
	  0x38113e43, 0x3c114243, 0x3e114643}},
	{MBCMD_AGC_DAB_5, 7, 0,
	 {0x40114c43, 0x44115443, 0x48115a43, 0x4a115e43,
	  0x50116043, 0x54116443, 0x5a112422}},
	{MBCMD_AGC_DAB_6, 7, 0,
	 {0x5e112822, 0x60113222, 0x62113622, 0x66113a22,
	  0x6a113e22, 0x6e114222, 0x72114622}},
	{MBCMD_AGC_DAB_7, 4, 0,
	 {0x76114822, 0x7a115022, 0x7e115422, 0x7f115622}},
	{MBCMD_AGC_DAB_8, 5, 0,
	 {0x2a112c43, 0x30113443, 0x34113a43, 0x38113e43, 0x3c114243}},
	{MBCMD_AGC_DAB_9, 5, 0,
	 {0x74222c43, 0x78223443, 0x7a223a43, 0x7e223e43, 0x7f224243}},
	{MBCMD_AGC_DAB_A, 5, 0,
	 {0x5a112422, 0x5e112822, 0x60113222, 0x62113622, 0x66113a22}},
	{MBCMD_AGC_DAB_B, 5, 0,
	 {0x5a116843, 0x5e116c43, 0x60117243, 0x62117643, 0x66117a43}},
	{MBCMD_AGC_DAB_C, 5, 0, {0x0, 0x0, 0x0, 0x0, 0x0}},
	{MBCMD_AGC_DAB_D, 5, 0, {0x0, 0x0, 0x0, 0x0, 0x0}},
	{MBCMD_AGC_DAB_E, 5, 0, {0x0, 0x0, 0x0, 0x0, 0x0}},
	{MBCMD_AGC_DAB_F, 5, 0, {0x0, 0x0, 0x0, 0x0, 0x0}},
};
#endif

I32S Tcc317xOpen(I32S _moduleIndex, Tcc317xOption_t * _Tcc317xOption, func_v_iii _eventCallBackFunc)
{
	return (Tcc317xAttach(_moduleIndex, _Tcc317xOption, _eventCallBackFunc));
}

I32S Tcc317xClose(I32S _moduleIndex)
{
	I32S i;
	I32S MaxLoop = 1;

	if (Tcc317xHandle[_moduleIndex][0].opened == 0)
		return TCC317X_RETURN_FAIL;

	MaxLoop = Tcc317xCurrDiversityCount[_moduleIndex];
	for (i = 0; i < MaxLoop; i++)
		Tcc317xPeripheralOnOff(&Tcc317xHandle[_moduleIndex][i], 0);
	return (Tcc317xDetach(_moduleIndex));
}

#define XTAL_BIAS_INIT_VAL 0x02
static void Tcc317xDBGPrintDriverVersionInfo(Tcc317xHandle_t *_handle)
{
	mailbox_t MailBox;
	I32U high,mid,low,type;

	/* display code binary version */
	/* Get ASM Version */
	Tcc317xMailboxTxRx(_handle, &MailBox, MBPARA_SYS_VERSION, NULL, 0, 0);
	_handle->dspCodeVersion = MailBox.data_array[0];
	
	TcpalPrintErr((I08S *) "#----------------------\n");
	TcpalPrintErr((I08S *) "# Code Version Info (%d/%d) : 0x%08X\n",
		      _handle->moduleIndex, _handle->diversityIndex,
		      MailBox.data_array[0]);

	high = (I32U)((MailBox.data_array[0]>>24)&0xFF);
	mid = (I32U)((MailBox.data_array[0]>>16)&0xFF);
	low = (I32U)((MailBox.data_array[0]>>8)&0xFF);
	type = (I32U)(MailBox.data_array[0]&0xFF);

	TcpalPrintErr((I08S *) "# Code Version %d.%d.%d, Type[%d]\n",
		      high, mid, low, type);

	TcpalPrintErr((I08S *) "# Driver Version : %02d.%02d.%02d (0x%08X)\n",
		      ((Tcc317xCoreVersion>>16) & 0xFF), 
		      ((Tcc317xCoreVersion>>8) & 0xFF), 
		      (Tcc317xCoreVersion & 0xFF), 
		      Tcc317xCoreVersion);

	if (_handle->options.useLBAND == 1) {
		TcpalPrintErr((I08S *) "# RF Reg Version(DMB) : %02d.%02d.%02d\n",
			((Tcc317x_RF_REG_DMB_VER>>16) & 0xFF), 
			((Tcc317x_RF_REG_DMB_VER>>8) & 0xFF), 
			(Tcc317x_RF_REG_DMB_VER & 0xFF));
		TcpalPrintErr((I08S *) "# RF Reg Version(DAB) [use] : %02d.%02d.%02d\n",
			      ((Tcc317x_RF_REG_DAB_VER>>16) & 0xFF), 
			      ((Tcc317x_RF_REG_DAB_VER>>8) & 0xFF), 
			      (Tcc317x_RF_REG_DAB_VER & 0xFF));
	} else {
		TcpalPrintErr((I08S *) "# RF Reg Version(DMB) [use] : %02d.%02d.%02d\n",
			((Tcc317x_RF_REG_DMB_VER>>16) & 0xFF), 
			((Tcc317x_RF_REG_DMB_VER>>8) & 0xFF), 
			(Tcc317x_RF_REG_DMB_VER & 0xFF));
		TcpalPrintErr((I08S *) "# RF Reg Version(DAB) : %02d.%02d.%02d\n",
			      ((Tcc317x_RF_REG_DAB_VER>>16) & 0xFF), 
			      ((Tcc317x_RF_REG_DAB_VER>>8) & 0xFF), 
			      (Tcc317x_RF_REG_DAB_VER & 0xFF));
	}

	TcpalPrintErr((I08S *) "#----------------------\n");
	TcpalPrintErr((I08S *) "TCC317x driver Build ver 022 \n");
}

static I32S Tcc317xInitSub(Tcc317xHandle_t *_handle)
{
	/* stream / int / gpio / etc settings */
	Tcc317xSetStreamControl(_handle);
	Tcc317xSetInterruptControl(_handle);
	Tcc317xSetGpio(_handle);
	
	/* Restart System for stablility */
	Tcc317xSetRegSysEnable(_handle, 0);
	Tcc317xSetRegSysEnable(_handle,
			       TC3XREG_SYS_EN_EP |
			       TC3XREG_SYS_EN_DSP);
	Tcc317xSetRegSysReset(_handle,
			      TC3XREG_SYS_RESET_DSP);
	
	if(Tcc317xGetAccessMail(_handle) != 0x1ACCE551){
		TcpalPrintErr((I08S *) "# TCC317x Init: Can't receive accessmail.\n");
		return TCC317X_RETURN_FAIL;
	}
	return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xInit(I32S _moduleIndex, I08U * _coldbootData, I32S _codeSize)
{
	I32S i = 0;

	if (Tcc317xHandle[_moduleIndex][0].opened == 0)
		return TCC317X_RETURN_FAIL;

	/* asm download and addressing */
	Tcc317xInitBroadcasting(_moduleIndex, _coldbootData, _codeSize);

	for (i = 0; i < Tcc317xCurrDiversityCount[_moduleIndex]; i++) {
		Tcc317xInitSub(&Tcc317xHandle[_moduleIndex][i]);

		Tcc317xRfInit(_moduleIndex, i, 0);	/* V-Band */
		Tcc317xSetAgcTableVhf(&Tcc317xHandle[_moduleIndex][i]);
		Tcc317xHandle[_moduleIndex][i].currentBand = 0;

		/* set xtal bias key */
		Tcc317xSetRegXtalBias(&Tcc317xHandle[_moduleIndex][i],
				      XTAL_BIAS_INIT_VAL);
		Tcc317xSetRegXtalBiasKey(&Tcc317xHandle[_moduleIndex][i],
					 0x5e);

		/* display code binary version */
		Tcc317xDBGPrintDriverVersionInfo(&Tcc317xHandle[_moduleIndex][i]);
	}

	return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xInit_DiversityNormalMode(I32S _moduleIndex, I32S _diversityIndex, I08U * _coldbootData, I32S _codeSize)
{
	if (Tcc317xHandle[_moduleIndex][0].opened == 0)
		return TCC317X_RETURN_FAIL;

	/* asm download and addressing */
	Tcc317xInitBroadcasting_DiversityNormalMode (_moduleIndex, 
	    _diversityIndex, _coldbootData, _codeSize);

	Tcc317xInitSub(&Tcc317xHandle[_moduleIndex][_diversityIndex]);

	Tcc317xRfInit(_moduleIndex, _diversityIndex, 0);	/* V-Band */
	Tcc317xSetAgcTableVhf(&Tcc317xHandle[_moduleIndex][_diversityIndex]);
	Tcc317xHandle[_moduleIndex][_diversityIndex].currentBand = 0;

	/* set xtal bias key */
	Tcc317xSetRegXtalBias(&Tcc317xHandle[_moduleIndex][_diversityIndex],
			      XTAL_BIAS_INIT_VAL);
	Tcc317xSetRegXtalBiasKey(&Tcc317xHandle[_moduleIndex][_diversityIndex],
				 0x5e);

	/* display code binary version */
	Tcc317xDBGPrintDriverVersionInfo(&Tcc317xHandle[_moduleIndex][_diversityIndex]);

	return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xTune(I32S _moduleIndex, I32S _frequency,
		 I32U * _reservedOption)
{
	I32S i;
	I32S LBand = 0;
	I32U NumData[7];
	I32U FreqData[7];
	I32S MaxLoop = 1;

	if (Tcc317xHandle[_moduleIndex][0].opened == 0)
		return TCC317X_RETURN_FAIL;

	MaxLoop = Tcc317xCurrDiversityCount[_moduleIndex];

	/* stream stop */
	Tcc317xStreamStop(_moduleIndex);

	/* stream start */
	Tcc317xStreamStart(_moduleIndex);

	for (i = 0; i < MaxLoop; i++) {
		mailbox_t mailbox;

		Tcc317xHandle[_moduleIndex][i].mailboxLog =
		    MAILBOX_LOG_FREQSET_STOP;

		if (_frequency > 1000000)	/* L-Band */
			LBand = 1;
		else		/* VHF */
			LBand = 0;

		if (Tcc317xHandle[_moduleIndex][i].currentBand != LBand)
			Tcc317xRfInit(_moduleIndex, i, LBand);

		Tcc317xRfTune((void *) (&Tcc317xHandle[_moduleIndex][i]),
			      _moduleIndex, i, _frequency, LBand,
			      Tcc317xHandle[_moduleIndex][0].options.
			      oscKhz);
		Tcc317xSetFpIirCoefficient(&Tcc317xHandle[_moduleIndex][i],
					   _frequency);

		TcpalMemset(&FreqData[0], 0, sizeof(I32U) * 7);
		TcpalMemset(&NumData[0], 0, sizeof(I32U) * 7);

		NumData[0] = 1;
		FreqData[0] = _frequency;
		Tcc317xHandle[_moduleIndex][i].mailboxLog =
		    MAILBOX_LOG_FREQSET_START;
		Tcc317xMailboxTxOnly(&Tcc317xHandle[_moduleIndex][i],
				     MBPARA_SYS_NUM_FREQ, &NumData[0], 1);
		Tcc317xHandle[_moduleIndex][i].mailboxLog =
		    MAILBOX_LOG_FREQSET_START_STEP1;
		Tcc317xMailboxTxOnly(&Tcc317xHandle[_moduleIndex][i],
				     MBPARA_SYS_FREQ_0_6, &FreqData[0], 1);
		Tcc317xHandle[_moduleIndex][i].mailboxLog =
		    MAILBOX_LOG_FREQSET_START_STEP2;

		if (Tcc317xHandle[_moduleIndex][i].currentBand != LBand) {
			if (LBand == 0)
				Tcc317xSetAgcTableVhf(&Tcc317xHandle
						      [_moduleIndex][i]);
			else
				Tcc317xSetAgcTableLband(&Tcc317xHandle
							[_moduleIndex][i]);
		}

		mailbox.data_array[0] = DP_CFG_1_DATA0;
		mailbox.data_array[1] = DP_CFG_1_DATA1;
		mailbox.data_array[2] = DP_CFG_1_DATA2;
		mailbox.data_array[2] = DP_CFG_1_DATA3;
		Tcc317xMailboxTxOnly(&Tcc317xHandle[_moduleIndex][i],
				     MBCMD_DP_DAB_CFG1,
				     &mailbox.data_array[0], 4);

		mailbox.data_array[0] = DP_CFG_2_DATA0;
		mailbox.data_array[1] = DP_CFG_2_DATA1;
		mailbox.data_array[2] = DP_CFG_2_DATA2;
		mailbox.data_array[2] = DP_CFG_2_DATA3;
		Tcc317xMailboxTxOnly(&Tcc317xHandle[_moduleIndex][i],
				     MBCMD_DP_DAB_CFG2,
				     &mailbox.data_array[0], 4);

		Tcc317xHandle[_moduleIndex][i].currentBand = LBand;
	}

	InitTdmbProcess(&Tcc317xHandle[_moduleIndex][0]);

	/* Warmboot & send StartMail */
	for (i = 0; i < MaxLoop; i++) {
		Tcc317xWarmBoot(_moduleIndex, i, 1);
	}

	return TCC317X_RETURN_SUCCESS;
}

I32U Tcc317xWarmBoot(I32S _moduleIndex, I32S _diversityIndex,
		     I32S _sendStartMail)
{
	I32U data;
	mailbox_t mailbox;
	I32S retmail;

	data = 0;

	retmail =
	    Tcc317xMailboxTxRx(&Tcc317xHandle[_moduleIndex]
			       [_diversityIndex], &mailbox,
			       MBPARA_SYS_WARMBOOT, NULL, 0, 1);
	if (retmail == -1) {
		TcpalPrintErr((I08S *) "# Error - WarmBoot\n");
	}

	data = mailbox.data_array[0];

	if (data != 0x1acce551) {
		TcpalPrintErr((I08S *)
			      "# Error - Can't receive accessmail [0x%x]\n",
			      data);
	}

	if (_sendStartMail)
		Tcc317xSendStartMail(&Tcc317xHandle[_moduleIndex]
				     [_diversityIndex]);

	return data;
}

I32U Tcc317xSendStartMail(Tcc317xHandle_t * _handle)
{
	mailbox_t mailbox;
	I32S retmail;
	/* fixed value */
	I32U data = 0x11;

	if (_handle->options.powersave)
		data |= 0x02;	/* power save */
	else
		data |= 0x00;	/* no power save */

	if (_handle->options.diversityPosition == TCC317X_DIVERSITY_NONE)
		data |= 0x19;
	else if (_handle->options.diversityPosition ==
		 TCC317X_DIVERSITY_MASTER)
		data |= 0x39;
	else if (_handle->options.diversityPosition ==
		 TCC317X_DIVERSITY_LAST)
		data |= 0x35;
	else
		data |= 0x31;

	switch (_handle->options.oscKhz) {
	case 19200:
		data |= 0 << 28;
		break;
	case 24576:
		data |= 1 << 28;
		break;
	case 38400:
		data |= 2 << 28;
		break;
	default:
		TcpalPrintErr((I08S *) " ## unknown osc clock!!\n");
		break;
	}

	retmail =
	    Tcc317xMailboxTxRx(_handle, &mailbox, MBPARA_SYS_START, &data,
			       1, 0);
	TcpalPrintLog((I08S *) "# Sending Startmail[ 0x%02x]\n", data);
	if (_handle->options.diversityPosition == TCC317X_DIVERSITY_NONE) {
		TcpalPrintLog((I08S *) "# [%d][%d] No Diversity mode\n",
			      _handle->moduleIndex, _handle->diversityIndex);
	} else if (_handle->options.diversityPosition ==
		 TCC317X_DIVERSITY_MASTER) {
		TcpalPrintLog((I08S *) "# [%d][%d] Diversity Master\n",
			      _handle->moduleIndex, _handle->diversityIndex);
	} else if (_handle->options.diversityPosition ==
		 TCC317X_DIVERSITY_LAST) {
		TcpalPrintLog((I08S *) "# [%d][%d] Diversity Last\n",
			      _handle->moduleIndex, _handle->diversityIndex);
	} else {
		TcpalPrintLog((I08S *) "# [%d][%d] Diversity Mid\n",
			      _handle->moduleIndex, _handle->diversityIndex);
	}

	if (retmail == -1) {
		TcpalPrintErr((I08S *) "# Error - Sending Startmail\n");
	}

	return retmail;
}

void Tcc317xSetAgcTableVhf(Tcc317xHandle_t * _handle)
{
	mailbox_t *mbox;
	I32S i, size_table = ARRAY_SIZE(agc_table_data_vhf);

	mbox = agc_table_data_vhf;
	for (i = 0; i < size_table; i++) {
		Tcc317xMailboxTxOnly(_handle, mbox[i].cmd,
				     mbox[i].data_array, mbox[i].word_cnt);
	}
}

void Tcc317xSetAgcTableLband(Tcc317xHandle_t * _handle)
{
	mailbox_t *mbox;
	I32S i, size_table = ARRAY_SIZE(agc_data_table_lband);

	mbox = agc_data_table_lband;
	for (i = 0; i < size_table; i++) {
		Tcc317xMailboxTxOnly(_handle, mbox[i].cmd,
				     mbox[i].data_array, mbox[i].word_cnt);
	}
}


void Tcc317xSetFpIirCoefficient(Tcc317xHandle_t * _handle, I32S freq_khz)
{
	mailbox_t mailbox;
	I32S i, sizeof_spur;
	struct tcbd_spur_data *spur_table;

	spur_table = spur_table_default;
	sizeof_spur = 0;

	switch (_handle->options.oscKhz) {
	case 19200:
	case 38400:
		spur_table = spur_table_default;
		sizeof_spur = ARRAY_SIZE(spur_table_default);
		break;
	case 24576:
		spur_table = spur_table_24576;
		sizeof_spur = ARRAY_SIZE(spur_table_24576);
		break;
	default:
		break;
	}

	mailbox.data_array[0] = 0;
	mailbox.data_array[1] = 0;
	for (i = 0; i < sizeof_spur; i++) {
		if (spur_table[i].freq_khz == freq_khz) {
			mailbox.data_array[0] = spur_table[i].data[0];
			mailbox.data_array[1] = spur_table[i].data[1];
			break;
		}
	}
	Tcc317xMailboxTxOnly(_handle, MBCMD_FP_DAB_IIR,
			     &mailbox.data_array[0], 2);
}

I32S Tcc317xStreamStop(I32S _moduleIndex)
{
	I32S i;
	I32U streamDataConfig0;
	I32U bufferConfig0;
	I32S MaxLoop = 1;

	if (Tcc317xHandle[_moduleIndex][0].opened == 0)
		return TCC317X_RETURN_FAIL;
	MaxLoop = Tcc317xCurrDiversityCount[_moduleIndex];

	for (i = 0; i < MaxLoop; i++) {
		/* interrupt clr and disable */
		if (Tcc317xHandle[_moduleIndex][i].options.useInterrupt)
			Tcc317xSetRegIrqEnable(&Tcc317xHandle[_moduleIndex]
					       [i], 0);

		/* disable stream data config */
		streamDataConfig0 =
		    Tcc317xHandle[_moduleIndex][i].options.config.
		    streamDataConfig0;
		BITCLR(streamDataConfig0, TC3XREG_STREAM_DATA_ENABLE);
		Tcc317xSetRegStreamConfig0(&Tcc317xHandle[_moduleIndex][i],
					   (I08U) (streamDataConfig0));

		/* disable buffer */
		bufferConfig0 =
		    Tcc317xHandle[_moduleIndex][i].options.config.
		    bufferConfig0;
		BITCLR(bufferConfig0,
		       TC3XREG_OBUFF_CONFIG_BUFF_A_EN |
		       TC3XREG_OBUFF_CONFIG_BUFF_B_EN |
		       TC3XREG_OBUFF_CONFIG_BUFF_C_EN |
		       TC3XREG_OBUFF_CONFIG_BUFF_D_EN);
		Tcc317xSetRegOutBufferConfig(&Tcc317xHandle[_moduleIndex]
					     [i], (I08U) bufferConfig0);

		/* peripheral disable clr */
		Tcc317xPeripheralOnOff(&Tcc317xHandle[_moduleIndex][i], 0);
	}
	return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xStreamStart(I32S _moduleIndex)
{
	I32S i;
	I32U streamDataConfig0;
	I32U streamDataConfig3;
	I32S MaxLoop = 1;

	if (Tcc317xHandle[_moduleIndex][0].opened == 0)
		return TCC317X_RETURN_FAIL;
	MaxLoop = Tcc317xCurrDiversityCount[_moduleIndex];

	for (i = 0; i < MaxLoop; i++) {
		/* buffer init */
		Tcc317xSetRegOutBufferInit(&Tcc317xHandle[_moduleIndex][i],
					   Tcc317xHandle[_moduleIndex][i].
					   options.config.bufferConfig1);
		Tcc317xSetRegOutBufferConfig(&Tcc317xHandle[_moduleIndex]
					     [i],
					     Tcc317xHandle[_moduleIndex]
					     [i].options.config.
					     bufferConfig0);

		streamDataConfig0 =
		    Tcc317xHandle[_moduleIndex][i].options.config.
		    streamDataConfig0;
		streamDataConfig3 =
		    Tcc317xHandle[_moduleIndex][i].options.config.
		    streamDataConfig3;

		BITSET(streamDataConfig0, TC3XREG_STREAM_DATA_ENABLE);

		if (Tcc317xHandle[_moduleIndex][i].options.useInterrupt)
			BITSET(streamDataConfig3,
			       TC3XREG_STREAM_DATA_FIFO_INIT |
			       TC3XREG_STREAM_DATA_FIFO_EN);
		else
			BITSET(streamDataConfig3,
			       TC3XREG_STREAM_DATA_FIFO_INIT);

		/* stream fifo init */
		Tcc317xSetRegStreamConfig3(&Tcc317xHandle[_moduleIndex][i],
					   (I08U) streamDataConfig3);

		/* peripheral enable clr */
		Tcc317xPeripheralOnOff(&Tcc317xHandle[_moduleIndex][i], 1);

		/* EP reset */
		Tcc317xSetRegSysReset(&Tcc317xHandle[_moduleIndex][i],
				      TC3XREG_SYS_RESET_DSP |
				      TC3XREG_SYS_RESET_EP);

		/* enable buffer */
		Tcc317xSetRegStreamConfig0(&Tcc317xHandle[_moduleIndex][i],
					   (I08U) streamDataConfig0);

		/* interrupt enable */
		if (Tcc317xHandle[_moduleIndex][i].options.useInterrupt)
			Tcc317xSetRegIrqEnable(&Tcc317xHandle[_moduleIndex]
					       [i],
					       TC3XREG_IRQ_EN_DATAINT);
	}
	return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xAttach(I32S _moduleIndex, Tcc317xOption_t * _Tcc317xOption, func_v_iii _eventCallBackFunc)
{
	I32S i;
	I32S ret = TCC317X_RETURN_FAIL;
	I08U chipId;
	I08U progId;

	if (_Tcc317xOption->chipSelection == TCC317X_TCC3170) {
		TcpalPrintLog((I08S *) "# Driver will set [TCC3170]\n");
	} else if (_Tcc317xOption->chipSelection == TCC317X_TCC3171) {
		TcpalPrintLog((I08S *) "# Driver will set [TCC3171]\n");
	} else {
		TcpalPrintLog((I08S *) "# Driver will set [???]\n");
	}

	Tcc317xCurrBoardMode[_moduleIndex] = _Tcc317xOption->boardType;

	switch (_Tcc317xOption->boardType) {
	case TCC317X_BOARD_SINGLE:
		TcpalPrintLog((I08S *)
			      "# TCC317X Attach Success! [Single Mode]\n");
		Tcc317xCurrtModuleCount[_moduleIndex] = 1;
		Tcc317xCurrDiversityCount[_moduleIndex] = 1;
		break;
	case TCC317X_BOARD_DUAL_CHAINMODE:
		TcpalPrintLog((I08S *)
			      "# TCC317X Attach Success! [Dual Mode]\n");
		Tcc317xCurrtModuleCount[_moduleIndex] = 2;
		Tcc317xCurrDiversityCount[_moduleIndex] = 1;
		break;
	case TCC317X_BOARD_TRIPLE_CHAINMODE:
		TcpalPrintLog((I08S *)
			      "# TCC317X Attach Success! [Triple Mode]\n");
		Tcc317xCurrtModuleCount[_moduleIndex] = 3;
		Tcc317xCurrDiversityCount[_moduleIndex] = 1;
		break;
	case TCC317X_BOARD_2DIVERSITY:
		TcpalPrintLog((I08S *)
			      "# TCC317X Attach Success! [2Diversity Mode]\n");
		Tcc317xCurrtModuleCount[_moduleIndex] = 1;
		Tcc317xCurrDiversityCount[_moduleIndex] = 2;
		break;
	case TCC317X_BOARD_3DIVERSITY:
		TcpalPrintLog((I08S *)
			      "# TCC317X Attach Success! [3Diversity Mode]\n");
		Tcc317xCurrtModuleCount[_moduleIndex] = 1;
		Tcc317xCurrDiversityCount[_moduleIndex] = 3;
		break;
	case TCC317X_BOARD_4DIVERSITY:
		TcpalPrintLog((I08S *)
			      "# TCC317X Attach Success! [4Diversity Mode]\n");
		Tcc317xCurrtModuleCount[_moduleIndex] = 1;
		Tcc317xCurrDiversityCount[_moduleIndex] = 4;
		break;
	case TCC317X_BOARD_MAX:
		TcpalPrintLog((I08S *)
			      "# TCC317X Attach Success! [Default Mode]\n");
		Tcc317xCurrtModuleCount[_moduleIndex] = 1;
		Tcc317xCurrDiversityCount[_moduleIndex] = 1;
		break;
	default:
		TcpalPrintLog((I08S *)
			      "# TCC317X Attach Success! [Default Mode]\n");
		Tcc317xCurrtModuleCount[_moduleIndex] = 1;
		Tcc317xCurrDiversityCount[_moduleIndex] = 1;
		break;
	}

	TcpalMemset(&Tcc317xHandle[_moduleIndex][0], 0x00,
		    sizeof(Tcc317xHandle_t) * Tcc317xCurrDiversityCount[_moduleIndex]);
	Tcc317xSaveOption(_moduleIndex, _Tcc317xOption);

	if(Tcc317xHandle[_moduleIndex][0].opened == 0)
		Tcc317xUseInterfaceCount ++;

	/* connect command interface function */
	/* set address */
	for (i = 0; i < Tcc317xCurrDiversityCount[_moduleIndex]; i++) {
		Tcc317xHandle[_moduleIndex][i].opened = 1;
		switch (Tcc317xHandle[_moduleIndex][0].options.
			commandInterface) {
		case TCC317X_IF_I2C:
			Tcc317xHandle[_moduleIndex][i].Read =
			    Tcc317xI2cRead;
			Tcc317xHandle[_moduleIndex][i].Write =
			    Tcc317xI2cWrite;
			Tcc317xHandle[_moduleIndex][i].currentAddress =
			    TCC317X_DEFAULT_ADDRESS;
			TcpalPrintLog((I08S *)
				      "# TCC317X Interface is I2C\n");
			break;

		case TCC317X_IF_TCCSPI:
			Tcc317xHandle[_moduleIndex][i].Read =
			    Tcc317xTccspiRead;
			Tcc317xHandle[_moduleIndex][i].Write =
			    Tcc317xTccspiWrite;
			Tcc317xHandle[_moduleIndex][i].currentAddress =
			    (TCC317X_DEFAULT_ADDRESS >> 1);
			TcpalPrintLog((I08S *)
				      "# TCC317X Interface is Tccspi\n");
			break;

		default:
			TcpalPrintErr((I08S *)
				      "# TCC317X Driver Can't support your interface yet\n");
			break;
		}
		Tcc317xHandle[_moduleIndex][i].asmDownloaded = 0;
		Tcc317xHandle[_moduleIndex][i].EventCallBack = _eventCallBackFunc;
	}

	/* interface semaphore only one semaphore */
	if (pTcc317xInterfaceSema == NULL) {
		TcpalCreateSemaphore(&Tcc317xInterfaceSema,
				     (I08S *) "InterfaceSemaphore", 1);
		pTcc317xInterfaceSema = (I32U *)(&Tcc317xInterfaceSema);
	} else {
		TcpalPrintLog((I08S *)
			      "# TCC317X - Already exist interface semaphore\n");
	}

	for (i = 0; i < Tcc317xCurrDiversityCount[_moduleIndex]; i++) {
		/* mailbox semaphore */
		TcpalCreateSemaphore(&Tcc317xMailboxSema[_moduleIndex][i],
				     (I08S *)(MailSemaphoreName
				     [_moduleIndex][i]),1);
		/* op & mailbox semaphore */
		TcpalCreateSemaphore(&Tcc317xOpMailboxSema[_moduleIndex]
				     [i],
				     (I08S *)(OPMailboxSemaphoreName
				     [_moduleIndex][i]), 1);
	}

	Tcc317xGetRegChipId(&Tcc317xHandle[_moduleIndex][0], &chipId);
	TcpalPrintLog((I08S *) "# ChipID 0x%02x\n", chipId);
	Tcc317xGetRegProgramId(&Tcc317xHandle[_moduleIndex][0], &progId);
	TcpalPrintLog((I08S *) "# progId 0x%02x\n", progId);

	if (progId == 0x00) {
		TcpalPrintLog((I08S *) "# Chip is ES Version!!!\n");
	} else if (progId == 0x01) {
		TcpalPrintLog((I08S *) "# Chip is SR Version!!!\n");
	} else {
		TcpalPrintLog((I08S *) "# Chip Version is unknown!!!\n");
	}

	if (chipId == 0x37)
		ret = TCC317X_RETURN_SUCCESS;

	return ret;
}

I32S Tcc317xDetach(I32S _moduleIndex)
{
	I32S i;
	/* link Dummy Function */
	for (i = 0; i < Tcc317xCurrDiversityCount[_moduleIndex]; i++) {
		Tcc317xHandle[_moduleIndex][i].Read = DummyFunction0;
		Tcc317xHandle[_moduleIndex][i].Write = DummyFunction1;
	}


	/* interface semaphore only one semaphore */
	/* delete all drivers */
	for (i = 0; i < Tcc317xCurrDiversityCount[_moduleIndex]; i++) {
		/* mailbox semaphore */
		TcpalDeleteSemaphore(&Tcc317xMailboxSema[_moduleIndex][i]);
		/* op & mailbox semaphore */
		TcpalDeleteSemaphore(&Tcc317xOpMailboxSema[_moduleIndex]
				     [i]);
	}

	if(Tcc317xHandle[_moduleIndex][0].opened == 1)
		Tcc317xUseInterfaceCount --;

	for (i = 0; i < Tcc317xCurrDiversityCount[_moduleIndex]; i++)
		Tcc317xHandle[_moduleIndex][i].opened = 0;

	if (Tcc317xUseInterfaceCount == 0) {
		TcpalDeleteSemaphore(&Tcc317xInterfaceSema);
		pTcc317xInterfaceSema = NULL;
		Tcc317xCurrBoardMode[_moduleIndex] = TCC317X_BOARD_SINGLE;

		/* init global values */
		Tcc317xCurrtModuleCount[_moduleIndex] = 1;
		Tcc317xCurrDiversityCount[_moduleIndex] = 1;
	}
	TcpalPrintLog((I08S *) "# TCC317X Detach Success!\n");
	return TCC317X_RETURN_SUCCESS;
}

#if 0
static I32S Tcc317xCodeDownload(I32S _moduleIndex, I08U * _coldbootData,
				I32S _codeSize)
{
	I32S coldsize;
	Tcc317xBoot_t boot;
	I32S ret = TCC317X_RETURN_SUCCESS;

	if (Tcc317xColdbootParserUtil(_coldbootData, _codeSize, &boot) ==
	    TCC317X_RETURN_SUCCESS) {
		if (boot.daguDataPtr)
			Tcc317xDspTableWrite(&Tcc317xHandle[_moduleIndex]
					     [0],
					     TC317X_CODE_TABLEBASE_DAGU,
					     boot.daguDataPtr,
					     boot.daguDataSize);
		if (boot.dintDataPtr)
			Tcc317xDspTableWrite(&Tcc317xHandle[_moduleIndex]
					     [0],
					     TC317X_CODE_TABLEBASE_DINT,
					     boot.dintDataPtr,
					     boot.dintDataSize);
		if (boot.randDataPtr)
			Tcc317xDspTableWrite(&Tcc317xHandle[_moduleIndex]
					     [0],
					     TC317X_CODE_TABLEBASE_RAND,
					     boot.randDataPtr,
					     boot.randDataSize);
		if (boot.colOrderDataPtr)
			Tcc317xDspTableWrite(&Tcc317xHandle[_moduleIndex]
					     [0],
					     TC317X_CODE_TABLEBASE_COL_ORDER,
					     boot.colOrderDataPtr,
					     boot.colOrderDataSize);

		coldsize = boot.coldbootDataSize - 4;	/* Except CRC Size */
		Tcc317xDspAsmWrite(&Tcc317xHandle[_moduleIndex][0],
				   boot.coldbootDataPtr, coldsize);

		ret =
		    Tcc317xCrcCheckAddressing(_moduleIndex, _coldbootData,
					      _codeSize, &boot);
	} else {
		ret = TCC317X_RETURN_FAIL;
	}
	return ret;
}

static I32S Tcc317xCrcCheckAddressing(I32S _moduleIndex,
				      I08U * _coldbootData, I32S _codeSize,
				      Tcc317xBoot_t * _boot)
{
	I32S i;
	I32U destCrc, srcCrc;
	Tcc317xHandle_t *handle;
	I32S count = 1;
	I08U activeBit = (0x01 << 7);
	I08U address;
	I08U chipId;
	I32S ret = TCC317X_RETURN_SUCCESS;
	I08U data[4];
	I08U boardMode = TCC317X_BOARD_SINGLE;
	I32S moduleCnt = 1;
	I32S divCnt = 1;

	boardMode = (I08U)(Tcc317xCurrBoardMode[_moduleIndex]);
	moduleCnt = (I08U)(Tcc317xCurrtModuleCount[_moduleIndex]);
	divCnt = (I08U)(Tcc317xCurrDiversityCount[_moduleIndex]);

	if (_moduleIndex == 0 && boardMode == TCC317X_BOARD_DUAL_CHAINMODE) /* chain dual mode */
		count = moduleCnt;
	else	/* single or diversity or individual dual mode*/
		count = divCnt;

	for (i = count - 1; i >= 0; i--) {
		/* Configure EP */
		if (boardMode == TCC317X_BOARD_DUAL_CHAINMODE)
			handle = &Tcc317xHandle[i][0];
		else
			handle = &Tcc317xHandle[_moduleIndex][i];

		Tcc317xGetRegDmaCrc32(handle, &data[0]);
		destCrc =
		    (data[0] << 24) | (data[1] << 16) | (data[2] << 8) |
		    (data[3]);
		srcCrc =
		    HTONL(GET4BYTES
			  (&_boot->
			   coldbootDataPtr[_boot->coldbootDataSize - 4]));

		if (destCrc == srcCrc) {
			I08U data[2];

			handle->asmDownloaded = 1;


			TcpalPrintLog((I08S *) "# [%d][%d] CRC Success\n",
				      handle->moduleIndex,
				      handle->diversityIndex);
			TcpalPrintLog((I08S *)
				      "# [%d][%d] OP can access Code memory.\n",
				      handle->moduleIndex,
				      handle->diversityIndex);

			activeBit = (0x01 << 7);
			address = handle->options.address;

			TcpalPrintLog((I08S *)
				      "# Set ChipAddr [%dth module][%dth] 0x%02x Real[0x%02x]\n",
				      handle->moduleIndex,
				      handle->diversityIndex, address,
				      handle->currentAddress);

			Tcc317xSetRegChipAddress(handle,
						 (activeBit |
						  (address >> 1)));

			switch (handle->options.commandInterface) {
			case TCC317X_IF_I2C:
				handle->currentAddress = address;
				break;

			case TCC317X_IF_TCCSPI:
				handle->currentAddress = (address >> 1);
				break;

			default:
				handle->currentAddress = address;
				break;
			}

			Tcc317xGetRegChipId(handle, &chipId);
			TcpalPrintLog((I08S *) "# ChipID 0x%02x\n",
				      chipId);

			data[0] = 0x10;
			data[1] = 0x00;
			Tcc317xSetRegGpioOutput(handle, &data[0]);

			data[0] = 0x10;
			data[1] = 0x00;
			Tcc317xSetRegGpioDirection(handle, &data[0]);
		} else {
			TcpalPrintErr((I08S *)
				      "#[%d][%d] CRC Fail!!!!!! \n",
				      _moduleIndex, i);
			ret = TCC317X_RETURN_FAIL;
		}
	}

	return ret;
}
#else
static I32S Tcc317xCodeDownload(I32S _moduleIndex, I32S _diversityIndex, I08U * _coldbootData,
				I32S _codeSize, I32S _separateFlag)
{
	I32S coldsize;
	Tcc317xBoot_t boot;
	I32S ret = TCC317X_RETURN_SUCCESS;

	if (Tcc317xColdbootParserUtil(_coldbootData, _codeSize, &boot) ==
	    TCC317X_RETURN_SUCCESS) {
		if (boot.daguDataPtr)
			Tcc317xDspTableWrite(&Tcc317xHandle[_moduleIndex]
					     [_diversityIndex],
					     TC317X_CODE_TABLEBASE_DAGU,
					     boot.daguDataPtr,
					     boot.daguDataSize);
		if (boot.dintDataPtr)
			Tcc317xDspTableWrite(&Tcc317xHandle[_moduleIndex]
					     [_diversityIndex],
					     TC317X_CODE_TABLEBASE_DINT,
					     boot.dintDataPtr,
					     boot.dintDataSize);
		if (boot.randDataPtr)
			Tcc317xDspTableWrite(&Tcc317xHandle[_moduleIndex]
					     [_diversityIndex],
					     TC317X_CODE_TABLEBASE_RAND,
					     boot.randDataPtr,
					     boot.randDataSize);
		if (boot.colOrderDataPtr)
			Tcc317xDspTableWrite(&Tcc317xHandle[_moduleIndex]
					     [_diversityIndex],
					     TC317X_CODE_TABLEBASE_COL_ORDER,
					     boot.colOrderDataPtr,
					     boot.colOrderDataSize);

		coldsize = boot.coldbootDataSize - 4;	/* Except CRC Size */
		Tcc317xDspAsmWrite(&Tcc317xHandle[_moduleIndex][_diversityIndex],
				   boot.coldbootDataPtr, coldsize);

		ret =
		    Tcc317xCrcCheckAddressing(_moduleIndex, _diversityIndex, _coldbootData,
					      _codeSize, &boot, _separateFlag);
	} else {
		ret = TCC317X_RETURN_FAIL;
	}
	return ret;
}

static I32S Tcc317xCrcCheckAddressing(I32S _moduleIndex, I32S _diversityIndex,
				      I08U * _coldbootData, I32S _codeSize,
				      Tcc317xBoot_t * _boot, I32S _separateFlag)
{
	I32S i;
	I32U destCrc, srcCrc;
	Tcc317xHandle_t *handle;
	I32S count = 1;
	I08U activeBit = (0x01 << 7);
	I08U address;
	I08U chipId;
	I32S ret = TCC317X_RETURN_SUCCESS;
	I08U data[4];
	I08U boardMode = TCC317X_BOARD_SINGLE;
	I32S moduleCnt = 1;
	I32S divCnt = 1;

	boardMode = (I08U)(Tcc317xCurrBoardMode[_moduleIndex]);
	moduleCnt = (I08U)(Tcc317xCurrtModuleCount[_moduleIndex]);
	divCnt = (I08U)(Tcc317xCurrDiversityCount[_moduleIndex]);

	if(_separateFlag) {
		count = 1;
	} else {
		if (_moduleIndex == 0 && boardMode == TCC317X_BOARD_DUAL_CHAINMODE) /* chain dual mode */
			count = moduleCnt;
		else	/* single or diversity or individual dual mode*/
			count = divCnt;
	}

	for (i = count - 1; i >= 0; i--) {
		/* Configure EP */
		if(_separateFlag) {
			handle = &Tcc317xHandle[_moduleIndex][_diversityIndex];
		} else {
			if (boardMode == TCC317X_BOARD_DUAL_CHAINMODE)
				handle = &Tcc317xHandle[i][0];
			else
				handle = &Tcc317xHandle[_moduleIndex][i];
		}
		Tcc317xGetRegDmaCrc32(handle, &data[0]);
		destCrc =
		    (data[0] << 24) | (data[1] << 16) | (data[2] << 8) |
		    (data[3]);
		srcCrc =
		    HTONL(GET4BYTES
			  (&_boot->
			   coldbootDataPtr[_boot->coldbootDataSize - 4]));

		if (destCrc == srcCrc) {
			I08U data[2];

			handle->asmDownloaded = 1;

			TcpalPrintLog((I08S *) "# [%d][%d] CRC Success\n",
				      handle->moduleIndex,
				      handle->diversityIndex);
			TcpalPrintLog((I08S *)
				      "# [%d][%d] OP can access Code memory.\n",
				      handle->moduleIndex,
				      handle->diversityIndex);

			activeBit = (0x01 << 7);
			address = handle->options.address;

			TcpalPrintLog((I08S *)
				      "# Set ChipAddr [%dth module][%dth] 0x%02x Real[0x%02x]\n",
				      handle->moduleIndex,
				      handle->diversityIndex, address,
				      handle->currentAddress);

			Tcc317xSetRegChipAddress(handle,
						 (activeBit |
						  (address >> 1)));

			switch (handle->options.commandInterface) {
			case TCC317X_IF_I2C:
				handle->currentAddress = address;
				break;

			case TCC317X_IF_TCCSPI:
				handle->currentAddress = (address >> 1);
				break;

			default:
				handle->currentAddress = address;
				break;
			}

			Tcc317xGetRegChipId(handle, &chipId);
			TcpalPrintLog((I08S *) "# ChipID 0x%02x\n",
				      chipId);

			data[0] = 0x10;
			data[1] = 0x00;
			Tcc317xSetRegGpioOutput(handle, &data[0]);

			data[0] = 0x10;
			data[1] = 0x00;
			Tcc317xSetRegGpioDirection(handle, &data[0]);
		} else {
			TcpalPrintErr((I08S *)
				      "#[%d][%d] CRC Fail!!!!!! \n",
				      _moduleIndex, i);
			ret = TCC317X_RETURN_FAIL;
		}
	}

	return ret;
}

#endif


I32S Tcc317xInitBroadcasting(I32S _moduleIndex, I08U * _coldbootData,
			     I32S _codeSize)
{
	I32S i;
	I32S ret;
	I32S subret;
	I32S count;
	Tcc317xHandle_t *handle;
	I32U accessMail;
	I08U remapPc[3];
	I08U boardMode = TCC317X_BOARD_SINGLE;
	I32S moduleCnt = 1;
	I32S divCnt = 1;

	boardMode = (I08U)(Tcc317xCurrBoardMode[_moduleIndex]);
	moduleCnt = (I08U)(Tcc317xCurrtModuleCount[_moduleIndex]);
	divCnt = (I08U)(Tcc317xCurrDiversityCount[_moduleIndex]);

	if (boardMode == TCC317X_BOARD_DUAL_CHAINMODE && _moduleIndex == 1
	    && Tcc317xHandle[_moduleIndex][0].asmDownloaded == 1) {
		/* already downloaded & inited */
		Tcc317xHandle[_moduleIndex][0].mainClkKhz = 
		    Tcc317xHandle[0][0].mainClkKhz;
		ret = TCC317X_RETURN_SUCCESS;
		return ret;
	}

	/* set pll */
	Tcc317xSetPll(&Tcc317xHandle[_moduleIndex][0]);

	/* ALL Parts Disable */
	Tcc317xSetRegSysEnable(&Tcc317xHandle[_moduleIndex][0], 0);
	/* ALL Parts Reset */
	Tcc317xSetRegSysReset(&Tcc317xHandle[_moduleIndex][0],
			      TC3XREG_SYS_RESET_DSP |
			      TC3XREG_SYS_RESET_EP);
	/* EP Enable */
	Tcc317xSetRegSysEnable(&Tcc317xHandle[_moduleIndex][0],
			       TC3XREG_SYS_EN_EP);
	/* remap */
	Tcc317xSetRegRemap(&Tcc317xHandle[_moduleIndex][0], 0x00);

	/* asm code download */
	subret =
	    Tcc317xCodeDownload(_moduleIndex, 0, _coldbootData, _codeSize, 0);

	if (subret != TCC317X_RETURN_SUCCESS)
		return TCC317X_RETURN_FAIL;

	if (_moduleIndex == 0 && boardMode == TCC317X_BOARD_DUAL_CHAINMODE) /* chain dual mode */
		count = moduleCnt;
	else	/* single or diversity or individual dual mode*/
		count = divCnt;

	for (i = 0; i < count; i++) {
		if (boardMode == TCC317X_BOARD_DUAL_CHAINMODE)
			handle = &Tcc317xHandle[i][0];
		else
			handle = &Tcc317xHandle[_moduleIndex][i];

		remapPc[0] = 0x02;	/* over size of ASM 8K */
		remapPc[1] = 0x80;
		remapPc[2] = 0x00;
		Tcc317xSetRegRemapPc(handle, &remapPc[0], 3);
		Tcc317xSetRegSysReset(handle, TC3XREG_SYS_RESET_DSP);
		Tcc317xSetRegSysEnable(handle,
				       TC3XREG_SYS_EN_DSP |
				       TC3XREG_SYS_EN_EP);
		accessMail = Tcc317xGetAccessMail(handle);
		if (accessMail != 0x1ACCE551)
			return TCC317X_RETURN_FAIL;
	}

	return TCC317X_RETURN_SUCCESS;
}

I32S Tcc317xInitBroadcasting_DiversityNormalMode (I32S _moduleIndex, I32S _diversityIndex, 
			     I08U * _coldbootData, I32S _codeSize)
{
	I32S subret;
	Tcc317xHandle_t *handle;
	I32U accessMail;
	I08U remapPc[3];

	handle = (&Tcc317xHandle[_moduleIndex][_diversityIndex]);

	/* set pll */
	Tcc317xSetPll(handle);

	/* ALL Parts Disable */
	Tcc317xSetRegSysEnable(handle, 0);
	/* ALL Parts Reset */
	Tcc317xSetRegSysReset(handle,
			      TC3XREG_SYS_RESET_DSP |
			      TC3XREG_SYS_RESET_EP);
	/* EP Enable */
	Tcc317xSetRegSysEnable(handle,
			       TC3XREG_SYS_EN_EP);
	/* remap */
	Tcc317xSetRegRemap(handle, 0x00);

	/* asm code download */
	subret =
	    Tcc317xCodeDownload (_moduleIndex, _diversityIndex, 
	        _coldbootData, _codeSize, 1);

	if (subret != TCC317X_RETURN_SUCCESS)
		return TCC317X_RETURN_FAIL;

	remapPc[0] = 0x02;	/* over size of ASM 8K */
	remapPc[1] = 0x80;
	remapPc[2] = 0x00;
	Tcc317xSetRegRemapPc(handle, &remapPc[0], 3);
	Tcc317xSetRegSysReset(handle, TC3XREG_SYS_RESET_DSP);
	Tcc317xSetRegSysEnable(handle,
			       TC3XREG_SYS_EN_DSP |
			       TC3XREG_SYS_EN_EP);
	accessMail = Tcc317xGetAccessMail(handle);
	if (accessMail != 0x1ACCE551)
		return TCC317X_RETURN_FAIL;

	return TCC317X_RETURN_SUCCESS;
}

static void Tcc317xSetInterruptControl(Tcc317xHandle_t * _handle)
{
	/* init irq disable */
	Tcc317xSetRegIrqMode(_handle, _handle->options.config.irqMode);
	Tcc317xSetRegIrqClear(_handle, TC3XREG_IRQ_STATCLR_ALL);
	if (_handle->options.useInterrupt)
		Tcc317xSetRegIrqEnable(_handle, 0);
}

static void Tcc317xOutBufferConfig(Tcc317xHandle_t * _handle)
{
	I08U data[2];
	data[0] = (((PHY_MEM_ADDR_A_START >> 2) >> 8) & 0xFF);
	data[1] = ((PHY_MEM_ADDR_A_START >> 2) & 0xFF);
	Tcc317xSetRegOutBufferStartAddressA(_handle, &data[0]);

	data[0] = (((PHY_MEM_ADDR_A_END >> 2) >> 8) & 0xFF);
	data[1] = ((PHY_MEM_ADDR_A_END >> 2) & 0xFF);
	Tcc317xSetRegOutBufferEndAddressA(_handle, &data[0]);

	data[0] = (((PHY_MEM_ADDR_B_START >> 2) >> 8) & 0xFF);
	data[1] = ((PHY_MEM_ADDR_B_START >> 2) & 0xFF);
	Tcc317xSetRegOutBufferStartAddressB(_handle, &data[0]);

	data[0] = (((PHY_MEM_ADDR_B_END >> 2) >> 8) & 0xFF);
	data[1] = ((PHY_MEM_ADDR_B_END >> 2) & 0xFF);
	Tcc317xSetRegOutBufferEndAddressB(_handle, &data[0]);

	data[0] = (((PHY_MEM_ADDR_C_START >> 2) >> 8) & 0xFF);
	data[1] = ((PHY_MEM_ADDR_C_START >> 2) & 0xFF);
	Tcc317xSetRegOutBufferStartAddressC(_handle, &data[0]);

	data[0] = (((PHY_MEM_ADDR_C_END >> 2) >> 8) & 0xFF);
	data[1] = ((PHY_MEM_ADDR_C_END >> 2) & 0xFF);
	Tcc317xSetRegOutBufferEndAddressC(_handle, &data[0]);

	data[0] = (((PHY_MEM_ADDR_D_START >> 2) >> 8) & 0xFF);
	data[1] = ((PHY_MEM_ADDR_D_START >> 2) & 0xFF);
	Tcc317xSetRegOutBufferStartAddressD(_handle, &data[0]);

	data[0] = (((PHY_MEM_ADDR_D_END >> 2) >> 8) & 0xFF);
	data[1] = ((PHY_MEM_ADDR_D_END >> 2) & 0xFF);
	Tcc317xSetRegOutBufferEndAddressD(_handle, &data[0]);

	data[0] = _handle->options.config.bufferConfig30;
	data[1] = _handle->options.config.bufferConfig31;
	Tcc317xSetRegOutBufferDFifoThr(_handle, &data[0]);

	Tcc317xSetRegOutBufferConfig(_handle,
				     _handle->options.config.
				     bufferConfig0);
	Tcc317xSetRegOutBufferInit(_handle,
				   _handle->options.config.bufferConfig1);
}

extern I64U Tcc317xDiv64(I64U x, I64U y);
#define SCALE       22
#define FIXED(x)    (x<<SCALE)
#define MUL(A,B)    ((A*B)>>SCALE)
#define DIV(A,B)    (Tcc317xDiv64((A<<SCALE), B))

static void Tcc317xSetStreamControl(Tcc317xHandle_t * _handle)
{
	I08U data[5];
	I32U streamClkSpeed;
	I32U dlr;

	if(_handle->options.config.smxConfig0 & 0x80) {
		TcpalPrintLog((I08S *)
			      "# Stream mixer can not support\n");
		_handle->options.config.smxConfig0 = 0x00;
	}

	Tcc317xSetRegStreamMix(_handle,
			       _handle->options.config.smxConfig0);
	Tcc317xSetRegDiversityIo(_handle, _handle->options.config.divio);
	Tcc317xOutBufferConfig(_handle);

	data[0] = _handle->options.config.streamDataConfig0;
	data[1] = _handle->options.config.streamDataConfig1;
	data[2] = _handle->options.config.streamDataConfig2;
	data[3] = _handle->options.config.streamDataConfig3;
	Tcc317xSetRegStreamConfig(_handle, &data[0]);

	data[0] = _handle->options.config.periConfig0;
	data[1] = _handle->options.config.periConfig1;
	data[2] = _handle->options.config.periConfig2;
	data[3] = _handle->options.config.periConfig3;
	data[4] = _handle->options.config.periConfig4;
	Tcc317xSetRegPeripheralConfig(_handle, &data[0]);

	if ((_handle->options.config.periConfig0 & 0x30) == 0x10) {	/*spi ms */
		I64U temp;
		I64U temp2;
		dlr = ((_handle->options.config.periConfig1 & 0x1C) >> 2);
		temp2 = _handle->mainClkKhz;
		temp = (DIV(temp2, ((1 + dlr) << 1))) >> SCALE;
		streamClkSpeed = (I32U) (temp);
		TcpalPrintLog((I08S *)
			      "# SET SPI Clk : %d khz [DLR : %d]\n",
			      streamClkSpeed, dlr);
	} else if ((_handle->options.config.periConfig0 & 0x30) == 0x20) {	/*ts */
		I64U temp;
		I64U temp2;
		dlr = (_handle->options.config.periConfig1 & 0x07);
		temp2 = _handle->mainClkKhz;
		temp = (DIV(temp2, ((1 + dlr) << 1))) >> SCALE;
		streamClkSpeed = (I32U) (temp);
		TcpalPrintLog((I08S *)
			      "# SET TS Clk : %d khz [DLR : %d]\n",
			      streamClkSpeed, dlr);
	} else {
		;		/*none */
	}

}

void Tcc317xPeripheralOnOff(Tcc317xHandle_t * _handle, I32S _onoff)
{
	if (_onoff)
		Tcc317xSetRegPeripheralConfig0(_handle,
					       _handle->options.
					       config.periConfig0 |
					       TC3XREG_PERI_EN |
					       TC3XREG_PERI_INIT_AUTOCLR);
	else
		Tcc317xSetRegPeripheralConfig0(_handle,
					       _handle->options.config.
					       periConfig0 |
					       TC3XREG_PERI_INIT_AUTOCLR);
}

static void Tcc317xSetGpio(Tcc317xHandle_t * _handle)
{
	I08U data[2];
	data[0] = _handle->options.config.gpioAltH;
	data[1] = _handle->options.config.gpioAltL;
	Tcc317xSetRegGpioAlt(_handle, &data[0]);

	data[0] = _handle->options.config.gpioDrH;
	data[1] = _handle->options.config.gpioDrL;
	Tcc317xSetRegGpioDirection(_handle, &data[0]);

	data[0] = _handle->options.config.gpioLrH;
	data[1] = _handle->options.config.gpioLrL;
	Tcc317xSetRegGpioOutput(_handle, &data[0]);

	data[0] = _handle->options.config.gpioDrvH;
	data[1] = _handle->options.config.gpioDrvL;
	Tcc317xSetRegGpioStrength(_handle, &data[0]);

	data[0] = _handle->options.config.gpiopeH;
	data[1] = _handle->options.config.gpiopeL;
	Tcc317xSetRegGpioPull(_handle, &data[0]);
}

static I32S Tcc317xSetPll(Tcc317xHandle_t * _handle)
{
	I08U PLL6, PLL7;
	I08U pll_f, pll_m, pll_r, pll_od;
	I32U fout, fvco;

	/*
	   PLL6 = 0x0F | 0x80;
	   PLL7 = 0x06;
	 */
	PLL6 =
	    (((_handle->options.pll >> 8) & 0xFF) | 0x80);
	PLL7 = ((_handle->options.pll) & 0xFF);

	Tcc317xSetRegPll7(_handle, PLL7);
	Tcc317xSetRegPll6(_handle, PLL6);
	TcpalSleep(1);		/* 1ms (orig: 300us) */

	pll_m = ((PLL6 & 0x40) >> 6);
	pll_f = (PLL6 & 0x3f) + 1;
	pll_r = (PLL7 >> 3) + 1;
	fvco =
	    (I32U) (MUL(_handle->options.oscKhz, DIV(pll_f, pll_r)));
	pll_od = ((PLL7 & 0x06) >> 1);

	if (pll_od)
		fout = fvco >> pll_od;
	else
		fout = fvco;

	if (pll_m)
		fout = fout >> pll_m;

	_handle->mainClkKhz = fout;
	TcpalPrintLog((I08S *) "# PLLSet %dHz\n", _handle->mainClkKhz);
	return TCC317X_RETURN_SUCCESS;
}

static void Tcc317xSaveOption(I32S _moduleIndex,
			      Tcc317xOption_t * _Tcc317xOption)
{
	I32S i;
	I32S MaxLoop = 1;

	MaxLoop = Tcc317xCurrDiversityCount[_moduleIndex];
	
	for (i = 0; i < MaxLoop; i++) {
		TcpalMemcpy(&Tcc317xHandle[_moduleIndex][i].options,
			    &_Tcc317xOption[i], sizeof(Tcc317xOption_t));

		Tcc317xHandle[_moduleIndex][i].moduleIndex =
		    (I08U) (_moduleIndex);
		Tcc317xHandle[_moduleIndex][i].diversityIndex = (I08U) (i);
	}
}

static I32S Tcc317xColdbootParserUtil(I08U * pData, I32U size,
				      Tcc317xBoot_t * pBOOTBin)
{
	I32U idx;
	I32U length;
	I08U *pBin;
	I08U *daguDataPtr;
	I08U *dintDataPtr;
	I08U *randDataPtr;
	I08U *colOrderDataPtr;
	I32U BootSize[5];

	/*
	 * coldboot      0x00000001
	 * dagu          0x00000002
	 * dint          0x00000003
	 * rand          0x00000004
	 * col_order: 0x00000005
	 * sizebyte      4byte
	 * data          nbyte
	 */

	pBin = NULL;
	daguDataPtr = NULL;
	dintDataPtr = NULL;
	randDataPtr = NULL;
	colOrderDataPtr = NULL;
	TcpalMemset(BootSize, 0, sizeof(unsigned int) * 5);

	/* cold boot */
	idx = 0;
	if (pData[idx + 3] != 0x01) {
		return TCC317X_RETURN_FAIL;
	}
	idx += 4;
	length =
	    (pData[idx] << 24) + (pData[idx + 1] << 16) +
	    (pData[idx + 2] << 8) + (pData[idx + 3]);
	idx += 4;

	BootSize[0] = length;
	pBin = &pData[idx];
	idx += length;
	size -= (length + 8);

	/* dagu */
	if (pData[idx + 3] != 0x02) {
		return TCC317X_RETURN_FAIL;
	}
	idx += 4;
	length =
	    (pData[idx] << 24) + (pData[idx + 1] << 16) +
	    (pData[idx + 2] << 8) + (pData[idx + 3]);
	idx += 4;

	if (length) {
		daguDataPtr = &pData[idx];
		BootSize[1] = length;
		idx += length;
	} else {
		BootSize[1] = 0;
	}
	size -= (length + 8);

	/* dint */
	if (pData[idx + 3] != 0x03) {
		return TCC317X_RETURN_FAIL;
	}
	idx += 4;
	length =
	    (pData[idx] << 24) + (pData[idx + 1] << 16) +
	    (pData[idx + 2] << 8) + (pData[idx + 3]);
	idx += 4;

	if (length) {
		dintDataPtr = &pData[idx];
		BootSize[2] = length;
		idx += length;
	} else {
		dintDataPtr = NULL;
		BootSize[2] = 0;
	}
	size -= (length + 8);

	/* rand */
	if (pData[idx + 3] != 0x04) {
		return TCC317X_RETURN_FAIL;
	}

	idx += 4;
	length =
	    (pData[idx] << 24) + (pData[idx + 1] << 16) +
	    (pData[idx + 2] << 8) + (pData[idx + 3]);
	idx += 4;

	if (length) {
		randDataPtr = &pData[idx];
		BootSize[3] = length;
		idx += length;
	} else {
		randDataPtr = NULL;
		BootSize[3] = 0;
	}
	size -= (length + 8);

	if (size >= 8) {
		if (pData[idx + 3] != 0x05) {
			return TCC317X_RETURN_FAIL;
		}

		idx += 4;
		length =
		    (pData[idx] << 24) + (pData[idx + 1] << 16) +
		    (pData[idx + 2] << 8) + (pData[idx + 3]);
		idx += 4;

		if (length) {
			colOrderDataPtr = &pData[idx];
			BootSize[4] = length;
			idx += length;
		} else {
			colOrderDataPtr = NULL;
			BootSize[4] = 0;
		}
		size -= (length + 8);
	}

	pBOOTBin->coldbootDataPtr = pBin;
	pBOOTBin->coldbootDataSize = BootSize[0];
	pBOOTBin->daguDataPtr = daguDataPtr;
	pBOOTBin->daguDataSize = BootSize[1];
	pBOOTBin->dintDataPtr = dintDataPtr;
	pBOOTBin->dintDataSize = BootSize[2];
	pBOOTBin->randDataPtr = randDataPtr;
	pBOOTBin->randDataSize = BootSize[3];
	pBOOTBin->colOrderDataPtr = colOrderDataPtr;
	pBOOTBin->colOrderDataSize = BootSize[4];

	return TCC317X_RETURN_SUCCESS;
}

I32U Tcc317xGetCoreVersion()
{
	return Tcc317xCoreVersion;
}

I32U Tcc317xGetDspCodeVersion(I32S _moduleIndex, I32S _diversityIndex)
{
	return Tcc317xHandle[_moduleIndex][_diversityIndex].dspCodeVersion;
}

I32S Tcc317xMailboxWrite(I32S _moduleIndex, I32S _diversityIndex,
			 I32U _command, I32U * dataArray, I32S wordSize)
{
	I32S ret = TCC317X_RETURN_SUCCESS;
	ret =
	    Tcc317xMailboxTxOnly(&Tcc317xHandle[_moduleIndex]
				 [_diversityIndex], _command, dataArray,
				 wordSize);
	return ret;
}

I32S Tcc317xMailboxRead(I32S _moduleIndex, I32S _diversityIndex,
			I32U _command, mailbox_t * _mailbox)
{
	I32S ret = TCC317X_RETURN_SUCCESS;
	ret =
	    Tcc317xMailboxTxRx(&Tcc317xHandle[_moduleIndex]
			       [_diversityIndex], _mailbox, _command, NULL,
			       0, 0);
	return ret;
}

#if defined (_TCC317X_SUPPORT_AUTO_SEARCHING_)
I32S Tcc317xAutoSearch(I32S _moduleIndex, I32S _diversityIndex,
		       I32U _BandSelect, Tcc317xAutoSearch_t * _Lockstate)
{
	I32S ret, loop, index, tickMs, tickCount;
	I08U lockRegister, maxdatabuff, mask, dataskip, subLockOk;
	I32U BandSelect;
	mailbox_t mailbox;
	DmbLock_t lock;
	DmbLock_t lockSub;
	TcpalTime_t startTime;
	TcpalTime_t timeOut;

	/* Init Stack variable */
	mask = 1;
	tickMs = 10;
	dataskip = 0;
	subLockOk = 0;
	BandSelect = _BandSelect;
	_Lockstate->LockSucessCount = 0;
	ret = TCC317X_RETURN_SUCCESS;
	tickCount =
	    (AUTOSEARCH_TDMB_OFDMDETECT_LOCK *
	     AUTOSEARCH_TDMB_OFDMDETECT_RETRY) / tickMs;
	timeOut =
	    (AUTOSEARCH_TDMB_OFDMDETECT_LOCK *
	     AUTOSEARCH_TDMB_OFDMDETECT_RETRY);
	TcpalMemset(&lock, 0x00, sizeof(DmbLock_t));
	TcpalMemset(&lockRegister, 0x00, sizeof(lockRegister));
	TcpalMemset(&lockSub, 0x00, sizeof(DmbLock_t));
	TcpalMemset(_Lockstate, 0x00, sizeof(Tcc317xAutoSearch_t));

	/* Beforehand Tune */
	if (BandSelect == 1)	/* Band3 */
		Tcc317xApiChannelSelect(_moduleIndex,
					TCC317X_BANDIII_TABLE[1], NULL);
	else if (BandSelect == 2)	/* LBand */
		Tcc317xApiChannelSelect(_moduleIndex,
					TCC317X_LBAND_TABLE[1], NULL);
	else			/* Korea */
		Tcc317xApiChannelSelect(_moduleIndex,
					TCC317X_KOREA_BAND_TABLE[1], NULL);

	/* Send To BB  - Band Select Info [CMD: 0x19] */
	Tcc317xMailboxWrite(_moduleIndex, _diversityIndex,
			    ((0x19 << 11) & 0xFFFF), &BandSelect, 0x01);

	/* Wait For Scan Finish */
	startTime = TcpalGetCurrentTimeCount_ms();
	for (loop = 0; loop < tickCount; loop++) {
		Tcc317xApiRegisterRead(_moduleIndex, _diversityIndex, 0x0B,
				       &lockRegister, 1);
		lockSub.OFDM_DETECT = (lockRegister >> 5) & 0x01;
		lock.OFDM_DETECT |= lockSub.OFDM_DETECT;

		if (lock.OFDM_DETECT) {	/* OFDM_DETECT(1) Means Finshed FastScan */
			subLockOk = 1;
			//TcpalPrintLog ((I08S*) "* [DMB AutoSearchs] DMB lock Success -> OFDM DETECT: (%s) - LockTime: (%d)ms\n",
			//      lock.OFDM_DETECT==1?"Lock!":"Lock_Fail!",TcpalGetCurrentTimeCount_ms()-startTime);
			break;
		} else {
			if (TcpalGetTimeIntervalCount_ms(startTime) >
			    timeOut) {
				TcpalPrintLog((I08S *)
					      "* [DMB AutoSearch Error] DMB Fast Scan Timed Out.. - MaxTime: (%d)ms\n",
					      AUTOSEARCH_TDMB_OFDMDETECT_LOCK);
				break;
			}
		}
		TcpalSleep(tickMs);
	}

	/* AutoSearch Lock Ok */
	if (subLockOk == 1) {
		TcpalMemset(&mailbox, 0x00, sizeof(mailbox_t));
		Tcc317xMailboxRead(_moduleIndex, _diversityIndex,
				   (((0x19 << 11) | 0x001) & 0xFFFF),
				   &mailbox);

		//for(loop = 0;loop < 3; loop++)        
		//      TcpalPrintLog ((I08S*) "* [DMB AutoSearch] -MailBox [HOST] <- [BB]:Data%d - (0x%x)\n",loop ,mailbox.data_array[loop]);                          

		/* Set AutoSearch frequency */
		if ((BandSelect == SELECT_BAND3) && ((mailbox.data_array[0]) || (mailbox.data_array[1]))) {	/* BAND III */
			maxdatabuff = BANDIII_MAXDATA0;
			for (loop = 0; loop < 2; loop++) {
				for (index = 1; index < maxdatabuff;
				     index++) {
					if (1 ==
					    ((mailbox.
					      data_array[loop] & mask <<
					      index) ? 1 : 0)) {
						//TcpalPrintLog ((I08S*) "* [DMB AutoSearch] Band3 Data%d Bit Index (%d), freq: (%d)\n",loop,index,TCC317X_BANDIII_TABLE[index-1+dataskip]);
						_Lockstate->
						    Lockfrequency_Band3
						    [index - 1 +
						     dataskip] =
						    TCC317X_BANDIII_TABLE
						    [index - 1 + dataskip];
						_Lockstate->
						    LockSucessCount++;
					}
				}
				maxdatabuff = BANDIII_MAXDATA1;
				dataskip = 32;
			}
		} else if ((BandSelect == SELECT_LBAND) && (mailbox.data_array[0])) {	/* L-BAND */
			for (index = 1; index < LBAND_MAXDATA0; index++) {
				if (1 ==
				    ((mailbox.
				      data_array[0] & mask << index) ? 1 :
				     0)) {
					//TcpalPrintLog ((I08S*) "* [DMB AutoSearch] LBand Data0 Bit Index (%d),  freq: (%d)\n",index,TCC317X_LBAND_TABLE[index-1]);
					_Lockstate->
					    Lockfrequency_Lband[index -
								1] =
					    TCC317X_LBAND_TABLE[index - 1];
					_Lockstate->LockSucessCount++;
				}
			}
		} else if ((BandSelect == SELECT_KOREA) && (mailbox.data_array[0])) {	/* KOREA BAND */
			for (index = 1; index < KOREA_MAXDATA0; index++) {
				if (1 ==
				    ((mailbox.
				      data_array[0] & mask << index) ? 1 :
				     0)) {
					//TcpalPrintLog ((I08S*) "* [DMB AutoSearch] Korea Data0 Bit Index (%d),  freq: (%d)\n",index,TCC317X_KOREA_BAND_TABLE[index-1]);
					_Lockstate->
					    Lockfrequency_Kband[index -
								1] =
					    TCC317X_KOREA_BAND_TABLE[index
								     - 1];
					_Lockstate->LockSucessCount++;
				}
			}
		} else {
			TcpalPrintLog((I08S *)
				      "* [DMB AutoSearch Error] Check The Mailbox Data .. \n");
		}

	} else {
		TcpalPrintLog((I08S *)
			      "* [DMB AutoSearch Error] Fast Scan Mode Lock Fail.. \n");
	}
	return ret;
}
#endif

I32S DummyFunction0(I32S _moduleIndex, I32S _chipAddress, I08U _inputData,
		    I08U * _outData, I32S _size)
{
	TcpalPrintLog((I08S *) "# Access dummy function 0\n");
	return TCC317X_RETURN_SUCCESS;
}

I32S DummyFunction1(I32S _moduleIndex, I32S _chipAddress, I08U _address,
		    I08U * _inputData, I32S _size)
{
	TcpalPrintLog((I08S *) "# Access dummy function 1\n");
	return TCC317X_RETURN_SUCCESS;
}

#if 0 /* not support yet */
static void Tcc317xConnectCommandInterface(I32S _moduleIndex, 
					   I32S _diversityIndex,
					   I08S _commandInterface)
{
	I32U i;
	i = (I32U)(_diversityIndex);

	switch (_commandInterface) {
	case TCC317X_IF_I2C:
		Tcc317xHandle[_moduleIndex][i].Read =
		    Tcc317xI2cRead;
		Tcc317xHandle[_moduleIndex][i].Write =
		    Tcc317xI2cWrite;
		Tcc317xHandle[_moduleIndex][i].currentAddress =
		    Tcc317xHandle[_moduleIndex][i].options.address;
		Tcc317xHandle[_moduleIndex][i].originalAddress =
		    Tcc317xHandle[_moduleIndex][i].options.address;
		TcpalPrintLog((I08S *)
			      "[TCC317X] Interface is I2C\n");
		break;
	
	case TCC317X_IF_TCCSPI:
		Tcc317xHandle[_moduleIndex][i].Read =
		    Tcc317xTccspiRead;
		Tcc317xHandle[_moduleIndex][i].Write =
		    Tcc317xTccspiWrite;
		Tcc317xHandle[_moduleIndex][i].currentAddress =
		    (Tcc317xHandle[_moduleIndex][i].
		     options.address >> 1);
		Tcc317xHandle[_moduleIndex][i].originalAddress =
		    (Tcc317xHandle[_moduleIndex][i].
		     options.address >> 1);
		TcpalPrintLog((I08S *)
			      "[TCC317X] Interface is Tccspi\n");
		break;
	
	default:
		TcpalPrintErr((I08S *)
			      "[TCC317X] Driver Can't support your interface yet\n");
		break;
	}
}


I32S Tcc317xChangeToDiversityMode (I32S _mergeIndex, 
				   Tcc317xOption_t * _Tcc317xOption)
{
	I32S ret = TCC317X_RETURN_SUCCESS;
	I08U chipId;
	Tcc317xRegisterConfig_t oldConfig[2];
	Tcc317xRegisterConfig_t currConfig[2];
	I08U data[4];
	I32U tempDivCnt = 0;
	I32U i;
	
	TcpalPrintLog((I08S *)
		      "[TCC317X] Changing to 2-Diversity Mode! \n");

	/* ---------------------------------------------------	*/
	/* close dual driver !!! 				*/
	/*							*/

	Tcc317xPeripheralOnOff(&Tcc317xHandle[1][0], 0);

	/* link Dummy Function */
	Tcc317xHandle[1][0].Read = DummyFunction0;
	Tcc317xHandle[1][0].Write = DummyFunction1;

	/* Dealloc handles */
	TcpalMemset(&Tcc317xHandle[1][0], 0, sizeof(Tcc317xHandle_t));

	/* interface semaphore only one semaphore */
	/* delete all drivers */
	/* mailbox semaphore */
	TcpalDeleteSemaphore(&Tcc317xMailboxSema[1][0]);
	/* op & mailbox semaphore */
	TcpalDeleteSemaphore(&Tcc317xOpMailboxSema[1][0]);

	/* ---------------------------------------------------	*/
	/* open diversity driver !!! 				*/
	/*							*/

	tempDivCnt = Tcc317xCurrDiversityCount[1];
	Tcc317xCurrDiversityCount[0] += Tcc317xCurrDiversityCount[1];
	Tcc317xCurrDiversityCount[1] = 0;

	TcpalMemcpy(&oldConfig[0], 
		    &Tcc317xHandle[0][0].options.config,
		    sizeof(Tcc317xRegisterConfig_t));
	
	TcpalMemcpy(&oldConfig[1], 
		    &Tcc317xHandle[1][0].options.config,
		    sizeof(Tcc317xRegisterConfig_t));

	Tcc317xSaveOption(0, 0, &_Tcc317xOption[0]);
	Tcc317xSaveOption(0, 1, &_Tcc317xOption[1]);

	TcpalMemcpy(&currConfig[0], 
		    &Tcc317xHandle[0][0].options.config,
		    sizeof(Tcc317xRegisterConfig_t));

	TcpalMemcpy(&currConfig[1], 
		    &Tcc317xHandle[0][1].options.config,
		    sizeof(Tcc317xRegisterConfig_t));

	Tcc317xHandle[0][1].handleOpen = 1;
	Tcc317xConnectCommandInterface (0, 1, 
		Tcc317xHandle[0][0].
		options.commandInterface);

	TcpalCreateSemaphore(&Tcc317xMailboxSema[0][1],
			     MailSemaphoreName[0][1], 1);
	/* op & mailbox semaphore */
	TcpalCreateSemaphore(&Tcc317xOpMailboxSema[0]
			     [1], OPMailboxSemaphoreName[0][1],
			     1);
	Tcc317xGetRegChipId(&Tcc317xHandle[0][1],
			    &chipId);
	TcpalPrintLog((I08S *) "[TCC317X][0][1] ChipID 0x%02x\n", chipId);

	if (chipId != 0x33)
		ret = TCC317X_RETURN_FAIL;

	/* change BB#0 single to Diversity Master - Set gpio alt(13~21), io misc */
	Tcc317xSetChangedGpioValue (0, 0, &oldConfig[0], &currConfig[0]);
	Tcc317xSetRegIoMISC(&Tcc317xHandle[0][0], currConfig[0].ioMisc_0x16);

	/* change BB#1 single to Diversity Slave - Set gpio alt, io misc */
	/* 				buff config, gpio drv, peripheral */
	Tcc317xSetChangedGpioValue (0, 1, &oldConfig[1], &currConfig[1]);
	Tcc317xSetRegIoMISC(&Tcc317xHandle[0][1], currConfig[1].ioMisc_0x16);
	Tcc317xOutBufferConfig(&Tcc317xHandle[0][1]);
	
	data[0] = currConfig[1].periConfig_0x30;
	data[1] = currConfig[1].periConfig_0x31;
	data[2] = currConfig[1].periConfig_0x32;
	data[3] = currConfig[1].periConfig_0x33;
	Tcc317xSetRegPeripheralConfig(&Tcc317xHandle[0][1], &data[0]);

	/* tune same as master */
	for (i = 0; i < tempDivCnt; i++) {
		I32U opConfig[TCC317X_DIVERSITY_MAX][16];
		Tcc317xRfSwitching(0, i, MasterInputfrequency,
				   &Tcc317xHandle[0][i].options);

		Tcc317xRfTune(0, i, MasterInputfrequency, 6000,
			      Tcc317xHandle[0][i].options.oscKhz, 
			      &MasterTuneOption);

		Tcc317xGetOpconfigValues(0, i, &MasterTuneOption,
					 &opConfig[i][0], 
					 (I32U)(MasterInputfrequency));

		/* op configure it need dsp disable->reset->enable */
		Tcc317xSetOpConfig(0, i,
				   &opConfig[i][0], 1);
	}

	/* dsp disable to enable, ep reset & peripheral enable */
	for (i = 0; i < tempDivCnt; i++) {
		Tcc317xDspEpReopenForStreamStart(0, i);
		Tcc317xSendStartMail(&Tcc317xHandle[0][i]);
	}

	return ret;
}

I32S Tcc317xChangeToDualMode (I32S _devideIndex, 
			      Tcc317xOption_t * _Tcc317xOption)
{
	I32S ret = TCC317X_RETURN_SUCCESS;
	I32S numberOfDemodule;
	I32S rest;
	I32S i;
	I08U chipId;
	Tcc317xRegisterConfig_t oldConfig[4];
	Tcc317xRegisterConfig_t currConfig[4];
	I08U data[4];

	numberOfDemodule = Tcc317xCurrDiversityCount[0];
	rest = numberOfDemodule - _devideIndex;
	TcpalPrintLog((I08S *)
		      "[TCC317X] Changing to Dual Mode! \n");

	/* ---------------------------------------------------	*/
	/* close diversity driver !!!				*/
	/*							*/

	/* link Dummy Function */
	for(i=0; i<rest; i++)	{
		Tcc317xHandle[0][_devideIndex+i].Read = DummyFunction0;
		Tcc317xHandle[0][_devideIndex+i].Write = DummyFunction1;
	}

	/* Dealloc handles */
	TcpalMemset(&Tcc317xHandle[0][_devideIndex], 0, 
		    sizeof(Tcc317xHandle_t)*rest);

	/* interface semaphore only one semaphore */
	/* delete all drivers */
	/* mailbox semaphore */
	for(i=0; i<rest; i++)	{
		TcpalDeleteSemaphore(&Tcc317xMailboxSema[0][_devideIndex+i]);
		/* op & mailbox semaphore */
		TcpalDeleteSemaphore(&Tcc317xOpMailboxSema[0][_devideIndex+i]);
	}

	/* ---------------------------------------------------	*/
	/* open Dual driver !!!				*/
	/*							*/

	for(i=0; i<numberOfDemodule; i++)	{
		if(i<_devideIndex)
			Tcc317xCurrDiversityCount[i] = _devideIndex;
		else
			Tcc317xCurrDiversityCount[i] = rest;
	}

	for(i=0; i<numberOfDemodule; i++)
		TcpalMemcpy(&oldConfig[i], 
			    &Tcc317xHandle[0][i].options.config,
			    sizeof(Tcc317xRegisterConfig_t));

	for(i=0; i<numberOfDemodule; i++)	{
		if(i<_devideIndex)
			Tcc317xSaveOption(0, 0, &_Tcc317xOption[i]);
		else
			Tcc317xSaveOption(1, 0, &_Tcc317xOption[i]);
	}

	TcpalMemcpy(&currConfig[0], 
		    &Tcc317xHandle[0][0].options.config,
		    sizeof(Tcc317xRegisterConfig_t));

	TcpalMemcpy(&currConfig[1], 
		    &Tcc317xHandle[1][0].options.config,
		    sizeof(Tcc317xRegisterConfig_t));

	Tcc317xHandle[1][0].handleOpen = 1;
	Tcc317xConnectCommandInterface (1, 0, 
		Tcc317xHandle[1][0].
		options.commandInterface);

	TcpalCreateSemaphore(&Tcc317xMailboxSema[1][0],
			     MailSemaphoreName[1][0], 1);
	/* op & mailbox semaphore */
	TcpalCreateSemaphore(&Tcc317xOpMailboxSema[1][0], 
			     OPMailboxSemaphoreName[1][0],
			     1);
	Tcc317xGetRegChipId(&Tcc317xHandle[1][0],
			    &chipId);
	TcpalPrintLog((I08S *) "[TCC317X][BB 1] ChipID [0x%02x]\n", chipId);

	if (chipId != 0x33)
		ret = TCC317X_RETURN_FAIL;

	/* change BB#0 Diversity to Single - Set gpio alt(13~21), io misc */
	Tcc317xSetChangedGpioValue (0, 0, &oldConfig[0], &currConfig[0]);
	Tcc317xSetRegIoMISC(&Tcc317xHandle[0][0], currConfig[0].ioMisc_0x16);

	/* change BB#1 Diversity to Single - Set gpio alt, io misc */
	/*				buff config, gpio drv, peripheral */
	Tcc317xSetChangedGpioValue (1, 0, &oldConfig[1], &currConfig[1]);
	Tcc317xSetRegIoMISC(&Tcc317xHandle[1][0], currConfig[1].ioMisc_0x16);
	Tcc317xOutBufferConfig(&Tcc317xHandle[1][0]);
	
	data[0] = currConfig[1].periConfig_0x30;
	data[1] = currConfig[1].periConfig_0x31;
	data[2] = currConfig[1].periConfig_0x32;
	data[3] = currConfig[1].periConfig_0x33;
	Tcc317xSetRegPeripheralConfig(&Tcc317xHandle[1][0], &data[0]);
	Tcc317xHandle[1][0].useDefaultPLL = 1;

	return ret;
}
#endif

