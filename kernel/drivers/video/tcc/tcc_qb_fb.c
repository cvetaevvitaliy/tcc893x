/*********************************************************************************
 * 
 * (C) Copyright All Rights Reserved by Telechips Inc.
 *
 * *  This material is confidential and shall remain as such.
 * *  Any unauthorized use, distribution, reproduction is strictly prohibited.
 * *
 * *******************************************************************************
 * *
 * *  File Name   : tcc_qb_fb.c
 * *
 * *  Description : fb display for Quickboot system
 * *
 * *******************************************************************************
 * *
 * *  yyyy/mm/dd     ver            descriptions                Author
 * *	---------	--------   ---------------       -----------------
 * *  2013/08/21     0.1            created                     KCMin
 * *******************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fb.h>
#include <linux/vt_kern.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/irq.h>
#include <linux/dma-mapping.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#include <mach/tca_lcdc.h>
#include <mach/tca_fb_hdmi.h>
#include <mach/tca_fb_output.h>

#include <linux/tcc_pwm.h>

#include <mach/timex.h>
#include <mach/globals.h>

#include <mach/vioc_outcfg.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_disp.h>
#include <mach/vioc_config.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include "tccfb.h"

static int debug = 0;
#define dprintk(msg...) if(debug) { printk("VIOC_I :" msg); }

static VIOC_DISP * pDISPBase;
static VIOC_WMIX * pWMIXBase;
static VIOC_WMIX * pWMIXBase_HDMI;
static VIOC_RDMA * pRDMABase;

#define fb_width(fb)	((fb)->var.xres)
#define fb_height(fb)	((fb)->var.yres)
#define fb_size(fb)	((fb)->var.xres * (fb)->var.yres * 2)

#define QUICKBOOT_IMG_BUFF_SIZE(fb) ((fb)->var.xres * (fb)->var.yres * 4)

dma_addr_t	QB_Lastframe_dma;	/* physical */
u_char *		QB_Lastframe_cpu;	/* virtual */

#if defined(CONFIG_USING_IMAGE_WHAT_YOU_WANT)
#define     QUICKBOOT_USER_LOGO     "QuickBoot_logo.rle"
#endif


#if defined(CONFIG_QUICK_BOOT_PROGRESS_BAR)

typedef struct
{
    int    sx;
    int    sy;
    int    width;
    int    height;
    int    addr;
}progress_bar_info_t;

progress_bar_info_t    bar;
unsigned int img_width ;
unsigned int img_height;
unsigned int QB_BAR_ADDR;
char * bar_addr_start;
#endif

extern TCC_OUTPUT_TYPE Output_SelectMode;

extern unsigned int tca_get_lcd_lcdc_num(void);
extern unsigned int tca_get_output_lcdc_num(void);

char FB_lcdc_num = 0;

int fb_quickboot_lastframe_display(void)
{

	struct fb_info *info;
	int err = 0;

	printk(KERN_WARNING "~~  %s:  \n",__func__);

	info = registered_fb[0];

	if (!info) {
		printk(KERN_WARNING "%s: Can not access framebuffer\n",
				__func__);
	}

	unsigned int size = QUICKBOOT_IMG_BUFF_SIZE(info);

	QB_Lastframe_cpu = dma_alloc_writecombine(0, size, &QB_Lastframe_dma, GFP_KERNEL); //virtual 

	if(!QB_Lastframe_cpu)
	{
		printk(KERN_WARNING, "%s Can not alloc back_up \n",__func__);
		err = -ENOMEM;
		goto free_memory;
	}

	dprintk(" %s back_up alloc addr = [0x%x] size = [%x] \n",__func__,QB_Lastframe_cpu, size);

	memset(QB_Lastframe_cpu, 0x0, size);
	memcpy(QB_Lastframe_cpu, info->screen_base + (info->var.xres * info->var.yoffset * (info->var.bits_per_pixel/8))  , size);

	return 0;


free_memory:
	dma_free_writecombine(0,size,QB_Lastframe_cpu, QB_Lastframe_dma);
	dprintk(" %s QB fb init failed.\n",__func__);


}
EXPORT_SYMBOL(fb_quickboot_lastframe_display);

void fb_quickboot_lastframe_display_release(void)
{
	struct fb_info *info;
	info = registered_fb[0];

	 dma_free_writecombine(0, info->var.xres * info->var.yres * 4, QB_Lastframe_cpu, QB_Lastframe_dma);
}
EXPORT_SYMBOL(fb_quickboot_lastframe_display_release);

static void memset32(void *_ptr, unsigned short val, unsigned count)
{

	char *ptr = _ptr;
	char b = val & 0x001f;
	char g = (val & 0x07e0)>>5;
	char r = (val & 0xf800)>>11;
	count >>= 1;
	while (count--) {
		*ptr++ = b<<3 | b>>2;
		*ptr++ = g<<2 | g>>4;
		*ptr++ = r<<3 | r>>2;
		*ptr++ = 0xff;
	}

}

#if defined(CONFIG_QUICK_BOOT_PROGRESS_BAR)
int fb_quickboot_progress_bar(int percent)
{
    char * bar_addr;
    char * bar_addr_width;
    char * bar_addr_height;

    static int mem_init = 0;

    int output_ret;
    if(!mem_init)
    {    
        struct fb_info *info;
	  struct tccfb_info *fbi;

        int err;
    
        printk(KERN_WARNING "~~  %s:  \n",__func__);
        
        info = registered_fb[0];

        if (!info) {
            printk(KERN_WARNING "%s: Can not access framebuffer\n",  __func__);
        }

        img_width = fb_width(info);
        img_height = fb_height(info);

        bar.sx = (img_width /4);
        bar.sy = (img_height / 4 ) * 3;
        bar.width = (img_width /2);
        bar.height = (img_height/10) - 1;    

        FB_lcdc_num = tca_get_lcd_lcdc_num();

        if(FB_lcdc_num)
                pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA04);
        else
                pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA00);

	 fbi = info->par;

        bar_addr_start = (char *)(info->screen_base) + (info->var.xres * info->var.yoffset * (info->var.bits_per_pixel/8));
        QB_BAR_ADDR = fbi->map_dma + (info->var.xres * info->var.yoffset * (info->var.bits_per_pixel/8));	

        mem_init = 1;
    }

        dprintk(" @@@ img_width = %d , img_height = %d percent = %d \n", img_width, img_height, percent);
        dprintk(" @@@ bar_add_start = 0x%x bar_add_start = 0x%x QB_BAR_ADDR = 0x%x ",bar_addr_start, *bar_addr_start, QB_BAR_ADDR);
        dprintk(" @@@ (%d, %d) ~ (%d, %d) \n", bar.sx, bar.sy, bar.width, bar.height);


        if( percent <= 100 )
        {
                bar_addr = (bar_addr_start + ((img_width * bar.sy * 4 ) + (bar.sx * 4)) );     
                bar_addr_width = (bar_addr + ((bar.width /100) * 4));
                bar_addr_height= (bar_addr + ( (img_width * bar.height  * 4 ) + ((bar.width / 100) * 4)));

                for(bar_addr; bar_addr < bar_addr_height; )
                     {
                        for(bar_addr; bar_addr < bar_addr_width ;)
                        {
                            // color setting
                            *bar_addr++ = 0x00;
                            *bar_addr++ = 0x00;
                            *bar_addr++ = 0xFF;
                            *bar_addr++ = 0xFF;
                        }
                        bar_addr += (img_width - (bar.width/100)) * 4;
                        bar_addr_width += (img_width)* 4;
                }
                VIOC_RDMA_SetImageBase(pRDMABase, QB_BAR_ADDR, 0 , 0 );
                VIOC_RDMA_SetImageEnable(pRDMABase);
                bar.sx += (bar.width / 100) ;
                
                dprintk(" @@@ (%d, %d) ~ (%d, %d) percent = %d\n", bar.sx, bar.sy, bar.width, bar.height, percent);

                if(Output_SelectMode != TCC_OUTPUT_NONE)		
                {		
        		//output_ret = TCC_OUTPUT_FB_Update_External(img_width, img_height, 4, QB_BAR_ADDR , Output_SelectMode);
        		output_ret = TCC_OUTPUT_FB_Update_External(img_width, img_height, 32, QB_BAR_ADDR , Output_SelectMode);

        		if(output_ret)
        			TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
	         }
        }
        
    return 0;
}
EXPORT_SYBBOL(fb_quickboot_progress_bar);
#endif

int fb_hibernation_logo(char *rle_filename)
{

	struct tccfb_info *fbi;
	struct fb_info *info;
	int fd, err = 0;
	unsigned count, max;

	unsigned short *data, *ptr;
	char *bits;

	char output_ret  = 0;
	unsigned int BaseAddr;

	mm_segment_t oldfs;	//setting for sys_open

	oldfs = get_fs();		//setting for sys_open
	set_fs(get_ds());		//setting for sys_open

	printk(KERN_WARNING "~~  %s:  %s\n",__func__, rle_filename);

	fd = sys_open(rle_filename, O_RDONLY, 0);
	if (fd < 0) {
		printk(KERN_WARNING "%s: Can not open %s\n",
				__func__, rle_filename);
		return -ENOENT;
	}
	count = (unsigned)sys_lseek(fd, (off_t)0, 2);
	if (count == 0) {
		sys_close(fd);
		err = -EIO;
		goto err_logo_close_file;
	}
	sys_lseek(fd, (off_t)0, 0);
	data = kmalloc(count, GFP_KERNEL);
	if (!data) {
		printk(KERN_WARNING "%s: Can not alloc data\n", __func__);
		err = -ENOMEM;
		goto err_logo_close_file;
	}

	if ((unsigned)sys_read(fd, (char *)data, count) != count) {
		printk(KERN_WARNING "%s: Can not read file \n",__func__);
		err = -EIO;
		goto err_logo_free_data;
	}

	info = registered_fb[0];

	if (!info) {
		printk(KERN_WARNING "%s: Can not access framebuffer\n",
				__func__);
		return -ENODEV;
	}

	fbi = info->par;

	max = fb_width(info) * fb_height(info);
	ptr = data;	

	bits = (char *)(info->screen_base) + (info->var.xres * info->var.yoffset * (info->var.bits_per_pixel/8));


	dprintk(KERN_WARNING "%s: registered_fb xres=[%d] yres=[%d] bpp=[%d]\n",__func__,info->var.xres,info->var.yres,info->var.bits_per_pixel );

	while (count > 3) {
		unsigned n = ptr[0];
		if (n > max)
			break;

		memset32( (char *)bits, ptr[1], n << 1);
		bits += info->var.bits_per_pixel/8 * n;
		max -= n;
		ptr += 2;
		count -= 4;
	}

	BaseAddr = fbi->map_dma + (info->var.xres * info->var.yoffset * (info->var.bits_per_pixel/8));	// physical mem address setting for hdmi

	if(Output_SelectMode != TCC_OUTPUT_NONE)		
	{		
		output_ret = TCC_OUTPUT_FB_Update_External(fb_width(info), fb_height(info), info->var.bits_per_pixel, BaseAddr , Output_SelectMode);

		if(output_ret)
			TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
	}

err_logo_free_data:
	kfree(data);

err_logo_close_file:
	sys_close(fd);

	set_fs(oldfs);

	dprintk(KERN_WARNING "~~  %s:  %s  err:%d end \n",__func__, rle_filename, err);

	return err;

}
EXPORT_SYMBOL(fb_hibernation_logo);


int fb_quickboot_display_fb_resume(struct platform_device *dev, unsigned int lcdc)
{
	struct fb_info	*fbinfo = platform_get_drvdata(dev);
	struct tccfb_info *fbi= fbinfo->par;
	unsigned int base_addr;
	unsigned int imgch;
	unsigned int ret = 0;

	imgch = fbi->fb->node;			//FB_driver

	if(lcdc)
	{
		pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA04);
		pWMIXBase = (VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX1);
	}
	else
	{
		pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA00);
		pWMIXBase = (VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX0);
	}	

#if defined(CONFIG_USING_LAST_FRAMEBUFFER)
	base_addr = (char *)QB_Lastframe_dma;

#elif defined(CONFIG_USING_IMAGE_WHAT_YOU_WANT)

	base_addr = fbi->map_dma + (fbinfo->var.xres * fbinfo->var.yoffset * (fbinfo->var.bits_per_pixel/8));

	ret = fb_hibernation_logo(QUICKBOOT_USER_LOGO);

	if(ret >= 0)	
#endif                
	{			
		VIOC_WMIX_SetPosition(pWMIXBase, imgch, 0, 0);
		VIOC_RDMA_SetImageFormat(pRDMABase, TCC_LCDC_IMG_FMT_RGB888);
		VIOC_RDMA_SetImageOffset(pRDMABase, TCC_LCDC_IMG_FMT_RGB888, fbinfo->var.xres); 	//offset
		VIOC_RDMA_SetImageSize(pRDMABase, fbinfo->var.xres, fbinfo->var.yres); 		//size
		VIOC_RDMA_SetImageBase(pRDMABase, base_addr, 0 , 0 );
		VIOC_RDMA_SetImageEnable(pRDMABase);
	}

}
EXPORT_SYMBOL(fb_quickboot_display_fb_resume);


