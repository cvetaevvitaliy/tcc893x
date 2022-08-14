/*
 * linux/arch/arm/mach-tcc892x/vioc_outcfg.c
 * Author:  <linux@telechips.com>
 * Created: June 10, 2008
 * Description: TCC VIOC h/w block 
 *
 * Copyright (C) 2008-2009 Telechips
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <linux/kernel.h>
#include <mach/vioc_outcfg.h>
#include <mach/globals.h>
#include <mach/io.h>

/* -------------------------------
2¡¯b00 : Display Device 0 Component
2¡¯b01 : Display Device 1 Component
2¡¯b10 : Display Device 2 Component
2¡¯b11 : NOT USED
---------------------------------*/
void VIOC_OUTCFG_SetOutConfig (unsigned nType, unsigned nDisp)
{
	VIOC_OUTCFG *gpOutConfig;
	gpOutConfig = (VIOC_OUTCFG *)tcc_p2v(HwVIOC_OUTCFG);

	printk ("%s : addr:%p, nType:%d nDisp:%d 0x%x \n", __func__, gpOutConfig, nType, nDisp, tcc_p2v(10));

	switch (nType)
	{
		case VIOC_OUTCFG_HDMI :
			//gpOutConfig->uMISCCFG.bREG.HDMISEL   = nDisp;
			BITCSET(gpOutConfig->uMISCCFG.nREG, 0x3, nDisp) ;
			break;
		case VIOC_OUTCFG_SDVENC :
			//gpOutConfig->uMISCCFG.bREG.SDVESEL   = nDisp;
			BITCSET(gpOutConfig->uMISCCFG.nREG, 0x3 << 4, nDisp << 4) ;
			break;
		case VIOC_OUTCFG_HDVENC :
			//gpOutConfig->uMISCCFG.bREG.HDVESEL   = nDisp;
			BITCSET(gpOutConfig->uMISCCFG.nREG, 0x3 << 8, nDisp << 8) ;
			break;
		case VIOC_OUTCFG_M80 :
			//gpOutConfig->uMISCCFG.bREG.M80SEL    = nDisp;
			BITCSET(gpOutConfig->uMISCCFG.nREG, 0x3 << 12, nDisp << 12) ;
			break;
		case VIOC_OUTCFG_MRGB :
			//gpOutConfig->uMISCCFG.bREG.MRGBSEL   = nDisp;
			BITCSET(gpOutConfig->uMISCCFG.nREG, 0x3 << 16, nDisp << 16) ;
			break;
		default :
			printk ("Not supported type ...");
			break;
	}
}

// vim:ts=4:et:sw=4:sts=4
