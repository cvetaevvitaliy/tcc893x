/****************************************************************************
 *   FileName    : tcc_tsif_interface.h
 *   Description : 
 ****************************************************************************
 *
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#ifndef	_TCC_TSIF_INTERFACE_H_
#define	_TCC_TSIF_INTERFACE_H_

#define MAX_PID_TABLE_CNT           32
#define MAX_PCR_CNT                 2
#define TS_PACKET_LIST              100

struct packet_info
{
    int valid; //1:valid, 0:invalid
    char *p1;
    int p1_size;

    char *p2;
    int p2_size;
    struct list_head list;
};

struct packet_list
{
    int current_index;
    spinlock_t lock;
    struct packet_info ts_packet[TS_PACKET_LIST];
};

struct pid_table
{
    unsigned int ts_pid[MAX_PID_TABLE_CNT];
    unsigned int ts_users[MAX_PID_TABLE_CNT];
    unsigned int pcr_pid[MAX_PCR_CNT];
};

struct tcc_tsif_instance
{
    struct file filp;
    struct mutex mutex;
    int users;

    int is_start_dma;

    struct pid_table pid_table;

    struct tasklet_struct tsif_tasklet;
    struct packet_list ts_packet_list;
    struct list_head ts_packet_bank;
};

int tcc_tsif_init(struct tcc_tsif_instance *tsif);
int tcc_tsif_deinit(struct tcc_tsif_instance *tsif);

int tcc_tsif_interface_open(struct device *device);
int tcc_tsif_interface_close(void);
int tcc_tsif_interface_set_pid(struct dvb_demux_feed *feed, unsigned int devid);
int tcc_tsif_interface_remove_pid(struct dvb_demux_feed *feed, unsigned int devid);
int tcc_tsif_interface_set_pcrpid(int on, struct dvb_demux_feed *feed, unsigned int devid);
unsigned int tcc_tsif_interface_get_stc(unsigned int index, unsigned int devid);

#endif	// _TCC_TSIF_INTERFACE_H_

