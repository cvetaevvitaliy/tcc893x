/****************************************************************************
FileName    : kernel/drivers/video/tcc/vioc/tcc_composite.c
Description : 

Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fb.h>

#ifdef CONFIG_PM_RUNTIME
#include <linux/pm_runtime.h>
#endif

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif
#include <mach/bsp.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include "../tccfb.h"
#include "tcc_composite.h"
#include "tcc_composite_internal.h"
#include <mach/tcc_composite_ioctl.h>

#include <mach/tccfb_ioctrl.h>
#include <mach/tca_fb_output.h>
#include <mach/gpio.h>

#include <mach/vioc_outcfg.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_disp.h>
#include <mach/vioc_global.h>
#include <mach/vioc_config.h>
#include <mach/vioc_scaler.h>
#include <mach/tca_display_config.h>

extern void tccxxx_GetAddress(unsigned char format, unsigned int base_Yaddr, unsigned int src_imgx, unsigned int  src_imgy,
									unsigned int start_x, unsigned int start_y, unsigned int* Y, unsigned int* U,unsigned int* V);

extern unsigned int tccfb_output_get_mode(void);

/*****************************************************************************

 VARIABLES

******************************************************************************/

extern char fb_power_state;

/* Debugging stuff */
static int debug = 0;
#define dprintk(msg...)	if (debug) { printk( "tcc_composite: " msg); }

#define TCC_LCDC1_USE

static int				Composite_LCDC_Num = -1;

#define DEVICE_NAME		"composite"
#define COMPOSITE_MINOR		205

#if defined (CONFIG_MACH_TCC9300ST)
#define COMPOSITE_DETECT_GPIO		TCC_GPE(30)
#define COMPOSITE_DETECT_EINTSEL	SEL_GPIOE30
#define COMPOSITE_DETECT_EINTNUM	5
#define COMPOSITE_DETECT_EINT		HwINT0_EI5
#elif defined (CONFIG_MACH_TCC8800ST)
#define COMPOSITE_DETECT_GPIO		TCC_GPF(27)
#define COMPOSITE_DETECT_EINTSEL	SEL_GPIOF27
#define COMPOSITE_DETECT_EINTNUM	7
#define COMPOSITE_DETECT_EINT		HwINT1_EI7
#elif defined (CONFIG_MACH_TCC8920ST)
#define COMPOSITE_DETECT_GPIO		TCC_GPF(1)
#define COMPOSITE_DETECT_EINTSEL	EXTINT_GPIOF_01
#define COMPOSITE_DETECT_EINTNUM	INT_EI7
#define COMPOSITE_DETECT_EINT		1<<INT_EI7
#elif defined(CONFIG_MACH_TCC8930ST)
#define COMPOSITE_DETECT_GPIO		NULL
#define COMPOSITE_DETECT_EINTSEL	0
#define COMPOSITE_DETECT_EINTNUM	0
#define COMPOSITE_DETECT_EINT		NULL
#else
#define COMPOSITE_DETECT_GPIO		NULL
#define COMPOSITE_DETECT_EINTSEL	0
#define COMPOSITE_DETECT_EINTNUM	0
#define COMPOSITE_DETECT_EINT		NULL
#endif

static struct clk *composite_lcdc0_clk;
static struct clk *composite_lcdc1_clk;
static struct clk *composite_dac_clk;
static struct clk *composite_ntscpal_clk;

static char tcc_composite_mode = COMPOSITE_MAX_M;
static char tcc_composite_started = 0;
static char tcc_composite_attached = 0;
static char tcc_composite_starter = 0;

static char composite_plugout = 0;
static char composite_plugout_count = 0;
static char composite_ext_interrupt = 0;

static char composite_bypass_mode = 0;

#if defined(CONFIG_CPU_FREQ_TCC92X) || defined (CONFIG_CPU_FREQ_TCC93XX)
extern struct tcc_freq_table_t gtTvClockLimitTable;
#endif

#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
#define COMPOSITE_EXCLUSIVE_UI_SCALER0		0
#define COMPOSITE_EXCLUSIVE_UI_SCALER1		1
#define COMPOSITE_EXCLUSIVE_UI_DVD_MAX_WD	720
#define COMPOSITE_EXCLUSIVE_UI_DVD_MAX_HT	576
static unsigned int composite_exclusive_ui_idx = 0;
static unsigned int composite_exclusive_ui_onthefly = FALSE;
static exclusive_ui_params composite_exclusive_ui_param;
#endif

static unsigned int gCompositeSuspendStatus = 0;

static struct device *pdev_composite;

#define RDMA_UVI_MAX_WIDTH             3072

/*****************************************************************************

 FUNCTIONS

******************************************************************************/

extern char TCC_FB_LCDC_NumSet(char NumLcdc, char OnOff);
extern void tca_vsync_video_display_enable(void);
extern void tca_vsync_video_display_disable(void);
#if defined(TCC_VIDEO_DISPLAY_BY_VSYNC_INT)
extern void tcc_vsync_set_firstFrameFlag(int firstFrameFlag);
extern int tcc_vsync_get_isVsyncRunning(void);
#endif /* TCC_VIDEO_DISPLAY_BY_VSYNC_INT */


/*****************************************************************************
 Function Name : tcc_composite_ext_handler()
******************************************************************************/
static irqreturn_t tcc_composite_ext_handler(int irq, void *dev_id)
{
	#if defined(CONFIG_MACH_TCC9300ST) || defined(CONFIG_MACH_TCC8800ST)
	PPIC pHwPIC = (volatile PPIC)tcc_p2v(HwVPIC_BASE);
	#elif defined(CONFIG_MACH_TCC8920ST)
	PPIC pHwPIC = (volatile PPIC)tcc_p2v(HwPIC_BASE);
	#endif
	
	dprintk("%s, composite_plugout_count=%d\n", __func__, composite_plugout_count);

	composite_plugout_count++;
	if(composite_plugout_count > 10)
	{
		composite_plugout_count = 0;
		composite_plugout = 1;

	#if defined(CONFIG_MACH_TCC9300ST)
        BITCLR(pHwPIC->INTMSK0, COMPOSITE_DETECT_EINT);
        BITCLR(pHwPIC->IEN0, COMPOSITE_DETECT_EINT);
		BITSET(pHwPIC->CLR0, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8800ST)
        BITCLR(pHwPIC->INTMSK1, COMPOSITE_DETECT_EINT);
        BITCLR(pHwPIC->IEN1, COMPOSITE_DETECT_EINT);
		BITSET(pHwPIC->CLR1, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8920ST)
        BITCLR(pHwPIC->INTMSK0.nREG, COMPOSITE_DETECT_EINT);
        BITCLR(pHwPIC->IEN0.nREG, COMPOSITE_DETECT_EINT);
		BITSET(pHwPIC->CLR0.nREG, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8930ST)
	#endif
	}
	else
	{
	#if defined(CONFIG_MACH_TCC9300ST)
		BITSET(pHwPIC->CLR0, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8800ST)
		BITSET(pHwPIC->CLR1, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8920ST)
		BITSET(pHwPIC->CLR0.nREG, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8930ST)
	#endif
	}

	return IRQ_HANDLED;
}

/*****************************************************************************
 Function Name : tcc_composite_ext_interrupt_sel()
******************************************************************************/
void tcc_composite_ext_interrupt_sel(char ext_int_num, char ext_int_sel)
{
#if defined(CONFIG_MACH_TCC9300ST) || defined(CONFIG_MACH_TCC8800ST)
	char shift_bit;
	#if defined(CONFIG_MACH_TCC9300ST)
	PGPIOINT pHwGPIOINT = (volatile PGPIOINT)tcc_p2v(HwEINTSEL_BASE);
	#else
	PGPIO pHwGPIOINT = (volatile PGPIO)tcc_p2v(HwGPIO_BASE);
	#endif
	PPIC pHwPIC = (volatile PPIC)tcc_p2v(HwVPIC_BASE);
	
	#if defined(CONFIG_MACH_TCC8800ST)
		if(ext_int_num >= 3)
			pHwPIC->EI37SEL = (pHwPIC->EI37SEL | (0x01<<ext_int_num));
	#endif

	if(ext_int_num < 4)
	{
		shift_bit = 8*(ext_int_num-0);
		BITCSET(pHwGPIOINT->EINTSEL0, 0x7F<<shift_bit, ext_int_sel<<shift_bit);
	}
	else if(ext_int_num < 8)
	{
		shift_bit = 8*(ext_int_num-4);
		BITCSET(pHwGPIOINT->EINTSEL1, 0x7F<<shift_bit, ext_int_sel<<shift_bit);
	}
	else if(ext_int_num < 12)
	{
		shift_bit = 8*(ext_int_num-8);
		BITCSET(pHwGPIOINT->EINTSEL2, 0x7F<<shift_bit, ext_int_sel<<shift_bit);
	}
	#if defined(CONFIG_MACH_TCC9300ST)
	else
	{
		shift_bit = 8*(ext_int_num-12);
		BITCSET(pHwGPIOINT->EINTSEL3, 0x7F<<shift_bit, ext_int_sel<<shift_bit);
	}
	#endif
#elif defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
    tcc_gpio_config_ext_intr(ext_int_num, ext_int_sel);
#endif
}

/*****************************************************************************
 Function Name : tcc_composite_ext_interrupt_set()
******************************************************************************/
void tcc_composite_ext_interrupt_set(char onoff)
{
#if defined(CONFIG_MACH_TCC9300ST) || defined(CONFIG_MACH_TCC8800ST) || defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
	int ret, irq_num;
	#if defined(CONFIG_MACH_TCC8920ST)
	PPIC pHwPIC = (volatile PPIC)tcc_p2v(HwPIC_BASE);
	#elif !defined(CONFIG_MACH_TCC8930ST)
	PPIC pHwPIC = (volatile PPIC)tcc_p2v(HwVPIC_BASE);
	#endif
	
	composite_plugout_count = 0;

	#if defined(CONFIG_MACH_TCC9300ST)
		irq_num = IRQ_EI5;
	#elif defined(CONFIG_MACH_TCC8920ST)
		irq_num = INT_EI7;
	#elif defined(CONFIG_MACH_TCC8930ST)
		irq_num = INT_EINT7;
	#endif
	
	if(onoff)
	{
		if(composite_ext_interrupt == 1)
			return;

		tcc_composite_ext_interrupt_sel(COMPOSITE_DETECT_EINTNUM, COMPOSITE_DETECT_EINTSEL);

	#if defined(CONFIG_MACH_TCC9300ST)
		BITCLR(pHwPIC->POL0, COMPOSITE_DETECT_EINT);
		BITCLR(pHwPIC->MODE0, COMPOSITE_DETECT_EINT);
		BITCLR(pHwPIC->MODEA0, COMPOSITE_DETECT_EINT);
		BITSET(pHwPIC->SEL0, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8800ST)
		BITCLR(pHwPIC->POL1, COMPOSITE_DETECT_EINT);
		BITCLR(pHwPIC->MODE1, COMPOSITE_DETECT_EINT);
		BITCLR(pHwPIC->MODEA1, COMPOSITE_DETECT_EINT);
		BITSET(pHwPIC->SEL1, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8920ST)
		BITCLR(pHwPIC->POL0.nREG, COMPOSITE_DETECT_EINT);
		BITCLR(pHwPIC->MODE0.nREG, COMPOSITE_DETECT_EINT);
		BITCLR(pHwPIC->MODEA0.nREG, COMPOSITE_DETECT_EINT);
		BITSET(pHwPIC->SEL0.nREG, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8930ST)
	#endif

		ret = request_irq(irq_num, tcc_composite_ext_handler, IRQF_SHARED, "TCC_COMPOSITE_EXT", tcc_composite_ext_handler);
		if(ret)	
		{
			printk("%s, ret=%d, irq_num=%d, request_irq ERROR!\n", __func__, ret, irq_num);
		}

 	#if defined(CONFIG_MACH_TCC9300ST)
		BITSET(pHwPIC->CLR0, COMPOSITE_DETECT_EINT);
		BITSET(pHwPIC->INTMSK0, COMPOSITE_DETECT_EINT);	
        BITSET(pHwPIC->IEN0, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8800ST)
		BITSET(pHwPIC->CLR1, COMPOSITE_DETECT_EINT);
		BITSET(pHwPIC->INTMSK1, COMPOSITE_DETECT_EINT);	
        BITSET(pHwPIC->IEN1, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8920ST)
        BITSET(pHwPIC->CLR0.nREG, COMPOSITE_DETECT_EINT);
		BITSET(pHwPIC->INTMSK0.nREG, COMPOSITE_DETECT_EINT);	
        BITSET(pHwPIC->IEN0.nREG, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8930ST)
	#endif

		composite_ext_interrupt = 1;
	}
	else
	{
		if(composite_ext_interrupt == 0)
			return;

		free_irq(irq_num, tcc_composite_ext_handler);
		
 	#if defined(CONFIG_MACH_TCC9300ST)
        BITCLR(pHwPIC->INTMSK0, COMPOSITE_DETECT_EINT);
        BITCLR(pHwPIC->IEN0, COMPOSITE_DETECT_EINT);
        BITSET(pHwPIC->CLR0, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8800ST)
        BITCLR(pHwPIC->INTMSK1, COMPOSITE_DETECT_EINT);
        BITCLR(pHwPIC->IEN1, COMPOSITE_DETECT_EINT);
        BITSET(pHwPIC->CLR1, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8920ST)
        BITCLR(pHwPIC->INTMSK0.nREG, COMPOSITE_DETECT_EINT);
        BITCLR(pHwPIC->IEN0.nREG, COMPOSITE_DETECT_EINT);
        BITSET(pHwPIC->CLR0.nREG, COMPOSITE_DETECT_EINT);
	#elif defined(CONFIG_MACH_TCC8930ST)
	#endif

		composite_ext_interrupt = 0;
	}

	dprintk("%s, onoff=%d\n", __func__, onoff);
 #endif
}

/*****************************************************************************
 Function Name : tcc_composite_detect()
******************************************************************************/
int tcc_composite_detect(void)
{
	int detect = true;

	#if defined (CONFIG_MACH_TCC9300ST) || defined(CONFIG_MACH_TCC8800ST) || defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
		#if defined(CONFIG_TCC_OUTPUT_AUTO_ALL)
			if(composite_plugout)
			{
				if(composite_ext_interrupt == 1)
					tcc_composite_ext_interrupt_set(FALSE);

				detect = false;
			}
			else
			{
				if(composite_ext_interrupt)
				{
					detect = true;
				}
				else
				{
					detect = gpio_get_value(COMPOSITE_DETECT_GPIO)? false : true;

					if(detect)
					{
						if(composite_ext_interrupt == 0)
							tcc_composite_ext_interrupt_set(TRUE);
			 		}

					dprintk("%s, detect=%d\n", __func__, detect);
				}
			}
		#elif defined(CONFIG_TCC_OUTPUT_AUTO_HDMI_CVBS) || defined(CONFIG_TCC_OUTPUT_ATTACH_HDMI_CVBS)
			/* Check the HDMI detection */
			#if defined(CONFIG_MACH_TCC9300ST)
				if(gpio_get_value(TCC_GPA(14)))
			#elif defined(CONFIG_MACH_TCC8800ST)
				if(gpio_get_value(TCC_GPD(25)))
			#elif defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
				if(gpio_get_value(TCC_GPHDMI(1)))
			#endif
				{
					detect = false;
				}
 		#elif defined(CONFIG_TCC_OUTPUT_ATTACH_DUAL_AUTO)
			detect = false;
		#endif
	#endif
 
	return detect;
}

/*****************************************************************************
 Function Name : tcc_composite_set_lcdc()
******************************************************************************/
int tcc_composite_set_lcdc(int lcdc_num)
{
	dprintk("%s  new_mode:%d , before_mode:%d \n",__func__, lcdc_num, Composite_LCDC_Num);

	if(lcdc_num == Composite_LCDC_Num)
		return FALSE;
	
	if(lcdc_num)
	{
		Composite_LCDC_Num = 1;
	}
	else
	{
		Composite_LCDC_Num = 0;
	}
	
	return TRUE;
}

/*****************************************************************************
 Function Name : tcc_composite_connect_lcdc()
******************************************************************************/
int tcc_composite_connect_lcdc(int lcdc_num)
{
	PDDICONFIG pHwDDICFG = (volatile PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);
	
	if(lcdc_num)
	{
		pHwDDICFG->NTSCPAL_EN.nREG |= Hw0;	// LCDC1	- default
	}
	else
	{
		pHwDDICFG->NTSCPAL_EN.nREG &= ~Hw0;	// LCDC0
	}

	return 0;
}

/*****************************************************************************
 Function Name : tcc_composite_get_started()
******************************************************************************/
int tcc_composite_get_started(void)
{
	int ret = 0;

	if(tcc_composite_started && !tcc_composite_attached)
		ret = 1;

	return ret;
}

/*****************************************************************************
 Function Name : tcc_composite_get_spec()
******************************************************************************/
void tcc_composite_get_spec(COMPOSITE_MODE_TYPE mode, COMPOSITE_SPEC_TYPE *spec)
{
	switch(mode)
	{
		case NTSC_M:
		case NTSC_M_J:
		case PAL_M:
		case NTSC_443:
		case PSEUDO_PAL:
			spec->composite_clk = 270000;
			spec->composite_bus_width = 8;
			spec->composite_lcd_width = 720;
			spec->composite_lcd_height = 480;
		#ifdef TCC_COMPOSITE_CCIR656
			spec->composite_LPW = 224 - 1; 					// line pulse width
			spec->composite_LPC = 720 * 2 - 1; 				// line pulse count (active horizontal pixel - 1)
			spec->composite_LSWC = 20 - 1;					// line start wait clock (the number of dummy pixel clock - 1)
			spec->composite_LEWC = 32 - 1;					// line end wait clock (the number of dummy pixel clock - 1)
		#else
			spec->composite_LPW = 212 - 1; 					// line pulse width
			spec->composite_LPC = 720 * 2 - 1; 				// line pulse count (active horizontal pixel - 1)
			spec->composite_LSWC = 32 - 1;					// line start wait clock (the number of dummy pixel clock - 1)
			spec->composite_LEWC = 32 - 1;					// line end wait clock (the number of dummy pixel clock - 1)
		#endif

			spec->composite_VDB = 0; 						// Back porch Vsync delay
			spec->composite_VDF = 0; 						// front porch of Vsync delay

			spec->composite_FPW1 = 1 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->composite_FLC1 = 480 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->composite_FSWC1 = 37 - 1;					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->composite_FEWC1 = 7 - 1;					// frame start wait cycle is the number of lines to insert at the begining each frame
			spec->composite_FPW2 = 1 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->composite_FLC2 = 480 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->composite_FSWC2 = 38 - 1;					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->composite_FEWC2 = 6 - 1; 					// frame start wait cycle is the number of lines to insert at the begining each frame
			break;

		case NTSC_N:
		case NTSC_N_J:
		case PAL_N:
		case PAL_B:
		case PAL_G:
		case PAL_H:
		case PAL_I:
		case PSEUDO_NTSC:		
			spec->composite_clk = 270000;
			spec->composite_bus_width = 8;
			spec->composite_lcd_width = 720;
			spec->composite_lcd_height = 576;
			spec->composite_LPW = 128 - 1; 					// line pulse width
			spec->composite_LPC = 720 * 2 - 1; 				// line pulse count (active horizontal pixel - 1)
			spec->composite_LSWC = 138 - 1;					// line start wait clock (the number of dummy pixel clock - 1)
			spec->composite_LEWC = 22 - 1;					// line end wait clock (the number of dummy pixel clock - 1)

			spec->composite_VDB = 0; 						// Back porch Vsync delay
			spec->composite_VDF = 0; 						// front porch of Vsync delay
				
			spec->composite_FPW1 = 1 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->composite_FLC1 = 576 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->composite_FSWC1 = 43-1;					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->composite_FEWC1 = 5-1;					// frame start wait cycle is the number of lines to insert at the begining each frame
			spec->composite_FPW2 = 1 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->composite_FLC2 = 576 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->composite_FSWC2 = 44-1;//4 					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->composite_FEWC2 = 4-1; 					// frame start wait cycle is the number of lines to insert at the begining each frame
			break;

		default:
			break;
	}
}

/*****************************************************************************
 Function Name : tcc_composite_set_lcd2tv()
******************************************************************************/
void tcc_composite_set_lcd2tv(COMPOSITE_MODE_TYPE type)
{
	COMPOSITE_SPEC_TYPE 	spec;
	stLTIMING				CompositeTiming;
	stLCDCTR				LcdCtrlParam;
	PVIOC_DISP				pDISPBase;
	PVIOC_WMIX				pWIXBase;
	PDDICONFIG 				pDDICfg = (PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);
	unsigned int			width, height;
	unsigned int			lcd_peri = 0;

	#define LCDC_CLK_DIV 1
	
	tcc_composite_get_spec(type, &spec);
	
	if(Composite_LCDC_Num)
		lcd_peri = PERI_LCD1;
	else
		lcd_peri = PERI_LCD0;

	//VIOC_DISP_SWReset(Composite_LCDC_Num);

	BITSET(pDDICfg->PWDN.nREG, Hw1);		// PWDN - TVE
	BITCLR(pDDICfg->SWRESET.nREG, Hw1);		// SWRESET - TVE
	BITSET(pDDICfg->SWRESET.nREG, Hw1);		// SWRESET - TVE	
	BITSET(pDDICfg->NTSCPAL_EN.nREG, Hw0);	// NTSCPAL_EN	
	
	if(Composite_LCDC_Num)	
	{
		pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP1);
		pWIXBase =(VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX1); 
	}
	else
	{
		pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP0);
		pWIXBase =(VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX0); 
	}

	tca_ckc_setperi(lcd_peri, ENABLE, 270000);

	width = spec.composite_lcd_width;
	height = spec.composite_lcd_height;
	
	CompositeTiming.lpw = spec.composite_LPW;
	CompositeTiming.lpc = spec.composite_LPC + 1;
	CompositeTiming.lswc = spec.composite_LSWC + 1;
	CompositeTiming.lewc = spec.composite_LEWC + 1;
	
	CompositeTiming.vdb = spec.composite_VDB;
	CompositeTiming.vdf = spec.composite_VDF;
	CompositeTiming.fpw = spec.composite_FPW1;
	CompositeTiming.flc = spec.composite_FLC1;
	CompositeTiming.fswc = spec.composite_FSWC1;
	CompositeTiming.fewc = spec.composite_FEWC1;
	CompositeTiming.fpw2 = spec.composite_FPW2;
	CompositeTiming.flc2 = spec.composite_FLC2;
	CompositeTiming.fswc2 = spec.composite_FSWC2;
	CompositeTiming.fewc2 = spec.composite_FEWC2;
	
	VIOC_DISP_SetTimingParam(pDISPBase, &CompositeTiming);
 
	memset(&LcdCtrlParam, 0, sizeof(LcdCtrlParam));

	#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV) || defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
		LcdCtrlParam.r2ymd = 0;
	#else
		LcdCtrlParam.r2ymd = 3;
	#endif

	LcdCtrlParam.ckg = 1;
	LcdCtrlParam.id= 0;
	LcdCtrlParam.iv = 0;
	LcdCtrlParam.ih = 1;
	LcdCtrlParam.ip = 1;
	LcdCtrlParam.clen = 1;
	LcdCtrlParam.r2y = 1;
	LcdCtrlParam.pxdw = 6;
	LcdCtrlParam.dp = 0;
	LcdCtrlParam.ni = 0;
	LcdCtrlParam.tv = 1;
	LcdCtrlParam.opt = 0;
	LcdCtrlParam.stn = 0;
	LcdCtrlParam.evsel = 0;
	LcdCtrlParam.ovp = 0;

#if defined(CONFIG_ARCH_TCC893X)
	LcdCtrlParam.advi = 1;
#else
	if(Composite_LCDC_Num)	
		LcdCtrlParam.advi = 1;
	else
		LcdCtrlParam.advi = 0;
#endif /* CONFIG_ARCH_TCC893X */

#if defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
	LcdCtrlParam.advi = 0;
#endif

	#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV) || defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
		if(tcc_composite_starter == 0)
			LcdCtrlParam.r2y = 0;
	#endif

	VIOC_DISP_SetControlConfigure(pDISPBase, &LcdCtrlParam);

	VIOC_DISP_SetSize(pDISPBase, width, height);
	VIOC_DISP_SetBGColor(pDISPBase, 0, 0 , 0);

	if(type == NTSC_M)
		pDISPBase->uADVI.nREG = 0xC2;
	else
		pDISPBase->uADVI.nREG = 0x84;

	//VIOC_DISP_TurnOn(pDISPBase);

	//VIOC_WMIX_SetOverlayPriority(pWIXBase, 0);
	VIOC_WMIX_SetSize(pWIXBase, width, height);
	VIOC_WMIX_SetPosition(pWIXBase, 0, 0, 0);
	VIOC_WMIX_SetChromaKey(pWIXBase, 0, 0, 0, 0, 0, 0xF8, 0xFC, 0xF8);
	VIOC_WMIX_SetUpdate(pWIXBase);

	#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV) || defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
		if(tcc_composite_starter == 0)
			VIOC_WMIX_SetBGColor(pWIXBase, 0x00, 0x80, 0x80, 0x00);
		else
			VIOC_WMIX_SetBGColor(pWIXBase, 0x00, 0x00, 0x00, 0xff);
	#else
		VIOC_WMIX_SetBGColor(pWIXBase, 0x00, 0x00, 0x00, 0xff);
	#endif

	if(Composite_LCDC_Num)	
	{
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_SDVENC, VIOC_OUTCFG_DISP1);
	}
	else
	{
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_SDVENC, VIOC_OUTCFG_DISP0);
	}
}

/*****************************************************************************
 Function Name : tcc_composite_get_lcdsize()
******************************************************************************/
void tcc_composite_get_lcdsize(unsigned int *width, unsigned int *height)
{
	PVIOC_DISP pDISPBase;

	if(Composite_LCDC_Num)	
		pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP1);
	else
		pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP0);

	VIOC_DISP_GetSize(pDISPBase, width, height);
}

/*****************************************************************************
 Function Name : tcc_composite_change_formattype()
******************************************************************************/
COMPOSITE_LCDC_IMG_FMT_TYPE tcc_composite_change_formattype(TCC_COMPOSITE_FORMAT_TYPE g2d_format)
{
	COMPOSITE_LCDC_IMG_FMT_TYPE LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_MAX;

	switch(g2d_format)
	{
		case GE_YUV444_sp:
			break;

		case GE_YUV440_sp:
			break;
				
		case GE_YUV422_sp: 
			LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_YUV422SP;
			break;
				
		case GE_YUV420_sp:
			LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_YUV420SP;
			break;

		case GE_YUV411_sp:
		case GE_YUV410_sp:
			break;

		case GE_YUV422_in:
			LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_YUV422ITL0;
			break;

		case GE_YUV420_in:
			LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_YUV420ITL0;
			break;

		case GE_YUV444_sq:
			break;
		case GE_YUV422_sq:
			LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_YUV422SQ;
			break;

		case GE_RGB332:
			LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_RGB332;
			break;
				
		case GE_RGB444:
		case GE_ARGB4444:
			LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_RGB444;
			break;

		case GE_RGB454:
		case GE_ARGB3454:
			break;

		case GE_RGB555:
		case GE_ARGB1555:
			LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_RGB555;
			break;
		case GE_RGB565:
			LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_RGB565;
			break;
			
		case GE_RGB666:
		case GE_ARGB4666:
		case GE_ARGB6666:
			LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_RGB666;
			break;

		case GE_RGB888:
		case GE_ARGB4888:
		case GE_ARGB8888:
			LCDC_fmt = COMPOSITE_LCDC_IMG_FMT_RGB888;
			break;

		default:
			break;
	}

	return LCDC_fmt;
}

/*****************************************************************************
 Function Name: tcc_composite_set_imagebase()
 Parameters
		: nType		: IMG_CH0, IMG_CH1, IMG_CH2
		: nBase0	: Base Address for Y Channel or RGB Channel
					: Valid for 3 Channel
		: nBase1	: Base Address for Cb(U)
					: Valid for IMG_CH0
		: nBase2	: Base Address for Cb(U) Channel
					: Valid for IMG_CH0
******************************************************************************/
void tcc_composite_set_imagebase(unsigned int type, unsigned int base0, unsigned int base1, unsigned int base2)
{
	switch (type) 
	{
		case	IMG_CH0 :
			break;

		case	IMG_CH1 :
			break;

		case	IMG_CH2 :
			break;

		default	:
			break;
	}
}

/*****************************************************************************
 Function Name : tcc_composite_get_offset()
******************************************************************************/
void tcc_composite_get_offset(COMPOSITE_LCDC_IMG_FMT_TYPE fmt, unsigned int width, unsigned int *offset0, unsigned int *offset1)
{
	switch(fmt)
	{
		case COMPOSITE_LCDC_IMG_FMT_1BPP:
		case COMPOSITE_LCDC_IMG_FMT_2BPP:
		case COMPOSITE_LCDC_IMG_FMT_4BPP:
		case COMPOSITE_LCDC_IMG_FMT_8BPP:
		case COMPOSITE_LCDC_IMG_FMT_RGB332:
			*offset0 = width;
			*offset1 = 0;
			break;
			
		case COMPOSITE_LCDC_IMG_FMT_RGB444:
		case COMPOSITE_LCDC_IMG_FMT_RGB565:
		case COMPOSITE_LCDC_IMG_FMT_RGB555:
			*offset0 = width * 2;
			*offset1 = 0;
			break;

		case COMPOSITE_LCDC_IMG_FMT_RGB888:
		case COMPOSITE_LCDC_IMG_FMT_RGB666:
			*offset0 = width * 4;
			*offset1 = 0;
			break;

		case COMPOSITE_LCDC_IMG_FMT_YUV420SP:
		case COMPOSITE_LCDC_IMG_FMT_YUV422SP:
			*offset0 = width;
			*offset1 = width/2;
			break;

		case COMPOSITE_LCDC_IMG_FMT_YUV422SQ:
			*offset0 = width * 2;
			*offset1 = 0;
			break;

		case COMPOSITE_LCDC_IMG_FMT_YUV420ITL0:
		case COMPOSITE_LCDC_IMG_FMT_YUV420ITL1:
		case COMPOSITE_LCDC_IMG_FMT_YUV422ITL0:
		case COMPOSITE_LCDC_IMG_FMT_YUV422ITL1:
			*offset0 = width;
			*offset1 = width;
			break;

		default:
			break;
	}
}

/*****************************************************************************
 Function Name: tcc_composite_set_imageinfo()
 Parameters
		: nType		: IMG_CH0, IMG_CH1, IMG_CH2
		: nHPos		: Horizontal Position
		: nVPos		: Vertical Position
		: nOffset0	: Horizontal Pixel Offset for Y Channel or RGB Channel
		: nOffset1	: Horizontal Pixel Offset for UV Channel
					: Valid for IMG_CH0
		: nHScale	: Horizontal Scale Factor
		: nVScale	: Vertical Scale Factor
		: nFormat	: Endian / YUV / BPP Setting
					: [3:0]	: BPP Information
					: [6:4]	: YUV Format Information
					: [7]		: Endian Information
******************************************************************************/
unsigned int tcc_composite_set_imageinfo (unsigned int flag, unsigned int type, unsigned int width, unsigned int height, unsigned int hpos, unsigned int vpos, unsigned int hoffset0, unsigned int hoffset1, unsigned int hscale, unsigned int vscale)
{
	unsigned int uiCH0Position = 0;
	
	switch (type) 
	{
		case IMG_CH0 :
			break;
			
		case IMG_CH1 :
			break;
			
		case IMG_CH2 :
			break;

		default	:
			break;
	}
	return uiCH0Position;
}

/*****************************************************************************
 Function Name : tcc_composite_set_imagectrl()
******************************************************************************/
void tcc_composite_set_imagectrl(unsigned int flag, unsigned int type, COMPOSITE_LCDC_IMG_CTRL_TYPE *imgctrl)
{
	switch (type) 
	{
		case IMG_CH0:
			break;

		case IMG_CH1:
			break;

		case IMG_CH2:
			break;

		default:
			break;
	}

	switch (type) 
	{
		case IMG_CH0:
			break;

		case IMG_CH1:
			break;

		case IMG_CH2:
			break;

		default:
			break;
	}
}

/*****************************************************************************
 Function Name : tcc_composite_set_bypass()
******************************************************************************/
void tcc_composite_set_bypass(char bypass_mode)
{
	dprintk("%s, bypass_mode=%d\n", __func__, bypass_mode);

	composite_bypass_mode = bypass_mode;
}

static int onthefly_using;
/*****************************************************************************
 Function Name : tcc_composite_update()
******************************************************************************/
// 0 : 3 : layer enable/disable 
#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
extern int onthefly_LastFrame;
extern int enabled_LastFrame;
void tcc_plugout_for_composite(int ch_layer)
{
	unsigned int iSCType;

	#if defined(CONFIG_ARCH_TCC893X)
		#if defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
		iSCType = VIOC_SC1;
		#else
		iSCType = VIOC_SC3;
		#endif
	#else
		iSCType = VIOC_SC1;
	#endif /* CONFIG_ARCH_TCC893X */

	if(ISSET(onthefly_using, 1<<ch_layer))
	{
		VIOC_CONFIG_PlugOut(iSCType);
		BITCLR(onthefly_using, 1 << ch_layer);
	}

}
EXPORT_SYMBOL(tcc_plugout_for_composite);
#endif
#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
extern unsigned int LastFrame;
#endif

void tcc_composite_update(struct tcc_lcdc_image_update *ImageInfo)
{
	VIOC_DISP *pDISPBase;
	VIOC_WMIX *pWMIXBase;
	VIOC_RDMA *pRDMABase;
#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
	VIOC_RDMA *pRDMABase_temp;
#endif		
	VIOC_SC *pSC;
	unsigned int lcd_width = 0, lcd_height = 0, lcd_h_pos = 0, lcd_w_pos = 0;
	unsigned int iSCType;
	unsigned int nRDMB;
	unsigned int interlace_output = 0;

	#if defined(CONFIG_ARCH_TCC893X)
		#if defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
		pSC = (VIOC_SC *)tcc_p2v(HwVIOC_SC1);
		iSCType = VIOC_SC1;
		#else
		pSC = (VIOC_SC *)tcc_p2v(HwVIOC_SC3);
		iSCType = VIOC_SC3;
		#endif
	#else
	pSC = (VIOC_SC *)tcc_p2v(HwVIOC_SC1);
	iSCType = VIOC_SC1;
	#endif /* CONFIG_ARCH_TCC893X */
	
	dprintk("%s enable:%d, layer:%d, fmt:%d, Fw:%d, Fh:%d, Iw:%d, Ih:%d, fmt:%d onthefly:%d\n", __func__, ImageInfo->enable, ImageInfo->Lcdc_layer,
			ImageInfo->fmt,ImageInfo->Frame_width, ImageInfo->Frame_height, ImageInfo->Image_width, ImageInfo->Image_height, ImageInfo->fmt, ImageInfo->on_the_fly);
	
	#if defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_ARCH_TCC893X)
	ImageInfo->Lcdc_layer = 3;
	#else		
	ImageInfo->Lcdc_layer = 2;
	#endif

	if(Composite_LCDC_Num)
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);		

		switch(ImageInfo->Lcdc_layer)
		{			
			case 0:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA04);
				break;
			case 1:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA05);
				break;
			case 2:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA06);
				break;
			case 3:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA07);
				break;
		}
#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
		pRDMABase_temp  = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA05);
#endif		
	}
	else
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
		
		switch(ImageInfo->Lcdc_layer)
		{			
			case 0:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA00);
				break;
			case 1:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA01);
				break;
			case 2:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA02);
				break;
			case 3:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA03);
				break;
		}		
#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
		pRDMABase_temp  = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA01);
#endif		
	}

	if(!ImageInfo->enable)	{
		volatile PVIOC_IREQ_CONFIG pIREQConfig;
		pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

		VIOC_RDMA_SetImageDisable(pRDMABase);		

		nRDMB = Composite_LCDC_Num*4 + ImageInfo->Lcdc_layer;
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<nRDMB), (0x1<<nRDMB)); // rdma reset
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<nRDMB), (0x0<<nRDMB)); // rdma reset

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
		if(enabled_LastFrame){
			VIOC_RDMA_SetImageDisable(pRDMABase_temp);	
		}
#endif	

		if(ImageInfo->MVCframeView != 1)
		{
			if(ISSET(onthefly_using, 1<<ImageInfo->Lcdc_layer))
			{
				dprintk("%s scaler_%d is plugged out!!\n", __func__, iSCType);
			
				VIOC_CONFIG_PlugOut(iSCType);
				BITCLR(onthefly_using, 1 << ImageInfo->Lcdc_layer);

				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+iSCType)), (0x1<<(28+iSCType))); // scaler reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+iSCType)), (0x0<<(28+iSCType))); // scaler reset

			}		
		}		
#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
		LastFrame = 0;
#endif
		return;
	}	

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);
	
	if((!lcd_width) || (!lcd_height)){
		printk(" %s Error :: lcd_width %d, lcd_height %d, hdmi_lcdc=%d, enable=%d\n", __func__,lcd_width, lcd_height, Composite_LCDC_Num, ImageInfo->enable);
		return;
	}
		
	if((ImageInfo->Lcdc_layer >= 4) || (ImageInfo->fmt >TCC_LCDC_IMG_FMT_MAX))
		return;

	#if defined(CONFIG_TCC_VIDEO_DISPLAY_BY_VSYNC_INT)
		if(ImageInfo->outputMode != OUTPUT_COMPOSITE)
			return;
	#endif

	//dprintk("%s lcdc:%d, pRDMA:0x%08x, pWMIX:0x%08x, pDISP:0x%08x, addr0:0x%08x\n", __func__, hdmi_lcdc, pRDMABase, pWMIXBase, pDISPBase, ImageInfo->addr0);

#if defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
	if( !(pDISPBase->uCTRL.nREG & HwDISP_NI )) {//interlace mode
		interlace_output = 1;
	}
#endif

	if((ImageInfo->MVCframeView != 1) && ImageInfo->on_the_fly)
	{
		unsigned int RDMA_NUM;
		RDMA_NUM = Composite_LCDC_Num ? (ImageInfo->Lcdc_layer + VIOC_SC_RDMA_04) : ImageInfo->Lcdc_layer;

		VIOC_SC_SetSrcSize(pSC, ImageInfo->Frame_width, ImageInfo->Frame_height);
		VIOC_SC_SetDstSize (pSC, ImageInfo->Image_width, ImageInfo->Image_height);			// set destination size in scaler
		VIOC_SC_SetOutSize (pSC, ImageInfo->Image_width, ImageInfo->Image_height);			// set output size in scaer
#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
		if(ImageInfo->Frame_width==ImageInfo->Image_width && ImageInfo->Frame_height==ImageInfo->Image_height) {
			VIOC_SC_SetBypass (pSC, ON);
			dprintk("%s scaler_%d is plug in SetBypass ON \n", __func__, iSCType);
		}else {
			VIOC_SC_SetBypass (pSC, OFF);
			dprintk("%s scaler_%d is plug in SetBypass OFF \n", __func__, iSCType);
		}

		if(!onthefly_using)
		{ 
			VIOC_RDMA_SetImageDisable(pRDMABase);
			dprintk("%s scaler_%d is plug in RDMA %d \n", __func__, RDMA_NUM, iSCType);
			BITSET(onthefly_using, 1 << ImageInfo->Lcdc_layer);
			VIOC_CONFIG_PlugIn (iSCType, RDMA_NUM);	
		}
#else
		if(!onthefly_using) 
		{	
			VIOC_RDMA_SetImageDisable(pRDMABase);
			dprintk("%s scaler_%d is plug in RDMA %d \n", __func__, RDMA_NUM, iSCType);
			BITSET(onthefly_using, 1 << ImageInfo->Lcdc_layer);
			VIOC_SC_SetBypass (pSC, OFF);
			VIOC_CONFIG_PlugIn (iSCType, RDMA_NUM);			
		}
#endif
	}
	else
	{
		if(ISSET(onthefly_using, 1<<ImageInfo->Lcdc_layer))
		{
			dprintk("%s scaler_%d is plug out  \n", __func__, iSCType);
			VIOC_RDMA_SetImageDisable(pRDMABase);
			VIOC_CONFIG_PlugOut(iSCType);
			BITCLR(onthefly_using, 1 << ImageInfo->Lcdc_layer);
		}
	}

	if(onthefly_using)
		VIOC_SC_SetUpdate (pSC);
		
	// position
	VIOC_WMIX_SetPosition(pWMIXBase, ImageInfo->Lcdc_layer, ImageInfo->offset_x, ImageInfo->offset_y);
	VIOC_WMIX_SetUpdate(pWMIXBase);	
		
	#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV) || defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
		VIOC_RDMA_SetImageY2REnable(pRDMABase, FALSE);
	#else
		if(ImageInfo->fmt >= TCC_LCDC_IMG_FMT_UYVY && ImageInfo->fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)
		{
			VIOC_RDMA_SetImageY2REnable(pRDMABase, TRUE);
			VIOC_RDMA_SetImageY2RMode(pRDMABase, 1);

			if( ImageInfo->Frame_width <= RDMA_UVI_MAX_WIDTH )
				VIOC_RDMA_SetImageUVIEnable(pRDMABase, TRUE);
			else
				VIOC_RDMA_SetImageUVIEnable(pRDMABase, FALSE);
		}
	#endif

	if(ImageInfo->one_field_only_interlace)
		VIOC_RDMA_SetImageOffset(pRDMABase, ImageInfo->fmt, ImageInfo->Frame_width*2);
	else
		VIOC_RDMA_SetImageOffset(pRDMABase, ImageInfo->fmt, ImageInfo->Frame_width);

	VIOC_RDMA_SetImageFormat(pRDMABase, ImageInfo->fmt);

	if(lcd_width > ImageInfo->Image_width)
		lcd_w_pos = (lcd_width - ImageInfo->Image_width)/2;
	
	if(lcd_height > ImageInfo->Image_height)
		lcd_h_pos = (lcd_height - ImageInfo->Image_height)/2;

	#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV) || defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
		if(composite_bypass_mode)
		{
			ImageInfo->offset_x = lcd_w_pos;
			ImageInfo->offset_y = lcd_h_pos;
		}
	#endif
	
	// scale
	//VIOC_RDMA_SetImageScale(pRDMABase, scale_x, scale_y);
	
	if( !( ((ImageInfo->crop_left == 0) && (ImageInfo->crop_right == ImageInfo->Frame_width)) &&  ((ImageInfo->crop_top == 0) && (ImageInfo->crop_bottom == ImageInfo->Frame_height)))  )
	{
		unsigned int addr_Y = (unsigned int)ImageInfo->addr0;
		unsigned int addr_U = (unsigned int)ImageInfo->addr1;
		unsigned int addr_V = (unsigned int)ImageInfo->addr2;
		
		dprintk(" Image Crop left=[%d], right=[%d], top=[%d], bottom=[%d] \n", ImageInfo->crop_left, ImageInfo->crop_right, ImageInfo->crop_top, ImageInfo->crop_bottom);

		tccxxx_GetAddress(ImageInfo->fmt, (unsigned int)ImageInfo->addr0, ImageInfo->Frame_width, ImageInfo->Frame_height, 		
								ImageInfo->crop_left, ImageInfo->crop_top, &addr_Y, &addr_U, &addr_V);		

		if(ImageInfo->one_field_only_interlace)
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->crop_right - ImageInfo->crop_left, (ImageInfo->crop_bottom - ImageInfo->crop_top)/2);
		else
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->crop_right - ImageInfo->crop_left, ImageInfo->crop_bottom - ImageInfo->crop_top);
		VIOC_RDMA_SetImageBase(pRDMABase, addr_Y, addr_U, addr_V);
	}
	else
	{	
		dprintk(" don't Image Crop left=[%d], right=[%d], top=[%d], bottom=[%d] \n", ImageInfo->crop_left, ImageInfo->crop_right, ImageInfo->crop_top, ImageInfo->crop_bottom);
		if(ImageInfo->one_field_only_interlace)
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->Frame_width, ImageInfo->Frame_height/2);
		else
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->Frame_width, ImageInfo->Frame_height);
		VIOC_RDMA_SetImageBase(pRDMABase, ImageInfo->addr0, ImageInfo->addr1, ImageInfo->addr2);		
	}
		
	VIOC_RDMA_SetImageIntl(pRDMABase, interlace_output);

	VIOC_WMIX_SetUpdate(pWMIXBase);	

	VIOC_RDMA_SetImageEnable(pRDMABase);

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
	if(enabled_LastFrame)
	{
		VIOC_RDMA_SetImageDisable(pRDMABase_temp);
		if(onthefly_LastFrame)
		{
			onthefly_LastFrame = 0;
			VIOC_CONFIG_PlugOut(VIOC_SC0); //printk("\n\n\n\n");
		}
	}
#endif
}

#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
/*****************************************************************************
 Function Name : tcc_composite_process()
******************************************************************************/
void tcc_composite_process(struct tcc_lcdc_image_update *update, char force_update)
{
	COMPOSITE_LCDC_IMG_CTRL_TYPE ImgCtrl;
	unsigned int Y_offset, UV_offset;
	unsigned int img_width = 0, img_height = 0, win_offset_x = 0, win_offset_y = 0;
	unsigned int lcd_w = 0, lcd_h =0;
	unsigned int lcd_ctrl_flag, mixer_interrupt_use = 0;
	exclusive_ui_ar_params aspect_ratio;
	PLCDC pLCDC;
	
	if(Composite_LCDC_Num)
		pLCDC = (volatile PLCDC)tcc_p2v(HwLCDC1_BASE);
	else
		pLCDC = (volatile PLCDC)tcc_p2v(HwLCDC0_BASE);
		
	tcc_composite_get_lcdsize(&lcd_w, &lcd_h);

	img_width = update->Image_width;
	img_height = update->Image_height;

	dprintk("%s, img_w=%d, img_h=%d, lcd_w=%d, lcd_h=%d, plane_w=%d, plane_ht=%d\n", __func__,
			img_width, img_height, lcd_w, lcd_h, composite_exclusive_ui_param.plane_width, composite_exclusive_ui_param.plane_height);

	/* Aspect Ratio Conversion for DVD Contents */
	//if((img_width <= COMPOSITE_EXCLUSIVE_UI_DVD_MAX_WD) && (img_height <= COMPOSITE_EXCLUSIVE_UI_DVD_MAX_HT))
	{
		memset((void *)&aspect_ratio, 0x00, sizeof(exclusive_ui_ar_params));
		TCC_OUTPUT_EXCLUSIVE_UI_CfgAR(Composite_LCDC_Num, img_width, img_height, &aspect_ratio);
	}

	/* Get the parameters for exclusive ui */
	TCC_OUTPUT_EXCLUSIVE_UI_GetParam(&composite_exclusive_ui_param);

	if(composite_exclusive_ui_param.enable)
	{
		/* Clear the on_the_flay for video */
		composite_exclusive_ui_onthefly = FALSE;

		/* Clear update flag and backup an information of the last video */
		TCC_OUTPUT_EXCLUSIVE_UI_SetImage(update);

		/*-------------------------------------*/
		/*           INTERLACE VIDEO           */
		/* Update video data with exclusive ui */
		/*-------------------------------------*/
		if(composite_exclusive_ui_param.interlace)
		{
			/* Interlace BD Contents with 1920x1080 graphic plane */
			if(!TCC_OUTPUT_EXCLUSIVE_UI_ViqeMtoM(Composite_LCDC_Num, img_width, img_height))
			{
				if((img_width == lcd_w) && (img_height == lcd_h))
				{
					/* 1920x1080 COMPOSITE Output */
				}
				else
				{
					/* 1280x720 COMPOSITE Output */
				}
			}
			else
			{
				/* Disable the lcdc channel_1 */
				TCC_OUTPUT_EXCLUSIVE_UI_Direct(Composite_LCDC_Num, FALSE);
				
				/* VIQE and M2M Scaler with On_the_fly */
				/* Check the output format */
				if(ISZERO(pLCDC->LCTRL, HwLCTRL_NI))
					lcd_h = lcd_h >> 1;
			
				if(composite_exclusive_ui_param.clear == TRUE)
				{
					/* M2M Scaler without On_the_fly */
					if(TCC_OUTPUT_EXCLUSIVE_UI_Scaler_OnTheFly(update, Composite_LCDC_Num) < 0)
						composite_exclusive_ui_onthefly = FALSE;
					else
						composite_exclusive_ui_onthefly = TRUE;

					/* Update Screen */
					tcc_composite_update(update);
				}
				else
				{
					TCC_OUTPUT_EXCLUSIVE_UI_Scaler(update, COMPOSITE_EXCLUSIVE_UI_SCALER1, composite_exclusive_ui_param.plane_width, composite_exclusive_ui_param.plane_height, composite_exclusive_ui_idx, 0);

					if(TCC_OUTPUT_EXCLUSIVE_UI_GetUpdate())
					{
						dprintk("E\n");
					}
					else
					{
						/* Overlay Mixer */
						TCC_OUTPUT_EXCLUSIVE_UI_Mixer(update, composite_exclusive_ui_idx, 0);
						/* Update overlay mixer image */
						TCC_OUTPUT_EXCLUSIVE_UI_Restore_Mixer(update, Composite_LCDC_Num, lcd_w, lcd_h, composite_exclusive_ui_idx);
						/* Set the interrupt flag */
						mixer_interrupt_use = 1;
	 				}
				}
			}
		}
		else
		/*-------------------------------------*/
		/*          PROGRESSIVE VIDEO          */
		/* Update video data with exclusive ui */
		/*-------------------------------------*/
		{
			if( (img_width == composite_exclusive_ui_param.plane_width) && (img_width == lcd_w) && (img_width == TCC_FB_OUT_MAX_WIDTH) &&
				(img_height == composite_exclusive_ui_param.plane_height) && (img_height == lcd_h) && (img_height == TCC_FB_OUT_MAX_HEIGHT) )
			{
				/* 1920x1080 COMPOSITE Output */
			}
			else
			{
				/* Disable the lcdc channel_1 */
				TCC_OUTPUT_EXCLUSIVE_UI_Direct(Composite_LCDC_Num, FALSE);

				if(composite_exclusive_ui_param.clear == FALSE)
				{
					/* M2M Scaler without On_the_fly */
					TCC_OUTPUT_EXCLUSIVE_UI_Scaler(update, COMPOSITE_EXCLUSIVE_UI_SCALER0, composite_exclusive_ui_param.plane_width, composite_exclusive_ui_param.plane_height, composite_exclusive_ui_idx, 0);
					/* Overlay Mixer */
					TCC_OUTPUT_EXCLUSIVE_UI_Mixer(update, composite_exclusive_ui_idx, 0);
 				}
				
			#if defined(CONFIG_TCC_VIDEO_DISPLAY_BY_VSYNC_INT)
				/* M2M Scaler without On_the_fly */
				TCC_OUTPUT_EXCLUSIVE_UI_Scaler(update, COMPOSITE_EXCLUSIVE_UI_SCALER0, lcd_w, lcd_h, composite_exclusive_ui_idx, 1);
				//hdmi_exclusive_ui_onthefly = FALSE;
			#else
				/* M2M Scaler with On_the_fly */
				if(TCC_OUTPUT_EXCLUSIVE_UI_Scaler_OnTheFly(update, Composite_LCDC_Num) < 0)
					composite_exclusive_ui_onthefly = FALSE;
				else
					composite_exclusive_ui_onthefly = TRUE;
			#endif
			}

			#if 0 //shkim - on_the_fly
			/* Set the on_the_flay for android UI */
			if(composite_exclusive_ui_onthefly == FALSE)
				TCC_OUTPUT_EXCLUSIVE_UI_SetOnthefly(Composite_LCDC_Num, 1, TRUE);
			else
				TCC_OUTPUT_EXCLUSIVE_UI_SetOnthefly(Composite_LCDC_Num, 1, FALSE);
			#endif
		}

		composite_exclusive_ui_idx = !composite_exclusive_ui_idx;
	}
	
	if(force_update &&  !mixer_interrupt_use)
	{
		tcc_composite_update(update);
	}
}

void tcc_composite_display_image(exclusive_ui_update UpdateImage)
{
	dprintk("%s\n", __func__);
			
	/* M2M Scaler with On_the_fly */
	if(TCC_OUTPUT_EXCLUSIVE_UI_Scaler_OnTheFly(&UpdateImage.image, UpdateImage.lcdc) < 0)
		composite_exclusive_ui_onthefly = FALSE;
	else
		composite_exclusive_ui_onthefly = TRUE;

	/* Update Screen */
	tcc_composite_update(&UpdateImage.image);
}
#endif

/*****************************************************************************
 Function Name : tcc_composite_get_mode()
******************************************************************************/
TCC_COMPOSITE_MODE_TYPE tcc_composite_get_mode(void)
{
	return tcc_composite_mode;
}

/*****************************************************************************
 Function Name : tcc_composite_enabled()
******************************************************************************/
int tcc_composite_enabled(void)
{
	volatile PNTSCPAL_ENCODER_CTRL pHwTVE_VEN = (PNTSCPAL_ENCODER_CTRL)HwNTSCPAL_ENC_CTRL_BASE;

	int iEnabled = 0;
	
	if(pHwTVE_VEN->VENCON.nREG & HwTVEVENCON_EN_EN)
	{
		iEnabled = 1;
	}

	return iEnabled;
}

/*****************************************************************************
 Function Name : tcc_composite_end()
******************************************************************************/
void tcc_composite_end(void)
{
	dprintk("%s, LCDC_Num = %d \n", __func__, Composite_LCDC_Num);

	internal_tve_enable(0, 0);

	if(composite_plugout)
		composite_plugout = 0;

	if(tcc_composite_started)
		tcc_composite_started = 0;
}

/*****************************************************************************
 Function Name : tcc_composite_start()
******************************************************************************/
void tcc_composite_start(TCC_COMPOSITE_MODE_TYPE mode)
{
	COMPOSITE_MODE_TYPE composite_mode;
	PVIOC_DISP				pDISPBase;
	
	pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP1);

	printk("%s mode=%d, lcdc_num=%d\n", __func__, mode, Composite_LCDC_Num);

	tcc_composite_mode = mode;

	if(mode == COMPOSITE_NTST_M)
		composite_mode = NTSC_M;
	else
		composite_mode = PAL_B;

	tcc_composite_connect_lcdc(Composite_LCDC_Num);
	tcc_composite_set_lcd2tv(composite_mode);

	internal_tve_enable(composite_mode, 1);

	tcc_composite_started = 1;
 
}

/*****************************************************************************
 Function Name : tcc_composite_clock_onoff()
******************************************************************************/
void tcc_composite_clock_onoff(char OnOff)
{
	dprintk("%s, ch = %d OnOff = %d \n", __func__, Composite_LCDC_Num, OnOff);

	if(OnOff)
	{
		if(Composite_LCDC_Num)
			clk_enable(composite_lcdc1_clk);
		else
			clk_enable(composite_lcdc0_clk);
		
		clk_enable(composite_dac_clk);
		clk_enable(composite_ntscpal_clk);
	}
	else
	{
		if(Composite_LCDC_Num)
			clk_disable(composite_lcdc1_clk);
		else		
			clk_disable(composite_lcdc0_clk);

		clk_disable(composite_dac_clk);
		clk_disable(composite_ntscpal_clk);
	}
}

/**
 *      tccfb_blank
 *	@blank_mode: the blank mode we want.
 *	@info: frame buffer structure that represents a single frame buffer
 *
 *	Blank the screen if blank_mode != 0, else unblank. Return 0 if
 *	blanking succeeded, != 0 if un-/blanking failed due to e.g. a
 *	video mode which doesn't support it. Implements VESA suspend
 *	and powerdown modes on hardware that supports disabling hsync/vsync:
 *	blank_mode == 2: suspend vsync
 *	blank_mode == 3: suspend hsync
 *	blank_mode == 4: powerdown
 *
 *	Returns negative errno on error, or zero on success.
 *
 */
#ifdef CONFIG_PM_RUNTIME

static int composite_enable(void)
{		
	printk("%s\n", __func__);

	pm_runtime_get_sync(pdev_composite);

	return 0;
}

static int composite_disable(void)
{
	printk("%s\n", __func__);

	pm_runtime_put_sync(pdev_composite);

	return 0;
}

#endif

static int composite_blank(int blank_mode)
{
	int ret = 0;
#if defined(CONFIG_MACH_TCC8920ST)
        return ret;
#endif
	printk("%s : blank(mode=%d)\n", __func__, blank_mode);

#ifdef CONFIG_PM_RUNTIME
	if( (pdev_composite->power.usage_count.counter==1) && (blank_mode == 0))
	{
		// usage_count = 1 ( resume ), blank_mode = 0 ( FB_BLANK_UNBLANK ) is stable state when booting
		// don't call runtime_suspend or resume state 
		printk("%s ### state = [%d] count =[%d] power_cnt=[%d] \n",__func__,blank_mode, pdev_composite->power.usage_count, pdev_composite->power.usage_count.counter);		  
		return 0;
	}

	switch(blank_mode)
	{
		case FB_BLANK_POWERDOWN:
		case FB_BLANK_NORMAL:
			composite_disable();
			break;
		case FB_BLANK_UNBLANK:
			composite_enable();
			break;
		case FB_BLANK_HSYNC_SUSPEND:
		case FB_BLANK_VSYNC_SUSPEND:
		default:
			ret = -EINVAL;
	}
#endif

	return ret;
}



/*****************************************************************************
 Function Name : tcc_composite_ioctl()
******************************************************************************/
static long tcc_composite_ioctl(struct file *file, unsigned int cmd, void *arg)
{
	TCC_COMPOSITE_START_TYPE start;
	struct tcc_lcdc_image_update update;			
	
	dprintk("composite_ioctl IOCTRL[%d], lcdc_num=%d\n", cmd, Composite_LCDC_Num);

	switch(cmd)
	{
		case TCC_COMPOSITE_IOCTL_START:
			if(copy_from_user(&start, arg, sizeof(start)))
				return -EFAULT;

			TCC_OUTPUT_FB_DetachOutput(1);
			
			#if !defined(CONFIG_MACH_TCC8920ST) && !defined(CONFIG_MACH_TCC8930ST)
			start.lcdc = tca_get_output_lcdc_num();
			#endif
			
			TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_COMPOSITE, start.lcdc, TRUE);

			if(start.lcdc != Composite_LCDC_Num)
			{
				tcc_composite_set_lcdc(start.lcdc);
				tcc_composite_clock_onoff(TRUE);
			}

			tcc_composite_start(start.mode);

			TCC_OUTPUT_UPDATE_OnOff(1, TCC_OUTPUT_COMPOSITE);

#if 0//def TCC_VIDEO_DISPLAY_BY_VSYNC_INT
			if( tcc_vsync_get_isVsyncRunning() )
				tca_vsync_video_display_enable();
#endif
			break;

		case TCC_COMPOSITE_IOCTL_UPDATE:
			if(copy_from_user(&update, arg, sizeof(update)))
				return -EFAULT;

			tcc_composite_update(&update);									
			break;
			
		case TCC_COMPOSITE_IOCTL_END:
			tcc_composite_end();

			TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_COMPOSITE, Composite_LCDC_Num, FALSE);			
						
#if 0//def TCC_VIDEO_DISPLAY_BY_VSYNC_INT
			if( tcc_vsync_get_isVsyncRunning() )
				tca_vsync_video_display_disable();
			tcc_vsync_set_firstFrameFlag(1);
#endif
			break;

		case TCC_COMPOSITE_IOCTL_PROCESS:
			#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
				copy_from_user(&update,arg,sizeof(update));
				tcc_composite_process(&update, 0);
				copy_to_user(arg,&update,sizeof(update));
			#endif
			break;

		case TCC_COMPOSITE_IOCTL_ATTACH:
			#if defined(CONFIG_TCC_OUTPUT_ATTACH) && !defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV)// && !defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
				copy_from_user(&start,arg,sizeof(start));
				tcc_composite_mode = start.mode;

				TCC_OUTPUT_FB_DetachOutput(0);
				TCC_OUTPUT_FB_AttachOutput(start.lcdc, tccfb_output_get_mode(), TCC_OUTPUT_COMPOSITE, tcc_composite_mode, 0);

				TCC_OUTPUT_FB_AttachUpdateFlag(start.lcdc);
			#endif
			break;

		case TCC_COMPOSITE_IOCTL_DETACH:
			#if defined(CONFIG_TCC_OUTPUT_ATTACH)
				TCC_OUTPUT_FB_DetachOutput(0);
			#endif
			break;
			
		case TCC_COPOSITE_IOCTL_BLANK:
		{
			unsigned int cmd;

			if (get_user(cmd, (unsigned int __user *) arg))
				return -EFAULT;

			dprintk(KERN_INFO "COMPOSITE: ioctl(TCC_COPOSITE_IOCTL_BLANK :  %d )\n", cmd);

			composite_blank(cmd);

			break;

		}

		case TCC_COPOSITE_IOCTL_GET_SUSPEND_STATUS:
		{
			dprintk(KERN_INFO "COMPOSITE: ioctl(TCC_COPOSITE_IOCTL_GET_SUSPEND_STATUS : %d )\n", gCompositeSuspendStatus);

			put_user(gCompositeSuspendStatus,(unsigned int __user*)arg);

			break;			
		}

		default:
			printk(" Unsupported IOCTL!!!\n");      
			break;			
	}

	return 0;
}

/*****************************************************************************
 Function Name : tcc_composite_attach()
******************************************************************************/
void tcc_composite_attach(char lcdc_num, char mode, char onoff, char starter_flag)
{
	char composite_mode;

	dprintk("%s, lcdc_num=%d, onoff=%d, mode=%d, starter=%d\n", __func__, lcdc_num, onoff, mode, starter_flag);

	
	tcc_composite_attached = onoff;
	tcc_composite_starter = starter_flag;

	if(onoff)
	{
		if(mode >= COMPOSITE_MAX_M)
			composite_mode = COMPOSITE_NTST_M;
		else
			composite_mode = mode;

		tcc_composite_set_lcdc(lcdc_num);
		tcc_composite_clock_onoff(TRUE);
		tcc_composite_start(composite_mode);
	}
	else
	{
		tcc_composite_end();
	}
}

/*****************************************************************************
 Function Name : tcc_composite_open()
******************************************************************************/
static int tcc_composite_open(struct inode *inode, struct file *filp)
{	
	dprintk("%s  \n", __func__);

	tcc_composite_clock_onoff(1);

	return 0;
}

/*****************************************************************************
 Function Name : tcc_composite_release()
******************************************************************************/
static int tcc_composite_release(struct inode *inode, struct file *file)
{
	dprintk(" %s  \n", __func__);

	tcc_composite_clock_onoff(0);

	return 0;
}

#ifdef CONFIG_PM_RUNTIME
int composite_runtime_suspend(struct device *dev)
{
	printk("%s:  \n", __FUNCTION__);

	tcc_composite_end();

	TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_COMPOSITE, Composite_LCDC_Num, FALSE);

	gCompositeSuspendStatus = 1;

	printk("%s: finish \n", __FUNCTION__);

	return 0;
}

int composite_runtime_resume(struct device *dev)
{
	printk("%s:  \n", __FUNCTION__);

	gCompositeSuspendStatus = 0;

	return 0;
}

#endif


#ifdef CONFIG_PM
static int composite_suspend(struct platform_device *pdev, pm_message_t state)
{
	dprintk(KERN_INFO "%s\n", __FUNCTION__);

	return 0;
}

static int composite_resume(struct platform_device *pdev)
{
	dprintk(KERN_INFO "%s\n", __FUNCTION__);

	return 0;
}
#else
#define composite_suspend NULL
#define composite_resume NULL
#endif

static struct file_operations tcc_composite_fops = 
{
	.owner          = THIS_MODULE,
	.unlocked_ioctl = tcc_composite_ioctl,
	.open           = tcc_composite_open,
	.release        = tcc_composite_release,	
};

static struct miscdevice tcc_composite_misc_device =
{
    .minor = COMPOSITE_MINOR,
    .name  = "composite",
    .fops  = &tcc_composite_fops,
};


static int composite_probe(struct platform_device *pdev)
{
	printk("%s  \n", __func__);

	struct tcc_hdmi_platform_data *composite_dev;

	pdev_composite = &pdev->dev;
	
	composite_dev = (struct tcc_hdmi_platform_data *)pdev_composite->platform_data;

	composite_lcdc0_clk = clk_get(0, "lcdc0");
	BUG_ON(composite_lcdc0_clk == NULL);
	composite_lcdc1_clk = clk_get(0, "lcdc1");
	BUG_ON(composite_lcdc1_clk == NULL);
	composite_dac_clk = clk_get(0, "vdac_phy");
	BUG_ON(composite_dac_clk == NULL);
	composite_ntscpal_clk = clk_get(0, "ntscpal");
	BUG_ON(composite_ntscpal_clk == NULL);

	if (misc_register(&tcc_composite_misc_device))
	{
	    printk(KERN_WARNING "COMPOSITE: Couldn't register device 10, %d.\n", COMPOSITE_MINOR);
	    return -EBUSY;
	}

	#if defined(CONFIG_OUTPUT_SKIP_KERNEL_LOGO)
		clk_enable(composite_dac_clk);
		clk_enable(composite_ntscpal_clk);
	#else
		tcc_composite_clock_onoff(1);
		internal_tve_enable(0, 0);
		tcc_composite_clock_onoff(0);
	#endif

	#if defined (CONFIG_MACH_TCC9300ST) || defined(CONFIG_MACH_TCC8800ST)
		#if defined(CONFIG_TCC_OUTPUT_AUTO_ALL)
			tcc_gpio_config(COMPOSITE_DETECT_GPIO, GPIO_FN(0));
			gpio_request(COMPOSITE_DETECT_GPIO, NULL);
			gpio_direction_input(COMPOSITE_DETECT_GPIO);
		#endif
	#endif

#ifdef CONFIG_PM_RUNTIME
	pm_runtime_set_active(pdev_composite);	
	pm_runtime_enable(pdev_composite);  
	pm_runtime_get_noresume(pdev_composite);  //increase usage_count 
#endif


	return 0;
}

static int composite_remove(struct platform_device *pdev)
{
	printk("%s LCDC:%d \n", __func__, Composite_LCDC_Num);

    misc_deregister(&tcc_composite_misc_device);

	if(composite_lcdc0_clk)
		clk_put(composite_lcdc0_clk);
	if(composite_lcdc1_clk)
		clk_put(composite_lcdc1_clk);
	if(composite_dac_clk)
		clk_put(composite_dac_clk);
	if(composite_ntscpal_clk)
		clk_put(composite_ntscpal_clk);

	return 0;
}

#ifdef CONFIG_PM_RUNTIME
static const struct dev_pm_ops composite_pm_ops = {
	.runtime_suspend      = composite_runtime_suspend,
	.runtime_resume       = composite_runtime_resume,
	.suspend	= composite_suspend,
	.resume = composite_resume,
};
#endif



static struct platform_driver tcc_composite = {
	.probe	= composite_probe,
	.remove	= composite_remove,
	.driver	= {
		.name	= "tcc_composite",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM_RUNTIME
		.pm		= &composite_pm_ops,
#endif
	},
};

/*****************************************************************************
 Function Name : tcc_composite_init()
******************************************************************************/
int __init tcc_composite_init(void)
{
	return platform_driver_register(&tcc_composite);
}

/*****************************************************************************
 Function Name : tcc_composite_cleanup()
******************************************************************************/
void __exit tcc_composite_cleanup(void)
{
	platform_driver_unregister(&tcc_composite);
}

module_init(tcc_composite_init);
module_exit(tcc_composite_cleanup);

MODULE_AUTHOR("Telechps");
MODULE_DESCRIPTION("Telechips COMPOSITE Out driver");
MODULE_LICENSE("GPL");


