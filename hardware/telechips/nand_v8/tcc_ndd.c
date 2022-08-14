/* 
 *       TITLE: LINUX NAND DRIVER
 *   CORE TYPE: TCCXXX
 * DRIVER TYPE: kernel module
 *        NAME: tccXXX_nand.ko
 *
 * @Copyright 2008 Telechips Inc. <linux@telechips.com>
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/blkpg.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/scatterlist.h>
#include <asm/mach-types.h>
#include <asm/memory.h>
#include <asm/dma.h>

#include <plat/pmap.h>

#include <mach/globals.h>
#include <plat/nand.h>
#include "nand_def.h"
#include "nand_drv_v8.h"
#include "tnftl_v8_api.h"
#include "nand_io_v8.h"
#include "tcc_ndd.h"
#include "Disk.h"

#include <linux/tcc_ioctl.h>

#define NDD_DEV_NAME		"ndd"
#define NDD_DEV_MAJOR 		240
#define MAX_NDD_DEV			1				// # of NAND device: 1
#define NDD_MAX_PARTITIONS  16				// Max # of partitions: 4

#define QUEUE_MAX_SECTORS	/*64*/128

#define DRIVER_NAME "tcc_nand"

#ifdef CONFIG_HIBERNATION
extern int in_suspend;
extern unsigned int do_hibernation;
#ifdef CONFIG_SNAPSHOT_BOOT
extern unsigned long do_snapshot_boot;
#endif
#endif

#define __USE_TCC_NAND_WORK_Q__

#ifdef __USE_TCC_NAND_WORK_Q__
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/kthread.h>
nand_priv_data_t tcc_nand_data[MAX_NDD_DEV];
#endif

//=============================================================================
//*
//*
//*                           [ CONST DATA DEFINE ]
//*
//*
//=============================================================================
extern struct dma_buf dma_t;
static int ndevices = 2;
module_param(ndevices, int, 0);

//=============================================================================
//*
//*
//*                           [ GLOBAL VARIABLE DEFINE ]
//*
//*
//=============================================================================

//=============================================================================
//*
//*
//*                           [ LOCAL FUNCTIONS DEFINE ]
//*
//*
//=============================================================================
static int 		ndd_ioctl (struct block_device *bdev, fmode_t fmode, unsigned cmd, unsigned long arg);
//static int 		ndd_getgeo(struct block_device *bdev, struct hd_geometry *geo);
static int 		ndd_open(struct block_device *bdev, fmode_t mode);
static int 		ndd_release(struct gendisk *disk, fmode_t mode);
static void 	ndd_request(struct request_queue *q);
static void 	ndd_transfer(Device *dev, unsigned long sector, unsigned long nsect, char *buffer, int write);

static Device 	ndd_dev[MAX_NDD_DEV];
static struct block_device_operations ndd_fops = 
{
	.owner = THIS_MODULE,
	.open = ndd_open,
	.release = ndd_release,
	.ioctl = ndd_ioctl,
	//.getgeo = ndd_getgeo,
};

//=============================================================================
//*
//*
//*                     [ EXTERN VARIABLE & FUNCTIONS DEFINE ]
//*
//*
//=============================================================================
//extern unsigned char *gNAND_PageBuffer;
extern unsigned char gNAND_PartitionInfoLoadFlag;
extern int			 FWUG_MainFunc(char *rom_buf, int iFileSize);

extern int NAND_Ioctl( int function, void *param );
extern void NAND_Suspend(void);
extern void NAND_Resume(void);

#ifdef __USE_TCC_NAND_WORK_Q__
void tcc_ndd_lock( spinlock_t *lock )
{
	#if defined(TCC893X)	
    spin_lock( lock ); 
	#else
	tcc_nand_lock();
	#endif
}

void tcc_ndd_unlock( spinlock_t *lock )
{
	#if defined(TCC893X)	
	    spin_unlock( lock );
	#else
		tcc_nand_unlock();
	#endif
}

static int tcc_nand_issue(void *d)
{
    nand_priv_data_t *nand_priv = (nand_priv_data_t *)d;
	unsigned int 	idx = 0, sector = 0;
	int 			first = 1;
	int 			xfer = 0;
    struct request_queue *q = nand_priv->req;
    struct request *req = NULL;

	down(&nand_priv->thread_sem);	
    do {
		Device *dev;		
		
        if (req==NULL) {
	        set_current_state(TASK_INTERRUPTIBLE);
				n_printk("fetch request\n");
				tcc_ndd_lock(q->queue_lock);
	            req = blk_fetch_request(q);
				tcc_ndd_unlock(q->queue_lock);
	    }

        if (!req) {
            if (kthread_should_stop()) {
                set_current_state(TASK_RUNNING);				
                break;
            }
			up(&nand_priv->thread_sem);
            schedule();
			down(&nand_priv->thread_sem);
            continue;
        }

        set_current_state(TASK_RUNNING);

		n_printk("fetch request succeed!\n");

        dev = req->rq_disk->private_data;
        if (req->cmd_type != REQ_TYPE_FS) {
			tcc_ndd_lock(q->queue_lock);
			if(__blk_end_request(req,-EIO,blk_rq_cur_bytes(req))==false)
			{
				req = NULL;
				first = 1;
			}
			tcc_ndd_unlock(q->queue_lock);
			continue;
        }
		switch(rq_data_dir(req))
		{
		/* read */
            case READ:
				n_printk("******* issue: Read_Sector:[%u]-[%u-%u]\n", (unsigned int)blk_rq_pos(req), blk_rq_sectors(req), blk_rq_cur_sectors(req));

				if (first) {
					idx = 0;
					ndd_transfer(dev, blk_rq_pos(req), blk_rq_sectors(req), dev->buf, 0);
					first = 0;
				}


				memcpy(req->buffer, &dev->buf[idx * 512], blk_rq_cur_bytes(req));
				idx += blk_rq_cur_sectors(req);
				
				if (blk_rq_cur_sectors(req) == blk_rq_sectors(req)) {
					first = 1;
				}				
				
				tcc_ndd_lock(q->queue_lock);
				if(__blk_end_request(req,0,blk_rq_cur_bytes(req))==false)
				{
					req = NULL;
					first = 1;
				}
				tcc_ndd_unlock(q->queue_lock);
				break;

			case WRITE:
				if (first)
				{
                    idx = 0;
                    first = 0;
                    xfer = 0;
					sector = blk_rq_pos(req);
                }

				n_printk("\nWrite_Sector:[%d]-[%lu-%d]",(int)req->sector, req->nr_sectors, req->current_nr_sectors);

                memcpy(&dev->buf[idx * 512], req->buffer, blk_rq_cur_bytes(req));
                idx += blk_rq_cur_sectors(req);

                if (blk_rq_cur_sectors(req) == blk_rq_sectors(req)) {
                    xfer = 1;
                }

                if (xfer) {
                    ndd_transfer(dev, sector, idx, dev->buf, 1);

                    first = 1;
                }

				tcc_ndd_lock(q->queue_lock);
				if(__blk_end_request(req,0,blk_rq_cur_bytes(req))==false)
				{
					req = NULL;
					first = 1;
				}
				tcc_ndd_unlock(q->queue_lock);
				break;

			default:
				tcc_ndd_lock(q->queue_lock);
				if(__blk_end_request(req,-EIO,blk_rq_cur_bytes(req))==false)
				{
					req = NULL;
					first = 1;
				}
				tcc_ndd_unlock(q->queue_lock);
				break;
        }
		up(&nand_priv->thread_sem);
        schedule();
		down(&nand_priv->thread_sem);
    } while (1);

	up(&nand_priv->thread_sem);
	
    return 0;
}
#endif

static int ndd_ioctl(struct block_device *bdev, fmode_t fmode, unsigned cmd, unsigned long arg)
{
    int 	res = 0;

	down(&tcc_nand_data[0].thread_sem);
	
	switch (cmd) {
		case IOCTL_NDD_GET_SERIALNUMBER:
		{
			if ( !NAND_GetSerialNumber((unsigned char* )arg, 32 ))
			{
				res = -EFAULT;
			}

			break;
		}
		case IOCTL_NDD_SET_INTERRUPTMODE:
		{
			unsigned short	mode = *((unsigned short *)arg);
			if ( mode == ENABLE )
			{
#ifdef __USE_TCC_NAND_WORK_Q__
				tcc_nand_data[0].nIrqMode = TCC_INTERRUPT_ENABLE;
#endif
			}
			else
			{
#ifdef __USE_TCC_NAND_WORK_Q__
				tcc_nand_data[0].nIrqMode = TCC_INTERRUPT_DISABLE;
#endif
			}
			break;
		}
		case IOCTL_STORAGE_PING:
		{
			// ping
			//break;
			res = 1;
			break;
		}
		case IOCTL_STORAGE_DIRECT_READ:
		{
			struct storage_direct sdArg;
			unsigned int sector, nsect;
			void *buf=NULL;
			sector_t start_sect, nr_sects;

			//printk("[tcc_ioctl:Read - \n");

			if (copy_from_user(&sdArg, (void __user *)arg, sizeof(struct storage_direct))) {
				printk("[tcc_nand] IOCTL_STORAGE_DIRECT_READ copy_from_user(struct) fail\n");
				res = -EFAULT;
			}

			sector = sdArg.pos>>9;
			nsect = sdArg.count>>9;
			start_sect = bdev->bd_part->start_sect;	/* start sector of partition */
			nr_sects = bdev->bd_part->nr_sects;		/* # sector of partition */
			sector += start_sect;

			if (sdArg.user_space) {
				printk("[tcc_nand] IOCTL_STORAGE_DIRECT_READ access from user space is not supported yet.\n");
				res = -EFAULT;
			} else {
				buf = sdArg.buf;
			}
			if ((sector + nsect) > (start_sect + nr_sects)) {
				printk("beyond-end read (%d + %d >= %lu)\n", sector, nsect, (long unsigned int)(start_sect + nr_sects));
				res = -EFAULT;
			}

			#if defined(__USE_NAND_ISR__)
			NAND_IO_IRQ_Enable(1);
			#endif

			res = NAND_ReadSector(0, sector, nsect, buf);

			#if defined(__USE_NAND_ISR__)
			NAND_IO_IRQ_Enable(0);
			#endif
			
			if ( res != 0 ) {
				printk("[tcc_nand] IOCTL_STORAGE_DIRECT_READ read fail\n");
				res = -EFAULT;
			}

			sdArg.pos += sdArg.count;
			sdArg.actual = sdArg.count;
			if (copy_to_user((void __user *)arg, &sdArg, sizeof(struct storage_direct))) {
				printk("[tcc_nand] IOCTL_STORAGE_DIRECT_READ copy_to_user(struct) fail\n");
				res = -EFAULT;
			}

			break;
		}
		case IOCTL_STORAGE_DIRECT_WRITE:
		{
			struct storage_direct sdArg;
			unsigned int sector, nsect;
			void *buf = NULL;
			sector_t start_sect, nr_sects;

			//printk("[tcc_ioctl:Write - \n");

			if (copy_from_user(&sdArg, (void __user *)arg, sizeof(struct storage_direct))) {
				printk("[tcc_nand] IOCTL_STORAGE_DIRECT_WRITE copy_from_user(struct) fail\n");
				res = -EFAULT;
			}

			sector = sdArg.pos>>9;
			nsect = sdArg.count>>9;
			start_sect = bdev->bd_part->start_sect;	/* start sector of partition */
			nr_sects = bdev->bd_part->nr_sects;		/* # sector of partition */
			sector += start_sect;

			if (sdArg.user_space) {
				printk("[tcc_nand] IOCTL_STORAGE_DIRECT_WRITE access from user space is not supported yet.\n");
				res = -EFAULT;
			} else {
				buf = sdArg.buf;
			}
			if ((sector + nsect) > (start_sect + nr_sects)) {
				printk("beyond-end write (%d + %d >= %lu)\n", sector, nsect, (long unsigned int)(start_sect + nr_sects));
				res = -EFAULT;
			}

			#if defined(__USE_NAND_ISR__)
			NAND_IO_IRQ_Enable(1);
			#endif
			
			res = NAND_WriteSector(0, sector, nsect, buf);

			#if defined(__USE_NAND_ISR__)
			NAND_IO_IRQ_Enable(0);
			#endif
			
			if ( res != 0 ) {
				printk("[tcc_nand] IOCTL_STORAGE_DIRECT_WRITE write fail\n");
				res = -EFAULT;
			}

			sdArg.pos += sdArg.count;
			sdArg.actual = sdArg.count;
			if (copy_to_user((void __user *)arg, &sdArg, sizeof(struct storage_direct)))
			{
				printk("[tcc_nand] IOCTL_STORAGE_DIRECT_WRITE copy_to_user(struct) fail\n");
				res = -EFAULT;
			}

			break;
		}
		default:
			printk("[tcc_nand] ioctl (0x%x)\n", cmd); 
			res = -ENOIOCTLCMD;
			break;
	}
	up(&tcc_nand_data[0].thread_sem);
	
	return res;
}

static int ndd_open(struct block_device *bdev, fmode_t mode)
{
	Device *dev = bdev->bd_disk->private_data;

	spin_lock(&dev->lock);
	dev->users++;
	spin_unlock(&dev->lock);

	return 0;
}

static int ndd_release(struct gendisk *disk, fmode_t mode)
{
	Device *dev = disk->private_data;

	spin_lock(&dev->lock);
	dev->users--;
	spin_unlock(&dev->lock);

	return 0;
}

#ifdef __USE_NAND_ISR__
extern void NAND_IO_ISR(void);
static irqreturn_t nand_isr2(int irq, void *dev_id)
{
	//printk("ni+");
	NAND_IO_ISR();
	//printk("ni-");
	return IRQ_HANDLED;
}

#endif

static void ndd_request(struct request_queue *q)
{
#ifdef __USE_TCC_NAND_WORK_Q__
    nand_priv_data_t *nq = (nand_priv_data_t *)q->queuedata;
    wake_up_process(nq->thread);
#endif
}

static void ndd_transfer(Device *dev, unsigned long sector,
		unsigned long nsect, char *buffer, int write)
{	
	if( (sector + nsect) > dev->size ) {
		printk(KERN_NOTICE "beyond-end write (%lu %lu %d)\n",
				sector, nsect, dev->size);
		return;
	}

	#if defined(__USE_NAND_ISR__)
	NAND_IO_IRQ_Enable(1);
	#endif

	if(write) {
		NAND_WriteSector(0, sector, nsect, buffer);
	} else {
		NAND_ReadSector(0, sector, nsect, buffer);
	}

	#if defined(__USE_NAND_ISR__)
	NAND_IO_IRQ_Enable(0);
	#endif

}

extern void* NAND_IO_GetISRStructure(void);	
pmap_t	pmap;
static int __init ndd_init(struct platform_device *pdev)
{
	int res, i;
    int ret = 0;

	pr_info("%s: initializing NAND block device driver\n", __func__);
	
	// Get DMA Buffer
	#if defined(TCC892X) || defined(TCC8925S)
	dma_t.buf_size = 16384;
	dma_t.v_addr = dma_alloc_coherent(0, dma_t.buf_size, &dma_t.dma_addr, GFP_KERNEL);
	#elif defined(TCC893X)
	ret = pmap_get_info("nand_mem", &pmap );
	if( ret )
		printk("[NAND_V8] Success to get PMAP Buffer \n");
	else
		printk("[NAND_V8] Get pmap ERROR !!!!!\n");

	dma_t.buf_size = PAGE_ALIGN(pmap.size);
	dma_t.dma_addr = pmap.base;
	dma_t.v_addr   = ioremap_wc( dma_t.dma_addr, dma_t.buf_size );
	#endif

	for (i = 0; i < MAX_NDD_DEV ; i++)
	{
		if(NAND_Init(i,NAND_INIT_TYPE_NORMAL,NULL) != NAND_SUCCESS)
		{
			printk("NAND_Init() is failed.\n");
			return -1;
		}

		res = register_blkdev(NDD_DEV_MAJOR, NDD_DEV_NAME);
		if (res < 0) {
			printk(KERN_WARNING "ndd: unable to get major number.\n");
            goto out;
		}

		{
			unsigned long ulTotalSector = 0;
			NAND_Ioctl(DEV_GET_DISK_SIZE, &ulTotalSector);
			ndd_dev[i].size = ulTotalSector;
		}
		ndd_dev[i].data = kmalloc(sizeof(Device), GFP_KERNEL);	//size should be less than 128*1024
		ndd_dev[i].buf = kmalloc((sizeof(char) * QUEUE_MAX_SECTORS * 512), GFP_KERNEL);
		ndd_dev[i].dma_buf_paddr = (char*)dma_t.dma_addr;
		ndd_dev[i].dma_buf_vaddr = dma_t.v_addr;

		if (!ndd_dev[i].data || 
			!ndd_dev[i].buf) 
		{
			printk("ndd: Unable to allocate NAND device structure.\n");
            goto out;
		}

		memset(ndd_dev[i].data, 0, sizeof(Device));

		// init gendisk
		ndd_dev[i].gd = alloc_disk(NDD_MAX_PARTITIONS);
		if (!ndd_dev[i].gd) {
			printk("ndd: allocate gendisk failed.\n");
            goto out;
		}

		spin_lock_init(&(ndd_dev[i].lock));
		ndd_dev[i].queue 			= blk_init_queue(ndd_request, &(ndd_dev[i].lock));
		blk_queue_max_hw_sectors(ndd_dev[i].queue, QUEUE_MAX_SECTORS);
		blk_queue_logical_block_size(ndd_dev[i].queue, 512);
		ndd_dev[i].gd->major 		= NDD_DEV_MAJOR;
		ndd_dev[i].gd->first_minor 	= i * NDD_MAX_PARTITIONS;
		ndd_dev[i].gd->fops 		= &ndd_fops;
		ndd_dev[i].gd->private_data = &ndd_dev[i];
		sprintf(ndd_dev[i].gd->disk_name, "ndd%c", 'a' + i);
		set_capacity(ndd_dev[i].gd, ndd_dev[i].size);
		ndd_dev[i].gd->queue 		= ndd_dev[i].queue;
		ndd_dev[i].gd->driverfs_dev = &pdev->dev;

#if defined(__USE_NAND_ISR__)
	{

		NAND_IO_IRQ_Init(1);
		
		ret = request_irq(NAND_IRQ_NFC,
						  nand_isr2,
						  IRQF_DISABLED,   	/* flags */\
						  "nand-isr",    	/* in /proc/interrupts */
						  NAND_IO_GetISRStructure() );
						  //1); 		/* user data passed to ISR */

		if (ret) {
			printk("[%s:%d] cannot request irq !!!\n", __func__, __LINE__);
			return -1;
		}
	}

#endif //defined(__USE_NAND_ISR__)

#ifdef __USE_TCC_NAND_WORK_Q__		
		tcc_nand_data[i].nIrqMode = TCC_INTERRUPT_DISABLE;
        tcc_nand_data[i].req = ndd_dev[i].queue;
        ndd_dev[i].queue->queuedata = &(tcc_nand_data[i]);
		
		// add gendisk
		tcc_nand_data[i].sg = kmalloc(sizeof(struct scatterlist) *
			128, GFP_KERNEL);
		if (!tcc_nand_data[i].sg) {
			ret = -ENOMEM;
			goto out;
		}
		sg_init_table(tcc_nand_data[i].sg, 128);		
		sema_init(&tcc_nand_data[i].thread_sem, 1);	
		
        tcc_nand_data[i].thread = kthread_run(tcc_nand_issue, &(tcc_nand_data[i]), ndd_dev[i].gd->disk_name);
        if (IS_ERR(tcc_nand_data[i].thread)) {
            printk("[%s:%d] cannot run kernel_thread !!!! \n", __func__, __LINE__);
            goto out;
        }
#endif
		add_disk(ndd_dev[i].gd);
	}

	printk("[tcc_nand] init ndd\n");
	return 0;

out:
    if (dma_t.v_addr) {
		#if defined(TCC892X) || defined(TCC8925S)
        dma_free_writecombine(0, dma_t.buf_size, dma_t.v_addr, dma_t.dma_addr);
		#else
		iounmap(dma_t.v_addr);
		#endif
    }
    memset(&dma_t, 0, sizeof(struct dma_buf));

    for (i = 0; i < MAX_NDD_DEV; i++) {
#ifdef __USE_TCC_NAND_WORK_Q__
        if (tcc_nand_data[i].thread) {
            kthread_stop(tcc_nand_data[i].thread);
            tcc_nand_data[i].thread = NULL;
        }
        memset(&(tcc_nand_data[i]), 0, sizeof(nand_priv_data_t));
#endif
        if (ndd_dev[i].data) { kfree(ndd_dev[i].data); }
        if (ndd_dev[i].buf) { kfree(ndd_dev[i].buf); }
        memset(&(ndd_dev[i]), 0, sizeof(Device));
    }

    return -1;
}

static void __exit ndd_exit(void)
{
	int i;
	printk("[tcc_nand]: exit ndd\n");

	#ifdef __USE_TCC_NAND_WORK_Q__
    for (i = 0; i < MAX_NDD_DEV; i++) {
        if (tcc_nand_data[i].thread) {
            kthread_stop(tcc_nand_data[i].thread);
            tcc_nand_data[i].thread = NULL;
        }
        memset(&(tcc_nand_data[i]), 0, sizeof(nand_priv_data_t));
    }
	#endif
	
	for(i = 0; i < MAX_NDD_DEV; i++) {	
		del_gendisk(ndd_dev[i].gd);
		put_disk(ndd_dev[i].gd);
		blk_cleanup_queue(ndd_dev[i].queue);
		kfree(ndd_dev[i].data);
		kfree(ndd_dev[i].buf);
	}

	#if defined(__USE_NAND_ISR__)
    NAND_IO_IRQ_Init(0);
    free_irq(NAND_IRQ_NFC, NAND_IO_GetISRStructure());
	#endif

    if (dma_t.v_addr) {
		#if defined(TCC892X) || defined(TCC8925S)
        dma_free_writecombine(0, dma_t.buf_size, dma_t.v_addr, dma_t.dma_addr);
		#else
		iounmap(dma_t.v_addr);
		#endif
    }
    memset(&dma_t, 0, sizeof(struct dma_buf));
	unregister_blkdev(NDD_DEV_MAJOR, NDD_DEV_NAME);
}

static int tcc_nand_v8_suspend(struct platform_device *pdev, pm_message_t state )
{

	pr_debug("%s\n", __func__);
	NAND_Suspend();	
	return 0;
}

static int tcc_nand_v8_resume(struct platform_device *pdev)
{
	pr_debug("%s\n", __func__);

	#ifdef CONFIG_HIBERNATION
	if( do_hibernation
		#ifdef CONFIG_SNAPSHOT_BOOT
            && do_snapshot_boot
        #endif
       ) 
    {
		//printk("!NAND Moult Init! - Hibernate! \n\n");
		NAND_Ioctl(0, NULL);
	} 
	else 
	#endif
	{
		//printk("!NAND Moult Init! - Normal Suspend! \n\n");
		NAND_Resume();
	}

	return 0;
}

int tcc_nand_read_sector( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer )
{
	int res;

	down(&tcc_nand_data[0].thread_sem);
	res = NAND_Area_ReadSector( uiDrvNum, uiAreaIndex, uiSectorAddress, uiSectorCount, pvBuffer );
	up(&tcc_nand_data[0].thread_sem);

	return res;
}

EXPORT_SYMBOL(tcc_nand_read_sector);

int tcc_nand_write_sector( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer )
{
	int res;

	down(&tcc_nand_data[0].thread_sem);

	res = NAND_Area_WriteSector(uiDrvNum, uiAreaIndex, uiSectorAddress, uiSectorCount, pvBuffer );

	up(&tcc_nand_data[0].thread_sem);

	return res;
}

EXPORT_SYMBOL(tcc_nand_write_sector);

static int __devinit tcc_nand_v8_probe(struct platform_device *pdev)
{
	ndd_init(pdev);
	return 0;
}

static int __devexit tcc_nand_v8_remove(struct platform_device *pdev)
{
	ndd_exit();
	return 0;
}

static struct platform_driver tcc_nand_v8_driver = {
	.probe		= tcc_nand_v8_probe,
	.remove		= __devexit_p(tcc_nand_v8_remove),
	.suspend	= tcc_nand_v8_suspend,
	.resume		= tcc_nand_v8_resume,
	.driver		= {
		.name		= DRIVER_NAME,
	},
};

static int __init tcc_nand_v8_init(void)
{
	return platform_driver_register(&tcc_nand_v8_driver);
}

static void __exit tcc_nand_v8_exit(void)
{
	platform_driver_unregister(&tcc_nand_v8_driver);
}

module_init(tcc_nand_v8_init);
module_exit(tcc_nand_v8_exit);

MODULE_AUTHOR("Telechips Inc. linux@telechips.com");
MODULE_DESCRIPTION("Telechips NAND driver");
MODULE_LICENSE("GPL");