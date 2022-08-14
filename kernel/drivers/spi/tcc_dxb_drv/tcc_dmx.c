/*
 *  Driver for TCC DMX
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

#include <linux/interrupt.h>

#include "tcc_dmx.h"
#include "tcc_tsif_interface.h"
#include "tcc_dxb_kernel.h"

/*****************************************************************************
 *
 * Defines
 *
 ******************************************************************************/
#if 0
#define dprintk(msg...)	 { printk( "[tcc_dmx]" msg); }
#else
#define dprintk(msg...)	 
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


/*****************************************************************************
 *
 * External Functions
 *
 ******************************************************************************/


 /*****************************************************************************
 *
 * Functions
 *
 ******************************************************************************/
static int tcc_dxb_start_feed(struct dvb_demux_feed *feed)
{
    struct tcc_dmx_instance *tcc_dmx = (struct tcc_dmx_instance *)feed->demux->priv;
    unsigned int devid = tcc_dmx->tsif_id;

    if(feed->demux->dmx.frontend->source == DMX_MEMORY_FE) {
        //return 0;
    }

    tcc_dmx->running++;

    if((feed->type == DMX_TYPE_TS) && (feed->pes_type == DMX_TS_PES_PCR0 ||
                                       feed->pes_type == DMX_TS_PES_PCR1 ||
                                       feed->pes_type == DMX_TS_PES_PCR2 ||
                                       feed->pes_type == DMX_TS_PES_PCR3))
    {
        if (feed->ts_type & TS_DEMUX)
        {
            tcc_tsif_interface_set_pcrpid(1, feed, devid);
        }
    }
    else
    {
        tcc_tsif_interface_set_pid(feed, devid);
    }
    return 0;
}

static int tcc_dxb_stop_feed(struct dvb_demux_feed *feed)
{
    struct tcc_dmx_instance *tcc_dmx = (struct tcc_dmx_instance *)feed->demux->priv;
    unsigned int devid = tcc_dmx->tsif_id;

    if(feed->demux->dmx.frontend->source == DMX_MEMORY_FE) {
        //return 0;
    }
    if((feed->type == DMX_TYPE_TS) && (feed->pes_type == DMX_TS_PES_PCR0 ||
                                       feed->pes_type == DMX_TS_PES_PCR1 ||
                                       feed->pes_type == DMX_TS_PES_PCR2 ||
                                       feed->pes_type == DMX_TS_PES_PCR3))
    {
        if (feed->ts_type & TS_DEMUX)
        {
            tcc_tsif_interface_set_pcrpid(0, feed, devid);
        }
    }
    else
    {
        tcc_tsif_interface_remove_pid(feed, devid);
    }

    tcc_dmx->running--;

    return 0;
}

static int tcc_dxb_get_stc(struct dmx_demux* demux, unsigned int num,
			u64 *stc, unsigned int *base)
{
    struct dvb_demux *dvbdmx = (struct dvb_demux *)demux->priv;
    struct tcc_dmx_instance *tcc_dmx = (struct tcc_dmx_instance *)dvbdmx->priv;
    unsigned int devid = tcc_dmx->tsif_id;

    *stc = (u64)tcc_tsif_interface_get_stc(num, devid);

    return 0;
}

static int tcc_dxb_set_source(struct dmx_demux* demux, const dmx_source_t *src)
{
    struct dvb_demux *dvbdmx = (struct dvb_demux *)demux->priv;
    struct tcc_dmx_instance *tcc_dmx = (struct tcc_dmx_instance *)dvbdmx->priv;
    int index = TSIF_DEV_COUNT;

	if (tcc_dmx->running != 0)
        return -1;

    if (*src == DMX_SOURCE_FRONT0)
        index = 0;
    else if (*src == DMX_SOURCE_FRONT1)
        index = 1;
    else if (*src == DMX_SOURCE_FRONT2)
        index = 2;
    else if (*src == DMX_SOURCE_FRONT3)
        index = 3;

    if (index < TSIF_DEV_COUNT)
        tcc_dmx->tsif_id = index;

    return 0;
}

static int tcc_dxb_write_to_decoder(struct dvb_demux_feed *feed, const u8 *buf, size_t len)
{
    return 0;
}

int tcc_dmx_init(struct tcc_dmx_instance *dxb_demux)
{
    int ret, i;
    struct tcc_dxb_instance *tcc_dxb;

    tcc_dxb = tcc_dxb_get_instance();

    dxb_demux->tsif_id = 0;
    dxb_demux->running = 0;

    /* register demux stuff */
    dxb_demux->demux.dmx.capabilities =
        DMX_TS_FILTERING | DMX_SECTION_FILTERING |
        DMX_MEMORY_BASED_FILTERING;
    dxb_demux->demux.priv       = dxb_demux;
    dxb_demux->demux.filternum  = 256;
    dxb_demux->demux.feednum    = 256;
    dxb_demux->demux.start_feed = tcc_dxb_start_feed;
    dxb_demux->demux.stop_feed  = tcc_dxb_stop_feed;
    dxb_demux->demux.write_to_decoder = tcc_dxb_write_to_decoder;
    ret = dvb_dmx_init(&dxb_demux->demux);
    if (ret < 0) {
        dprintk(KERN_INFO "%s::%d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    dxb_demux->dmxdev.filternum    = 256;
    dxb_demux->dmxdev.demux        = &dxb_demux->demux.dmx;
    dxb_demux->dmxdev.capabilities = 0;
    dxb_demux->dmxdev.demux->get_stc = tcc_dxb_get_stc;
    dxb_demux->dmxdev.demux->set_source = tcc_dxb_set_source;

    ret = dvb_dmxdev_init(&dxb_demux->dmxdev, &tcc_dxb->adapter);
    if (ret < 0) {
        dprintk(KERN_INFO "%s::%d\n", __FUNCTION__,__LINE__);
        return -1;
    }

    for (i = 0; i < FE_DEV_COUNT; i++) {
        dxb_demux->fe_hw[i].source = DMX_FRONTEND_0 + i;
        ret = dxb_demux->demux.dmx.add_frontend(&dxb_demux->demux.dmx, &dxb_demux->fe_hw[i]);
        if (ret < 0) {
            dprintk(KERN_INFO "%s::%d\n", __FUNCTION__,__LINE__);
            return -1;
        }
    }

    dxb_demux->fe_mem.source = DMX_MEMORY_FE;
    ret = dxb_demux->demux.dmx.add_frontend(&dxb_demux->demux.dmx, &dxb_demux->fe_mem);
    if (ret < 0) {
        dprintk(KERN_INFO "%s::%d\n", __FUNCTION__,__LINE__);
        return -1;
    }

    ret = dxb_demux->demux.dmx.connect_frontend(&dxb_demux->demux.dmx, &dxb_demux->fe_hw[0]);
    if (ret < 0) {
        dprintk(KERN_INFO "%s::%d\n", __FUNCTION__,__LINE__);
        return -1;
    }

    /* register network adapter */
    dvb_net_init(&tcc_dxb->adapter, &dxb_demux->net, &dxb_demux->demux.dmx);
    return 0;
}

int tcc_dmx_deinit(struct tcc_dmx_instance *dxb_demux)
{
	int i;

    dvb_net_release(&dxb_demux->net);
    dxb_demux->demux.dmx.remove_frontend(&dxb_demux->demux.dmx, &dxb_demux->fe_mem);
    for (i = 0; i < FE_DEV_COUNT; i++) {
    	dxb_demux->demux.dmx.remove_frontend(&dxb_demux->demux.dmx, &dxb_demux->fe_hw[i]);
    }
    dvb_dmxdev_release(&dxb_demux->dmxdev);
    dvb_dmx_release(&dxb_demux->demux);

    return 0;
}

