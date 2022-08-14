/*
 * IR driver for remote controller : tcc_remocon.c
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

#include <platform/reg_physical.h>
#include <platform/interrupts.h>
#include <platform/irqs.h>
#include <platform/gpio.h>
#include <dev/gpio.h>

#include "tcc_remocon.h"

unsigned long rmt_clk_rate;
unsigned int gLastKeyCode;
unsigned long long Buf = 0;
int BitCnt = 0;

#define nop_delay(x) for(int cnt=0 ; cnt<x ; cnt++){ \
					__asm__ __volatile__ ("nop\n"); }

static void tcc_remocon_set_io(void)
{
	#if defined(PLATFORM_TCC892X)
		PGPIO pGpioG = (volatile PGPIO)tcc_p2v(HwGPIOG_BASE);
		BITSET(pGpioG->GPGIS.nREG, Hw17);
	#elif defined(PLATFORM_TCC893X)
		if (HW_REV == 0x7231)
		{
			PGPIO pGpioC = (volatile PGPIO)tcc_p2v(HwGPIOC_BASE);
			BITSET(pGpioC->GPCIS.nREG, Hw29);
		}
		else
		{
			PGPIO pGpioG = (volatile PGPIO)tcc_p2v(HwGPIOG_BASE);
			BITSET(pGpioG->GPGIS.nREG, Hw17);
		}
	#endif
}

static void RemoconConfigure(void)
{
	PREMOTECON pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	#if defined(PLATFORM_TCC892X)
		#if defined(PBUS_DIVIDE_CLOCK)
		BITCLR(pRcu->INPOL.nREG, Hw8);
		#else
		BITSET(pRcu->INPOL.nREG, Hw8);
		#endif

		BITSET(pRcu->INPOL.nREG, Hw0);
		BITSET(pRcu->INPOL.nREG, Hw9); //FIL
		BITCLR(pRcu->INPOL.nREG, Hw11); //FT
		BITSET(pRcu->INPOL.nREG, Hw10); //FT
	#elif defined(PLATFORM_TCC893X)
		BITSET(pRcu->INPOL.nREG, Hw0); //INV

		#if defined(PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(PBUS_DIVIDE_CLOCK)
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

		#if defined(PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(PBUS_DIVIDE_CLOCK)
		BITCLR(pRcu->BDD.nREG, Hw23); //BDSC
		#else
		BITSET(pRcu->BDD.nREG, Hw23); //BDSC
		#endif

		BITSET(pRcu->BDD.nREG, Hw24); //BDDC
	#endif

	tcc_remocon_set_io();
} 

static void RemoconStatus(void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	#if defined(PLATFORM_TCC892X)
		BITCLR(pRcu->STA.nREG, Hw13-Hw0); //ICF
		BITSET(pRcu->STA.nREG, Hw12); //OF
	#elif defined(PLATFORM_TCC893X)
		BITCLR(pRcu->STA.nREG, Hw12-Hw0); //ICF
		BITSET(pRcu->STA.nREG, Hw12); //OF
	#endif
}

static void RemoconDivide(int state)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);
	unsigned int	uiclock;
	unsigned int	uidiv;

	#if defined(PLATFORM_TCC892X)
		uiclock = state == 1? 120000*100/2:tca_ckc_getfbusctrl(FBUS_IO)*100;
		uidiv   = uiclock/32768;

		BITCLR(pRcu->CLKDIV.nREG, Hw32-Hw0);
		BITSET(pRcu->CLKDIV.nREG, 0xC8); //END_CNT
		#if defined(PBUS_DIVIDE_CLOCK)
		BITSET(pRcu->CLKDIV.nREG, uidiv<<14); //CLK_DIV
		#else
		BITSET(pRcu->CLKDIV.nREG, 1<<14); //CLK_DIV
		#endif
	#elif defined(PLATFORM_TCC893X)
		uiclock = state == 1 ? 240000*100:(rmt_clk_rate);
		uidiv   = uiclock/32768;

		BITSET(pRcu->CLKDIV.nREG, 0xC8); //END_CNT
		#if defined(PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(PBUS_DIVIDE_CLOCK)
		BITSET(pRcu->CLKDIV.nREG, uidiv<<14); //CLK_DIV
		#else
		BITSET(pRcu->CLKDIV.nREG, Hw14); //CLK_DIV
		#endif
	#endif
}

static void RemoconCommandOpen(void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	#if defined(PLATFORM_TCC892X)
		BITCLR(pRcu->CMD.nREG, Hw0);
		BITSET(pRcu->CMD.nREG, Hw1|Hw2);	

		BITSET(pRcu->CMD.nREG, (0x1E<<3)); //TH

		BITSET(pRcu->CMD.nREG, Hw12); //WS
		BITSET(pRcu->CMD.nREG, Hw13); //DEN
		BITSET(pRcu->CMD.nREG, Hw14); //FWEN
	#elif defined(PLATFORM_TCC893X)
		BITCLR(pRcu->CMD.nREG, Hw0); //FF
		BITSET(pRcu->CMD.nREG, Hw1|Hw2); //EN|CLEAR

		#if defined(PBUS_DIVIDE_CLOCK)
		BITSET(pRcu->CMD.nREG, (IR_FIFO_READ_COUNT<<3)); //TH
		#else
		BITSET(pRcu->CMD.nREG, (0x1E<<3)); //TH
		#endif

		BITSET(pRcu->CMD.nREG, Hw12); //WS
		BITSET(pRcu->CMD.nREG, Hw13); //DEN
		BITSET(pRcu->CMD.nREG, Hw14); //FWEN
	#endif
}

static void RemoconIntClear(void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	#if defined(PLATFORM_TCC892X)
		BITSET(pRcu->INPOL.nREG, Hw0); //INV
		#if defined(PBUS_DIVIDE_CLOCK)
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
	#elif defined(PLATFORM_TCC893X)
		BITSET(pRcu->INPOL.nREG, Hw0); //INV
		#if defined(PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(PBUS_DIVIDE_CLOCK)
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

		#if defined(PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(PBUS_DIVIDE_CLOCK)
		#else
		BITCLR(pRcu->STA.nREG, Hw12-Hw0); //ICF
		#endif

		BITSET(pRcu->CMD.nREG, Hw2); //CLEAR
		BITSET(pRcu->BDD.nREG, Hw1); //BDDR
		BITCLR(pRcu->CMD.nREG, Hw0); //FF
		BITSET(pRcu->CMD.nREG, Hw13); //DEN
		#if defined(PBUS_DIVIDE_CLOCK)
		BITSET(pRcu->STA.nREG, 0x1FE); //ICF
		#else
		BITSET(pRcu->STA.nREG, 0xFFF); //ICF
		#endif

		BITSET(pRcu->BDD.nREG, Hw0); //BE
		BITSET(pRcu->BDD.nREG, Hw5); //BDXD
  
		#if defined(PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(PBUS_DIVIDE_CLOCK)
		BITCLR(pRcu->BDD.nREG, Hw23); //BDSC
		#else
		BITSET(pRcu->BDD.nREG, Hw23); //BDSC
		#endif
		BITSET(pRcu->BDD.nREG, Hw24); //BDDC
		#if defined(PBUS_DIVIDE_CLOCK)
		pRcu->PBD00.nREG = 0x00000004;
		pRcu->PBD01.nREG = 0x00000000;
		#else
		#endif
	#endif
}

static void RemoconInit(int state)
{
	PREMOTECON pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	RemoconDivide(state);
	RemoconIntClear();
}

static int check_keycode()
{
	if (BitCnt != IR_SIGNAL_MAXIMUN_BIT_COUNT)
		return 0;

	int Id = (Buf & IR_ID_MASK);
	int Code = (Buf  & IR_CODE_MASK) >> IR_BUFFER_BIT_SHIFT;

	Buf = 0;
	BitCnt = 0;

	if (Id == REMOCON_ID)
	{
		gLastKeyCode = Code;
		return Code;
	}
	return 0;
}

static int rem_getrawdata(int rd_data)
{
	if ((rd_data > LOW_MIN_VALUE) && (rd_data < LOW_MAX_VALUE))
	{		  
		BitCnt++;
	}
	else if ((rd_data > HIGH_MIN_VALUE) && (rd_data < HIGH_MAX_VALUE))
	{
		Buf |= (((unsigned long long)1) << BitCnt);
		BitCnt++;
	}
	else
	{
		Buf = 0;
		BitCnt = 0;
	}

	return check_keycode();
}

static enum handler_return remocon_handler(void *arg)
{
	PREMOTECON pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	int i, vcnt;
	int rd_data[IR_FIFO_READ_COUNT];

	#if defined(PLATFORM_TCC892X)

		for(i=0; i<10; i++)
		{
			vcnt = pRcu->FSTA.nREG & 0x3F;
			if(vcnt < IR_FIFO_READ_COUNT)
			{
				nop_delay(1);
			}
			else
			{
				break;
			}
		}

		for(i=0; i<IR_FIFO_READ_COUNT; i++)
		{
			rd_data[i] = pRcu->RDATA.nREG;
		}

		for(i=0; i<IR_FIFO_READ_COUNT; i++)
		{
			if (rem_getrawdata(rd_data[i] & 0xffff) || rem_getrawdata((rd_data[i] >> 16) & 0xffff))
				break;
		}
	#elif defined(PLATFORM_TCC893X)
		for(i=0; i<10; i++)
		{
			vcnt = pRcu->FSTA.nREG & 0x3F;
			if(vcnt < IR_FIFO_READ_COUNT)
			{
				nop_delay(1);
			}
			else
			{
				break;
			}
		}

		for(i=0; i<IR_FIFO_READ_COUNT; i++)
		{
			rd_data[i] = pRcu->RDATA.nREG;
		}

		Buf = ((pRcu->BDR1.nREG & 0x7) << 29)|(pRcu->BDR0.nREG >> 3);
		BitCnt = IR_SIGNAL_MAXIMUN_BIT_COUNT;

		check_keycode();
	#endif

	RemoconInit(0);
}

int initRemocon(void)
{
	PREMOTECON pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	gLastKeyCode = 0;

	#if defined(PLATFORM_TCC892X)
		#if defined(PBUS_DIVIDE_CLOCK)
		#else
		#endif

		gpio_config(TCC_GPG(17), GPIO_FN7|GPIO_OUTPUT|GPIO_LOW);
	#elif defined(PLATFORM_TCC893X)
		#if defined(PBUS_DIVIDE_CLOCK_WITH_XTIN) || defined(PBUS_DIVIDE_CLOCK)
		PPMU pPmu = (volatile PPMU)tcc_p2v(HwPMU_BASE);

		BITSET(pPmu->PMU_CONFIG.nREG, Hw26);
		BITCLR(pPmu->PMU_CONFIG.nREG, Hw25);
		BITSET(pPmu->PMU_CONFIG.nREG, Hw24);
		BITCLR(pPmu->PMU_CONFIG.nREG, Hw23);
		BITCLR(pPmu->PMU_CONFIG.nREG, Hw22|Hw21);
		BITCLR(pPmu->PMU_CONFIG.nREG, Hw20);

		tca_ckc_setperi(PERI_REMOTE, ENABLE, 240000 / 4);
		#else
		tca_ckc_setperi(PERI_REMOTE, ENABLE, 240000);
		#endif

		rmt_clk_rate = tca_ckc_getperi(PERI_REMOTE) * 100;

		if (HW_REV == 0x7231)
			gpio_config(TCC_GPC(29), GPIO_FN9|GPIO_OUTPUT|GPIO_LOW);
		else
			gpio_config(TCC_GPG(17), GPIO_FN14|GPIO_OUTPUT|GPIO_LOW);
	#endif

	RemoconConfigure();
	RemoconStatus();
	RemoconDivide(0);
	RemoconCommandOpen();
	RemoconIntClear();

	#if defined(PLATFORM_TCC893X)
		long long power_code = 0;
		pRcu->SD.nREG = (START_MAX_VALUE<<16)|START_MIN_VALUE;
		pRcu->DBD0.nREG = (LOW_MAX_VALUE<<16)|LOW_MIN_VALUE;
		#if defined(PBUS_DIVIDE_CLOCK)
		pRcu->DBD1.nREG = (REPEAT_MAX_VALUE<<16)|HIGH_MIN_VALUE;
		#else
		pRcu->DBD1.nREG = (HIGH_MAX_VALUE<<16)|HIGH_MIN_VALUE;
		#endif
		pRcu->RBD.nREG = (REPEAT_MAX_VALUE<<16)|REPEAT_MIN_VALUE;
		power_code = (((long long)SCAN_POWER<<16)|(long long)REMOCON_ID)<<3;
		#if defined(PBUS_DIVIDE_CLOCK)
		pRcu->PBD00.nREG = 0x00000004;
		pRcu->PBD01.nREG = 0x00000000;
		#else
		pRcu->PBD00.nREG = power_code&0xFFFFFFFF;
		pRcu->PBD01.nREG = (power_code>>32)&0x00000007;
		#endif
		pRcu->PBD10.nREG = 0xFFFFFFFF;
		pRcu->PBD11.nREG = 0xFFFFFFFF;
	#endif

	return 0;
}

int setUseRemoteIrq(int i)
{
	if (i == 1)
	{
		register_int_handler(INT_REMOCON, &remocon_handler, NULL);
		unmask_interrupt(INT_REMOCON);
	}
	else
	{
		mask_interrupt(INT_REMOCON);
		register_int_handler(INT_REMOCON, NULL, NULL);
		remocon_handler(0);
		#if defined(PLATFORM_TCC892X)
		while(((PPIC)HwPIC_BASE)->STS1.bREG.REMOCON)
		{
			((PPIC)HwPIC_BASE)->CLR1.bREG.REMOCON = 1;
			nop_delay(1);
		}
		#endif
	}
	return 0;
}

int getRemoteKey(void)
{
	return gLastKeyCode;
}

