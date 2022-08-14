/*
 * linux/drivers/video/tcc/tccfb.h
 *
 * Author: <linux@telechips.com>
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

#ifndef __TCCFB_H
#define __TCCFB_H
#include <linux/kthread.h>
struct tccfb_platform_data;

struct tccfb_info {
	struct fb_info		*fb;
	struct device		*dev;

	//struct tccfb_mach_info *mach_info;

	/* raw memory addresses */
	dma_addr_t		map_dma;	/* physical */
	u_char *		map_cpu;	/* virtual */
	u_int			map_size;

	/* addresses of pieces placed in raw buffer */
	u_char *		screen_cpu;	/* virtual address of buffer */
	dma_addr_t		screen_dma;	/* physical address of buffer */

	u_int			imgch;

	struct tccfb_platform_data *pdata;
#ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend early_suspend;
    struct early_suspend earlier_suspend;
#endif
	u_char  active_vsync;
	ktime_t vsync_timestamp;
	struct work_struct vsync_work;

	bool				output_on;
	struct mutex		output_lock;
	
#ifdef CONFIG_SYNC_FB
	struct list_head	fb_update_regs_list;
	struct mutex		fb_timeline_lock;	

	struct kthread_worker	fb_update_regs_worker;
	struct task_struct	*fb_update_regs_thread;
	struct kthread_work	fb_update_regs_work;
	
	struct sw_sync_timeline *fb_timeline;
	int fb_timeline_max;

	struct list_head	ext_update_regs_list;
	struct mutex		ext_timeline_lock;	

	struct kthread_worker	ext_update_regs_worker;
	struct task_struct	*ext_update_regs_thread;
	struct kthread_work	ext_update_regs_work;
	
	struct sw_sync_timeline *ext_timeline;
	int ext_timeline_max;
#endif//
};


struct tcc_fenc_reg_data {
	struct list_head	list;
	struct fb_var_screeninfo 	var;	/* Current var */
	u_char *					screen_cpu;	/* virtual address of buffer */
	dma_addr_t				screen_dma;	/* physical address of buffer */
	int						fence_fd;	/* physical address of buffer */
	struct sync_fence *fence;
};

#define PALETTE_BUFF_CLEAR (0x80000000)	/* entry is clear/invalid */

#if defined(CONFIG_TCC_VIDEO_DISPLAY_BY_VSYNC_INT)
#define TCC_VIDEO_DISPLAY_BY_VSYNC_INT
#endif

#if defined(CONFIG_TCC_VIDEO_DISPLAY_DEINTERLACE_MODE)
#define TCC_VIDEO_DISPLAY_DEINTERLACE_MODE
#endif

#if defined(CONFIG_TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT)
#define TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
#endif

//int tccfb_init(void);

#endif
