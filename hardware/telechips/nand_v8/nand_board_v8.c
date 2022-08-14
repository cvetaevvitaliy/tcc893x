/****************************************************************************
 *   FileName    : nand_board_v8.c
 *   Description : board specific functions for NAND Driver
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#include "nand_def.h"

#if defined(NAND_FOR_KERNEL)
#include <mach/reg_physical.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#elif defined(ANDROID_BOOT)
#include "config.h"
#include "debug.h"
#include <platform/globals.h>

#include <platform/reg_physical.h>
#endif

#include "nand_board_v8.h"

//#define NAND_BD_ASSERT(a)			if(!(a)) while(1);
#define NAND_BD_ASSERT(a)			if(!(a))\
									{\
										ND_TRACE("[ASSERT] %s:%d: %s\n",__FUNCTION__,__LINE__,#a);\
									}

//**********************************************************************
//*		Define Dev Board
//**********************************************************************
#if defined(TCC88XX)

	#define TCC88XX_88M_BOARD

#elif defined(TCC892X) || defined(TCC8925S)

	#if defined(TARGET_M805_892X_EVM) || defined(CONFIG_MACH_M805_892X)
	#define TCC892X_M805_BOARD
	#elif defined(CONFIG_MACH_TCC8920ST) || defined(TARGET_BOARD_STB)
	#define TCC892X_STB
	#else
	#define TCC892X_88M_BOARD
	#endif

#elif defined(TCC893X)
	#if defined(CONFIG_MACH_TCC8930ST) || defined(TARGET_BOARD_STB)
		#define TCC893X_STB
	#else
		#if defined(TCC8935)
		#define TCC8935_88M_BOARD
		#elif defined(TCC8933)
		#define TCC8935_88M_BOARD
		#elif defined(TCC8930)
		#define TCC8930_88M_BOARD
		#endif
	#endif
#else

	#error unknown board configuration

#endif

extern unsigned int system_rev;
#if defined(NAND_FOR_KERNEL) && defined(TCC893X)
extern spinlock_t gpio_lock;
#endif

#ifndef BITSET
#define BITSET(X, MASK)				(X) |= (MASK)
#define BITCLR(X, MASK)				(X) &= (~(MASK))
#define BITCSET(X, CMASK, SMASK)	(X) = ((X) & (~(CMASK))) | (SMASK)
#endif

#if defined(NAND_FOR_KERNEL) && defined(_LINUX_)
#define GPIO_BASE_ADDRESS		(HwGPIO_BASE)
#else
#if defined(TCC89XX) || defined(TCC92XX) || defined(TCC93XX)
#define GPIO_BASE_ADDRESS		(&HwGPIO_BASE)
#elif defined(TCC88XX) || defined(TCC892X)  || defined(TCC8925S) || defined(TCC893X)
#define GPIO_BASE_ADDRESS		(HwGPIO_BASE)
#else
#error
#endif
#endif

static PGPIO s_pNAND_BD_GPIO;

#if defined(TCC_NAND_WP_A19)
#define	NFC_nWP_GPnEN							(s_pNAND_BD_GPIO->GPAEN.nREG)
#define	NFC_nWP_GPnDAT							(s_pNAND_BD_GPIO->GPADAT.nREG)
#define	NFC_nWP_GPn_Bit							Hw19
#elif defined(TCC_NAND_WP_B8)
#define	NFC_nWP_GPnEN							(s_pNAND_BD_GPIO->GPBEN.nREG)
#define	NFC_nWP_GPnDAT							(s_pNAND_BD_GPIO->GPBDAT.nREG)
#define	NFC_nWP_GPn_Bit							Hw8
#elif defined(TCC_NAND_WP_B15)
#define	NFC_nWP_GPnEN							(s_pNAND_BD_GPIO->GPBEN.nREG)
#define	NFC_nWP_GPnDAT							(s_pNAND_BD_GPIO->GPBDAT.nREG)
#define	NFC_nWP_GPn_Bit							Hw15
#elif defined(TCC_NAND_WP_B22)
#define	NFC_nWP_GPnEN							(s_pNAND_BD_GPIO->GPBEN.nREG)
#define	NFC_nWP_GPnDAT							(s_pNAND_BD_GPIO->GPBDAT.nREG)
#define	NFC_nWP_GPn_Bit							Hw22
#elif defined(TCC_NAND_WP_B27)
#define	NFC_nWP_GPnEN							(s_pNAND_BD_GPIO->GPBEN.nREG)
#define	NFC_nWP_GPnDAT							(s_pNAND_BD_GPIO->GPBDAT.nREG)
#define	NFC_nWP_GPn_Bit							Hw27
#elif defined(TCC_NAND_WP_B28)
#define	NFC_nWP_GPnEN							(s_pNAND_BD_GPIO->GPBEN.nREG)
#define	NFC_nWP_GPnDAT							(s_pNAND_BD_GPIO->GPBDAT.nREG)
#define	NFC_nWP_GPn_Bit							Hw28
#elif defined(TCC_NAND_WP_B31)
#define	NFC_nWP_GPnEN							(s_pNAND_BD_GPIO->GPBEN.nREG)
#define	NFC_nWP_GPnDAT							(s_pNAND_BD_GPIO->GPBDAT.nREG)
#define	NFC_nWP_GPn_Bit							Hw31
#elif defined(TCC_NAND_WP_D0)
#define	NFC_nWP_GPnEN							(s_pNAND_BD_GPIO->GPDEN.nREG)
#define	NFC_nWP_GPnDAT							(s_pNAND_BD_GPIO->GPDDAT.nREG)
#define	NFC_nWP_GPn_Bit							Hw0
#elif defined(TCC_NAND_WP_D8)
#define	NFC_nWP_GPnEN							(s_pNAND_BD_GPIO->GPDEN.nREG)
#define	NFC_nWP_GPnDAT							(s_pNAND_BD_GPIO->GPDDAT.nREG)
#define	NFC_nWP_GPn_Bit							Hw8
#elif defined(TCC_NAND_WP_F27)
#define	NFC_nWP_GPnEN							(s_pNAND_BD_GPIO->GPFEN.nREG)
#define	NFC_nWP_GPnDAT							(s_pNAND_BD_GPIO->GPFDAT.nREG)
#define	NFC_nWP_GPn_Bit							Hw27
#endif

#if 0
static unsigned int s_NAND_BD_TestPortEnableBitmap;
void NAND_BD_TestPort_Enable(int iIndex, unsigned int fEnable)
{
	if(fEnable)
		BITSET(s_NAND_BD_TestPortEnableBitmap, 1<<iIndex);
	else
		BITCLR(s_NAND_BD_TestPortEnableBitmap, 1<<iIndex);
}

void NAND_BD_TestPort_Set(int iIndex, unsigned int fBit)
{
	if( iIndex == 0
		&& s_NAND_BD_TestPortEnableBitmap & ( 1 << iIndex ) )
	{
		s_pNAND_BD_GPIO->GPFDAT.bREG.GP03 = (fBit)? 1 : 0;
	}
}

void NAND_BD_TestPort_Init(unsigned int uiEnableBitmap)
{
	s_pNAND_BD_GPIO->GPFEN.bREG.GP03 = 1;	// Output Mode
	s_pNAND_BD_GPIO->GPFDAT.bREG.GP03 = 0;

	s_NAND_BD_TestPortEnableBitmap = uiEnableBitmap;
}
#endif

unsigned int NAND_BD_Get_CSCountMax( void )
{
	return 2/*NAND_2CS_ONLY*/;
}

static unsigned int NAND_BD_Get_BusWidthMax( void )
{
	return 8/*NAND_8BIT_ONLY*/;
}

static void NAND_BD_Init_PowerCheckPin(void)
{
#if defined (CONFIG_MACH_TCC8920)
	#if defined(TCC892X)
	BITSET( s_pNAND_BD_GPIO->GPFPE.nREG, Hw29);
	BITSET( s_pNAND_BD_GPIO->GPFPS.nREG, Hw29);
	BITCLR( s_pNAND_BD_GPIO->GPFEN.nREG, Hw29);    // Enable Register Setting.
	#endif
#elif defined(TCC8930)
	BITSET( s_pNAND_BD_GPIO->GPAPE.nREG, Hw22);
	BITSET( s_pNAND_BD_GPIO->GPAPS.nREG, Hw22);
	BITCLR( s_pNAND_BD_GPIO->GPAEN.nREG, Hw22);    // Enable Register Setting.
#endif
}

static unsigned int NAND_BD_PowerCheck(void)
{
#if defined (CONFIG_MACH_TCC8920)
	#if defined(TCC892X)
	if(!(s_pNAND_BD_GPIO->GPFDAT.nREG & Hw29))
	{

		NAND_BD_Enable_WP_Port();
		ND_TRACE("\n [NAND	] Power Failure Detected !!!!!\n");

		do{
			if(!(s_pNAND_BD_GPIO->GPFDAT.nREG & Hw29)){
				ND_TRACE("\n [NAND	] Go to Wile Loop.......\n");
			}else{
				ND_TRACE("\n [NAND	] Reset Key was Released. \n Go to write Normal Routin\n\n");
				break;
			}
		}while(1);
				
	}
	#endif
#elif defined(TCC8930)
	if(!(s_pNAND_BD_GPIO->GPADAT.nREG & Hw22))
	{
		NAND_BD_Enable_WP_Port();
		ND_TRACE("\n [NAND	] Power Failure Detected !!!!!\n");

		do{
			if(!(s_pNAND_BD_GPIO->GPADAT.nREG & Hw22)){
				ND_TRACE("\n [NAND	] Go to Wile Loop.......\n");
			}else{
				ND_TRACE("\n [NAND	] Reset Key was Released. \n Go to write Normal Routin\n\n");
				break;
			}
		}while(1);
				
	}	
#endif
	return TRUE;
}

static void NAND_BD_Init_DATA_Port(unsigned int uiMask)
{
	if(uiMask&Hw0)
	{
		//
		// NAND_D[0:7]
		//
		#if defined(TCC88XX)
		BITCSET(s_pNAND_BD_GPIO->GPBFN0, 0xFFFFFFFF, 0x11111111);
		#elif defined(TCC892X)  || defined(TCC8925S) || defined(TCC893X)
		BITCSET(s_pNAND_BD_GPIO->GPAFN0.nREG, 0xFFFFFFFF, 0x11111111);
		BITCSET(s_pNAND_BD_GPIO->GPAPE.nREG, 0x000000FF, 0x000000FF);	
		BITCSET(s_pNAND_BD_GPIO->GPAPS.nREG, 0x000000FF, 0x000000FF);
		#else
		#err NAND Port Error
		#endif
	}
	if(uiMask&Hw1)
	{
		//
		// NAND_D[8:15]
		//
		#if defined(TCC88XX)
		BITCSET(s_pNAND_BD_GPIO->GPBFN1, 0xFFFFFFFF, 0x11111111);
		#elif defined(TCC892X) || defined(TCC8925S)
		BITCSET(s_pNAND_BD_GPIO->GPDFN0.nREG, 0xFFFFFFFF, 0x11111111);
		BITCSET(s_pNAND_BD_GPIO->GPDPE.nREG, 0x000000FF, 0x000000FF);	
		BITCSET(s_pNAND_BD_GPIO->GPDPS.nREG, 0x000000FF, 0x000000FF);
		#elif defined(TCC893X)
			#if defined(TCC8930)
			BITCSET(s_pNAND_BD_GPIO->GPDFN0.nREG, 0xFFFFFFFF, 0x11111111);
			BITCSET(s_pNAND_BD_GPIO->GPDPE.nREG, 0x000000FF, 0x000000FF);	
			BITCSET(s_pNAND_BD_GPIO->GPDPS.nREG, 0x000000FF, 0x000000FF);
			#endif
		#else
		#err NAND Port Error
		#endif
	}
}

static void NAND_BD_Init_WE_OE_ALE_CLE_Port(void)
{
	// WE/OE/ALE/CLE
#if defined(TCC88XX)
	// WE = EDI_WE[1] / OE = EDI_OE[1] / ALE = EDI_XA[01] / CLE = EDI_XA[00]
	BITCSET(s_pNAND_BD_GPIO->GPBFN2, 0x00FFF0F0, 0x00111010);
#elif defined(TCC892X) || defined(TCC8925S) || defined(TCC893X)
	// WE = EDI_WE[0] / OE = EDI_OE[0] / ALE = EDI_XA[01] / CLE = EDI_XA[00]
	BITCSET(s_pNAND_BD_GPIO->GPAFN1.nREG, 0x0000FFFF, 0x00001111);
#endif
}

static void NAND_BD_Init_RDY_Port(unsigned int uiMask)
{
	if(uiMask&Hw0)
	{
		//
		// RDY 0
		//
		#if defined(TCC88XX)
		BITCSET(s_pNAND_BD_GPIO->GPBFN3, 0x00F00000, 0x00100000);
		#elif defined(TCC892X) || defined(TCC8925S) || defined(TCC893X)
		BITCSET(s_pNAND_BD_GPIO->GPAFN2.nREG, 0x0000000F, 0x00000001);
		#endif
	}
	if(uiMask&Hw1)
	{
		//
		// Ready/Busy 1
		//
		#if defined(TCC892X) || defined(TCC8925S)
		//BITCSET(s_pNAND_BD_GPIO->GPDFN1.nREG, 0x0000F000, 0x00001000); // RDY1 port is tied to RDY0
		#elif defined(TCC893X)
			#if defined(TCC8930)
			BITCSET(s_pNAND_BD_GPIO->GPDFN1.nREG, 0x0000F000, 0x00001000);
			#endif
		#endif
	}
	if(uiMask&Hw2)
	{
		//
		// Ready/Busy 2
		//
		#if defined(TCC892X) || defined(TCC8925S)
		//BITCSET(s_pNAND_BD_GPIO->GPDFN1.nREG, 0x000F0000, 0x00010000); // RDY2 port is tied to RDY0
		#elif defined(TCC893X)
			#if defined(TCC8930)
			BITCSET(s_pNAND_BD_GPIO->GPDFN1.nREG, 0x000F0000, 0x00010000);
			#endif
		#endif
	}
	if(uiMask&Hw3)
	{
		//
		// Ready/Busy 3
		//
		#if defined(TCC892X) || defined(TCC8925S)
		//BITCSET(s_pNAND_BD_GPIO->GPDFN1.nREG, 0x00F00000, 0x00100000); // RDY3 port is tied to RDY0
		#elif defined(TCC893X)
			#if defined(TCC8930)
			BITCSET(s_pNAND_BD_GPIO->GPDFN1.nREG, 0x00F00000, 0x00100000);
			#endif
		#endif
	}
}

static void NAND_BD_Init_CS_Port(unsigned int uiMask)
{
	if(uiMask&Hw0)
	{
		//
		// CS 0
		//
		#if defined(TCC88XX)
		BITCSET(s_pNAND_BD_GPIO->GPBFN3, 0x0F000000, 0x01000000);	//EDI_CSN[5]=GPIOB[30]
		#elif defined(TCC892X) || defined(TCC8925S) || defined(TCC893X)
		BITCSET(s_pNAND_BD_GPIO->GPAFN1.nREG, 0x000F0000, 0x00010000);
		#endif
	}
	if(uiMask&Hw1)
	{
		//
		// CS 1
		//
		#if defined(TCC88XX)
		BITCSET(s_pNAND_BD_GPIO->GPBFN2, 0xF0000000, 0x10000000);	//EDI_CSN[0]=GPIOB[23]
		#elif defined(TCC892X) || defined(TCC8925S) || defined(TCC893X)
		BITCSET(s_pNAND_BD_GPIO->GPAFN1.nREG, 0x00F00000, 0x00100000);
		#endif
	}
	if(uiMask&Hw2)
	{
		//
		// CS 2
		//
		#if defined(TCC892X) || defined(TCC8925S) || defined(TCC893X)
		BITCSET(s_pNAND_BD_GPIO->GPAFN1.nREG, 0x0F000000, 0x01000000);
		#endif
	}
	if(uiMask&Hw3)
	{
		//
		// CS 3
		//
		#if defined(TCC892X) || defined(TCC8925S) || defined(TCC893X)
		BITCSET(s_pNAND_BD_GPIO->GPAFN1.nREG, 0xF0000000, 0x10000000);
		#endif
	}
}

typedef enum {
	PORT_INIT,
	PORT_HIGH,
	PORT_LOW
} PORT_CTRL_T;
/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static void NAND_BD_Control_WP_Port( PORT_CTRL_T ePortCtrl )
{	
#if defined(TCC88XX_88M_BOARD)
	if(ePortCtrl == PORT_INIT)
	{
		BITCLR(s_pNAND_BD_GPIO->GPBFN3,0x0000F000);
		BITSET(s_pNAND_BD_GPIO->GPBEN,Hw27);
	}
	else if(ePortCtrl == PORT_HIGH)
	{
		BITSET(s_pNAND_BD_GPIO->GPBDAT,Hw27);
	}
	else if(ePortCtrl == PORT_LOW)
	{
		BITCLR(s_pNAND_BD_GPIO->GPBDAT,Hw27);
	}
#elif defined(TCC892X_88M_BOARD)
//=================================================
// NAND Write protect Pin Set
// TCC892X_88M_BOARD
//  hw_rev=0x1005,0x1007	WP : GPIO_B[8]
//  hw_rev=0x1006			WP : GPIO_D[0]
//  hw_rev=0x1008			WP : GPIO_D[8]
//  hw_rev=others			WP : GPIO_F[27]
//=================================================
	if(system_rev == 0x1005 || system_rev == 0x1007)
	{
		if(ePortCtrl == PORT_INIT)
		{
			BITCLR(s_pNAND_BD_GPIO->GPBFN1.nREG,0x0000000F);
			BITSET(s_pNAND_BD_GPIO->GPBEN.nREG,Hw8);
		}
		else if(ePortCtrl == PORT_HIGH)
		{
			BITSET(s_pNAND_BD_GPIO->GPBDAT.nREG,Hw8);
		}
		else if(ePortCtrl == PORT_LOW)
		{
			BITCLR(s_pNAND_BD_GPIO->GPBDAT.nREG,Hw8);
		}
	}
	else if(system_rev == 0x1006)
	{
		if(ePortCtrl == PORT_INIT)
		{
			BITCLR(s_pNAND_BD_GPIO->GPDFN0.nREG,0x0000000F);
			BITSET(s_pNAND_BD_GPIO->GPDEN.nREG,Hw0);
		}
		else if(ePortCtrl == PORT_HIGH)
		{
			BITSET(s_pNAND_BD_GPIO->GPDDAT.nREG,Hw0);
		}
		else if(ePortCtrl == PORT_LOW)
		{
			BITCLR(s_pNAND_BD_GPIO->GPDDAT.nREG,Hw0);
		}
	}
	else if(system_rev == 0x1008)
	{
		if(ePortCtrl == PORT_INIT)
		{
			BITCLR(s_pNAND_BD_GPIO->GPDFN1.nREG,0x0000000F);
			BITSET(s_pNAND_BD_GPIO->GPDEN.nREG,Hw8);
		}
		else if(ePortCtrl == PORT_HIGH)
		{
			BITSET(s_pNAND_BD_GPIO->GPDDAT.nREG,Hw8);
		}
		else if(ePortCtrl == PORT_LOW)
		{
			BITCLR(s_pNAND_BD_GPIO->GPDDAT.nREG,Hw8);
		}
	}
	else
	{
		if(ePortCtrl == PORT_INIT)
		{
			BITCLR(s_pNAND_BD_GPIO->GPFFN3.nREG,0x0000F000);
			BITSET(s_pNAND_BD_GPIO->GPFEN.nREG,Hw27);
		}
		else if(ePortCtrl == PORT_HIGH)
		{
			BITSET(s_pNAND_BD_GPIO->GPFDAT.nREG,Hw27);
		}
		else if(ePortCtrl == PORT_LOW)
		{
			BITCLR(s_pNAND_BD_GPIO->GPFDAT.nREG,Hw27);
		}
	}
	//#if defined(TCC_NAND_WP_A19)
	//BITCLR(s_pNAND_BD_GPIO->GPAFN2.nREG,0x0000F000);
	//#elif defined(TCC_NAND_WP_B15)
	//BITCLR(s_pNAND_BD_GPIO->GPBFN1.nREG,0xF0000000);
	//#elif defined(TCC_NAND_WP_B22)
	//BITCLR(s_pNAND_BD_GPIO->GPBFN2.nREG,0x0F000000);
	//#elif defined(TCC_NAND_WP_B27)
	//BITCLR(s_pNAND_BD_GPIO->GPBFN3.nREG,0x0000F000);
	//#elif defined(TCC_NAND_WP_B28)
	//BITCLR(s_pNAND_BD_GPIO->GPBFN3.nREG,0x000F0000);
	//#elif defined(TCC_NAND_WP_B31)
	//BITCLR(s_pNAND_BD_GPIO->GPBFN3.nREG,0xF0000000);
#elif defined(TCC892X_M805_BOARD)
	if(system_rev == 0x2002
		|| system_rev == 0x2003
		|| system_rev == 0x2004
		|| system_rev == 0x2005
		|| system_rev == 0x2006
		|| system_rev == 0x2007
		|| system_rev == 0x2008
		|| system_rev == 0x2009)
	{
		if(ePortCtrl == PORT_INIT)
		{
			BITCLR(s_pNAND_BD_GPIO->GPDFN0.nREG,0x0000000F);
			BITSET(s_pNAND_BD_GPIO->GPDEN.nREG,Hw0);
		}
		else if(ePortCtrl == PORT_HIGH)
		{
			BITSET(s_pNAND_BD_GPIO->GPDDAT.nREG,Hw0);
		}
		else if(ePortCtrl == PORT_LOW)
		{
			BITCLR(s_pNAND_BD_GPIO->GPDDAT.nREG,Hw0);
		}
	}
	else
	{
		if(ePortCtrl == PORT_INIT)
		{
			BITCLR(s_pNAND_BD_GPIO->GPDFN1.nREG,0x0000000F);
			BITSET(s_pNAND_BD_GPIO->GPDEN.nREG,Hw8);
		}
		else if(ePortCtrl == PORT_HIGH)
		{
			BITSET(s_pNAND_BD_GPIO->GPDDAT.nREG,Hw8);
		}
		else if(ePortCtrl == PORT_LOW)
		{
			BITCLR(s_pNAND_BD_GPIO->GPDDAT.nREG,Hw8);
		}
	}
#elif defined(TCC892X_STB) || defined(TCC893X_STB)
	if(ePortCtrl == PORT_INIT)
	{
		BITCLR(s_pNAND_BD_GPIO->GPFFN3.nREG,0x0000F000);
		BITSET(s_pNAND_BD_GPIO->GPFEN.nREG,Hw27);
	}
	else if(ePortCtrl == PORT_HIGH)
	{
		BITSET(s_pNAND_BD_GPIO->GPFDAT.nREG,Hw27);
	}
	else if(ePortCtrl == PORT_LOW)
	{
		BITCLR(s_pNAND_BD_GPIO->GPFDAT.nREG,Hw27);
	}

#elif defined(TCC8930_88M_BOARD)
	unsigned long flags;

	#if defined(NAND_FOR_KERNEL)
	spin_lock_irqsave(&gpio_lock, flags);
	#endif
	if(ePortCtrl == PORT_INIT)
	{
		BITCLR(s_pNAND_BD_GPIO->GPBFN1.nREG,0x0000000F);
		BITSET(s_pNAND_BD_GPIO->GPBEN.nREG,Hw8);
	}
	else if(ePortCtrl == PORT_HIGH)
	{
		BITSET(s_pNAND_BD_GPIO->GPBDAT.nREG,Hw8);
	}
	else if(ePortCtrl == PORT_LOW)
	{
		BITCLR(s_pNAND_BD_GPIO->GPBDAT.nREG,Hw8);
	}
	#if defined(NAND_FOR_KERNEL)
	spin_unlock_irqrestore(&gpio_lock, flags);
	#endif
#elif defined(TCC8935_88M_BOARD)
	if(ePortCtrl == PORT_INIT)
	{
		BITCLR(s_pNAND_BD_GPIO->GPDFN0.nREG,0x0000000F);
		BITSET(s_pNAND_BD_GPIO->GPDEN.nREG,Hw0);
	}
	else if(ePortCtrl == PORT_HIGH)
	{
		BITSET(s_pNAND_BD_GPIO->GPDDAT.nREG,Hw0);
	}
	else if(ePortCtrl == PORT_LOW)
	{
		BITCLR(s_pNAND_BD_GPIO->GPDDAT.nREG,Hw0);
	}
#else
	#Error: NAND Write Protect Pin Set
#endif
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_BD_Init(unsigned int *puiPortStatus)
{
#if defined(NAND_FOR_KERNEL)
	#if defined(_LINUX_)
		s_pNAND_BD_GPIO 		= (PGPIO)tcc_p2v(GPIO_BASE_ADDRESS);
	#elif defined(_WINCE_)
		s_pNAND_BD_GPIO 		= (PGPIO)tcc_allocbaseaddress((unsigned int)GPIO_BASE_ADDRESS);
	#endif
#else
		s_pNAND_BD_GPIO 		= (PGPIO)(GPIO_BASE_ADDRESS);
#endif

	*puiPortStatus = NAND_PORT_STATUS_DATA_WIDTH_8BIT;

	switch(NAND_BD_Get_BusWidthMax())
	{
		case 8:
			// NAND_D[0:7]
			NAND_BD_Init_DATA_Port(Hw0);
			break;
		case 16:
			// NAND_D[0:15]
			NAND_BD_Init_DATA_Port(Hw0|Hw1);

			*puiPortStatus |= NAND_PORT_STATUS_DATA_WIDTH_16BIT;
			break;
		default:
			NAND_BD_ASSERT(0);
			break;
	}

	switch(NAND_BD_Get_CSCountMax())
	{
		case 1:
			NAND_BD_Init_CS_Port(Hw0/*CS0*/);
			*puiPortStatus |= NAND_PORT_STATUS_NAND_ON_CS0;
			break;
		case 2:
			NAND_BD_Init_CS_Port(Hw0/*CS0*/|Hw1/*CS1*/);
			*puiPortStatus |= (NAND_PORT_STATUS_NAND_ON_CS0|NAND_PORT_STATUS_NAND_ON_CS1);
			break;
		case 4:
			NAND_BD_Init_CS_Port(Hw0/*CS0*/|Hw1/*CS1*/|Hw2/*CS2*/|Hw3/*CS3*/);
			*puiPortStatus |= (NAND_PORT_STATUS_NAND_ON_CS0|NAND_PORT_STATUS_NAND_ON_CS1|NAND_PORT_STATUS_NAND_ON_CS2|NAND_PORT_STATUS_NAND_ON_CS3);
			break;
		default:
			NAND_BD_ASSERT(0);
			break;
	}

	NAND_BD_Init_RDY_Port(Hw0/*RDY0*/);

	NAND_BD_Init_WE_OE_ALE_CLE_Port();

	NAND_BD_Control_WP_Port(PORT_INIT);

	NAND_BD_Init_PowerCheckPin();

#if defined(TCC892X) || defined(TCC8925S) || defined(TCC893X)
	// Maximize the IO strength
	BITSET(s_pNAND_BD_GPIO->GPACD0.nREG, 0xFFFFFFFF);
	BITSET(s_pNAND_BD_GPIO->GPACD1.nREG, 0x00000003);
#endif
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_BD_Enable_WP_Port( void )
{
	NAND_BD_Control_WP_Port(PORT_LOW);
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
unsigned int NAND_BD_Disable_WP_Port( void )
{
	if( NAND_BD_PowerCheck() == FALSE )
	{
		return FALSE;
	}

	NAND_BD_Control_WP_Port(PORT_HIGH);
	return TRUE;
}

