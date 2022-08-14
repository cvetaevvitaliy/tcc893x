/*
 * Copyright (C) 2010 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "config.h"
#include "debug.h"
#include <platform/globals.h>
#include <platform/tcc_ckc.h>
#include <platform/reg_physical.h>
#include "tcc_ddr.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef BITCSET
#define BITCSET(X, CMASK, SMASK)	( (X) = ((((unsigned int)(X)) & ~((unsigned int)(CMASK))) | ((unsigned int)(SMASK))) )
#endif

#define addr_clk(x) (0x74000000+x)
#define addr_mem(x) (0x73000000+x)
#define time2cycle(time, tCK)		((int)((time + tCK - 1) / tCK))

#define denali_ctl(x) (*(volatile unsigned long *)addr_mem(0x500000+(x*4)))
#define denali_phy(x) (*(volatile unsigned long *)addr_mem(0x600000+(x*4)))
#define ddr_phy(x) (*(volatile unsigned long *)addr_mem(0x420400+(x*4)))


#define CKC_CHANGE_FUNC_ADDR        0x10003000
#define CKC_CHANGE_FUNC_SIZE        0x1000
#define CKC_CHANGE_ARG_BASE         (CKC_CHANGE_FUNC_ADDR + CKC_CHANGE_FUNC_SIZE)
#define CKC_CHANGE_ARG_SIZE         0x100
#define CKC_CHANGE_ARG(x)           (*(volatile unsigned long *)(CKC_CHANGE_ARG_BASE + (4*(x))))
#define CKC_CHANGE_STACK_TOP        0x10005000


#define OLD_SETTING
#define INIT_CONFIRM
#define PHY_MRS_SETTING
#define IMPROVE_DDI_PERFORMANCE
#define DBG_RESULT_TRAINING 0
//#define DQS_EXTENSION
//#define ZDEN
//#define AUTO_TRAIN_CK_CHANGE
//#define CONFIG_DRAM_AUTO_TRAINING
//#define MANUAL_TRAIN
//#define MEM_BIST

/****************************************************************************************
* FUNCTION :void InitRoutine_Start(void)
* DESCRIPTION :
* ***************************************************************************************/
volatile void InitRoutine_Start(void)
{
#ifdef MANUAL_TRAIN0
// WL delay	
	unsigned int	b0_wld = 0x00000010;
	unsigned int	b1_wld = 0x00000010;
	unsigned int	b2_wld = 0x00000010;
	unsigned int	b3_wld = 0x00000010;

// DQS gate delay
	unsigned int	b0_dqs_gd = 0x00000050;
	unsigned int	b1_dqs_gd = 0x00000050;
	unsigned int	b2_dqs_gd = 0x00000050;
	unsigned int	b3_dqs_gd = 0x00000050;
	 
	unsigned int	b0_dqs_gsl = 0x00000000;
	unsigned int	b1_dqs_gsl = 0x00000000;
	unsigned int	b2_dqs_gsl = 0x00000000;
	unsigned int	b3_dqs_gsl = 0x00000000;

// READ DQS Master delay
	unsigned int	b0_rdqsn_d = 0x00000000;
	unsigned int	b1_rdqsn_d = 0x00000000;
	unsigned int	b2_rdqsn_d = 0x00000000;
	unsigned int	b3_rdqsn_d = 0x00000000;

	unsigned int	b0_rdqs_d = 0x00000020;
	unsigned int	b1_rdqs_d = 0x00000020;
	unsigned int	b2_rdqs_d = 0x00000020;
	unsigned int	b3_rdqs_d = 0x00000020;

// READ delays
//
	unsigned int	b0_rmdl		= 0x60;
	unsigned int	b1_rmdl		= 0x60;
	unsigned int	b2_rmdl		= 0x60;
	unsigned int	b3_rmdl		= 0x60;

	unsigned int	b0_dm_rbd	= 0x00;
	unsigned int	b0_dq0_rbd 	= 0x00;
	unsigned int	b0_dq1_rbd 	= 0x00;
	unsigned int	b0_dq2_rbd 	= 0x00;
	unsigned int	b0_dq3_rbd 	= 0x00;
	unsigned int	b0_dq4_rbd 	= 0x00;
	unsigned int	b0_dq5_rbd 	= 0x00;
	unsigned int	b0_dq6_rbd 	= 0x00;
	unsigned int	b0_dq7_rbd 	= 0x00;

	unsigned int	b1_dm_rbd	= 0x00;
	unsigned int	b1_dq0_rbd 	= 0x00;
	unsigned int	b1_dq1_rbd 	= 0x00;
	unsigned int	b1_dq2_rbd 	= 0x00;
	unsigned int	b1_dq3_rbd 	= 0x00;
	unsigned int	b1_dq4_rbd 	= 0x00;
	unsigned int	b1_dq5_rbd 	= 0x00;
	unsigned int	b1_dq6_rbd 	= 0x00;
	unsigned int	b1_dq7_rbd 	= 0x00;

	unsigned int	b2_dm_rbd	= 0x00;
	unsigned int	b2_dq0_rbd 	= 0x00;
	unsigned int	b2_dq1_rbd 	= 0x00;
	unsigned int	b2_dq2_rbd 	= 0x00;
	unsigned int	b2_dq3_rbd 	= 0x00;
	unsigned int	b2_dq4_rbd 	= 0x00;
	unsigned int	b2_dq5_rbd 	= 0x00;
	unsigned int	b2_dq6_rbd 	= 0x00;
	unsigned int	b2_dq7_rbd 	= 0x00;

	unsigned int	b3_dm_rbd	= 0x00;
	unsigned int	b3_dq0_rbd 	= 0x00;
	unsigned int	b3_dq1_rbd 	= 0x00;
	unsigned int	b3_dq2_rbd 	= 0x00;
	unsigned int	b3_dq3_rbd 	= 0x00;
	unsigned int	b3_dq4_rbd 	= 0x00;
	unsigned int	b3_dq5_rbd 	= 0x00;
	unsigned int	b3_dq6_rbd 	= 0x00;
	unsigned int	b3_dq7_rbd 	= 0x00;

// WRITE DQ Master Delay
	unsigned int	b0_wdq_d = 0x00000020;
	unsigned int	b1_wdq_d = 0x00000020;
	unsigned int	b2_wdq_d = 0x00000020;
	unsigned int	b3_wdq_d = 0x00000020;
	
// WRITE delays
	unsigned int	b0_dqs_wbd	= 0x00;
	unsigned int	b0_dm_wbd	= 0x00;
	unsigned int	b0_dq0_wbd 	= 0x00;
	unsigned int	b0_dq1_wbd 	= 0x00;
	unsigned int	b0_dq2_wbd 	= 0x00;
	unsigned int	b0_dq3_wbd 	= 0x00;
	unsigned int	b0_dq4_wbd 	= 0x00;
	unsigned int	b0_dq5_wbd 	= 0x00;
	unsigned int	b0_dq6_wbd 	= 0x00;
	unsigned int	b0_dq7_wbd 	= 0x00;

	unsigned int	b1_dqs_wbd	= 0x00;
	unsigned int	b1_dm_wbd	= 0x00;
	unsigned int	b1_dq0_wbd 	= 0x00;
	unsigned int	b1_dq1_wbd 	= 0x00;
	unsigned int	b1_dq2_wbd 	= 0x00;
	unsigned int	b1_dq3_wbd 	= 0x00;
	unsigned int	b1_dq4_wbd 	= 0x00;
	unsigned int	b1_dq5_wbd 	= 0x00;
	unsigned int	b1_dq6_wbd 	= 0x00;
	unsigned int	b1_dq7_wbd 	= 0x00;

	unsigned int	b2_dqs_wbd	= 0x00;
	unsigned int	b2_dm_wbd	= 0x00;
	unsigned int	b2_dq0_wbd 	= 0x00;
	unsigned int	b2_dq1_wbd 	= 0x00;
	unsigned int	b2_dq2_wbd 	= 0x00;
	unsigned int	b2_dq3_wbd 	= 0x00;
	unsigned int	b2_dq4_wbd 	= 0x00;
	unsigned int	b2_dq5_wbd 	= 0x00;
	unsigned int	b2_dq6_wbd 	= 0x00;
	unsigned int	b2_dq7_wbd 	= 0x00;

	unsigned int	b3_dqs_wbd	= 0x00;
	unsigned int	b3_dm_wbd	= 0x00;
	unsigned int	b3_dq0_wbd 	= 0x00;
	unsigned int	b3_dq1_wbd 	= 0x00;
	unsigned int	b3_dq2_wbd 	= 0x00;
	unsigned int	b3_dq3_wbd 	= 0x00;
	unsigned int	b3_dq4_wbd 	= 0x00;
	unsigned int	b3_dq5_wbd 	= 0x00;
	unsigned int	b3_dq6_wbd 	= 0x00;
	unsigned int	b3_dq7_wbd 	= 0x00;

// Not Used
	unsigned int	b0_dqsn_rbd = 0x00;
	unsigned int	b0_dqs_rbd = 0x00;
	unsigned int	b0_dq_oebd = 0x00;
	unsigned int	b0_dqs_oebd = 0x00;

	unsigned int	b1_dqsn_rbd = 0x00;
	unsigned int	b1_dqs_rbd = 0x00;
	unsigned int	b1_dq_oebd = 0x00;
	unsigned int	b1_dqs_oebd = 0x00;

	unsigned int	b2_dqsn_rbd = 0x00;
	unsigned int	b2_dqs_rbd = 0x00;
	unsigned int	b2_dq_oebd = 0x00;
	unsigned int	b2_dqs_oebd = 0x00;

	unsigned int	b3_dqsn_rbd = 0x00;
	unsigned int	b3_dqs_rbd = 0x00;
	unsigned int	b3_dq_oebd = 0x00;
	unsigned int	b3_dqs_oebd = 0x00;
#endif

#if defined(DRAM_DDR3)
	volatile int i;
	int count = 16;
	register unsigned int tmp;
	uint nCL, nCWL;

	#define PLL0_VALUE      0x01013806	// PLL0 : 624MHz for CPU
	//#define PLL1_VALUE      0x01012C03	// PLL1 : 1.2GHz for MEM
	//#define MEMBUS_CLK      300 // = PLL1/4
#if defined(TARGET_TCC8935_UPC) || defined(TARGET_TCC8935_DONGLE)
	#define PLL1_VALUE      0x4201F406	// PLL1 : 500MHz for MEM
#else
	#define PLL1_VALUE      0x01012C06	// PLL1 : 600MHz for MEM
#endif
	#define MEMBUS_CLK      300 // = PLL1/4
	#define DDR3_CLK        (MEMBUS_CLK*2)
	#define tCK (1000000/DDR3_CLK)

//--------------------------------------------------------------------------
//Clock setting..
	
	*(volatile unsigned long *)addr_clk(0x000000) = 0x002ffff4; //cpu bus : XIN
	*(volatile unsigned long *)addr_clk(0x000004) = 0x00200014; //mem bus : XIN/2 
	*(volatile unsigned long *)addr_clk(0x000010) = 0x00200014; //io bus : XIN/2
#if defined(TSBM_ENABLE) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
	*(volatile unsigned long *)addr_clk(0x00001C) = 0x00200014; //hsio bus : XIN/2
#endif
	*(volatile unsigned long *)addr_clk(0x000020) = 0x00200014; //smu bus : XIN/2
	*(volatile unsigned long *)addr_clk(0x000030) = PLL0_VALUE; //pll0 for cpu
	*(volatile unsigned long *)addr_clk(0x000030) = 0x80000000 | PLL0_VALUE;
	*(volatile unsigned long *)addr_clk(0x000034) = PLL1_VALUE; //pll1 for mem
	*(volatile unsigned long *)addr_clk(0x000034) = 0x80000000 | PLL1_VALUE;
#if defined(TSBM_ENABLE)
	*(volatile unsigned long *)addr_clk(0x000038) = PLL1_VALUE; //pll1 for mem
	*(volatile unsigned long *)addr_clk(0x000038) = 0x80000000 | PLL1_VALUE;
#endif
	i=3200; while(i--);
	//*(volatile unsigned long *)addr_clk(0x000000) = 0x002FFFF0;  // cpu
	*(volatile unsigned long *)addr_clk(0x000000) = 0x002FFFF0;  // cpu
	//*(volatile unsigned long *)addr_clk(0x000004) = 0x00200011;  // mem bus
	*(volatile unsigned long *)addr_clk(0x000004) = 0x00200011;  // mem bus
	*(volatile unsigned long *)addr_clk(0x000010) = 0x00200011;  // io bus
#if defined(TSBM_ENABLE)
	*(volatile unsigned long *)addr_clk(0x00001C) = 0x00200032;  // hsio bus
#endif
	*(volatile unsigned long *)addr_clk(0x000020) = 0x00200011;  // smu bus
	*(volatile unsigned long *)addr_clk(0x0000E0) = 0x2000001A;  // sdmmc3 peri (24MHz)	// for emmcboot
	i=3200; while(i--);


//--------------------------------------------------------------------------
// Phy setting..

	//----------------------------------------
	// PHY RESETn (de-assert)
	denali_phy(2) = 0x1; // phy cfg resetn
	//----------------------------------------

    //----------------------------------------
    // PHY Configuration
#if defined(TARGET_TCC8935_UPC) || defined(TARGET_TCC8935_DONGLE)
	ddr_phy(0xb) = (200<<0) | (638<<18);  // PTR4
	ddr_phy(0x7) = (0x10<<0) | (0x07D0<<6) | (0x190<<21);  // PTR0
	ddr_phy(0x8) = (0x578<<0) | (0x2EE0<<16);  // PTR1
	ddr_phy(0x9) = (0x0F<<0) | (0x0F<<5) | (0x0F<<10) | (0x10<<15);  // PTR2
	ddr_phy(0xa) = (533334<<0) | (384<<20);  // PTR3
#else
	ddr_phy(0xb) = (266400<<0) | (1333<<18);  // PTR4
	ddr_phy(0x7) = (16<<0) | (1333<<6) | (334<<21);  // PTR0
	ddr_phy(0x8) = (2997<<0) | (33300<<16);  // PTR1
	ddr_phy(0x9) = (0x0F<<0) | (0x0F<<5) | (0x0F<<10) | (0x10<<15);  // PTR2
	ddr_phy(0xa) = (666000<<0) | (227<<20);  // PTR3
#endif

	#ifdef ZDEN
		ddr_phy(0x1) = 0x40050000;
	#else
		ddr_phy(0x1) = 0x00050000;
	#endif
// DQS gate VT off
/*
	ddr_phy(0x02) &= 0xffffffc0;

	ddr_phy(0x7c) = 0x5002;
	ddr_phy(0x8c) = 0x5002;
	ddr_phy(0x9c) = 0x5002;
	ddr_phy(0xAc) = 0x5002;

	ddr_phy(0x7a) = 0x10;
	ddr_phy(0x8a) = 0x10;
	ddr_phy(0x9a) = 0x10;
	ddr_phy(0xAa) = 0x10;
*/

// fail 2.5 days, output-Z DIV_6
//	ddr_phy(0xf) = 	  ( 0x0 << 13 )		// Data Slew Rate 
//			| ( 0xf << 9  )		// DQSNRES 344 ohm (def 500ohm)
//			| ( 0x7 << 5  )		// DQSRES  344 ohm (def 500ohm)		
//			| ( 0x1 << 2  )		// MDLEN
//			| ( 0x1 << 0  );	// DXODT
//	ddr_phy(0xf) = 	  ( 0x0 << 13 )		// Data Slew Rate 
//			| ( 0x9 << 9  )		// DQSNRES 688 ohm (def 500ohm)
//			| ( 0x1 << 5  )		// DQSRES  688 ohm (def 500ohm)		
//			| ( 0x1 << 2  )		// MDLEN
//			| ( 0x1 << 0  );	// DXODT

#ifdef ZDEN
// ZQ over-ride
	ddr_phy(0x60) =   ( 0x1 << 29 )		// ZCALBYP
			| ( 0x0 << 30 )		// ZCALEN
			| ( 0x1 << 28 )		// ZDEN
			| ( 0x0D << 15 )	// pull-up ODT 58ohm fix
			| ( 0x0D << 10 )	// pull-down don't care
			| ( 0x09 <<  5 )	// pull-up output Z 40.6 ohm
			| ( 0x09 <<  0 );	// pull-down don't care
#endif

//	i=3200; while(i--);

#if defined(CONFIG_DRAM_16BIT_USED)
	// 16bit
	ddr_phy(0x90) = (0x0<<0)|(0x1<<4)|(0x1<<5)|(0x1<<6)|(0x1<<14);	// DX2GCR disable, power-down all  
	ddr_phy(0xa0) = (0x0<<0)|(0x1<<4)|(0x1<<5)|(0x1<<6)|(0x1<<14);	// DX3GCR disable, power-down all  
#endif

#if defined(DQS_EXTENSION)
	ddr_phy(0x10) |= (1<<6);  // edit DQS range
	ddr_phy(0x7A) = 0x0;
	ddr_phy(0x8A) = 0x0;
	ddr_phy(0x9A) = 0x0;
	ddr_phy(0xAA) = 0x0;
#else
	#ifndef CONFIG_DRAM_AUTO_TRAINING
	ddr_phy(0x7A) = 0x50505050;	//0x20202020;
	ddr_phy(0x8A) = 0x50505050;	//0x20202020;
	ddr_phy(0x9A) = 0x50505050;	//0x20202020;
	ddr_phy(0xAA) = 0x50505050;	//0x20202020;
	#endif
	#ifdef CONFIG_ODT_TCC8931
        ddr_phy(0x61) = (3<<4) | (15<<0);
        ddr_phy(0x65) = (3<<4) | (15<<0);
        ddr_phy(0x69) = (3<<4) | (15<<0);
        ddr_phy(0x6D) = (3<<4) | (15<<0);
	#endif

	#if defined(TARGET_M805_893X_EVM) && defined(CONFIG_CHIP_TCC8937S)
	ddr_phy(0x61) = (2<<4) | (10<<0);
	ddr_phy(0x65) = (2<<4) | (10<<0);
	ddr_phy(0x69) = (2<<4) | (10<<0);
	ddr_phy(0x6D) = (2<<4) | (10<<0);
	#endif
#endif

	(*(volatile unsigned long *)0x73810010) |= 0x00008000; //Hw15; MBUSCFG.DCLKR - Bus Clock : DRAM Clock = 1:2

//--------------------------------------------------------------------------
// Timing Parameter

	if(tCK >= 2500){ /* 2.5ns, 400MHz */
		nCL = 6;
		nCWL = 5;
	}else if(tCK >= 1875){ // 1.875ns, 533..MHz
		nCL = 8;
		nCWL = 6;
	}else if(tCK >= 1500){ // 1.5 ns, 666..MHz
		if(DDR3_MAX_SPEED < DDR3_1600)
			nCL = 9;
		else
			nCL = 10;
#if defined(TARGET_BOARD_STB) && defined(CONFIG_CHIP_TCC8930)
		nCWL = 8;
#else
		nCWL = 7;
#endif
	}else if(tCK >= 1250){ // 1.25 ns, 800MHz
		nCL = 11;
		nCWL = 8;
	}else if(tCK >= 1070){ // 1.07 ns, 933..MHz
		nCL = 13;
		nCWL = 9;
	}else if(tCK >= 935){ // 0.935 ns, 1066..MHz
		nCL = 14;
		nCWL = 10;
	}

	denali_ctl(0) = 0x20410600; //DRAM_CLASS[11:8] = 6:DDR3, 4:DDR2

#ifdef OLD_SETTING
	denali_ctl(2) = time2cycle(10000, tCK); //TINIT[23:0]
	denali_ctl(3) = time2cycle(200000000, tCK); //TRST_PWRON
	denali_ctl(4) = time2cycle(500000000, tCK); //CKE_INACTIVE
	if(DDR3_AL == AL_DISABLED){ //nAL = 0;
		denali_ctl(5) = (0x1<<24 | (nCWL)<<16 | ((nCL<<1)|0)<<8 | 0x8);
	}
	else if(DDR3_AL == AL_CL_MINUS_ONE){ //nAL = nCL - 1;
		denali_ctl(5) = (0x1<<24 | (nCWL+nCL-1)<<16 | ((nCL<<1)|0)<<8 | 0x8);
	}	
	else if(DDR3_AL == AL_CL_MINUS_TWO){ //nAL = nCL - 2;
		denali_ctl(5) = (0x1<<24 | (nCWL+nCL-2)<<16 | ((nCL<<1)|0)<<8 | 0x8);
	}
#else
	denali_ctl(2) = 0x00000007; //TINIT[23:0]
	denali_ctl(3) = 0x000000a0; //TRST_PWRON
	denali_ctl(4) = 0x00000090; //CKE_INACTIVE
	if(DDR3_AL == AL_DISABLED){ //nAL = 0;
		denali_ctl(5) = (0x4<<24 | (nCWL)<<16 | ((nCL<<1)|0)<<8 | 0x0);
	}
	else if(DDR3_AL == AL_CL_MINUS_ONE){ //nAL = nCL - 1;
		denali_ctl(5) = (0x4<<24 | (nCWL+nCL-1)<<16 | ((nCL<<1)|0)<<8 | 0x0);
	}	
	else if(DDR3_AL == AL_CL_MINUS_TWO){ //nAL = nCL - 2;
		denali_ctl(5) = (0x4<<24 | (nCWL+nCL-2)<<16 | ((nCL<<1)|0)<<8 | 0x0);
	}
#endif

	denali_ctl(6) = (time2cycle(DDR3_tRAS_ps,tCK)<<24 | time2cycle(DDR3_tRC_ps,tCK)<<16 | time2cycle(DDR3_tRRD_ps,tCK)<<8 | DDR3_tCCD_ck);
	denali_ctl(7) = (DDR3_tMRD_ck<<24 | time2cycle(DDR3_tRTP_ps,tCK)<<16 | time2cycle(DDR3_tRP_ps,tCK)<<8 | (time2cycle(DDR3_tWTR_ps,tCK)+1));
	if(time2cycle(DDR3_tMOD_ps,tCK) < DDR3_tMOD_ck)
		denali_ctl(8) = (time2cycle(DDR3_tRAS_MAX_ps,tCK)<<8 | DDR3_tMOD_ck);
	else
		denali_ctl(8) = (time2cycle(DDR3_tRAS_MAX_ps,tCK)<<8 | time2cycle(DDR3_tMOD_ps,tCK));
	denali_ctl(9) = ((time2cycle(DDR3_tCKE_ps,tCK)+1)<<8 | time2cycle(DDR3_tCKE_ps,tCK));
	denali_ctl(10) = (time2cycle(DDR3_tWR_ps,tCK)<<24 | time2cycle(DDR3_tRCD_ps,tCK)<<16 | 1<<8 | 1);
	denali_ctl(11) = (1<<24 | DDR3_tDLLK_ck<<8 | (time2cycle(DDR3_tWR_ps,tCK)+nCL));
	denali_ctl(12) = (1<<16 | time2cycle(DDR3_tFAW_ps,tCK)<<8 | 3);
	denali_ctl(13) = time2cycle(DDR3_tRP_ps,tCK)+1;
	denali_ctl(14) = (time2cycle(DDR3_tRFC_ps,tCK)<<16 | 1<<8 | 0);
	denali_ctl(15) = time2cycle(DDR3_tREFI_ps,tCK);
	denali_ctl(16) = (time2cycle(DDR3_tXPDLL_ps,tCK)<<16 | time2cycle(DDR3_tXP_ps,tCK)); // DDR3 Only
	denali_ctl(17) = 0x0;//(6<<16 | 2); //TXARD[-0] = 0x2, TXARDS[-16] = 0x6 // DDR2 only
	denali_ctl(18) = (time2cycle(DDR3_tXS_ps,tCK)<<16 | DDR3_tXSDLL_ck);
	denali_ctl(19) = (1<<16); //ENABLE_QUICK_SREFRESH = 0x1
	denali_ctl(20) = (time2cycle(DDR3_tCKSRX_ps,tCK)<<16 | time2cycle(DDR3_tCKSRE_ps,tCK)<<8);
	denali_ctl(21) = 0; denali_ctl(24) = 0; denali_ctl(25) = 0; denali_ctl(26) = 0;

//--------------------------------------------------------------------------
// MRS Setting

	// MRS0
	tmp = DDR3_BURST_LEN | (DDR3_READ_BURST_TYPE<<3);

	if(nCL < 5)
		tmp |= ((5-4)<<4);
	else if(nCL > 11)
		tmp |= ((11-4)<<4);
	else
		tmp |= ((nCL-4)<<4);

	if(time2cycle(DDR3_tWR_ps,tCK) <= 5) // Write Recovery for autoprecharge
		tmp |= WR_5<<9;
	else if(time2cycle(DDR3_tWR_ps,tCK) == 6)
		tmp |= WR_6<<9;
	else if(time2cycle(DDR3_tWR_ps,tCK) == 7)
		tmp |= WR_7<<9;
	else if(time2cycle(DDR3_tWR_ps,tCK) == 8)
		tmp |= WR_8<<9;
	else if((time2cycle(DDR3_tWR_ps,tCK) == 9) || (time2cycle(DDR3_tWR_ps,tCK) == 10))
		tmp |= WR_10<<9;
	else if(time2cycle(DDR3_tWR_ps,tCK) >= 11)
		tmp |= WR_12<<9;

	tmp	|= (1<<8); // DLL Reset

	denali_ctl(27) = tmp<<8;
	BITCSET(denali_ctl(30), 0x0000FFFF, tmp);
#ifdef PHY_MRS_SETTING
	ddr_phy(0x15) = tmp;
#endif

	// MRS1
	BITCSET(denali_ctl(28), 0x0000FFFF, (DDR3_AL<<3) | (DDR3_ODT<<2) | (DDR3_DIC<<1));
	BITCSET(denali_ctl(30), 0xFFFF0000, ((DDR3_AL<<3) | (DDR3_ODT<<2) | (DDR3_DIC<<1))<<16);
#ifdef PHY_MRS_SETTING
	ddr_phy(0x16) = (DDR3_AL<<3) | (DDR3_ODT<<2) | (DDR3_DIC<<1);
#endif

	// MRS2
	tmp = 0;
	if(nCWL <= 5)
		tmp |= 0;
	else if(nCWL == 6)
		tmp |= 1<<3;
	else if(nCWL == 7)
		tmp |= 2<<3;
	else if(nCWL >= 8)
		tmp |= 3<<3;

	BITCSET(denali_ctl(28), 0xFFFF0000, tmp<<16);
	BITCSET(denali_ctl(31), 0x0000FFFF, tmp);
#ifdef PHY_MRS_SETTING
	ddr_phy(0x17) = tmp;
#endif

	// MRS3
	BITCSET(denali_ctl(29), 0xFFFF0000, 0<<16);
	BITCSET(denali_ctl(32), 0x0000FFFF, 0);
#ifdef PHY_MRS_SETTING
	ddr_phy(0x18) = 0;
#endif

//--------------------------------------------------------------------------

#ifndef OLD_SETTING
	//BIST Start
	BITCLR(denali_ctl(32), 1<<16); //BIST_GO = 0x0
	denali_ctl(33) = (1<<16)|(1<<8); //BIST_ADDR_CHECK[-16] = 0x1, BIST_DATA_CHECK[-8] = 0x1
	//BIST End
#endif
	denali_ctl(34) = 0; denali_ctl(35) = 0; denali_ctl(36) = 0; denali_ctl(37) = 0;

	denali_ctl(38) = (DDR3_tZQOPER_ck<<16 | DDR3_tZQINIT_ck);
	denali_ctl(39) = (0x2<<24 | DDR3_tZQCS_ck); //ZQCS, ZQ_ON_SREF_EXIT
	denali_ctl(40) = 0x1<<16; //ZQCS_ROTATE=0x1, REFRESH_PER_ZQ=0x0
	denali_ctl(41) = 0xFF<<24|DDR3_APBIT<<16; //AGE_COUNT = 0xff
#ifdef IMPROVE_DDI_PERFORMANCE
	denali_ctl(42) = 0;
	denali_ctl(43) = 0;
#else
	denali_ctl(42) = 0x1<<24|0x1<<16|0x1<<8|0xFF; //COMMAND_AGE_CCOUNT = 0xff, ADDR_CMP_EN = 0x1, BANK_SPLIT_EN = 0x1 PLACEMENT_EN = 0x1
	denali_ctl(43) = 0x1<<16|0x1<<8|0x1; //PRIORITY_EN = 0x1, RW_SAME_EN = 0x1,SWAP_EN = 0x1
#endif

#if defined(CONFIG_DRAM_16BIT_USED)
	if(DDR3_LOGICAL_CHIP_NUM == 2)
		denali_ctl(44) = (0x1<<24|0xc<<16|0x3<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
	else
		denali_ctl(44) = (0x1<<24|0xc<<16|0x1<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
#else
	if(DDR3_LOGICAL_CHIP_NUM == 2)
		denali_ctl(44) = (0x0<<24|0xc<<16|0x3<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
	else
		denali_ctl(44) = (0x0<<24|0xc<<16|0x1<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
#endif

	denali_ctl(45) = 0x0; //Q_FULLNESS = 0x000
#ifndef OLD_SETTING
	denali_ctl(47) = 0x0; //INT_ACK = 0x000000
#endif
	denali_ctl(48) = 0x0;

//--------------------------------------------------------------------------
// ODT Setting

#ifdef OLD_SETTING
	BITCSET(denali_ctl(64), 0x00030000, 0x0<<16); //ODT_RD_MAP_CS0
	if(DDR3_LOGICAL_CHIP_NUM == 2)
		BITCSET(denali_ctl(65), 0x00000003, 0x0<<0); //ODT_RD_MAP_CS1

	if(DDR3_PINMAP_TYPE == 0 || DDR3_PINMAP_TYPE == 1){
		BITCSET(denali_ctl(64), 0x03000000, 0x1<<24); //ODT_WR_MAP_CS0
		if(DDR3_LOGICAL_CHIP_NUM == 2)
			BITCSET(denali_ctl(65), 0x00000300, 0x1<<8); //ODT_WR_MAP_CS1
	} else {
		BITCSET(denali_ctl(64), 0x03000000, 0x3<<24); //ODT_WR_MAP_CS0
		if(DDR3_LOGICAL_CHIP_NUM == 2)
			BITCSET(denali_ctl(65), 0x00000300, 0x3<<8); //ODT_WR_MAP_CS1
	}
#else
	if(DDR3_LOGICAL_CHIP_NUM == 1){
		BITCSET(denali_ctl(64), 0x00030000, 0x2<<16); //ODT_RD_MAP_CS0
		BITCSET(denali_ctl(64), 0x03000000, 0x1<<24); //ODT_WR_MAP_CS0
		BITCSET(denali_ctl(65), 0x00000003, 0x1<<0); //ODT_RD_MAP_CS1
		BITCSET(denali_ctl(65), 0x00000300, 0x2<<8); //ODT_WR_MAP_CS1
	}else{
		BITCSET(denali_ctl(64), 0x00030000, 0x0<<16); //ODT_RD_MAP_CS0
		BITCSET(denali_ctl(64), 0x03000000, 0x1<<24); //ODT_WR_MAP_CS0
		BITCSET(denali_ctl(65), 0x00000003, 0x0<<0); //ODT_RD_MAP_CS1
		BITCSET(denali_ctl(65), 0x00000300, 0x1<<8); //ODT_WR_MAP_CS1
	}
#endif
	BITCSET(denali_ctl(65), 0x000F0000, 0x2<<16); //ADD_ODT_CLK_R2W_SAMECS = 0x2
	denali_ctl(66) = 0x1<<24|0x1<<16|0x2<<8|0x2; //ADD_ODT_CLK_DIFFTYPE_DIFFCS = 0x2, ADD_ODT_CLK_SAMETYPE_DIFFCS = 0x2, R2R_DIFFCS_DLY = 0x1, R2W_DIFFCS_DLY = 0x1
	denali_ctl(67) = 0x2<<24|0x0<<16|0x1<<8|0x1; //W2R_DIFFCS_DLY = 0x1, W2W_DIFFCS_DLY = 0x1, R2R_SAMECS_DLY = 0x0, R2W_SAMECS_DLY = 0x2
	BITCSET(denali_ctl(68), 0x00000707, 0x0<<8|0x0); //W2R_SAMECS_DLY = 0x0, W2W_SAMECS_DLY = 0x0
	denali_ctl(69) = 0; denali_ctl(71) = 0;
	denali_ctl(72) = 0x0<<16|0x28<<8|0x19; //WLDQSEN = 0x19, WLMRD = 0x28, WRLVL_EN = 0x0
	denali_ctl(73) = 0;
	denali_ctl(74) = (0x1<<8); //WRLVL_DELAY_0 = 0x0001, WRLVL_REG_EN = 0x00
	denali_ctl(75) = (0x1<<16|0x1); //WRLVL_DELAY_2 = 0x0001, WRLVL_DELAY_1 = 0x0001
	denali_ctl(76) = 0x1; //RDLVL_GATE_REQ = 0x00, RDLVL_REQ = 0x00, WRLVL_DELAY_3 = 0x0001
	denali_ctl(77) = 0; denali_ctl(78) = 0; denali_ctl(80) = 0;
	denali_ctl(81) = 0x1<<16|0x2222; //RDLVL_GATE_DELAY_0 = 0x1, RDLVL_DELAY_0 = 0x2222
	denali_ctl(83) = 0x0<<16; //RDLVL_OFFSET_DELAY_1 = 0x0000
	denali_ctl(84) = 0x2222<<8; //RDLVL_DELAY_1 = 0x2222, RDLVL_OFFSET_DIR_1 = 0x00
	denali_ctl(85) = 0x1; //RDLVL_GATE_DELAY_1 = 0x0001
	denali_ctl(87) = 0; //RDLVL_OFFSET_DIR_2 = 0x00, RDLVL_OFFSET_DELAY_2 = 0x0000
#ifdef OLD_SETTING
	denali_ctl(88) = 0x2121; //RDLVL_GATE_DELAY_2 = 0x0000, RDLVL_DELAY_2 = 0x2121
#else
	denali_ctl(88) = 0x1<<16|0x2222; //RDLVL_GATE_DELAY_2 = 0x0001, RDLVL_DELAY_2 = 0x2222
#endif
	denali_ctl(90) = 0;
#ifdef OLD_SETTING
	denali_ctl(91) = 0x2121<<8; //RDLVL_DELAY_3 = 0x2121, RDLVL_OFFSET_DIR_3 = 0x00
	denali_ctl(92) = 0xFFFF<<16; //AXI0_EN_SIZE_LT_WIDTH_INSTR = 0xffff, RDLVL_GATE_DELAY_3 = 0x0000
#else
	denali_ctl(91) = 0x2222<<8; //RDLVL_DELAY_3 = 0x2222, RDLVL_OFFSET_DIR_3 = 0x00
	denali_ctl(92) = 0xFFFF<<16|0x1; //AXI0_EN_SIZE_LT_WIDTH_INSTR = 0xffff, RDLVL_GATE_DELAY_3 = 0x0001
#endif
	denali_ctl(93) = 0x8<<8|0x8; //AXI0_R_PRIORITY = 0x8. AXI0_W_PRIORITY = 0x8
	denali_ctl(94) = 0;


//--------------------------------------------------------------------------
// DFI Timing

	BITCSET(denali_ctl(95), 0x3F0000FF, 0xd<<24); //TDFI_PHY_RDLAT = 0xd <- MOD_THIS (0x0d, FIXED) 0x0c, DLL_RST_ADJ_DLY = 0x00
	BITCLR(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x0
	denali_ctl(97) = 0x200<<16|0x1450; //TDFI_CTRLUPD_MAX = 0x1450, TDFI_PHYUPD_TYPE0 = 0x200
	denali_ctl(98) = 0x200<<16|0x200; //TDFI_PHYUPD_TYPE1 = 0x200, TDFI_PHYUPD_TYPE2 = 0x200
	denali_ctl(99) = 0x1450<<16|0x200; //TDFI_PHYUPD_TYPE3 = 0x200, TDFI_PHYUPD_RESP = 0x1450
	denali_ctl(100) = 0x00006590; //TDFI_CTRLUPD_INTERVAL = 0x00006590
	denali_ctl(101) = (0x00<<24|0x02<<16|(nCWL-3+(nCWL%2))<<8|(nCL-3+(nCL%2)));
	denali_ctl(102) = 0x3<<24|0x8000<<8|0x1; //TDFI_DRAM_CLK_ENABLE = 0x1, DFI_WRLVL_MAX_DELAY = 0x8000, TDFI_WRLVL_EN = 0x3
	denali_ctl(103) = 0x4<<16|0x7<<8|0x3; //TDFI_WRLVL_DLL = 0x3, TDFI_WRLVL_LOAD = 0x7, TDFI_WRLVL_RESPLAT = 0x4
	denali_ctl(104) = 0xA; //TDFI_WRLVL_WW = 0xa
	denali_ctl(105) = 0; denali_ctl(106) = 0;
	denali_ctl(107) = 0x10<<16|0xFFFF; //RDLVL_MAX_DELAY = 0xffff, RDLVL_GATE_MAX_DELAY = 0x10
	denali_ctl(108) = 0x1a<<24|0x7<<16|0x3<<8|0x3; //TDFI_RDLVL_EN = 0x3, TDFI_RDLVL_DLL = 0x3, TDFI_RDLVL_LOAD = 0x7, TDFI_RDLVL_RESPLAT = 0x1a 
	denali_ctl(109) = 0xF; //TDFI_RDLVL_RR = 0xf
	denali_ctl(110) = 0; denali_ctl(111) = 0; denali_ctl(112) = 0; denali_ctl(113) = 0; denali_ctl(114) = 0;
	denali_ctl(115) = 0x2<<8|0x4; //RDLVL_DQ_ZERO_COUNT = 0x4, RDLVL_GATE_DQ_ZERO_COUNT = 0x2
	denali_ctl(117) = 0;
	denali_ctl(118) = 0; //ODT_ALT_EN = 0x0
	denali_ctl(119) = 0; //LP_AUTO_PD_IDLE = 0x0000, LP_AUTO_MEM_GATE_EN = 0x00
	denali_ctl(120) = 0x00000040; //REFRESH_PER_ZQ = 0x00000040
	denali_ctl(121) = 0x1<<24|(11-DDR3_COLBITS)<<16|(16-DDR3_ROWBITS)<<8|(3-DDR3_BANKBITS); //RW_SAME_PAGE_EN = 0x1, COL_DIFF = 0x01, ROW_DIFF = 0x01, BANK_DIFF = 0x00
	denali_ctl(122) = 0x0b<<24|0x03<<16|0x01<<8|0x01; //NUM_Q_ENTRIES_ACT_DISABLE = 0x0b, DISABLE_RW_GROUP_W_BNK_CONFLICT = 0x03, bReg.W2R_SPLIT_EN = 0x01, CS_SAME_EN = 0x01
	denali_ctl(123) = 0x01<<24|0x00<<16|0x00<<8|0x00; //CTRLUPD_REQ_PER_AREF_EN = 0x01, CTRLUPD_REQ = 0x00, IN_ORDER_ACCEPT = 0x00, DISABLE_RD_INTERLEAVE = 0x00
	denali_ctl(124) = 0x1<<8|0x06; //TDFI_PHY_WRDATA = 1,TODTL_2CMD = 0x06

//--------------------------------------------------------------------------
// Start denali MC init

#ifndef INIT_CONFIRM
	if(DDR3_LOGICAL_CHIP_NUM == 1){
	#ifdef ZDEN
		ddr_phy(0x1) = 0x40050001; // Wait for Synop. PHY init done...
	#else
		ddr_phy(0x1) = 0x00050001; // Wait for Synop. PHY init done...
	#endif
	}
#endif

	BITSET(denali_ctl(0), 1); //START = 1

#ifndef INIT_CONFIRM
	if(DDR3_LOGICAL_CHIP_NUM == 2){
#endif
	
	#ifdef	ZDEN	
		while((ddr_phy(0x4)&0x7) != 0x7); // Wait for Synop. PHY init done...
	#else
		while((ddr_phy(0x4)&0xf) != 0xf); // Wait for Synop. PHY init done...
	#endif
		ddr_phy(0x1) = 0x00050001; // PIR
		while(!(denali_ctl(46)&0x20)); // PHY Init
#ifndef INIT_CONFIRM
	}
#endif



/*
//	ZQ	
//	1 : 512
	
	i=3200; while(i--);
//	ddr_phy(0x61) = 0x0;  
	ddr_phy(0x60) =   ( 1 << 28 ) 		// ZDEN
			| ( 0x08 << 21 )	// pull-up ODT	
			| ( 0x6a << 14 )	// pull-dn ODT	
			| ( 0x1a <<  7 )	// pull-up DRV	
			| ( 0x00 <<  0 );	// pull-dn DRV	
	i=3200; while(i--);
*/

#ifdef CONFIG_DRAM_AUTO_TRAINING

// 1CS
	ddr_phy(0x1a) &= 0xf1ffff7f;	// no data compare	
// 2CS
//	ddr_phy(0x1a) &= 0xf3ffff7f;	// no data compare	
//
	

	i=3200; while(i--);

phy_wl_t:
	ddr_phy(0x01) |= (0x1 << 9);
	ddr_phy(0x01) |= 0x1;
	count--;

	while( ( ddr_phy(0x04) & 0x21 ) != 0x21 );

	if( ( ddr_phy(0x04) & 0x00200021 ) == 0x00200021 && count > 0 )
		goto phy_wl_t;

	count = 16;
	i=3200; while(i--);


phy_dqs_t:
	ddr_phy(0x01) |= (0x1 << 10);
	ddr_phy(0x01) |= 0x1;
	count--;

	while( ( ddr_phy(0x04) & 0x41 ) != 0x41 );

	if( ( ddr_phy(0x04) & 0x00400041 ) == 0x00400041 && count > 0 )
		goto phy_dqs_t;

	count = 16;
	i=3200; while(i--);

phy_wla_t:
	ddr_phy(0x01) |= (0x1 << 11);
	ddr_phy(0x01) |= 0x1;
	count--;

	while( ( ddr_phy(0x04) & 0x81 ) != 0x81 );

	if( ( ddr_phy(0x04) & 0x00800081 ) == 0x00800081 && count > 0 )
		goto phy_wla_t;

	count = 16;
	i=3200; while(i--);

phy_rddskw_t:
	ddr_phy(0x01) |= (0x1 << 12);
	ddr_phy(0x01) |= 0x1;
	count--;

	while( ( ddr_phy(0x04) & 0x101 ) != 0x101 );

	if( ( ddr_phy(0x04) & 0x01000101 ) == 0x01000101 && count > 0 )
		goto phy_rddskw_t;

	count = 16;
	i=3200; while(i--);


phy_wrdskw_t:
	ddr_phy(0x01) |= (0x1 << 13);
	ddr_phy(0x01) |= 0x1;
	count--;

	while( ( ddr_phy(0x04) & 0x201 ) != 0x201 );

	if( ( ddr_phy(0x04) & 0x02000201 ) == 0x02000201 && count > 0 )
		goto phy_rddskw_t;

	count = 16;
	i=3200; while(i--);


phy_rdeye_t:
	ddr_phy(0x01) |= (0x1 << 14);
	ddr_phy(0x01) |= 0x1;
	count--;

	while( ( ddr_phy(0x04) & 0x401 ) != 0x401 );

	if( ( ddr_phy(0x04) & 0x04000401 ) == 0x04000401 && count > 0 )
		goto phy_rdeye_t;

	count = 16;
	i=3200; while(i--);


phy_wreye_t:
	ddr_phy(0x01) |= (0x1 << 15);
	ddr_phy(0x01) |= 0x1;
	count--;

	while( ( ddr_phy(0x04) & 0x801 ) != 0x801 );

	if( ( ddr_phy(0x04) & 0x08000801 ) == 0x08000801 && count > 0 )
		goto phy_wreye_t;

	count = 16;
	i=3200; while(i--);


// ZQ over-ride
//	ddr_phy(0x60) =   ( 0x0 << 29 )		// ZCALBYP
//			| ( 0x1 << 30 )		// ZCALEN
//			| ( 0x1 << 28 )		// ZDEN
//			| ( 0x0D << 15 )	// pull-up ODT 58ohm fix
//			| ( 0x0D << 10 )	// pull-down don't care
//			| ( 0x09 <<  5 )	// pull-up output Z 40.6 ohm
//			| ( 0x08 <<  0 );	// pull-down don't care

	i=3200; while(i--);

//	PGCR1 -
//	[12:11] : LPFDEPTH
//	[14:13] : FDEPTH
//	ddr_phy(0x03) = ddr_phy(0x03) | ( 0x3 << 11 ) | ( 0x3 << 13 );


#endif

// INIT
#ifdef MANUAL_TRAIN0
	
// VT Drift Disable
	ddr_phy(0x02) &= 0xffffffc0;
	ddr_phy(0x03) |= (0x1 << 26);
	 
// DX?LCDLR0 : WL
//      [31:24] : RANK3 Write Leveling Delay
//      [23:16] : RANK2 Write Leveling Delay
//      [15:08] : RANK1 Write Leveling Delay
//      [07:00] : RANK0 Write Leveling Delay
ddr_phy(0x78) = b0_wld;
ddr_phy(0x88) = b1_wld;
ddr_phy(0x98) = b2_wld;
ddr_phy(0xA8) = b3_wld;


// DX?GTR       : DQS
//      [19:18] : RANK3 Write Leveling System Latency : 01 -> Write Latency = WL (00)
//      [17:16] : RANK2 Write Leveling System Latency : 01 -> Write Latency = WL (00)
//      [15:14] : RANK1 Write Leveling System Latency : 01 -> Write Latency = WL (01)
//      [13:12] : RANK0 Write Leveling System Latency : 01 -> Write Latency = WL (01)

//      [11:09] : RAND3 DQS Gatng System Latency
//      [08:06] : RAND2 DQS Gatng System Latency
//      [05:03] : RAND1 DQS Gatng System Latency
//      [02:00] : RAND0 DQS Gatng System Latency (11)
ddr_phy(0x7c) = 0x00005000 | b0_dqs_gsl;
ddr_phy(0x8c) = 0x00005000 | b1_dqs_gsl;
ddr_phy(0x9c) = 0x00005000 | b2_dqs_gsl;
ddr_phy(0xAc) = 0x00005000 | b3_dqs_gsl;

// DX?LCDLR2 : DQS
//      [31:24] : RANK3 Read DQS Gating Delay
//      [23:16] : RANK2 Read DQS Gating Delay
//      [15:08] : RANK1 Read DQS Gating Delay
//      [07:00] : RANK0 Read DQS Gating Delay
ddr_phy(0x7a) = b0_dqs_gd;
ddr_phy(0x8a) = b1_dqs_gd;
ddr_phy(0x9a) = b2_dqs_gd;
ddr_phy(0xAa) = b3_dqs_gd;


// DX?LCDLR1 : RD - WR
//      [31:24] : Reserved
//      [23:16] : RDQSND (Read DQSN Delay)
//      [15:08] : RDQSD (Read DQS Delay)
//      [07:00] : WDQD (Write Data Delay)
ddr_phy(0x79) = (b0_rdqsn_d << 16) | (b0_rdqs_d) << 8 | b0_wdq_d;
ddr_phy(0x89) = (b1_rdqsn_d << 16) | (b1_rdqs_d) << 8 | b1_wdq_d;
ddr_phy(0x99) = (b2_rdqsn_d << 16) | (b2_rdqs_d) << 8 | b2_wdq_d;
ddr_phy(0xA9) = (b3_rdqsn_d << 16) | (b3_rdqs_d) << 8 | b3_wdq_d;


// RMDL

ddr_phy(0x7B) = (b0_rmdl<<16);
ddr_phy(0x8B) = (b1_rmdl<<16);
ddr_phy(0x9B) = (b2_rmdl<<16);
ddr_phy(0xAB) = (b3_rmdl<<16);


// DX?BDLR3 : RD
//      [31:30] : Reserved
//      [29:24] : DQ4RBD
//      [23:18] : DQ3RBDk
//      [17:12] : DQ2RBD
//      [11:06] : DQ1RBD
//      [05:00] : DQ0RBD



ddr_phy(0x76) = (b0_dq4_rbd << 24) | (b0_dq3_rbd << 18) | (b0_dq2_rbd << 12) | (b0_dq1_rbd < 6) | b0_dq0_rbd;
ddr_phy(0x86) = (b1_dq4_rbd << 24) | (b1_dq3_rbd << 18) | (b1_dq2_rbd << 12) | (b1_dq1_rbd < 6) | b1_dq0_rbd;
ddr_phy(0x96) = (b2_dq4_rbd << 24) | (b2_dq3_rbd << 18) | (b2_dq2_rbd << 12) | (b2_dq1_rbd < 6) | b2_dq0_rbd;
ddr_phy(0xA6) = (b3_dq4_rbd << 24) | (b3_dq3_rbd << 18) | (b3_dq2_rbd << 12) | (b3_dq1_rbd < 6) | b3_dq0_rbd;
	        
// DX?BDLR4 : RD
//      [31:24] : Reserved
//      [23:18] : DMRBD
//      [17:12] : DQ7RBD
//      [11:06] : DQ6RBD
//      [05:00] : DQ5RBD
ddr_phy(0x77) = (b0_dm_rbd << 18) | (b0_dq7_rbd << 12) | (b0_dq6_rbd < 6) | b0_dq5_rbd;
ddr_phy(0x87) = (b1_dm_rbd << 18) | (b1_dq7_rbd << 12) | (b1_dq6_rbd < 6) | b1_dq5_rbd;
ddr_phy(0x97) = (b2_dm_rbd << 18) | (b2_dq7_rbd << 12) | (b2_dq6_rbd < 6) | b2_dq5_rbd;
ddr_phy(0xA7) = (b3_dm_rbd << 18) | (b3_dq7_rbd << 12) | (b3_dq6_rbd < 6) | b3_dq5_rbd;

// DX?BDLR0 : WR
//      [31:30] : Reserved
//      [29:24] : DQ4WBD
//      [23:18] : DQ3WBD
//      [17:12] : DQ2WBD
//      [11:06] : DQ1WBD
//      [05:00] : DQ0WBD



ddr_phy(0x73) = (b0_dq4_wbd << 24) | (b0_dq3_wbd << 18) | (b0_dq2_wbd << 12) | (b0_dq1_wbd < 6) | b0_dq0_wbd;
ddr_phy(0x83) = (b1_dq4_wbd << 24) | (b1_dq3_wbd << 18) | (b1_dq2_wbd << 12) | (b1_dq1_wbd < 6) | b1_dq0_wbd;
ddr_phy(0x93) = (b2_dq4_wbd << 24) | (b2_dq3_wbd << 18) | (b2_dq2_wbd << 12) | (b2_dq1_wbd < 6) | b2_dq0_wbd;
ddr_phy(0xA3) = (b3_dq4_wbd << 24) | (b3_dq3_wbd << 18) | (b3_dq2_wbd << 12) | (b3_dq1_wbd < 6) | b3_dq0_wbd;

// DX?BDLR1 : WR
//      [31:30] : Reserved
//      [29:24] : DQSWBD
//      [23:18] : DMWBD
//      [17:12] : DQ7WBD
//      [11:06] : DQ6WBD
//      [05:00] : DQ5WBD
ddr_phy(0x74) = (b0_dqs_wbd << 24) | (b0_dm_wbd << 18) | (b0_dq7_wbd << 12) | (b0_dq6_wbd < 6) | b0_dq5_wbd;
ddr_phy(0x84) = (b1_dqs_wbd << 24) | (b1_dm_wbd << 18) | (b1_dq7_wbd << 12) | (b1_dq6_wbd < 6) | b1_dq5_wbd;
ddr_phy(0x94) = (b2_dqs_wbd << 24) | (b2_dm_wbd << 18) | (b2_dq7_wbd << 12) | (b2_dq6_wbd < 6) | b2_dq5_wbd;
ddr_phy(0xA4) = (b3_dqs_wbd << 24) | (b3_dm_wbd << 18) | (b3_dq7_wbd << 12) | (b3_dq6_wbd < 6) | b3_dq5_wbd;

// DX?BDLR2 : 0
//      [31:24] : Reserved
//      [23:18] : DQSN RD BD
//      [17:12] : DQS  RD BD
//      [11:06] : DQ OE BD
//      [05:00] : DQS OE BD


// Not Used
ddr_phy(0x75) = (b0_dqsn_rbd << 18) | (b0_dqs_rbd << 12) | (b0_dq_oebd < 6) | b0_dqs_oebd;
ddr_phy(0x85) = (b1_dqsn_rbd << 18) | (b1_dqs_rbd << 12) | (b1_dq_oebd < 6) | b1_dqs_oebd;
ddr_phy(0x95) = (b2_dqsn_rbd << 18) | (b2_dqs_rbd << 12) | (b2_dq_oebd < 6) | b2_dqs_oebd;
ddr_phy(0xA5) = (b3_dqsn_rbd << 18) | (b3_dqs_rbd << 12) | (b3_dq_oebd < 6) | b3_dqs_oebd;


	i=3200; while(i--);

#endif	// ifndef AUTO_TRAIN


#else
	#error "not selected ddr type.."
#endif
}

/****************************************************************************************
* FUNCTION :void InitRoutine_End(void)
* DESCRIPTION :
* ***************************************************************************************/
volatile void InitRoutine_End(void)
{
}

/*===========================================================================
TYPE
===========================================================================*/
#if defined(DRAM_DDR3)
enum {
	PLL_VALUE = 0,
	CKC_CTRL_VALUE,
	CLK_RATE,
	DENALI_CTL_2,
	DENALI_CTL_3,
	DENALI_CTL_4,
	DENALI_CTL_5,
	DENALI_CTL_6,
	DENALI_CTL_7,
	DENALI_CTL_8,
	DENALI_CTL_9,
	DENALI_CTL_10,
	DENALI_CTL_11,
	DENALI_CTL_12,
	DENALI_CTL_13,
	tRFC,
	DENALI_CTL_15,
	DENALI_CTL_16,
	DENALI_CTL_18,
	tCKSRX,
	tCKSRE,
	DENALI_CTL_101,
	MR0,
	MR1,
	MR2,
	MR3
};
#else
	#error "not selected ddr type.."
#endif

/*===========================================================================
FUNCTION
===========================================================================*/
static unsigned int get_cycle(unsigned int tck, unsigned int ps, unsigned int ck)
{
	unsigned int ret;

	ret = time2cycle(ps, tck);

	if(ret > ck)
		return ret;
	else
		return ck;
}

/*===========================================================================
FUNCTION
===========================================================================*/
static unsigned int get_membus_ckc(unsigned int mem_freq)
{
	tPMS nPLL;
	unsigned int tmp_src, tmp_div;

	nPLL.fpll = mem_freq;
	if (tcc_find_pms(&nPLL, XIN_CLK_RATE))
		return 0;

	tmp_src = 4;	// XIN
	tmp_div = 1;	// 
	CKC_CHANGE_ARG(CKC_CTRL_VALUE) = 0x00200000|(tmp_src&0xF)|((tmp_div<<4)&0xF0);
	CKC_CHANGE_ARG(PLL_VALUE) = ((nPLL.vsel&0x1) << 30) | ((nPLL.s&0x7) << 24) | (1 << 20) | (0 << 18) | ((nPLL.m&0x3FF) << 8) | (nPLL.p&0x3F);
	CKC_CHANGE_ARG(CLK_RATE) = (nPLL.fpll/10000);

	return CKC_CHANGE_ARG(CLK_RATE);
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void get_ddr_param(unsigned int mem_freq)
{
#if defined(DRAM_DDR3)

	unsigned tck = 0;
	unsigned nCL = 0;
	unsigned nCWL = 0;
	unsigned tmp = 0;

	tck = (1000000/mem_freq);

	if(tck >= 2500){ /* 2.5ns, 400MHz */
		nCL = 6;
		nCWL = 5;
	}else if(tck >= 1875){ // 1.875ns, 533..MHz
		nCL = 8;
		nCWL = 6;
	}else if(tck >= 1500){ // 1.5 ns, 666..MHz
		if(DDR3_MAX_SPEED < DDR3_1600)
			nCL = 9;
		else
			nCL = 10;
#if defined(TARGET_BOARD_STB) && defined(CONFIG_CHIP_TCC8930)
		nCWL = 8;
#else
		nCWL = 7;
#endif
	}else if(tck >= 1250){ // 1.25 ns, 800MHz
		nCL = 11;
		nCWL = 8;
	}else if(tck >= 1070){ // 1.07 ns, 933..MHz
		nCL = 13;
		nCWL = 9;
	}else if(tck >= 935){ // 0.935 ns, 1066..MHz
		nCL = 14;
		nCWL = 10;
	}

#ifdef OLD_SETTING
	CKC_CHANGE_ARG(DENALI_CTL_2) = time2cycle(10000, tck); //TINIT[23:0]
	CKC_CHANGE_ARG(DENALI_CTL_3) = 0; //time2cycle(200000000, tck); //TRST_PWRON
	CKC_CHANGE_ARG(DENALI_CTL_4) = time2cycle(500000000, tck); //CKE_INACTIVE
	if(DDR3_AL == AL_DISABLED){ //nAL = 0;
		CKC_CHANGE_ARG(DENALI_CTL_5) = (0x1<<24 | (nCWL)<<16 | ((nCL<<1)|0)<<8 | 0x8);
	}
	else if(DDR3_AL == AL_CL_MINUS_ONE){ //nAL = nCL - 1;
		CKC_CHANGE_ARG(DENALI_CTL_5) = (0x1<<24 | (nCWL+nCL-1)<<16 | ((nCL<<1)|0)<<8 | 0x8);
	}	
	else if(DDR3_AL == AL_CL_MINUS_TWO){ //nAL = nCL - 2;
		CKC_CHANGE_ARG(DENALI_CTL_5) = (0x1<<24 | (nCWL+nCL-2)<<16 | ((nCL<<1)|0)<<8 | 0x8);
	}
#else
	CKC_CHANGE_ARG(DENALI_CTL_2) = 0x00000007; //TINIT[23:0]
	CKC_CHANGE_ARG(DENALI_CTL_3) = 0; //0x000000a0; //TRST_PWRON
	CKC_CHANGE_ARG(DENALI_CTL_4) = 0x00000090; //CKE_INACTIVE
	if(DDR3_AL == AL_DISABLED){ //nAL = 0;
		CKC_CHANGE_ARG(DENALI_CTL_5) = (0x4<<24 | (nCWL)<<16 | ((nCL<<1)|0)<<8 | 0x0);
	}
	else if(DDR3_AL == AL_CL_MINUS_ONE){ //nAL = nCL - 1;
		CKC_CHANGE_ARG(DENALI_CTL_5) = (0x4<<24 | (nCWL+nCL-1)<<16 | ((nCL<<1)|0)<<8 | 0x0);
	}	
	else if(DDR3_AL == AL_CL_MINUS_TWO){ //nAL = nCL - 2;
		CKC_CHANGE_ARG(DENALI_CTL_5) = (0x4<<24 | (nCWL+nCL-2)<<16 | ((nCL<<1)|0)<<8 | 0x0);
	}
#endif

	CKC_CHANGE_ARG(DENALI_CTL_6) = (get_cycle(tck, DDR3_tRAS_ps, DDR3_tRAS_ck)<<24 | get_cycle(tck, DDR3_tRC_ps, DDR3_tRC_ck)<<16 | get_cycle(tck, DDR3_tRRD_ps, DDR3_tRRD_ck)<<8 | DDR3_tCCD_ck);
	CKC_CHANGE_ARG(DENALI_CTL_7) = (DDR3_tMRD_ck<<24 | get_cycle(tck, DDR3_tRTP_ps, DDR3_tRTP_ck)<<16 | get_cycle(tck, DDR3_tRP_ps, DDR3_tRP_ck)<<8 | (get_cycle(tck, DDR3_tWTR_ps, DDR3_tWTR_ck)+1));
	CKC_CHANGE_ARG(DENALI_CTL_8) = (get_cycle(tck, DDR3_tRAS_MAX_ps, 1)<<8 | get_cycle(tck, DDR3_tMOD_ps, DDR3_tMOD_ck));
	CKC_CHANGE_ARG(DENALI_CTL_9) = ((get_cycle(tck, DDR3_tCKE_ps, DDR3_tCKE_ck)+1)<<8 | get_cycle(tck, DDR3_tCKE_ps, DDR3_tCKE_ck));
	CKC_CHANGE_ARG(DENALI_CTL_10) = (get_cycle(tck, DDR3_tWR_ps, DDR3_tWR_ck)<<24 | get_cycle(tck, DDR3_tRCD_ps, DDR3_tRCD_ck)<<16 | 1<<8 | 1);
	CKC_CHANGE_ARG(DENALI_CTL_11) = (1<<24 | DDR3_tDLLK_ck<<8 | (get_cycle(tck, DDR3_tWR_ps, DDR3_tWR_ck)+nCL));
	CKC_CHANGE_ARG(DENALI_CTL_12) = (1<<16 | get_cycle(tck, DDR3_tFAW_ps, DDR3_tFAW_ck)<<8 | 3);
	CKC_CHANGE_ARG(DENALI_CTL_13) = get_cycle(tck, DDR3_tRP_ps, DDR3_tRP_ck)+1;
	CKC_CHANGE_ARG(tRFC) = get_cycle(tck, DDR3_tRFC_ps, 1);
	CKC_CHANGE_ARG(DENALI_CTL_15) = get_cycle(tck, DDR3_tREFI_ps, 1);
	CKC_CHANGE_ARG(DENALI_CTL_16) = (get_cycle(tck, DDR3_tXPDLL_ps, DDR3_tXPDLL_ck)<<16 | get_cycle(tck, DDR3_tXP_ps, DDR3_tXP_ck)); // DDR3 Only
	CKC_CHANGE_ARG(DENALI_CTL_18) = (get_cycle(tck, DDR3_tXS_ps, DDR3_tXS_ck)<<16 | DDR3_tXSDLL_ck);
	CKC_CHANGE_ARG(tCKSRX) = get_cycle(tck, DDR3_tCKSRX_ps, DDR3_tCKSRX_ck);
	CKC_CHANGE_ARG(tCKSRE) = get_cycle(tck, DDR3_tCKSRE_ps, DDR3_tCKSRE_ck);

//--------------------------------------------------------------------------
// DFI Timing

	CKC_CHANGE_ARG(DENALI_CTL_101) = (0x00<<24|0x02<<16|(nCWL-3+(nCWL%2))<<8|(nCL-3+(nCL%2)));

//--------------------------------------------------------------------------
// MRS Setting

	// MRS0
	tmp = DDR3_BURST_LEN | (DDR3_READ_BURST_TYPE<<3);

	if(nCL < 5)
		tmp |= ((5-4)<<4);
	else if(nCL > 11)
		tmp |= ((11-4)<<4);
	else
		tmp |= ((nCL-4)<<4);

	if(get_cycle(tck, DDR3_tWR_ps, DDR3_tWR_ck) <= 5) // Write Recovery for autoprecharge
		tmp |= WR_5<<9;
	else if(get_cycle(tck, DDR3_tWR_ps, DDR3_tWR_ck) == 6)
		tmp |= WR_6<<9;
	else if(get_cycle(tck, DDR3_tWR_ps, DDR3_tWR_ck) == 7)
		tmp |= WR_7<<9;
	else if(get_cycle(tck, DDR3_tWR_ps, DDR3_tWR_ck) == 8)
		tmp |= WR_8<<9;
	else if((get_cycle(tck, DDR3_tWR_ps, DDR3_tWR_ck) == 9) || (get_cycle(tck, DDR3_tWR_ps, DDR3_tWR_ck) == 10))
		tmp |= WR_10<<9;
	else if(get_cycle(tck, DDR3_tWR_ps, DDR3_tWR_ck) >= 11)
		tmp |= WR_12<<9;

	tmp	|= (1<<8); // DLL Reset

	CKC_CHANGE_ARG(MR0) = tmp;

	// MRS1
	CKC_CHANGE_ARG(MR1) = ((DDR3_AL<<3) | (DDR3_ODT<<2) | (DDR3_DIC<<1));

	// MRS2
	tmp = 0;
	if(nCWL <= 5)
		tmp |= 0;
	else if(nCWL == 6)
		tmp |= 1<<3;
	else if(nCWL == 7)
		tmp |= 2<<3;
	else if(nCWL >= 8)
		tmp |= 3<<3;

	CKC_CHANGE_ARG(MR2) = tmp;

	//dprintf (INFO, "MEM timing : nCL=%d, nCWL=%d\n", nCL, nCWL);
#else
	#error "not selected ddr type.."
#endif
}

/*===========================================================================
FUNCTION
===========================================================================*/
void _change_mem_clock(void)
{
#if defined(DRAM_DDR3)
	volatile int i;

//--------------------------------------------------------------------------
// holding to access to dram

	denali_ctl(47) = 0xFFFFFFFF;
	BITSET(denali_ctl(44),0x1); //inhibit_dram_cmd=1
	while(!(denali_ctl(46)&(0x1<<15)));	// wait for inhibit_dram_cmd
	BITSET(denali_ctl(47), 0x1<<15);

//--------------------------------------------------------------------------
//enter self-refresh

	BITSET(denali_phy(13), 0x1<<10); //lp_ext_req=1
	while(!(denali_phy(13)&(0x1<<26))); //until lp_ext_ack==1
	BITCSET(denali_phy(13), 0x000000FF, (2<<2)|(1<<1)|(0));
	BITSET(denali_phy(13), 0x1<<8); //lp_ext_cmd_strb=1
	while((denali_phy(13)&(0x003F0000)) != (0x25<<16)); //until lp_ext_state==0x25 : check self-refresh state
	BITCLR(denali_phy(13), 0x1<<8); //lp_ext_cmd_strb=0
	BITCLR(denali_phy(13), 0x1<<10); //lp_ext_req=0
	while(denali_phy(13)&(0x1<<26)); //until lp_ext_ack==0
	BITSET(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x3
	BITCLR(denali_ctl(0),0x1); //START[0] = 0

	ddr_phy(0x0E) = 0xffffffff; // AC IO Config

//--------------------------------------------------------------------------
//Clock setting..
	*(volatile unsigned long *)addr_clk(0x000004) = CKC_CHANGE_ARG(CKC_CTRL_VALUE);  // mem bus
	while((*(volatile unsigned long *)addr_clk(0x000004))&(1<<29)); // CFGREQ
	i=100; while(i--);
	*(volatile unsigned long *)addr_clk(0x000034) = CKC_CHANGE_ARG(PLL_VALUE) | 0x00100000; //pll1 for mem , lock_en
	*(volatile unsigned long *)addr_clk(0x000034) |= 0x80000000; //pll1 for mem , lock_en
	i=20; while(i--)while((*(volatile unsigned long *)addr_clk(0x000034) & 0x00200000) == 0);
	i=100; while(i--);
	*(volatile unsigned long *)addr_clk(0x000004) = 0x00200011;  // mem bus
	while((*(volatile unsigned long *)addr_clk(0x000004))&(1<<29)); // CFGREQ
	i=10000; while(i--);

//--------------------------------------------------------------------------
// Timing Parameter

#if (1)
	denali_ctl(2) = CKC_CHANGE_ARG(DENALI_CTL_2);
	denali_ctl(3) = CKC_CHANGE_ARG(DENALI_CTL_3);
	denali_ctl(4) = CKC_CHANGE_ARG(DENALI_CTL_4);
	denali_ctl(5) = CKC_CHANGE_ARG(DENALI_CTL_5);
	denali_ctl(6) = CKC_CHANGE_ARG(DENALI_CTL_6);
	denali_ctl(7) = CKC_CHANGE_ARG(DENALI_CTL_7);
	denali_ctl(8) = CKC_CHANGE_ARG(DENALI_CTL_8);
	denali_ctl(9) = CKC_CHANGE_ARG(DENALI_CTL_9);
	denali_ctl(10) = CKC_CHANGE_ARG(DENALI_CTL_10);
	denali_ctl(11) = CKC_CHANGE_ARG(DENALI_CTL_11);
	denali_ctl(12) = CKC_CHANGE_ARG(DENALI_CTL_12);
	denali_ctl(13) = CKC_CHANGE_ARG(DENALI_CTL_13);
	denali_ctl(14) = (CKC_CHANGE_ARG(tRFC)<<16 | 1<<8 | 0);
	denali_ctl(15) = CKC_CHANGE_ARG(DENALI_CTL_15);
	denali_ctl(16) = CKC_CHANGE_ARG(DENALI_CTL_16);
	denali_ctl(18) = CKC_CHANGE_ARG(DENALI_CTL_18);
	denali_ctl(20) = (CKC_CHANGE_ARG(tCKSRX)<<16 | CKC_CHANGE_ARG(tCKSRE)<<8);

//--------------------------------------------------------------------------
// DFI Timing

	denali_ctl(101) = CKC_CHANGE_ARG(DENALI_CTL_101);

//--------------------------------------------------------------------------
// MRS Setting

	// MRS0
	denali_ctl(27) = CKC_CHANGE_ARG(MR0)<<8;
	BITCSET(denali_ctl(30), 0x0000FFFF, CKC_CHANGE_ARG(MR0));
#ifdef PHY_MRS_SETTING
	ddr_phy(0x15) = CKC_CHANGE_ARG(MR0);
#endif

	// MRS1
	BITCSET(denali_ctl(28), 0x0000FFFF, CKC_CHANGE_ARG(MR1));
	BITCSET(denali_ctl(30), 0xFFFF0000, CKC_CHANGE_ARG(MR1)<<16);
#ifdef PHY_MRS_SETTING
	ddr_phy(0x16) = CKC_CHANGE_ARG(MR1);
#endif

	// MRS2
	BITCSET(denali_ctl(28), 0xFFFF0000, CKC_CHANGE_ARG(MR2)<<16);
	BITCSET(denali_ctl(31), 0x0000FFFF, CKC_CHANGE_ARG(MR2));
#ifdef PHY_MRS_SETTING
	ddr_phy(0x17) = CKC_CHANGE_ARG(MR2);
#endif
#endif

//--------------------------------------------------------------------------
// Exit self-refresh

	ddr_phy(0x0E) = 0x30c01812; // AC IO Config

	BITCLR(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x0
	BITCLR(denali_ctl(39), 0x3<<24); //ZQ_ON_SREF_EXIT = 0
 	BITSET(denali_ctl(0),0x1); //START[0] = 1

	//for(i=0;i<11;i++){
	//	BITSET(denali_ctl(123), 0x1<<16); //CTRLUPD_REQ = 1
	//	while(!(denali_ctl(46)&(0x20000)));
	//	BITSET(denali_ctl(47), 0x20000);
	//}

	BITCSET(denali_ctl(20), 0xFF000000, ((2<<2)|(0<<1)|(1))<<24);
	while(!(denali_ctl(46)&(0x40)));
	BITSET(denali_ctl(47), 0x40);

//--------------------------------------------------------------------------
// MRS Write

	// MRS2
	BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(28)&0xFFFF0000)>>16);
	BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(31)&0x0000FFFF)<<16);
	denali_ctl(26) = (2<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
	while(!(denali_ctl(46)&(0x4000)));
	BITSET(denali_ctl(47), 0x4000);

	// MRS3
	BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(29)&0xFFFF0000)>>16);
	BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(32)&0x0000FFFF)<<16);
	denali_ctl(26) = (3<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
	while(!(denali_ctl(46)&(0x4000)));
	BITSET(denali_ctl(47), 0x4000);

	// MRS1
	BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(28)&0x0000FFFF)>>0);
	BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(30)&0xFFFF0000)<<0);
	denali_ctl(26) = (1<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
	while(!(denali_ctl(46)&(0x4000)));
	BITSET(denali_ctl(47), 0x4000);

	// MRS0
	BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(27)&0x00FFFF00)>>8);
	BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(30)&0x0000FFFF)<<16);
	denali_ctl(26) = (0<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
	while(!(denali_ctl(46)&(0x4000)));
	BITSET(denali_ctl(47), 0x4000);

//--------------------------------------------------------------------------

	BITCLR(denali_ctl(40) ,0x1<<16); //ZQCS_ROTATE=0x0
	BITCSET(denali_ctl(39) ,0x3<<16, 0x2<<16); //ZQ_REQ=2 : 0x1=short calibration, 0x2=long calibration

//--------------------------------------------------------------------------
// release holding to access to dram

	i = 10;	while(i--) BITSET(denali_ctl(13), 1<<24); // AREFRESH = 1
	BITCLR(denali_ctl(44),0x1); //inhibit_dram_cmd=0

//--------------------------------------------------------------------------
#endif	
}

#if defined(TSBM_ENABLE)||defined(OTP_UID_INCLUDE)
static unsigned int g_tcc_mem_freq = 0;
void set_tcc_mem_freq(unsigned int mem_freq)
{
    g_tcc_mem_freq = mem_freq;
}

unsigned int get_tcc_mem_freq(void)
{
    return g_tcc_mem_freq;
}
#endif

/*===========================================================================
FUNCTION
===========================================================================*/
int change_mem_clock(unsigned int freq)
{
	int i;
	typedef void (*FuncPtr)(void);
	unsigned flags, tmp, stk;
	unsigned int   mem_freq;
	FuncPtr pFunc = (FuncPtr)(CKC_CHANGE_FUNC_ADDR);

	//Bruce_temp_8920, memcpy가 오동작한다.
	//memcpy((void *)CKC_CHANGE_FUNC_ADDR, (void*)_change_mem_clock, CKC_CHANGE_FUNC_SIZE);
	{
		unsigned src = (unsigned)_change_mem_clock;
		unsigned dest = CKC_CHANGE_FUNC_ADDR;

		for(i=0;i<CKC_CHANGE_FUNC_SIZE;i+=4)
			*(volatile unsigned long *)(dest+i) = *(volatile unsigned long *)(src+i);
	}

	mem_freq = get_membus_ckc(freq);
	dprintf (INFO, "MEM FREQ : %dMHz\n", mem_freq);
	get_ddr_param(mem_freq);

#if defined(TSBM_ENABLE)||defined(OTP_UID_INCLUDE)
	set_tcc_mem_freq(mem_freq);
#endif

	__asm__ __volatile__( \
	"mrs	%0, cpsr\n" \
	"cpsid	i\n" \
	"mrs	%1, cpsr\n" \
	"orr	%1, %1, #128\n" \
	"msr	cpsr_c, %1" \
	: "=r" (flags), "=r" (tmp) \
	: \
	: "memory", "cc");

	//CKC_CHANGE_STACK_TOP
	__asm__ __volatile__( \
	"mov	%0, sp \n" \
	"ldr	sp, =0x10005000 \n" \
	: "=r" (stk) \
	: \
	: "memory", "cc");

	//arm_invalidate_tlb();

	{
		uint32_t val;
		__asm__ volatile("mrc	p15, 0, %0, c1, c0, 0" : "=r" (val));
		val &= ~0x1;
		__asm__ volatile("mcr	p15, 0, %0, c1, c0, 0" :: "r" (val));
	}

	pFunc();

	{
		uint32_t val;
		__asm__ volatile("mrc	p15, 0, %0, c1, c0, 0" : "=r" (val));
		val |= 0x1;
		__asm__ volatile("mcr	p15, 0, %0, c1, c0, 0" :: "r" (val));
	}

	__asm__ __volatile__( \
	"mov	sp, %0 \n" \
	: \
	: "r" (stk) \
	: "memory", "cc");

	__asm__ __volatile__( \
	"msr	cpsr_c, %0\n " \
	"cpsid	i" \
	: \
	: "r" (flags) \
	: "memory", "cc");

	return 0;
}

/*===========================================================================
FUNCTION
===========================================================================*/
int sdram_test(void)
{
	int i,temp,test_result;
#ifdef MEM_BIST

	ddr_phy(0x45) = 0;
	for(i=0; i<4; i++) {

		tca_ckc_setfbusctrl( FBUS_MEM,    ENABLE, 6000000);	/*FBUS_MEM      600 MHz */

		ddr_phy(0x45) = (i<<4);	

	}
#else
	dprintf(INFO, "\n");
        dprintf(INFO, "+++++ DRAM TEST Start +++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

#if DBG_RESULT_TRAINING
        dprintf(INFO, "0. ZQ Calibration Result\n");
	temp = ( ddr_phy(0x62) & 0x000f8000 ) >> 15;	// pull-up ODT

        	dprintf(INFO, " ODT = %x\n", temp);
	if     ( temp == 0x01 )
        	dprintf(INFO, " ODT = 512 ohm\n");
	else if( temp == 0x02 )
        	dprintf(INFO, " ODT = 256 ohm\n");
	else if( temp == 0x03 )
        	dprintf(INFO, " ODT = 171 ohm\n");
	else if( temp == 0x06 )
        	dprintf(INFO, " ODT = 138 ohm\n");
	else if( temp == 0x07 )
        	dprintf(INFO, " ODT = 103 ohm\n");
	else if( temp == 0x04 )
        	dprintf(INFO, " ODT = 86.4 ohm\n");
	else if( temp == 0x05 )
        	dprintf(INFO, " ODT = 74.2 ohm\n");
	else if( temp == 0x0c )
        	dprintf(INFO, " ODT = 65.1 ohm\n");
	else if( temp == 0x0d )
        	dprintf(INFO, " ODT = 58.0 ohm\n");
	else if( temp == 0x0e )
        	dprintf(INFO, " ODT = 52.4 ohm\n");
	else if( temp == 0x0f )
        	dprintf(INFO, " ODT = 47.8 ohm\n");
	else if( temp == 0x0a )
        	dprintf(INFO, " ODT = 43.9 ohm\n");
	else if( temp == 0x0b )
        	dprintf(INFO, " ODT = 40.6 ohm\n");
	else if( temp == 0x08 )
        	dprintf(INFO, " ODT = 37.9 ohm\n");
	else if( temp == 0x09 )
        	dprintf(INFO, " ODT = 35.4 ohm\n");
	else if( temp == 0x18 )
        	dprintf(INFO, " ODT = 33.3 ohm\n");
	else if( temp == 0x19 )
        	dprintf(INFO, " ODT = 31.4 ohm\n");
	else if( temp == 0x1a )
        	dprintf(INFO, " ODT = 29.8 ohm\n");
	else if( temp == 0x1b )
        	dprintf(INFO, " ODT = 28.3 ohm\n");
	else if( temp == 0x1e )
        	dprintf(INFO, " ODT = 27.0 ohm\n");
	else if( temp == 0x1c )
        	dprintf(INFO, " ODT = 25.8 ohm\n");
	else if( temp == 0x1d )
        	dprintf(INFO, " ODT = 24.6 ohm\n");
	else if( temp == 0x14 )
        	dprintf(INFO, " ODT = 23.6 ohm\n");
	else if( temp == 0x15 )
        	dprintf(INFO, " ODT = 22.7 ohm\n");
	else if( temp == 0x16 )
        	dprintf(INFO, " ODT = 21.9 ohm\n");
	else if( temp == 0x17 )
        	dprintf(INFO, " ODT = 20.4 ohm\n");
	else if( temp == 0x12 )
        	dprintf(INFO, " ODT = 19.7 ohm\n");
	else if( temp == 0x13 )
        	dprintf(INFO, " ODT = 19.1 ohm\n");
	else if( temp == 0x10 )
        	dprintf(INFO, " ODT = 18.5 ohm\n");
	else if( temp == 0x11 )
        	dprintf(INFO, " ODT = 18.0 ohm\n");
	else 
        	dprintf(INFO, " ODT = Error !!!\n");

	temp = ( ddr_phy(0x62) & 0x000003e0 ) >> 5;	// pull-up Output-Z

        	dprintf(INFO, " DIC = %x\n", temp);
	if     ( temp == 0x01 )
        	dprintf(INFO, " DIC = 512 ohm\n");
	else if( temp == 0x02 )
        	dprintf(INFO, " DIC = 256 ohm\n");
	else if( temp == 0x03 )
        	dprintf(INFO, " DIC = 171 ohm\n");
	else if( temp == 0x06 )
        	dprintf(INFO, " DIC = 138 ohm\n");
	else if( temp == 0x07 )
        	dprintf(INFO, " DIC = 103 ohm\n");
	else if( temp == 0x04 )
        	dprintf(INFO, " DIC = 86.4 ohm\n");
	else if( temp == 0x05 )
        	dprintf(INFO, " DIC = 74.2 ohm\n");
	else if( temp == 0x0c )
        	dprintf(INFO, " DIC = 65.1 ohm\n");
	else if( temp == 0x0d )
        	dprintf(INFO, " DIC = 58.0 ohm\n");
	else if( temp == 0x0e )
        	dprintf(INFO, " DIC = 52.4 ohm\n");
	else if( temp == 0x0f )
        	dprintf(INFO, " DIC = 47.8 ohm\n");
	else if( temp == 0x0a )
        	dprintf(INFO, " DIC = 43.9 ohm\n");
	else if( temp == 0x0b )
        	dprintf(INFO, " DIC = 40.6 ohm\n");
	else if( temp == 0x08 )
        	dprintf(INFO, " DIC = 37.9 ohm\n");
	else if( temp == 0x09 )
        	dprintf(INFO, " DIC = 35.4 ohm\n");
	else if( temp == 0x18 )
        	dprintf(INFO, " DIC = 33.3 ohm\n");
	else if( temp == 0x19 )
        	dprintf(INFO, " DIC = 31.4 ohm\n");
	else if( temp == 0x1a )
        	dprintf(INFO, " DIC = 29.8 ohm\n");
	else if( temp == 0x1b )
        	dprintf(INFO, " DIC = 28.3 ohm\n");
	else if( temp == 0x1e )
        	dprintf(INFO, " DIC = 27.0 ohm\n");
	else if( temp == 0x1c )
        	dprintf(INFO, " DIC = 25.8 ohm\n");
	else if( temp == 0x1d )
        	dprintf(INFO, " DIC = 24.6 ohm\n");
	else if( temp == 0x14 )
        	dprintf(INFO, " DIC = 23.6 ohm\n");
	else if( temp == 0x15 )
        	dprintf(INFO, " DIC = 22.7 ohm\n");
	else if( temp == 0x16 )
        	dprintf(INFO, " DIC = 21.9 ohm\n");
	else if( temp == 0x17 )
        	dprintf(INFO, " DIC = 20.4 ohm\n");
	else if( temp == 0x12 )
        	dprintf(INFO, " DIC = 19.7 ohm\n");
	else if( temp == 0x13 )
        	dprintf(INFO, " DIC = 19.1 ohm\n");
	else if( temp == 0x10 )
        	dprintf(INFO, " DIC = 18.5 ohm\n");
	else if( temp == 0x11 )
        	dprintf(INFO, " DIC = 18.0 ohm\n");
	else 
        	dprintf(INFO, " DIC = Error !!!\n");

	dprintf(INFO, "\n");
        dprintf(INFO, "1. Write Leveling Delay\n");
        dprintf(INFO, "WLD      B0:0x%8x B1:0x%8x B2:0x%8x B3:0x%8x\n", ddr_phy(0x78), ddr_phy(0x88), ddr_phy(0x98), ddr_phy(0xA8) );
	dprintf(INFO, "\n");
        dprintf(INFO, "2. DQS Gate Delay\n");
        dprintf(INFO, "DGD      B0:0x%8x B1:0x%8x B2:0x%8x B3:0x%8x\n", ddr_phy(0x7a), ddr_phy(0x8a), ddr_phy(0x9a), ddr_phy(0xAa) );
        dprintf(INFO, "DGSD     B0:0x%8x B1:0x%8x B2:0x%8x B3:0x%8x\n", ddr_phy(0x7c), ddr_phy(0x8c), ddr_phy(0x9c), ddr_phy(0xAc) );
	dprintf(INFO, "\n");
        dprintf(INFO, "3. Read Delay\n");
        //dprintf(INFO, "RDQSND   B0:0x%8x B1:0x%8x B2:0x%8x B3:0x%8x\n"  , ( ddr_phy(0x79) & 0x00ff0000 ) >> 16
        //                                                                , ( ddr_phy(0x89) & 0x00ff0000 ) >> 16
        //                                                                , ( ddr_phy(0x99) & 0x00ff0000 ) >> 16
        //                                                                , ( ddr_phy(0xA9) & 0x00ff0000 ) >> 16
        //                                                                );
        dprintf(INFO, "RDQSD    B0:0x%8x B1:0x%8x B2:0x%8x B3:0x%8x\n"  , ( ddr_phy(0x79) & 0x0000ff00 ) >> 8
                                                                        , ( ddr_phy(0x89) & 0x0000ff00 ) >> 8
                                                                        , ( ddr_phy(0x99) & 0x0000ff00 ) >> 8
                                                                        , ( ddr_phy(0xA9) & 0x0000ff00 ) >> 8
                                                                        );
        //dprintf(INFO, "RDMDL    B0:0x%8x B1:0x%8x B2:0x%8x B3:0x%8x\n"  , ( ddr_phy(0x7B) & 0x00ff0000 ) >> 16
        //                                                                , ( ddr_phy(0x8B) & 0x00ff0000 ) >> 16
        //                                                                , ( ddr_phy(0x9B) & 0x00ff0000 ) >> 16
        //                                                                , ( ddr_phy(0xAB) & 0x00ff0000 ) >> 16
        //                                                                );

        dprintf(INFO, "B0RBDL D0:0x%2x D1:0x%2x D2:0x%2x D3:0x%2x D4:0x%2x D5:0x%2x D6:0x%2x D7:0x%2x DM:0x%2x\n"
                                                                        , ( ddr_phy(0x76) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x76) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x76) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x76) & 0x00fc0000 ) >> 18
                                                                        , ( ddr_phy(0x76) & 0x3f000000 ) >> 24

                                                                        , ( ddr_phy(0x77) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x77) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x77) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x77) & 0x00fc0000 ) >> 18
                                                                        );
        dprintf(INFO, "B1RBDL D0:0x%2x D1:0x%2x D2:0x%2x D3:0x%2x D4:0x%2x D5:0x%2x D6:0x%2x D7:0x%2x DM:0x%2x\n"
                                                                        , ( ddr_phy(0x86) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x86) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x86) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x86) & 0x00fc0000 ) >> 18
                                                                        , ( ddr_phy(0x86) & 0x3f000000 ) >> 24

                                                                        , ( ddr_phy(0x87) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x87) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x87) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x87) & 0x00fc0000 ) >> 18
                                                                        );

        dprintf(INFO, "B2RBDL D0:0x%2x D1:0x%2x D2:0x%2x D3:0x%2x D4:0x%2x D5:0x%2x D6:0x%2x D7:0x%2x DM:0x%2x\n"
                                                                        , ( ddr_phy(0x96) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x96) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x96) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x96) & 0x00fc0000 ) >> 18
                                                                        , ( ddr_phy(0x96) & 0x3f000000 ) >> 24

                                                                        , ( ddr_phy(0x97) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x97) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x97) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x97) & 0x00fc0000 ) >> 18
                                                                        );
        dprintf(INFO, "B3RBDL D0:0x%2x D1:0x%2x D2:0x%2x D3:0x%2x D4:0x%2x D5:0x%2x D6:0x%2x D7:0x%2x DM:0x%2x\n"
                                                                        , ( ddr_phy(0xA6) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0xA6) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0xA6) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0xA6) & 0x00fc0000 ) >> 18
                                                                        , ( ddr_phy(0xA6) & 0x3f000000 ) >> 24

                                                                        , ( ddr_phy(0xA7) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0xA7) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0xA7) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0xA7) & 0x00fc0000 ) >> 18
                                                                        );

	dprintf(INFO, "\n");
        dprintf(INFO, "4. Write Delay\n");


        dprintf(INFO, "WDD      B0:0x%8x B1:0x%8x B2:0x%8x B3:0x%8x\n"  , ( ddr_phy(0x79) & 0x000000ff ) >> 0
                                                                        , ( ddr_phy(0x89) & 0x000000ff ) >> 0
                                                                        , ( ddr_phy(0x99) & 0x000000ff ) >> 0
                                                                        , ( ddr_phy(0xA9) & 0x000000ff ) >> 0
                                                                        );


        dprintf(INFO, "B0WBDL D0:0x%2x D1:0x%2x D2:0x%2x D3:0x%2x D4:0x%2x D5:0x%2x D6:0x%2x D7:0x%2x DM:0x%2x\n" // DS:0x%2x\n"
                                                                        , ( ddr_phy(0x73) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x73) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x73) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x73) & 0x00fc0000 ) >> 18
                                                                        , ( ddr_phy(0x73) & 0x3f000000 ) >> 24

                                                                        , ( ddr_phy(0x74) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x74) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x74) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x74) & 0x00fc0000 ) >> 18
                                //                                        , ( ddr_phy(0x74) & 0x3f000000 ) >> 24
                                                                        );

        dprintf(INFO, "B1WBDL D0:0x%2x D1:0x%2x D2:0x%2x D3:0x%2x D4:0x%2x D5:0x%2x D6:0x%2x D7:0x%2x DM:0x%2x\n" // DS:0x%2x\n"
                                                                        , ( ddr_phy(0x83) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x83) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x83) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x83) & 0x00fc0000 ) >> 18
                                                                        , ( ddr_phy(0x83) & 0x3f000000 ) >> 24

                                                                        , ( ddr_phy(0x84) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x84) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x84) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x84) & 0x00fc0000 ) >> 18
                                //                                        , ( ddr_phy(0x84) & 0x3f000000 ) >> 24
                                                                        );

        dprintf(INFO, "B2WBDL D0:0x%2x D1:0x%2x D2:0x%2x D3:0x%2x D4:0x%2x D5:0x%2x D6:0x%2x D7:0x%2x DM:0x%2x\n" // DS:0x%2x\n"
                                                                        , ( ddr_phy(0x93) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x93) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x93) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x93) & 0x00fc0000 ) >> 18
                                                                        , ( ddr_phy(0x93) & 0x3f000000 ) >> 24

                                                                        , ( ddr_phy(0x94) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0x94) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0x94) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0x94) & 0x00fc0000 ) >> 18
                                //                                        , ( ddr_phy(0x94) & 0x3f000000 ) >> 24
                                                                        );
        dprintf(INFO, "B3WBDL D0:0x%2x D1:0x%2x D2:0x%2x D3:0x%2x D4:0x%2x D5:0x%2x D6:0x%2x D7:0x%2x DM:0x%2x\n" // DS:0x%2x\n"
                                                                        , ( ddr_phy(0xA3) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0xA3) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0xA3) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0xA3) & 0x00fc0000 ) >> 18
                                                                        , ( ddr_phy(0xA3) & 0x3f000000 ) >> 24

                                                                        , ( ddr_phy(0xA4) & 0x0000003f ) >> 0
                                                                        , ( ddr_phy(0xA4) & 0x00000fc0 ) >> 6
                                                                        , ( ddr_phy(0xA4) & 0x0003f000 ) >> 12
                                                                        , ( ddr_phy(0xA4) & 0x00fc0000 ) >> 18
                                //                                        , ( ddr_phy(0xA4) & 0x3f000000 ) >> 24
                                                                        );

#endif

	dprintf(INFO, "\n");
	temp = ddr_phy(0x04);

#if 0	//in V3.01 removed
	if( temp & 0x00010000 )
		dprintf(INFO, "+ DDR system Init \t\t[Fail]\n");
	else
		dprintf(INFO, "+ DDR system Init \t\t[Pass]\n");
		
	if( temp & 0x00020000 )
		dprintf(INFO, "+ PLL Lock \t\t\t[Fail]\n");
	else
		dprintf(INFO, "+ PLL Lock \t\t\t[Pass]\n");
		
	if( temp & 0x00040000 )
		dprintf(INFO, "+ DDL Calibration \t\t[Fail]\n");
	else
		dprintf(INFO, "+ DDL Calibration \t\t[Pass]\n");
#endif

	if( temp & 0x00100000 )
		dprintf(INFO, "+ Z Calibration \t\t[Fail]\n");
	else
		dprintf(INFO, "+ Z Calibration \t\t[Pass]\n");

#if 0	//in v3.01 removed
	if( temp & 0x00100000 )
		dprintf(INFO, "+ DRAM Init \t\t\t[Fail]\n");
	else
		dprintf(INFO, "+ DRAM Init \t\t\t[Pass]\n");
#endif

	if( temp & 0x00200000 )
		dprintf(INFO, "+ Write Leveling \t\t[Fail]\n");
	else
		dprintf(INFO, "+ Write Leveling \t\t[Pass]\n");

	if( temp & 0x00400000 )
		dprintf(INFO, "+ DQS Gate Training \t\t[Fail]\n");
	else
		dprintf(INFO, "+ DQS Gate Training \t\t[Pass]\n");

	if( temp & 0x00800000 )
		dprintf(INFO, "+ Write Leveling Adjust \t[Fail]\n");
	else
		dprintf(INFO, "+ Write Leveling Adjust \t[Pass]\n");

	if( temp & 0x01000000 )
		dprintf(INFO, "+ Read Bit Deskew \t\t[Fail]\n");
	else
		dprintf(INFO, "+ Read Bit Deskew \t\t[Pass]\n");

	if( temp & 0x02000000 )
		dprintf(INFO, "+ Write Bit Deskew \t\t[Fail]\n");
	else
		dprintf(INFO, "+ Write Bit Deskew \t\t[Pass]\n");

	if( temp & 0x04000000 )
		dprintf(INFO, "+ Read Eye Training \t\t[Fail]\n");
	else
		dprintf(INFO, "+ Read Eye Training \t\t[Pass]\n");

	if( temp & 0x08000000 )
		dprintf(INFO, "+ Write Eye Training \t\t[Fail]\n");
	else
		dprintf(INFO, "+ Write Eye Training \t\t[Pass]\n");


// Address Line Check (1G)
//
//	Row Check
//	Address 0x00000000 ~ 0x3fffffff
	dprintf(INFO, "\n");
        dprintf(INFO, "5. DDR Address/Data Line Check 1Gbyte Space\n");

	test_result = 0;

	//for( i=0; i<0x3fffffff; i=i+4096) {
	for( i=0x80000000; i<0xBfffffff; i=i+4096) {
		temp = *(unsigned int *)(i);

		*(unsigned int *)(i) = 0xaaaaaaaa;

		if( *(unsigned int *)(i) == 0xaaaaaaaa )	
			test_result = 0;		// pass
		else
			test_result += 1;		// fail

		*(unsigned int *)(i) = temp;
	}

        if( test_result != 0 ) {
        	dprintf(INFO, "+ Row Address Check \t\t[Fail]\n");
	}
	else
        	dprintf(INFO, "+ Row Address Check \t\t[Pass]\n");

	test_result = 0;

	for( i=0x80000000; i<0x80001000; i=i+4) {
		temp = *(unsigned int *)(i);

		*(unsigned int *)(i) = 0xaaaaaaaa;

		if( *(unsigned int *)(i) == 0xaaaaaaaa )	
			test_result = 0;		// pass
		else
			test_result += 1;		// fail

		*(unsigned int *)(i) = temp;
	}

        if( test_result != 0 ) {
        	dprintf(INFO, "+ Column Address Check \t\t[Fail]\n");
	}
	else
        	dprintf(INFO, "+ Column Address Check \t\t[Pass]\n");

	test_result = 0;

	for( i=0x80000000; i<0x80000004; i=i+4) {
		temp = *(unsigned int *)(i);

		*(unsigned int *)(i) = 0xaaaaaaaa;
		if( *(unsigned int *)(i) == 0xaaaaaaaa )	test_result = 0;		// pass
		else 						test_result += 1;		// fail

		*(unsigned int *)(i) = 0x55555555;
		if( *(unsigned int *)(i) == 0x55555555 )	test_result = 0;		// pass
		else 						test_result += 1;		// fail

		*(unsigned int *)(i) = 0x00000000;
		if( *(unsigned int *)(i) == 0x00000000 )	test_result = 0;		// pass
		else 						test_result += 1;		// fail

		*(unsigned int *)(i) = 0x00000000;
		if( *(unsigned int *)(i) == 0x00000000 )	test_result = 0;		// pass
		else 						test_result += 1;		// fail

		*(unsigned int *)(i) = temp;
	}

        if( test_result != 0 ) {
        	dprintf(INFO, "+ Data Line Check \t\t[Fail]\n");
	}
	else
        	dprintf(INFO, "+ Data Line Check \t\t[Pass]\n");

	dprintf(INFO, "\n");
        dprintf(INFO, "+++++ DRAM TEST End +++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	dprintf(INFO, "\n");

// MEM BIST

/*
        test_result = 0;
        dprintf(INFO, "%%%%%%%%%% 1. DDR Bist Start %%%%%%%%%%\n");
      	// [16] : address checking
      	// [08] : data checking 
      	// [5:0] : space
        denali_ctl(33) = 0x00010010;            // addr, size
        denali_ctl(34) = 0x01000000;            // start address
        denali_ctl(32) |= (0x1 << 16);          // go

        i=3200; while(i--);

        while(!( denali_ctl(46) & 0x80 ) );             // wait done
        denali_ctl(47) |= (0x1 << 7);               // flag clear

        ddr_phy(0x45) = ( denali_ctl(32) & 0x03000000 );            // flag
        denali_ctl(32) &= 0xfffeffff;           // bist stop


        if( test_result == 0 ) {
        	dprintf(INFO, "BIST Fail %x\n", denali_ctl(32));
        	ddr_phy(0x45) = 0;
        }
        else {
        dprintf(INFO, "BIST Pass %x\n", denali_ctl(32));
        ddr_phy(0x45) = 1;
        }
*/
        

/*
	for( temp = 0; temp < 125; temp++ ) {
		dprintf(INFO, "DENALI_CTL_%3d : %8x\n", temp, denali_ctl(temp));	
	}
	for( temp = 0; temp < 184; temp++ ) {
		dprintf(INFO, "PHY_%3d : %8x\n", temp, ddr_phy(temp));	
	}
*/
#endif	
}

/************* end of file *************************************************************/

