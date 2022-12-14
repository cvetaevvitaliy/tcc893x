/* 
 * arch/arm/mach-tcc893x/tca_spi.c
 *
 * Author:  <linux@telechips.com>
 * Created: 10th Jun, 2008 
 * Description: Telechips Linux SPI Driver
 *
 * Copyright (c) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/gpio.h>
#include <mach/io.h>
#include <mach/bsp.h>
#include <mach/tca_spi.h>

static int tca_spi_isenabledma(struct tca_spi_handle *h)
{
    return (h->regs->DMACTR & Hw0) ? 1 : 0;
}

static int tca_spi_dmastop(struct tca_spi_handle *h)
{
	if (!h->tx_pkt_remain) {
		BITCLR(h->regs->MODE, Hw3); /* Operation Disable */
	}

    BITCLR(h->regs->DMACTR, Hw31 | Hw30); /* disable DMA Transmit & Receive */
    BITSET(h->regs->DMAICR, Hw29| Hw28);
    BITCLR(h->regs->DMACTR, Hw0); /* DMA disable */

	return 0;
}

static int tca_spi_dmastop_slave(struct tca_spi_handle *h)
{
    BITCLR(h->regs->DMACTR, Hw31 | Hw30); /* disable DMA Transmit & Receive */
    BITSET(h->regs->DMAICR, Hw29| Hw28);
    BITCLR(h->regs->DMACTR, Hw0); /* DMA disable */

	return 0;
}

static int tca_spi_dmastart(struct tca_spi_handle *h)
{
	if (h->ctf) {
		BITSET(h->regs->MODE, Hw4);
	} else {
		BITCLR(h->regs->MODE, Hw4);
	}

	BITSET(h->regs->MODE, Hw3); /* Operation Enable */

    BITSET(h->regs->DMACTR, Hw31 | Hw30); /* enable DMA Transmit & Receive */
    BITCLR(h->regs->DMACTR, Hw17 | Hw16 | Hw15 | Hw14); /* set Multiple address mode */

    BITCLR(h->regs->DMAICR, Hw16); /* disable DMA Packet Interrupt */
    BITSET(h->regs->DMAICR, Hw17); /* enable DMA Done Interrupt */
    BITCLR(h->regs->DMAICR, Hw20); /* set rx interrupt */

    BITSET(h->regs->DMACTR, Hw0); /* DMA enable */
	return 0;
}

static int tca_spi_dmastart_slave(struct tca_spi_handle *h)
{
    //BITSET(h->regs->DMACTR, Hw31 | Hw30); /* enable DMA Transmit & Receive */
    BITSET(h->regs->DMACTR, Hw30); /* enable DMA Transmit */
    BITCLR(h->regs->DMACTR, Hw17 | Hw16 | Hw15 | Hw14); /* set Multiple address mode */

    BITSET(h->regs->DMAICR, Hw16); /* enable DMA Packet Interrupt */
    //BITSET(h->regs->DMAICR, Hw17); /* enable DMA Done Interrupt */
    BITCLR(h->regs->DMAICR, Hw20); /* set rx interrupt */

    //h->regs->DMAICR = h->regs->DMAICR | (256 & 0x1FFF);
    BITSET(h->regs->DMAICR, Hw16); /* enable DMA Packet Interrupt */

	if (h->dma_mode == 0) {
		BITCSET(h->regs->DMACTR, Hw5|Hw4, Hw29);	/* Normal mode & Continuous mode*/
	} else {
		BITSET(h->regs->DMACTR, Hw4);				/* MPEG2-TS mode */
	}

    BITSET(h->regs->DMACTR, Hw0); /* DMA enable */
	return 0;
}

static void tca_spi_clearfifopacket(struct tca_spi_handle *h)
{
    /* clear tx/rx FIFO & Packet counter  */
    BITSET(h->regs->MODE, Hw15 | Hw14);
    BITSET(h->regs->DMACTR, Hw2);
    BITCLR(h->regs->DMACTR, Hw2);
    BITCLR(h->regs->MODE, Hw15 | Hw14);
}

static void tca_spi_setpacketcnt(struct tca_spi_handle *h, int size)
{
    /* set packet count & size */
    h->regs->PACKET = (size & 0x1FFF);
}

static void tca_spi_setpacketcnt_slave(struct tca_spi_handle *h, int size)
{
    unsigned int packet_cnt = (h->dma_total_packet_cnt & 0x1FFF) - 1;
    unsigned int packet_size = (MPEG_PACKET_SIZE & 0x1FFF);
    unsigned int intr_packet_cnt = (h->dma_intr_packet_cnt & 0x1FFF) - 1;

    if (packet_size != size) {
        size = packet_size;
    }

    h->regs->PACKET = (packet_cnt << 16) | packet_size;
    BITCSET(h->regs->DMAICR, 0x1FFF, intr_packet_cnt);
}

static void tca_spi_setbitwidth(struct tca_spi_handle *h, int width)
{
    int width_value = (width - 1) & 0x1F;

    /* set bit width */
    BITCLR(h->regs->MODE, Hw12 | Hw11 | Hw10 | Hw9 | Hw8);
    h->regs->MODE |= (width_value << 8);
    if (width_value & Hw4) {
        BITCLR(h->regs->DMACTR, Hw28);
    } else {
        BITSET(h->regs->DMACTR, Hw28);
    }
}

static void tca_spi_setdmaaddr(struct tca_spi_handle *h)
{
    /* set dma txbase/rxbase & request DMA tx/rx */
    h->regs->TXBASE = h->tx_dma.dma_addr;
    h->regs->RXBASE = h->rx_dma.dma_addr;

    h->regs->INTEN = h->tx_dma.dma_addr ? (h->regs->INTEN | Hw31) : (h->regs->INTEN & ~Hw31);
    h->regs->INTEN = h->rx_dma.dma_addr ? (h->regs->INTEN | Hw30) : (h->regs->INTEN & ~Hw30);
}

static void tca_spi_setdmaaddr_slave(struct tca_spi_handle *h)
{
    /* set dma txbase/rxbase & request DMA tx/rx */
    h->regs->RXBASE = h->rx_dma.dma_addr;
    //Hw18~Hw16 : Revceive FIFO threshold for interrupt/DMA request
    h->regs->INTEN = h->rx_dma.dma_addr ? (h->regs->INTEN | Hw30|Hw18|Hw17|Hw16|Hw15) : (h->regs->INTEN & ~Hw30);
}

static void tca_spi_hwinit(struct tca_spi_handle *h)
{
    /* init => set SPI mode, set Master mode ... */
    memset((void *)(h->regs), 0, sizeof(struct tca_spi_regs));

    h->set_bit_width(h, 32);

    h->set_dma_addr(h);

	/* Clear Fifo must be operated before operation enable */
    h->clear_fifo_packet(h);

    /* [SCK] Tx: risiing edge, Rx: falling edge */
    BITSET((h->regs)->MODE, Hw17 | Hw18);
}

static void tca_spi_hwinit_slave(struct tca_spi_handle *h)
{
    /* init => set SPI mode, set Master mode ... */
    memset((void *)(h->regs), 0, sizeof(struct tca_spi_regs));

    h->set_bit_width(h, 32);

    h->set_dma_addr(h);

	/* Clear Fifo must be operated before operation enable */
    h->clear_fifo_packet(h);

    /* [SCK] Tx: risiing edge, Rx: falling edge & enable Operation */
    BITSET((h->regs)->MODE, Hw17 | Hw18 | Hw3);
    BITSET((h->regs)->MODE, Hw2 | Hw5);
}

static void tca_spi_set_mpegtspidmode(struct tca_spi_handle *h, int is_set)
{
    if (is_set) {
        BITSET(h->regs->DMACTR, Hw19 | Hw18);
    } else {
        BITCLR(h->regs->DMACTR, Hw19 | Hw18);
    }
}

static int tca_spi_set_port(struct tca_spi_handle *h)
{
    static int init_port = 1;
	int ret = 0, gpsb_channel, gpsb_port;
	volatile PGPSBPORTCFG gpsb_pcf_regs = (volatile PGPSBPORTCFG)tcc_p2v(HwGPSB_PORTCFG_BASE);
    int i;

    if(init_port)
    {
  	/* The port map value for each channel should be different.
	 * clear gpsb port config except used port (ch0, ch1)
	 */
	    BITSET(gpsb_pcf_regs->PCFG0.nREG, Hw32-Hw0);	// GPS use ch2
    	BITSET(gpsb_pcf_regs->PCFG1.nREG, Hw16-Hw0);
        init_port = 0;
    }

    gpsb_channel = h->gpsb_channel;
    gpsb_port =  h->gpsb_port;
    printk("%s:channel[%d]port[%d]\n", __func__, gpsb_channel, gpsb_port);
	if(h->gpsb_channel <= 3){
       BITCSET(gpsb_pcf_regs->PCFG0.nREG, (Hw8-Hw0)<<(gpsb_channel*8), gpsb_port<<(gpsb_channel*8)); 
       for(i=0; i <= 3; i++) {
	       if (i == h->gpsb_channel) continue;
	       if (((gpsb_pcf_regs->PCFG0.nREG & (Hw8-Hw0)<<(i*8)) >> (i*8)) == gpsb_port) 
		       BITCSET(gpsb_pcf_regs->PCFG0.nREG, (Hw8-Hw0)<<(i*8), 0xFF<<(i*8));
       }
    }else{
       BITCSET(gpsb_pcf_regs->PCFG1.nREG, (Hw8-Hw0)<<((gpsb_channel-4)*8), gpsb_port<<((gpsb_channel-4)*8)); 
       for(i=4; i <= 5; i++) {
	       if (i == h->gpsb_channel) continue;
	       if (((gpsb_pcf_regs->PCFG1.nREG & (Hw8-Hw0)<<((i-4)*8)) >> ((i-4)*8)) == gpsb_port) 
		       BITCSET(gpsb_pcf_regs->PCFG1.nREG, (Hw8-Hw0)<<((i-4)*8), 0xFF<<((i-4)*8));
       }
    }
	switch (h->gpsb_port) {
		case 2:
		#if defined(CONFIG_CHIP_TCC8930) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
			tcc_gpio_config(TCC_GPD(4), GPIO_FN(6));
			tcc_gpio_config(TCC_GPD(5), GPIO_FN(6));
			tcc_gpio_config(TCC_GPD(6), GPIO_FN(6));
			tcc_gpio_config(TCC_GPD(7), GPIO_FN(6));
		#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933) 
			tcc_gpio_config(TCC_GPD(22), GPIO_FN(15));
			tcc_gpio_config(TCC_GPD(23), GPIO_FN(15));
			tcc_gpio_config(TCC_GPD(24), GPIO_FN(15));
			tcc_gpio_config(TCC_GPD(25), GPIO_FN(15));
		#endif
			break;
		case 3:
		#if defined(CONFIG_CHIP_TCC8930) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
			tcc_gpio_config(TCC_GPD(11), GPIO_FN(6));
			tcc_gpio_config(TCC_GPD(12), GPIO_FN(6));
			tcc_gpio_config(TCC_GPD(13), GPIO_FN(6));
			tcc_gpio_config(TCC_GPD(14), GPIO_FN(6));
		#endif
			break;
		case 4:
		{
		    tcc_gpio_config(TCC_GPD(17), GPIO_FN(6));
		    tcc_gpio_config(TCC_GPD(18), GPIO_FN(6));
		    tcc_gpio_config(TCC_GPD(19), GPIO_FN(6));
		    tcc_gpio_config(TCC_GPD(20), GPIO_FN(6));
			break;
		}
		case 5:
		{
		    tcc_gpio_config(TCC_GPB(0), GPIO_FN(9));
		    tcc_gpio_config(TCC_GPB(1), GPIO_FN(9));
		    tcc_gpio_config(TCC_GPB(2), GPIO_FN(9));
		    tcc_gpio_config(TCC_GPB(3), GPIO_FN(9));
			break;
		}
		case 6:
		{
		    tcc_gpio_config(TCC_GPB(11), GPIO_FN(9));
		    tcc_gpio_config(TCC_GPB(12), GPIO_FN(9));
		    tcc_gpio_config(TCC_GPB(13), GPIO_FN(9));
		    tcc_gpio_config(TCC_GPB(14), GPIO_FN(9));
			break;
		}
		case 9:
		{
		    tcc_gpio_config(TCC_GPC(14), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPC(15), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPC(16), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPC(17), GPIO_FN(5));
			break;
		}
	    case 10:
		{
		    tcc_gpio_config(TCC_GPC(22), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPC(23), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPC(24), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPC(25), GPIO_FN(5));
			break;
		}
		case 11:
		{
		    tcc_gpio_config(TCC_GPC(28), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPC(29), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPC(30), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPC(31), GPIO_FN(5));
			break;
		}
		case 12:
		{
		#if defined(CONFIG_CHIP_TCC8930) || defined(CONFIG_CHIP_TCC8935)
		    tcc_gpio_config(TCC_GPE(17), GPIO_FN(6));
		    tcc_gpio_config(TCC_GPE(18), GPIO_FN(6));
		    tcc_gpio_config(TCC_GPE(19), GPIO_FN(6));
		    tcc_gpio_config(TCC_GPE(20), GPIO_FN(6));
		#elif defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
		    tcc_gpio_config(TCC_GPE(12), GPIO_FN(4));
		    tcc_gpio_config(TCC_GPE(13), GPIO_FN(4));
		    tcc_gpio_config(TCC_GPE(14), GPIO_FN(4));
		    tcc_gpio_config(TCC_GPE(15), GPIO_FN(4));
		#endif
			break;
		}
		case 13:
		{
		    tcc_gpio_config(TCC_GPE(24), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPE(25), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPE(26), GPIO_FN(5));
		    tcc_gpio_config(TCC_GPE(27), GPIO_FN(5));
			break;
		}
		case 17:
		{
		    tcc_gpio_config(TCC_GPG(0), GPIO_FN(2));
		    tcc_gpio_config(TCC_GPG(1), GPIO_FN(2));
		    tcc_gpio_config(TCC_GPG(2), GPIO_FN(2));
		    tcc_gpio_config(TCC_GPG(3), GPIO_FN(2));
			break;
		}
		default:
		{
			ret = -1;
			break;
		}
	}
	return ret;
}

static int tca_spi_clear_port(struct tca_spi_handle *h)
{
	int ret = 0;
	/* set "GPIO output low" */
	switch (h->gpsb_port) {
		case 2:
		{
		#if defined(CONFIG_CHIP_TCC8930) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
		    tcc_gpio_config(TCC_GPD(4), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPD(5), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPD(6), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPD(7), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933)
		    tcc_gpio_config(TCC_GPD(22), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPD(23), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPD(24), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPD(25), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		#endif
		    break;
		}
		case 3:
		{
		#if defined(CONFIG_CHIP_TCC8930) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
			tcc_gpio_config(TCC_GPD(11), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
			tcc_gpio_config(TCC_GPD(12), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
			tcc_gpio_config(TCC_GPD(13), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
			tcc_gpio_config(TCC_GPD(14), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		#endif
			break;
		}
		case 4:
		{
   		    tcc_gpio_config(TCC_GPD(17), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPD(18), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPD(19), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPD(20), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
			break;
		}
		case 5:
		{
		    tcc_gpio_config(TCC_GPB(0), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPB(1), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPB(2), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPB(3), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
			break;
		}
		case 6:
		{
  		    tcc_gpio_config(TCC_GPB(11), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPB(12), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPB(13), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPB(24), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
			break;
		}
		case 9:
		{
		    tcc_gpio_config(TCC_GPC(14), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPC(15), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPC(16), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPC(17), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
			break;
		}
	    case 10:
		{
		    tcc_gpio_config(TCC_GPC(22), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPC(23), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPC(24), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPC(25), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
			break;
		}
		case 11:
		{
		    tcc_gpio_config(TCC_GPC(28), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPC(29), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPC(30), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPC(31), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
			break;
		}
		case 12:
		{
		#if defined(CONFIG_CHIP_TCC8930) || defined(CONFIG_CHIP_TCC8935)
		    tcc_gpio_config(TCC_GPE(17), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPE(18), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPE(19), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPE(20), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		#elif defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
		    tcc_gpio_config(TCC_GPE(12), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPE(13), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPE(14), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPE(15), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		#endif
			break;
		}
		case 13:
		{
		    tcc_gpio_config(TCC_GPE(24), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPE(25), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPE(26), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPE(27), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
			break;
		}
		case 17:
		{
		    tcc_gpio_config(TCC_GPG(0), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPG(1), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPG(2), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
		    tcc_gpio_config(TCC_GPG(3), GPIO_FN(0)|GPIO_OUTPUT|GPIO_LOW);
			break;
		}
		default:
		{
			ret = -1;
			break;
		}
	}
	return ret;
}

/******************************
 * return value
 *
 * ret == 0: success
 * ret > 0 or ret < 0: fail
 ******************************/
int tca_spi_init(tca_spi_handle_t *h,
                 volatile struct tca_spi_regs *regs,
                 int irq,
                 dma_alloc_f tea_dma_alloc,
                 dma_free_f tea_dma_free,
                 int dma_size,
                 int id,
                 int is_slave,
                 int port,
                 const char *gpsb_name)
{
    int ret = -1;


	volatile PPIC pic_regs = (volatile PPIC)tcc_p2v(HwPIC_BASE);

    if (h) { memset(h, 0, sizeof(tca_spi_handle_t)); }
    if (regs) {

		h->regs = regs;
        h->irq = irq;
        h->id = id;
		h->is_slave = is_slave;
		h->gpsb_port = port;

		if(!strcmp(gpsb_name,"gpsb0"))
            h->gpsb_channel = 0;
        else if(!strcmp(gpsb_name,"gpsb1"))
            h->gpsb_channel = 1;
        else if(!strcmp(gpsb_name,"gpsb2"))
            h->gpsb_channel = 2;
        else
            return ret;

        h->dma_stop = is_slave ? tca_spi_dmastop_slave : tca_spi_dmastop;
        h->dma_start = is_slave ? tca_spi_dmastart_slave : tca_spi_dmastart;
        h->clear_fifo_packet = tca_spi_clearfifopacket;
        h->set_packet_cnt = is_slave ? tca_spi_setpacketcnt_slave : tca_spi_setpacketcnt;
        h->set_bit_width = tca_spi_setbitwidth;
        h->set_dma_addr = is_slave ? tca_spi_setdmaaddr_slave : tca_spi_setdmaaddr;
        h->hw_init = is_slave ? tca_spi_hwinit_slave : tca_spi_hwinit;
        h->is_enable_dma = tca_spi_isenabledma;
        h->set_mpegts_pidmode = tca_spi_set_mpegtspidmode;

        h->tea_dma_alloc = tea_dma_alloc;
        h->tea_dma_free = tea_dma_free;

        h->dma_total_size = dma_size;
		h->dma_mode = 1;	/* default MPEG2-TS DMA mode */

		h->ctf = 0;
		h->tx_pkt_remain= 0;

        /* interrupt init */
		BITSET(pic_regs->MODE1.nREG, (INT_GPSB-32));	// level-trigger
		BITCLR(pic_regs->POL1.nREG, (INT_GPSB-32));	// active-high

		if (tca_spi_set_port(h)) {
			goto exit;
		}

        if (h->tea_dma_alloc) {
			if (h->tea_dma_alloc(&(h->rx_dma), dma_size) == 0) {
                if (is_slave) {
                    ret = 0;
                } else if (h->tea_dma_alloc(&(h->tx_dma), dma_size) == 0 && h->tea_dma_alloc(&(h->tx_dma_1), dma_size) == 0) {
                    ret = 0;
                }
            }
        } else {
        	/* Already, tsif has rx_dma buf */
        	ret = 0;
        }

exit:
        if (ret) { tca_spi_clean(h); }
    }
	
    return ret;
}

void tca_spi_clean(tca_spi_handle_t *h)
{
    if (h) {
        if (h->tea_dma_free) {
			h->tea_dma_free(&(h->tx_dma));
			h->tea_dma_free(&(h->rx_dma));
			h->tea_dma_free(&(h->tx_dma_1));
		}

		/* release GPIO */
		tca_spi_clear_port(h);

		memset(h, 0, sizeof(tca_spi_handle_t));
    }
}

int tca_spi_register_pids(tca_spi_handle_t *h, unsigned int *pids, unsigned int count)
{
    int ret = 0, gpsb_channel = -1;
    gpsb_channel = h->gpsb_channel;

    //supporting pids is 32 
    if (count <= 32) {
        volatile unsigned long* PIDT;
        int i = 0, pid_ch;

        if(gpsb_channel == 0)
            pid_ch = HwGPSB_PIDT_CH0;
        else if(gpsb_channel == 1)
            pid_ch = HwGPSB_PIDT_CH1;
        else if(gpsb_channel == 2)
            pid_ch = HwGPSB_PIDT_CH2;

        for (i = 0; i < 32; i++) {
            PIDT = (volatile unsigned long *)tcc_p2v(HwGPSB_PIDT(i));
            *PIDT = 0;
        }
        if (count > 0) {
            //check bypass mode
            if(count == 1 && pids[0] == 0x2000)
            {
                h->set_mpegts_pidmode(h, 0);
                printk("%s:Bypass mode !!!\n",__func__);
                return 0;
            }

            for (i = 0; i < count; i++) {
                PIDT = (volatile unsigned long *)tcc_p2v(HwGPSB_PIDT(i));
                *PIDT = pids[i] & 0x1FFFFFFF;
                BITSET(*PIDT, pid_ch);
                printk("PIDT 0x%08X : 0x%08X\n", (unsigned int)PIDT, (unsigned int)*PIDT);

            }
            h->set_mpegts_pidmode(h, 1);
        } 
    }
    else {
        printk("tsif: PID TABLE is so big !!!\n");
        ret = -EINVAL;
    }
    return ret;
}

EXPORT_SYMBOL(tca_spi_init);
EXPORT_SYMBOL(tca_spi_clean);
EXPORT_SYMBOL(tca_spi_register_pids);


/****************************************************************************
 * Below is SW Timer for Braodcasting.
 * It is used for calculating STC time for PCR time.
 * Timer should be 1ms resolution.
****************************************************************************/

#define	TCCTIMER_VALUE_MAX		(357913)//(0x100000000/12000) // 32bit max for 6000Khz

static volatile PTIMER pTCCTIMER = NULL;
static unsigned int guiRefTime, guiPrevTime, guiOffsetTime; //ms

int TCCREFTIME_Open(void)
{
	if (pTCCTIMER != NULL)
		return 1;

	pTCCTIMER	= (volatile PTIMER)tcc_p2v(HwTMR_BASE);
	guiRefTime = 0;
	guiPrevTime = 0;
	guiOffsetTime = 0;
	return 0;
}

int TCCREFTIME_Close(void)
{
	if(pTCCTIMER == NULL)
		return 0;
	
	pTCCTIMER = NULL;
	return 0;
}

unsigned int TCCREFTIME_GetTime(void)
{
	if(pTCCTIMER == NULL)
		return 0;	
    guiRefTime = pTCCTIMER->TC32MCNT.nREG/12000; //because 12Mhz, divided by 12000 needs
	if(guiRefTime < guiPrevTime )
		guiOffsetTime += TCCTIMER_VALUE_MAX;		
	guiPrevTime = guiRefTime;

	//PRINTF("%s :: %d",__func__, (guiRefTime+guiOffsetTime));	
	return (guiRefTime+guiOffsetTime);
}

void TCCREFTIME_TestMain(void)
{
	int start, check, sec;
	TCCREFTIME_Open();		
	
	//PRINTF("0x%x", pTCCTIMER->TCFG4);	
	//for(i=0;i<50;i++)
	start = TCCREFTIME_GetTime();
	sec = 0;
	while(1)
	{
		//PRINTF("0x%x :: %d\n", pTCCTIMER->TCNT4, pTCCTIMER->TCNT4);
		//PRINTF("REAL :: 0x%x :: %d", TCCREFTIME_GetTime(), TCCREFTIME_GetTime());		
		check = TCCREFTIME_GetTime();
		if( (check - start) == 1000 )
		{
			printk("%d second\n", ++sec);
			start = check;
		}		
		if(sec == 30)
			break;
	}
	TCCREFTIME_Close();
}

EXPORT_SYMBOL(TCCREFTIME_Open);
EXPORT_SYMBOL(TCCREFTIME_Close);
EXPORT_SYMBOL(TCCREFTIME_GetTime);

