/*
 * linux/arch/arm/mach-tcc892x/include/mach/vioc_disp.h
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

#ifndef __VIOC_DISP_H__
#define	__VIOC_DISP_H__

#include  <mach/reg_physical.h>


enum {
	VIOC_HDMI_1920X1080P_60Hz = 0,
	VIOC_HDMI_1920X1080P_59Hz,
	VIOC_HDMI_1920X1080P_50Hz,
	VIOC_HDMI_1920X1080I_60Hz,
	VIOC_HDMI_1920X1080I_59Hz,
	VIOC_HDMI_1920X1080I_50Hz,
	VIOC_HDMI_1280X720P_60Hz,
	VIOC_HDMI_1280X720P_59Hz,
	VIOC_HDMI_1280X720P_50Hz,
	VIOC_HDMI_720X480P_60Hz,
	VIOC_HDMI_720X480P_59Hz,
	VIOC_HDMI_720X576P_50Hz,
	VIOC_HDMI_720X480I_60Hz,
	VIOC_HDMI_720X480I_59Hz,
	VIOC_HDMI_720X576I_50Hz,
	VIOC_HDVE_720X480I_60Hz,
	VIOC_HDVE_720X576I_50Hz,
	VIOC_HDVE_720X480P_60Hz,
	VIOC_HDVE_720X576P_50Hz,
	VIOC_HDVE_1280X720P_60Hz,
	VIOC_HDVE_1280X720P_50Hz,
	VIOC_HDVE_1920X1080I_60Hz,
	VIOC_HDVE_1920X1080I_50Hz,
	VIOC_HDVE_1920X1080P_30Hz,
	VIOC_HDVE_SMPTE295I_50Hz,
	VIOC_HDVE_SMPTE295P_30Hz,
	VIOC_SDVE_NTSC_CVBS,
	VIOC_SDVE_PAL_CVBS,
	VIOC_SDVE_NTSC_SVIDEO,
	VIOC_SDVE_PAL_SVIDEO,
	VIOC_SDVE_NTSC_YPbPr,
	VIOC_SDVE_PAL_YPbPr,
	VIOC_SDVE_NTSC_RGB,
	VIOC_SDVE_PAL_RGB,
	VIOC_DEFAUKLT_TIME_MAX
};

typedef struct {
	unsigned int	nType;
	unsigned int	CLKDIV;
	unsigned int	IV;
	unsigned int	IH;
	unsigned int	IP;
	unsigned int	DP;
	unsigned int	NI;
	unsigned int	TV;
	unsigned int	LPW;
	unsigned int	LPC;
	unsigned int	LSWC;
	unsigned int	LEWC;
	unsigned int	FPW;
	unsigned int	FLC;
	unsigned int	FSWC;
	unsigned int	FEWC;
	unsigned int	FPW2;
	unsigned int	FLC2;
	unsigned int	FSWC2;
	unsigned int	FEWC2;
} VIOC_TIMING_INFO;




/* Interface APIs */
extern void VIOC_DISP_SetDefaultTimingParam(VIOC_DISP *pDISP, unsigned int nType);
extern void VIOC_DISP_SetControlConfigure(VIOC_DISP *pDISP, stLCDCTR *pCtrlParam);
extern void VIOC_DISP_SetPXDW(VIOC_DISP *pDISP, unsigned char PXWD);
extern void VIOC_DISP_SetR2YMD(VIOC_DISP *pDISP, unsigned char R2YMD);
extern void VIOC_DISP_SetR2Y(VIOC_DISP *pDISP, unsigned char R2Y);
extern void VIOC_DISP_SetSWAP(VIOC_DISP *pDISP, unsigned char SWAP);
extern void VIOC_DISP_SetSize (VIOC_DISP *pDISP, unsigned int nWidth, unsigned int nHeight);
extern void VIOC_DISP_GetSize(VIOC_DISP *pDISP, unsigned int *nWidth, unsigned int *nHeight);

extern void VIOC_DISP_SetBGColor(VIOC_DISP *pDISP, unsigned int BG0, unsigned int BG1, unsigned int BG2);
extern void VIOC_DISP_SetPosition(VIOC_DISP *pDISP, unsigned int startX, unsigned int startY );
extern void VIOC_DISP_GetPosition(VIOC_DISP *pDISP, unsigned int *startX, unsigned int *startY );
extern void VIOC_DISP_SetColorEnhancement(VIOC_DISP *pDISP, signed char contrast, signed char brightness, signed char hue );
extern void VIOC_DISP_GetColorEnhancement(VIOC_DISP *pDISP, signed char *contrast, signed char *brightness, signed char *hue );
extern void VIOC_DISP_SetClippingEnable(VIOC_DISP *pDISP, unsigned int enable);
extern void VIOC_DISP_GetClippingEnable(VIOC_DISP *pDISP, unsigned int *enable);
extern void VIOC_DISP_SetClipping(VIOC_DISP *pDISP, unsigned int uiUpperLimitY, unsigned int uiLowerLimitY, unsigned int uiUpperLimitUV, unsigned int uiLowerLimitUV);
extern void VIOC_DISP_GetClipping(VIOC_DISP *pDISP, unsigned int *uiUpperLimitY, unsigned int *uiLowerLimitY, unsigned int *uiUpperLimitUV, unsigned int *uiLowerLimitUV);
extern void VIOC_DISP_SetDither(VIOC_DISP *pDISP, unsigned int ditherEn, unsigned int ditherSel, unsigned char mat[4][4]);
extern void VIOC_DISP_SetTimingParam (VIOC_DISP *pDISP, stLTIMING *pTimeParam);
//extern void VIOC_DISP_SetPixelClockDiv(VIOC_DISP *pDISP, stLTIMING *pTimeParam);
extern void VIOC_DISP_SetPixelClockDiv(VIOC_DISP *pDISP, unsigned int div);
extern void VIOC_DISP_TurnOn (VIOC_DISP *pDISP);
extern void VIOC_DISP_TurnOff (VIOC_DISP *pDISP);
extern unsigned int  VIOC_DISP_Get_TurnOnOff(VIOC_DISP *pDISP);
extern int VIOC_DISP_Wait_DisplayDone(VIOC_DISP *pDISP);
extern void VIOC_DISP_SetControl(VIOC_DISP *pDISP, stLCDCPARAM *pLcdParam);
extern void VIOC_DISP_SetPort( void );
extern void VIOC_DISP_SWReset( unsigned int DISP );
extern void VIOC_DISP_DisplayOnOff( unsigned int onOff );
extern void VIOC_DISP_SetIreqMask(VIOC_DISP * pDISP, unsigned int mask, unsigned int set);
extern void VIOC_DISP_IreqHandler( unsigned int vectorID );
#endif
