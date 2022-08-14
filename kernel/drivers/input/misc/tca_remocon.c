/*
 * IR driver for remote controller : tca_remocon.c
 *
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <asm/io.h>
#include <mach/bsp.h>
#include <asm/mach-types.h>
#include "tcc_remocon.h"
#include <linux/clk.h>
#include <linux/err.h>

extern unsigned long rmt_clk_rate;

//======================================================
// REMOCON initialize
//======================================================
void    RemoconCommandOpen (void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

#if defined(CONFIG_ARCH_TCC892X)
	BITCLR(pRcu->CMD.nREG, Hw0);
	BITSET(pRcu->CMD.nREG, Hw1|Hw2);	

	BITSET(pRcu->CMD.nREG, (0x1E<<3)); //TH

	BITSET(pRcu->CMD.nREG, Hw12); //WS
	BITSET(pRcu->CMD.nREG, Hw13); //DEN
	BITSET(pRcu->CMD.nREG, Hw14); //FWEN
#elif defined(CONFIG_ARCH_TCC893X)
	BITCLR(pRcu->CMD.nREG, Hw0); //FF
	BITSET(pRcu->CMD.nREG, Hw1|Hw2); //EN|CLEAR

  #if defined(CONFIG_PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(CONFIG_XTIN_CLOCK)
	BITSET(pRcu->CMD.nREG, (0x1E<<3)); //TH
  #else
	BITSET(pRcu->CMD.nREG, (IR_FIFO_READ_COUNT<<3)); //TH
  #endif

	BITSET(pRcu->CMD.nREG, Hw12); //WS
	BITSET(pRcu->CMD.nREG, Hw13); //DEN
	BITSET(pRcu->CMD.nREG, Hw14); //FWEN
#endif
}

//======================================================
// configure address
//======================================================
void    RemoconConfigure (void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

#if defined(CONFIG_ARCH_TCC892X)
  #if defined(CONFIG_PBUS_DIVIDE_CLOCK)
	BITCLR(pRcu->INPOL.nREG, Hw8);
  #else
	BITSET(pRcu->INPOL.nREG, Hw8);
  #endif
	BITSET(pRcu->INPOL.nREG, Hw0);
	BITSET(pRcu->INPOL.nREG, Hw9); //FIL
	BITCLR(pRcu->INPOL.nREG, Hw11); //FT
	BITSET(pRcu->INPOL.nREG, Hw10); //FT
	tcc_remocon_set_io();
#elif defined(CONFIG_ARCH_TCC893X)
	BITSET(pRcu->INPOL.nREG, Hw0); //INV

  #if defined(CONFIG_PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(CONFIG_PBUS_DIVIDE_CLOCK_WITHOUT_XTIN)
	BITCLR(pRcu->INPOL.nREG, Hw8); //SCLK
  #else
	BITSET(pRcu->INPOL.nREG, Hw8); //SCLK
  #endif
  
	BITSET(pRcu->INPOL.nREG, Hw9); //FIL
	BITCLR(pRcu->INPOL.nREG, Hw11); //FT
	BITSET(pRcu->INPOL.nREG, Hw10); //FT
	BITCLR(pRcu->INPOL.nREG, Hw12); //FCLK
	BITSET(pRcu->INPOL.nREG, Hw13); //CXTIN

	BITSET(pRcu->BDD.nREG, Hw1); //BDDR
	BITSET(pRcu->BDD.nREG, Hw0); //BE
	BITSET(pRcu->BDD.nREG, Hw5); //BDXD
  #if defined(CONFIG_PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(CONFIG_PBUS_DIVIDE_CLOCK_WITHOUT_XTIN)
	BITCLR(pRcu->BDD.nREG, Hw23); //BDSC
  #else
	BITSET(pRcu->BDD.nREG, Hw23); //BDSC
  #endif
	BITSET(pRcu->BDD.nREG, Hw24); //BDDC

	tcc_remocon_set_io();
#endif
}

//======================================================
// REMOCON STATUS : irq enacle and fifo overflow (active low)
//======================================================
void    RemoconStatus (void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

#if defined(CONFIG_ARCH_TCC892X)
	BITCLR(pRcu->STA.nREG, Hw13-Hw0); //ICF
	BITSET(pRcu->STA.nREG, Hw12); //OF
#elif defined(CONFIG_ARCH_TCC893X)
  #if defined(CONFIG_PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(CONFIG_XTIN_CLOCK)
	BITCLR(pRcu->STA.nREG, Hw12-Hw0); //ICF
  #else
	BITCLR(pRcu->STA.nREG, Hw12-Hw0); //ICF
  #endif
	BITSET(pRcu->STA.nREG, Hw12); //OF
#endif
}

//======================================================
// REMOCON DIVIDE enable & ir clk & end count setting
// (end count use remocon clk)
//======================================================
void    RemoconDivide (int state)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);
	unsigned int	uiclock;
	unsigned int	uidiv;
#if defined(CONFIG_ARCH_TCC893X)
	unsigned long remote_clock_rate;
	struct clk *remote_clk = clk_get(0, "remocon");
	if(IS_ERR(remote_clk)) {
		printk("can't find remocon clk driver!");
		remote_clk = NULL;
	} else {
	#if defined(CONFIG_PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(CONFIG_PBUS_DIVIDE_CLOCK_WITHOUT_XTIN)
		remote_clock_rate = rmt_clk_rate;
	#else
		remote_clock_rate = rmt_clk_rate;//clk_get_rate(remote_clk);
	#endif
		uiclock           = state == 1? 240000*100:(remote_clock_rate);
		uidiv             = uiclock/32768;
	}
#else
	uiclock = state == 1? 120000*100/2:tca_ckc_getfbusctrl(FBUS_IO)*100;
	uidiv   = uiclock/32768;
#endif

	//printk("+++++++++++++++++++++++++++++++++++++[%d][%d]\n", uiclock, uidiv);

#if defined(CONFIG_ARCH_TCC892X)
	BITCLR(pRcu->CLKDIV.nREG, Hw32-Hw0);
	BITSET(pRcu->CLKDIV.nREG, 0xC8); //END_CNT
  #if defined(CONFIG_PBUS_DIVIDE_CLOCK)
	BITSET(pRcu->CLKDIV.nREG, uidiv<<14); //CLK_DIV
  #else
	BITSET(pRcu->CLKDIV.nREG, 1<<14); //CLK_DIV
  #endif
#elif defined(CONFIG_ARCH_TCC893X)
	BITSET(pRcu->CLKDIV.nREG, 0xC8); //END_CNT
  #if defined(CONFIG_PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(CONFIG_PBUS_DIVIDE_CLOCK_WITHOUT_XTIN)
	BITSET(pRcu->CLKDIV.nREG, uidiv<<14); //CLK_DIV
  #else
	BITSET(pRcu->CLKDIV.nREG, Hw14); //CLK_DIV
  #endif
#endif
}

//======================================================
// REMOCON diable 
//======================================================
void    DisableRemocon (void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

#if defined(CONFIG_ARCH_TCC892X)
	BITSET(pRcu->CMD.nREG, Hw0); //FF
	BITCLR(pRcu->CMD.nREG, Hw1); //EN
	BITCLR(pRcu->CMD.nREG, Hw2); //CLEAR
	BITCLR(pRcu->CMD.nREG, Hw13); //DEN
	BITCLR(pRcu->CMD.nREG, Hw14); //FWEN
#elif defined(CONFIG_ARCH_TCC893X)
	BITSET(pRcu->CMD.nREG, Hw0); //FF
	BITCLR(pRcu->CMD.nREG, Hw1); //EN
	BITCLR(pRcu->CMD.nREG, Hw2); //CLEAR
	BITCLR(pRcu->CMD.nREG, Hw13); //DEN
	BITCLR(pRcu->CMD.nREG, Hw14); //FWEN
	
	BITCLR(pRcu->BDD.nREG, Hw1); //BDDR
#endif
}

//======================================================
// REMOCON functions
//======================================================
void    RemoconInit (int state)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	RemoconDivide(state);
	RemoconIntClear();
}

void    RemoconIntClear ()
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

#if defined(CONFIG_ARCH_TCC892X)
	BITSET(pRcu->INPOL.nREG, Hw0); //INV
  #if defined(CONFIG_PBUS_DIVIDE_CLOCK)
	BITCLR(pRcu->INPOL.nREG, Hw8); //SCLK
  #else
	BITSET(pRcu->INPOL.nREG, Hw8); //SCLK
  #endif

	BITSET(pRcu->INPOL.nREG, Hw9); //FIL
	BITCLR(pRcu->INPOL.nREG, Hw11); //FT
	BITSET(pRcu->INPOL.nREG, Hw10); //FT
	BITCLR(pRcu->INPOL.nREG, Hw12); //FCLK

	BITCLR(pRcu->CMD.nREG, Hw13); //DEN
	BITCLR(pRcu->CMD.nREG, Hw2); //CLEAR
	BITSET(pRcu->CMD.nREG, Hw0); //FF

	BITSET(pRcu->CMD.nREG, Hw2); //CLEAR
 	BITCLR(pRcu->CMD.nREG, Hw0); //FF
	BITSET(pRcu->CMD.nREG, Hw13); //DEN
	BITSET(pRcu->STA.nREG, 0xFFF); //ICF

#elif defined(CONFIG_ARCH_TCC893X)
	BITSET(pRcu->INPOL.nREG, Hw0); //INV
  #if defined(CONFIG_PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(CONFIG_PBUS_DIVIDE_CLOCK_WITHOUT_XTIN)
	BITCLR(pRcu->INPOL.nREG, Hw8); //SCLK
  #else
	BITSET(pRcu->INPOL.nREG, Hw8); //SCLK
  #endif
	BITSET(pRcu->INPOL.nREG, Hw9); //FIL
	BITCLR(pRcu->INPOL.nREG, Hw11); //FT
	BITSET(pRcu->INPOL.nREG, Hw10); //FT
	BITCLR(pRcu->INPOL.nREG, Hw12); //FCLK
	BITSET(pRcu->INPOL.nREG, Hw13); //CXTIN

	BITCLR(pRcu->CMD.nREG, Hw13); //DEN
	BITCLR(pRcu->CMD.nREG, Hw2); //CLEAR
	BITCLR(pRcu->BDD.nREG, Hw1); //BDDR
	BITSET(pRcu->CMD.nREG, Hw0); //FF

	//udelay(120);

  #if defined(CONFIG_PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(CONFIG_PBUS_DIVIDE_CLOCK_WITHOUT_XTIN)
  #else
	BITCLR(pRcu->STA.nREG, Hw12-Hw0); //ICF
  #endif
	BITSET(pRcu->CMD.nREG, Hw2); //CLEAR
	BITSET(pRcu->BDD.nREG, Hw1); //BDDR
	BITCLR(pRcu->CMD.nREG, Hw0); //FF
	BITSET(pRcu->CMD.nREG, Hw13); //DEN
  #if defined(CONFIG_PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(CONFIG_XTIN_CLOCK)
	BITSET(pRcu->STA.nREG, 0xFFF); //ICF
  #else
	BITSET(pRcu->STA.nREG, 0x1FE); //ICF
  #endif

	BITSET(pRcu->BDD.nREG, Hw0); //BE
	BITSET(pRcu->BDD.nREG, Hw5); //BDXD
  
  #if defined(CONFIG_PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(CONFIG_PBUS_DIVIDE_CLOCK_WITHOUT_XTIN)
	BITCLR(pRcu->BDD.nREG, Hw23); //BDSC
  #else
	BITSET(pRcu->BDD.nREG, Hw23); //BDSC
  #endif
	BITSET(pRcu->BDD.nREG, Hw24); //BDDC
  #if defined(CONFIG_PBUS_DIVIDE_CLOCK_WITHOUT_XTIN)
	pRcu->PBD00.nREG = 0x00000004;
	pRcu->PBD01.nREG = 0x00000000;
  #else
  #endif
#endif
}
