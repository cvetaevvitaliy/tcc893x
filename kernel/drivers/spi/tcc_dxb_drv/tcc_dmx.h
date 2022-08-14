/****************************************************************************
 *   FileName    : tcc_dmx.h
 *   Description : 
 ****************************************************************************
 *
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#ifndef _TCC_DMX_H_
#define _TCC_DMX_H_

#include "../../media/dvb/dvb-core/dvb_frontend.h"
#include "../../media/dvb/dvb-core/dvb_demux.h"
#include "../../media/dvb/dvb-core/dvb_net.h"
#include "../../media/dvb/dvb-core/dmxdev.h"

#define FE_DEV_COUNT  2  // the number of frontend driver(0: dummy frontend)

struct tcc_dmx_instance
{
    unsigned int tsif_id;             // connected hwdemux id
    unsigned int running;
    struct dmx_frontend fe_hw[FE_DEV_COUNT];
    struct dmx_frontend fe_mem;
    struct dvb_net net;
    struct dvb_demux demux;
    struct dmxdev dmxdev;
};

int tcc_dmx_init(struct tcc_dmx_instance *dxb_demux);
int tcc_dmx_deinit(struct tcc_dmx_instance *dxb_demux);

#endif // _TCC_DMX_H_