/*
 * linux/drivers/video/tcc/tccfb.c
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
#include <linux/err.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/gpio.h>
#include <linux/irq.h>

#ifdef CONFIG_DMA_SHARED_BUFFER
#include <linux/ion.h>

#include <linux/dma-buf.h>
#include <linux/memblock.h>
#include <linux/miscdevice.h>
#include <linux/export.h>
#include <linux/rbtree.h>
#include <linux/rtmutex.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#endif

#include <asm/system.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#endif
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>

#include <mach/bsp.h>

#include "tccfb.h"
#include <plat/pmap.h>
#include <mach/tcc_fb.h>
#include <mach/tcc_scaler_ctrl.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_composite_ioctl.h>
#include <mach/tcc_component_ioctl.h>
#include <mach/TCC_LCD_Interface.h>

#include <mach/tca_fb_hdmi.h>
#include <mach/tca_fb_output.h>
#include <mach/tca_lcdc.h>
#include <mach/globals.h>
#include <linux/console.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_disp.h>
#ifdef CONFIG_PM_RUNTIME
#include <linux/pm_runtime.h>
#endif
#include <mach/vioc_lut.h>

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
#include <mach/vioc_global.h>
#include <mach/vioc_config.h>
#endif

#include <mach/vioc_api.h>
#include <mach/tca_display_config.h>

#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
#include "tcc_vioc_viqe_interface.h"
#include "viqe.h"
#endif

#if defined(CONFIG_SYNC_FB)
#include <linux/sw_sync.h>
#include <linux/file.h>
#endif

#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
#include <mach/tcc_wmixer_ioctrl.h>

struct tcc_lcdc_image_update		Last_ImageInfo;
unsigned int LastFrame = 0;
EXPORT_SYMBOL(LastFrame);
pmap_t fb_lastframe_pbuf;

#if defined(CONFIG_ARCH_TCC892X)
pmap_t fb_lastframe_pbuf_ext;
#endif

extern int tccxxx_wmixer_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
extern int tccxxx_wmixer_release(struct inode * inode, struct file * filp);
extern int tccxxx_wmixer_open(struct inode * inode, struct file * filp);

unsigned int LastFrame_for_ResChanged = 0;
unsigned int LastFrame_for_CodecChanged = 0;
spinlock_t LastFrame_lockDisp;
#endif

#if  defined(CONFIG_HDMI_DISPLAY_LASTFRAME)

#if defined(CONFIG_ARCH_TCC893X) || defined(CONFIG_ARCH_TCC892X)
struct inode   lastframe_wm_alpha_inode;
struct file    lastframe_wm_alpha_flip;

extern int tccxxx_wmixer1_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
extern int tccxxx_wmixer1_release(struct inode * inode, struct file * filp);
extern int tccxxx_wmixer1_open(struct inode * inode, struct file * filp);
#endif

#if defined(CONFIG_ARCH_TCC892X)
struct inode lastframe_sc_inode;
struct file lastframe_sc_flip;

extern int tccxxx_scaler2_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
extern int tccxxx_scaler2_release(struct inode * inode, struct file * filp);
extern int tccxxx_scaler2_open(struct inode * inode, struct file * filp);

#endif

VIOC_RDMA * pLastFrame_RDMABase = NULL;

int onthefly_LastFrame = 0;
#endif
#if 1//defined(CONFIG_HDMI_DISPLAY_LASTFRAME)
int enabled_LastFrame = 0;
#endif// CONFIG_HDMI_DISPLAY_LASTFRAME


static unsigned int EX_OUT_LCDC;
static unsigned int LCD_LCDC_NUM;

#define FB_NUM_BUFFERS 3

#define VIQE_DUPLICATE_ROUTINE

#if defined(CONFIG_HIBERNATION) 
extern unsigned int do_hibernation;
extern unsigned int do_hibernate_boot;
extern int android_system_booting_finished;

#endif//CONFIG_HIBERNATION

extern char output_lcdc_onoff;

#if defined(CONFIG_MACH_TCC8930ST)
static int vioc_reset_cnt = 0;
#endif

/* Debugging stuff */
static int debug = 0;
#define dprintk(msg...)	if (debug) { printk( "tccfb: " msg); }

static int screen_debug = 0;
#define sprintk(msg...)	if (screen_debug) { printk( "tcc92fb scr: " msg); }
static int tccfb_set_par(struct fb_info *info);
extern int tca_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info);
extern void tca_fb_activate_var(struct tccfb_info *fbi,  struct fb_var_screeninfo *var);

#ifdef CONFIG_PM_RUNTIME
void tca_fb_runtime_suspend(void);
void tca_fb_runtime_resume(void);
#endif
extern int tca_fb_suspend(struct device *dev);
extern int tca_fb_resume(struct device *dev);
extern int tca_fb_init(void);
extern void tca_fb_exit(void);
extern int tcc_lcd_interrupt_reg(char SetClear, struct tccfb_info *info);
extern void tca_init_vsync(struct tccfb_info *dev);
extern void tca_vsync_enable(struct tccfb_info *dev, int on);
extern int tccfb_set_wmixer_layer_order(unsigned int num, unsigned int order, unsigned int fb_power_state);
extern int tca_fb_set_wmix_layer_order(unsigned int num, unsigned int order, unsigned int fb_power_state);

#if defined(CONFIG_FB_TCC_COMPOSITE)
extern int tcc_composite_detect(void);
extern void tcc_composite_update(struct tcc_lcdc_image_update *update);
extern void tcc_composite_set_bypass(char bypass_mode);
#endif

#if defined(CONFIG_FB_TCC_COMPONENT)
extern int tcc_component_detect(void);
extern void tcc_component_update(struct tcc_lcdc_image_update *update);
#endif

extern void TCC_OUTPUT_LCDC_Reset(void);

extern unsigned char hdmi_get_phy_status(void);
extern unsigned char hdmi_get_system_en(void);

#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)
extern void tca_lcdc_change_disp_port(unsigned int lcd_org_num, unsigned int lcd_dst_num);
extern unsigned int tca_fb_output_path_change(unsigned int onoff);
extern int tcc_lcd_interrupt_onoff(unsigned int lcdc, unsigned int onoff);
#endif

extern void TCC_OUTPUT_FB_setFBInfo(struct tccfb_info* info);

static pmap_t pmap_fb_video;
#define FB_VIDEO_MEM_BASE	pmap_fb_video.base
#define FB_VIDEO_MEM_SIZE	pmap_fb_video.size


#define CONFIG_FB_TCC_DEVS_MAX	3	// do not change!
#define CONFIG_FB_TCC_DEVS		1

#if (CONFIG_FB_TCC_DEVS > CONFIG_FB_TCC_DEVS_MAX)
	#undef CONFIG_FB_TCC_DEVS
	#define CONFIG_FB_TCC_DEVS	CONFIG_FB_TCC_DEVS_MAX
#endif


#define SCREEN_DEPTH_MAX	32	// 32 or 16
//								 : 32 - 32bpp(alpah+rgb888)
//								 : 16 - 16bpp(rgb565)


const unsigned int default_scn_depth[CONFIG_FB_TCC_DEVS_MAX] =
{
/* fb0, Layer0: bottom */  (16), // 32 or 16
/* fb1, Layer1: middle */  (16), //  "
/* fb2, Layer2: top    */  (16)  //  "
};


#define LCD_OUT_LCDC 	1
static struct lcd_panel *lcd_panel;


TCC_OUTPUT_TYPE	Output_SelectMode =  TCC_OUTPUT_NONE;
static unsigned int Output_BaseAddr;


static char  HDMI_pause = 0;
static char HDMI_video_mode = 0;

static unsigned int HDMI_video_width = 0;
static unsigned int HDMI_video_height = 0;
static unsigned int HDMI_video_hz = 0;


char fb_power_state;

// checking TCC8925S port change state !!!
static unsigned int Port_change_state = 0;


#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)
spinlock_t port_change_lock ;
#endif

// for frame buffer clear
static u_char *fb_mem_vaddr[CONFIG_FB_TCC_DEVS]= {0,};
static u_int   fb_mem_size [CONFIG_FB_TCC_DEVS]= {0,};

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
// this feature is for that lcd display video by using vsync
#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
typedef enum{
	LCD_START_VSYNC,
	HDMI_START_VSYNC
};
static int who_start_vsync = LCD_START_VSYNC;
#endif

static int debug_v = 0;
#define vprintk(msg...)	if (debug_v) { printk( "tccfb_vsync: " msg); }

#if 0
static int testToggleBit1=0;
static int testToggleBit=0;
#endif

inline static int tcc_vsync_get_time(void);
inline static void tcc_vsync_set_time(int time);
#define USE_VSYNC_TIMER
//#define USE_SOFT_IRQ_FOR_VSYNC

#define TIME_BUFFER_COUNT			30
// if time gab of video frame over base time is bigger than this value, that frame is skipped.
#define VIDEO_SYNC_MARGIN_LATE		50
// if time gap of video frame over base time is less than this value, that frame is displayed immediately
#define VIDEO_SYNC_MARGIN_EARLY		50

#define TIME_MARK_SKIP				(-1)

typedef struct{
	int readIdx ;
	int writeIdx ;
	int clearIdx;

	int valid_buff_count;
	int readable_buff_count;

	int max_buff_num ;
	int last_cleared_buff_id;
	int available_buffer_id_on_vpu;
	int cur_lcdc_buff_addr;
	struct tcc_lcdc_image_update stImage[6] ;
}tcc_vsync_buffer_t;

typedef struct {
	tcc_vsync_buffer_t vsync_buffer;

	int isVsyncRunning;
	// for time sync
	unsigned int unVsyncCnt ;
	int baseTime;
	unsigned int timeGapIdx ;
	unsigned int timeGapBufferFullFlag;
	int timeGap[TIME_BUFFER_COUNT];
	int timeGapTotal;
	int updateGapTime;
	int vsync_interval;
	int perfect_vsync_flag;

	int skipFrameStatus;
	int overlayUsedFlag;
	int outputMode;
	int video_frame_rate;

	//for deinterlace mode
	int deinterlace_mode;
	int firstFrameFlag;
	int frameInfo_interlace;
	int m2m_mode;
	int output_toMemory;
	int nDeinterProcCount;
	int nTimeGapToNextField;
	int interlace_output;
	int interlace_bypass_lcdc;
	int mvcMode;
}tcc_video_disp ;

typedef struct _tcc_viqe_param_t
{
	int mode;
	int region;
	int strength1;
	int strength2;
	int modeparam;
} tcc_viqe_param_t;

tcc_viqe_param_t viqe_param;

spinlock_t vsync_lock ;
spinlock_t vsync_lockDisp ;

tcc_video_disp tccvid_vsync ;
static int vsync_started = 0;
static int vsync_output_mode = 0;

static int lcd_video_started = 0;

DECLARE_WAIT_QUEUE_HEAD( wq_consume ) ;
DECLARE_WAIT_QUEUE_HEAD( wq_consume1 ) ;

#ifdef CONFIG_EXTEND_DISPLAY_DELAY
#define EXTEND_DISPLAY_DELAY_T		3000

typedef struct {
	TCC_OUTPUT_TYPE	Ex_SelectMode;
	unsigned int 			UI_updated;
	unsigned int 			base_addr;
	external_fbioput_vscreeninfo sc_info;
	//for video 
	unsigned int 			Video_updated;	
	struct tcc_lcdc_image_update  VideoImg;
}extenddisplay_delay_DisplayInfo;
static extenddisplay_delay_DisplayInfo last_sc_info;

typedef struct {
	extenddisplay_delay_DisplayInfo *displaydata;
	spinlock_t delay_spinlock;
	atomic_t update_allow;
	struct delayed_work delay_work;
}extenddisplay_delay_StructInfo;

static extenddisplay_delay_StructInfo ExtDispStruct;
#endif // CONFIG_EXTEND_DISPLAY_DELAY

#ifdef USE_SOFT_IRQ_FOR_VSYNC
static struct work_struct vsync_work_q;
#endif

struct tcc_lcdc_image_update CurrDisplayingImage;
int tcc_video_frame_move( struct file *wmix_file, struct tcc_lcdc_image_update* inFframeInfo, WMIXER_INFO_TYPE* WmixerInfo, unsigned int target_addr, unsigned int target_format);


#if defined(CONFIG_SYNC_FB)
extern void tca_fb_vsync_activate(struct fb_var_screeninfo *var, struct tccfb_info *fbi);

static void tcc_fd_fence_wait(struct sync_fence *fence)
{
	int err = sync_fence_wait(fence, 1000);
	if (err >= 0)
		return;

	if (err == -ETIME)
		err = sync_fence_wait(fence, 10 * MSEC_PER_SEC);

}
extern void tca_check_reg(void);


static void tcc_fb_update_regs(struct tccfb_info *tccfb, struct tcc_fenc_reg_data *regs)
{
	pm_runtime_get_sync(tccfb->fb->dev);

	if ((regs->fence_fd > 0) && (regs->fence))	{
		tcc_fd_fence_wait(regs->fence);
		sync_fence_put(regs->fence);
	}

	#if !defined(CONFIG_MACH_TCC8920ST) && !defined(CONFIG_MACH_TCC8930ST) 
		tca_fb_activate_var(tccfb, &regs->var);
	#endif

	tca_fb_vsync_activate(NULL, NULL);
	tccfb->fb->fbops->fb_pan_display(&regs->var, tccfb->fb);

	pm_runtime_put_sync(tccfb->fb->dev);

	sw_sync_timeline_inc(tccfb->fb_timeline, 1);
}

static void fence_handler(struct kthread_work *work)
{
	struct tcc_fenc_reg_data *data, *next;
	struct list_head saved_list;

	struct tccfb_info *tccfb =
			container_of(work, struct tccfb_info, fb_update_regs_work);

	mutex_lock(&tccfb->fb_timeline_lock);
	saved_list = tccfb->fb_update_regs_list;
	list_replace_init(&tccfb->fb_update_regs_list, &saved_list);

	mutex_unlock(&tccfb->fb_timeline_lock);

	list_for_each_entry_safe(data, next, &saved_list, list) {
		tcc_fb_update_regs(tccfb, data);
		list_del(&data->list);
		kfree(data);
	}
}

static void ext_fence_handler(struct kthread_work *work)
{
	int save_timeline_fb_max = 0, save_timeline_value = 0;

	struct tccfb_info *tccfb =
			container_of(work, struct tccfb_info, ext_update_regs_work);

	mutex_lock(&tccfb->ext_timeline_lock);

	save_timeline_fb_max = tccfb->ext_timeline_max;
	save_timeline_value = tccfb->ext_timeline->value;

	mutex_unlock(&tccfb->ext_timeline_lock);
	
	TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);

	for(save_timeline_fb_max; save_timeline_fb_max > save_timeline_value; save_timeline_value ++) 	{
		sw_sync_timeline_inc(tccfb->ext_timeline, 1);
	}
}
#endif

inline static int tcc_vsync_set_max_buffer(tcc_vsync_buffer_t * buffer_t, int buffer_count)
{
	buffer_t->max_buff_num = buffer_count;
	return buffer_count;
}

inline static int tcc_vsync_push_buffer(tcc_vsync_buffer_t * buffer_t, struct tcc_lcdc_image_update* inputData)
{

	if(buffer_t->valid_buff_count >= buffer_t->max_buff_num || buffer_t->readable_buff_count >= buffer_t->max_buff_num)
	{
		printk("error: buffer full %d, max %d %d ts %d \n", buffer_t->valid_buff_count, buffer_t->max_buff_num,buffer_t->readable_buff_count,inputData->time_stamp);
		return -1;
	}

	memcpy(&(buffer_t->stImage[buffer_t->writeIdx]),(void*)inputData, sizeof(struct tcc_lcdc_image_update) );
	
	//printk("tcc_vsync_push_buffer writeIdx(%d) valid_buff_count %d buffer_unique_id %d addr0 %x \n", 
	//	buffer_t->writeIdx,buffer_t->valid_buff_count, buffer_t->stImage[buffer_t->writeIdx].buffer_unique_id,buffer_t->stImage[buffer_t->writeIdx].addr0);
		
	if(++buffer_t->writeIdx >= buffer_t->max_buff_num)
		buffer_t->writeIdx = 0;
	

	buffer_t->valid_buff_count++;
	buffer_t->readable_buff_count++;
	
	return 0;
}

inline static int tcc_vsync_pop_buffer(tcc_vsync_buffer_t * buffer_t)
{
	if(buffer_t->readable_buff_count == 0)
	{
		printk("error: buffer empty \n");
		return -1;
	}

	if(++buffer_t->readIdx >= buffer_t->max_buff_num)
		buffer_t->readIdx = 0;

	buffer_t->readable_buff_count--;

	//printk("tcc_vsync_pop_buffer readIdxbuffer_t(%d) writeIdx(%d) valid_ valid_count %d readble %d\n", 
	//	buffer_t->readIdx,buffer_t->writeIdx,buffer_t->valid_buff_count,buffer_t->readable_buff_count);

	return buffer_t->readable_buff_count;
}

inline static void* tcc_vsync_get_buffer(tcc_vsync_buffer_t * buffer_t, int offset)
{
	int readIdx;

	if((buffer_t->readable_buff_count-offset) > 0)
	{
		readIdx = buffer_t->readIdx;

		while(offset)
		{
			if(++readIdx >= buffer_t->max_buff_num)
				readIdx = 0;

			offset--;
		}

		return (void*)&(buffer_t->stImage[readIdx]);
	}
	else
	{
		return 0;
	}
}

inline static int tcc_vsync_clean_buffer(tcc_vsync_buffer_t * buffer_t)
{
	if(buffer_t->readIdx == buffer_t->clearIdx || buffer_t->valid_buff_count == 0)
	{
		vprintk("error: no clean buffer clearIdx(%d) valid_buff_count(%d) \n",  buffer_t->clearIdx,buffer_t->valid_buff_count );
		return -1;
	}

	vprintk("tcc_vsync_clean_buffer start clearIdx(%d) readIdx(%d) writeIdx(%d) valid_buff_count %d  \n", buffer_t->clearIdx,buffer_t->readIdx,buffer_t->writeIdx,buffer_t->valid_buff_count);
	do
	{
		if(++buffer_t->clearIdx >= buffer_t->max_buff_num)
			buffer_t->clearIdx = 0;
		
		if(buffer_t->last_cleared_buff_id < buffer_t->stImage[buffer_t->clearIdx].buffer_unique_id)
			buffer_t->last_cleared_buff_id = buffer_t->stImage[buffer_t->clearIdx].buffer_unique_id;

		buffer_t->valid_buff_count--;

	}while(buffer_t->readIdx != buffer_t->clearIdx);
	wake_up_interruptible( &wq_consume ) ;
	vprintk("tcc_vsync_clean_buffer valid_buff_count %d  \n", buffer_t->valid_buff_count);
	//printk("tcc_vsync_clean_buffer clearIdx(%d) readIdx(%d) writeIdx(%d) buff_id %d valid_count %d readble %d \n",
	//	buffer_t->clearIdx,buffer_t->readIdx,buffer_t->writeIdx,buffer_t->last_cleared_buff_id,buffer_t->valid_buff_count,buffer_t->readable_buff_count);
	return buffer_t->valid_buff_count;
}


inline static int tcc_vsync_pop_all_buffer(tcc_vsync_buffer_t * buffer_t)
{

	if(buffer_t->valid_buff_count == 0)
	{
		vprintk("error: buffer empty \n");
		return -1;
	}

	while(buffer_t->valid_buff_count)
	{
		if(buffer_t->last_cleared_buff_id < buffer_t->stImage[buffer_t->clearIdx].buffer_unique_id)
			buffer_t->last_cleared_buff_id = buffer_t->stImage[buffer_t->clearIdx].buffer_unique_id;

		if(++buffer_t->clearIdx >= buffer_t->max_buff_num)
			buffer_t->clearIdx = 0;

		buffer_t->valid_buff_count--;
	}

	buffer_t->readIdx = buffer_t->clearIdx;
	buffer_t->readable_buff_count = 0;
	//printk("tcc_vsync_pop_all_buffer valid_buff_count %d last_cleared_buff_id %d  \n", buffer_t->valid_buff_count,buffer_t->last_cleared_buff_id);
	return 0;
}


inline static int tcc_vsync_is_full_buffer(tcc_vsync_buffer_t * buffer_t)
{
	if(buffer_t->valid_buff_count >= buffer_t->max_buff_num)
		return 1;
	else
		return 0;
}

inline static int tcc_vsync_is_empty_buffer(tcc_vsync_buffer_t * buffer_t)
{
	if(buffer_t->valid_buff_count > 0)
		return 0;
	else
		return 1;
}

static unsigned int tcc_check_lcdc_enable(int lcdc_num)
{
	VIOC_DISP * pDISPBase;
		
	if(lcdc_num)
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
	else
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
	
	return pDISPBase->uCTRL.bREG.LEN;
}

static void tcc_check_interlace_output(int output_mode)
{
	VIOC_DISP * pDISPBase;
	char output_lcdc = EX_OUT_LCDC;

	if(output_lcdc)
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
	else
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
	
	if(pDISPBase->uCTRL.nREG & HwDISP_NI)
		tccvid_vsync.interlace_output = 0;
	else
		tccvid_vsync.interlace_output = 1;
}

static void DisplayUpdate(void)
{
	int current_time;
	int time_gap;
	static int time_gap_sign = 1;
	int valid_buff_count;
	int i;
	
	
	struct tcc_lcdc_image_update *pNextImage;

	if(tccvid_vsync.vsync_buffer.readable_buff_count < 2)
	{
		vprintk("There is no output data\n");
		//return;
	}

	current_time = tcc_vsync_get_time();
	tcc_vsync_clean_buffer(&tccvid_vsync.vsync_buffer);

	valid_buff_count = tccvid_vsync.vsync_buffer.readable_buff_count;
	valid_buff_count--;
	//printk("DisplayUpdate valid_buff_count : %d readIdx %d \n", valid_buff_count,tccvid_vsync.vsync_buffer.readIdx) ;
	//vprintk("DisplayUpdate valid_buff_count : %d\n", valid_buff_count) ;
	
	for(i=0; i < valid_buff_count; i++)
	{
		pNextImage = tcc_vsync_get_buffer(&tccvid_vsync.vsync_buffer, 0);
		vprintk("DisplayUpdate pNextImage->time_stamp : %d %d\n", pNextImage->time_stamp,current_time) ;
		//printk("DisplayUpdateI time_stamp(%d), current_time(%d) sync_time(%d)\n", pNextImage->time_stamp, current_time,pNextImage->sync_time) ;

		time_gap = (current_time+VIDEO_SYNC_MARGIN_EARLY) - pNextImage->time_stamp;
		if(time_gap >= 0)
		{
			tcc_vsync_pop_buffer(&tccvid_vsync.vsync_buffer);

			if(tccvid_vsync.perfect_vsync_flag == 1 && time_gap < 4 )
			{
				tcc_vsync_set_time(current_time + (tccvid_vsync.vsync_interval>>1)*time_gap_sign);
				if(time_gap_sign > 0)
					time_gap_sign = -1;
				else
					time_gap_sign = 1;
			}
		}
		else
		{
			break;
		}
	}

	pNextImage = &tccvid_vsync.vsync_buffer.stImage[tccvid_vsync.vsync_buffer.readIdx];
	
	//if(tccvid_vsync.outputMode == Output_SelectMode && tccvid_vsync.vsync_buffer.cur_lcdc_buff_addr != pNextImage->addr0  )
	if(tccvid_vsync.outputMode == Output_SelectMode )
	{
		//printk("mt(%d), remain_buff(%d) readIdx(%d) write(%d) readaddr(%x) cleared_buff_id(%d) buffer_unique_id %d\n", pNextImage->time_stamp, tccvid_vsync.vsync_buffer.readable_buff_count,
		//	tccvid_vsync.vsync_buffer.readIdx,tccvid_vsync.vsync_buffer.writeIdx,pNextImage->addr0,tccvid_vsync.vsync_buffer.last_cleared_buff_id,tccvid_vsync.vsync_buffer.stImage[tccvid_vsync.vsync_buffer.readIdx].buffer_unique_id) ;
		tccvid_vsync.vsync_buffer.cur_lcdc_buff_addr = pNextImage->addr0;
#ifdef CONFIG_VIDEO_DISPLAY_SWAP_VPU_FRAME
		memcpy(&CurrDisplayingImage, pNextImage, sizeof(struct tcc_lcdc_image_update));
#endif

#if 0
	    if(testToggleBit)
	    {
			PGPION hwGPIOC;
	        testToggleBit = 0; 

			hwGPIOC = (volatile PGPION)tcc_p2v(HwGPIOC_BASE);
			//hwGPIOC->GPEN.nREG|= (unsigned int)(0x00004000);
			//hwGPIOC->GPDAT.nREG |= (unsigned int)(0x00004000);
			hwGPIOC->GPFN1.bREG.GPFN14 = 0;
			hwGPIOC->GPEN.bREG.GP14 = 1;
			hwGPIOC->GPDAT.bREG.GP14 = 1;

			
	    }
	    else
	    {
			PGPION hwGPIOC;
	        testToggleBit = 1;

			hwGPIOC = (volatile PGPION)tcc_p2v(HwGPIOC_BASE);
			//hwGPIOC->GPEN.nREG |= (unsigned int)(0x00004000);
			//hwGPIOC->GPDAT.nREG &= (unsigned int)(~0x00004000);
			hwGPIOC->GPFN1.bREG.GPFN14 = 0;
			hwGPIOC->GPEN.bREG.GP14 = 1;
			hwGPIOC->GPDAT.bREG.GP14 = 0;

	    }
#endif

		switch(Output_SelectMode)
		{
			case TCC_OUTPUT_NONE:
				//TCC_HDMI_DISPLAY_UPDATE(LCD_LCDC_NUM, pNextImage);
				break;
			
			#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
			case TCC_OUTPUT_LCD:
				TCC_LCD_DISPLAY_UPDATE(LCD_LCDC_NUM, pNextImage);
				break;
			#endif
			
			case TCC_OUTPUT_HDMI:
				#ifdef CONFIG_EXTEND_DISPLAY_DELAY //for video 
				if(!atomic_read(&ExtDispStruct.update_allow))
				{
					//last_sc_info.VideoImg = *pNextImage;
					memcpy(&(last_sc_info.VideoImg), pNextImage, sizeof(struct tcc_lcdc_image_update));
					last_sc_info.Video_updated = false;
				}
				else
				#endif//CONFIG_EXTEND_DISPLAY_DELAY
				{
					#ifdef MVC_PROCESS
						if(TCC_OUTPUT_FB_GetMVCStatus() && pNextImage->MVCframeView == 1 && pNextImage->MVC_Base_addr0)
						{
							TCC_HDMI_DISPLAY_UPDATE_3D(EX_OUT_LCDC, pNextImage);
						}
						else
					#endif
						{
							TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, pNextImage);
						}
				}

				break;
			case TCC_OUTPUT_COMPONENT:
				#if defined(CONFIG_FB_TCC_COMPONENT)
					tcc_component_update(pNextImage);
				#endif
				break;
			case TCC_OUTPUT_COMPOSITE:
				#if defined(CONFIG_FB_TCC_COMPOSITE)
					tcc_composite_update(pNextImage);
				#endif
				break;
				
			default:
				break;
		}
	}
	return;
}

#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
static int SavedOddfirst = -1;

#if defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
extern unsigned char gDispBFSignal;
#endif
static int byPassImageToLCDC(struct tcc_lcdc_image_update *pImage, int ref_cnt, int lcdc_num)
{
	VIOC_DISP * pDISPBase;
	unsigned int lstatus = 0;
	int ret = 0;
	
	if(lcdc_num)
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
	else
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
	
	lstatus = pDISPBase->uLSTATUS.nREG;
	
	SavedOddfirst = pImage->odd_first_flag;
		
	#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
		/* check output path flag */
		if(pImage->output_path)
		{
			if(output_path_addr != pImage->addr0)
			{
				/* bottom field first */
				if(SavedOddfirst == 1)
				{
					/* current even field : top field displaying now */
					#if 0//defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
					if(gDispBFSignal == 0)	// 0 : TOP field is out. 1: BOTTOM field is out.
					#else
					if(ISSET(lstatus, HwLSTATUS_EF))
					#endif
					{
						output_path_addr = pImage->addr0;
						//printk("bf, 0x%08x update\n", output_path_addr);
					}
					/* current odd field : bottom field displaying now */
					else
					{
						return ret;
					}
				}
				/* top field first */
				else
				{
					/* current even field : top field displaying now */
					#if 0//defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
					if(gDispBFSignal == 0)	// 0 : TOP field is out. 1: BOTTOM field is out.
					#else
					if(ISSET(lstatus, HwLSTATUS_EF))
					#endif
					{
						return ret;
					}
					/* current odd field : bottom field displaying now */
					else
					{
						output_path_addr = pImage->addr0;
						//printk("ef, 0x%08x update\n", output_path_addr);
					}
				}
			}
			else
			{
				//printk("@@@-%c-%c-0x%08x-%d\n", SavedOddfirst? 'o':'e', ISSET(lstatus, HwLSTATUS_EF)? 'e':'o', pImage->addr0, pImage->output_path);
				return ret;
			}
		}
		else
		{
			/* clear ouput path address */
			output_path_addr = 0;
		}
	#endif

	if(SavedOddfirst==0){
		if(tccvid_vsync.nDeinterProcCount ==0){
			#if 0//defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
			if(gDispBFSignal == 0) 	// 0 : TOP field is out. 1: BOTTOM field is out.
			#else
			if(ISSET(lstatus, HwLSTATUS_EF))
			#endif
			{
				ret = 1;
			}
		}
		else{
			#if 0//defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
			if(gDispBFSignal == 1)	// 0 : TOP field is out. 1: BOTTOM field is out.
			#else
			if(!ISSET(lstatus, HwLSTATUS_EF))
			#endif
			{
				ret = 1;
			}
		}
	}
	else{

		if(tccvid_vsync.nDeinterProcCount ==1){
			#if 0//defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
			if(gDispBFSignal == 0)	// 0 : TOP field is out. 1: BOTTOM field is out.
			#else
			if(ISSET(lstatus, HwLSTATUS_EF))
			#endif
			{
				ret = 1;
			}
		}
		else{
			#if 0//defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
			if(gDispBFSignal == 1)	// 0 : TOP field is out. 1: BOTTOM field is out.
			#else
			if(!ISSET(lstatus, HwLSTATUS_EF))
			#endif
			{
				ret = 1;
			}
		}
	}
	
	if(ret == 1){
		//printk("nDeinterProcCount(%d), SavedOddfirst(%d)\n", tccvid_vsync.nDeinterProcCount, SavedOddfirst) ;
		return ret;
	}
	
	switch(Output_SelectMode)
	{
		case TCC_OUTPUT_NONE:
			//TCC_HDMI_DISPLAY_UPDATE(LCD_LCDC_NUM, pImage);
			break;
#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
		case TCC_OUTPUT_LCD:
			TCC_LCD_DISPLAY_UPDATE(LCD_LCDC_NUM, pImage);
			break;
#endif
		case TCC_OUTPUT_HDMI:
			TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, pImage);
			break;
		case TCC_OUTPUT_COMPONENT:
			#if defined(CONFIG_FB_TCC_COMPONENT)
				tcc_component_update(pImage);
			#endif
			break;
		case TCC_OUTPUT_COMPOSITE:
			#if defined(CONFIG_FB_TCC_COMPOSITE)
				tcc_composite_update(pImage);
			#endif
			break;
			
		default:
			break;
	}

	return ret;

}

static void DisplayUpdateWithDeinterlace(void)
{
	int current_time;
	int time_gap;
	static int time_gap_sign = 1;
	int valid_buff_count;
	int i;
	#ifndef VIQE_DUPLICATE_ROUTINE	
	int time_diff;
	#endif
	static struct tcc_lcdc_image_update *pNextImage;

	if(tccvid_vsync.vsync_buffer.readable_buff_count < 3)
	{
		vprintk("There is no output data.\n");
		//return;
	}

	current_time = tcc_vsync_get_time();

	#ifndef VIQE_DUPLICATE_ROUTINE
	if(tccvid_vsync.nDeinterProcCount == 0)
	{
		tcc_vsync_clean_buffer(&tccvid_vsync.vsync_buffer);

		valid_buff_count = tccvid_vsync.vsync_buffer.readable_buff_count;
		valid_buff_count--;
		for(i=0; i < valid_buff_count; i++)
		{
			pNextImage = tcc_vsync_get_buffer(&tccvid_vsync.vsync_buffer, 0);

			time_gap = (current_time+VIDEO_SYNC_MARGIN_EARLY) - pNextImage->time_stamp;
			if(time_gap >= 0)
			{
				tcc_vsync_pop_buffer(&tccvid_vsync.vsync_buffer);

				if(tccvid_vsync.perfect_vsync_flag == 1 && time_gap < 4 )
				{
					tcc_vsync_set_time(current_time + (tccvid_vsync.vsync_interval>>1)*time_gap_sign);
					if(time_gap_sign > 0)
						time_gap_sign = -1;
					else
						time_gap_sign = 1;
				}
			}
			else
			{
				break;
			}
		}


		pNextImage = &tccvid_vsync.vsync_buffer.stImage[tccvid_vsync.vsync_buffer.readIdx];
//		printk("mt(%d), st(%d)\n", pNextImage->time_stamp, current_time) ;
		time_diff = abs (( pNextImage->time_stamp - (current_time+VIDEO_SYNC_MARGIN_EARLY) ));
#ifdef CONFIG_VIDEO_DISPLAY_SWAP_VPU_FRAME
		memcpy(&CurrDisplayingImage, pNextImage, sizeof(struct tcc_lcdc_image_update));
#endif
		
		//if(tccvid_vsync.interlace_bypass_lcdc ||(tccvid_vsync.vsync_buffer.cur_lcdc_buff_addr != pNextImage->addr0))// &&  time_diff < 100) )
		{
			tccvid_vsync.vsync_buffer.cur_lcdc_buff_addr = pNextImage->addr0;

			if(tccvid_vsync.outputMode == Output_SelectMode)
			{
				if(!tccvid_vsync.interlace_bypass_lcdc)
				{
					switch(Output_SelectMode)
					{
						case TCC_OUTPUT_NONE:
							break;
						#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
						case TCC_OUTPUT_LCD:
							TCC_VIQE_DI_Run60Hz(pNextImage->on_the_fly, pNextImage->addr0, pNextImage->addr1, pNextImage->addr2,
												pNextImage->Frame_width, pNextImage->Frame_height,
												pNextImage->crop_top,pNextImage->crop_bottom, pNextImage->crop_left, pNextImage->crop_right, 
												pNextImage->Image_width, pNextImage->Image_height, 
												pNextImage->offset_x, pNextImage->offset_y, pNextImage->odd_first_flag, pNextImage->frameInfo_interlace, 0);
							break;
						#endif
						case TCC_OUTPUT_HDMI:
						case TCC_OUTPUT_COMPONENT:
						case TCC_OUTPUT_COMPOSITE:
						#ifdef CONFIG_EXTEND_DISPLAY_DELAY //for video 
							if(!atomic_read(&ExtDispStruct.update_allow))
							{
								memcpy(&(last_sc_info.VideoImg), pNextImage, sizeof(struct tcc_lcdc_image_update));
								last_sc_info.Video_updated = false;
							}
							else
						#endif
							TCC_VIQE_DI_Run60Hz(pNextImage->on_the_fly, pNextImage->addr0, pNextImage->addr1, pNextImage->addr2,
												pNextImage->Frame_width, pNextImage->Frame_height,
												pNextImage->crop_top,pNextImage->crop_bottom, pNextImage->crop_left, pNextImage->crop_right, 
												pNextImage->Image_width, pNextImage->Image_height, 
												pNextImage->offset_x, pNextImage->offset_y, pNextImage->odd_first_flag, pNextImage->frameInfo_interlace, 0);
							break;
							
						default:
							break;
					}
				}
				else
				{
					
					if(byPassImageToLCDC(pNextImage, 0, EX_OUT_LCDC) == 1){
						tccvid_vsync.nDeinterProcCount =0;
						return;
					}
				}
			}	
			tccvid_vsync.nDeinterProcCount ++;
		}
		
	}
	else
	{
		if(tccvid_vsync.outputMode == Output_SelectMode)
		{
			if(!tccvid_vsync.interlace_bypass_lcdc)
			{
				switch(Output_SelectMode)
				{
					case TCC_OUTPUT_NONE:
						break;
					#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
					case TCC_OUTPUT_LCD:
						TCC_VIQE_DI_Run60Hz(pNextImage->on_the_fly, pNextImage->addr0, pNextImage->addr1, pNextImage->addr2,
											pNextImage->Frame_width, pNextImage->Frame_height,
											pNextImage->crop_top,pNextImage->crop_bottom, pNextImage->crop_left, pNextImage->crop_right, 
											pNextImage->Image_width, pNextImage->Image_height, 
											pNextImage->offset_x, pNextImage->offset_y, pNextImage->odd_first_flag, pNextImage->frameInfo_interlace, 0);
						break;
					#endif
					case TCC_OUTPUT_HDMI:
					case TCC_OUTPUT_COMPONENT:
					case TCC_OUTPUT_COMPOSITE:
					#ifdef CONFIG_EXTEND_DISPLAY_DELAY //for video 
						if(!atomic_read(&ExtDispStruct.update_allow))
						{
							memcpy(&(last_sc_info.VideoImg), pNextImage, sizeof(struct tcc_lcdc_image_update));
							last_sc_info.Video_updated = false;
						}
						else
					#endif
						TCC_VIQE_DI_Run60Hz(pNextImage->on_the_fly, pNextImage->addr0, pNextImage->addr1, pNextImage->addr2,
											pNextImage->Frame_width, pNextImage->Frame_height,
											pNextImage->crop_top,pNextImage->crop_bottom, pNextImage->crop_left, pNextImage->crop_right, 
											pNextImage->Image_width, pNextImage->Image_height, 
											pNextImage->offset_x, pNextImage->offset_y, pNextImage->odd_first_flag, pNextImage->frameInfo_interlace, 0);
						break;
						
					default:
						break;
				}
			}
		}
		tccvid_vsync.nDeinterProcCount = 0;

	}
	#else
	
	if(tccvid_vsync.nDeinterProcCount == 0)
	{
		tcc_vsync_clean_buffer(&tccvid_vsync.vsync_buffer);

		valid_buff_count = tccvid_vsync.vsync_buffer.readable_buff_count;
		valid_buff_count--;
		for(i=0; i < valid_buff_count; i++)
		{
			pNextImage = tcc_vsync_get_buffer(&tccvid_vsync.vsync_buffer, 0);

			time_gap = (current_time+VIDEO_SYNC_MARGIN_EARLY) - pNextImage->time_stamp;
#ifdef CONFIG_VIDEO_DISPLAY_SWAP_VPU_FRAME
		memcpy(&CurrDisplayingImage, pNextImage, sizeof(struct tcc_lcdc_image_update));
#endif

			if(time_gap >= 0)
			{
				tcc_vsync_pop_buffer(&tccvid_vsync.vsync_buffer);

				if(tccvid_vsync.perfect_vsync_flag == 1 && time_gap < 4 )
				{
					tcc_vsync_set_time(current_time + (tccvid_vsync.vsync_interval>>1)*time_gap_sign);
					if(time_gap_sign > 0)
						time_gap_sign = -1;
					else
						time_gap_sign = 1;
				}
			}
			else
			{
				break;
			}
		}
		//printk("mt(%d), st(%d)\n", pNextImage->time_stamp, current_time) ;
		
		//printk("C(%d), pass(%d)\n", tccvid_vsync.nDeinterProcCount,tccvid_vsync.interlace_bypass_lcdc) ;

		if(tccvid_vsync.interlace_bypass_lcdc )
		{
			pNextImage = &tccvid_vsync.vsync_buffer.stImage[tccvid_vsync.vsync_buffer.readIdx];
			tccvid_vsync.vsync_buffer.cur_lcdc_buff_addr = pNextImage->addr0;

			if(tccvid_vsync.outputMode == Output_SelectMode)
			{
				if(byPassImageToLCDC(pNextImage, 0, EX_OUT_LCDC) == 1){
					tccvid_vsync.nDeinterProcCount =0;
					return;
				}
			}	
		}
		tccvid_vsync.nDeinterProcCount ++;
	}
	else{
		tccvid_vsync.nDeinterProcCount = 0;
	}
	
	if(!tccvid_vsync.interlace_bypass_lcdc )
	{
		switch(Output_SelectMode)
		{
			case TCC_OUTPUT_NONE:
				break;
			#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
			case TCC_OUTPUT_LCD:
				viqe_render_field(current_time);
				break;
			#endif
			case TCC_OUTPUT_HDMI:
			case TCC_OUTPUT_COMPONENT:
			case TCC_OUTPUT_COMPOSITE:
				viqe_render_field(current_time);
				break;
				
			default:
				break;
		}
			
	}
	
	#endif
	return;
}
#endif // TCC_VIDEO_DISPLAY_DEINTERLACE_MODE

void tca_video_vsync_interrupt_onoff(int onoff, int lcdc_num)
{
	VIOC_RDMA *pRDMABase;
	
	if(lcdc_num)
		pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA04);
	else
		pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA00);

	VIOC_RDMA_SetIreqMask(pRDMABase, VIOC_RDMA_IREQ_ALL_MASK, 1);

	if(onoff)
		VIOC_RDMA_SetIreqMask(pRDMABase, VIOC_RDMA_IREQ_IEOFF_MASK, 0);
	else
		VIOC_RDMA_SetIreqMask(pRDMABase, VIOC_RDMA_IREQ_IEOFF_MASK, 0);
}

void tca_video_vsync_interrupt_maskset(int lcdc_num)
{
	VIOC_RDMA *pRDMABase;
	
	if(lcdc_num)
		pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA04);
	else
		pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA00);

	VIOC_RDMA_SetStatus(pRDMABase, VIOC_RDMA_STAT_IEOFF);
}

static irqreturn_t tcc_lcd_handler0_for_video(int irq, void *dev_id)
{
	//tca_lcdc_interrupt_onoff(1, 0);
	//tca_video_vsync_interrupt_maskset(0);
	//tca_video_vsync_interrupt_onoff(1, 0);
#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
	if(LastFrame)
		return IRQ_HANDLED;
#endif

	#ifdef USE_SOFT_IRQ_FOR_VSYNC
		if (schedule_work(&vsync_work_q) == 0 ) {
			printk("vsync error:cannot schedule work !!!\n");
		}
#else

#ifndef USE_VSYNC_TIMER
		if((++tccvid_vsync.unVsyncCnt) &0x01)
			tccvid_vsync.baseTime += 16;
		else
			tccvid_vsync.baseTime += 17;

		if(!(tccvid_vsync.unVsyncCnt%6))
			tccvid_vsync.baseTime++;
#endif

#ifndef VIQE_DUPLICATE_ROUTINE
		if( tcc_vsync_is_empty_buffer(&tccvid_vsync.vsync_buffer) == 0 )
		{
			#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
				if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory)
					DisplayUpdateWithDeinterlace();
				else
			#endif
					DisplayUpdate();				

			#if defined(CONFIG_TCC_OUTPUT_ATTACH)
				TCC_OUTPUT_FB_AttachUpdateFlag(0);
			#endif
		}
#else
		#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
		if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory){
			if(tccvid_vsync.interlace_bypass_lcdc){
				if( tcc_vsync_is_empty_buffer(&tccvid_vsync.vsync_buffer) == 0 )
					DisplayUpdateWithDeinterlace();
			}
			else{
				if( tcc_vsync_is_empty_buffer(&tccvid_vsync.vsync_buffer) == 0 )
					DisplayUpdateWithDeinterlace();
			}
			
			#if defined(CONFIG_TCC_OUTPUT_ATTACH)
				TCC_OUTPUT_FB_AttachUpdateFlag(0);
			#endif
		}
		else
		#endif
		if( tcc_vsync_is_empty_buffer(&tccvid_vsync.vsync_buffer) == 0 )
		{
			DisplayUpdate();
			#if defined(CONFIG_TCC_OUTPUT_ATTACH)
				TCC_OUTPUT_FB_AttachUpdateFlag(0);
			#endif
		}
#endif
#endif	

	return IRQ_HANDLED;	
}

static irqreturn_t tcc_lcd_handler1_for_video(int irq, void *dev_id)
{
	//tca_lcdc_interrupt_onoff(1, 1);
	//tca_video_vsync_interrupt_maskset(1);
	//tca_video_vsync_interrupt_onoff(1, 1);

#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
	if(LastFrame)
		return IRQ_HANDLED;
#endif
	
#ifdef USE_SOFT_IRQ_FOR_VSYNC
		if (schedule_work(&vsync_work_q) == 0 ) {
            printk("vsync error:cannot schedule work !!!\n");
        }
#else
	
#ifndef USE_VSYNC_TIMER
	   if((++tccvid_vsync.unVsyncCnt) &0x01)
	           tccvid_vsync.baseTime += 16;
	   else
	           tccvid_vsync.baseTime += 17;

	   if(!(tccvid_vsync.unVsyncCnt%6))
	           tccvid_vsync.baseTime++;
#endif

	#ifndef VIQE_DUPLICATE_ROUTINE
		if( tcc_vsync_is_empty_buffer(&tccvid_vsync.vsync_buffer) == 0)
		{
			#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
				if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory)
					DisplayUpdateWithDeinterlace();
				else
			#endif
					DisplayUpdate();

			#if defined(CONFIG_TCC_OUTPUT_ATTACH)
				TCC_OUTPUT_FB_AttachUpdateFlag(1);
			#endif
		}
	#else
		#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
			if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory){
				if(tccvid_vsync.interlace_bypass_lcdc){
					if( tcc_vsync_is_empty_buffer(&tccvid_vsync.vsync_buffer) == 0 )
						DisplayUpdateWithDeinterlace();
				}
				else{
					DisplayUpdateWithDeinterlace();
				}
				#if defined(CONFIG_TCC_OUTPUT_ATTACH)
					TCC_OUTPUT_FB_AttachUpdateFlag(1);
				#endif
			}
			else
		#endif
			if( tcc_vsync_is_empty_buffer(&tccvid_vsync.vsync_buffer) == 0 )
			{
				DisplayUpdate();
				#if defined(CONFIG_TCC_OUTPUT_ATTACH)
					TCC_OUTPUT_FB_AttachUpdateFlag(1);
				#endif
			}
	#endif
#endif	

	return IRQ_HANDLED;	
}

#ifdef USE_VSYNC_TIMER
static unsigned int tcc_vsync_timer_count = 0;
static int tcc_vsync_timer_last_get_time = 0;

static irqreturn_t tcc_vsync_timer_handler(int irq, void *dev_id)
{
	PTIMER pTIMER_reg = (volatile PTIMER)tcc_p2v(HwTMR_BASE);

	#if defined(CONFIG_ARCH_TCC892X)
		if(system_rev == 0x1005 || system_rev == 0x1006 || system_rev == 0x1007 ||system_rev == 0x1008 || system_rev == 0x2002 || system_rev == 0x2003 || system_rev == 0x2004 || system_rev == 0x2005 || system_rev == 0x2006 || system_rev == 0x2007 || system_rev == 0x2008 || system_rev == 0x2009)
		{
				pTIMER_reg->TIREQ.nREG = 0x00000202;
		}
		else
		{
				pTIMER_reg->TIREQ.nREG = 0x00000101;
		}
	#elif defined(CONFIG_ARCH_TCC893X)
		if(machine_is_tcc893x() || machine_is_m805_893x())
			pTIMER_reg->TIREQ.nREG = 0x00000202;
		else
			pTIMER_reg->TIREQ.nREG = 0x00000101;
	#endif					
	
	vprintk("tcc_vsync_timer_handler \n");
	//tcc_vsync_timer_count +=55924;

	return IRQ_HANDLED;	
}

static int timer_interrupt_onoff = 0;
void tccfb_vsync_timer_onoff(int onOff)
{
	//PPIC pHwPIC_reg = (volatile PPIC)tcc_p2v(HwPIC_BASE);
	PTIMER pTIMER_reg = (volatile PTIMER)tcc_p2v(HwTMR_BASE);

	tcc_vsync_timer_count = 0;
	tcc_vsync_timer_last_get_time = 0;
	printk("tccfb_vsync_timer_onoff onOff %d\n",onOff);

	if(onOff == 1)	{
		PCKC pCKC = (volatile PCKC)tcc_p2v(HwCKC_BASE);
		BITCSET(pCKC->PCLKCTRL01.nREG, 0xFFFFFFFF, 0x24000000);

		#if defined(CONFIG_ARCH_TCC892X)
			if(system_rev == 0x1005 || system_rev == 0x1006 || system_rev == 0x1007 ||system_rev == 0x1008 || system_rev == 0x2002 || system_rev == 0x2003 || system_rev == 0x2004 || system_rev == 0x2005 || system_rev == 0x2006 || system_rev == 0x2007 || system_rev == 0x2008 || system_rev == 0x2009)
			{
				pTIMER_reg->TCFG1.bREG.EN = 1;
				pTIMER_reg->TCFG1.bREG.IEN = 1;
				pTIMER_reg->TCFG1.bREG.TCKSEL= 5;
				pTIMER_reg->TREF1.nREG= 0x0000FFFF;
				pTIMER_reg->TIREQ.bREG.TI1 = 1;
				pTIMER_reg->TIREQ.bREG.TF1 = 1;
				pTIMER_reg->TCNT1.bREG.TCNT = 0;
			}
			else
			{
				pTIMER_reg->TCFG0.bREG.EN = 1;
				pTIMER_reg->TCFG0.bREG.IEN = 1;
				pTIMER_reg->TCFG0.bREG.TCKSEL= 5;
				pTIMER_reg->TREF0.nREG= 0x0000FFFF;
				pTIMER_reg->TIREQ.bREG.TI0 = 1;
				pTIMER_reg->TIREQ.bREG.TF0 = 1;
				pTIMER_reg->TCNT0.bREG.TCNT = 0;
			}
		#elif defined(CONFIG_ARCH_TCC893X)
			if(machine_is_tcc893x() || machine_is_m805_893x()){
				pTIMER_reg->TCFG1.bREG.EN = 1;
				pTIMER_reg->TCFG1.bREG.IEN = 1;
				pTIMER_reg->TCFG1.bREG.TCKSEL= 5;
				pTIMER_reg->TREF1.nREG= 0x0000FFFF;
				pTIMER_reg->TIREQ.bREG.TI1 = 1;
				pTIMER_reg->TIREQ.bREG.TF1 = 1;
				pTIMER_reg->TCNT1.bREG.TCNT = 0;
			}
			else{
				pTIMER_reg->TCFG0.bREG.EN = 1;
				pTIMER_reg->TCFG0.bREG.IEN = 1;
				pTIMER_reg->TCFG0.bREG.TCKSEL= 5;
				pTIMER_reg->TREF0.nREG= 0x0000FFFF;
				pTIMER_reg->TIREQ.bREG.TI0 = 1;
				pTIMER_reg->TIREQ.bREG.TF0 = 1;
				pTIMER_reg->TCNT0.bREG.TCNT = 0;
			}
		#endif
		
		//pHwPIC_reg->SEL0.bREG.TC0= 1;
		//pHwPIC_reg->IEN0.bREG.TC0 = 1;
		//pHwPIC_reg->INTMSK0.bREG.TC0 = 1;
		//pHwPIC_reg->MODEA0.bREG.TC0 = 1;

		if(timer_interrupt_onoff == 0)
		{
			int ret = 0;

		#if defined(CONFIG_ARCH_TCC892X)
			if(system_rev == 0x1005 || system_rev == 0x1006 || system_rev == 0x1007 ||system_rev == 0x1008 || system_rev == 0x2002 || system_rev == 0x2003 || system_rev == 0x2004 || system_rev == 0x2005 || system_rev == 0x2006 || system_rev == 0x2007 || system_rev == 0x2008 || system_rev == 0x2009)
			{
				ret = request_irq(INT_TC_TI1, tcc_vsync_timer_handler, IRQF_SHARED, "TCC_TC1", tcc_vsync_timer_handler);
			}
			else
			{
				ret = request_irq(INT_TC_TI0, tcc_vsync_timer_handler, IRQF_SHARED,	"TCC_TC0", tcc_vsync_timer_handler);
			}
		#elif defined(CONFIG_ARCH_TCC893X)
			if(machine_is_tcc893x() || machine_is_m805_893x())
			{
				ret = request_irq(INT_TC_TI1, tcc_vsync_timer_handler, IRQF_SHARED, "TCC_TC1", tcc_vsync_timer_handler);
			}
			else
			{
				ret = request_irq(INT_TC_TI0, tcc_vsync_timer_handler, IRQF_SHARED, "TCC_TC0", tcc_vsync_timer_handler);
			}
		#endif
			timer_interrupt_onoff = 1;
		}
	
	}
	else	{

		#if defined(CONFIG_ARCH_TCC892X)
			if(system_rev == 0x1005 || system_rev == 0x1006 || system_rev == 0x1007 ||system_rev == 0x1008 || system_rev == 0x2002 || system_rev == 0x2003 || system_rev == 0x2004 || system_rev == 0x2005 || system_rev == 0x2006 || system_rev == 0x2007 || system_rev == 0x2008 || system_rev == 0x2009)
			{
				pTIMER_reg->TCFG1.bREG.EN = 0;
				pTIMER_reg->TCFG1.bREG.IEN = 0;
				pTIMER_reg->TCFG1.bREG.TCKSEL= 0;
				pTIMER_reg->TREF1.nREG= 0x0000FFFF;
				pTIMER_reg->TIREQ.bREG.TI1 = 0;
				pTIMER_reg->TIREQ.bREG.TF1 = 0;
				pTIMER_reg->TCNT1.bREG.TCNT = 0;
			}
			else
			{
				pTIMER_reg->TCFG0.bREG.EN = 0;
				pTIMER_reg->TCFG0.bREG.IEN = 0;
				pTIMER_reg->TCFG0.bREG.TCKSEL= 0;
				pTIMER_reg->TREF0.nREG= 0x0000FFFF;
				pTIMER_reg->TIREQ.bREG.TI0 = 0;
				pTIMER_reg->TIREQ.bREG.TF0 = 0;
				pTIMER_reg->TCNT0.bREG.TCNT = 0;

			}
		#elif defined(CONFIG_ARCH_TCC893X)
			if(machine_is_tcc893x() || machine_is_m805_893x())
			{
				pTIMER_reg->TCFG1.bREG.EN = 0;
				pTIMER_reg->TCFG1.bREG.IEN = 0;
				pTIMER_reg->TCFG1.bREG.TCKSEL= 0;
				pTIMER_reg->TREF1.nREG= 0x0000FFFF;
				pTIMER_reg->TIREQ.bREG.TI1 = 0;
				pTIMER_reg->TIREQ.bREG.TF1 = 0;
				pTIMER_reg->TCNT1.bREG.TCNT = 0;
			}
			else
			{
				pTIMER_reg->TCFG0.bREG.EN = 0;
				pTIMER_reg->TCFG0.bREG.IEN = 0;
				pTIMER_reg->TCFG0.bREG.TCKSEL= 0;
				pTIMER_reg->TREF0.nREG= 0x0000FFFF;
				pTIMER_reg->TIREQ.bREG.TI0 = 0;
				pTIMER_reg->TIREQ.bREG.TF0 = 0;
				pTIMER_reg->TCNT0.bREG.TCNT = 0;
			
			}
		#endif

		// interrupt disable
		//pHwPIC_reg->CLR0.bREG.TC0 = 1;
		//pHwPIC_reg->IEN0.bREG.TC0 = 0;
		//pHwPIC_reg->INTMSK0.bREG.TC0 = 0;

		if(timer_interrupt_onoff == 1)
		{
			#if defined(CONFIG_ARCH_TCC892X)
				if(system_rev == 0x1005 || system_rev == 0x1006 || system_rev == 0x1007 ||system_rev == 0x1008 || system_rev == 0x2002 || system_rev == 0x2003 || system_rev == 0x2004 || system_rev == 0x2005 || system_rev == 0x2006 || system_rev == 0x2007 || system_rev == 0x2008 || system_rev == 0x2009)
				{
						free_irq(INT_TC_TI1, tcc_vsync_timer_handler);			
				}
				else
				{
						free_irq(INT_TC_TI0, tcc_vsync_timer_handler);
				}
			#elif defined(CONFIG_ARCH_TCC893X)
				if(machine_is_tcc893x() || machine_is_m805_893x())
					free_irq(INT_TC_TI1, tcc_vsync_timer_handler);
				else
					free_irq(INT_TC_TI0, tcc_vsync_timer_handler);
			#endif					
			timer_interrupt_onoff = 0;
		}

	}
}

unsigned int tcc_vsync_get_timer_clock(void)
{
	int timer_tick;
	int msec_time;
	
	PTIMER pTIMER_reg = (volatile PTIMER)tcc_p2v(HwTMR_BASE);


	#if defined(CONFIG_ARCH_TCC892X)
		if(system_rev == 0x1005 || system_rev == 0x1006 || system_rev == 0x1007 ||system_rev == 0x1008 || system_rev == 0x2002 || system_rev == 0x2003 || system_rev == 0x2004 || system_rev == 0x2005 || system_rev == 0x2006 || system_rev == 0x2007 || system_rev == 0x2008 || system_rev == 0x2009)
		{
				timer_tick = pTIMER_reg->TCNT1.bREG.TCNT;
		}
		else
		{
				timer_tick = pTIMER_reg->TCNT0.bREG.TCNT;
		}

	msec_time = timer_tick*85/1000 + timer_tick/3000;
	#elif defined(CONFIG_ARCH_TCC893X)
		if(machine_is_tcc893x() || machine_is_m805_893x())
		{
			timer_tick = pTIMER_reg->TCNT1.bREG.TCNT;
		}
		else
		{
			timer_tick = pTIMER_reg->TCNT0.bREG.TCNT;
		}
		
		msec_time = timer_tick*85/2000 + timer_tick/6000;
	#endif					

	

	msec_time += (tcc_vsync_timer_count/10);
	
	//printk(" timer_tick(%d) msec_time(%d), tcc_vsync_timer_last_get_time(%d) %d\n",timer_tick,msec_time,tcc_vsync_timer_last_get_time,tcc_vsync_timer_count);
	if(msec_time < tcc_vsync_timer_last_get_time)
	{
		#if defined(CONFIG_ARCH_TCC892X)
		//printk("msec_time(%d), tcc_vsync_timer_last_get_time(%d)\n", msec_time, tcc_vsync_timer_last_get_time);
		tcc_vsync_timer_count +=55924;
		msec_time += 5592;
		#elif defined(CONFIG_ARCH_TCC893X)
		tcc_vsync_timer_count +=27962;
		msec_time += 2796;
		#endif
	}

	//if(tcc_vsync_timer_last_get_time >= msec_time)
	//	printk(" same time msec_time(%d), tcc_vsync_timer_last_get_time(%d)\n",tcc_vsync_timer_last_get_time,msec_time);
	
	tcc_vsync_timer_last_get_time = msec_time;
	return msec_time;
}
#endif

inline static void tcc_vsync_set_time(int time)
{
#ifdef USE_VSYNC_TIMER
	tccvid_vsync.baseTime = tcc_vsync_get_timer_clock() - time;
	//printk("set base time %d kernel time %d time %d \n",tccvid_vsync.baseTime,tcc_vsync_get_timer_clock(),time);
#else
	tccvid_vsync.baseTime = time;
#endif
}

inline static int tcc_vsync_get_time(void)
{
#ifdef USE_VSYNC_TIMER
	return tcc_vsync_get_timer_clock() - tccvid_vsync.baseTime;
#else
	return tccvid_vsync.baseTime;
#endif

}


static int lcdc_interrupt_onoff = 0;
void tca_vsync_video_display_enable(void)
{
	int ret;
	char hdmi_lcdc = EX_OUT_LCDC;

	#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
	if(who_start_vsync == LCD_START_VSYNC)
		hdmi_lcdc = LCD_LCDC_NUM;
	#endif
	
	if(lcdc_interrupt_onoff == 0)
	{
		if(hdmi_lcdc){
			/*
			tca_video_vsync_interrupt_onoff(1, 1);

			ret = request_irq(INT_VIOC_RD4, tcc_lcd_handler1_for_video,	IRQF_SHARED,
					"INT_VIOC_RD4",	tcc_lcd_handler1_for_video);
			*/
			#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
			if(who_start_vsync == LCD_START_VSYNC)
			{
				tca_lcdc_interrupt_onoff(0, 1);
				ret = request_irq(INT_VIOC_DEV1, tcc_lcd_handler1_for_video,IRQ_TYPE_EDGE_FALLING|IRQF_SHARED,"TCC_VIOC_DEV1", tcc_lcd_handler1_for_video);
				tca_lcdc_interrupt_onoff(1, 1);
				
				printk("tca_vsync_video_display_enable : request_irq 1 ret %d %d %d \n",ret,Output_SelectMode,who_start_vsync);
			}
			else
			#endif
			{
			tca_lcdc_interrupt_onoff(1, 1);
			ret = request_irq(INT_VIOC_DEV1, tcc_lcd_handler1_for_video,IRQF_SHARED,"TCC_VIOC_DEV1", tcc_lcd_handler1_for_video);
				
				printk("tca_vsync_video_display_enable : request_irq 1 ret %d %d  \n",ret,Output_SelectMode);
			}
		}
		else{
			/*
			tca_video_vsync_interrupt_onoff(1, 0);
				
			ret= request_irq(INT_VIOC_RD0, tcc_lcd_handler0_for_video,	IRQF_SHARED,
					"INT_VIOC_RD0",	tcc_lcd_handler0_for_video);
			*/
			#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
			if(who_start_vsync == LCD_START_VSYNC)
			{
				tca_lcdc_interrupt_onoff(0, 0);
				ret= request_irq(INT_VIOC_DEV0, tcc_lcd_handler0_for_video,IRQ_TYPE_EDGE_FALLING |IRQF_SHARED, "TCC_VIOC_DEV0", tcc_lcd_handler0_for_video);
				tca_lcdc_interrupt_onoff(1, 0);
				
				printk("tca_vsync_video_display_enable : request_irq 0 ret %d %d %d \n",ret,Output_SelectMode,who_start_vsync);
			}
			else
			#endif
			{
				#if defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
				// Don't Disable DISP interrupt for BF counting.
				#else
				//tca_lcdc_interrupt_onoff(1, 0);
				#endif
			#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)
			ret= request_irq(INT_VIOC_DEV0, tcc_lcd_handler0_for_video,IRQ_TYPE_EDGE_FALLING|IRQF_SHARED, "TCC_VIOC_DEV0", tcc_lcd_handler0_for_video);
			#else
			ret= request_irq(INT_VIOC_DEV0, tcc_lcd_handler0_for_video,IRQF_SHARED, "TCC_VIOC_DEV0", tcc_lcd_handler0_for_video);
			#endif			
			
				printk("tca_vsync_video_display_enable : request_irq 0 ret %d %d \n",ret,Output_SelectMode);
			}
		}

#ifdef USE_SOFT_IRQ_FOR_VSYNC
	    INIT_WORK(&vsync_work_q, tcc_video_display_update_isr);
#endif
		lcdc_interrupt_onoff = 1;
	}
	else
	{
		printk("tccfb error: lcdc interrupt have been enabled already\n");
	}
}
void tca_vsync_video_display_disable(void)
{
	char hdmi_lcdc = EX_OUT_LCDC;

	#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
	if(who_start_vsync == LCD_START_VSYNC)
		hdmi_lcdc = LCD_LCDC_NUM;

	printk("tca_vsync_video_display_disable %d %d \n",Output_SelectMode,who_start_vsync);
	#endif
	
	if(lcdc_interrupt_onoff == 1)
	{
		if(hdmi_lcdc){
			//tca_video_vsync_interrupt_onoff(0, 1);
			//free_irq(INT_VIOC_RD4, tcc_lcd_handler1_for_video);
			#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
			if(who_start_vsync == LCD_START_VSYNC)
			{
			tca_lcdc_interrupt_onoff(0, 1);
			free_irq(INT_VIOC_DEV1, tcc_lcd_handler1_for_video);
				tca_lcdc_interrupt_onoff(1, 1);
			}
			else
			#endif
			{
				#if defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
				// Don't Disable DISP interrupt for BF counting.
				#else
				//tca_lcdc_interrupt_onoff(0, 1);
				#endif
				free_irq(INT_VIOC_DEV1, tcc_lcd_handler1_for_video);
			}
		}
		else {
			//tca_video_vsync_interrupt_onoff(0, 0);
			//free_irq(INT_VIOC_RD0, tcc_lcd_handler0_for_video);
			#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
			if(who_start_vsync == LCD_START_VSYNC)
			{
			tca_lcdc_interrupt_onoff(0, 0);
			free_irq(INT_VIOC_DEV0, tcc_lcd_handler0_for_video);
				tca_lcdc_interrupt_onoff(1, 0);
			}
			else
			#endif
			{
			#if defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
			// Don't Disable DISP interrupt for BF counting.
			#else
			//tca_lcdc_interrupt_onoff(0, 0);
			#endif
			free_irq(INT_VIOC_DEV0, tcc_lcd_handler0_for_video);
		}
		}
		lcdc_interrupt_onoff = 0;
	}
	else
	{
		printk("tccfb error: lcdc interrupt have been disabled already\n");
	}
}

void tcc_vsync_set_firstFrameFlag(int firstFrameFlag)
{
	tccvid_vsync.firstFrameFlag = firstFrameFlag;
}

int tcc_vsync_get_isVsyncRunning(void)
{
	return tccvid_vsync.isVsyncRunning;
}

static int tccfb_calculateSyncTime(int currentTime)
{
	static int lastUdateTime = 0;
	int diffTime;
	int avgTime;
	int base_time;


	if(lastUdateTime == currentTime)
		return 0;

	lastUdateTime= currentTime;

	spin_lock_irq(&vsync_lock) ;
	base_time = tcc_vsync_get_time();
	spin_unlock_irq(&vsync_lock) ;
	
	diffTime = 	currentTime - base_time;

	tccvid_vsync.timeGapTotal -= tccvid_vsync.timeGap[tccvid_vsync.timeGapIdx];
	tccvid_vsync.timeGapTotal += diffTime;
	tccvid_vsync.timeGap[tccvid_vsync.timeGapIdx++] = diffTime;

	if(tccvid_vsync.timeGapIdx >= TIME_BUFFER_COUNT)
		tccvid_vsync.timeGapIdx = 0;

	if(tccvid_vsync.timeGapIdx == 0)
		tccvid_vsync.timeGapBufferFullFlag = 1;

	if(tccvid_vsync.timeGapBufferFullFlag)
		avgTime = tccvid_vsync.timeGapTotal / TIME_BUFFER_COUNT;
	else
		avgTime = tccvid_vsync.timeGapTotal / (int)(tccvid_vsync.timeGapIdx);


	vprintk("diffTime %d, avgTime %d, base : %d\n", diffTime, avgTime, base_time);

	if( (tccvid_vsync.timeGapBufferFullFlag || tccvid_vsync.unVsyncCnt < 100) 
		&& (avgTime > tccvid_vsync.updateGapTime || avgTime < -(tccvid_vsync.updateGapTime))	)
	{
		memset(tccvid_vsync.timeGap, 0x00, sizeof(tccvid_vsync.timeGap));
		tccvid_vsync.timeGapBufferFullFlag = 0;
		tccvid_vsync.timeGapIdx = 0;
		tccvid_vsync.timeGapTotal = 0;
		
		//printk("changed time base time %d kernel time %d time %d \n",tccvid_vsync.baseTime,tcc_vsync_get_timer_clock(),currentTime);
		spin_lock_irq(&vsync_lock) ;
		tcc_vsync_set_time(base_time+avgTime);
		spin_unlock_irq(&vsync_lock) ;

		printk("changed base time : %d, add time: %d diffTime %d \n",base_time+avgTime, avgTime,diffTime);
	}
	
	return 0;
}

static void tccfb_ResetSyncTime(int currentTime)
{
	memset(tccvid_vsync.timeGap, 0x00, sizeof(tccvid_vsync.timeGap));
	tccvid_vsync.timeGapBufferFullFlag = 0;
	tccvid_vsync.timeGapIdx = 0;
	tccvid_vsync.timeGapTotal = 0;
	
	spin_lock_irq(&vsync_lock) ;
	tcc_vsync_set_time(currentTime);
	tccvid_vsync.unVsyncCnt=0;
	spin_unlock_irq(&vsync_lock) ;

	printk("reset base time : %d\n",currentTime);
}

int tcc_video_get_displayed(void)
{

	if(tccvid_vsync.skipFrameStatus || !vsync_started)
	{
		if(tccvid_vsync.skipFrameStatus){
			vprintk("frame skip mode return\n");
			return -2;
		}
		return -1;
	}
	
	//printk("# last disp num(%d) \n", tccvid_vsync.vsync_buffer.last_cleared_buff_id ) ;
	if(tccvid_vsync.mvcMode == 1)
		return tccvid_vsync.vsync_buffer.last_cleared_buff_id > 1 ? (tccvid_vsync.vsync_buffer.last_cleared_buff_id-2):0 ; 
	else
		return tccvid_vsync.vsync_buffer.last_cleared_buff_id ? (tccvid_vsync.vsync_buffer.last_cleared_buff_id-1):0 ; 

}
EXPORT_SYMBOL(tcc_video_get_displayed);

void tcc_video_clear_frame(int idx)
{
	int syncBufferIdx;
	
	syncBufferIdx = idx;

	printk("video frame clear %d\n", syncBufferIdx) ;

	spin_lock_irq(&vsync_lock) ;
	tcc_vsync_pop_all_buffer(&tccvid_vsync.vsync_buffer);
	spin_unlock_irq(&vsync_lock) ;
	//printk("valid video fram count %d\n", tccvid_vsync.vsync_buffer.valid_buff_count) ;

	tccvid_vsync.vsync_buffer.available_buffer_id_on_vpu = syncBufferIdx;//+1;
}
EXPORT_SYMBOL(tcc_video_clear_frame);

int tcc_video_get_available_buffer_id(void)
{
	return tccvid_vsync.vsync_buffer.available_buffer_id_on_vpu;
}
EXPORT_SYMBOL(tcc_video_get_available_buffer_id);

int tcc_video_swap_vpu_frame(int idx)
{
#ifdef CONFIG_VIDEO_DISPLAY_SWAP_VPU_FRAME
	WMIXER_INFO_TYPE WmixerInfo;
	struct tcc_lcdc_image_update TempImage;
	struct inode	wm_inode;
	struct file		wm_flip;

	if(!vsync_started || CurrDisplayingImage.enable == 0 || LastFrame){
		printk("----> return tcc_video_swap_vpu_frame :: reason %d/%d/%d \n", vsync_started, CurrDisplayingImage.enable, LastFrame);
		return -1;
	}

	if( tccvid_vsync.outputMode == Output_SelectMode )
	{
		tccxxx_wmixer_open((struct inode *)&wm_inode, (struct file *)&wm_flip);
		msleep(16); // wait until previous interrupt is completed!!
		dprintk("----> start tcc_video_swap_vpu_frame :: %d \n", CurrDisplayingImage.buffer_unique_id);
		memcpy(&TempImage, &CurrDisplayingImage, sizeof(struct tcc_lcdc_image_update));
		tcc_video_frame_move(&wm_flip, &TempImage, &WmixerInfo, fb_lastframe_pbuf.base, TempImage.fmt);
		TempImage.addr0 = WmixerInfo.dst_y_addr;
		TempImage.addr1 = WmixerInfo.dst_u_addr;
		TempImage.addr2 = WmixerInfo.dst_v_addr;
		TempImage.buffer_unique_id = tccvid_vsync.vsync_buffer.available_buffer_id_on_vpu;

		if(tccvid_vsync.deinterlace_mode)
		{
				#ifndef VIQE_DUPLICATE_ROUTINE
				TCC_VIQE_DI_Run60Hz(TempImage.on_the_fly, TempImage.addr0, TempImage.addr1, TempImage.addr2,
									TempImage.Frame_width, TempImage.Frame_height,
									TempImage.crop_top,TempImage.crop_bottom, TempImage.crop_left, TempImage.crop_right, 
									TempImage.Image_width, TempImage.Image_height, 
									TempImage.offset_x, TempImage.offset_y, TempImage.odd_first_flag, TempImage.frameInfo_interlace, 1);
				#else
				int current_time;
				
				viqe_render_init();

				current_time = tcc_vsync_get_time();
				viqe_render_frame(TempImage.addr0, TempImage.addr1, TempImage.addr2, TempImage.odd_first_flag, tccvid_vsync.vsync_interval, current_time,
								TempImage.Frame_width, TempImage.Frame_height,	// srcWidth, srcHeight
								TempImage.crop_top, TempImage.crop_bottom, TempImage.crop_left, TempImage.crop_right,
								TempImage.Image_width, TempImage.Image_height,
								TempImage.offset_x, TempImage.offset_y, TempImage.frameInfo_interlace, 1);
				viqe_render_field(current_time);
				#endif

		}
		else
		{
			switch(Output_SelectMode)
			{
		#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
				case TCC_OUTPUT_LCD:
					TCC_LCD_DISPLAY_UPDATE(LCD_LCDC_NUM, &TempImage);
					break;
		#endif
				case TCC_OUTPUT_HDMI:
					TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, &TempImage);
					break;
				case TCC_OUTPUT_COMPONENT:
		#if defined(CONFIG_FB_TCC_COMPONENT)
						tcc_component_update(&TempImage);
		#endif
					break;
				case TCC_OUTPUT_COMPOSITE:
		#if defined(CONFIG_FB_TCC_COMPOSITE)
						tcc_composite_update(&TempImage);
		#endif
					break;
				default:
					break;
			}
		}
		dprintk("----> video seek %d, %d :: backup %d \n", idx, CurrDisplayingImage.buffer_unique_id, CurrDisplayingImage.buffer_unique_id) ;
		msleep(16); // wait until current setting is applyed completely!!
		tccxxx_wmixer_release((struct inode *)&wm_inode, (struct file *)&wm_flip);
	}

	return CurrDisplayingImage.buffer_unique_id;
#else
	return -1;
#endif
}
EXPORT_SYMBOL(tcc_video_swap_vpu_frame);

void tcc_video_set_framerate(int fps)
{
	int media_time_gap;
	printk("### TCC_LCDC_VIDEO_SET_FRAMERATE %d\n", (int)fps);

	if(fps < 1000)
	{
		if(fps == 29)
		{
			fps = 30;
		}
		else if(fps == 23)
		{
			fps = 24;
		}
	}
	else
	{
		fps += 500;
		fps /= 1000;
	}

	tccvid_vsync.video_frame_rate = fps;

	if(tccvid_vsync.video_frame_rate > 0)
		media_time_gap = 1000/tccvid_vsync.video_frame_rate/2;
	else
		media_time_gap = 16;
	
	tccvid_vsync.nTimeGapToNextField= media_time_gap;

	printk("### video_frame_rate(%d), media_time_gap(%d)\n",tccvid_vsync.video_frame_rate, media_time_gap);
}
EXPORT_SYMBOL(tcc_video_set_framerate);

void tcc_video_skip_frame_start(void)
{
	vprintk("video fram skip start\n") ;
	printk("TCC_LCDC_VIDEO_SKIP_FRAME_START \n") ;
	spin_lock_irq(&vsync_lock) ;
	tcc_vsync_pop_all_buffer(&tccvid_vsync.vsync_buffer);
	spin_unlock_irq(&vsync_lock);
	tccvid_vsync.skipFrameStatus = 1;
}
EXPORT_SYMBOL(tcc_video_skip_frame_start);

void tcc_video_skip_frame_end(void)
{
	if(tccvid_vsync.skipFrameStatus)
	{
		vprintk("video fram skip end\n") ;
		spin_lock_irq(&vsync_lock) ;
		tcc_vsync_pop_all_buffer(&tccvid_vsync.vsync_buffer);
		spin_unlock_irq(&vsync_lock);
		printk("TCC_LCDC_VIDEO_SKIP_FRAME_END \n") ;

		tccvid_vsync.skipFrameStatus = 0;
		tccfb_ResetSyncTime(0);
	}

}
EXPORT_SYMBOL(tcc_video_skip_frame_end);

void tcc_video_skip_one_frame(int frame_id)
{
	spin_lock_irq(&vsync_lockDisp) ;
	//printk("TCC_LCDC_VIDEO_SKIP_ONE_FRAME frame_id %d\n",frame_id) ;
	tccvid_vsync.vsync_buffer.last_cleared_buff_id = frame_id;
	spin_unlock_irq(&vsync_lockDisp) ;
}
EXPORT_SYMBOL(tcc_video_skip_one_frame);

int tcc_video_get_valid_count(void)
{
	return tccvid_vsync.vsync_buffer.readable_buff_count; 
}
EXPORT_SYMBOL(tcc_video_get_valid_count);

#endif

int tcc_video_frame_move( struct file *wmix_file, struct tcc_lcdc_image_update* inFframeInfo, WMIXER_INFO_TYPE* WmixerInfo, unsigned int target_addr, unsigned int target_format)
{
	int ret = 0;

	memset(WmixerInfo, 0x00, sizeof(WmixerInfo));
	WmixerInfo->rsp_type		= WMIXER_POLLING;

	WmixerInfo->img_width 	= inFframeInfo->Frame_width;
	WmixerInfo->img_height	= inFframeInfo->Frame_height;

	//source info
	WmixerInfo->src_y_addr	= (unsigned int)inFframeInfo->addr0;
	WmixerInfo->src_u_addr	= (unsigned int)inFframeInfo->addr1;
	WmixerInfo->src_v_addr	= (unsigned int)inFframeInfo->addr2;

	WmixerInfo->src_fmt		= inFframeInfo->fmt;

	//destination info
	WmixerInfo->dst_y_addr	= target_addr;
	WmixerInfo->dst_u_addr	= (unsigned int)GET_ADDR_YUV42X_spU(WmixerInfo->dst_y_addr, WmixerInfo->img_width, WmixerInfo->img_height);
	WmixerInfo->dst_v_addr	= (unsigned int)GET_ADDR_YUV422_spV(WmixerInfo->dst_u_addr, WmixerInfo->img_width, WmixerInfo->img_height);

	WmixerInfo->dst_fmt		= target_format;

	ret = tccxxx_wmixer_ioctl((struct file *)wmix_file, TCC_WMIXER_IOCTRL_KERNEL, (unsigned long)WmixerInfo);

	printk("### tcc_video_frame_move pre-processing(%dx%d - 0x%x) \n", inFframeInfo->Frame_width, inFframeInfo->Frame_height, inFframeInfo->addr0);

	return ret;
}

int tcc_video_lastframe_just_rdma_off(char hdmi_lcdc)
{
	int lcdc_layer = 0;
	VIOC_RDMA * pRDMABase;
	VIOC_DISP * pDISPBase;
	unsigned int lcd_width = 0, lcd_height = 0;

#if defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_ARCH_TCC893X)
	lcdc_layer = 3;
#else
	lcdc_layer = 2;
#endif

	if(hdmi_lcdc)
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);

		switch(lcdc_layer)
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
	}
	else
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);

		switch(lcdc_layer)
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
				pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA03);
				break;
		}
	}

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);

	if((!lcd_width) || (!lcd_height)){
		printk(" %s LCD :: Error :: lcd_width %d, lcd_height %d \n", __func__,lcd_width, lcd_height);
		return -1;
	}

	VIOC_RDMA_SetImageDisable(pRDMABase);

	return 0;
}

int tcc_video_last_frame(int res_changed)
{
	if(!HDMI_video_mode || LastFrame){
		if(res_changed&LASTFRAME_FOR_RESOLUTION_CHANGE)
			LastFrame_for_ResChanged = 1;
		else if(res_changed&LASTFRAME_FOR_CODEC_CHANGE)
			LastFrame_for_CodecChanged = 1;
		printk("----> TCC_LCDC_HDMI_LASTFRAME returned due to (%d or %d)!! res_changed(%d), codec_changed(%d) \n", !HDMI_video_mode, LastFrame, LastFrame_for_ResChanged, LastFrame_for_CodecChanged);
		return 0;
	}

#if  defined(CONFIG_HDMI_DISPLAY_LASTFRAME)
	if(enabled_LastFrame)
	{
		if( Output_SelectMode == TCC_OUTPUT_HDMI ||Output_SelectMode== TCC_OUTPUT_COMPOSITE || Output_SelectMode == TCC_OUTPUT_COMPONENT
#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
			|| Output_SelectMode == TCC_OUTPUT_LCD 
#endif
		)
		{
			struct tcc_lcdc_image_update lastUpdated;

			memcpy(&lastUpdated, &Last_ImageInfo, sizeof(struct tcc_lcdc_image_update));

			LastFrame = 1;
			LastFrame_for_ResChanged = res_changed&0x1;
			LastFrame_for_CodecChanged = res_changed&0x2;

			spin_lock_irq(&vsync_lock);
			tcc_vsync_pop_all_buffer(&tccvid_vsync.vsync_buffer);
			spin_unlock_irq(&vsync_lock);
			
			if(lastUpdated.Frame_width == 0 || lastUpdated.Frame_height == 0)
			{
				printk("Error :: no setting for Last-Frame \n");
				return -1;
			}

			if(Last_ImageInfo.enable == 0){
				printk("----> return last-frame :: The channel has already disabled. info(%dx%d), res_changed(%d), codec_changed(%d)\n",
							Last_ImageInfo.Frame_width, Last_ImageInfo.Frame_height, LastFrame_for_ResChanged, LastFrame_for_CodecChanged);
				return -100;
			}

			if(Last_ImageInfo.outputMode != Output_SelectMode-TCC_OUTPUT_LCD){
				printk("----> return last-frame :: mode is different between %d and %d \n", Last_ImageInfo.outputMode, Output_SelectMode-TCC_OUTPUT_LCD);
				return -100;
			}

			//tccxxx_wmixer_open((struct inode *)&wm_inode, (struct file *)&wm_flip);
			//tcc_video_frame_move( &wm_flip, &lastUpdated, &WmixerInfo, fb_lastframe_pbuf.base, TCC_LCDC_IMG_FMT_YUYV);
			//tccxxx_wmixer_release((struct inode *)&wm_inode, (struct file *)&wm_flip);

	#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
				// this code prevent a error of transparent on layer0 after stopping v
			if(lastUpdated.Lcdc_layer == 0 && lastUpdated.enable == 0)
			{
				printk("Last-frame for a error prevention \n");
				spin_lock_irq(&vsync_lock);
				tccvid_vsync.outputMode = -1;
				spin_unlock_irq(&vsync_lock);
			}
	#endif


	#if 1//def CONFIG_TCC_OUTPUT_ATTACH
		#if defined(CONFIG_ARCH_TCC893X)
		{
			WMIXER_ALPHASCALERING_INFO_TYPE fbmixer;

			memset(&fbmixer, 0x00, sizeof(fbmixer));

			fbmixer.rsp_type                = WMIXER_POLLING;

			fbmixer.src_img_width   = lastUpdated.Frame_width;
			fbmixer.src_img_height  = lastUpdated.Frame_height;
			fbmixer.src_win_left    = 0;
			fbmixer.src_win_top     = 0;
			fbmixer.src_win_right   = lastUpdated.Frame_width;
			fbmixer.src_win_bottom = lastUpdated.Frame_height;

			//source info
			fbmixer.src_y_addr      = (unsigned int)lastUpdated.addr0;
			fbmixer.src_u_addr      = (unsigned int)lastUpdated.addr1;
			fbmixer.src_v_addr      = (unsigned int)lastUpdated.addr2;
			fbmixer.src_fmt = lastUpdated.fmt;

			//destination info
			fbmixer.dst_img_width   = lastUpdated.Image_width;
			fbmixer.dst_img_height  = lastUpdated.Image_height;
			fbmixer.dst_win_left    = 0;
			fbmixer.dst_win_top     = 0;
			fbmixer.dst_win_right   = lastUpdated.Image_width;
			fbmixer.dst_win_bottom  = lastUpdated.Image_height;
			
			if(tccvid_vsync.deinterlace_mode > 0)
				fbmixer.interlaced= true;
			else
				fbmixer.interlaced = false;

			fbmixer.dst_y_addr	    = (unsigned int)fb_lastframe_pbuf.base;
			fbmixer.dst_fmt         = TCC_LCDC_IMG_FMT_YUYV;

			tccxxx_wmixer1_open((struct inode *)&lastframe_wm_alpha_inode, (struct file *)&lastframe_wm_alpha_flip);
			tccxxx_wmixer1_ioctl((struct file *)&lastframe_wm_alpha_flip, TCC_WMIXER_ALPHA_SCALING_KERNEL, (unsigned long)&fbmixer);
			tccxxx_wmixer1_release((struct inode *)&lastframe_wm_alpha_inode, (struct file *)&lastframe_wm_alpha_flip);

			lastUpdated.addr0	 	= (unsigned int)fbmixer.dst_y_addr;
		}
		#else	//defined(CONFIG_ARCH_TCC893X)
		{
			WMIXER_INFO_TYPE WmixerInfo;
			SCALER_TYPE ScalerInfo;
			char interlaced_scaler = 0;

			if( fb_lastframe_pbuf_ext.size <= 0 )
			{
				printk("error: insufficient memory to process last-frame!! check if size of 'fb_wmixer_ext' is 4MB in bootloader \n");
				return -100;
			}

			memset(&WmixerInfo, 0x00, sizeof(WmixerInfo));
			WmixerInfo.rsp_type		= WMIXER_POLLING;

			WmixerInfo.img_width 	= lastUpdated.Frame_width;
			WmixerInfo.img_height	= lastUpdated.Frame_height;

			//source info
			WmixerInfo.src_y_addr	= (unsigned int)lastUpdated.addr0;
			WmixerInfo.src_u_addr	= (unsigned int)lastUpdated.addr1;
			WmixerInfo.src_v_addr	= (unsigned int)lastUpdated.addr2;

			WmixerInfo.src_fmt		= lastUpdated.fmt;

			//destination info
			WmixerInfo.dst_y_addr	= (unsigned int)fb_lastframe_pbuf_ext.base;
			WmixerInfo.dst_u_addr	= (unsigned int)GET_ADDR_YUV42X_spU(WmixerInfo.dst_y_addr, WmixerInfo.img_width, WmixerInfo.img_height);
			WmixerInfo.dst_v_addr	= (unsigned int)GET_ADDR_YUV422_spV(WmixerInfo.dst_u_addr, WmixerInfo.img_width, WmixerInfo.img_height);

			WmixerInfo.dst_fmt		= TCC_LCDC_IMG_FMT_YUYV;	//test
			printk("### TCC_LCDC_HDMI_LASTFRAME pre-processing(%dx%d - 0x%x) \n", lastUpdated.Frame_width, lastUpdated.Frame_height, lastUpdated.addr0);

			tccxxx_wmixer_open((struct inode *)&lastframe_wm_alpha_inode, (struct file *)&lastframe_wm_alpha_flip);
			tccxxx_wmixer_ioctl((struct file *)&lastframe_wm_alpha_flip, (unsigned int)TCC_WMIXER_IOCTRL_KERNEL, (unsigned long)&WmixerInfo);
			tccxxx_wmixer_release((struct inode *)&lastframe_wm_alpha_inode, (struct file *)&lastframe_wm_alpha_flip);

			if(!interlaced_scaler && tccvid_vsync.deinterlace_mode > 0)
			{
				int i, w, h;
				unsigned int base_addr, src_base_addr, dst_base_addr;

				printk("Last-frame for s/w De-interlace. \n");
				base_addr = (unsigned int)ioremap_nocache(fb_lastframe_pbuf_ext.base,fb_lastframe_pbuf_ext.size);
				w = lastUpdated.Frame_width;
				h = lastUpdated.Frame_height;

				src_base_addr = base_addr + (w*h*2) - w*2;
				dst_base_addr = base_addr + (w*h*2) - w*2*2;

				//printk("0x%x 0x%x 0x%x \n",base_addr,src_base_addr,dst_base_addr);

				for(i=0; i<h/2; i++)
				{
					//memcpy(dst_base_addr, src_base_addr, w*2);
					//dst_base_addr -= w*2;
					memcpy((void *)dst_base_addr, (void *)src_base_addr, w*2);
					dst_base_addr -= w*2 * 2;
					src_base_addr -= w*2 * 2;
				}
				iounmap((void*)base_addr);

			}

			memset(&ScalerInfo, 0x00, sizeof(ScalerInfo));
			ScalerInfo.responsetype = SCALER_POLLING;

			ScalerInfo.src_fmt			= TCC_LCDC_IMG_FMT_YUYV;
			ScalerInfo.src_ImgWidth		= lastUpdated.Frame_width;
			ScalerInfo.src_ImgHeight	= lastUpdated.Frame_height;
			ScalerInfo.src_winLeft		= 0;
			ScalerInfo.src_winTop		= 0;
			ScalerInfo.src_winRight		= lastUpdated.Frame_width;
			ScalerInfo.src_winBottom	= lastUpdated.Frame_height;

			ScalerInfo.src_Yaddr		= (char*)fb_lastframe_pbuf_ext.base;
			ScalerInfo.src_Uaddr		= (char*)GET_ADDR_YUV42X_spU(ScalerInfo.src_Yaddr, ScalerInfo.src_ImgWidth, ScalerInfo.src_ImgHeight);
			ScalerInfo.src_Vaddr		= (char*)GET_ADDR_YUV422_spV(ScalerInfo.src_Uaddr, ScalerInfo.src_ImgWidth, ScalerInfo.src_ImgHeight);

			ScalerInfo.dest_fmt			= TCC_LCDC_IMG_FMT_YUYV;
			ScalerInfo.dest_ImgWidth	= lastUpdated.Image_width;
			ScalerInfo.dest_ImgHeight	= lastUpdated.Image_height;
			ScalerInfo.dest_winLeft		= 0;
			ScalerInfo.dest_winTop		= 0;
			ScalerInfo.dest_winRight	= lastUpdated.Image_width;
			ScalerInfo.dest_winBottom	= lastUpdated.Image_height;

			ScalerInfo.dest_Yaddr		= (char*)fb_lastframe_pbuf.base;
			ScalerInfo.dest_Uaddr		= (char*)GET_ADDR_YUV42X_spU(ScalerInfo.dest_Yaddr, ScalerInfo.dest_ImgWidth, ScalerInfo.dest_ImgHeight);
			ScalerInfo.dest_Vaddr		= (char*)GET_ADDR_YUV422_spV(ScalerInfo.dest_Uaddr, ScalerInfo.dest_ImgWidth, ScalerInfo.dest_ImgHeight);

			if(interlaced_scaler && tccvid_vsync.deinterlace_mode > 0)
				ScalerInfo.interlaced = true;
			else
				ScalerInfo.interlaced = false;

			tccxxx_scaler2_open((struct inode *)&lastframe_sc_inode, (struct file *)&lastframe_sc_flip);
			tccxxx_scaler2_ioctl((struct file *)&lastframe_sc_flip, (unsigned int)TCC_SCALER_IOCTRL_KERENL, (unsigned long)&ScalerInfo);
			tccxxx_scaler2_release((struct inode *)&lastframe_sc_inode, (struct file *)&lastframe_sc_flip);

			lastUpdated.addr0	 	= (unsigned int)ScalerInfo.dest_Yaddr;
		}
		#endif//defined(CONFIG_ARCH_TCC893X)

			lastUpdated.Frame_width  = lastUpdated.Image_width;
			lastUpdated.Frame_height = lastUpdated.Image_height;	
	#else
			Last_ImageInfo.addr0	 	= (unsigned int)fb_lastframe_pbuf.base;
	#endif
			lastUpdated.fmt	 		= TCC_LCDC_IMG_FMT_YUYV; // W x H * 2 = Max 4Mb

			printk("### TCC_LCDC_HDMI_LASTFRAME Start info(%dx%d), res_changed(%d) \n", lastUpdated.Frame_width, lastUpdated.Frame_height, LastFrame_for_ResChanged);

			{
				int ret = 0;

				spin_lock_irq(&LastFrame_lockDisp);
	#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
				if( Output_SelectMode == TCC_OUTPUT_LCD )
					ret = TCC_HDMI_LAST_FRAME_UPDATE(LCD_LCDC_NUM, (struct tcc_lcdc_image_update *)&lastUpdated);
				else
	#endif
					ret = TCC_HDMI_LAST_FRAME_UPDATE(EX_OUT_LCDC, (struct tcc_lcdc_image_update *)&lastUpdated);
				spin_unlock_irq(&LastFrame_lockDisp);
				if( ret >= 0){
	#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
					if( Output_SelectMode == TCC_OUTPUT_LCD )
						TCC_HDMI_PREV_FRAME_OFF(LCD_LCDC_NUM, (tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory && !tccvid_vsync.interlace_bypass_lcdc));
					else
	#endif
						TCC_HDMI_PREV_FRAME_OFF(EX_OUT_LCDC, (tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory && !tccvid_vsync.interlace_bypass_lcdc));

				}

			}

	#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
			if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory && !tccvid_vsync.interlace_bypass_lcdc)
				TCC_VIQE_DI_DeInit60Hz();
	#endif

			printk("### TCC_LCDC_HDMI_LASTFRAME End \n");
		}
		else
		{
			printk("TCC_LCDC_HDMI_LASTFRAME skip :: reason(Output_SelectMode == %d)  \n", Output_SelectMode);
		}

		return 0;
	}
	else
#endif
	{
		if(Output_SelectMode == TCC_OUTPUT_HDMI ||Output_SelectMode == TCC_OUTPUT_COMPOSITE || Output_SelectMode == TCC_OUTPUT_COMPONENT 
	#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
			|| Output_SelectMode ==  TCC_OUTPUT_LCD
	#endif
		)
		{
			WMIXER_INFO_TYPE WmixerInfo;
			struct tcc_lcdc_image_update lastUpdated;

			LastFrame = 1;
			LastFrame_for_ResChanged = res_changed&LASTFRAME_FOR_RESOLUTION_CHANGE;
			LastFrame_for_CodecChanged = res_changed&LASTFRAME_FOR_CODEC_CHANGE ? 1:0;

			if(Last_ImageInfo.enable == 0){
				printk("----> return last-frame :: The channel has already disabled. info(%dx%d), res_changed(%d), codec_changed(%d)\n",
							Last_ImageInfo.Frame_width, Last_ImageInfo.Frame_height, LastFrame_for_ResChanged, LastFrame_for_CodecChanged);
				return -100;
			}

			if(Last_ImageInfo.outputMode != Output_SelectMode-TCC_OUTPUT_LCD){
				printk("----> return last-frame :: mode is different between %d and %d \n", Last_ImageInfo.outputMode, Output_SelectMode-TCC_OUTPUT_LCD);
				return -100;
			}

			if(tccvid_vsync.deinterlace_mode > 0 && vsync_started){
				if(!LastFrame_for_CodecChanged){
					printk("----> return last-frame :: deinterlace_mode %d. info(%dx%d), res_changed(%d), codec_changed(%d) \n", tccvid_vsync.deinterlace_mode,
								Last_ImageInfo.Frame_width, Last_ImageInfo.Frame_height, LastFrame_for_ResChanged, LastFrame_for_CodecChanged);
					return -100;
				}
			}

			memcpy(&lastUpdated, &Last_ImageInfo, sizeof(struct tcc_lcdc_image_update));

			if(vsync_started == 1){
				spin_lock_irq(&vsync_lock);
				tcc_vsync_pop_all_buffer(&tccvid_vsync.vsync_buffer);
				spin_unlock_irq(&vsync_lock);
			}

			if(LastFrame_for_CodecChanged)
			{
				spin_lock_irq(&LastFrame_lockDisp);
				if(lastUpdated.outputMode == OUTPUT_HDMI)
					tcc_video_lastframe_just_rdma_off(EX_OUT_LCDC);
				else
					tcc_video_lastframe_just_rdma_off(LCD_LCDC_NUM);
				spin_unlock_irq(&LastFrame_lockDisp);

				printk("---->  fake TCC_LCDC_HDMI_LASTFRAME to diable only RDMA channel. info(%dx%d), res_changed(%d), codec_changed(%d) \n",
								lastUpdated.Frame_width, lastUpdated.Frame_height, LastFrame_for_ResChanged, LastFrame_for_CodecChanged);
				return -100;
			}
			else
			{
				struct inode	wm_inode;
				struct file		wm_flip;

				tccxxx_wmixer_open((struct inode *)&wm_inode, (struct file *)&wm_flip);
				tcc_video_frame_move( &wm_flip, &lastUpdated, &WmixerInfo, fb_lastframe_pbuf.base, TCC_LCDC_IMG_FMT_YUYV);
				tccxxx_wmixer_release((struct inode *)&wm_inode, (struct file *)&wm_flip);

				lastUpdated.fmt = WmixerInfo.dst_fmt;
				lastUpdated.addr0 = WmixerInfo.dst_y_addr;
				lastUpdated.addr1 = WmixerInfo.dst_u_addr;
				lastUpdated.addr2 = WmixerInfo.dst_v_addr;
			}

			spin_lock_irq(&LastFrame_lockDisp);
			printk("----> fake TCC_LCDC_HDMI_LASTFRAME Start info(%dx%d), res_changed(%d), codec_changed(%d) \n", lastUpdated.Frame_width, lastUpdated.Frame_height, LastFrame_for_ResChanged, LastFrame_for_CodecChanged);

			switch(Output_SelectMode)
			{
				#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
				case TCC_OUTPUT_LCD:
					TCC_LCD_DISPLAY_UPDATE(LCD_LCDC_NUM, &lastUpdated);
					break;
				#endif
				case TCC_OUTPUT_HDMI:
					TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, &lastUpdated);
					break;
				case TCC_OUTPUT_COMPONENT:
				#if defined(CONFIG_FB_TCC_COMPONENT)
						tcc_component_update(&lastUpdated);
				#endif
					break;
				case TCC_OUTPUT_COMPOSITE:
				#if defined(CONFIG_FB_TCC_COMPOSITE)
						tcc_composite_update(&lastUpdated);
				#endif
					break;
				default:
					break;
			}

			printk("----> fake TCC_LCDC_HDMI_LASTFRAME End \n");
			spin_unlock_irq(&LastFrame_lockDisp);

			return -0x100;
		}
		else
		{
			printk("fake TCC_LCDC_HDMI_LASTFRAME skip :: reason(Output_SelectMode == %d)  \n", Output_SelectMode);
		}
	}

	return -1;
}
EXPORT_SYMBOL(tcc_video_last_frame);

int tcc_video_ctrl_last_frame(int enable)
{
#if defined(CONFIG_HDMI_DISPLAY_LASTFRAME) 
	#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
	enabled_LastFrame = 1;
	#else
	enabled_LastFrame = enable;
	#endif
#else
	enabled_LastFrame = 0;
#endif
	return 0;
}
EXPORT_SYMBOL(tcc_video_ctrl_last_frame);

void tccfb_output_starter(char output_type, char lcdc_num, struct lcdc_timimg_parms_t *lcdc_timing)
{
	switch(output_type)
	{
		case TCC_OUTPUT_HDMI:
 		 	TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_HDMI, lcdc_num, 1);
 			TCC_HDMI_LCDC_Timing(lcdc_num, lcdc_timing);
			TCC_OUTPUT_UPDATE_OnOff(1, TCC_OUTPUT_HDMI);
			Output_SelectMode = TCC_OUTPUT_HDMI;
			break;

		case TCC_OUTPUT_COMPOSITE:
 		 	TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_COMPOSITE, lcdc_num, 1);
			Output_SelectMode = TCC_OUTPUT_COMPOSITE;
			break;

		case TCC_OUTPUT_COMPONENT:
 		 	TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_COMPONENT, lcdc_num, 1);
			Output_SelectMode = TCC_OUTPUT_COMPONENT;
			break;
	}

	#if defined(TCC_VIDEO_DISPLAY_BY_VSYNC_INT)
		memset( &tccvid_vsync, 0, sizeof( tccvid_vsync ) );
		tccvid_vsync.overlayUsedFlag = -1;
		tccvid_vsync.outputMode = -1;
		tccvid_vsync.firstFrameFlag = 1;
		tccvid_vsync.deinterlace_mode= -1;
		tccvid_vsync.m2m_mode = -1;
		tccvid_vsync.output_toMemory = -1;
	#endif
}

void tccfb_output_set_mode(char output_type)
{
	switch(output_type)
	{
		case TCC_OUTPUT_HDMI:
			Output_SelectMode = TCC_OUTPUT_HDMI;
			break;

		case TCC_OUTPUT_COMPOSITE:
			Output_SelectMode = TCC_OUTPUT_COMPOSITE;
			break;

		case TCC_OUTPUT_COMPONENT:
			Output_SelectMode = TCC_OUTPUT_COMPONENT;
			break;
	}
}

unsigned int tccfb_output_get_mode(void)
{
	return Output_SelectMode;
}

#ifdef CONFIG_EXTEND_DISPLAY_DELAY
static void extenddisplay_work_handler(struct work_struct *work)
{
	struct delayed_work *d_work = to_delayed_work(work);

	extenddisplay_delay_StructInfo *pExtDispStruct =  container_of(d_work, extenddisplay_delay_StructInfo, delay_work);
	extenddisplay_delay_DisplayInfo *pLastImageInfo = pExtDispStruct->displaydata;

	external_fbioput_vscreeninfo sc_info = pLastImageInfo->sc_info;

	sprintk("%s Ex_SelectMode:%d Output_SelectMode: %d  add:0x%x updated UI:%d video:%d\n", 
		__func__, pLastImageInfo->Ex_SelectMode , Output_SelectMode, pLastImageInfo->base_addr+ sc_info.offset, pLastImageInfo->UI_updated, pLastImageInfo->Video_updated);

	atomic_set(&pExtDispStruct->update_allow, true);


	if((pLastImageInfo->Ex_SelectMode == Output_SelectMode))
	{
		spin_lock(&pExtDispStruct->delay_spinlock);

		if(!pLastImageInfo->UI_updated)
		{
			TCC_OUTPUT_FB_Update_External(sc_info.width, sc_info.height, sc_info.bits_per_pixel, pLastImageInfo->base_addr+ sc_info.offset, Output_SelectMode);
			pLastImageInfo->UI_updated = true;
		}
		//for video 
		if(!pLastImageInfo->Video_updated)
		{
			if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory && !tccvid_vsync.interlace_bypass_lcdc)
			{
				struct tcc_lcdc_image_update *input_image;
				input_image= &pLastImageInfo->VideoImg;
				TCC_VIQE_DI_Run60Hz(input_image->on_the_fly, input_image->addr0, input_image->addr1, input_image->addr2,
													input_image->Frame_width, input_image->Frame_height,
													input_image->crop_top,input_image->crop_bottom, input_image->crop_left, input_image->crop_right, 
													input_image->Image_width, input_image->Image_height, 
													input_image->offset_x, input_image->offset_y, input_image->odd_first_flag, input_image->frameInfo_interlace, 0);
			}
			else
				TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, &pLastImageInfo->VideoImg);
			pLastImageInfo->Video_updated = true;
		}
		spin_unlock(&pExtDispStruct->delay_spinlock);

		TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);		
	}
}



#endif//CONFIG_EXTEND_DISPLAY_DELAY

static int tccfb_check_var(struct fb_var_screeninfo *var,
			       struct fb_info *info)
{
	/* validate bpp */
	if (var->bits_per_pixel > 32)
		var->bits_per_pixel = 32;
	else if (var->bits_per_pixel < 16)
		var->bits_per_pixel = 16;

	/* set r/g/b positions */
	if (var->bits_per_pixel == 16) {
		var->red.offset 	= 11;
		var->green.offset	= 5;
		var->blue.offset	= 0;
		var->red.length 	= 5;
		var->green.length	= 6;
		var->blue.length	= 5;
		var->transp.length	= 0;
	} else if (var->bits_per_pixel == 32) {
		var->red.offset 	= 16;
		var->green.offset	= 8;
		var->blue.offset	= 0;
		var->transp.offset	= 24;
		var->red.length 	= 8;
		var->green.length	= 8;
		var->blue.length	= 8;
		var->transp.length	= 8;
	} else {
		var->red.length 	= var->bits_per_pixel;
		var->red.offset 	= 0;
		var->green.length	= var->bits_per_pixel;
		var->green.offset	= 0;
		var->blue.length	= var->bits_per_pixel;
		var->blue.offset	= 0;
		var->transp.length	= 0;
	}
	if (var->yres_virtual < var->yoffset + var->yres)
		var->yres_virtual = var->yoffset + var->yres;

	return 0;
}

/* tccfb_pan_display
 *
 * pandisplay (set) the controller from the given framebuffer
 * information
*/
static int tccfb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	char tcc_output_ret = 0;
	unsigned int base_addr = 0;
	struct tccfb_info *fbi =(struct tccfb_info *) info->par;

	if(!fb_power_state)
		return 0;

 	base_addr = Output_BaseAddr = fbi->map_dma +var->xres * (var->bits_per_pixel/8) * var->yoffset;

	sprintk("%s addr:0x%x Yoffset:%d \n", __func__, base_addr, var->yoffset);

	#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
		if(Output_SelectMode > TCC_OUTPUT_LCD)
	#else
		if(Output_SelectMode)
	#endif
		{
			unsigned char hdmi_phy_status, hdmi_system_en = 0, update_flag = 0;

			if( Output_SelectMode == TCC_OUTPUT_HDMI) {
				hdmi_phy_status = hdmi_get_phy_status();
				hdmi_system_en = hdmi_get_system_en();

				if( hdmi_phy_status && hdmi_system_en )
					update_flag = 1;
				else
					update_flag = 0;
			} else {
				update_flag = 1;
			}
			
			if(update_flag == 1) {
				#ifdef MVC_PROCESS
					if(TCC_OUTPUT_FB_GetMVCStatus())
						tcc_output_ret = TCC_OUTPUT_FB_Update_3D(var->xres, var->yres, var->bits_per_pixel, base_addr, Output_SelectMode);
					else
						tcc_output_ret = TCC_OUTPUT_FB_Update(var->xres, var->yres, var->bits_per_pixel, base_addr, Output_SelectMode);
				#else
					tcc_output_ret = TCC_OUTPUT_FB_Update(var->xres, var->yres, var->bits_per_pixel, base_addr, Output_SelectMode);
				#endif

					if(tcc_output_ret )
						TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
			}
		}

	#if !defined(CONFIG_MACH_TCC8920ST) && !defined(CONFIG_MACH_TCC8930ST)
	tca_fb_pan_display(var, info);
	#endif
	
	return 0;
}
/* tccfb_activate_var
 *
 * activate (set) the controller from the given framebuffer
 * information
*/
static void tccfb_activate_var(struct tccfb_info *fbi,  struct fb_var_screeninfo *var)
{
	unsigned int imgch = 0;

	sprintk("%s node:0x%x TCC_DEVS:%d \n", __func__, fbi->fb->node, CONFIG_FB_TCC_DEVS);

	if((0 <= fbi->fb->node) && (fbi->fb->node < CONFIG_FB_TCC_DEVS))
		imgch = fbi->fb->node;
	else
		return;

	tca_fb_activate_var(fbi, var);
}

/*
 *      tccfb_set_par - Optional function. Alters the hardware state.
 *      @info: frame buffer structure that represents a single frame buffer
 *
 */
static int tccfb_set_par(struct fb_info *info)
{
	struct tccfb_info *fbi = info->par;
	struct fb_var_screeninfo *var = &info->var;

	sprintk("- tccfb_set_par pwr:%d  output:%d \n",fb_power_state, Output_SelectMode);

	if (var->bits_per_pixel == 16)
		fbi->fb->fix.visual = FB_VISUAL_TRUECOLOR;
	else if (var->bits_per_pixel == 32)
		fbi->fb->fix.visual = FB_VISUAL_TRUECOLOR;
	else
		fbi->fb->fix.visual = FB_VISUAL_PSEUDOCOLOR;

	fbi->fb->fix.line_length = (var->xres*var->bits_per_pixel)/8;

	#if !defined(CONFIG_MACH_TCC8920ST) && !defined(CONFIG_MACH_TCC8930ST) 
		/* activate this new configuration */
	   	if(fb_power_state)
			tccfb_activate_var(fbi, var);
	#endif

	return 0;
}

/* For ISDBT */
static unsigned int tccfb_get_act_disp_num(void)
{
	unsigned int disp_num = 0;
		
	if(Output_SelectMode == TCC_OUTPUT_NONE){
		disp_num = tca_get_lcd_lcdc_num();		
	}else{
		disp_num = tca_get_output_lcdc_num();
	}

	printk("[%s] output:%d, disp_num:%d\n", __func__, Output_SelectMode, disp_num);

	return disp_num;
}

static int tccfb_ioctl(struct fb_info *info, unsigned int cmd,unsigned long arg)
{
	struct tccfb_info *fb_info = info->par;
	unsigned int imgch=0;
	int screen_width, screen_height;

	screen_width = lcd_panel->xres;
	screen_height = lcd_panel->yres;

	if((0 <= info->node) && (info->node < CONFIG_FB_TCC_DEVS))	{
		imgch = info->node;
	}
	else	{
		dprintk("ioctl: Error - fix.id[%d]\n", info->node);
		return 0;
	}


	switch(cmd)
	{
		case TCC_LCDC_GET_ACT_DISP_NUM:
			{
				unsigned int disp_num=0;
				disp_num = tccfb_get_act_disp_num();				
				if (copy_to_user((unsigned int *)arg, &disp_num, sizeof(unsigned int))) {
					return -EFAULT;
				}
			}
			break;
			
		case TCC_LCDC_SET_LAYER_ORDER:
			{
				unsigned int old_layer_order=0;
				tccfb_set_wmix_order_type	wmix_order_type;				

				if (copy_from_user((void*)&wmix_order_type, (const void*)arg, sizeof(tccfb_set_wmix_order_type)))
					return -EFAULT;

				old_layer_order = tccfb_set_wmixer_layer_order(wmix_order_type.num, wmix_order_type.ovp,fb_power_state);
				
				wmix_order_type.ovp = old_layer_order;
				
				if (copy_to_user((void *)arg, &wmix_order_type, sizeof(tccfb_set_wmix_order_type))) {
					return -EFAULT;
				}
			}
			break;
			
		case TCC_LCDC_GET_NUM:
			{
				unsigned int LCD_lcdc_number;
				LCD_lcdc_number = tca_get_lcd_lcdc_num();
				dprintk("%s: TCC_LCDC_GET_NUM :: %d    \n", __func__ , LCD_lcdc_number);
				
				if (copy_to_user((unsigned int *)arg, &LCD_lcdc_number, sizeof(unsigned int))) {
					return -EFAULT;
				}
			}
			break;

			
		case TCC_LCDC_SET_COLORENHANCE:
			{
				VIOC_DISP *pDISP;
				lcdc_colorenhance_params set_colorenhance, get_colorenhance;

				if(copy_from_user((void *)&set_colorenhance, (const void *)arg, sizeof(lcdc_colorenhance_params)))
					return -EFAULT;

				if (set_colorenhance.lcdc_num)
					pDISP = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
				else
					pDISP = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);

				VIOC_DISP_GetColorEnhancement(pDISP, (signed char *)&get_colorenhance.contrast, (signed char *)&get_colorenhance.brightness, (signed char *)&get_colorenhance.hue);

				if(set_colorenhance.hue > 0xFF)
					set_colorenhance.hue = get_colorenhance.hue;

				if(set_colorenhance.brightness > 0xFF)
					set_colorenhance.brightness = get_colorenhance.brightness;

				if(set_colorenhance.contrast > 0xFF)
					set_colorenhance.contrast = get_colorenhance.contrast;

				VIOC_DISP_SetColorEnhancement(pDISP, set_colorenhance.contrast, set_colorenhance.brightness, set_colorenhance.hue);
			}
			break;

		case TCC_LCDC_SET_GAMMA:
			{
				unsigned int lut_number = 0;
				lcdc_gamma_params lcdc_gamma;
				VIOC_LUT *pLUT =(VIOC_LUT*)tcc_p2v(HwVIOC_LUT);
				
				if(copy_from_user((void *)&lcdc_gamma, (const void *)arg, sizeof(lcdc_gamma_params)))
					return -EFAULT;

				if(lcdc_gamma.lcdc_num)
					lut_number = VIOC_LUT_DEV1;
				else
					lut_number = VIOC_LUT_DEV0;					

				if(lcdc_gamma.onoff)
					VIOC_LUT_Set_value(pLUT, lut_number,  lcdc_gamma.GammaBGR);
				
				VIOC_LUT_Enable(pLUT, lut_number, lcdc_gamma.onoff);
			}
			break;

			
		case TCC_LCD_HDMI_PORT_CHANGE_GET_STATE:
			{
				dprintk("%s: TCC_LCD_HDMI_PORT_CHANGE_GET_STATE :: %d    \n", __func__ , Port_change_state);
				
				if (copy_to_user((unsigned int *)arg, &Port_change_state, sizeof(unsigned int))) {
					return -EFAULT;
				}
			}
			break;

	#if defined(CONFIG_ARCH_TCC893X)
		
		case TCC_LCDC_SET_WMIXER_OVP:
			{
				unsigned int WMIXER_OVP = 0;

				if(copy_from_user((void *)&WMIXER_OVP, (const void *)arg,sizeof(unsigned int)))
					return -EFAULT;

				printk("%s: WMIXER_OVP :: %d   \n", __func__ , WMIXER_OVP);

				VIOC_API_WMIX_set_layer_order(LCD_LCDC_NUM,WMIXER_OVP);

			}
			break;
			
	#endif

		case TCC_LCDC_SET_LUT_BRIGHT:
			{
				lut_ctrl_params	LutInfo;

				if(copy_from_user((void *)&LutInfo, (const void *)arg, sizeof(lut_ctrl_params)))
					return -EFAULT;

				printk("%s: LUT onoff=[%d] brightness[%d]  \n", __func__ , LutInfo.onoff, LutInfo.brightness);

				VIOC_API_LUT_set_brightness(LCD_LCDC_NUM, &LutInfo);

			}
			break;

		case TCC_LCDC_SET_LUT_DVI:
			{
				lut_ctrl_params	LutInfo;

				if(copy_from_user((void *)&LutInfo, (const void *)arg, sizeof(lut_ctrl_params)))
					return -EFAULT;

				printk("%s: LUT onoff=[%d] dvi\n", __func__ , LutInfo.onoff);

				VIOC_API_LUT_set_DVI(EX_OUT_LCDC, &LutInfo);

			}
			break;


		case TCC_LCD_HDMI_PORT_CHANGE_ON:
	#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)
		
			tcc_lcd_interrupt_onoff(0,0);
			
			spin_lock(&port_change_lock);
			tca_lcdc_change_disp_port(0,1);
			spin_unlock(&port_change_lock);		
			
			tca_fb_output_path_change(1);

			tcc_lcd_interrupt_onoff(1,1);		
	#endif
			break;

			
			
		case TCC_LCDC_HDMI_START:
			
			printk(" TCC_LCDC_HDMI_START: \n");			
				
			#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)
			TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_HDMI, 0, 1); //hdmi path init
			#else
			TCC_OUTPUT_FB_DetachOutput(1);
			TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_HDMI, EX_OUT_LCDC, 1);
			#endif
			
			#ifdef CONFIG_EXTEND_DISPLAY_DELAY
			ExtDispStruct.displaydata = &last_sc_info;
			atomic_set(&ExtDispStruct.update_allow, false);
			schedule_delayed_work(&ExtDispStruct.delay_work, msecs_to_jiffies(EXTEND_DISPLAY_DELAY_T));
			#endif//CONFIG_EXTEND_DISPLAY_DELAY
			
			break;

		case TCC_LCDC_HDMI_TIMING:
			{
				
			#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)
				if(Port_change_state)
				{
					EX_OUT_LCDC = tca_get_lcd_lcdc_num();
					LCD_LCDC_NUM = tca_get_output_lcdc_num();
				}
			#endif

				struct lcdc_timimg_parms_t lcdc_timing;
				dprintk(" TCC_LCDC_HDMI_TIMING: \n");

				if (copy_from_user((void*)&lcdc_timing, (const void*)arg, sizeof(struct lcdc_timimg_parms_t)))
					return -EFAULT;

	#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
				printk(" TCC_LCDC_HDMI_TIMING: out %d start %d \n",Output_SelectMode,who_start_vsync);
				
				Output_SelectMode = TCC_OUTPUT_HDMI;
				if(tccvid_vsync.isVsyncRunning && tcc_check_lcdc_enable(LCD_LCDC_NUM))
				{
					printk("HDMI TIMING, by the way LCD and vsync enable \n");
					// interlace video on LCD -> HDMI plug in -> garbage screen debugging
					#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
					if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory &&!tccvid_vsync.interlace_bypass_lcdc)
					{
						printk("and Interlaced \n");
					}
					else
					#endif
					{
						struct tcc_lcdc_image_update ImageInfo;
						memset(&ImageInfo, 0x00, sizeof(struct tcc_lcdc_image_update));
						ImageInfo.enable = 0;
						TCC_LCD_DISPLAY_UPDATE(LCD_LCDC_NUM, &ImageInfo);
					}
				
				}
				
				//VIOC_DISP_SWReset(EX_OUT_LCDC);
				TCC_HDMI_LCDC_Timing(EX_OUT_LCDC, &lcdc_timing);
				TCC_OUTPUT_UPDATE_OnOff(1, TCC_OUTPUT_HDMI);

		#if defined(CONFIG_HWCOMPOSER_OVER_1_1_FOR_MID)
				if(true 
					#if defined(CONFIG_DRAM_16BIT_USED)
					&& (fb_info->fb->var.xres*fb_info->fb->var.yres <= PRESENTATION_LIMIT_RESOLUTION)
					#endif//
				)
				{
					TCC_OUTPUT_RDMA_Update(Output_SelectMode);
					TCC_OUTPUT_LCDC_OutputEnable(EX_OUT_LCDC, 1);
				}
				else
				{
					if(TCC_OUTPUT_FB_Update(fb_info->fb->var.xres, fb_info->fb->var.yres, fb_info->fb->var.bits_per_pixel, Output_BaseAddr, Output_SelectMode) > 0)
					{
						TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
						TCC_OUTPUT_LCDC_OutputEnable(EX_OUT_LCDC, 1);
					}
				}
		#else
				if(TCC_OUTPUT_FB_Update(fb_info->fb->var.xres, fb_info->fb->var.yres, fb_info->fb->var.bits_per_pixel, Output_BaseAddr, Output_SelectMode) > 0)
				{
					TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
					TCC_OUTPUT_LCDC_OutputEnable(EX_OUT_LCDC, 1);
				}
		#endif
				TCC_OUTPUT_FB_MouseShow(0, TCC_OUTPUT_HDMI);

				//spin_lock_irq(&vsync_lock);
				//tccvid_vsync.outputMode = -1;
				//spin_unlock_irq(&vsync_lock);
	#else
				if(lcd_video_started)
				{
					struct tcc_lcdc_image_update ImageInfo;
					
					printk(" TCC_LCDC_HDMI_TIMING => TCC_LCDC_DISPLAY_END \n");

					memset(&ImageInfo, 0x00, sizeof(struct tcc_lcdc_image_update));
					ImageInfo.enable = 0;

					TCC_LCD_DISPLAY_UPDATE(LCD_LCDC_NUM, (struct tcc_lcdc_image_update *)&ImageInfo);					

					lcd_video_started = 0;
				}

				//VIOC_DISP_SWReset(EX_OUT_LCDC);
				TCC_HDMI_LCDC_Timing(EX_OUT_LCDC, &lcdc_timing);				
				TCC_OUTPUT_UPDATE_OnOff(1, TCC_OUTPUT_HDMI);	
			
				Output_SelectMode = TCC_OUTPUT_HDMI;

		#if defined(CONFIG_HWCOMPOSER_OVER_1_1_FOR_MID)
			#if defined(CONFIG_DRAM_16BIT_USED) && defined (CONFIG_ARCH_TCC893X)
				if( fb_info->fb->var.xres*fb_info->fb->var.yres <= PRESENTATION_LIMIT_RESOLUTION) {
					TCC_OUTPUT_LCDC_OutputEnable(EX_OUT_LCDC, 1);
				} else {
					if(TCC_OUTPUT_FB_Update(fb_info->fb->var.xres, fb_info->fb->var.yres, fb_info->fb->var.bits_per_pixel, Output_BaseAddr, Output_SelectMode) > 0)
					{
						TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
						TCC_OUTPUT_LCDC_OutputEnable(EX_OUT_LCDC, 1);
					}
				}
			#else
				#ifdef CONFIG_TCC_HDMI_DRIVER_V1_3
					TCC_OUTPUT_FB_init(Output_SelectMode);			
				#endif//
				TCC_OUTPUT_LCDC_OutputEnable(EX_OUT_LCDC, 1);
			#endif
		#else
			#if defined(CONFIG_ARCH_TCC893X)
			{
				unsigned char hdmi_phy_status = 0;

				
				hdmi_phy_status = hdmi_get_phy_status();
				
				if(hdmi_phy_status) {
					TCC_OUTPUT_LCDC_OutputEnable(EX_OUT_LCDC, 1);
				}
			}
			#else
				if(TCC_OUTPUT_FB_Update(fb_info->fb->var.xres, fb_info->fb->var.yres, fb_info->fb->var.bits_per_pixel, Output_BaseAddr, Output_SelectMode) > 0)
				{
					TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
					TCC_OUTPUT_LCDC_OutputEnable(EX_OUT_LCDC, 1);
				}
			#endif
		#endif

				TCC_OUTPUT_FB_MouseShow(0, TCC_OUTPUT_HDMI);

				#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
					TCC_OUTPUT_FB_RestoreVideoImg(EX_OUT_LCDC);
				#endif
						
		#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT

				if(tccvid_vsync.isVsyncRunning)
					tca_vsync_video_display_enable();

				#if defined(CONFIG_TCC_OUTPUT_AUTO_HDMI_CVBS) || defined(CONFIG_TCC_OUTPUT_ATTACH)
					tcc_vsync_set_firstFrameFlag(1);
				#endif

				spin_lock_irq(&vsync_lock);
				tccvid_vsync.outputMode = -1;
				spin_unlock_irq(&vsync_lock);
		#endif
	#endif
			}
			break;

		case TCC_LCDC_HDMI_UI_UPDATE:
			{
				unsigned char hdmi_phy_status = 0;

				
				hdmi_phy_status = hdmi_get_phy_status();

				if(hdmi_phy_status) {
					if(TCC_OUTPUT_FB_Update(fb_info->fb->var.xres, fb_info->fb->var.yres, fb_info->fb->var.bits_per_pixel, Output_BaseAddr, Output_SelectMode) > 0)
					{
						TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
					}
				}
			}
			break;

		case TCC_LCDC_HDMI_LASTFRAME:
			{
				int res_changed;
				
				if (copy_from_user((void*)&res_changed, (const void*)arg, sizeof(res_changed)))
					return -EFAULT;
				else
				{
					int ret;

					if((ret = tcc_video_last_frame(res_changed)) > 0)
						tccvid_vsync.deinterlace_mode = 0;
					
					return ret;
				}
			}
			break;

		case TCC_LCDC_GET_LAST_FRAME_STATUS:
			if(enabled_LastFrame)
				return 0;
			else
				return -EFAULT;
			break;
			
		case TCC_LCDC_HDMI_DISPLAY:
			{
				struct tcc_lcdc_image_update ImageInfo;
				if (copy_from_user((void *)&ImageInfo, (const void *)arg, sizeof(struct tcc_lcdc_image_update))){
					return -EFAULT;
				}

				dprintk("%s : TCC_LCDC_HDMI_DISPLAY  Output_SelectMode:%d \n", __func__, Output_SelectMode);

				if(Output_SelectMode == TCC_OUTPUT_HDMI)	{
		#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
					// this code prevent a error of transparent on layer0 after stopping v
					if(ImageInfo.Lcdc_layer == 0 && ImageInfo.enable == 0)
					{
						spin_lock_irq(&vsync_lock);
						tccvid_vsync.outputMode = -1;
						spin_unlock_irq(&vsync_lock);
					}
		#endif

		#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
					if(ImageInfo.enable != 0){
						if(LastFrame)
						{
							if( LastFrame_for_ResChanged && (Last_ImageInfo.Frame_width == ImageInfo.Frame_width && Last_ImageInfo.Frame_height == ImageInfo.Frame_height) ){
								return 0;
							}

							if( LastFrame_for_CodecChanged && (Last_ImageInfo.codec_id == ImageInfo.codec_id) ){
								return 0;
							}
							printk("----> TCC_LCDC_HDMI_DISPLAY last-frame cleared : %dx%d, 0x%x \n", ImageInfo.Frame_width, ImageInfo.Frame_height, ImageInfo.addr0);
							LastFrame = LastFrame_for_ResChanged = LastFrame_for_CodecChanged = 0;
						}
						memcpy(&Last_ImageInfo, &ImageInfo, sizeof(struct tcc_lcdc_image_update));
					}
		#endif
        
             #ifdef CONFIG_EXTEND_DISPLAY_DELAY //for video
                                if(!atomic_read(&ExtDispStruct.update_allow))
                                {
                                memcpy(&(last_sc_info.VideoImg), &ImageInfo, sizeof(struct tcc_lcdc_image_update));
                                last_sc_info.Video_updated = false;
                                }
                                else
            #endif
                                {
                                    TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, (struct tcc_lcdc_image_update *)&ImageInfo);
                                }
				}

			}
			break;

		case TCC_LCDC_HDMI_END:
			#ifdef CONFIG_EXTEND_DISPLAY_DELAY
			atomic_set(&ExtDispStruct.update_allow, true);
			cancel_delayed_work_sync(&ExtDispStruct.delay_work);
			#endif //CONFIG_EXTEND_DISPLAY_DELAY
			
			#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
				printk(" TCC_LCDC_HDMI_END: out %d start %d \n",Output_SelectMode,who_start_vsync);
				if(Output_SelectMode == TCC_OUTPUT_HDMI) 
				{
					struct tcc_lcdc_image_update lcdc_image;

					//this check is for the case LCD video - HDMI cable plug out - plug in after short time(before HDMI video) - LCD video
					if(who_start_vsync == LCD_START_VSYNC)
						Output_SelectMode = TCC_OUTPUT_LCD;
					else
						Output_SelectMode = TCC_OUTPUT_NONE;
					
					memset(&lcdc_image, 0x00, sizeof(struct tcc_lcdc_image_update));
					lcdc_image.enable = 0;
					lcdc_image.Lcdc_layer = 2;
					lcdc_image.fmt = TCC_LCDC_IMG_FMT_RGB565;

					TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, &lcdc_image);
					TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_HDMI, EX_OUT_LCDC, 0);
				}

				TCC_OUTPUT_FB_MouseShow(0, TCC_OUTPUT_HDMI);

				tcc_vsync_set_firstFrameFlag(1);
				
				#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
				if(enabled_LastFrame)
				{
					if( pLastFrame_RDMABase != NULL )
						VIOC_RDMA_SetImageDisable(pLastFrame_RDMABase);
					if(onthefly_LastFrame)
					{
						onthefly_LastFrame = 0;
						VIOC_CONFIG_PlugOut(VIOC_SC0);
					}
				}
				#endif

			#else
				printk(" TCC_LCDC_HDMI_END: EX_OUT_LCDC:%d \n", EX_OUT_LCDC);

				if(Output_SelectMode == TCC_OUTPUT_HDMI) 
				{				
					struct tcc_lcdc_image_update lcdc_image;

					Output_SelectMode = TCC_OUTPUT_NONE;
					memset(&lcdc_image, 0x00, sizeof(struct tcc_lcdc_image_update));
					lcdc_image.enable = 0;
					lcdc_image.Lcdc_layer = 2;
					lcdc_image.fmt = TCC_LCDC_IMG_FMT_RGB565;

				#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)	
					// setting for changed hdmi path

					TCC_HDMI_DISPLAY_UPDATE(0, &lcdc_image);	
					TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_HDMI, 0, 0);

					#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
						if(tccvid_vsync.isVsyncRunning)
							tca_vsync_video_display_disable();

						#if !defined(CONFIG_TCC_OUTPUT_AUTO_HDMI_CVBS) && !defined(CONFIG_TCC_OUTPUT_ATTACH)
							tcc_vsync_set_firstFrameFlag(1);
						#endif
					#endif

					EX_OUT_LCDC = tca_get_output_lcdc_num();
					LCD_LCDC_NUM = tca_get_lcd_lcdc_num();

					tcc_lcd_interrupt_onoff(1,0);

					spin_lock(&port_change_lock);
					tca_lcdc_change_disp_port(1,0);
					spin_unlock(&port_change_lock);							

					tca_fb_output_path_change(0);					
										
					tcc_lcd_interrupt_onoff(0,1);

				#else
					#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
						TCC_OUTPUT_FB_BackupVideoImg(EX_OUT_LCDC);
					#endif

					TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, &lcdc_image);
					TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_HDMI, EX_OUT_LCDC, 0);

					#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
						if(tccvid_vsync.isVsyncRunning)
							tca_vsync_video_display_disable();

						#if !defined(CONFIG_TCC_OUTPUT_AUTO_HDMI_CVBS) && !defined(CONFIG_TCC_OUTPUT_ATTACH)
							tcc_vsync_set_firstFrameFlag(1);
						#endif
					#endif
				#endif
				}

				TCC_OUTPUT_LCDC_OutputEnable(EX_OUT_LCDC, 0);

				TCC_OUTPUT_FB_MouseShow(0, TCC_OUTPUT_HDMI);			
				
				#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
					if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory &&!tccvid_vsync.interlace_bypass_lcdc)
						TCC_VIQE_DI_DeInit60Hz();
				#endif

				#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
				if(enabled_LastFrame)
				{
					if( pLastFrame_RDMABase != NULL )
						VIOC_RDMA_SetImageDisable(pLastFrame_RDMABase);
					if(onthefly_LastFrame)
					{
						onthefly_LastFrame = 0;
						VIOC_CONFIG_PlugOut(VIOC_SC0);
					}
				}
				#endif			

			#endif
			break;

		case TCC_LCDC_HDMI_CHECK:
			{
				unsigned int ret_mode = 0;
				#if defined(CONFIG_MACH_TCC8920ST)
				if(fb_power_state == 0 || HDMI_pause || ((screen_width < screen_height)&& (!HDMI_video_mode)))
				#else
				if(HDMI_pause || ((screen_width < screen_height)&& (!HDMI_video_mode)))
				#endif
				{
					ret_mode = 1;
					dprintk("\n %d : %d %d  \n ", HDMI_pause, screen_width, screen_height);
				}

				put_user(ret_mode, (unsigned int __user*)arg);
			}
			break;

		case TCC_LCDC_HDMI_MODE_SET:
 			{
				TCC_HDMI_M uiHdmi;

				if(get_user(uiHdmi, (int __user *) arg))
					return -EFAULT;

				dprintk("%s: TCC_LCDC_HDMI_MODE_SET [%d] video_M:%d Output_SelectMode:%d   \n", __func__ , uiHdmi , HDMI_video_mode, Output_SelectMode);

				switch(uiHdmi)
				{
					case TCC_HDMI_SUSEPNED:
						HDMI_pause = 1;
						break;
					case TCC_HDMI_RESUME:
						HDMI_pause = 0;
						break;
					case TCC_HDMI_VIDEO_START:
						HDMI_video_mode = 1;
						break;
					case TCC_HDMI_VIDEO_END:
						HDMI_video_mode = 0;
			#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
						LastFrame = LastFrame_for_ResChanged = LastFrame_for_CodecChanged = 0;
			#endif
			#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
						if(enabled_LastFrame)
						{
							if( pLastFrame_RDMABase != NULL )
								VIOC_RDMA_SetImageDisable(pLastFrame_RDMABase);
							if(onthefly_LastFrame)
							{
								onthefly_LastFrame = 0;
								VIOC_CONFIG_PlugOut(VIOC_SC0);
							}
						}
			#endif
						break;
					default:
						break;
				}
 			}
			break;
			
		case TCC_LCDC_HDMI_GET_SIZE:
			{
				tcc_display_size HdmiSize;
				HdmiSize.width = HDMI_video_width;
				HdmiSize.height = HDMI_video_height;
  				HdmiSize.frame_hz= HDMI_video_hz;

				dprintk("%s: TCC_LCDC_HDMI_GET_SIZE -  HDMI_video_width:%d HDMI_video_height:%d   \n", __func__ , HDMI_video_width, HDMI_video_height);
				if (copy_to_user((tcc_display_size *)arg, &HdmiSize, sizeof(HdmiSize)))		{
					return -EFAULT;
				}
			}
			break;

		case TCC_LCDC_SET_OUTPUT_RESIZE_MODE:
			{
				tcc_display_resize mode;

				if(copy_from_user((void *)&mode, (const void *)arg, sizeof(tcc_display_resize)))
					return -EFAULT;

				//printk("%s : TCC_LCDC_SET_OUTPUT_RESIZE_MODE, mode=%d\n", __func__, mode);

				TCC_OUTPUT_SetOutputResizeMode(mode);

				if(TCC_OUTPUT_FB_Update(fb_info->fb->var.xres, fb_info->fb->var.yres, fb_info->fb->var.bits_per_pixel, Output_BaseAddr, Output_SelectMode) > 0)
				{
					TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
				}
			}
			break;

		case TCC_LCDC_HDMI_SET_SIZE:
			{
				tcc_display_size HdmiSize;
				if (copy_from_user((void *)&HdmiSize, (const void *)arg, sizeof(tcc_display_size)))
					return -EFAULT;

				HDMI_video_width = HdmiSize.width;
				HDMI_video_height = HdmiSize.height;
				HDMI_video_hz = HdmiSize.frame_hz;

				if(HDMI_video_hz == 23)
					HDMI_video_hz = 24;
				else if(HDMI_video_hz == 29)
					HDMI_video_hz = 30;

				dprintk("%s: TCC_LCDC_HDMI_SET_SIZE -  HDMI_video_width:%d HDMI_video_height:%d   \n", __func__ , HDMI_video_width, HDMI_video_height);
			}
			break;

		case TCC_LCDC_COMPOSITE_CHECK:
			{
				unsigned int composite_detect = 1;

				#if defined(CONFIG_FB_TCC_COMPOSITE)
					composite_detect = tcc_composite_detect();
				#endif

				if (copy_to_user((void *)arg, &composite_detect, sizeof(unsigned int)))
					return -EFAULT;
			}
			break;
			
		case TCC_LCDC_COMPOSITE_MODE_SET:
			{
				LCDC_COMPOSITE_MODE composite_mode;
				if(copy_from_user((void *)&composite_mode, (const void *)arg, sizeof(LCDC_COMPOSITE_MODE))){
					return -EFAULT;
				}

				if(composite_mode == LCDC_COMPOSITE_UI_MODE)
				{
					Output_SelectMode = TCC_OUTPUT_COMPOSITE;

 					printk("TCC_LCDC_COMPOSITE_MODE_SET : Output_SelectMode = %d , composite_mode = %d\n", Output_SelectMode, composite_mode);

					#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
						TCC_OUTPUT_FB_ClearVideoImg();
					#endif

					if(TCC_OUTPUT_FB_Update(fb_info->fb->var.xres, fb_info->fb->var.yres, fb_info->fb->var.bits_per_pixel, Output_BaseAddr, Output_SelectMode) > 0)
					{
						TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);

						#if defined(CONFIG_TCC_OUTPUT_AUTO_HDMI_CVBS) || defined(CONFIG_TCC_OUTPUT_ATTACH)
							#if !defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
								tca_lcdc_interrupt_onoff(TRUE, EX_OUT_LCDC);
							#endif
						#endif

						TCC_OUTPUT_LCDC_OutputEnable(EX_OUT_LCDC, 1);
					}

					TCC_OUTPUT_FB_MouseShow(1, TCC_OUTPUT_COMPOSITE);

					#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
						if(tccvid_vsync.isVsyncRunning)
							tca_vsync_video_display_enable();

						#if defined(CONFIG_TCC_OUTPUT_AUTO_HDMI_CVBS) || defined(CONFIG_TCC_OUTPUT_ATTACH)
							tcc_vsync_set_firstFrameFlag(1);
						#endif

						spin_lock_irq(&vsync_lock);
						tccvid_vsync.outputMode = -1;
						spin_unlock_irq(&vsync_lock);
					#endif
				}
				else if(composite_mode == LCDC_COMPOSITE_NONE_MODE)
				{
					Output_SelectMode = TCC_OUTPUT_NONE;

					#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
						if(tccvid_vsync.isVsyncRunning)
							tca_vsync_video_display_disable();

						#if !defined(CONFIG_TCC_OUTPUT_AUTO_HDMI_CVBS) && !defined(CONFIG_TCC_OUTPUT_ATTACH)
							tcc_vsync_set_firstFrameFlag(1);
						#endif
					#endif

					#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
						if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory &&!tccvid_vsync.interlace_bypass_lcdc)
							TCC_VIQE_DI_DeInit60Hz();
					#endif
				}
			}
			break;

		case TCC_LCDC_COMPONENT_CHECK:
			{
				unsigned int component_detect;

				#if defined(CONFIG_FB_TCC_COMPONENT)
					component_detect = tcc_component_detect();
				#endif

				if (copy_to_user((void *)arg, &component_detect, sizeof(unsigned int)))
					return -EFAULT;
			}
			break;
			
		case TCC_LCDC_COMPONENT_MODE_SET:
			{
				LCDC_COMPONENT_MODE component_mode;
				if(copy_from_user((void *)&component_mode, (const void *)arg, sizeof(LCDC_COMPONENT_MODE))){
					return -EFAULT;
				}

				if(component_mode == LCDC_COMPONENT_UI_MODE)
				{
					Output_SelectMode = TCC_OUTPUT_COMPONENT;

 					printk("TCC_LCDC_COMPONENT_MODE_SET : Output_SelectMode = %d , component_mode = %d\n", Output_SelectMode, component_mode);

					#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
						TCC_OUTPUT_FB_ClearVideoImg();
					#endif

					if(TCC_OUTPUT_FB_Update(fb_info->fb->var.xres, fb_info->fb->var.yres, fb_info->fb->var.bits_per_pixel, Output_BaseAddr, Output_SelectMode) > 0)
					{
						TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);

						#if defined(CONFIG_TCC_OUTPUT_AUTO_HDMI_CVBS) || defined(CONFIG_TCC_OUTPUT_ATTACH)
							#if !defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
								tca_lcdc_interrupt_onoff(TRUE, EX_OUT_LCDC);
							#endif
						#endif

						TCC_OUTPUT_LCDC_OutputEnable(EX_OUT_LCDC, 1);
					}

					TCC_OUTPUT_FB_MouseShow(1, TCC_OUTPUT_COMPONENT);

					#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
						if(tccvid_vsync.isVsyncRunning)
							tca_vsync_video_display_enable();

						#if defined(CONFIG_TCC_OUTPUT_AUTO_HDMI_CVBS) || defined(CONFIG_TCC_OUTPUT_ATTACH)
							tcc_vsync_set_firstFrameFlag(1);
						#endif

						spin_lock_irq(&vsync_lock);
						tccvid_vsync.outputMode = -1;
						spin_unlock_irq(&vsync_lock);
					#endif
				}
				else if(component_mode == LCDC_COMPONENT_NONE_MODE)
				{
					Output_SelectMode = TCC_OUTPUT_NONE;

					#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
						if(tccvid_vsync.isVsyncRunning)
							tca_vsync_video_display_disable();

						#if !defined(CONFIG_TCC_OUTPUT_AUTO_HDMI_CVBS) && !defined(CONFIG_TCC_OUTPUT_ATTACH)
							tcc_vsync_set_firstFrameFlag(1);
						#endif
					#endif

					#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
						if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory &&!tccvid_vsync.interlace_bypass_lcdc)
							TCC_VIQE_DI_DeInit60Hz();
					#endif
				}
			}
			break;

		case TCC_LCDC_FBCHANNEL_ONOFF:
			{
				unsigned int onOff;
			
				if(copy_from_user((void *)&onOff, (const void *)arg, sizeof(unsigned int)))
					return -EFAULT;

				#if defined(CONFIG_MACH_TCC8930ST) || defined(CONFIG_MACH_TCC8920ST) 
					#if !defined(CONFIG_TCC_OUTPUT_ATTACH)
						TCC_OUTPUT_FB_RDMA_OnOff(onOff, Output_SelectMode);
					#endif
				#endif
			}
			break;
			
		case TCC_LCDC_MOUSE_SHOW:
			{
				unsigned int enable;

				if(copy_from_user((void *)&enable, (const void *)arg, sizeof(unsigned int)))
					return -EFAULT;
				TCC_OUTPUT_FB_MouseShow(enable, Output_SelectMode);
			}
			break;
		case TCC_LCDC_MOUSE_MOVE:
			{
				tcc_mouse mouse;
				if (copy_from_user((void *)&mouse, (const void *)arg, sizeof(tcc_mouse)))
					return -EFAULT;
				TCC_OUTPUT_FB_MouseMove(fb_info->fb->var.xres, fb_info->fb->var.yres, &mouse, Output_SelectMode);
			}
			break;
		case TCC_LCDC_MOUSE_ICON:
			{
				tcc_mouse_icon mouse_icon;
				if (copy_from_user((void *)&mouse_icon, (const void *)arg, sizeof(tcc_mouse_icon)))
					return -EFAULT;
				TCC_OUTPUT_FB_MouseSetIcon(&mouse_icon);
			}
			break;

		case TCC_LCDC_3D_UI_ENABLE:
			{
				unsigned int mode;

				if(copy_from_user((void *)&mode, (const void *)arg, sizeof(unsigned int)))
					return -EFAULT;

				TCC_OUTPUT_FB_Set3DMode(TRUE, mode);
				if(TCC_OUTPUT_FB_Update(fb_info->fb->var.xres, fb_info->fb->var.yres, fb_info->fb->var.bits_per_pixel, Output_BaseAddr, Output_SelectMode) > 0)
				{
					TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
				}
			}
			break;
			
		case TCC_LCDC_3D_UI_DISABLE:
			{
				TCC_OUTPUT_FB_Set3DMode(FALSE, 0);
				if(TCC_OUTPUT_FB_Update(fb_info->fb->var.xres, fb_info->fb->var.yres, fb_info->fb->var.bits_per_pixel, Output_BaseAddr, Output_SelectMode) > 0)
				{
					TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
				}
			}
			break;
			
		case TCC_LCDC_ATTACH_SET_STATE:
			{
				unsigned int attach_state;

				if(copy_from_user((void *)&attach_state, (const void *)arg, sizeof(unsigned int)))
					return -EFAULT;

				#if defined(CONFIG_TCC_OUTPUT_ATTACH)
					TCC_OUTPUT_FB_AttachSetSate(attach_state);
				#endif
			}
			break;
			
		case TCC_LCDC_ATTACH_GET_STATE:
			{
				unsigned int attach_state = 0;

				#if defined(CONFIG_TCC_OUTPUT_ATTACH)
					attach_state = TCC_OUTPUT_FB_AttachGetSate();
				#endif

				if (copy_to_user((void *)arg, &attach_state, sizeof(unsigned int)))
					return -EFAULT;
			}
			break;
			
		case TCC_LCDC_SET_DISPLAY_TYPE:
			{
			}
			break;
			
		case TCC_LCDC_GET_DISPLAY_TYPE:
			{
				unsigned int display_type = 0; /* Support HDMI/CVBS/COMPONENT output */

				#if defined(CONFIG_HDB892F_BOARD_YJ8925T) || defined(CONFIG_STB_BOARD_YJ8935T) || defined(CONFIG_STB_BOARD_YJ8933T) || defined(CONFIG_STB_BOARD_YJ8930T)
					display_type = 1; /* Support HDMI/CVBS output */
				#endif
				
				#if defined(CONFIG_STB_BOARD_UPC) || defined(CONFIG_STB_BOARD_UPC_TCC893X)
					display_type = 2; /* Support HDMI output */
				#endif
				
				#if defined(CONFIG_STB_BOARD_DONGLE) || defined(CONFIG_STB_BOARD_DONGLE_TCC893X)
					display_type = 2; /* Support HDMI output */
				#endif

				if (copy_to_user((void *)arg, &display_type, sizeof(unsigned int)))
					return -EFAULT;

				TCC_OUTPUT_FB_DetachOutput(1);
			}
			break;

	#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
	case TCC_LCDC_VIDEO_START_VSYNC:
		{
			int backup_time;
			int backup_frame_rate;
			struct tcc_lcdc_image_update input_image;

			#if defined(CONFIG_TCC_OUTPUT_AUTO_HDMI_CVBS) || defined(CONFIG_TCC_OUTPUT_ATTACH)
				if(Output_SelectMode == TCC_OUTPUT_NONE)
				{
					//printk("### Error: there is no valid output on STB\n");
					return -EFAULT;
				}
			#endif

			printk("\n### TCC_LCDC_VIDEO_START_VSYNC \n");

			if (copy_from_user((void *)&input_image , (const void *)arg, sizeof(struct tcc_lcdc_image_update)))
			{
				printk("fatal error") ;
				return -EFAULT;
			}
			
			#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
			if(!input_image.ex_output){
				who_start_vsync = LCD_START_VSYNC;
				Output_SelectMode = TCC_OUTPUT_LCD;
			}
			else{
				Output_SelectMode = TCC_OUTPUT_HDMI;
				who_start_vsync = HDMI_START_VSYNC;
			}
			#endif
			
	#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
			spin_lock_irq(&LastFrame_lockDisp);
			if(LastFrame && LastFrame_for_ResChanged == 0 && LastFrame_for_CodecChanged == 0){
				LastFrame = 0;
				printk("----> LastFrame = 0 in TCC_LCDC_VIDEO_START_VSYNC \n");
			}
			spin_unlock_irq(&LastFrame_lockDisp);
	#endif

			if(!fb_power_state)
			{
				printk("##### Error ### vsync start\n");				
				return -1;
			}
			if(vsync_started == 0)
			{
				backup_time = tccvid_vsync.nTimeGapToNextField; 
				backup_frame_rate = tccvid_vsync.video_frame_rate;
				memset( &tccvid_vsync, 0, sizeof( tccvid_vsync ) ) ; 
				tccvid_vsync.isVsyncRunning = 1;
				tccvid_vsync.overlayUsedFlag = -1;
				tccvid_vsync.outputMode = -1;
				tccvid_vsync.firstFrameFlag = 1;
				tccvid_vsync.deinterlace_mode= -1;
				tccvid_vsync.m2m_mode = -1;
				tccvid_vsync.output_toMemory = -1;
				tccvid_vsync.mvcMode = 0;
				tccvid_vsync.nTimeGapToNextField = backup_time;
				tccvid_vsync.video_frame_rate = backup_frame_rate;
				viqe_render_init();
				if(backup_time)
					tccvid_vsync.updateGapTime = backup_time;
				else
					tccvid_vsync.updateGapTime = 16;

				if(Output_SelectMode == TCC_OUTPUT_HDMI && HDMI_video_hz != 0)
				{
					tccvid_vsync.vsync_interval = (1000/HDMI_video_hz);
				
					if( (tccvid_vsync.video_frame_rate > 0) 
						&& (HDMI_video_hz >= tccvid_vsync.video_frame_rate) 
						&& ((HDMI_video_hz % tccvid_vsync.video_frame_rate) == 0)
					)
						tccvid_vsync.perfect_vsync_flag = 1;
					else
						tccvid_vsync.perfect_vsync_flag = 0;						
				}
				else
				{
					tccvid_vsync.vsync_interval = (1000/60);
				}
				printk("vsync_interval (%d), perfect_flag(%d)\n", tccvid_vsync.vsync_interval, tccvid_vsync.perfect_vsync_flag);
				spin_lock_init(&vsync_lock);
				spin_lock_init(&vsync_lockDisp);
#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
				spin_lock_init(&LastFrame_lockDisp);
#endif
#ifdef USE_VSYNC_TIMER
				tccfb_vsync_timer_onoff(1);
				msleep(0);
#endif

				spin_lock_irq(&vsync_lock) ;
				tcc_vsync_set_time(0);
				spin_unlock_irq(&vsync_lock) ;

				tca_vsync_video_display_enable();

				tcc_vsync_set_max_buffer(&tccvid_vsync.vsync_buffer, input_image.max_buffer);

				printk("### TCC_LCDC_VIDEO_START_VSYNC max %d ex_out %d \n", input_image.max_buffer,input_image.ex_output);

				vsync_output_mode = Output_SelectMode;

				vsync_started = 1;
			}
		}
		break ;

	case TCC_LCDC_VIDEO_END_VSYNC:
		{
			printk("\nTCC_LCDC_VIDEO_END_VSYNC fb_power_state:%d vsync_started:%d Output_SelectMode:%d vsync_output_mode:%d\n", fb_power_state, vsync_started, Output_SelectMode, vsync_output_mode);
			if(vsync_started == 1)
			{
#ifdef USE_VSYNC_TIMER
				tccfb_vsync_timer_onoff(0);
#endif

				tca_vsync_video_display_disable();			
				tccvid_vsync.skipFrameStatus = 1;
				tccvid_vsync.nTimeGapToNextField = 0;
				tccvid_vsync.isVsyncRunning = 0;

				if(!enabled_LastFrame 
					|| (enabled_LastFrame && (Last_ImageInfo.outputMode != Output_SelectMode-TCC_OUTPUT_LCD)) // Play Interlaced contents => HDMI plug in or out => will be displayed broken image.
				)
				{
					{
						struct tcc_lcdc_image_update ImageInfo;
						
					#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
						if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory &&!tccvid_vsync.interlace_bypass_lcdc)
							TCC_VIQE_DI_DeInit60Hz();
					#endif

						memset(&ImageInfo, 0x00, sizeof(struct tcc_lcdc_image_update));
						ImageInfo.Lcdc_layer = 2;
						ImageInfo.enable = 0;
				
						if(vsync_output_mode == TCC_OUTPUT_HDMI){
							#ifdef MVC_PROCESS
								if(TCC_OUTPUT_FB_GetMVCStatus() && tccvid_vsync.mvcMode==1)
									TCC_HDMI_DISPLAY_UPDATE_3D(EX_OUT_LCDC, (struct tcc_lcdc_image_update *)&ImageInfo);
								else
							#endif
									TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, (struct tcc_lcdc_image_update *)&ImageInfo);
						}
						else if(vsync_output_mode == TCC_OUTPUT_COMPOSITE){
							#if defined(CONFIG_FB_TCC_COMPOSITE)
								tcc_composite_update((struct tcc_lcdc_image_update *)&ImageInfo);
							#endif
						}
						else if(vsync_output_mode == TCC_OUTPUT_COMPONENT){
							#if defined(CONFIG_FB_TCC_COMPONENT)
								tcc_component_update((struct tcc_lcdc_image_update *)&ImageInfo);
							#endif
						}
						#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
						else if(vsync_output_mode == TCC_OUTPUT_LCD){
							TCC_LCD_DISPLAY_UPDATE(LCD_LCDC_NUM, (struct tcc_lcdc_image_update *)&ImageInfo);
						}
						#endif
					}

					tccvid_vsync.deinterlace_mode = 0;
				}

				/*
				if(Output_SelectMode == TCC_OUTPUT_NONE)
				{
					struct tcc_lcdc_image_update ImageInfo;
					memset(&ImageInfo, 0x00, sizeof(struct tcc_lcdc_image_update));
					TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, &ImageInfo);
				}
				*/
				vsync_started = 0;
			}

			if(!enabled_LastFrame)
				Last_ImageInfo.enable = 0;

			CurrDisplayingImage.enable = 0;
		}
		break ;

	case TCC_LCDC_VIDEO_SET_SIZE_CHANGE:
		{
			printk("### Display size is changed, firstFrame(%d) deint_mode(%d) \n", tccvid_vsync.firstFrameFlag, tccvid_vsync.deinterlace_mode);

			//if(tccvid_vsync.firstFrameFlag == 0)
			{
				spin_lock_irq(&vsync_lock) ;
				// have to add code about pop all buffer.
				tcc_vsync_pop_all_buffer(&tccvid_vsync.vsync_buffer);
				printk("### vsync pop all buffer success!\n");
				
				spin_unlock_irq(&vsync_lock) ;
			}
		}
		break ;

	case TCC_LCDC_VIDEO_SET_FRAMERATE:
		tcc_video_set_framerate((int)arg);
		break ;

	case TCC_LCDC_VIDEO_SET_MVC_STATUS:
		#if defined(MVC_PROCESS)
		TCC_OUTPUT_FB_SetMVCStatus((int)arg);
		#endif
		break ;

	case TCC_LCDC_VIDEO_PUSH_VSYNC:
		if(!vsync_started)
			return 0;

		//if(Output_SelectMode == TCC_OUTPUT_NONE || Output_SelectMode == TCC_OUTPUT_HDMI || Output_SelectMode == TCC_OUTPUT_COMPONENT || Output_SelectMode == TCC_OUTPUT_COMPOSITE)
		{
			struct tcc_lcdc_image_update input_image;
			int error_type = 0;
			int check_time;

			if (copy_from_user((void *)&input_image , (const void *)arg, sizeof(struct tcc_lcdc_image_update)))
			{
				printk("fatal error") ;
				return 0;
			}

			#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
			{
				VIOC_DISP *pDISPBase;
				unsigned int output_lcdc_num = 0;
				unsigned int lcd_width, lcd_height;

				output_lcdc_num = tca_get_output_lcdc_num();

	            if (output_lcdc_num)
					pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
	            else
					pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);

				VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);

				if(!lcd_width || !lcd_height || !VIOC_DISP_Get_TurnOnOff(pDISPBase))
				{
					spin_lock_irq(&vsync_lockDisp);
					tccvid_vsync.vsync_buffer.last_cleared_buff_id = input_image.buffer_unique_id;
					spin_unlock_irq(&vsync_lockDisp);

					return tccvid_vsync.vsync_buffer.writeIdx;
				}
			}
			#endif

	#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
			spin_lock_irq(&LastFrame_lockDisp);
			if(LastFrame)
			{
				if( LastFrame_for_ResChanged && (Last_ImageInfo.Frame_width == input_image.Frame_width && Last_ImageInfo.Frame_height == input_image.Frame_height) )
				{
					spin_unlock_irq(&LastFrame_lockDisp);
					return 0;
				}
				if( LastFrame_for_CodecChanged && (Last_ImageInfo.codec_id == input_image.codec_id) )
				{
					spin_unlock_irq(&LastFrame_lockDisp);
					return 0;
				}
				printk("----> PUSH_VSYNC :: last-frame cleared : %dx%d, 0x%x \n", input_image.Frame_width, input_image.Frame_height, input_image.addr0);
				LastFrame = LastFrame_for_ResChanged = LastFrame_for_CodecChanged = 0;
			}
			spin_unlock_irq(&LastFrame_lockDisp);
	#endif

			if(0)
			{
				printk("## Frame %d x %d , Image %d x %d, start x:%d y:%d, Addr %x\n", input_image.Frame_width, input_image.Frame_height, input_image.Image_width, input_image.Image_height, input_image.offset_x, input_image.offset_y, input_image.addr0);

				printk("### push valid(%d), max(%d), vtime(%d), ctime(%d), buff_id(%d)\n",
							tccvid_vsync.vsync_buffer.valid_buff_count,
							tccvid_vsync.vsync_buffer.max_buff_num,
							input_image.time_stamp, input_image.sync_time, input_image.buffer_unique_id);
			}

			if(0)//(!input_image.output_path && input_image.buffer_unique_id<3)
			{
				printk("push v(%d), c(%d), id(%d)\n",input_image.time_stamp, input_image.sync_time, input_image.buffer_unique_id);

				spin_lock_irq(&vsync_lockDisp) ;
				tccvid_vsync.vsync_buffer.last_cleared_buff_id = input_image.buffer_unique_id;
				spin_unlock_irq(&vsync_lockDisp) ;
				return tccvid_vsync.vsync_buffer.writeIdx;
			}
			
			if(!tccvid_vsync.isVsyncRunning )
			{
				printk("vsync already ended !! %d buffer_unique_id %d \n",input_image.time_stamp, input_image.buffer_unique_id);
				
				spin_lock_irq(&vsync_lockDisp) ;
				tccvid_vsync.vsync_buffer.last_cleared_buff_id = input_image.buffer_unique_id;
				spin_unlock_irq(&vsync_lockDisp) ;
				goto TCC_VSYNC_PUSH_ERROR;
				//input_image.time_stamp = 0;
				//return 0;
			}

	#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
			memcpy(&Last_ImageInfo, &input_image, sizeof(struct tcc_lcdc_image_update));
	#endif	

			spin_lock_irq(&vsync_lock) ;
			check_time = abs(tcc_vsync_get_time() - input_image.sync_time);
			spin_unlock_irq(&vsync_lock) ;

			if(check_time> 200)
			{
				#ifdef USE_VSYNC_TIMER
				vprintk("reset time base time %d kernel time %d time %d \n",tccvid_vsync.baseTime,tcc_vsync_get_timer_clock(),input_image.sync_time);
				#endif
				tccfb_ResetSyncTime(0);
			}
			
			tccfb_calculateSyncTime(input_image.sync_time);

			if(tccvid_vsync.outputMode < 0)
			{
				switch(input_image.outputMode)
				{
					case OUTPUT_NONE:
						#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
						tccvid_vsync.outputMode = TCC_OUTPUT_LCD;
						#else
						tccvid_vsync.outputMode = TCC_OUTPUT_NONE;			
						#endif
						break;
					case OUTPUT_HDMI:
						tccvid_vsync.outputMode = TCC_OUTPUT_HDMI;
						tcc_check_interlace_output(tccvid_vsync.outputMode);
						break;
					case OUTPUT_COMPOSITE:
						tccvid_vsync.outputMode = TCC_OUTPUT_COMPOSITE; 		
						tcc_check_interlace_output(tccvid_vsync.outputMode);
						break;
					case OUTPUT_COMPONENT:
						tccvid_vsync.outputMode = TCC_OUTPUT_COMPONENT; 		
						tcc_check_interlace_output(tccvid_vsync.outputMode);
						break;
					default:
						tccvid_vsync.outputMode = TCC_OUTPUT_NONE;									
				}
				
				printk("tccvid_vsync.outputMode %d input_image.outputMode %d %d \n", tccvid_vsync.outputMode ,input_image.outputMode,Output_SelectMode);			
			}

			if(tccvid_vsync.deinterlace_mode < 0)
			{
				unsigned int lcdCtrlNum, lcd_width, lcd_height;
				VIOC_DISP * pDISPBase;
						
				#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
				if((tccvid_vsync.outputMode == TCC_OUTPUT_LCD))
				#else
				if((tccvid_vsync.outputMode == TCC_OUTPUT_NONE))
				#endif
					lcdCtrlNum = LCD_LCDC_NUM;
				else
					lcdCtrlNum = EX_OUT_LCDC; 
				
				/* Set the output resolution */
				if (lcdCtrlNum)
					pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
				else
					pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);

				VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);
				
				tccvid_vsync.deinterlace_mode = input_image.deinterlace_mode;
				tccvid_vsync.frameInfo_interlace = input_image.frameInfo_interlace;
				tccvid_vsync.output_toMemory = input_image.output_toMemory;
				printk("### deinterlace_mode(%d), output_toMemory(%d)\n", tccvid_vsync.deinterlace_mode, tccvid_vsync.output_toMemory);
				
				tccvid_vsync.interlace_bypass_lcdc = 0;
				tccvid_vsync.mvcMode = input_image.MVCframeView;
			
				if( (tccvid_vsync.deinterlace_mode == 1) && 
					(tccvid_vsync.interlace_output == 1) && 
					(tccvid_vsync.output_toMemory == 0) &&
					(input_image.Frame_width == input_image.Image_width) && (input_image.Frame_height == input_image.Image_height) )
				{
					printk("### interlace_bypass_lcdc set !!\n");
					tccvid_vsync.interlace_bypass_lcdc = 1;
				}

			#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV) || defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
				if( (tccvid_vsync.outputMode == TCC_OUTPUT_COMPOSITE) && 
					(tccvid_vsync.deinterlace_mode == 1) && 
					(tccvid_vsync.interlace_output == 1) && 
					(tccvid_vsync.output_toMemory == 0) &&
					(input_image.Frame_width == input_image.Image_width) )
				{
					if(tccvid_vsync.interlace_bypass_lcdc == 0)
					{
						printk("### interlace_bypass_lcdc set for testing composite signal\n");
						tccvid_vsync.interlace_bypass_lcdc = 1;
					}
				}

				#if defined(CONFIG_FB_TCC_COMPOSITE)
					tcc_composite_set_bypass(tccvid_vsync.interlace_bypass_lcdc);
				#endif
			#endif
			}

		#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV) || defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
			if((tccvid_vsync.outputMode == TCC_OUTPUT_COMPOSITE) && (tccvid_vsync.interlace_bypass_lcdc))
			{
				if(input_image.Frame_width <= input_image.Image_width)
					input_image.Image_width = input_image.Frame_width;
			}
		#endif
			
#ifdef TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
			if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory && tccvid_vsync.firstFrameFlag &&!tccvid_vsync.interlace_bypass_lcdc)
			{
				int lcdCtrlNum;
				printk("first TCC_excuteVIQE_60Hz \n") ;

				
				#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
				if((tccvid_vsync.outputMode == TCC_OUTPUT_LCD))
				#else
				if((tccvid_vsync.outputMode == TCC_OUTPUT_NONE))
				#endif
					lcdCtrlNum = LCD_LCDC_NUM;
				else
					lcdCtrlNum = EX_OUT_LCDC;	

				tccvid_vsync.nDeinterProcCount = 0;
				tccvid_vsync.m2m_mode =  input_image.m2m_mode;

				TCC_VIQE_DI_Init60Hz(Output_SelectMode, lcdCtrlNum, input_image.Lcdc_layer, input_image.on_the_fly, input_image.fmt, 
										input_image.Frame_width, input_image.Frame_height,	// srcWidth, srcHeight
										input_image.Image_width, input_image.Image_height,
										input_image.offset_x, input_image.offset_y,
										input_image.odd_first_flag);
			}
#endif
			
			tccvid_vsync.firstFrameFlag = 0;

			#if defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
			{
				#if defined(CONFIG_MACH_TCC8930ST)
				{
					VIOC_DISP * pDISPBase;
					unsigned int output_lcdc_num = 0;

			
					output_lcdc_num = tca_get_output_lcdc_num();
					
					if (output_lcdc_num)
						pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
					else
						pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);

					//if(tccvid_vsync.interlace_bypass_lcdc)
					if( !(pDISPBase->uCTRL.nREG & HwDISP_NI )) {//interlace mode
						input_image.on_the_fly = 0;
					}
				}
				#endif
			}
			#endif
			
			// This is for Display underrun issue.
			if(!tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory )
			{
				if(tccvid_vsync.outputMode == Output_SelectMode )
				{
					if(tccvid_vsync.outputMode == TCC_OUTPUT_HDMI)
						TCC_HDMI_SET_OnTheFly(EX_OUT_LCDC,&input_image);
					#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
					else if(tccvid_vsync.outputMode == TCC_OUTPUT_LCD)
						TCC_LCD_SET_OnTheFly(LCD_LCDC_NUM,&input_image);
					#endif
				}
			}

			if(input_image.output_path)
			{
			
			//printk("### output_path buffer_unique_id %d \n",input_image.buffer_unique_id);
				spin_lock_irq(&vsync_lockDisp) ;
				tcc_vsync_pop_all_buffer(&tccvid_vsync.vsync_buffer);
				tccvid_vsync.vsync_buffer.last_cleared_buff_id = input_image.buffer_unique_id;
				spin_unlock_irq(&vsync_lockDisp) ;

				if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory && !tccvid_vsync.interlace_bypass_lcdc)		//tccvid_vsync.deinterlace_mode && //
				{
					tccvid_vsync.nDeinterProcCount = 0;
					
					switch(Output_SelectMode)
					{
						case TCC_OUTPUT_NONE:
							break;
						#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
						case TCC_OUTPUT_LCD:
							{
								TCC_VIQE_DI_Run60Hz(input_image.on_the_fly, input_image.addr0, input_image.addr1, input_image.addr2,
													input_image.Frame_width, input_image.Frame_height,
													input_image.crop_top,input_image.crop_bottom, input_image.crop_left, input_image.crop_right, 
													input_image.Image_width, input_image.Image_height, 
													input_image.offset_x, input_image.offset_y, input_image.odd_first_flag, input_image.frameInfo_interlace, 0);
							}
							break;
						#endif
						case TCC_OUTPUT_HDMI:
						case TCC_OUTPUT_COMPONENT:
						case TCC_OUTPUT_COMPOSITE:
						#ifdef CONFIG_EXTEND_DISPLAY_DELAY //for video 
							if(!atomic_read(&ExtDispStruct.update_allow))
							{
								memcpy(&(last_sc_info.VideoImg), &input_image, sizeof(struct tcc_lcdc_image_update));
								last_sc_info.Video_updated = false;
							}
							else
						#endif
							{
								TCC_VIQE_DI_Run60Hz(input_image.on_the_fly, input_image.addr0, input_image.addr1, input_image.addr2,
													input_image.Frame_width, input_image.Frame_height,
													input_image.crop_top,input_image.crop_bottom, input_image.crop_left, input_image.crop_right, 
													input_image.Image_width, input_image.Image_height, 
													input_image.offset_x, input_image.offset_y, input_image.odd_first_flag, input_image.frameInfo_interlace, 0);
							}
							break;
							
						default:
							break;
					}
					
				}
				else if(tccvid_vsync.interlace_bypass_lcdc){
					tccvid_vsync.nDeinterProcCount =0;
					if(byPassImageToLCDC(&input_image, 0, EX_OUT_LCDC) == 1){
						//tccvid_vsync.nDeinterProcCount =0;
						//return;
					}
				}
				else
				{
					
					switch(Output_SelectMode)
					{
						case TCC_OUTPUT_NONE:
							//TCC_HDMI_DISPLAY_UPDATE(LCD_LCDC_NUM, &input_image);
							break;
						#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
						case TCC_OUTPUT_LCD:
							TCC_LCD_DISPLAY_UPDATE(LCD_LCDC_NUM, &input_image);
							break;
						#endif
						case TCC_OUTPUT_HDMI:
							#ifdef CONFIG_EXTEND_DISPLAY_DELAY //for video 
							if(!atomic_read(&ExtDispStruct.update_allow))
							{
								memcpy(&(last_sc_info.VideoImg), &input_image, sizeof(struct tcc_lcdc_image_update));
								last_sc_info.Video_updated = false;
							}
							else
							#endif//CONFIG_EXTEND_DISPLAY_DELAY
							{
								#ifdef MVC_PROCESS
									if(TCC_OUTPUT_FB_GetMVCStatus() && input_image.MVCframeView == 1 && input_image.MVC_Base_addr0)
										TCC_HDMI_DISPLAY_UPDATE_3D(EX_OUT_LCDC, &input_image);
									else
								#endif
										TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, &input_image);
							}
							break;
						case TCC_OUTPUT_COMPONENT:
						#if defined(CONFIG_FB_TCC_COMPONENT)
								tcc_component_update(&input_image);
						#endif
							break;
						case TCC_OUTPUT_COMPOSITE:
						#if defined(CONFIG_FB_TCC_COMPOSITE)
								tcc_composite_update(&input_image);
						#endif
							break;
						default:
							break;
					}

				//input_image.time_stamp = 0;
				//	return tccvid_vsync.vsync_buffer.writeIdx;
				}

				input_image.time_stamp = 0;
				goto PUSH_VIDEO_FORCE;
			}
			
			if(tccvid_vsync.skipFrameStatus)
			{
				error_type = 1;
			}
			else if(tccvid_vsync.outputMode != Output_SelectMode)
			{
				vprintk("vsync push error : output mode different %d %d \n", tccvid_vsync.outputMode ,Output_SelectMode);			
				error_type = 2;
			}
			else if(tccvid_vsync.vsync_buffer.available_buffer_id_on_vpu > input_image.buffer_unique_id)
			{
				vprintk("vsync push error : buffer index sync fail omx_buf_id: %d, cur_buff_id: %d \n", 
						tccvid_vsync.vsync_buffer.available_buffer_id_on_vpu, input_image.buffer_unique_id);
				error_type = 3;
			}
			else if(input_image.time_stamp < input_image.sync_time)
			{
				/*
				if(input_image.time_stamp < 2000)
				{
					vprintk("vsync push error 4: vtime: %d, sync_time: %d \n", input_image.time_stamp, input_image.sync_time);
					error_type = 4;
				}
				else
				*/
				{
					//printk("vsync push error : vtime: %d, writeIdx %d \n", input_image.time_stamp, tccvid_vsync.vsync_buffer.writeIdx);
					input_image.time_stamp = TIME_MARK_SKIP;
					
				}
			}

			if(error_type > 0)
			{
				vprintk("vsync push error : %d buffer_unique_id %d\n", error_type,input_image.buffer_unique_id);
				spin_lock_irq(&vsync_lockDisp) ;
				tccvid_vsync.vsync_buffer.last_cleared_buff_id = input_image.buffer_unique_id;
				spin_unlock_irq(&vsync_lockDisp) ;
				goto TCC_VSYNC_PUSH_ERROR;
			}

PUSH_VIDEO_FORCE : 
			if(tcc_vsync_is_full_buffer(&tccvid_vsync.vsync_buffer))
			{
				vprintk("mio push wait start\n") ;
				wait_event_interruptible_timeout( wq_consume, 
												tcc_vsync_is_full_buffer(&tccvid_vsync.vsync_buffer) == 0, 
												msecs_to_jiffies(500)) ;
				vprintk("mio push wait end \n");
			}

			spin_lock_irq(&vsync_lock) ;
			if(tcc_vsync_push_buffer(&tccvid_vsync.vsync_buffer, &input_image) < 0)
			{
				printk("critical error: vsync buffer full by fault buffer controll\n");
			}

			if(tccvid_vsync.deinterlace_mode && !tccvid_vsync.output_toMemory && !tccvid_vsync.interlace_bypass_lcdc)
			{
				int curTime = tcc_vsync_get_time();

				switch(Output_SelectMode)
				{
					case TCC_OUTPUT_NONE:
						break;
					#ifdef TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
					case TCC_OUTPUT_LCD:
						{
							viqe_render_frame(input_image.addr0, input_image.addr1, input_image.addr2, input_image.odd_first_flag, tccvid_vsync.vsync_interval, curTime,
											input_image.Frame_width, input_image.Frame_height,	// srcWidth, srcHeight
											input_image.crop_top, input_image.crop_bottom, input_image.crop_left, input_image.crop_right,
											input_image.Image_width, input_image.Image_height,
											input_image.offset_x, input_image.offset_y, input_image.frameInfo_interlace, 0);
						}
						break;
					#endif
					case TCC_OUTPUT_HDMI:
					case TCC_OUTPUT_COMPONENT:
					case TCC_OUTPUT_COMPOSITE:
					#ifdef CONFIG_EXTEND_DISPLAY_DELAY //for video 
						if(!atomic_read(&ExtDispStruct.update_allow))
						{
							memcpy(&(last_sc_info.VideoImg), &input_image, sizeof(struct tcc_lcdc_image_update));
							last_sc_info.Video_updated = false;
						}
						else
					#endif
						{
							viqe_render_frame(input_image.addr0, input_image.addr1, input_image.addr2, input_image.odd_first_flag, tccvid_vsync.vsync_interval, curTime,
											input_image.Frame_width, input_image.Frame_height,	// srcWidth, srcHeight
											input_image.crop_top, input_image.crop_bottom, input_image.crop_left, input_image.crop_right,
											input_image.Image_width, input_image.Image_height,
											input_image.offset_x, input_image.offset_y, input_image.frameInfo_interlace, 0);
						}
						break;
						
					default:
						break;
				}
				
			}
			
			spin_unlock_irq(&vsync_lock) ;
			//printk("OddFirst = %d, interlaced %d\n", input_image.odd_first_flag, input_image.deinterlace_mode);
			vprintk("vtime : %d, curtime : %d\n", input_image.time_stamp, input_image.sync_time) ;

		}

TCC_VSYNC_PUSH_ERROR:
		return tccvid_vsync.vsync_buffer.writeIdx;
		break ;
		
	case TCC_LCDC_VIDEO_GET_DISPLAYED :
		return tcc_video_get_displayed(); 
		break ;
		
	case TCC_LCDC_VIDEO_GET_VALID_COUNT:
		return tcc_video_get_valid_count();
		break ;
		
	case TCC_LCDC_VIDEO_CLEAR_FRAME:
		tcc_video_clear_frame((int)arg);
		break ;

	case TCC_LCDC_VIDEO_SWAP_VPU_FRAME:
		return tcc_video_swap_vpu_frame((int)arg);
	break;

	case TCC_LCDC_VIDEO_SKIP_FRAME_START:
		tcc_video_skip_frame_start();
		break ;

	case TCC_LCDC_VIDEO_SKIP_FRAME_END:
		tcc_video_skip_frame_end();
		break ;
		
	case TCC_LCDC_VIDEO_SKIP_ONE_FRAME:
		tcc_video_skip_one_frame((int)arg);
		break ;
#endif

	case TCC_LCDC_REFER_VSYNC_ENABLE:
		tca_vsync_enable(fb_info, 1);
		break;

	case TCC_LCDC_REFER_VSYNC_DISABLE:
		tca_vsync_enable(fb_info, 0);
		break;

	case TCC_LCD_FBIOPUT_VSCREENINFO:
		{
#if defined(CONFIG_SYNC_FB)
			struct fb_var_screeninfo var_info;
			struct tcc_fenc_reg_data *regs;
			struct sync_fence *fence;
			struct sync_pt *pt;
			int ret = 0, fd = 0;

			if (copy_from_user(&var_info,
					   (struct fb_var_screeninfo __user *)arg,
					   sizeof(struct fb_var_screeninfo))) {
				ret = -EFAULT;
				break;
			}
			fd = get_unused_fd();

			if(fd< 0){
				pr_err(" fb fence sync get fd error : %d \n", fd);
				break;
			}

			mutex_lock(&fb_info->output_lock);
		
			if (!fb_info->output_on) {
				fb_info->fb_timeline_max++;
				pt = sw_sync_pt_create(fb_info->fb_timeline, fb_info->fb_timeline_max);
				fence = sync_fence_create("display", pt);
				sync_fence_install(fence, fd);
				var_info.reserved[2] = fd;
				var_info.reserved[3] = 0xf;
				sw_sync_timeline_inc(fb_info->fb_timeline, 1);
				printk("lcd display update on power off state \n");
		
				if (copy_to_user((struct fb_var_screeninfo __user *)arg,
						 &var_info,
						 sizeof(struct fb_var_screeninfo))) {
					ret = -EFAULT;
				}
				mutex_unlock(&fb_info->output_lock);
				break;
			}

			regs = kzalloc(sizeof(struct tcc_fenc_reg_data), GFP_KERNEL);

			if (!regs) {
				pr_err("fb fence sync could not allocate \n");
				mutex_unlock(&fb_info->output_lock);
				return -ENOMEM;
			}				

			mutex_lock(&fb_info->fb_timeline_lock);
			regs->screen_cpu = fb_info->screen_cpu;
			regs->screen_dma= fb_info->screen_dma;
			regs->var = var_info;

			regs->fence_fd = var_info.reserved[2];
			if(regs->fence_fd > 0)
			{
				regs->fence = sync_fence_fdget(regs->fence_fd);
				if (!regs->fence ) {
					printk("failed to import fence fd\n");
				}
			}	
			list_add_tail(&regs->list, &fb_info->fb_update_regs_list);

			fb_info->fb_timeline_max++;
			pt = sw_sync_pt_create(fb_info->fb_timeline, fb_info->fb_timeline_max);
			fence = sync_fence_create("display", pt);
			sync_fence_install(fence, fd);
			var_info.reserved[2] = fd;
			var_info.reserved[3] = 0xf;
			mutex_unlock(&fb_info->fb_timeline_lock);
				
			queue_kthread_work(&fb_info->fb_update_regs_worker,
									&fb_info->fb_update_regs_work);

			mutex_unlock(&fb_info->output_lock);

			if (copy_to_user((struct fb_var_screeninfo __user *)arg,
					 &var_info,
					 sizeof(struct fb_var_screeninfo))) {
				ret = -EFAULT;
				break;
			}
#endif//
		}
		break;


	case TCC_HDMI_FBIOPUT_VSCREENINFO:
		{
			external_fbioput_vscreeninfo sc_info;

			if((!fb_power_state) || (Output_SelectMode != TCC_OUTPUT_HDMI))
				return 0;

			if (copy_from_user((void*)&sc_info, (const void*)arg, sizeof(external_fbioput_vscreeninfo)))
				return -EFAULT;

			#if defined(CONFIG_EXTEND_DISPLAY_DELAY) && defined(CONFIG_ARCH_TCC893X)
			//do not accept TCC892X. at least one channel must be turned on in TCC892X. 
			//prevent under run state in CONFIG_EXTEND_DISPLAY_DELAY(TCC892X)
			if(!atomic_read(&ExtDispStruct.update_allow)) 
			{
				spin_lock(&ExtDispStruct.delay_spinlock);
				last_sc_info.sc_info = sc_info;
				last_sc_info.base_addr = fb_info->map_dma ;
				last_sc_info.Ex_SelectMode = Output_SelectMode;
				last_sc_info.UI_updated = false;
				spin_unlock(&ExtDispStruct.delay_spinlock);
				return 0;
			}
			#endif// CONFIG_EXTEND_DISPLAY_DELAY

			if(TCC_OUTPUT_FB_Update_External(sc_info.width, sc_info.height, sc_info.bits_per_pixel, fb_info->map_dma + sc_info.offset, Output_SelectMode) > 0) 
			{
				TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
			}
			
#if defined(CONFIG_SYNC_FB)

			mutex_lock(&fb_info->ext_timeline_lock);
			{
				struct sync_fence *fence;
				struct sync_pt *pt;
				int fd;
				
				fd = get_unused_fd();
				fb_info->ext_timeline_max++;
				pt = sw_sync_pt_create(fb_info->ext_timeline, fb_info->ext_timeline_max);
				fence = sync_fence_create("display_ext", pt);
				sync_fence_install(fence, fd);
				sc_info.fence_fd = fd;
				sw_sync_timeline_inc(fb_info->ext_timeline, 1);

			}
//			queue_kthread_work(&fb_info->ext_update_regs_worker,
//									&fb_info->ext_update_regs_work);		
			mutex_unlock(&fb_info->ext_timeline_lock);
			
			if (copy_to_user((external_fbioput_vscreeninfo *)arg, (void*)&sc_info, sizeof(external_fbioput_vscreeninfo)))
				return -EFAULT;
			
			#endif
		}
		break;

		case TCC_LCDC_DISPLAY_START:
			{
				printk(" TCC_LCDC_DISPLAY_START \n");
			}
			break;

		case TCC_LCDC_DISPLAY_END:
			{
				struct tcc_lcdc_image_update ImageInfo;
				printk(" TCC_LCDC_DISPLAY_END lcd_video_started %d\n", lcd_video_started);

				if(!lcd_video_started)
					return 0;

				memset(&ImageInfo, 0x00, sizeof(struct tcc_lcdc_image_update));
				ImageInfo.enable = 0;

				TCC_LCD_DISPLAY_UPDATE(LCD_LCDC_NUM, (struct tcc_lcdc_image_update *)&ImageInfo);

				lcd_video_started = 0;
			}
			break;

		case TCC_LCDC_DISPLAY_UPDATE:
			{
				struct tcc_lcdc_image_update ImageInfo;
				
				if(Output_SelectMode != TCC_OUTPUT_NONE)
					return 0;
				
				dprintk(" TCC_LCDC_DISPLAY_UPDATE \n");

				if (copy_from_user((void *)&ImageInfo, (const void *)arg, sizeof(struct tcc_lcdc_image_update))){
					return -EFAULT;
				}

				TCC_LCD_DISPLAY_UPDATE(LCD_LCDC_NUM, (struct tcc_lcdc_image_update *)&ImageInfo);
				lcd_video_started = 1;
			}
			break;

		case TCC_LCDC_CTRL_LAST_FRAME:
			{
				tcc_video_ctrl_last_frame((int)arg);
			}
			break;

	default:
		dprintk("ioctl: Unknown [%d/0x%X]", cmd, cmd);
		break;
	}


	return 0;
}

static void schedule_palette_update(struct tccfb_info *fbi,
				    unsigned int regno, unsigned int val)
{
	unsigned long flags;

	local_irq_save(flags);

	local_irq_restore(flags);
}

/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int tccfb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	struct tccfb_info *fbi = info->par;
	unsigned int val;

	/* dprintk("setcol: regno=%d, rgb=%d,%d,%d\n", regno, red, green, blue); */

	switch (fbi->fb->fix.visual) {
		case FB_VISUAL_TRUECOLOR:
			/* true-colour, use pseuo-palette */

			if (regno < 16) {
				u32 *pal = fbi->fb->pseudo_palette;

				val  = chan_to_field(red,   &fbi->fb->var.red);
				val |= chan_to_field(green, &fbi->fb->var.green);
				val |= chan_to_field(blue,  &fbi->fb->var.blue);

				pal[regno] = val;
			}
			break;

		case FB_VISUAL_PSEUDOCOLOR:
			if (regno < 256) {
				/* currently assume RGB 5-6-5 mode */

				val  = ((red   >>  0) & 0xf800);
				val |= ((green >>  5) & 0x07e0);
				val |= ((blue  >> 11) & 0x001f);

				//writel(val, S3C2410_TFTPAL(regno));
				schedule_palette_update(fbi, regno, val);
			}
			break;

		default:
			return 1;   /* unknown type */
	}

	return 0;
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
#if defined(CONFIG_TOUCHSCREEN_TCCTS) || defined(CONFIG_TOUCHSCREEN_TCC_GT813) || defined(CONFIG_TOUCHSCREEN_ILITEK_AIMVF)
extern void tcc_ts_disable(void);
extern void tcc_ts_enable(void);
#endif

static int tcc_fb_enable(struct fb_info *info)
{		
	int ret = 0;
	struct tccfb_info *fbi =(struct tccfb_info *) info->par;		

	mutex_lock(&fbi->output_lock);
	if(fbi->output_on) {
		ret = -EBUSY;
		goto err;
	}
	
	fbi->output_on = true;
	pm_runtime_get_sync(fbi->dev);

err:
	mutex_unlock(&fbi->output_lock);
	return ret;
}

static int tcc_fb_disable(struct fb_info *info)
{
	int ret = 0;
	struct tccfb_info *fbi =(struct tccfb_info *) info->par;

	mutex_lock(&fbi->output_lock);

#if defined(CONFIG_SYNC_FB)	
	flush_kthread_worker(&fbi->fb_update_regs_worker);
	flush_kthread_worker(&fbi->ext_update_regs_worker);
#endif//
	if(!fbi->output_on) {
		ret = -EBUSY;
		goto err;
	}
	fbi->output_on = false;
	pm_runtime_put_sync(fbi->dev);	
	
err:	
	mutex_unlock(&fbi->output_lock);
	return ret;
}
#endif



static int tccfb_blank(int blank_mode, struct fb_info *info)
{
#ifdef CONFIG_PM_RUNTIME
	int ret = 0;

	struct tccfb_info *fbi =(struct tccfb_info *) info->par;

	pm_runtime_get_sync(fbi->dev);

	switch(blank_mode)
	{
		case FB_BLANK_POWERDOWN:
		case FB_BLANK_NORMAL:
#if defined(CONFIG_TOUCHSCREEN_TCCTS) || defined(CONFIG_TOUCHSCREEN_TCC_GT813) || defined(CONFIG_TOUCHSCREEN_ILITEK_AIMVF)
			tcc_ts_disable();
			#endif
			tcc_fb_disable(info);
			break;
		case FB_BLANK_UNBLANK:
			tcc_fb_enable(info);
#if defined(CONFIG_TOUCHSCREEN_TCCTS) || defined(CONFIG_TOUCHSCREEN_TCC_GT813) || defined(CONFIG_TOUCHSCREEN_ILITEK_AIMVF)
			tcc_ts_enable();
			#endif
			break;
		case FB_BLANK_HSYNC_SUSPEND:
		case FB_BLANK_VSYNC_SUSPEND:
		default:
			ret = -EINVAL;
	}
	pm_runtime_put_sync(fbi->dev);

#endif

	return 0;
}

#ifdef CONFIG_DMA_SHARED_BUFFER
static struct sg_table *fb_ion_map_dma_buf(struct dma_buf_attachment *attachment,
					enum dma_data_direction direction)
{
	struct sg_table *table;
	struct fb_info *info = (struct fb_info *)attachment->dmabuf->priv;
	struct page *page;
	int err;

	table = kzalloc(sizeof(struct sg_table), GFP_KERNEL);
	err = sg_alloc_table(table, 1, GFP_KERNEL);
	if (err)
	{
	        kfree(table);
	        return ERR_PTR(err);
	}
	sg_set_page(table->sgl, NULL, info->fix.smem_len, 0);
	sg_dma_address(table->sgl) = info->fix.smem_start;
	debug_dma_map_sg(NULL, table->sgl, table->nents, table->nents, DMA_BIDIRECTIONAL);

	return table;
}

static void fb_ion_unmap_dma_buf(struct dma_buf_attachment *attachment,
			      struct sg_table *table,
			      enum dma_data_direction direction)
{
	struct fb_info *info = (struct fb_info *)attachment->dmabuf->priv;

	debug_dma_unmap_sg(NULL, table->sgl, table->nents, DMA_BIDIRECTIONAL);
	sg_free_table(table);

	kfree(table);
}

static int fb_ion_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	return -ENOSYS;
}

static void fb_ion_dma_buf_release(struct dma_buf *dmabuf)
{
	return -ENOSYS;
}

static void *fb_ion_dma_buf_kmap(struct dma_buf *dmabuf, unsigned long offset)
{
	return -ENOSYS;
}


struct dma_buf_ops fb_dma_buf_ops = {
	.map_dma_buf = fb_ion_map_dma_buf,
	.unmap_dma_buf = fb_ion_unmap_dma_buf,
	.mmap = fb_ion_mmap,
	.release = fb_ion_dma_buf_release,
	.kmap_atomic = fb_ion_dma_buf_kmap,
	.kmap = fb_ion_dma_buf_kmap,
};

struct dma_buf* tccfb_dmabuf_export(struct fb_info *info)
{
	struct dma_buf *buf = dma_buf_export(info, &fb_dma_buf_ops, info->fix.smem_len, O_RDWR);
	return buf;
}
#endif



#ifdef CONFIG_PM_RUNTIME
/* suspend and resume support for the lcd controller */
static int tccfb_suspend(struct device *dev)
{

struct platform_device *fb_device = container_of(dev, struct platform_device, dev);
	struct fb_info	   *fbinfo = platform_get_drvdata(fb_device);


printk("%s  0x%p", fb_device->name , fbinfo);

	if((system_rev ==0x2002) || (system_rev == 0x2004) || (system_rev == 0x2005) || (system_rev == 0x2006) || (system_rev == 0x2007) || (system_rev == 0x2008) || (system_rev == 0x2009))
	{
		fb_power_state = 0;

		if (lcd_panel->set_power)
			lcd_panel->set_power(lcd_panel, 0, LCD_LCDC_NUM);
	}

	#if defined(CONFIG_HIBERNATION)
	if(do_hibernation) {
		if( android_system_booting_finished == 0) {
			Output_SelectMode = TCC_OUTPUT_NONE;
			output_lcdc_onoff = 0;
		}
	}
	#endif

	tca_fb_suspend(dev);
	return 0;
}

static int tccfb_resume(struct device *dev)
{

	tca_fb_resume(dev);

	if((system_rev == 0x2002) ||(system_rev == 0x2004) || (system_rev == 0x2005) || (system_rev == 0x2006) || (system_rev == 0x2007) || (system_rev == 0x2008) || (system_rev == 0x2009))
	{
		if (lcd_panel->set_power)
			lcd_panel->set_power(lcd_panel, 1, LCD_LCDC_NUM);
		
		//fb_power_state = 1;	
	}
	return 0;
}

static void tccfb_shutdown(struct platform_device *dev)
{
        printk(" %s \n",__func__);
        tccfb_suspend(&dev->dev);
}

static int tccfb_freeze(struct device *dev)
{

	/*		Touch Screen Disable		*/
#if defined(CONFIG_TOUCHSCREEN_TCCTS) || defined(CONFIG_TOUCHSCREEN_TCC_GT813) || defined(CONFIG_TOUCHSCREEN_ILITEK_AIMVF)
	tcc_ts_disable();
#endif

    //It used to suspend when creating Quickboot Image
	tca_fb_suspend(dev);
      printk(" %s \n",__func__);

      return 0;
}


static int tccfb_thaw(struct device *dev)
{
    //After you create a QuickBoot Image, before restarting
	tca_fb_resume(dev);
      printk(" %s \n",__func__);
	  
	  /*		Touch Screen Enable		*/
#if defined(CONFIG_TOUCHSCREEN_TCCTS) || defined(CONFIG_TOUCHSCREEN_TCC_GT813) || defined(CONFIG_TOUCHSCREEN_ILITEK_AIMVF)
	tcc_ts_enable();
#endif

      return 0;
}

static int tccfb_restore(struct device *dev)
{
    // Used for Loading after resume to Quickboot Image
	tca_fb_resume(dev);
      printk(" %s \n",__func__);    

	  /*		Touch Screen Enable		*/
#if defined(CONFIG_TOUCHSCREEN_TCCTS) || defined(CONFIG_TOUCHSCREEN_TCC_GT813) || defined(CONFIG_TOUCHSCREEN_ILITEK_AIMVF)
	tcc_ts_enable();
#endif

      return 0;
}
#endif



static struct fb_ops tccfb_ops = {
	.owner			= THIS_MODULE,
	.fb_check_var	= tccfb_check_var,
	.fb_set_par		= tccfb_set_par,
	.fb_blank		= tccfb_blank,
	.fb_setcolreg	= tccfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_ioctl		= tccfb_ioctl,
	.fb_pan_display = tccfb_pan_display,
#ifdef CONFIG_DMA_SHARED_BUFFER
	.fb_dmabuf_export = tccfb_dmabuf_export,
#endif	
};


/*
 * tccfb_map_video_memory():
 *	Allocates the DRAM memory for the frame buffer.  This buffer is
 *	remapped into a non-cached, non-buffered, memory region to
 *	allow palette and pixel writes to occur without flushing the
 *	cache.  Once this area is remapped, all virtual memory
 *	access to the video memory should occur at the new region.
 */

static int __init tccfb_map_video_memory(struct tccfb_info *fbi, int plane)
{

	fbi->map_size = PAGE_ALIGN(fbi->fb->fix.smem_len + PAGE_SIZE);


	if(FB_VIDEO_MEM_SIZE == 0)
	{
		fbi->map_cpu = dma_alloc_writecombine(fbi->dev, fbi->map_size, &fbi->map_dma, GFP_KERNEL);
		printk("map_video_memory (fbi=%p) kernel memory, dma:0x%x map_size:%08x\n", fbi,fbi->map_dma, fbi->map_size);
	}
	else
	{
		fbi->map_dma =  FB_VIDEO_MEM_BASE;
		fbi->map_cpu = ioremap_nocache(fbi->map_dma, fbi->map_size);
		printk("map_video_memory (fbi=%p) used map memory,map dma:0x%x size:%08x\n", fbi, fbi->map_dma ,fbi->map_size);
	}

	fbi->map_size = fbi->fb->fix.smem_len;

	if (fbi->map_cpu) {
		/* prevent initial garbage on screen */
		dprintk("map_video_memory: clear %p:%08x\n", fbi->map_cpu, fbi->map_size);

		memset(fbi->map_cpu, 0x00, fbi->map_size);

		fbi->screen_dma		= fbi->map_dma;
		fbi->fb->screen_base	= fbi->map_cpu;
		fbi->fb->fix.smem_start  = fbi->screen_dma;

		// Set the LCD frame buffer start address
		switch (plane)
		{
			case 2:	// IMG2
				fb_mem_vaddr[plane] = fbi->map_cpu;
				fb_mem_size [plane] = fbi->map_size;
				break;
			case 1:	// IMG1
				fb_mem_vaddr[plane] = fbi->map_cpu;
				fb_mem_size [plane] = fbi->map_size;
				break;
			case 0:	// IMG0
				fb_mem_vaddr[plane] = fbi->map_cpu;
				fb_mem_size [plane] = fbi->map_size;
				break;
		}
		dprintk("map_video_memory: dma=%08x cpu=%p size=%08x\n",
			fbi->map_dma, fbi->map_cpu, fbi->fb->fix.smem_len);
	}

	return fbi->map_cpu ? 0 : -ENOMEM;
}

static inline void tccfb_unmap_video_memory(struct tccfb_info *fbi)
{
	dma_free_writecombine(fbi->dev,fbi->map_size,fbi->map_cpu, fbi->map_dma);

	fb_mem_vaddr[fbi->imgch] = (u_char*)NULL;
	fb_mem_size [fbi->imgch] = (u_int)NULL;
}

static char tccfb_driver_name[]="tccfb";
static int __init tccfb_probe(struct platform_device *pdev)
{
	struct tccfb_info *info;
	struct fb_info *fbinfo;
	int ret;
	int plane = 0;
	unsigned int screen_width, screen_height;

	if (!lcd_panel) {
		pr_err("tccfb: no LCD panel data\n");
		return -EINVAL;
	}
	pr_info("LCD panel is %s %s %d x %d\n", lcd_panel->manufacturer, lcd_panel->name,
		lcd_panel->xres, lcd_panel->yres);

        screen_width      = lcd_panel->xres;
        screen_height     = lcd_panel->yres;

#if defined(CONFIG_TCC_HDMI_UI_SIZE_1280_720)
        if(tcc_display_data.resolution == 1)
        {
            screen_width      = 720;
            screen_height     = 576;
        }
        else if(tcc_display_data.resolution == 2)
        {
            screen_width 	  = 800;
            screen_height 	  = 480;
        }
#endif

	printk("%s, screen_width=%d, screen_height=%d\n", __func__, screen_width, screen_height);

	pmap_get_info("fb_video", &pmap_fb_video);

#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
	pmap_get_info("exclusive_viqe", &pmap_eui_viqe);
#endif

	for (plane = 0; plane < CONFIG_FB_TCC_DEVS; plane++)
	{
		fbinfo = framebuffer_alloc(sizeof(struct tccfb_info), &pdev->dev);
		info = fbinfo->par;
		info->fb = fbinfo;
		info->dev = &pdev->dev;
		#if defined(CONFIG_HIBERNATION) 
		platform_set_drvdata(pdev, fbinfo);
		#else
		platform_set_drvdata(pdev, info);
		#endif
		printk("0x%p, 0x%p platform_data:0x%p \n", info, fbinfo, platform_get_drvdata(pdev));
		#ifdef CONFIG_PM_RUNTIME
		pm_runtime_set_active(info->dev);	
		pm_runtime_enable(info->dev);  
		pm_runtime_get_noresume(info->dev);  //increase usage_count 
		#endif

		strcpy(fbinfo->fix.id, tccfb_driver_name);

		fbinfo->fix.type		= FB_TYPE_PACKED_PIXELS;
		fbinfo->fix.type_aux		= 0;
		fbinfo->fix.xpanstep		= 0;
		fbinfo->fix.ypanstep		= 1;
		fbinfo->fix.ywrapstep		= 0;
		fbinfo->fix.accel		= FB_ACCEL_NONE;

		fbinfo->var.nonstd		= 0;
		fbinfo->var.activate		= FB_ACTIVATE_NOW;

		fbinfo->var.accel_flags		= 0;
		fbinfo->var.vmode		= FB_VMODE_NONINTERLACED;

		fbinfo->fbops			= &tccfb_ops;
		fbinfo->flags			= FBINFO_FLAG_DEFAULT;

		fbinfo->var.xres		= screen_width;
		fbinfo->var.xres_virtual	= fbinfo->var.xres;
		fbinfo->var.yres		= screen_height;

		fbinfo->var.yres_virtual	= fbinfo->var.yres * FB_NUM_BUFFERS;
		fbinfo->var.bits_per_pixel	= default_scn_depth[plane];
		fbinfo->fix.line_length 	= fbinfo->var.xres * fbinfo->var.bits_per_pixel/8;

		tccfb_check_var(&fbinfo->var, fbinfo);

		// the memory size that LCD should occupy
		fbinfo->fix.smem_len = pmap_fb_video.size;
		info->imgch = plane;

		/* Initialize video memory */
		ret = tccfb_map_video_memory(info, plane);
		if (ret) {
			dprintk( KERN_ERR "Failed to allocate video RAM: %d\n", ret);
			ret = -ENOMEM;
		}

		ret = register_framebuffer(fbinfo);
		if (ret < 0) {
			dprintk(KERN_ERR "Failed to register framebuffer device: %d\n", ret);
			goto free_video_memory;
		}
		tccfb_set_par(fbinfo);

		if (plane == 0)	// top layer
			if (fb_prepare_logo(fbinfo,	FB_ROTATE_UR)) {
			/* Start display and show logo on boot */
			dprintk("fb_show_logo\n");
			fb_set_cmap(&fbinfo->cmap, fbinfo);
			fb_show_logo(fbinfo, FB_ROTATE_UR);
        	}
		printk(KERN_INFO "fb%d: %s frame buffer device\n", fbinfo->node, fbinfo->fix.id);
	}

	#if !defined(CONFIG_MACH_TCC8920ST) && !defined(CONFIG_MACH_TCC8930ST)
		tcc_lcd_interrupt_reg(TRUE, info);
	#endif

	tca_init_vsync(info);
	TCC_OUTPUT_FB_setFBInfo(info);

	#if defined(CONFIG_TCC8925S_DISP_PORT_CHANGE)
	spin_lock_init(&port_change_lock);
	Port_change_state = 1;
	#endif

	mutex_init(&info->output_lock);
	info->output_on = true;
	
#if defined(CONFIG_SYNC_FB)
	INIT_LIST_HEAD(&info->fb_update_regs_list);
	mutex_init(&info->fb_timeline_lock);
	init_kthread_worker(&info->fb_update_regs_worker);

	info->fb_update_regs_thread = kthread_run(kthread_worker_fn,
			&info->fb_update_regs_worker, "tccfb");

	if (IS_ERR(info->fb_update_regs_thread)) {
		int err = PTR_ERR(info->fb_update_regs_thread);
		info->fb_update_regs_thread = NULL;

		pr_err("failed to run update_regs thread\n");
		return err;
	}
	init_kthread_work(&info->fb_update_regs_work, fence_handler);
	info->fb_timeline = sw_sync_timeline_create("tccfb");
	info->fb_timeline_max	=  0;


	INIT_LIST_HEAD(&info->ext_update_regs_list);
	mutex_init(&info->ext_timeline_lock);
	init_kthread_worker(&info->ext_update_regs_worker);

	info->ext_update_regs_thread = kthread_run(kthread_worker_fn,
			&info->ext_update_regs_worker, "tccfb-ext");

	if (IS_ERR(info->ext_update_regs_thread)) {
		int err = PTR_ERR(info->ext_update_regs_thread);
		info->ext_update_regs_thread = NULL;

		pr_err("failed to run update_regs thread\n");
		return err;
	}
	init_kthread_work(&info->ext_update_regs_work, ext_fence_handler);
	info->ext_timeline = sw_sync_timeline_create("tccfb-ext");
	info->ext_timeline_max	=  0;
#endif

	#ifdef CONFIG_EXTEND_DISPLAY_DELAY
	spin_lock_init(&ExtDispStruct.delay_spinlock);
	INIT_DELAYED_WORK(&ExtDispStruct.delay_work, extenddisplay_work_handler);
	#endif// CONFIG_EXTEND_DISPLAY_DELAY

	return 0;

free_video_memory:
	tccfb_unmap_video_memory(info);
	dprintk("TCC92xx fb init failed.\n");
	return ret;
}


/*
 *  Cleanup
 */
static int tccfb_remove(struct platform_device *pdev)
{
	struct fb_info	   *fbinfo = platform_get_drvdata(pdev);
	struct tccfb_info *info = fbinfo->par;

	dprintk(" %s  \n", __func__);

	#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
	#else	
		tcc_lcd_interrupt_reg(FALSE, info);
	#endif

	tccfb_unmap_video_memory(info);
#ifdef CONFIG_PM_RUNTIME
	pm_runtime_disable(info->dev);
#endif
	//release_mem_region((unsigned long)S3C24XX_VA_LCD, S3C24XX_SZ_LCD);
	unregister_framebuffer(fbinfo);

	return 0;
}

int tccfb_register_panel(struct lcd_panel *panel)
{
	dprintk(" %s  name:%s \n", __func__, panel->name);

	lcd_panel = panel;
	return 1;
}
EXPORT_SYMBOL(tccfb_register_panel);

struct lcd_panel *tccfb_get_panel(void)
{
	return lcd_panel;
}
EXPORT_SYMBOL(tccfb_get_panel);

#ifdef CONFIG_PM_RUNTIME
int tcc_fb_runtime_suspend(struct device *dev)
{

	printk("%s:  \n", __FUNCTION__);

	//dprintk("%s ### state = [%d] count =[%d] pm_state=[%d] \n",__func__,pm_runtime_status_suspended(dev), dev->power.usage_count, dev->power.runtime_status);

	fb_power_state = 0;
	if((system_rev != 0x2002) && (system_rev != 0x2004) && (system_rev != 0x2005) && (system_rev != 0x2006) && (system_rev != 0x2007) && (system_rev != 0x2008) && (system_rev != 0x2009))
	{
		//fb_power_state = 0;

		if (lcd_panel->set_power)
			lcd_panel->set_power(lcd_panel, 0, LCD_LCDC_NUM);
	}
	tca_fb_runtime_suspend();

	printk("%s: finish \n", __FUNCTION__);

	return 0;
}

int tcc_fb_runtime_resume(struct device *dev)
{

	printk("%s:  \n", __FUNCTION__);

	//dprintk("%s ### state = [%d] count =[%d] pm_state=[%d] \n",__func__,pm_runtime_status_suspended(dev), dev->power.usage_count, dev->power.runtime_status);

	tca_fb_runtime_resume();
	if((system_rev != 0x2002) && (system_rev != 0x2004) && (system_rev != 0x2005) && (system_rev != 0x2006) && (system_rev != 0x2007) && (system_rev != 0x2008) && (system_rev != 0x2009))
	{
		if (lcd_panel->set_power)
			lcd_panel->set_power(lcd_panel, 1, LCD_LCDC_NUM);

		//fb_power_state = 1;
	}
	fb_power_state = 1;

	printk("%s: finish \n", __FUNCTION__);

	return 0;
}

#endif


#ifdef CONFIG_PM_RUNTIME
static const struct dev_pm_ops tccfb_pm_ops = {
	.runtime_suspend      = tcc_fb_runtime_suspend,
	.runtime_resume       = tcc_fb_runtime_resume,
	.suspend	= tccfb_suspend,
	.resume = tccfb_resume,
	.freeze = tccfb_freeze,
	.thaw = tccfb_thaw,
	.restore = tccfb_restore,
};
#endif

static struct platform_driver tccfb_driver = {
	.probe		= tccfb_probe,
	.remove		= tccfb_remove,
	.shutdown	= tccfb_shutdown,
	.driver		= {
		.name	= "tccfb",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM_RUNTIME
		.pm		= &tccfb_pm_ops,
#endif
	},
};


//int __devinit tccfb_init(void)
static int __init tccfb_init(void)
{
    printk(KERN_INFO " %s (built %s %s)\n", __func__, __DATE__, __TIME__);

	fb_power_state = 1;

	EX_OUT_LCDC = tca_get_output_lcdc_num();
	LCD_LCDC_NUM = tca_get_lcd_lcdc_num();

	tca_fb_init();

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
	spin_lock_init(&vsync_lock) ;
	spin_lock_init(&vsync_lockDisp ) ;
#endif

#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
	pmap_get_info("fb_wmixer", &fb_lastframe_pbuf);
	printk(" %s wmixer base:0x%08x size:%d\n",__func__,fb_lastframe_pbuf.base, fb_lastframe_pbuf.size );

#if defined(CONFIG_ARCH_TCC892X)
	pmap_get_info("fb_wmixer_ext", &fb_lastframe_pbuf_ext);
	printk(" %s wmixer_ext base:0x%08x size:%d\n",__func__,fb_lastframe_pbuf_ext.base, fb_lastframe_pbuf_ext.size );
#endif

	spin_lock_init(&LastFrame_lockDisp);

	tcc_video_ctrl_last_frame(0);
#endif//CONFIG_HDMI_DISPLAY_LASTFRAME

	return platform_driver_register(&tccfb_driver);
}

static void __exit tccfb_exit(void)
{
	dprintk(" %s \n", __func__);

	tca_fb_exit();
	platform_driver_unregister(&tccfb_driver);
}


module_init(tccfb_init);
module_exit(tccfb_exit);

MODULE_AUTHOR("linux <linux@telechips.com>");
MODULE_DESCRIPTION("Telechips TCC Framebuffer driver");
MODULE_LICENSE("GPL");
