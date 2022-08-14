#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/poll.h>
#include <linux/spi/spi.h>
#include <linux/spi/tcc_tsif.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/dma.h>

#include <mach/bsp.h>
#include <mach/tca_hwdemux_tsif.h>

#include <i-tv/itv_common.h>
#include "itccxxxx_tsif_p.h"





#define MPEG_PACKET_SIZE 		(188)

#undef  TSIF_DMA_SIZE
#define TSIF_DMA_SIZE 				(188*10000)
#define SECTION_DMA_SIZE 			(512*1024)

#undef  TSIF_DMA_PACKET_CNT
#define TSIF_DMA_PACKET_CNT		512


struct tcc_hwdmx_handle {
    struct tcc_tsif_handle tsif_ex_handle;
    int read_buff_offset;
    int write_buff_offset;
    int end_buff_offset;
    int loop_count;
    int state;
    unsigned char * mem;
    wait_queue_head_t wait_q;
    unsigned int dma_mode; //0:normal mode (not mpeg2ts mode), 1:mpeg2ts mode(start with 0x47)
};

typedef enum {
	P_TSIF_STATE_DEINIT, 
	P_TSIF_STATE_INIT, 
	P_TSIF_STATE_START, 
	P_TSIF_STATE_STOP 
} e_p_tsif_state;

typedef struct {
	e_p_tsif_state state;

	int mode;	// true : TSIF_Serial mode, false : TSIF_Parallel mode.
	struct tcc_tsif_handle handle;

	void *cb_data;
	void (*callback)(void *);
	
	#define PIDTABLE_MAXCNT		32
	
	unsigned short pidtable[PIDTABLE_MAXCNT];

	struct tcc_hwdmx_handle demux;

	spinlock_t lock;
} st_p_tsif;

st_p_tsif p_tsif_inst = {
	.state = P_TSIF_STATE_DEINIT, 
};

static const char fw_name[] = "tcc_tsif/HWDemux.bin";
static struct device *pdev = NULL;
static struct class *tsif_ex_class;

#define TSIF_DEV_NAME4CM3	"tcc-tsif-cm3"
#define TSIF_DEV_MAJOR4CM3	ITV_MAJOR
#define TSIF_DEV_MINOR4CM3	10

//#define TSIF_CM3_PIDFILTER_USE		//cm3가 bypass mode로 동작 될 경우 pid filter를 사용할 수 없음.
//#define TSIF_CM3_DEBUG


#ifdef TSIF_CM3_DEBUG
static unsigned int prev_time = 0;
static unsigned int total_size = 0;

static void tcc_tsif_cm3_debug(struct tcc_hwdmx_handle *demux, char *pBuffer, int updated_buff_offset, int end_buff_offset)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_handle *handle = &instance->handle;

	struct timeval	time;
	unsigned int cur_time;

	unsigned short pid;
	unsigned char cc;

	int pos, i, j;

	#define DEBUG_CHK_TIME		3


	int ret = 0;
	char *p1 = NULL, *p2 = NULL;
	int p1_size = 0, p2_size = 0;

	//printk("\t### up[%d] end[%d] wr[%d]\n", updated_buff_offset, end_buff_offset, demux->write_buff_offset);
	
	if(updated_buff_offset > demux->write_buff_offset) {
		p1 = (char *)(pBuffer + demux->write_buff_offset);
		p1_size = updated_buff_offset - demux->write_buff_offset;
	} else if (end_buff_offset == demux->write_buff_offset) {
		p1 = (char *)pBuffer;
		p1_size = updated_buff_offset;
	} else {
		p1 = (char *)(pBuffer + demux->write_buff_offset);
		p1_size = end_buff_offset - demux->write_buff_offset;

		p2 = (char *)pBuffer;
		p2_size = updated_buff_offset;
	}

	demux->write_buff_offset = updated_buff_offset;

#if 1
	total_size += p1_size + p2_size;
	
	if(prev_time) {
		do_gettimeofday(&time);
		cur_time = time.tv_sec;

		if((cur_time - prev_time) >= DEBUG_CHK_TIME) { 
			if(p1)	printk("\t\t### [%dsec / %dbyte] p1[0x%08x : 0x%02x / %d] ", (cur_time - prev_time), total_size, p1, *p1, p1_size);	
			if(p2)	printk("p2[0x%08x : 0x%02x / %d]", p2, *p2, p2_size);
			printk(" %dbyte\n", (p1_size + p2_size));

			prev_time = cur_time;
			total_size = 0;
		}
	} else {
		if(p1)	printk("\t\t### [0sec / %dbyte] p1[0x%08x : 0x%02x / %d] ", total_size, p1, *p1, p1_size);	
		if(p2)	printk("p2[0x%08x : 0x%02x / %d]", p2, *p2, p2_size);
		printk(" %dbyte\n", (p1_size + p2_size));
		
		do_gettimeofday(&time);
		prev_time = time.tv_sec;
		total_size = 0;
	}
#else
	if(prev_time) {
		do_gettimeofday(&time);
		cur_time = time.tv_sec;

		if((cur_time - prev_time) >= DEBUG_CHK_TIME) { 
			if(p1 && p1_size) {
				for(i=0; i<(p1_size/188); i++) {
					pid = ((*(unsigned char*)(p1 + (188 * i)  + 1) & 0x1f) << 8) | *(unsigned char*)(p1 + (188 * i) + 2);
					cc 	= *(unsigned char*)(p1 + (188 * i) + 3) & 0x0f;

					for(j=0; j<PIDTABLE_MAXCNT; j++) {
						if(pid == instance->pidtable[j]) {
							printk("\t###  [0x%08x] 0x%02x 0x%04x 0x%02x\n", (p1 + (188 * i)), *(p1 + (188 * i)), pid, cc);
							prev_time = cur_time;
						}
					}
				}
			}

			if(p2 && p2_size) {
				for(i=0; i<(p2_size/188); i++) {
					pid = ((*(unsigned char*)(p2 + (188 * i)  + 1) & 0x1f) << 8) | *(unsigned char*)(p2 + (188 * i) + 2);
					cc 	= *(unsigned char*)(p2 + (188 * i) + 3) & 0x0f;

					for(j=0; j<PIDTABLE_MAXCNT; j++) {
						if(pid == instance->pidtable[j]) {
							printk("\t###  [0x%08x] 0x%02x 0x%04x 0x%02x\n", (p2 + (188 * i)), *(p1 + (188 * i)), pid, cc);
							prev_time = cur_time;
						}
					}
				}
			}
		}
	} else {
		do_gettimeofday(&time);
		prev_time = time.tv_sec;
	}
#endif
}
#endif

static int tcc_tsif_cm3_packet_cnt_chk(int wr_pos, int end_pos)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_handle *handle = &instance->handle;
	struct tcc_hwdmx_handle *demux = &instance->demux;

	int intr_packet_cnt = 0;
	int rd_pos =  demux->write_buff_offset;

	if(wr_pos > rd_pos) {
		intr_packet_cnt = wr_pos - rd_pos;
	} else if (end_pos == rd_pos) {
		intr_packet_cnt = wr_pos;
	} else {
		intr_packet_cnt = end_pos - rd_pos + wr_pos;
	}
	intr_packet_cnt /= MPEG_PACKET_SIZE;

	handle->dma_recv_packet_cnt += intr_packet_cnt;

	if(handle->dma_recv_packet_cnt >= handle->dma_intr_packet_cnt) {
		handle->dma_recv_packet_cnt = 0;
		
		return 1;
	} else {
		return 0;
	}
}

static int tcc_tsif_cm3_updated_callback(unsigned int dmxid, unsigned int ftype, unsigned int fid, unsigned int value1, unsigned int value2, unsigned int bErrCRC)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_handle *handle = &instance->handle;
	struct tcc_hwdmx_handle *demux = &instance->demux;

	//printk("\n\t######### %s : %d\n", __func__, ftype);

	switch(ftype) {
		case 0: // HW_DEMUX_SECTION
			break;
		 case 1: // HW_DEMUX_TS
		 		if(tcc_tsif_cm3_packet_cnt_chk(value1, value2) > 0) {
			 		handle->cur_q_pos = demux->tsif_ex_handle.dma_buffer->v_addr + value1;
			 		
			 		if(instance->callback)
						instance->callback(instance->cb_data);
		 		}

#ifdef TSIF_CM3_DEBUG
		 		tcc_tsif_cm3_debug(demux, demux->tsif_ex_handle.dma_buffer->v_addr, value1, value2);
#endif
		 	break;
		 case 2: // HW_DEMUX_PES
		 	break;
		 case 3: // HW_DEMUX_PCR
		 	break;
		 default:
		 	break;
	}
}

int tcc_tsif_cm3_fw_init(void)
{
 	st_p_tsif *instance = &p_tsif_inst;

	struct tcc_tsif_handle *handle = &instance->handle;
	struct tcc_hwdmx_handle *demux = &instance->demux;
	struct tea_dma_buf *dma_buffer;

	const struct firmware *fw = NULL;

	int ret = 0;

	memset(&demux->tsif_ex_handle, 0, sizeof(tcc_tsif_handle_t));
	demux->tsif_ex_handle.dmx_id = 0;

	ret = request_firmware(&fw, fw_name, pdev);
	if(ret != 0) {
		printk("Failed to load[Err:%d]\n", ret);
		return -1;
	}

	demux->dma_mode = 1; //0:normal mode (not mpeg2ts mode), 1:mpeg2ts mode(start with 0x47)

	dma_buffer = handle->dma_buffer;
	if(dma_buffer == NULL) {
		return -ENOMEM;
	}
	demux->tsif_ex_handle.dma_buffer = dma_buffer;

	if(fw && fw->size) {
		demux->tsif_ex_handle.fw_data = fw->data;
		demux->tsif_ex_handle.fw_size = fw->size;
	} else {
		demux->tsif_ex_handle.fw_data = NULL;
		demux->tsif_ex_handle.fw_size = 0;
	}

	demux->tsif_ex_handle.dma_mode = demux->dma_mode;

	//Check working mode
	if( demux->tsif_ex_handle.dma_mode == 0) {
		//not mepg2ts modec
		demux->tsif_ex_handle.working_mode = 0; //tsif for tdmb
	} else {
		//mpeg2ts mode
		demux->tsif_ex_handle.working_mode = 1; //tsif for isdbt & dvbt
		demux->tsif_ex_handle.serial_mode = instance->mode;	//1:serial 0:parallel
	}

	demux->tsif_ex_handle.tsif_port_num = 3;	//gpio_d[0:10]

	printk("%s : dma_mode[%d] working_mod[%d] serial_mode[%d] tsif_port_num[%d]\n",__func__, demux->tsif_ex_handle.dma_mode, demux->tsif_ex_handle.working_mode, 	\
															demux->tsif_ex_handle.serial_mode, demux->tsif_ex_handle.tsif_port_num);

	if (tca_tsif_cm3_fw_init(&demux->tsif_ex_handle)) {
		printk("%s: tca_tsif_init error !!!!!\n", __func__);
		if(dma_buffer) {
			if(handle->dma_buffer) {
			} else {
				if(dma_buffer->v_addr) {
					dma_free_writecombine(0, dma_buffer->buf_size, dma_buffer->v_addr, dma_buffer->dma_addr);
					printk("tcc893x_tsif_init : dma buffer free @0x%X(Phy=0x%X), size:%d\n", (unsigned int)dma_buffer->v_addr, (unsigned int)dma_buffer->dma_addr, dma_buffer->buf_size);
				}
				
				if(dma_buffer->v_sec_addr) {
					dma_free_writecombine(0, dma_buffer->buf_sec_size, dma_buffer->v_sec_addr, dma_buffer->dma_sec_addr);
					printk("tcc893x_tsif_deinit : dma sec buffer free @0x%X(Phy=0x%X), size:%d\n", (unsigned int)dma_buffer->v_sec_addr, (unsigned int)dma_buffer->dma_sec_addr, dma_buffer->buf_sec_size);
				}
			}
			kfree(dma_buffer);
			demux->tsif_ex_handle.dma_buffer = NULL;
		}
		tca_tsif_cm3_fw_deinit(&demux->tsif_ex_handle);
		return -EBUSY;
	}

	demux->write_buff_offset = 0;
	demux->read_buff_offset = 0;
	demux->end_buff_offset = 0;
	demux->loop_count = 0;

	tca_tsif_buffer_updated_callback(&demux->tsif_ex_handle, tcc_tsif_cm3_updated_callback);

	return ret;
 }
EXPORT_SYMBOL_GPL(tcc_tsif_cm3_fw_init);

void tcc_tsif_cm3_fw_deinit(void)
{
	st_p_tsif *instance = &p_tsif_inst;

	struct tcc_tsif_handle *handle = &instance->handle;
	struct tcc_hwdmx_handle *demux = &instance->demux;
	struct tea_dma_buf *dma_buffer = demux->tsif_ex_handle.dma_buffer;

	if(demux->mem)	vfree(demux->mem);
	demux->mem = NULL;

	demux->write_buff_offset = 0;
	demux->read_buff_offset = 0;
	demux->end_buff_offset = 0;
	demux->loop_count = 0;
	demux->tsif_ex_handle.dma_buffer = NULL;

	tca_tsif_cm3_fw_deinit(&demux->tsif_ex_handle);
}
EXPORT_SYMBOL_GPL(tcc_tsif_cm3_fw_deinit);

static int tcc_tsif_cm3_start(void)
{
	st_p_tsif *instance = &p_tsif_inst;

	struct tcc_tsif_handle *handle = &instance->handle;
	struct tcc_hwdmx_handle *demux = &instance->demux;

	int ret = 0;

	printk("%s : dma_mode[%d] working_mod[%d] serial_mode[%d] tsif_port_num[%d]\n",__func__, demux->tsif_ex_handle.dma_mode, demux->tsif_ex_handle.working_mode, 	\
															demux->tsif_ex_handle.serial_mode, demux->tsif_ex_handle.tsif_port_num);

	if (tca_tsif_start(&demux->tsif_ex_handle)) {
		printk("%s: tca_tsif_start error !!!!!\n", __func__);
		return -EBUSY;
	}

	demux->write_buff_offset = 0;
	demux->read_buff_offset = 0;
	demux->end_buff_offset = 0;
	demux->loop_count = 0;

	return ret;
}

static void tcc_tsif_cm3_stop(void)
{
	st_p_tsif *instance = &p_tsif_inst;

	struct tcc_tsif_handle *handle = &instance->handle;
	struct tcc_hwdmx_handle *demux = &instance->demux;

	demux->write_buff_offset = 0;
	demux->read_buff_offset = 0;
	demux->end_buff_offset = 0;
	demux->loop_count = 0;

	tca_tsif_stop(&demux->tsif_ex_handle);
}

int tcc_tsif_p_get_pos(void)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_handle *handle = &instance->handle;

	return handle->cur_q_pos;
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_get_pos);

void tcc_tsif_p_insert_pid(int pid)
{
	st_p_tsif *instance = &p_tsif_inst;

	int i, pos=0;
	
#ifdef TSIF_CM3_PIDFILTER_USE
	struct tcc_hwdmx_handle *demux = &instance->demux;
	struct tcc_tsif_filter filter_param;
#endif

	spin_lock(&instance->lock);

	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		if(instance->pidtable[i] == pid) {
			printk("[%s] pid table already insert val : %d\n", __func__, pid);
			return;
		}
	}

	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		if(instance->pidtable[i] == 0xffff) {
			pos = i;
			break;
		}
		
		if(i == (PIDTABLE_MAXCNT - 1)) {
			printk("[%s] pid table full : %d\n", __func__, (i + 1));
			return;
		}
	}
	
	instance->pidtable[pos] = pid;

#ifdef TSIF_CM3_PIDFILTER_USE
	if(instance->state == P_TSIF_STATE_START) {
		filter_param.f_id = 0;
		filter_param.f_type = 1;
		filter_param.f_size = 0;
		filter_param.f_comp = NULL;
		filter_param.f_mask = NULL;
		filter_param.f_mode = NULL;
		filter_param.f_pid = pid;
		tca_tsif_add_filter(&demux->tsif_ex_handle, &filter_param);
	}
#endif

	spin_unlock(&instance->lock);

	//printk("[%s] pidtable insert : %d-%d\n", __func__, pos, pid);
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_insert_pid);

void tcc_tsif_p_remove_pid(int pid)
{
	st_p_tsif *instance = &p_tsif_inst;

	int i, pos=0;
	
#ifdef TSIF_CM3_PIDFILTER_USE
	struct tcc_hwdmx_handle *demux = &instance->demux;
	struct tcc_tsif_filter filter_param;
#endif	

	spin_lock(&instance->lock);

	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		if(instance->pidtable[i] == pid) {
			pos = i;
			break;
		}
		
		if(i == (PIDTABLE_MAXCNT - 1)) {
			printk("[%s] pid table not found : %d\n", __func__, pid);
			return;
		}
	}
	
	instance->pidtable[pos] = 0xffff;

#ifdef TSIF_CM3_PIDFILTER_USE
	if(instance->state == P_TSIF_STATE_START) {
		filter_param.f_id = 0;
		filter_param.f_type = 1;
		filter_param.f_size = 0;
		filter_param.f_comp = NULL;
		filter_param.f_mask = NULL;
		filter_param.f_mode = NULL;
		filter_param.f_pid = pid;
		tca_tsif_remove_filter(&demux->tsif_ex_handle, &filter_param);
	}
#endif

	spin_unlock(&instance->lock);

	//printk("[%s] pidtable remove : %d-%d\n", __func__, pos, pid);
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_remove_pid);

void tcc_tsif_p_set_packetcnt(int cnt)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_handle *handle = &instance->handle;
	
	handle->dma_intr_packet_cnt = cnt;
	
	printk("[%s] intr_packet_cnt:%d\n", __func__, handle->dma_intr_packet_cnt);
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_set_packetcnt);

int tcc_tsif_p_start(void)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_handle *handle = &instance->handle;

#ifdef TSIF_CM3_PIDFILTER_USE	
	struct tcc_hwdmx_handle *demux = &instance->demux;
	struct tcc_tsif_filter filter_param;

	int i;
#endif	
	
  	if(tcc_tsif_cm3_start() < 0) {
		printk("[%s] tcc_tsif_cm3_start() fail...\n", __func__);
		return -1;
	}

#ifdef TSIF_CM3_PIDFILTER_USE
	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		if(instance->pidtable[i] != 0xffff) {
			filter_param.f_id = 0;
			filter_param.f_type = 1;
			filter_param.f_size = 0;
			filter_param.f_comp = NULL;
			filter_param.f_mask = NULL;
			filter_param.f_mode = NULL;
			filter_param.f_pid = instance->pidtable[i];
			tca_tsif_add_filter(&demux->tsif_ex_handle, &filter_param);

			printk("[%s] insert pid at tsif start : 0x%04x\n", __func__, filter_param.f_pid);
		}
	}
#endif

	handle->dma_recv_packet_cnt = 0;
	printk("[%s] intr_cnt:%d / recv_packet_size:%d\n", __func__, handle->dma_intr_packet_cnt, handle->dma_recv_packet_cnt);	
	
	instance->state = P_TSIF_STATE_START;

	return 0;
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_start);

int tcc_tsif_p_stop(void)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_handle *handle = &instance->handle;

#ifdef TSIF_CM3_PIDFILTER_USE	
	struct tcc_hwdmx_handle *demux = &instance->demux;
	struct tcc_tsif_filter filter_param;

	int i;

	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		if(instance->pidtable[i] != 0xffff) {
			filter_param.f_id = 0;
			filter_param.f_type = 1;
			filter_param.f_size = 0;
			filter_param.f_comp = NULL;
			filter_param.f_mask = NULL;
			filter_param.f_mode = NULL;
			filter_param.f_pid = instance->pidtable[i];
			tca_tsif_remove_filter(&demux->tsif_ex_handle, &filter_param);

			printk("[%s] remove pid at tsif stop : 0x%04x\n", __func__, filter_param.f_pid);
		}
	}
#endif

	instance->state = P_TSIF_STATE_STOP;
	
	tcc_tsif_cm3_stop();

	return 0;
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_stop);

int tcc_tsif_p_init(unsigned char **buffer_addr, unsigned int *real_buffer_size, unsigned int buffer_size, 
		unsigned int pkt_intr_cnt, unsigned int timing_mode, unsigned int pid_mode, unsigned int serial_mode, 
		void (*callback)(void *), void *cb_data) 
{
	int error = 0;
	int i;
	
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_handle *handle = &instance->handle;

	if(instance->state != P_TSIF_STATE_DEINIT)
	{
		printk("[%s] Already Init : 0x%x\n", __func__, instance->state);
		return -EACCES;
	}

	memset(instance, 0x00, sizeof(st_p_tsif));

	instance->callback = callback;
	instance->cb_data = cb_data;
	instance->mode = serial_mode;

	handle->dma_buffer = kmalloc(sizeof(struct tea_dma_buf), GFP_KERNEL);
	if(handle->dma_buffer) {
		handle->dma_buffer->buf_size = TSIF_DMA_SIZE;
		handle->dma_buffer->v_addr = dma_alloc_writecombine(0, handle->dma_buffer->buf_size, &handle->dma_buffer->dma_addr, GFP_KERNEL);
		printk("tsif > static dma buffer alloc @0x%X(Phy=0x%X), size:%d\n", (unsigned int)handle->dma_buffer->v_addr, (unsigned int)handle->dma_buffer->dma_addr, handle->dma_buffer->buf_size);

		if(handle->dma_buffer->v_addr == NULL) {
			kfree(handle->dma_buffer);
			handle->dma_buffer = NULL;

			error = -1;
		} else {
			handle->dma_buffer->buf_sec_size = SECTION_DMA_SIZE;
			handle->dma_buffer->v_sec_addr = dma_alloc_writecombine(0, handle->dma_buffer->buf_sec_size, &handle->dma_buffer->dma_sec_addr, GFP_KERNEL);
			printk("tsif > static dma sec buffer alloc @0x%X(Phy=0x%X), size:%d\n", (unsigned int)handle->dma_buffer->v_sec_addr, (unsigned int)handle->dma_buffer->dma_sec_addr, handle->dma_buffer->buf_sec_size);
			
			if(handle->dma_buffer->v_sec_addr == NULL) {
				if(handle->dma_buffer->v_addr) {
					dma_free_writecombine(0, handle->dma_buffer->buf_size, handle->dma_buffer->v_addr, handle->dma_buffer->dma_addr);
					printk("tsif > static dma buffer free @0x%X(Phy=0x%X), size:%d\n", (unsigned int)handle->dma_buffer->v_addr, (unsigned int)handle->dma_buffer->dma_addr, handle->dma_buffer->buf_size);
				}
				kfree(handle->dma_buffer);
				handle->dma_buffer = NULL;

				error = -1;
			}
		}
	}

	handle->cur_q_pos = handle->dma_buffer->v_addr;
	handle->dma_intr_packet_cnt = TSIF_DMA_PACKET_CNT;
	printk("tsif > dma size:0x%x / intr_cnt:%d\n", TSIF_DMA_SIZE, handle->dma_intr_packet_cnt);	

	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		instance->pidtable[i] = 0xffff;
	}
		
	*buffer_addr = handle->dma_buffer->v_addr;
	*real_buffer_size = TSIF_DMA_SIZE;
	
	tsif_ex_class = class_create(THIS_MODULE, TSIF_DEV_NAME4CM3);
	pdev = device_create(tsif_ex_class, NULL, MKDEV(TSIF_DEV_MAJOR4CM3, TSIF_DEV_MINOR4CM3), NULL, TSIF_DEV_NAME4CM3);

	instance->state = P_TSIF_STATE_INIT;

	spin_lock_init(&instance->lock);
	
	return error;
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_init);

void tcc_tsif_p_deinit(void)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_handle *handle = &instance->handle;

	if(instance->state == P_TSIF_STATE_DEINIT)
		return;
	
	if(handle->dma_buffer) {
		if(handle->dma_buffer->v_addr) {
			dma_free_writecombine(0, handle->dma_buffer->buf_size, handle->dma_buffer->v_addr, handle->dma_buffer->dma_addr);
			printk("tcc893x_tsif_deinit : static dma buffer free @0x%X(Phy=0x%X), size:%d\n", (unsigned int)handle->dma_buffer->v_addr, (unsigned int)handle->dma_buffer->dma_addr, handle->dma_buffer->buf_size);
		}
		
		if(handle->dma_buffer->v_sec_addr) {
			dma_free_writecombine(0, handle->dma_buffer->buf_sec_size, handle->dma_buffer->v_sec_addr, handle->dma_buffer->dma_sec_addr);
			printk("tcc893x_tsif_deinit : static dma sec buffer free @0x%X(Phy=0x%X), size:%d\n", (unsigned int)handle->dma_buffer->v_sec_addr, (unsigned int)handle->dma_buffer->dma_sec_addr, handle->dma_buffer->buf_sec_size);
		}
		
		kfree(handle->dma_buffer);
		handle->dma_buffer = NULL;
	}

	device_destroy(tsif_ex_class, MKDEV(TSIF_DEV_MAJOR4CM3, TSIF_DEV_MINOR4CM3));
	class_destroy(tsif_ex_class);

	instance->state = P_TSIF_STATE_DEINIT;
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_deinit);
