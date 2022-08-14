/****************************************************************************
 *   FileName    :  Kernel_nand_drv.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#ifndef __TCC_NDD_H
#define __TCC_NDD_H

//*****************************************************************************
//*
//*
//*                          [ INTERNAL DEFINATION ]
//*
//*
//*****************************************************************************

//===================================================================
// IOCTL
//===================================================================
#define IOCTL_NDD_GET_INFO			_IO(NDD_DEV_MAJOR, 1)
#define IOCTL_NDD_MULTI_HD_WRITE	_IO(NDD_DEV_MAJOR, 2)
#define IOCTL_NDD_MULTI_HD_READ		_IO(NDD_DEV_MAJOR, 3)
#define IOCTL_NDD_BOOT_WRITE		_IO(NDD_DEV_MAJOR, 4)
#define IOCTL_NDD_HD_WRITE			_IO(NDD_DEV_MAJOR, 5)
#define IOCTL_NDD_HD_READ			_IO(NDD_DEV_MAJOR, 6)
#define IOCTL_NDD_ALIGN_CACHE		_IO(NDD_DEV_MAJOR, 7)
#define IOCTL_NDD_GET_SERIALNUMBER  _IO(NDD_DEV_MAJOR, 8)
#define IOCTL_NDD_SET_INTERRUPTMODE _IO(NDD_DEV_MAJOR, 9)
#define IOCTL_NDD_MTD_WRITE			_IO(NDD_DEV_MAJOR, 10)
#define IOCTL_NDD_MTD_INIT			_IO(NDD_DEV_MAJOR, 11)

#define TCC_INTERRUPT_DISABLE	0
#define TCC_INTERRUPT_ENABLE	1

//*****************************************************************************
//*
//*
//*                       [ INTERNAL STRUCT DEFINE ]
//*
//*
//*****************************************************************************
typedef struct _nand_priv_data_t
{
	struct task_struct	*thread;
	struct request_queue	*req;
	struct scatterlist	*sg;
	int			nIrqMode;
	struct semaphore	thread_sem;
} nand_priv_data_t;

struct dma_buf {
	void 		*v_addr;
	unsigned int 	dma_addr;
	int 		buf_size;
};

struct ndd_data {
	loff_t 		i_size;
};

struct ndd_hidden_info {
	unsigned int	DriveNum;	// # of multi-hidden area (0~2) - ONLY multi hidden
	unsigned int  	HiddenAddress;	// start page offset
	unsigned int  	SectorNum;	// write/read page size
	unsigned char 	*DataBuffer;	// buffer
	unsigned int  	DataBufferSize;	// buffer size
};

struct ndd_boot_info {
	char 		*buf;		// bootloader buffer
	unsigned int 	len;		// bootloader length
};

struct ndd_mtd_info{
	unsigned char *DataBuffer;
	unsigned int len;
	unsigned int PageOffset;
};

typedef struct __tag_ndd_dev {
	int			size;
	u8			*data;
	short			users;
	short			media_change;
	struct request_queue 	*queue;
	spinlock_t		lock;
	struct gendisk 		*gd;
	struct timer_list	timer;
	char 			*buf;
	char			*dma_buf_vaddr;
	char			*dma_buf_paddr;
} Device;

#endif
