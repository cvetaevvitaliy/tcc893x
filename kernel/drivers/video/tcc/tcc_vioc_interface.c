/*
 * linux/drivers/video/tcc/tccfb_interface.c
 *
 * Based on:    Based on s3c2410fb.c, sa1100fb.c and others
 * Author:  <linux@telechips.com>
 * Created: June 10, 2008
 * Description: TCC LCD Controller Frame Buffer Driver
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
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/irq.h>

#include <linux/types.h>
#include <linux/vt_kern.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#include <asm/system.h>

#ifdef CONFIG_PM
#include "tcc_hibernation_logo.h"
#include <linux/pm.h>
#endif
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/mach-types.h>

#include <mach/bsp.h>

#include <plat/pmap.h>
#include <mach/tcc_fb.h>
#include "tccfb.h"
#include <mach/tccfb_ioctrl.h>
#include <mach/TCC_LCD_Interface.h>
#include <mach/tcc_scaler_ctrl.h>

#include <mach/tcc_composite_ioctl.h>
#include <mach/tcc_component_ioctl.h>

#include <mach/tca_lcdc.h>
#include <mach/tca_fb_hdmi.h>
#include <mach/tca_fb_output.h>

#include <linux/tcc_pwm.h>

#include <mach/timex.h>

#include <mach/vioc_outcfg.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_disp.h>
#include <mach/vioc_config.h>
#include <mach/tca_display_config.h>

#if defined(CONFIG_ARCH_TCC892X)
#include <mach/vioc_plugin_tcc892x.h>
#endif

#if defined(CONFIG_ARCH_TCC893X)
#include <mach/vioc_plugin_tcc893x.h>
#endif
#include <mach/vioc_api.h>
#include <mach/vioc_config.h>

#include <mach/vioc_lut.h>
#include <linux/err.h>

#if defined(CONFIG_CLOCK_TABLE)
#include <mach/clocktable.h>
static struct func_clktbl_t *fb_clktbl = NULL;
#endif

/* Debugging stuff */
static int debug = 0;
#define dprintk(msg...)	if (debug) { printk( "VIOC_I: " msg); }

static int screen_debug = 0;
#define sprintk(msg...)	if (screen_debug) { printk( "VIOC scr: " msg); }


struct lcd_struct {
	spinlock_t lock;
	wait_queue_head_t waitq;
	char state;
};
static struct lcd_struct lcdc_struct;


char Fb_Lcdc_num = 0;

static VIOC_DISP * pDISPBase;
static VIOC_WMIX* pWMIXBase;
static VIOC_WMIX* pWMIXBase_HDMI;
static VIOC_RDMA *pRDMABase;

static struct clk *lcdc0_clk;
static struct clk *lcdc1_clk;

#ifdef CONFIG_LCD_CPU_INTERFACE
static struct clk *lcd_si;
#endif//CONFIG_LCD_CPU_INTERFACE

extern TCC_OUTPUT_TYPE	Output_SelectMode;

#if defined(CONFIG_HIBERNATION) 
extern unsigned int do_hibernation;
extern unsigned int do_hibernate_boot;

#if defined(CONFIG_DISPLAY_IMAGE_DURING_QUICKBOOT)
extern int fb_quickboot_display_fb_resume(struct platform_device *dev, unsigned int lcdc);
#endif
        
#if defined(CONFIG_USING_LAST_FRAMEBUFFER)
extern void fb_quickboot_lastframe_display_release(void);
#endif

#endif//CONFIG_HIBERNATION


#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)
struct tccfb_info *fb_lcd_Info = NULL;
#endif

void tcafb_clock_init(void)
{
	lcdc0_clk = clk_get(0, "lcdc0");
	if (IS_ERR(lcdc0_clk))
		lcdc0_clk = NULL;

	lcdc1_clk = clk_get(0, "lcdc1");


#if defined(CONFIG_CLOCK_TABLE)       
	fb_clktbl = clocktable_get("fb_clktbl");
	if(IS_ERR(fb_clktbl))
		fb_clktbl = NULL;    
	else
		clocktable_ctrl(fb_clktbl, 0, CLKTBL_ENABLE);
#endif

	#ifdef CONFIG_LCD_CPU_INTERFACE
	lcd_si = clk_get(0, "lcdsi");
	#endif//

	BUG_ON(lcdc0_clk == NULL);
	BUG_ON(lcdc1_clk == NULL);
	#ifdef CONFIG_LCD_CPU_INTERFACE
	BUG_ON(lcd_si == NULL);
	#endif//

	
}

void tca92xxfb_clock_delete(void)
{
	clk_put(lcdc0_clk);
	clk_put(lcdc1_clk);

	#ifdef CONFIG_LCD_CPU_INTERFACE
	clk_put(lcd_si);
	#endif//

#ifdef CONFIG_CLOCK_TABLE
	if (fb_clktbl) {
		if (fb_clktbl->count)
			clocktable_ctrl(fb_clktbl, 0, CLKTBL_DISABLE);
		clocktable_put(fb_clktbl);
		fb_clktbl = NULL;
	}
#endif
}

static int  tcafb_clock_set(int cmd)
{
	int ret = 0;
    switch (cmd) {
	    case PWR_CMD_OFF:
			#if defined(CONFIG_CLOCK_TABLE)
			if (fb_clktbl)
				clocktable_ctrl(fb_clktbl, 0, CLKTBL_DISABLE);
			#endif
			clk_disable(lcdc0_clk);
			clk_disable(lcdc1_clk);

			#ifdef CONFIG_LCD_CPU_INTERFACE
			clk_disable(lcd_si);
			#endif//

			break;
			
	    case PWR_CMD_ON:
			clk_enable(lcdc0_clk);
			clk_enable(lcdc1_clk);

			#ifdef CONFIG_LCD_CPU_INTERFACE
			clk_enable(lcd_si);
			#endif//

			#if defined(CONFIG_CLOCK_TABLE)
			if (fb_clktbl)
				clocktable_ctrl(fb_clktbl, 0, CLKTBL_ENABLE);
			#endif

			break;

	    default:
			ret = -EINVAL;
	        break;
    }

    return ret;
	
}


/* tccfb_pan_display
 *
 * pandisplay (set) the controller from the given framebuffer
 * information
*/
int tca_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	int ret = 1;

	#ifdef CONFIG_FB_TCC_USE_VSYNC_INTERRUPT
	if(lcdc_struct.state == 0)
		ret = wait_event_interruptible_timeout(lcdc_struct.waitq, lcdc_struct.state == 1, msecs_to_jiffies(50));

	if(!ret)	{
	 	printk("  [%s %d]: tcc_setup_interrupt timed_out!! \n",__func__, ret);
	}
	#else
		msleep(16);
	#endif //CONFIG_FB_TCC_USE_VSYNC_INTERRUPT
	return 0;
}
EXPORT_SYMBOL(tca_fb_pan_display);

void tca_fb_vsync_activate(struct fb_var_screeninfo *var, struct tccfb_info *fbi)
{
	#ifdef CONFIG_FB_TCC_USE_VSYNC_INTERRUPT
	pDISPBase->uLSTATUS.nREG = 0xFFFFFFFF;
	lcdc_struct.state = 0;
	tca_lcdc_interrupt_onoff(TRUE, Fb_Lcdc_num);

	#endif//
}
EXPORT_SYMBOL(tca_fb_vsync_activate);


/* tccfb_activate_var
 * activate (set) the controller from the given framebuffer
 * information
*/
void tca_fb_activate_var(struct tccfb_info *fbi,  struct fb_var_screeninfo *var)
{
	unsigned int imgch, fmt ,  base_addr;
	unsigned int regl, lcd_width, lcd_height, img_width, img_height;


	unsigned int alpha_type = 0, alpha_blending_en = 0;
	unsigned int chromaR, chromaG, chromaB, chroma_en;

	imgch = fbi->fb->node; //FB_driver
	if(fbi->fb->var.bits_per_pixel == 32) 	{
		chroma_en = 0;
		alpha_type = 1;
		alpha_blending_en = 1;
		fmt = TCC_LCDC_IMG_FMT_RGB888;
	}
	else if(fbi->fb->var.bits_per_pixel == 16)	{
		chroma_en = 1;
		alpha_type = 0;
		alpha_blending_en = 0;
		fmt = TCC_LCDC_IMG_FMT_RGB565; 
	}
	else	{
		printk("%s:fb%d Not Supported BPP!\n", __FUNCTION__, fbi->fb->node);
		return;
	}

	chromaR = chromaG = chromaB = 0;

	sprintk("%s: fb%d Supported BPP!\n", __FUNCTION__, fbi->fb->node);

 	base_addr = fbi->map_dma + var->xres * var->yoffset * (var->bits_per_pixel/8);
//	if(fbi->fb->var.yoffset > fbi->fb->var.yres)	{
//		base_addr = PAGE_ALIGN(base_addr);
//	}

	dprintk("%s: fb%d Baddr:0x%x Naddr:0x%x!\n", __FUNCTION__, fbi->fb->node, base_addr, pRDMABase->nBASE0);

	regl = pDISPBase->uLSIZE.nREG;

	lcd_width = (regl & 0xFFFF);
	lcd_height = ((regl>>16) & 0xFFFF);
	img_width = var->xres;
	img_height = var->yres;

	if(img_width > lcd_width)	
		img_width = lcd_width;
	
	if(img_height > lcd_height)	
		img_height = lcd_height;

	
	/* write new registers */
	switch(imgch)
	{
		case 0:
			// default framebuffer 
			VIOC_WMIX_SetPosition(pWMIXBase, imgch, 0, 0);

			VIOC_RDMA_SetImageFormat(pRDMABase, fmt );	//fmt
			VIOC_RDMA_SetImageOffset (pRDMABase, fmt, img_width  );	//offset	
			VIOC_RDMA_SetImageSize (pRDMABase,  img_width, img_height  );	//size	
			VIOC_RDMA_SetImageBase (pRDMABase , base_addr, 0 , 0 );
			VIOC_RDMA_SetImageEnable(pRDMABase);

			VIOC_RDMA_SetImageAlphaSelect(pRDMABase, alpha_type);
			VIOC_RDMA_SetImageAlphaEnable(pRDMABase, alpha_blending_en);

			//overlay setting
			VIOC_WMIX_SetChromaKey(pWMIXBase, imgch, chroma_en, chromaR, chromaG, chromaB, 0xF8, 0xFC, 0xF8);			

			VIOC_WMIX_SetUpdate(pWMIXBase);	
			VIOC_RDMA_SetImageUpdate(pRDMABase);
			break;

		case 1:
			break;

		case 2:
			break;
	}
	sprintk("%s: fb  end\n", __FUNCTION__);

	return;
	
}
EXPORT_SYMBOL(tca_fb_activate_var);

static volatile VIOC_RDMA pRDMA_BackUp;
static volatile VIOC_WMIX pWMIX_BackUp;
static volatile VIOC_WMIX pWMIX_HDMI_BackUp;

#ifdef CONFIG_PM_RUNTIME

void tca_fb_runtime_suspend(void)
{

	printk("### %s:  \n", __FUNCTION__);

	if((system_rev != 0x2002) && (system_rev != 0x2004) && (system_rev != 0x2005) && (system_rev != 0x2006) && (system_rev != 0x2007) && (system_rev != 0x2008) && (system_rev != 0x2009))
	{
		pRDMA_BackUp = *pRDMABase;
		pWMIX_HDMI_BackUp = *pWMIXBase_HDMI;

		#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
		tca_lcdc_interrupt_onoff(FALSE, Fb_Lcdc_num);
		#endif
		
		if (Fb_Lcdc_num)
			disable_irq(INT_VIOC_DEV1);
		else
			disable_irq(INT_VIOC_DEV0);

		VIOC_DISP_TurnOff(pDISPBase);
		VIOC_RDMA_SetImageDisable(pRDMABase);

		tcafb_clock_set(PWR_CMD_OFF);
	}
	else
	{
		VIOC_RDMA *pRDMABase_video;

		#if defined(CONFIG_ARCH_TCC893X)
			pRDMABase_video = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA07);
			//video RDMA must be setting RDMA7

		#else
			if(Fb_Lcdc_num)	
			{
				#if defined(CONFIG_CHIP_TCC8925S)
					pRDMABase_video = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA07);
				#else
					pRDMABase_video = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA06);
				#endif
			}
			else	
			{
				#if defined(CONFIG_CHIP_TCC8925S)
					pRDMABase_video = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA03);
				#else
					pRDMABase_video = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA02);
				#endif
			}			
		#endif

#if defined(CONFIG_MACH_TCC8920)
		VIOC_RDMA_SetImageSize(pRDMABase_video, 0, 0);
		VIOC_RDMA_SetImageUpdate(pRDMABase_video);

		VIOC_RDMA_SetImageSize(pRDMABase, 0, 0);
		VIOC_RDMA_SetImageUpdate(pRDMABase);
#endif

	}
	
}
EXPORT_SYMBOL(tca_fb_runtime_suspend);


void tca_fb_runtime_resume(void)
{


	VIOC_WMIX * pWIXBase;
	unsigned int output_lcdc_num;
	unsigned int lut_number = 0;
	VIOC_LUT *pLUT =(VIOC_LUT*)tcc_p2v(HwVIOC_LUT);

	printk("### %s:  \n", __FUNCTION__);
    
	if((system_rev != 0x2002) && (system_rev != 0x2004) && (system_rev != 0x2005) && (system_rev != 0x2006) && (system_rev != 0x2007) && (system_rev != 0x2008) && (system_rev != 0x2009))
	{
		tcafb_clock_set(PWR_CMD_ON);	
		*pRDMABase = pRDMA_BackUp;
		*pWMIXBase_HDMI = pWMIX_HDMI_BackUp;

#if defined(CONFIG_MACH_TCC8920)		
		//VIOC_RDMA_SetImageSize(pRDMABase, 0, 0);
		VIOC_RDMA_SetImageEnable(pRDMABase);
#else
		VIOC_RDMA_SetImageDisable(pRDMABase);
#endif

	output_lcdc_num = tca_get_output_lcdc_num();
	if(output_lcdc_num)
		pWIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);
	else
		pWIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);

	#if defined(CONFIG_CHIP_TCC8925S)
		#if defined(CONFIG_STB_BOARD_DONGLE)
		VIOC_WMIX_SetOverlayPriority(pWIXBase, 16);		//Image1 - Image0 - Image2 - Image3
		#endif
	#elif defined(CONFIG_ARCH_TCC893X)
		#if (defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)) && defined(CONFIG_STB_BOARD_DONGLE_TCC893X)
		VIOC_WMIX_SetOverlayPriority(pWIXBase, 9);		//Image2 - Image1 - Image0 - Image3
		#else
		VIOC_WMIX_SetOverlayPriority(pWIXBase, 8);		//Image2 - Image0 - Image1 - Image3
		#endif
	#else
		#if defined(CONFIG_MACH_TCC8920ST)
		VIOC_WMIX_SetOverlayPriority(pWIXBase, 0);		//Image3 - Image0 - Image1 - Image2
		#else
		VIOC_WMIX_SetOverlayPriority(pWIXBase, 24);		//Image0 - Image1 - Image2 - Image3
		#endif
	#endif

		VIOC_WMIX_SetUpdate(pWMIXBase);

		if (Fb_Lcdc_num)
			enable_irq(INT_VIOC_DEV1);
		else
			enable_irq(INT_VIOC_DEV0);

		tca_fb_vsync_activate(NULL, NULL);
	}

	if(tca_get_lcd_lcdc_num())
		lut_number = VIOC_LUT_DEV1;
	else
		lut_number = VIOC_LUT_DEV0;					

	VIOC_LUT_ReStore(pLUT, lut_number);

}
EXPORT_SYMBOL(tca_fb_runtime_resume);

#endif



/* suspend and resume support for the lcd controller */
int tca_fb_suspend(struct device *dev)
{
	printk("%s:  \n", __FUNCTION__);
	if((system_rev == 0x2002) || (system_rev == 0x2004) || (system_rev == 0x2005) || (system_rev == 0x2006) || (system_rev == 0x2007) || (system_rev == 0x2008) || (system_rev == 0x2009))
	{
		pRDMA_BackUp = *pRDMABase;
		pWMIX_HDMI_BackUp = *pWMIXBase_HDMI;

		#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
		tca_lcdc_interrupt_onoff(FALSE, Fb_Lcdc_num);
		#endif
		
		if (Fb_Lcdc_num)
			disable_irq(INT_VIOC_DEV1);
		else
			disable_irq(INT_VIOC_DEV0);

		VIOC_DISP_TurnOff(pDISPBase);
		VIOC_RDMA_SetImageDisable(pRDMABase);

		tcafb_clock_set(PWR_CMD_OFF);
	}
	
	return 0;
}
EXPORT_SYMBOL(tca_fb_suspend);


int tca_fb_resume(struct device *dev)
{
	printk("%s:  \n", __FUNCTION__);

	if((system_rev == 0x2002) || (system_rev == 0x2004) || (system_rev == 0x2005) || (system_rev == 0x2006) || (system_rev == 0x2007) || (system_rev == 0x2008) || (system_rev == 0x2009))
	{
		tcafb_clock_set(PWR_CMD_ON);

		*pRDMABase = pRDMA_BackUp;
		*pWMIXBase_HDMI = pWMIX_HDMI_BackUp;
		
		#if defined(CONFIG_MACH_TCC8920)
			VIOC_RDMA_SetImageSize(pRDMABase, 0, 0);
		#endif

		VIOC_RDMA_SetImageEnable(pRDMABase);

		VIOC_WMIX_SetUpdate(pWMIXBase);	

		if (Fb_Lcdc_num)
			enable_irq(INT_VIOC_DEV1);
		else
			enable_irq(INT_VIOC_DEV0);

		tca_fb_vsync_activate(NULL, NULL);
	}	
#if defined(CONFIG_HIBERNATION) 
	else if(do_hibernation)
	{
		tca_fb_vsync_activate(NULL, NULL);
	}
	if( do_hibernate_boot)
	{
		VIOC_WMIX* pWIXBase;
		unsigned int output_lcdc_num;

		output_lcdc_num = tca_get_output_lcdc_num();
		if(output_lcdc_num)
			pWIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);
		else
			pWIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);

		#if defined(CONFIG_CHIP_TCC8925S)
			#if defined(CONFIG_STB_BOARD_DONGLE)
			VIOC_WMIX_SetOverlayPriority(pWIXBase, 16);		//Image1 - Image0 - Image2 - Image3
			#endif
		#elif defined(CONFIG_ARCH_TCC893X)
			#if defined(CONFIG_CHIP_CC8935S) && defined(CONFIG_STB_BOARD_DONGLE_TCC893X)
			VIOC_WMIX_SetOverlayPriority(pWIXBase, 9);		//Image2 - Image1 - Image0 - Image3
			#else
			VIOC_WMIX_SetOverlayPriority(pWIXBase, 8);		//Image2 - Image0 - Image1 - Image3
			#endif
		#else
			#if defined(CONFIG_MACH_TCC8920ST)
			VIOC_WMIX_SetOverlayPriority(pWIXBase, 0);		//Image3 - Image0 - Image1 - Image2
			#else
			VIOC_WMIX_SetOverlayPriority(pWIXBase, 24);		//Image0 - Image1 - Image2 - Image3
			#endif
		#endif

		#if defined(CONFIG_DISPLAY_IMAGE_DURING_QUICKBOOT)        
                    fb_quickboot_display_fb_resume(dev, Fb_Lcdc_num);
              #endif

			VIOC_WMIX_SetUpdate(pWMIXBase);	

	}

#endif
	return 0;
}
EXPORT_SYMBOL(tca_fb_resume);

static void send_vsync_event(struct work_struct *work)
{
	char buf[64];
	char *envp[] = { NULL, NULL };
	struct tccfb_info *fbdev = container_of(work, typeof(*fbdev), vsync_work);

	snprintf(buf, sizeof(buf), "VSYNC=%llu", ktime_to_ns(fbdev->vsync_timestamp));
	envp[0] = buf;
	kobject_uevent_env(&fbdev->dev->kobj, KOBJ_CHANGE, envp);

	dprintk("%s %s\n",__func__, buf );
}

void tca_init_vsync(struct tccfb_info *dev)
{
	#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)
	fb_lcd_Info = dev;
	#endif

    INIT_WORK(&dev->vsync_work, send_vsync_event);
}
EXPORT_SYMBOL(tca_init_vsync);

void tca_vsync_enable(struct tccfb_info *dev, int on)
{
	dprintk("## VSYNC(%d)", on);

	dev->active_vsync = on;
}
EXPORT_SYMBOL(tca_vsync_enable);


irqreturn_t tcc_lcd_handler(int irq, void *dev_id)
{
	static int VIOCFifoUnderRun = 0;
	struct tccfb_info *fbdev = dev_id;
	unsigned int VOICstatus = pDISPBase->uLSTATUS.nREG;

	fbdev->vsync_timestamp = ktime_get();

	sprintk("%s  lcdc_struct.state:0x%x VOICstatus:0x%x, FifoUnderRun:0x%x \n",
				__func__, lcdc_struct.state, VOICstatus,VIOCFifoUnderRun);
	
	if(VOICstatus & VIOC_DISP_IREQ_RU_MASK)
	{
		if(fbdev->active_vsync)	{
			schedule_work(&fbdev->vsync_work);
		}
			
		pDISPBase->uLSTATUS.nREG = VIOC_DISP_IREQ_RU_MASK;

		if(lcdc_struct.state == 0)		{
			lcdc_struct.state = 1;
			wake_up_interruptible(&lcdc_struct.waitq);
		}
		#if !(defined(CONFIG_VIOC_FIFO_UNDER_RUN_COMPENSATION) || defined(TCC_VIDEO_DISPLAY_BY_VSYNC_INT))
		tca_lcdc_interrupt_onoff(FALSE, Fb_Lcdc_num);
		#endif
	}
#ifdef CONFIG_VIOC_FIFO_UNDER_RUN_COMPENSATION
	{
		if(VOICstatus & VIOC_DISP_IREQ_FU_MASK)
		{
			pDISPBase->uLSTATUS.nREG = VIOC_DISP_IREQ_FU_MASK;

			if(VIOCFifoUnderRun == 0)
				VIOC_DISP_TurnOff(pDISPBase);

			VIOCFifoUnderRun ++;
			dprintk("%s FIFO UNDERUN STATUS:0x%x VIOCFifoUnderRun:0x%x\n",__func__, VOICstatus, VIOCFifoUnderRun);	
		}

		if(VOICstatus & VIOC_DISP_IREQ_DD_MASK)
		{
			pDISPBase->uLSTATUS.nREG = VIOC_DISP_IREQ_DD_MASK;

			if(VIOCFifoUnderRun)
				vioc_display_device_reset(Fb_Lcdc_num);

			VIOC_DISP_TurnOn(pDISPBase);

			printk("%s DISABEL DONE Lcdc_num:%d 0x%p  STATUS:0x%x VIOCFifoUnderRun:0x%x \n",
					__func__,Fb_Lcdc_num, pDISPBase, VOICstatus, VIOCFifoUnderRun);	

			VIOCFifoUnderRun = 0;	
		}
	}
#endif//
	return IRQ_HANDLED;
}

int tcc_lcd_interrupt_reg(char SetClear, struct tccfb_info *info)
{
	int ret = 0;
	dprintk("%s SetClear:%d lcdc_num:%d \n",__func__, SetClear, Fb_Lcdc_num);

	if(SetClear)	{

			if(Fb_Lcdc_num)	{
				ret	= request_irq(INT_VIOC_DEV1, tcc_lcd_handler,	IRQ_TYPE_EDGE_FALLING|IRQF_SHARED,
									"TCC_LCD",	info);

			}
			else {
				ret	= request_irq(INT_VIOC_DEV0, tcc_lcd_handler,	IRQ_TYPE_EDGE_FALLING|IRQF_SHARED,
									"TCC_LCD",	info);	
			}		

			tca_lcdc_interrupt_onoff(1, Fb_Lcdc_num);
	}
	else 	{
		if(Fb_Lcdc_num)
			free_irq(INT_VIOC_DEV1, tcc_lcd_handler);
		else
			free_irq(INT_VIOC_DEV0, tcc_lcd_handler);
	}
	return ret;
}
EXPORT_SYMBOL(tcc_lcd_interrupt_reg);

#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)
int tcc_lcd_interrupt_onoff(unsigned int lcdc, unsigned int onoff)
{
	int ret = 0;

	PCKC				pCKC ;
	pCKC = (CKC *)tcc_p2v(HwCKC_BASE);

	if(onoff)
	{
		if(lcdc){						
			tca_lcdc_interrupt_onoff(0, lcdc);
			disable_irq(INT_VIOC_DEV1);

			ret	= request_irq(INT_VIOC_DEV1, tcc_lcd_handler,	IRQ_TYPE_EDGE_FALLING|IRQF_SHARED,
							"TCC_LCD",	(fb_lcd_Info !=NULL)? fb_lcd_Info:tcc_lcd_handler);

			tca_lcdc_interrupt_onoff(1, lcdc);
	 	}
		else 
		{
			tca_lcdc_interrupt_onoff(0, lcdc);
			disable_irq(INT_VIOC_DEV0);

			ret	= request_irq(INT_VIOC_DEV0, tcc_lcd_handler,	IRQ_TYPE_EDGE_FALLING|IRQF_SHARED,
							"TCC_LCD",	(fb_lcd_Info !=NULL)? fb_lcd_Info:tcc_lcd_handler);				

			tca_lcdc_interrupt_onoff(1, lcdc);
		 }				
	}
	else 	
	{
		tca_lcdc_interrupt_onoff(0, lcdc); 
	
		if(lcdc)
		{
			disable_irq(INT_VIOC_DEV1);
			free_irq(INT_VIOC_DEV1, fb_lcd_Info);
		}
		else
		{
			disable_irq(INT_VIOC_DEV0);
			free_irq(INT_VIOC_DEV0, fb_lcd_Info);
		}
	}	
}EXPORT_SYMBOL(tcc_lcd_interrupt_onoff);

void tca_lcdc_change_disp_port(unsigned int lcd_org_num, unsigned int lcd_dst_num)
{
	if(lcd_dst_num)
	{
		Fb_Lcdc_num = 1;
		pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA04);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);

		pWMIXBase_HDMI = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
		pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP1);
	}
	else
	{		
		Fb_Lcdc_num = 0;
		pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA00);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
		
		pWMIXBase_HDMI = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);
		pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP0);		

	}
	vioc_disp_port_change(lcd_org_num, lcd_dst_num);

}
EXPORT_SYMBOL(tca_lcdc_change_disp_port);

#endif
	
int tccfb_set_wmixer_layer_order(unsigned int num, unsigned int order, unsigned int fb_power_state)
{
	VIOC_WMIX * pWMIX;
	unsigned int old_layer_order = 0;

	if(fb_power_state)
		pWMIX = (VIOC_WMIX *)tcc_p2v((num == 0)?HwVIOC_WMIX0:HwVIOC_WMIX1); 
	else
		pWMIX = (VIOC_WMIX *)&pWMIX_BackUp;
	
	VIOC_WMIX_GetOverlayPriority(pWMIX, &old_layer_order);

	printk("[%s] disp_num:%d, pWMIX:%p, ovp(%d -> %d), fb_power_state:%d\n", 
		__func__, num, pWMIX, old_layer_order, order, fb_power_state);	

	if(fb_power_state)
		VIOC_WMIX_SetOverlayPriority(pWMIX, order);
	else
		BITCSET(pWMIX_BackUp.uCTRL.nREG, 0x1F, order);

	return old_layer_order;
}
EXPORT_SYMBOL(tccfb_set_wmixer_layer_order);


#if defined(CONFIG_ARCH_TCC893X)

#define	CFG_DEV_SEL		(HwVIOC_CONFIG+0x00BC)	//0x7200A0BC
#define	CFG_MISC		(HwVIOC_CONFIG+0x0084)	//0x7200A084

int tca_fb_output_path_init(void)
{
	volatile int DEV_SEL_WMIX_DISP = (volatile int)tcc_p2v(CFG_DEV_SEL);
	volatile int CFG_MISC_DISP_OUTPUT = (volatile int)tcc_p2v(CFG_MISC);

	BITCSET(DEV_SEL_WMIX_DISP, DEV0_PATH, 1 << 0);	// WMIX 1 - DISP 0
	BITCSET(DEV_SEL_WMIX_DISP, DEV1_PATH, 0 << 8); //  WMIX 0 - DISP 1
	BITCSET(DEV_SEL_WMIX_DISP, P1_EN, (1 << 15));
	BITCSET(DEV_SEL_WMIX_DISP, P0_EN, (1 << 7)); 		
	
	#if defined(CONFIG_CHIP_TCC8930)
	
	BITCSET(CFG_MISC_DISP_OUTPUT, LCD0_SEL, 1 << 24);	// DISP 1 - LCD, 
	BITCSET(CFG_MISC_DISP_OUTPUT, LCD1_SEL, 0 << 26);	// DISP 0 - HDMI

	#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
	
	BITCSET(CFG_MISC_DISP_OUTPUT, LCD0_SEL, 0 << 24 );	// DISP 0 - LCD, 
	BITCSET(CFG_MISC_DISP_OUTPUT, LCD1_SEL, 1 << 26);	// DISP 1 - HDMI
	
	#endif	
	
	return true;
}
#endif

int tca_fb_init(void)
{
	pmap_t pmap_fb_video;

	printk(KERN_INFO " tcc892X %s (built %s %s)\n", __func__, __DATE__, __TIME__);

	pmap_get_info("fb_video", &pmap_fb_video);

	Fb_Lcdc_num = tca_get_lcd_lcdc_num();


#if defined(CONFIG_ARCH_TCC893X)

#if 0// just temp!!!
		pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA04);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);
		pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP0);
		// LCD must be setting RDMA 4 - WMIXER 1 - DISP 0 - choose output path (lcd or hdmi number)

		tca_fb_output_path_init();
#else
		// just temp!!!
		#if defined(CONFIG_CHIP_TCC8930)
		
		pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA04);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);
		pWMIXBase_HDMI = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);

		pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP1);

		#else
		pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA00);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
		pWMIXBase_HDMI = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);
		pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP0);		

		#endif
#endif


#else

	if(Fb_Lcdc_num){
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_MRGB, VIOC_OUTCFG_DISP1);
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_HDMI, VIOC_OUTCFG_DISP0);

		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);
		pWMIXBase_HDMI = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
		pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP1);
		pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA04);
	} else {
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_MRGB, VIOC_OUTCFG_DISP0);
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_HDMI, VIOC_OUTCFG_DISP1);

		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
		pWMIXBase_HDMI = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);
		pDISPBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP0);
		pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA00);
	}
#endif

	//hdmi wmixer priority setting
	VIOC_WMIX_SetOverlayPriority(pWMIXBase_HDMI,24);

	tcafb_clock_init();
	tcafb_clock_set(PWR_CMD_ON);

	TCC_OUTPUT_LCDC_Init();
	init_waitqueue_head(&lcdc_struct.waitq);
	spin_lock_init(&lcdc_struct.lock) ;

	lcdc_struct.state = 1;

	VIOC_LUT_Set_LutBackUp_Init();

	printk(" %s LCDC:%d  pRDMABase:%p  pWMIXBase:%p pDISPBase:%p end \n", __func__, Fb_Lcdc_num, pRDMABase, pWMIXBase, pDISPBase);

	return 0;
}
EXPORT_SYMBOL(tca_fb_init);


void tca_fb_exit(void)
{
	dprintk(" %s LCDC:%d \n", __func__, Fb_Lcdc_num);

    #if defined(CONFIG_HIBERNATION) && defined(CONFIG_USING_LAST_FRAMEBUFFER)
        fb_quickboot_lastframe_display_release();
    #endif

	tca92xxfb_clock_delete();    
}
EXPORT_SYMBOL(tca_fb_exit);


