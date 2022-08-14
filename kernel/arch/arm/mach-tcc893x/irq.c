/*
 * linux/arch/arm/mach-tcc893x/irq.c
 *
 * Author:  <linux@telechips.com>
 * Description: Interrupt handler for Telechips TCC893x chipset
 *
 * Copyright (C) 2011 Telechips, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/agpgart.h>
#include <linux/types.h>

#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <asm/io.h>

#include <mach/bsp.h>

#if defined(CONFIG_ARM_GIC)
#include <asm/hardware/gic.h>
#endif


// Global
static volatile PPIC pPIC;
static volatile PVIC pVIC;
static volatile PGPSBPORTCFG pPGPSBPORTCFG;
static volatile PUARTPORTCFG pUARTPORTCFG;
static volatile PGDMACTRL pPGDMACTRL0, pPGDMACTRL1, pPGDMACTRL2, pPGDMACTRL3;
static volatile PTIMER pTIMER;
static volatile PVIOC_IREQ_CONFIG pVIOC_IREQ_CONFIG;
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
static volatile PTSIFIRQSTATUS pPTSIFIRQSTATUS;
#endif
static unsigned int gvioc_mask0, gvioc_mask1;


/*********************************************************
 * Enable IRQ
 *********************************************************/
static void tcc893x_irq_enable(struct irq_data *data)
{
	int irq;
	if ((data->hwirq >= INT_VIOC_BASE) && (data->hwirq <= INT_VIOC_NUM)) {
		irq = data->hwirq - INT_VIOC_BASE;
		if (irq >= 32) {
			gvioc_mask1 |= (1 << (irq-32));
			pVIOC_IREQ_CONFIG->nIRQMASKCLR.nREG[1] = (1 << (irq-32));
		}
		else {
			gvioc_mask0 |= (1 << irq);
			pVIOC_IREQ_CONFIG->nIRQMASKCLR.nREG[0] = (1 << irq);
		}
	}
}


/*********************************************************
 * Disable IRQ
 *********************************************************/
static void tcc893x_irq_disable(struct irq_data *data)
{
	int irq;
	if ((data->hwirq >= INT_VIOC_BASE) && (data->hwirq <= INT_VIOC_NUM)) {
		irq = data->hwirq - INT_VIOC_BASE;
		if (irq >= 32) {
			gvioc_mask1 &= ~(1 << (irq-32));
			pVIOC_IREQ_CONFIG->nIRQMASKSET.nREG[1] = (1 << (irq-32));
		}
		else {
			gvioc_mask0 &= ~(1 << irq);
			pVIOC_IREQ_CONFIG->nIRQMASKSET.nREG[0] = (1 << irq);
		}
	}
}

/*********************************************************
 * Set type: Level (High / Low), Edge (High / Low)
 *********************************************************/
static int tcc893x_irq_set_type(struct irq_data *data, unsigned int flow_type)
{
    u32 irq = data->hwirq - GIC_SPI_OFFSET;
    u32 mask = 1 << (irq % 32);

    if (flow_type == IRQ_TYPE_LEVEL_LOW || flow_type == IRQ_TYPE_EDGE_FALLING)
    {
        if (irq < 32) {
            pPIC->POL0.nREG |= mask;
        }
        else {
            pPIC->POL1.nREG |= mask;
        }
    }
    else {
        if (irq < 32) {
            pPIC->POL0.nREG &= ~mask;
        }
        else {
            pPIC->POL1.nREG &= ~mask;
        }
    }

	return 0;
}


/*********************************************************
 *
 *  The cascade interrupt handlers 
 *
 *********************************************************/
unsigned int tcc893x_irq_uart_number(void)
{

	if (pUARTPORTCFG->ISTS.bREG.U0)
		return INT_UART0;
	else if (pUARTPORTCFG->ISTS.bREG.U1)
		return INT_UART1;
	else if (pUARTPORTCFG->ISTS.bREG.U2)
		return INT_UART2;
	else if (pUARTPORTCFG->ISTS.bREG.U3)
		return INT_UART3;
	else if (pUARTPORTCFG->ISTS.bREG.U4)
		return INT_UART4;
	else if (pUARTPORTCFG->ISTS.bREG.U5)
		return INT_UART5;
	else if (pUARTPORTCFG->ISTS.bREG.U6)
		return INT_UART6;
	else if (pUARTPORTCFG->ISTS.bREG.U7)
		return INT_UART7;
	else
        return INT_UART;
}

unsigned int tcc893x_irq_gpsb_number(void)
{

	if (pPGPSBPORTCFG->CIRQST.bREG.ISTD0)
		return INT_GPSB0_DMA;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTD1)
		return INT_GPSB1_DMA;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTD2)
		return INT_GPSB2_DMA;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC0)
		return INT_GPSB0_CORE;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC1)
		return INT_GPSB1_CORE;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC2)
		return INT_GPSB2_CORE;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC3)
		return INT_GPSB3_CORE;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC4)
		return INT_GPSB4_CORE;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC5)
		return INT_GPSB5_CORE;
	else 
		return INT_GPSB;
}

unsigned int tcc893x_irq_gdma_number(void)
{

	if (pPGDMACTRL0->CHCONFIG.bREG.MIS0)
		return INT_DMA0_CH0;
	else if (pPGDMACTRL0->CHCONFIG.bREG.MIS1)
		return INT_DMA0_CH1;
	else if (pPGDMACTRL0->CHCONFIG.bREG.MIS2)
		return INT_DMA0_CH2;
	else if (pPGDMACTRL1->CHCONFIG.bREG.MIS0)
		return INT_DMA1_CH0;
	else if (pPGDMACTRL1->CHCONFIG.bREG.MIS1)
		return INT_DMA1_CH1;
	else if (pPGDMACTRL1->CHCONFIG.bREG.MIS2)
		return INT_DMA1_CH2;
	else if (pPGDMACTRL2->CHCONFIG.bREG.MIS0)
		return INT_DMA2_CH0;
	else if (pPGDMACTRL2->CHCONFIG.bREG.MIS1)
		return INT_DMA2_CH1;
	else if (pPGDMACTRL2->CHCONFIG.bREG.MIS2)
		return INT_DMA2_CH2;
	else
		return INT_GDMA;
}

unsigned int tcc893x_irq_tc_number(void)
{
	if (pTIMER->TIREQ.bREG.TI0)
		return INT_TC_TI0;
	else if (pTIMER->TIREQ.bREG.TI1)
		return INT_TC_TI1;
	else if (pTIMER->TIREQ.bREG.TI2)
		return INT_TC_TI2;
	else if (pTIMER->TIREQ.bREG.TI3)
		return INT_TC_TI3;
	else if (pTIMER->TIREQ.bREG.TI4)
		return INT_TC_TI4;
	else if (pTIMER->TIREQ.bREG.TI5)
		return INT_TC_TI5;
	else
		return INT_TC0;
}


unsigned int tcc893x_irq_vioc_number(void)
{
    int i;
    int flag;
    unsigned int bitmask;

    // SYNCSTATUS ==>  0: async interrupt, 1: sync interrupt
    for (i = 0;i < (INT_VIOC_NUM - INT_VIOC_BASE + 1);i++) {
        if (i < 32) {
			bitmask = 1 << i;
            if (gvioc_mask0 & bitmask) {
                flag = pVIOC_IREQ_CONFIG->uIREQSELECT.nREG[0] & bitmask ? 
                        (pVIOC_IREQ_CONFIG->uSYNCSTATUS.nREG[0] & bitmask) : 
                        (pVIOC_IREQ_CONFIG->uRAWSTATUS.nREG[0] & bitmask);
                if (flag)       break;
            }
        }
        else {
			bitmask = 1 << (i-32);
            if (gvioc_mask1 & bitmask) {
                flag = pVIOC_IREQ_CONFIG->uIREQSELECT.nREG[1] & bitmask ? 
                        (pVIOC_IREQ_CONFIG->uSYNCSTATUS.nREG[1] & bitmask) : 
                        (pVIOC_IREQ_CONFIG->uRAWSTATUS.nREG[1] & bitmask);
                if (flag)       break;
            }
        }
    }

    if (i >= (INT_VIOC_NUM - INT_VIOC_BASE)) {
        /* clear interrupt */
        return INT_LCDC;
    }

    return (i + INT_VIOC_BASE);

}

unsigned int tcc893x_irq_tsif_number(void)
{
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
	if(pPTSIFIRQSTATUS->IRQSTS.bREG.DMA_CH0)
		return INT_TSIF_DMA0;
	else if(pPTSIFIRQSTATUS->IRQSTS.bREG.DMA_CH1)
		return INT_TSIF_DMA1;
	else if(pPTSIFIRQSTATUS->IRQSTS.bREG.DMA_CH2)
		return INT_TSIF_DMA2;
    else
        return INT_TSIF;
#else
    return INT_TSIF;
#endif

}






void __init tcc_init_irq(void)
{
	printk(KERN_DEBUG "%s\n", __func__);


    /*********************************************************
     * Disable the VIC block
     *********************************************************/

	//reset interrupt
	pPIC = (volatile PPIC)tcc_p2v(HwPIC_BASE);
	pVIC = (volatile PVIC)tcc_p2v(HwVIC_BASE);
	pPGPSBPORTCFG = (volatile PGPSBPORTCFG)tcc_p2v(HwGPSB_PORTCFG_BASE);
	pUARTPORTCFG = (volatile PUARTPORTCFG)tcc_p2v(HwUART_PORTCFG_BASE);
	pPGDMACTRL0 = (volatile PGDMACTRL)tcc_p2v(HwGDMA0_BASE);
	pPGDMACTRL1 = (volatile PGDMACTRL)tcc_p2v(HwGDMA1_BASE);
	pPGDMACTRL2 = (volatile PGDMACTRL)tcc_p2v(HwGDMA2_BASE);
	pTIMER = (volatile PTIMER)tcc_p2v(HwTMR_BASE);
    pVIOC_IREQ_CONFIG = (volatile PVIOC_IREQ_CONFIG)tcc_p2v(HwVIOC_IREQ);
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
	pPTSIFIRQSTATUS = (volatile PTSIFIRQSTATUS)tcc_p2v(HwTSIF_IRQSTS_BASE);
#endif

	/* ADD IOREMAP */

	//clear IEN Field
	pPIC->IEN0.nREG = (unsigned long long)0x00000000; // All Interrupt Disable
	pPIC->IEN1.nREG = (unsigned long long)0x00000000; // All Interrupt Disable

	//clear SEL Field
	pPIC->SEL0.nREG = (unsigned long long)0xFFFFFFFF; //using IRQ
	pPIC->SEL1.nREG = (unsigned long long)0xFFFFFFFF; //using IRQ

	//clear TIG Field
	pPIC->TIG0.nREG = (unsigned long long)0x00000000; //Test Interrupt Disable
	pPIC->TIG1.nREG = (unsigned long long)0x00000000; //Test Interrupt Disable

	//clear POL Field
	pPIC->POL0.nREG = (unsigned long long)0x00000000; //Default ACTIVE Low
	pPIC->POL1.nREG = (unsigned long long)0x00000000; //Default ACTIVE Low

	//clear MODE Field
	pPIC->MODE0.nREG = (unsigned long long)0xFFFFFFFF; //Trigger Mode - Level Trigger Mode
	pPIC->MODE1.nREG = (unsigned long long)0xFFFFFFFF; //Trigger Mode - Level Trigger Mode

	//clear SYNC Field
	pPIC->SYNC0.nREG = (unsigned long long)0xFFFFFFFF; //SYNC Enable
	pPIC->SYNC1.nREG = (unsigned long long)0xFFFFFFFF; //SYNC Enable

	//clear WKEN Field
	pPIC->WKEN0.nREG = (unsigned long long)0x00000000; //Wakeup all disable
	pPIC->WKEN1.nREG = (unsigned long long)0x00000000; //Wakeup all disable

	//celar MODEA Field
	pPIC->MODEA0.nREG = (unsigned long long)0x00000000; //both edge - all disable
	pPIC->MODEA1.nREG = (unsigned long long)0x00000000; //both edge - all disable

	//clear INTMSK Field
	pPIC->INTMSK0.nREG = (unsigned long long)0x00000000; //not using INTMSK
	pPIC->INTMSK1.nREG = (unsigned long long)0x00000000; //not using INTMSK

	//clear ALLMSK Field
	pPIC->ALLMSK.bREG.IRQ = 1; //using only IRQ
	pPIC->ALLMSK.bREG.FIQ = 0;



    /*********************************************************
     * Initialize the GIC block
     *********************************************************/
    gic_arch_extn.irq_unmask    = tcc893x_irq_enable;
    gic_arch_extn.irq_mask      = tcc893x_irq_disable;
    gic_arch_extn.irq_set_type  = tcc893x_irq_set_type;

	gic_init(0, 29, (void __iomem *)tcc_p2v(HwARMPERI_INT_DIST_BASE), (void __iomem *)tcc_p2v(HwARMPERI_GIC_BASE));


	/*********************************************************
	 *  Configure the cascade interrupt handlers 
	 *
	 *   - UART
	 *   - GPSB
	 *   - GDMA
	 *   - TimerCount0
	 *   - LCDC
	 *   - TSIF
	 *********************************************************/
	gic_cascade_irq(0, INT_UART);
	gic_cascade_irq(0, INT_GPSB);
	gic_cascade_irq(0, INT_GDMA);
	gic_cascade_irq(0, INT_TC0 );
	gic_cascade_irq(0, INT_LCDC);
	gic_cascade_irq(0, INT_TSIF);


    /* Video Input Output Control */
    pVIOC_IREQ_CONFIG->nIRQMASKSET.nREG[0] = 0xffffffff;    /* Disable VIOC IRQ */
    pVIOC_IREQ_CONFIG->nIRQMASKSET.nREG[1] = 0xffffffff;

    gvioc_mask0 = 0;                                        /* Clear mask value for VIOC */
    gvioc_mask1 = 0;

}

/* end of file */
