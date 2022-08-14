/****************************************************************************
 *   FileName    : tcc_dxb_kernel.h
 *   Description : 
 ****************************************************************************
 *
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#ifndef	_TCC_DXB_KERNEL_H_
#define	_TCC_DXB_KERNEL_H_

#define DRV_NAME      "tcc_dxb_drv"
#define DVB_NAME      "tcc_dxb_kernel"

#define TSIF_DEV_COUNT 2 // the number of tsif/hwdmx
#define DMX_DEV_COUNT  2 // the number of linuxtv demux

struct tcc_dxb_instance
{
    struct dvb_adapter adapter;
    struct tcc_tsif_instance tsif[TSIF_DEV_COUNT];
    struct tcc_dmx_instance dmx[DMX_DEV_COUNT];
};

struct tcc_dxb_instance* tcc_dxb_get_instance(void);
int tcc_dxb_get_board_type(void);

#endif	// _TCC_DXB_KERNEL_H_

