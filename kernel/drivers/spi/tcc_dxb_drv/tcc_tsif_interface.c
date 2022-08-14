/*
 *  Driver for TCC TSIF
 *
 *  Written by C2-G1-3T
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.=
 */

#include <linux/spi/tcc_tsif.h>
#include <linux/interrupt.h>

#if defined(CONFIG_ARCH_TCC893X)
    #if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
        #define SUPPORT_HARDWARE_DEMUX
    #endif
#endif

#ifdef SUPPORT_HARDWARE_DEMUX
#include <mach/tca_hwdemux_tsif.h>
#else
#include <mach/tca_tsif.h>
#endif

#include "tcc_dmx.h"
#include "tcc_tsif_interface.h"
#include "tcc_dxb_kernel.h"


/*****************************************************************************
 *
 * Defines
 *
 ******************************************************************************/
#if 0
#define dprintk(msg...)	 { printk( "[tcc_tsif_interface]" msg); }
#else
#define dprintk(msg...)	 
#endif

#define	TSIF_PACKETSIZE             188
#define TSIF_INT_PACKETCOUNT        1       //39	//int packet 13*3

#ifdef SUPPORT_HARDWARE_DEMUX
#define TSIF_MAX_PACKETCOUNT        8190    //max packet 0x1fff-1 = 8190 = 2*3*3*5*7*13
#else
#define TSIF_MAX_PACKETCOUNT        5577    //max packet 0x1fff-1 = 8190 = 2*3*3*5*7*13
#endif

/*****************************************************************************
 *
 * structures
 *
 ******************************************************************************/


/*****************************************************************************
 *
 * Variables
 *
 ******************************************************************************/
static const char fw_name[] = "tcc_dxb_drv/tcc_dxb_drv.fw";
static struct tcc_dxb_instance *gtcc_dxb = NULL;


/*****************************************************************************
 *
 * External Functions
 *
 ******************************************************************************/
extern int tcc_tsif_set_external_tsdemux(struct file *filp, int (*decoder)(char *p1, int p1_size, char *p2, int p2_size, int devid), int max_dec_packet, int devid);
extern int tcc_tsif_open(struct inode *inode, struct file *filp);
extern int tcc_tsif_release(struct inode *inode, struct file *filp);
extern int tcc_export_tsif_ioctl(struct file *filp, unsigned int cmd, unsigned long arg, int idevid);


 /*****************************************************************************
 *
 * Functions
 *
 ******************************************************************************/
//#define TS_PACKET_CHK_MODE
#ifdef TS_PACKET_CHK_MODE
typedef	struct {
	struct ts_packet_chk_t* next;
	
	unsigned short pid;
	unsigned char cc;
	
	unsigned long long cnt;
	unsigned int err;
} ts_packet_chk_t;


typedef	struct {
	unsigned long long total_cnt;
	unsigned int total_err;
	unsigned int packet_cnt;

	#define DEBUG_CHK_TIME		10	//sec
	unsigned int cur_time;
	unsigned int prev_time;
	unsigned int debug_time;
	struct timeval time;

  	ts_packet_chk_t* packet;
} ts_packet_chk_info_t;
ts_packet_chk_info_t*	ts_packet_chk_info = NULL;


static void itv_ts_cc_debug(int mod) 
{
	if(ts_packet_chk_info != NULL) {
		if(ts_packet_chk_info->packet != NULL) {
			printk("\n[total:%llu / err:%d (%d sec)]\n", ts_packet_chk_info->total_cnt, ts_packet_chk_info->total_err, (ts_packet_chk_info->debug_time * DEBUG_CHK_TIME));
			
			if(mod) {
				ts_packet_chk_t* tmp = NULL;
				
				tmp = ts_packet_chk_info->packet;
				do {
					printk("\t\tpid:0x%04x => cnt:%llu err:%d\n", tmp->pid, tmp->cnt, tmp->err);
					tmp = tmp->next;
				} while(tmp != NULL);
			}
		}
	}
}

static void itv_ts_cc_remove(unsigned short pid)
{
	ts_packet_chk_t* tmp = NULL;
	ts_packet_chk_t* tmp_prev = NULL;

	if(ts_packet_chk_info->packet) {
		tmp = ts_packet_chk_info->packet;
		while(1) {
			if(tmp->pid == pid) {
				if(tmp->next == NULL) {
					if(tmp_prev == NULL) {		//1st
						ts_packet_chk_info->packet = NULL;
						kfree(tmp);
					} else {					//last
						tmp_prev->next = NULL;
						kfree(tmp);
					}
				} else {
					tmp_prev = tmp;
					tmp = tmp->next;

					ts_packet_chk_info->packet = tmp;
					kfree(tmp_prev);
				}
				
				break;
			} else {
				tmp_prev = tmp;
				tmp = tmp->next;

				if(tmp == NULL) {
					break;
				}
			}
		}
	}
}

static void itv_ts_cc_check(unsigned char* buf)
{
	unsigned short pid;
	unsigned char cc;

	ts_packet_chk_t* tmp = NULL;

	pid = ((*(unsigned char*)(buf + 1) & 0x1f) << 8) | *(unsigned char*)(buf + 2);
	cc 	= *(unsigned char*)(buf + 3) & 0x0f;
	
	if(ts_packet_chk_info != NULL) {
		ts_packet_chk_info->total_cnt++;

		if(ts_packet_chk_info->packet == NULL) {
			tmp = (ts_packet_chk_t*)kmalloc(sizeof(ts_packet_chk_t), GFP_ATOMIC);
			if(tmp == NULL) {
				printk("\t ts_packet_chk_t mem alloc err..\n");
			}

			memset(tmp, 0x0, sizeof(ts_packet_chk_t));
			tmp->pid = pid;
			tmp->cc	= cc;
			tmp->next = NULL;
			tmp->cnt++;
			ts_packet_chk_info->packet = tmp;
			ts_packet_chk_info->packet_cnt++;
			
			printk("\t>>>> create[%d] : 0x%04x / %02d\n", ts_packet_chk_info->packet_cnt, tmp->pid, tmp->cc);
		} else {
			unsigned char new = 0;
			unsigned int temp;
	
			tmp = ts_packet_chk_info->packet;
			while(1) {
				if(tmp->pid == pid) {
					tmp->cnt++;

					if(tmp->pid != 0x1fff) {
						if(cc > tmp->cc) {
							temp = tmp->err;
							tmp->err += ((cc - tmp->cc + 0xf) % 0xf) - 1;
							if(temp != tmp->err) {
								ts_packet_chk_info->total_err += tmp->err - temp;
								printk("\t(%dmin) pid:0x%04x => cnt:%llu err:%d [%d -> %d]\n", ts_packet_chk_info->debug_time, tmp->pid, tmp->cnt, tmp->err, tmp->cc, cc);
							}
							tmp->cc = cc;
						} else if(cc < tmp->cc) {
							temp = tmp->err;
							tmp->err += (0x10 - tmp->cc + cc) - 1;
							if(temp != tmp->err) {
								ts_packet_chk_info->total_err += tmp->err - temp;
								printk("\t(%dmin) pid:0x%04x => cnt:%llu err:%d [%d -> %d]\n", ts_packet_chk_info->debug_time, tmp->pid, tmp->cnt, tmp->err, tmp->cc, cc);
							}
							tmp->cc = cc;
						}
					}
					break;
				} else {
					tmp = tmp->next;
					
					if(tmp == NULL) {
						new = 1;
						break;
					}
				}
			}

			if(new) {
				ts_packet_chk_t* tmp_ = NULL;
	
				tmp = (ts_packet_chk_t*)kmalloc(sizeof(ts_packet_chk_t), GFP_ATOMIC);
				if(tmp == NULL) {
					printk("\t ts_packet_chk_t mem alloc err..\n");
				}

				memset(tmp, 0x0, sizeof(ts_packet_chk_t));
				tmp->pid = pid;
				tmp->cc	= cc;
				tmp->next = NULL;
				tmp->cnt++;
				
				ts_packet_chk_info->packet_cnt++;
				tmp_ = ts_packet_chk_info->packet;
				do {
					if(tmp_->next == NULL) {
						tmp_->next= tmp;
						break;
					} else {
						tmp_ = tmp_->next;
					}
				} while(1);

				printk("\t>>>> create[%d] : 0x%04x / %02d\n", ts_packet_chk_info->packet_cnt, tmp->pid, tmp->cc);
			}
		}

		do_gettimeofday(&ts_packet_chk_info->time);
		ts_packet_chk_info->cur_time = ((ts_packet_chk_info->time.tv_sec * 1000) & 0x00ffffff) + (ts_packet_chk_info->time.tv_usec / 1000);
		if((ts_packet_chk_info->cur_time - ts_packet_chk_info->prev_time) > (DEBUG_CHK_TIME * 1000)) {
			//itv_ts_cc_debug(0);
			itv_ts_cc_debug(1);

			ts_packet_chk_info->prev_time = ts_packet_chk_info->cur_time;
			ts_packet_chk_info->debug_time++;
		}
	}
}
#endif	//#ifdef TS_PACKET_CHK_MODE


static void tcc_dxb_ts_callback(char *p1, int p1_size, char *p2, int p2_size, int devid)
{
    struct dvb_demux *demux;
    int i;

	if (gtcc_dxb == NULL)
		return;

#ifdef TS_PACKET_CHK_MODE
        {
            int i;
            if(p1 && p1_size)
            {
                for(i=0; i<p1_size; i+= 188)
    				itv_ts_cc_check(p1+i);
            }

            if(p2 && p2_size)
            {
                for(i=0; i<p2_size; i+= 188)
    				itv_ts_cc_check(p2+i);
            }
        }
#endif
    for (i=0; i<2; i++)
    {
        if(gtcc_dxb->dmx[i].tsif_id != devid)
        {
            continue;
        }

        demux = &gtcc_dxb->dmx[i].demux;

        if(demux->dmx.frontend->source != DMX_MEMORY_FE && demux->users > 0)
        {
            if(p1 != NULL && p1_size > 0)
            {
                if(p1[0] != 0x47)
                {
                    dprintk("packet error 1... [%x]\n", p1[0]);
#ifndef SUPPORT_HARDWARE_DEMUX
                    tcc_export_tsif_ioctl(0, IOCTL_TSIF_RESET, 0, 0);
#endif
                }
                else
                {
                    /* no wraparound, dump olddma..newdma */
                    dvb_dmx_swfilter_packets(demux, p1, p1_size / 188);
                }
            }
            if(p2 != NULL && p2_size > 0)
            {
                if(p2[0] != 0x47)
                {
                    dprintk("packet error 2... [%x]\n", p2[0]);
#ifndef SUPPORT_HARDWARE_DEMUX
                    tcc_export_tsif_ioctl(0, IOCTL_TSIF_RESET, 0, 0);
#endif
                }
                else
                {
                    dvb_dmx_swfilter_packets(demux, p2, p2_size / 188);
                }
            }
        }
    }    
}

static void tcc_dxb_sec_callback(unsigned int fid, int crc_err, char *p, int size, int devid)
{
    struct dvb_demux *demux;
    struct dvb_demux_filter *dmxfilter;
    struct dvb_demux_feed *feed;
    struct dmx_section_filter *secfilter;
    int i;

	if (gtcc_dxb == NULL)
		return;

    if(p != NULL && size > 0)
    {
        for (i=0; i<2; i++)
        {
            if(gtcc_dxb->dmx[i].tsif_id != devid)
            {
                continue;
            }

            demux = &gtcc_dxb->dmx[i].demux;

            if(demux->dmx.frontend->source != DMX_MEMORY_FE && demux->users > 0)
            {
                dmxfilter = &demux->filter[fid];
                if (dmxfilter->state == DMX_STATE_GO)
                {
                    secfilter = &dmxfilter->filter;
                    if(dmxfilter->doneq)
                    {
                        unsigned char xor, neq=0;
                        for (i = 0; i < DVB_DEMUX_MASK_MAX; i++) {
                            xor = secfilter->filter_value[i] ^ p[i];
                            neq |= dmxfilter->maskandnotmode[i] & xor;
                        }
                        if (!neq)
                            return; //filter not matched
                    }

                    feed = dmxfilter->feed;
                    if (feed->type == DMX_TYPE_SEC && feed->state == DMX_STATE_GO)
                    {
                        if (!feed->feed.sec.check_crc || !crc_err)
                        {
                            feed->cb.sec(p, size, NULL, 0, secfilter, DMX_OK);
                        }
                    }
                }
            }
        }
    }
}

static void tcc_dxb_running_feed(long unsigned int devid)
{
    struct tcc_tsif_instance *pHandle;

    char *p1, *p2;
    int p1_size, p2_size;
	int index;
	unsigned long flags;
    struct packet_info *p_info;

	if (gtcc_dxb == NULL)
		return;

	pHandle = &gtcc_dxb->tsif[devid];

    spin_lock_irqsave(&pHandle->ts_packet_list.lock, flags);
	for(index=0; index<TS_PACKET_LIST; index++)
    {
        if(list_empty(&pHandle->ts_packet_bank))
            break;
        p_info = list_first_entry(&pHandle->ts_packet_bank, struct packet_info, list);    
        if(p_info == NULL)
        {
            dprintk("%s:invalid NULL packet !!!\n", __func__);
            break;
        }
        list_del(&p_info->list);

        if(p_info->valid != 1)
        {
            dprintk("%s:invalid packet !!!\n", __func__);
            p_info->valid = 0;
            continue;
        }
        p1 = p_info->p1;
        p1_size = p_info->p1_size;
        p2 = p_info->p2;
        p2_size = p_info->p2_size;
        p_info->valid = 0;

#ifdef SUPPORT_HARDWARE_DEMUX
        if (p1_size < 188) // Section - p1_size : 0(crc ok) or 1(crc err)
        {
            tcc_dxb_sec_callback((unsigned int)p1, p1_size, p2, p2_size, devid);
        }
        else
        {
            tcc_dxb_ts_callback(p1, p1_size, p2, p2_size, devid);
        }
#else
        tcc_dxb_ts_callback(p1, p1_size, p2, p2_size, devid);
#endif
    }
    spin_unlock_irqrestore(&pHandle->ts_packet_list.lock, flags);
}

static int tcc_tsif_parse_packet(char *p1, int p1_size, char *p2, int p2_size, int devid)
{
    struct tcc_tsif_instance *pHandle;
    int index;
    struct packet_info *p_info;

	if (gtcc_dxb == NULL)
		return -1;

	pHandle = &gtcc_dxb->tsif[devid];
	index = pHandle->ts_packet_list.current_index;
	p_info = &pHandle->ts_packet_list.ts_packet[index];

    if(p_info->valid == 0)
    {
        p_info->p1 = p1;
        p_info->p1_size = p1_size;

        p_info->p2 = p2;
        p_info->p2_size = p2_size;

        p_info->valid = 1;

        list_add_tail(&p_info->list, &pHandle->ts_packet_bank);
        if(++index >= TS_PACKET_LIST)
            index = 0;
        pHandle->ts_packet_list.current_index = index;
        tasklet_schedule(&pHandle->tsif_tasklet);    
    }
    else
    {
        dprintk("%s:no space in packet_bank !!!\n", __func__);
    }
    return 0;
}

int tcc_tsif_init(struct tcc_tsif_instance *tsif)
{
    gtcc_dxb = tcc_dxb_get_instance();

    memset(tsif, 0x0, sizeof(struct tcc_tsif_instance));
    mutex_init(&tsif->mutex);

    return 0;
}

int tcc_tsif_deinit(struct tcc_tsif_instance *tsif)
{
    return 0;
}

static int tcc_tsif_interface_init(unsigned int devid, struct device *device)
{
	struct inode inode;
	struct tcc_tsif_instance *pHandle;
	int err = 0;

	if (gtcc_dxb == NULL)
		return -1;

	pHandle = &gtcc_dxb->tsif[devid];

    mutex_lock(&pHandle->mutex);
    pHandle->users++;
    if (pHandle->users == 1)
    {
        pHandle->filp.private_data = NULL;
        inode.i_rdev = MKDEV(0, devid);
        err = tcc_tsif_open(&inode, &pHandle->filp);
        if (err == 0)
        {
            memset(&pHandle->pid_table, 0x0, sizeof(struct pid_table));
            memset(&pHandle->ts_packet_list, 0x0, sizeof(struct packet_list));

            INIT_LIST_HEAD(&pHandle->ts_packet_bank);
            spin_lock_init(&pHandle->ts_packet_list.lock);

            tasklet_init(&pHandle->tsif_tasklet, tcc_dxb_running_feed, devid);

            tcc_tsif_set_external_tsdemux(&pHandle->filp, tcc_tsif_parse_packet, 1, 0);

#ifdef TS_PACKET_CHK_MODE
            ts_packet_chk_info = (ts_packet_chk_info_t*)kmalloc(sizeof(ts_packet_chk_info_t), GFP_ATOMIC);
            if(ts_packet_chk_info == NULL) {
                printk("\t ts_packet_chk_info_t mem alloc err..\n");
            }
        	memset(ts_packet_chk_info, 0x0, sizeof(ts_packet_chk_info_t));
#endif	    
            dprintk("Oepn TSIF Driver[ID=%d]\n", devid);
        }
        else
        {
            pHandle->users = 0;
            dprintk("Failed to open tsif driver[ID=%d, Err=%d]\n", devid, err);
        }
    }

    mutex_unlock(&pHandle->mutex);

    return err;
}

static int tcc_tsif_interface_deinit(unsigned int devid)
{
    struct inode inode;
    struct tcc_tsif_instance *pHandle;
    int err = 0;

	if (gtcc_dxb == NULL)
		return -1;

	pHandle = &gtcc_dxb->tsif[devid];

    if (pHandle->users == 0)
        return -1;

    mutex_lock(&pHandle->mutex);
    pHandle->users--;
    if (pHandle->users == 0)
    {
        tcc_tsif_set_external_tsdemux(&pHandle->filp, tcc_tsif_parse_packet, 0, 0);
        tasklet_kill(&pHandle->tsif_tasklet);
        inode.i_rdev = MKDEV(0, devid);
        err = tcc_tsif_release(&inode, &pHandle->filp);
        if (err == 0)
        {
            dprintk("Close TSIF Driver[ID=%d]\n", devid);
        }
        else
        {
            pHandle->users = 1;
            dprintk("Failed to close tsif driver[ID=%d, Err=%d]\n", devid, err);
        }
#ifdef TS_PACKET_CHK_MODE
{
        ts_packet_chk_t* tmp = NULL;
        ts_packet_chk_t* tmp_ = NULL;

        if(ts_packet_chk_info != NULL) {
            if(ts_packet_chk_info->packet != NULL) {

                itv_ts_cc_debug(1);
                
                tmp = ts_packet_chk_info->packet;
                do {
                    tmp_ = tmp;
                    tmp = tmp->next;
                    kfree(tmp_);
                } while(tmp != NULL);
            }
            kfree(ts_packet_chk_info);
            ts_packet_chk_info = NULL;
	    }
}
#endif    
    }

    mutex_unlock(&pHandle->mutex);

    return err;
}

int tcc_tsif_interface_open(struct device *device)
{
    tcc_tsif_interface_init(0, device);
#ifdef SUPPORT_HARDWARE_DEMUX
    tcc_tsif_interface_init(1, device);
#endif
    return 0;
}

int tcc_tsif_interface_close(void)
{
    tcc_tsif_interface_deinit(0);
#ifdef SUPPORT_HARDWARE_DEMUX
    tcc_tsif_interface_deinit(1);
#endif
    return 0;
}

static int tcc_tsif_hwdmx_can_support(void)
{
#ifdef SUPPORT_HARDWARE_DEMUX
    return tca_tsif_can_support();
#else
    return false;
#endif
}

static int tcc_tsif_set_pid_for_hwdmx(struct tcc_tsif_instance *pHandle, struct dvb_demux_feed *feed)
{
#ifdef SUPPORT_HARDWARE_DEMUX
    struct tcc_tsif_filter param;
    int i, j = MAX_PID_TABLE_CNT;

    switch (feed->type)
    {
        case DMX_TYPE_TS:
        {
            param.f_type = 1; // HW_DEMUX_TS
            param.f_id = 0;
            param.f_pid = feed->pid;
            param.f_size = 0;
            for (i = 0; i < MAX_PID_TABLE_CNT; i++)
            {
                if (pHandle->pid_table.ts_users[i])
                {
                    if (pHandle->pid_table.ts_pid[i] == param.f_pid)
                    {
                        pHandle->pid_table.ts_users[i]++;
                        mutex_unlock(&pHandle->mutex);
                        return 0;
                    }
                }
                else if (i < j)
                {
                    j = i;
                }
            }
            if (j == MAX_PID_TABLE_CNT)
            {
               dprintk("Failed to add filter[pid = %d]\n", param.f_pid);
               mutex_unlock(&pHandle->mutex);
               return -1;
            }
            pHandle->pid_table.ts_pid[j] = param.f_pid;
            pHandle->pid_table.ts_users[j]++;
            if (tcc_export_tsif_ioctl(&pHandle->filp, IOCTL_TSIF_ADD_PID, (unsigned long)&param, 0) != 0) 
            {
                dprintk("%s : pid setting error !!!\n", __func__);
            }
            break;
        }
        case DMX_TYPE_SEC:
        {
            struct dvb_demux *demux = feed->demux;

            for (i = 0; i < demux->filternum; i++)
            {
                if (demux->filter[i].state != DMX_STATE_READY)
                    continue;
                if (demux->filter[i].type != DMX_TYPE_SEC)
                    continue;
                if (demux->filter[i].filter.parent != &feed->feed.sec)
                    continue;
                demux->filter[i].state = DMX_STATE_GO;
                if (demux->dmx.frontend->source != DMX_MEMORY_FE)
                {
                    param.f_type = 0; // HW_DEMUX_SECTION
                    param.f_id = demux->filter[i].index;
                    param.f_pid = feed->pid;
                    param.f_comp = demux->filter[i].filter.filter_value;
                    param.f_mask = demux->filter[i].filter.filter_mask;
                    param.f_mode = demux->filter[i].filter.filter_mode;
                    for (j = DMX_MAX_FILTER_SIZE; j > 0; j--)
                    {
                        if (param.f_mask[j-1] != 0x0)
                        {
                            break;
                        }
                    }
                    param.f_size = j;
                    if (tcc_export_tsif_ioctl(&pHandle->filp, IOCTL_TSIF_ADD_PID, (unsigned long)&param, 0) != 0) 
                    {
                        dprintk("%s : pid setting error !!!\n", __func__);
                    }
                }
            }
            break;
        }
        case DMX_TYPE_PES:
        default:
        {
            break;
        }
    }
#endif

    return 0;
}

static int tcc_tsif_set_pid(struct tcc_tsif_instance *pHandle, struct dvb_demux_feed *feed)
{
    struct tcc_tsif_pid_param pid_param;
    struct tcc_tsif_param dma_param;
    int i, j = MAX_PID_TABLE_CNT;

    pid_param.valid_data_cnt = 0;
    j = MAX_PID_TABLE_CNT;
    for (i = 0; i < MAX_PID_TABLE_CNT; i++)
    {
        if (pHandle->pid_table.ts_users[i])
        {
            if (pHandle->pid_table.ts_pid[i] == feed->pid)
            {
                pHandle->pid_table.ts_users[i]++;
                j = -1;
            }
            pid_param.pid_data[pid_param.valid_data_cnt] = pHandle->pid_table.ts_pid[i];
            pid_param.valid_data_cnt++;
        }
        else if (i < j)
        {
            j = i;
        }
    }
    if (j >= 0 && j < MAX_PID_TABLE_CNT)
    {
        pHandle->pid_table.ts_pid[j] = feed->pid;
        pHandle->pid_table.ts_users[j]++;
        pid_param.pid_data[pid_param.valid_data_cnt] = pHandle->pid_table.ts_pid[j];
        pid_param.valid_data_cnt++;
    }

    if (tcc_export_tsif_ioctl(&pHandle->filp, IOCTL_TSIF_SET_PID, (unsigned long)&pid_param, 0) != 0) 
    {
        dprintk("%s : pid setting error !!!\n", __func__);
        //mutex_unlock(&pHandle->mutex);
        //return -1;
    }
    if (pHandle->is_start_dma == 0)
    {
        dma_param.ts_total_packet_cnt = TSIF_MAX_PACKETCOUNT;
        dma_param.ts_intr_packet_cnt = TSIF_INT_PACKETCOUNT;
        dma_param.dma_mode = DMA_MPEG2TS_MODE;	
        dma_param.mode = 0x04;		//SPI_CS_HIGH;	//dibcom(DVBT), TCC35XX
        //dma_param.mode = 0x02;		//SPI_MODE_2;		//sharp(ISDBT)
        //dma_param.mode = 0x01;
        //dma_param.mode = 0x00;
        if (tcc_export_tsif_ioctl(&pHandle->filp, IOCTL_TSIF_DMA_START, (unsigned long)&dma_param, 0) != 0) 
        {
            dprintk("%s : dma setting error !!!\n", __func__);
            //mutex_unlock(&pHandle->mutex);
            //return -1;
        }
    }
    pHandle->is_start_dma++;

    return 0;
}

int tcc_tsif_interface_set_pid(struct dvb_demux_feed *feed, unsigned int devid)
{
    struct tcc_tsif_instance *pHandle;
    int ret;

	if (gtcc_dxb == NULL)
		return -1;

	pHandle = &gtcc_dxb->tsif[devid];

    if (pHandle->users == 0)
        return -1;

    mutex_lock(&pHandle->mutex);

    if (tcc_tsif_hwdmx_can_support())
    {
        ret = tcc_tsif_set_pid_for_hwdmx(pHandle, feed);
    }
    else
    {
        ret = tcc_tsif_set_pid(pHandle, feed);
    }

    mutex_unlock(&pHandle->mutex);

    return ret;
}

int tcc_tsif_interface_set_pcrpid(int on, struct dvb_demux_feed *feed, unsigned int devid)
{
    struct tcc_tsif_instance *pHandle;
    struct tcc_tsif_pcr_param param;
    int ret;

	if (gtcc_dxb == NULL)
		return -1;

	pHandle = &gtcc_dxb->tsif[devid];

    if (pHandle->users == 0)
        return -1;

    switch (feed->pes_type)
    {
        case DMX_PES_PCR0:
            param.index = 0;
            break;
        case DMX_PES_PCR1:
            param.index = 1;
            break;
        case DMX_PES_PCR2:
            param.index = 2;
            break;
        case DMX_PES_PCR3:
            param.index = 3;
            break;
        default:
            param.index = 0;
            break;
    }

    if (on)
    {
        param.pcr_pid = feed->pid;
#ifdef SUPPORT_HARDWARE_DEMUX
        if (param.index > 0)
            tcc_tsif_interface_set_pid(feed, devid);
        //tcc_tsif_set_external_tsdemux(&pHandle->filp, tcc_tsif_parse_packet, 1, 0); //255->1 changed, because it controlled by cm3
#else
        tcc_tsif_interface_set_pid(feed, devid);
        tcc_tsif_set_external_tsdemux(0, tcc_tsif_parse_packet, 1/*255*/, 0);
#endif
    }
    else
    {
        param.pcr_pid = 0xffff;
#ifdef SUPPORT_HARDWARE_DEMUX
        if (param.index > 0)
            tcc_tsif_interface_remove_pid(feed, devid);
        //tcc_tsif_set_external_tsdemux(&pHandle->filp, tcc_tsif_parse_packet, 1, 0);
#else
        tcc_tsif_interface_remove_pid(feed, devid);
        tcc_tsif_set_external_tsdemux(0, tcc_tsif_parse_packet, 1, 0);
#endif
    }

    mutex_lock(&pHandle->mutex);
    ret = tcc_export_tsif_ioctl(&pHandle->filp, IOCTL_TSIF_SET_PCRPID, (unsigned long)&param, 0);
    mutex_unlock(&pHandle->mutex);

    return ret;
}

static int tcc_tsif_remove_pid_for_hwdmx(struct tcc_tsif_instance *pHandle, struct dvb_demux_feed *feed)
{
#ifdef SUPPORT_HARDWARE_DEMUX
    struct tcc_tsif_filter param;
    int i;

    switch (feed->type)
    {
        case DMX_TYPE_TS:
        {
            param.f_type = 1; // HW_DEMUX_TS
            param.f_id = 0;
            param.f_pid = feed->pid;
            param.f_size = 0;
            for (i = 0; i < MAX_PID_TABLE_CNT; i++)
            {
                if (pHandle->pid_table.ts_users[i])
                {
                    if (pHandle->pid_table.ts_pid[i] == param.f_pid)
                    {
                        pHandle->pid_table.ts_users[i]--;
                        if (pHandle->pid_table.ts_users[i])
                        {
                            mutex_unlock(&pHandle->mutex);
                            return 0;
                        }
                        break;
                    }
                }
            }
            if (i == MAX_PID_TABLE_CNT)
            {
                dprintk("Failed to remove filter[pid = %d]\n", param.f_pid);
                mutex_unlock(&pHandle->mutex);
                return -1;
            }
            if (tcc_export_tsif_ioctl(&pHandle->filp, IOCTL_TSIF_REMOVE_PID, (unsigned long)&param, 0) != 0) 
            {
                dprintk("%s : pid setting error !!!\n", __func__); 
            }
            break;
        }
        case DMX_TYPE_SEC:
        {
            struct dvb_demux *demux = feed->demux;

            for (i = 0; i<demux->filternum; i++) {
                if (demux->filter[i].state == DMX_STATE_GO && demux->filter[i].filter.parent == &feed->feed.sec)
                {
                    demux->filter[i].state = DMX_STATE_READY;
                    if (demux->dmx.frontend->source != DMX_MEMORY_FE)
                    {
                        param.f_type = 0; // HW_DEMUX_SECTION
                        param.f_id = demux->filter[i].index;
                        param.f_pid = feed->pid;
                        param.f_size = 0;
                        if (tcc_export_tsif_ioctl(&pHandle->filp, IOCTL_TSIF_REMOVE_PID, (unsigned long)&param, 0) != 0) 
                        {
                            dprintk("%s : pid setting error !!!\n", __func__); 
                        }
                    }
                }
            }
            break;
        }
        case DMX_TYPE_PES:
        default:
        {
            break;
        }
    }
#endif

    return 0;
}

static int tcc_tsif_remove_pid(struct tcc_tsif_instance *pHandle, struct dvb_demux_feed *feed)
{
    struct tcc_tsif_pid_param pid_param;
    int i;

    pid_param.valid_data_cnt = 0;
    for (i = 0; i < MAX_PID_TABLE_CNT; i++)
    {
        if (pHandle->pid_table.ts_users[i])
        {
            if (pHandle->pid_table.ts_pid[i] == feed->pid)
            {
                pHandle->pid_table.ts_users[i]--;
            }
        }
        if (pHandle->pid_table.ts_users[i])
        {
            pid_param.pid_data[pid_param.valid_data_cnt] = pHandle->pid_table.ts_pid[i];
            pid_param.valid_data_cnt++;
        }
    }
    if (tcc_export_tsif_ioctl(&pHandle->filp, IOCTL_TSIF_SET_PID, (unsigned long)&pid_param, 0) != 0) 
    {
        dprintk("%s : pid setting error !!!\n", __func__); 
        //mutex_unlock(&pHandle->mutex);
        //return -1;
    }

    pHandle->is_start_dma--;
    if (pHandle->is_start_dma == 0)
    {
        if(tcc_export_tsif_ioctl(&pHandle->filp, IOCTL_TSIF_DMA_STOP, 0, 0)!=0)
        {
            dprintk("%s : dma setting error !!!\n", __func__);
            //mutex_unlock(&pHandle->mutex);
            //return -1;
        }        
    }

    return 0;
}

int tcc_tsif_interface_remove_pid(struct dvb_demux_feed *feed, unsigned int devid)
{
    struct tcc_tsif_instance *pHandle;
    int ret;

	if (gtcc_dxb == NULL)
		return -1;

	pHandle = &gtcc_dxb->tsif[devid];

    if (pHandle->users == 0)
        return -1;

    mutex_lock(&pHandle->mutex);

    if (tcc_tsif_hwdmx_can_support())
    {
        ret = tcc_tsif_remove_pid_for_hwdmx(pHandle, feed);
    }
    else
    {
        ret = tcc_tsif_remove_pid(pHandle, feed);
    }

    mutex_unlock(&pHandle->mutex);

    return 0;
}

unsigned int tcc_tsif_interface_get_stc(unsigned int index, unsigned int devid)
{
    struct tcc_tsif_instance *pHandle;
    unsigned int stc = index;

	if (gtcc_dxb == NULL)
		return -1;

	pHandle = &gtcc_dxb->tsif[devid];

    if (pHandle->users == 0)
        return -1;

    mutex_lock(&pHandle->mutex);
    tcc_export_tsif_ioctl(&pHandle->filp, IOCTL_TSIF_GET_STC, (unsigned long)&stc, 0);
    mutex_unlock(&pHandle->mutex);

    return stc;
}
