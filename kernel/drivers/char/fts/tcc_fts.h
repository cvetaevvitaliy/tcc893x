/****************************************************************************
FileName    : kernel/drivers/char/fts/tcc_fts.h
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

#ifndef _FTS_H
#define _FTS_H

#define FTS_DEBUG	0

#if FTS_DEBUG
#define DBG(fmt...)		printk(fmt)
#else
#define DBG(fmt...)
#endif

#if defined(CONFIG_MMC_TCC_SUPPORT_EMMC)
#define FTS_PATH_MISC					"/dev/block/mmcblk0p9"
#define FTS_PATH_TCC					"/dev/block/mmcblk0p10"
#else
#define FTS_PATH_MISC					"/dev/block/ndda9"
#define FTS_PATH_TCC					"/dev/block/ndda10"
#endif

#define FTS_CRASH_COUNT_OFFSET			(512*1024) 	/* 1024th page */
#define FTS_CRASH_COUNT_PAGE_SIZE		(512)
#define FTS_CRASH_COUNT_PAGE_NUM		(256*1024)/512

#define FTS_OUTPUT_SETTING_OFFSET		(512*0) 	/*     0th page */
#define FTS_OUTPUT_SETTING_PAGE_SIZE	(512)
#define FTS_OUTPUT_SETTING_PAGE_NUM		(1024*1024*1)/512

static long fts_ioctl(struct file *, unsigned int , unsigned long );
static int fts_open(struct inode *, struct file *);
static int fts_release(struct inode* , struct file*);

#endif

