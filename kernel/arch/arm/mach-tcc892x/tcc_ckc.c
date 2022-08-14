
/****************************************************************************

 Copyright (C) 2013 Telechips Inc.

 This program is free software; you can redistribute it and/or modify it under the terms
 of the GNU General Public License as published by the Free Software Foundation;
 either version 2 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 Suite 330, Boston, MA 02111-1307 USA
 ****************************************************************************/


#include <mach/bsp.h>
#include <asm/io.h>
#include <linux/mm.h>       // for PAGE_ALIGN
#include <linux/kernel.h>
#include <linux/module.h>
#include <mach/tcc_ckc.h>

#if defined(CONFIG_CHIP_TCC8925S)
#define CPU_SRC_CH  0
#define CPU_SRC_PLL             CLKCTRL_SEL_PLL0

#define MEM_SRC_CH  1
#define MEM_SRC_PLL             CLKCTRL_SEL_PLL1
#else
#define CPU_SRC_CH  5
#define CPU_SRC_PLL             CLKCTRL_SEL_PLL5

#define MEM_SRC_CH  4
#define MEM_SRC_PLL             CLKCTRL_SEL_PLL4
#endif
#if defined(CONFIG_CHIP_TCC8925S)
#define MAX_TCC_PLL             4
#else
#define MAX_TCC_PLL             6
#endif
#define MAX_CLK_SRC             (MAX_TCC_PLL*2 + 1)     // XIN

#define CKC_BASE                HwCKC_BASE
#define REG_CLKCTRL             io_p2v(CKC_BASE+0x000)
#define REG_PLL                 io_p2v(CKC_BASE+0x030)
#define REG_CLKDIVC             io_p2v(CKC_BASE+0x050)
#define REG_SWRESET             io_p2v(CKC_BASE+0x064)
#define REG_PCLKCTRL            io_p2v(CKC_BASE+0x080)


/****************************************************************************************
* Register Configurations
* ***************************************************************************************/

/* PLL */
#define PLL_VCO_MAX             (2000*10000)
#define PLL_VCO_MIN             (1000*10000)
#define PLL_VCO_SEL             (1400*10000)
#define PLL_P_MAX               63
#define PLL_P_MIN               1
#define PLL_P_SHIFT             0
#define PLL_P_MASK              PLL_P_MAX
#define PLL_M_MAX               1023
#define PLL_M_MIN               16
#define PLL_M_SHIFT             8
#define PLL_M_MASK              PLL_M_MAX
#define PLL_S_MAX               5
#define PLL_S_MIN               0
#define PLL_S_SHIFT             24
#define PLL_S_MASK              0x7
#define PLL_LOCKEN_SHIFT        20
#define PLL_LOCKST_SHIFT        21
#define PLL_LOCK_MASK           0x1
#define PLL_VSEL_SHIFT          30
#define PLL_VSEL_MASK           0x1
#define PLL_EN_SHIFT            31
#define PLL_EN_MASK             0x1
#define tcc_pll_write(reg,en,vsel,p,m,s) { \
    volatile unsigned int i; \
    if (en) { \
        *(volatile unsigned *)reg = (((vsel&PLL_VSEL_MASK)<<PLL_VSEL_SHIFT)|((s&PLL_S_MASK)<<PLL_S_SHIFT)|(PLL_LOCK_MASK<<PLL_LOCKEN_SHIFT) \
                                |((m&PLL_M_MASK)<<PLL_M_SHIFT)|((p&PLL_P_MASK)<<PLL_P_SHIFT)); \
        for (i=100; i ; i--); \
        *(volatile unsigned *)reg |= (en&PLL_EN_MASK)<<PLL_EN_SHIFT; \
        for (i=100; i ; i--) \
            while(((*(volatile unsigned *)reg)&(PLL_LOCK_MASK<<PLL_LOCKST_SHIFT))==0); \
        for (i=0x20; i ; i--); \
    } else \
        *(volatile unsigned *)reg &= ~(PLL_EN_MASK<<PLL_EN_SHIFT); \
}

/* CLKCTRL */
#define CLKCTRL_SEL_MIN         0
#define CLKCTRL_SEL_MAX         15
#define CLKCTRL_SEL_SHIFT       0
#define CLKCTRL_SEL_MASK        CLKCTRL_SEL_MAX
#define CLKCTRL_CONFIG_MIN      1
#define CLKCTRL_CONFIG_MAX      15
#define CLKCTRL_CONFIG_SHIFT    4
#define CLKCTRL_CPU_MASK        0xFFFF
#define CLKCTRL_CONFIG_MASK     0xFFFF    //CLKCTRL_CONFIG_MAX
#define CLKCTRL_EN_SHIFT        21
#define CLKCTRL_EN_MASK         0x1
#define CLKCTRL_CFGRQ_SHIFT     29
#define CLKCTRL_CHGRQ_SHIFT     31
#define tcc_cpu_write(reg,en,config,sel) { \
    *(volatile unsigned *)reg = ((*(volatile unsigned *)reg)&(~(CLKCTRL_SEL_MASK<<CLKCTRL_SEL_SHIFT)))|((sel&CLKCTRL_SEL_MASK)<<CLKCTRL_SEL_SHIFT); \
    while((*(volatile unsigned *)reg) & (1<<CLKCTRL_CHGRQ_SHIFT)); \
    *(volatile unsigned *)reg = ((*(volatile unsigned *)reg)&(~(CLKCTRL_CPU_MASK<<CLKCTRL_CONFIG_SHIFT)))|((config&CLKCTRL_CPU_MASK)<<CLKCTRL_CONFIG_SHIFT); \
    while((*(volatile unsigned *)reg) & (1<<CLKCTRL_CFGRQ_SHIFT)); \
    *(volatile unsigned *)reg = ((*(volatile unsigned *)reg)&(~(CLKCTRL_EN_MASK<<CLKCTRL_EN_SHIFT)))|((en&CLKCTRL_EN_MASK)<<CLKCTRL_EN_SHIFT); \
    while((*(volatile unsigned *)reg) & (1<<CLKCTRL_CFGRQ_SHIFT)); \
}
#define tcc_clkctrl_write(reg,en,config,sel) { \
    *(volatile unsigned *)reg = ((*(volatile unsigned *)reg)&(~(CLKCTRL_SEL_MASK<<CLKCTRL_SEL_SHIFT)))|((sel&CLKCTRL_SEL_MASK)<<CLKCTRL_SEL_SHIFT); \
    while((*(volatile unsigned *)reg) & (1<<CLKCTRL_CHGRQ_SHIFT)); \
    *(volatile unsigned *)reg = ((*(volatile unsigned *)reg)&(~(CLKCTRL_CONFIG_MASK<<CLKCTRL_CONFIG_SHIFT)))|((config&CLKCTRL_CONFIG_MASK)<<CLKCTRL_CONFIG_SHIFT); \
    while((*(volatile unsigned *)reg) & (1<<CLKCTRL_CFGRQ_SHIFT)); \
    *(volatile unsigned *)reg = ((*(volatile unsigned *)reg)&(~(CLKCTRL_EN_MASK<<CLKCTRL_EN_SHIFT)))|((en&CLKCTRL_EN_MASK)<<CLKCTRL_EN_SHIFT); \
    while((*(volatile unsigned *)reg) & (1<<CLKCTRL_CFGRQ_SHIFT)); \
}
enum { /* CLKCTRL SEL */
    CLKCTRL_SEL_PLL0=0,
    CLKCTRL_SEL_PLL1,
    CLKCTRL_SEL_PLL2,
    CLKCTRL_SEL_PLL3,
    CLKCTRL_SEL_XIN,
    CLKCTRL_SEL_PLL0DIV,
    CLKCTRL_SEL_PLL1DIV,
    CLKCTRL_SEL_XTIN,
    CLKCTRL_SEL_PLL4,
    CLKCTRL_SEL_PLL5,
    CLKCTRL_SEL_PLL2DIV,
    CLKCTRL_SEL_PLL3DIV,
    CLKCTRL_SEL_PLL4DIV,
    CLKCTRL_SEL_PLL5DIV,
    CLKCTRL_SEL_XINDIV,
    CLKCTRL_SEL_XTINDIV,
};

/* PeriPheral Clocks */
#define PCLKCTRL_DIV_MIN        0
#define PCLKCTRL_DIV_DCO_MIN    1
#define PCLKCTRL_DIV_SHIFT      0
#define PCLKCTRL_DIV_XXX_MAX    0xFFF
#define PCLKCTRL_DIV_XXX_MASK   PCLKCTRL_DIV_XXX_MAX
#define PCLKCTRL_DIV_YYY_MAX    0xFFFF
#define PCLKCTRL_DIV_YYY_MASK   PCLKCTRL_DIV_YYY_MAX
#define PCLKCTRL_SEL_MIN        0
#define PCLKCTRL_SEL_MAX        31
#define PCLKCTRL_SEL_SHIFT      24
#define PCLKCTRL_SEL_MASK       PCLKCTRL_SEL_MAX
#define PCLKCTRL_EN_SHIFT       29
#define PCLKCTRL_EN_MASK        0x1
#define PCLKCTRL_MD_SHIFT       31
#define PCLKCTRL_MD_MASK        0x1
#define tcc_pclkctrl_write(reg,md,en,sel,div,type) { \
    if (type == PCLKCTRL_TYPE_XXX) { \
        *(volatile unsigned *)reg = ((en&PCLKCTRL_EN_MASK)<<PCLKCTRL_EN_SHIFT) \
                                    |((sel&PCLKCTRL_SEL_MASK)<<PCLKCTRL_SEL_SHIFT)|((div&PCLKCTRL_DIV_XXX_MASK)<<PCLKCTRL_DIV_SHIFT); \
    } else if (type == PCLKCTRL_TYPE_YYY) { \
        *(volatile unsigned *)reg = ((md&PCLKCTRL_MD_MASK)<<PCLKCTRL_MD_SHIFT)|((en&PCLKCTRL_EN_MASK)<<PCLKCTRL_EN_SHIFT) \
                                    |((sel&PCLKCTRL_SEL_MASK)<<PCLKCTRL_SEL_SHIFT)|((div&PCLKCTRL_DIV_YYY_MASK)<<PCLKCTRL_DIV_SHIFT); \
    } \
}
typedef enum { /* PCLK Type */
    PCLKCTRL_TYPE_XXX=0,
    PCLKCTRL_TYPE_YYY,
    PCLKCTRL_TYPE_MAX
} tPCLKTYPE;
enum { /* PCLK Mode Selection */
    PCLKCTRL_MODE_DCO=0,
    PCLKCTRL_MODE_DIVIDER,
    PCLKCTRL_MODE_MAX
};
enum{ /* Peri. Clock Source */
    PCLKCTRL_SEL_PLL0=0,
    PCLKCTRL_SEL_PLL1,
    PCLKCTRL_SEL_PLL2,
    PCLKCTRL_SEL_PLL3,
    PCLKCTRL_SEL_XIN,
    PCLKCTRL_SEL_PLL0DIV,            // 5
    PCLKCTRL_SEL_PLL1DIV,
    PCLKCTRL_SEL_PLL2DIV,
    PCLKCTRL_SEL_PLL3DIV,
    PCLKCTRL_SEL_XTIN,
    PCLKCTRL_SEL_HDMITMDS=11,
    PCLKCTRL_SEL_HDMIPCLK,
    PCLKCTRL_SEL_HDMIXIN, // 27Mhz
    PCLKCTRL_SEL_XINDIV=16,
    PCLKCTRL_SEL_XTINDIV,
    PCLKCTRL_SEL_PLL4,
    PCLKCTRL_SEL_PLL5,
    PCLKCTRL_SEL_PLL4DIV,            // 20
    PCLKCTRL_SEL_PLL5DIV,
};


/****************************************************************************************
* Local Variables
* ***************************************************************************************/
static volatile PCKC            pCKC ;
static volatile PPMU            pPMU ;
static volatile PIOBUSCFG       pIOBUSCFG;
static volatile PDDICONFIG      pDDIBUSCFG;
static volatile PGRPBUSCFG      pGPUBUSCFG;
static volatile PVIDEOBUSCFG    pVIDEOBUSCFG;
static volatile PHSIOBUSCFG     pHSIOBUSCFG;
static volatile PMEMBUSCFG      pMEMBUSCFG;
static volatile unsigned        stHDMIReg;
static volatile unsigned        stHDMIAudReg;
static volatile unsigned        stIPPWDNReg;

static unsigned int             stMinPLLCh;
static unsigned int             stMinPLLRate;
static unsigned int             stClockSource[MAX_CLK_SRC];
static unsigned int             temp_mem_src;
static unsigned int             temp_mem_div;
static unsigned int             temp_mem_rate;
static int initialized = 0;

extern unsigned int tcc_ddr_set_clock(unsigned int freq, unsigned int src, unsigned int div);

/****************************************************************************************
* Local Functions
* ***************************************************************************************/
static unsigned int tcc_ckc_getplldivder(unsigned int ch);


#if defined(CONFIG_HIBERNATION)
void tca_ckc_hibernate_init(void)
{
    initialized = 0;
    tca_ckc_init();
}
#endif

/****************************************************************************************
* FUNCTION :void tca_ckc_init(void)
* DESCRIPTION :
* ***************************************************************************************/
void tca_ckc_init(void)
{
    int i;
    unsigned int search_rate, search_src, search_div;
    pCKC = (CKC *)io_p2v(HwCKC_BASE);
    pPMU = (PMU *)io_p2v(HwPMU_BASE);
    pIOBUSCFG = (IOBUSCFG *)io_p2v(HwIOBUSCFG_BASE);
    pDDIBUSCFG = (DDICONFIG *)io_p2v(HwDDI_CONFIG_BASE);
    pGPUBUSCFG = (GRPBUSCFG *)io_p2v(HwGRPBUSCONFIG_BASE);
    pVIDEOBUSCFG = (VIDEOBUSCFG *)io_p2v(HwVIDEOBUSCONFIG_BASE);
    pHSIOBUSCFG = (HSIOBUSCFG *)io_p2v(HwHSIOBUSCFG_BASE);
    pMEMBUSCFG = (MEMBUSCFG *)io_p2v(HwMBUSCFG_BASE);

    if (initialized)
        return;

    /* IOBUS AHB2AXI: flushs prefetch buffer when bus state is IDLE or WRITE
       enable:  A2XMOD1 (Audio DMA, GPSB, DMA2/3, EHI1)
       disable: A2XMOD0 (USB1.1Host, USB OTG, SD/MMC, IDE, DMA0/1, MPEFEC, EHI0)
    */
#if 0
    pIOBUSCFG->IO_A2X.bREG.A2XMOD2 = 1;
    pIOBUSCFG->IO_A2X.bREG.A2XMOD1 = 1;
    pIOBUSCFG->IO_A2X.bREG.A2XMOD0 = 1;
    pHSIOBUSCFG->HSIO_CFG.bREG.A2X_USB20H = 3;

    /* Disable PLL Divider */
    *(volatile unsigned *)REG_CLKDIVC = 0x01010101;    // PLL0,PLL1,PLL2,PLL3
    *(volatile unsigned *)(REG_CLKDIVC+4) = 0x01010101;    // PLL4,PLL5,XIN,XTIN
#endif

    stHDMIReg = 0x24000000;
    stHDMIAudReg = 0x24000000;
    stIPPWDNReg = 0x00000000;

    for (i=0 ; i<MAX_TCC_PLL ; i++) {
        if (i == CPU_SRC_CH) {
            stClockSource[i] = 0;
            pr_info("    pll_%d:  cpu clock source\n", i);
        }
        else if (i == MEM_SRC_CH) {
            stClockSource[i] = 0;
            pr_info("    pll_%d:  memory clock source\n", i);
        }
        else {
            stClockSource[i] = tca_ckc_getpll(i);
            pr_info("    pll_%d:  %d kHz (Fixed)\n", i, stClockSource[i]/10);
        }

        if (stClockSource[i]) {
            if (!stMinPLLRate) {
                stMinPLLRate = stClockSource[i];
                stMinPLLCh = i;
            }
            else if (stClockSource[i] < stMinPLLRate) {
                stMinPLLRate = stClockSource[i];
                stMinPLLCh = i;
            }
        }
    }
    for ( ; i<(MAX_TCC_PLL*2) ; i++) {
        if ((i-MAX_TCC_PLL) == CPU_SRC_CH) {
            stClockSource[i] = 0;
            pr_info("div_pll_%d:  cpu clock source\n", i-MAX_TCC_PLL);
        }
        else if ((i-MAX_TCC_PLL) == MEM_SRC_CH) {
            stClockSource[i] = 0;
            pr_info("div_pll_%d:  memory clock source\n", i-MAX_TCC_PLL);
        }
        else {
            stClockSource[i] = 0;
            pr_info("div_pll_%d:  0 kHz (Fixed)\n", i-MAX_TCC_PLL);
//            stClockSource[i] = tca_ckc_getdividpll(i-MAX_TCC_PLL);
//            pr_info("div_pll_%d:  %d kHz (Fixed)\n", i-MAX_TCC_PLL, stClockSource[i]/10);
        }
    }

    stClockSource[i] = XIN_CLK_RATE;    // XIN
    pr_info("      xin:  %d kHz (Fixed)\n", stClockSource[i]/10);

    initialized = 1;

    temp_mem_div = 0;
    temp_mem_src = 0;
    temp_mem_rate = 0xFFFFFFFF;
    for (i=0 ; i<MAX_TCC_PLL ; i++) {
        if ((stClockSource[i] == 0) || (stClockSource[i] < 300*10000))
            continue;

        search_src = i;
        search_div = stClockSource[i]/(300*10000);
        search_rate = stClockSource[i]/search_div;

        if (search_rate < temp_mem_rate) {
            temp_mem_rate = search_rate;
            temp_mem_div = search_div;
            temp_mem_src = search_src;
        }
    }
}

inline int tcc_find_pms(tPMS *PLL)
{
    unsigned int fvco, m_mod_100, srch_p, srch_m, srch_m_mod_100;
    if (PLL->fpll ==0) {
        PLL->en = 0;
        return 0;
    }
    for (PLL->s=PLL_S_MAX,fvco=(PLL->fpll<<PLL_S_MAX) ; (PLL->s >= PLL_S_MIN) && PLL->s ; fvco=(PLL->fpll<<(--PLL->s))) {
        if (fvco >= PLL_VCO_MIN && fvco <= PLL_VCO_MAX)
            break;
    }
    if (fvco < PLL_VCO_MIN || fvco > PLL_VCO_MAX)
        return -1;
    PLL->vsel = (fvco > PLL_VCO_SEL)?1:0;
    PLL->p = PLL->m = 0;
    srch_m_mod_100 = 50;
    for (srch_p=PLL_P_MIN ; srch_p<=PLL_P_MAX ; srch_p++) {
        m_mod_100 = (srch_p*fvco)/(XIN_CLK_RATE/100);
        srch_m = (srch_p*fvco)/XIN_CLK_RATE;
        if (srch_m < PLL_M_MIN || srch_m > PLL_M_MAX)
            continue;
        m_mod_100 %= 100;
        if (m_mod_100 > 50)
            m_mod_100 = 100-m_mod_100;
        if (srch_m_mod_100 > m_mod_100) {
            srch_m_mod_100 = m_mod_100;
            PLL->p = srch_p;
            PLL->m = srch_m;
        }
    }
    if (srch_m_mod_100 == 50)
        return -1;
    PLL->fpll = ((PLL->m*XIN_CLK_RATE)/PLL->p)>>PLL->s;
    PLL->en = 1;
    return 0;
}

static inline void tcc_ckc_reset_clock_source(unsigned ch, unsigned value)
{
    if (ch >= MAX_CLK_SRC)
        return;

    if ((ch == CPU_SRC_CH) ||
        (ch == MEM_SRC_CH) ||
        0 ) {
        stClockSource[ch] = 0;
        stClockSource[MAX_TCC_PLL+ch] = 0;
        return;
    }

    if (ch < MAX_TCC_PLL) {
        stClockSource[ch] = tca_ckc_getpll(ch);
        stClockSource[MAX_TCC_PLL+ch] = tcc_ckc_getplldivder(ch);
    }
    else if (ch == MAX_CLK_SRC-1) // XIN
        stClockSource[ch] = XIN_CLK_RATE;
}

unsigned int tca_ckc_setpll(unsigned int fpll, unsigned int ch)  /* fpll(100Hz) */
{
    volatile unsigned   *PLLCFG = (volatile unsigned *)REG_PLL+ch;
    tPMS                nPLL;

    if (ch >= MAX_TCC_PLL)
        return 0;

    nPLL.fpll = fpll;
    if (tcc_find_pms(&nPLL))
        goto tca_ckc_setpll_failed;
    tcc_pll_write(PLLCFG, nPLL.en, nPLL.vsel, nPLL.p, nPLL.m, nPLL.s);
    tcc_ckc_reset_clock_source(ch, nPLL.fpll);
    return tca_ckc_getpll(ch);

tca_ckc_setpll_failed:
    tcc_pll_write(PLLCFG, 0, 0, PLL_P_MIN, PLL_M_MIN, PLL_S_MIN);
    return 0;
}

unsigned int tca_ckc_getpll(unsigned int ch)
{
    volatile unsigned   *PLLCFG = (volatile unsigned *)REG_PLL+ch;
    tPMS                nPLLCFG;

    if (ch >= MAX_TCC_PLL)
        return 0;

    nPLLCFG.p = ((*(volatile unsigned *)PLLCFG)>>PLL_P_SHIFT)&(PLL_P_MASK);
    nPLLCFG.m = ((*(volatile unsigned *)PLLCFG)>>PLL_M_SHIFT)&(PLL_M_MASK);
    nPLLCFG.s = ((*(volatile unsigned *)PLLCFG)>>PLL_S_SHIFT)&(PLL_S_MASK);
    nPLLCFG.en = ((*(volatile unsigned *)PLLCFG)>>PLL_EN_SHIFT)&(PLL_EN_MASK);

    if (nPLLCFG.en == 0)
        return 0;

    return (((XIN_CLK_RATE*nPLLCFG.m)/nPLLCFG.p)>>nPLLCFG.s);
}

static unsigned int tcc_ckc_getplldivder(unsigned int ch)
{
    volatile unsigned   *CLKDIVC;
    unsigned int        offset=0, fpll=0, pdiv=0;

    if (ch >= MAX_TCC_PLL)
        return 0;

    switch(ch) {
        case 0:
        case 1:
        case 2:
        case 3:
            CLKDIVC = (volatile unsigned *)REG_CLKDIVC;
            offset = (3-ch)*8;
            break;
        case 4:
        case 5:
            CLKDIVC = (volatile unsigned *)REG_CLKDIVC+4;
            offset = (3-(ch-4))*8;
            break;
        default:
            return 0;
    }
    if ((((*(volatile unsigned *)CLKDIVC) >> offset)&0x80) == 0)    /* check plldivc enable bit */
        return 0;
    pdiv = ((*(volatile unsigned *)CLKDIVC) >> offset)&0x3F;
    if (!pdiv)  /* should not be zero */
        return 0;
    fpll = tca_ckc_getpll(ch);
    return (unsigned int)fpll/(pdiv+1);
}

static inline int tcc_find_clkctrl(tCLKCTRL *CLKCTRL)
{
    unsigned int i, div[MAX_CLK_SRC], div_100[MAX_CLK_SRC], searchsrc, overclksrc;
    searchsrc = 0xFFFFFFFF;
    overclksrc = 0xFFFFFFFF;

#if (1) /* sometimes the clkctrl can not work, when it setted lower clock */
    if (CLKCTRL->freq < 480000)
        CLKCTRL->freq = 480000;
#endif

    if (CLKCTRL->freq <= (XIN_CLK_RATE/2)) {
        CLKCTRL->sel = CLKCTRL_SEL_XIN;
        CLKCTRL->freq = XIN_CLK_RATE/2;
        CLKCTRL->config = CLKCTRL_CONFIG_MIN;
    }
    else {
        for (i=0 ; i<MAX_CLK_SRC ; i++) {
            if (stClockSource[i] < CLKCTRL->freq || stClockSource[i] == 0)
                continue;
            div_100[i] = stClockSource[i]/(CLKCTRL->freq/100);
            if (div_100[i] > (CLKCTRL_CONFIG_MAX+1)*100)
                div_100[i] = (CLKCTRL_CONFIG_MAX+1)*100;
            /* find maximum frequency pll source */
            if (div_100[i] <= 100) {
                if (overclksrc == 0xFFFFFFFF)
                    overclksrc = i;
                else if (stClockSource[i] > stClockSource[overclksrc])
                    overclksrc = i;
                continue;
            }
            div[i]= div_100[i]/100;
            if (div_100[i]%100)
                div[i] += 1;
            if (div[i] < 2)
                div[i] = 2;
            div_100[i] = CLKCTRL->freq - stClockSource[i]/div[i];
            if (searchsrc == 0xFFFFFFFF)
                searchsrc = i;
            else {
                /* find similar clock */
                if (div_100[i] < div_100[searchsrc])
                    searchsrc = i;
                /* find even division vlaue */
                else if(div_100[i] == div_100[searchsrc]) {
                    if (div[searchsrc]%2)
                        searchsrc = i;
                    else if (div[searchsrc] > div[i])
                        searchsrc = i;
                }
            }
        }
        if (searchsrc == 0xFFFFFFFF) {
            if (overclksrc == 0xFFFFFFFF) {
                overclksrc = 0;
                for (i=1 ; i<MAX_CLK_SRC ; i++) {
                    if (stClockSource[i] > stClockSource[overclksrc])
                        overclksrc = i;
                }
            }
            searchsrc = overclksrc;
            div[searchsrc] = 2;
        }        
        switch(searchsrc) {
            case 0: CLKCTRL->sel = CLKCTRL_SEL_PLL0; break;
            case 1: CLKCTRL->sel = CLKCTRL_SEL_PLL1; break;
            case 2: CLKCTRL->sel = CLKCTRL_SEL_PLL2; break;
            case 3: CLKCTRL->sel = CLKCTRL_SEL_PLL3; break;
#if (MAX_TCC_PLL > 4)
            case 4: CLKCTRL->sel = CLKCTRL_SEL_PLL4; break;
            case 5: CLKCTRL->sel = CLKCTRL_SEL_PLL5; break;
            case 6: CLKCTRL->sel = CLKCTRL_SEL_PLL0DIV; break;
            case 7: CLKCTRL->sel = CLKCTRL_SEL_PLL1DIV; break;
            case 8: CLKCTRL->sel = CLKCTRL_SEL_PLL2DIV; break;
            case 9: CLKCTRL->sel = CLKCTRL_SEL_PLL3DIV; break;
            case 10: CLKCTRL->sel = CLKCTRL_SEL_PLL4DIV; break;
            case 11: CLKCTRL->sel = CLKCTRL_SEL_PLL5DIV; break;
            case 12: CLKCTRL->sel = CLKCTRL_SEL_XIN; break;
#else
            case 4: CLKCTRL->sel = CLKCTRL_SEL_PLL0DIV; break;
            case 5: CLKCTRL->sel = CLKCTRL_SEL_PLL1DIV; break;
            case 6: CLKCTRL->sel = CLKCTRL_SEL_PLL2DIV; break;
            case 7: CLKCTRL->sel = CLKCTRL_SEL_PLL3DIV; break;
            case 8: CLKCTRL->sel = CLKCTRL_SEL_XIN; break;
#endif
            default: return -1;
        }
        if (div[searchsrc] > (CLKCTRL_CONFIG_MAX+1))
            div[searchsrc] = CLKCTRL_CONFIG_MAX+1;
        else if (div[searchsrc] <= CLKCTRL_CONFIG_MIN)
            div[searchsrc] = CLKCTRL_CONFIG_MIN+1;
        CLKCTRL->freq = stClockSource[searchsrc]/div[searchsrc];
        CLKCTRL->config = div[searchsrc] - 1;
    }
    return 0;
}

unsigned int tca_ckc_setfbusctrl(unsigned int clkname, unsigned int isenable, unsigned int freq)  /* freq(100Hz) */
{
    volatile unsigned   *CLKCTRL = (volatile unsigned *)REG_CLKCTRL+clkname;
    tCLKCTRL            nCLKCTRL;

    if (clkname == FBUS_CPU) {
        tPMS                nPLL;
        volatile unsigned   *PLLCFG = (volatile unsigned *)REG_PLL+CPU_SRC_CH;
        tcc_cpu_write(CLKCTRL, 1, CLKCTRL_CPU_MASK, CLKCTRL_SEL_XIN);
        nPLL.fpll = freq;
        if (tcc_find_pms(&nPLL))
            return 0;
        tcc_pll_write(PLLCFG, nPLL.en, nPLL.vsel, nPLL.p, nPLL.m, nPLL.s);
        tcc_cpu_write(CLKCTRL, 1, CLKCTRL_CPU_MASK, CPU_SRC_PLL);
        return tca_ckc_getpll(CPU_SRC_CH);
    }
    else if (clkname == FBUS_MEM) {
        return tcc_ddr_set_clock(freq/10, temp_mem_src, temp_mem_div);
    }

    nCLKCTRL.freq = freq;
    if (tcc_find_clkctrl(&nCLKCTRL))
        return 0;

    switch (isenable) {
        case CKC_DISABLE:
            nCLKCTRL.en = 0;
            break;
        case CKC_ENABLE:
            nCLKCTRL.en = 1;
            break;
        default:
            nCLKCTRL.en = (*(volatile unsigned *)CLKCTRL)&(1<<CLKCTRL_EN_SHIFT) ? 1 : 0;
            break;
    }
    tcc_clkctrl_write(CLKCTRL, nCLKCTRL.en, nCLKCTRL.config, nCLKCTRL.sel);
    return nCLKCTRL.freq;
}

unsigned int tca_ckc_getfbusctrl(unsigned int clkname)
{
    volatile unsigned   *CLKCTRL = (volatile unsigned *)REG_CLKCTRL+clkname;
    tCLKCTRL            nCLKCTRL;
    unsigned int        src_freq = 0;

    nCLKCTRL.en = ((*(volatile unsigned *)CLKCTRL) & (1<<CLKCTRL_EN_SHIFT)) ? 1 : 0;
//    if (nCLKCTRL.en == 0)
//        return 0;

    nCLKCTRL.sel = ((*(volatile unsigned *)CLKCTRL) & (CLKCTRL_SEL_MASK<<CLKCTRL_SEL_SHIFT))>>CLKCTRL_SEL_SHIFT;
    switch (nCLKCTRL.sel) {
        case CLKCTRL_SEL_PLL0:
            src_freq =  tca_ckc_getpll(PLL_0);
            break;
        case CLKCTRL_SEL_PLL1:
            src_freq =  tca_ckc_getpll(PLL_1);
            break;
        case CLKCTRL_SEL_PLL2:
            src_freq =  tca_ckc_getpll(PLL_2);
            break;
        case CLKCTRL_SEL_PLL3:
            src_freq =  tca_ckc_getpll(PLL_3);
            break;
        case CLKCTRL_SEL_XIN:
            src_freq =  XIN_CLK_RATE;
            break;
        case CLKCTRL_SEL_PLL0DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_0);
            break;
        case CLKCTRL_SEL_PLL1DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_1);
            break;
        case CLKCTRL_SEL_XTIN:
            src_freq =  XTIN_CLK_RATE;
            break;
#if (MAX_TCC_PLL > 4)
        case CLKCTRL_SEL_PLL4:
            src_freq =  tca_ckc_getpll(PLL_4);
            break;
        case CLKCTRL_SEL_PLL5:
            src_freq =  tca_ckc_getpll(PLL_5);
            break;
#endif
        case CLKCTRL_SEL_PLL2DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_2);
            break;
        case CLKCTRL_SEL_PLL3DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_3);
            break;
#if (MAX_TCC_PLL > 4)
        case CLKCTRL_SEL_PLL4DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_4);
            break;
        case CLKCTRL_SEL_PLL5DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_5);
            break;
#endif
/*
        case CLKCTRL_SEL_XINDIV:
            src_freq =  XIN_CLK_RATE/2;
            break;
        case CLKCTRL_SEL_XTINDIV:
            src_freq =  XTIN_CLK_RATE/2;
            break;
*/
        default: return 0;
    }

    if(clkname == FBUS_CPU) {
        int i, lcnt=0;
        nCLKCTRL.config = ((*(volatile unsigned *)CLKCTRL) & (CLKCTRL_CPU_MASK<<CLKCTRL_CONFIG_SHIFT))>>CLKCTRL_CONFIG_SHIFT;
        for(i = 0; i < 16; i++) {
            if((nCLKCTRL.config & 0x1))
                lcnt++;
            nCLKCTRL.config = nCLKCTRL.config>>1;
        }
        nCLKCTRL.freq = (src_freq * lcnt)/16;
    }
    else {
        nCLKCTRL.config = ((*(volatile unsigned *)CLKCTRL) & (CLKCTRL_CONFIG_MASK<<CLKCTRL_CONFIG_SHIFT))>>CLKCTRL_CONFIG_SHIFT;
        nCLKCTRL.freq = src_freq / (nCLKCTRL.config+1);
    }

    return nCLKCTRL.freq;
}

static inline int tcc_find_pclk(tPCLKCTRL *PCLKCTRL, tPCLKTYPE type)
{
    unsigned int    div_min, div_max, div[MAX_CLK_SRC], div_100[MAX_CLK_SRC], i, searchsrc, overclksrc, dco_shift=0;

    switch (type) {
        case PCLKCTRL_TYPE_XXX:
            PCLKCTRL->md = PCLKCTRL_MODE_DIVIDER;
            div_max = PCLKCTRL_DIV_XXX_MAX+1;
            div_min = PCLKCTRL_DIV_MIN+1;
            break;
        case PCLKCTRL_TYPE_YYY:
            PCLKCTRL->md = PCLKCTRL_MODE_DCO;
            div_max = PCLKCTRL_DIV_YYY_MAX+1;
            div_min = PCLKCTRL_DIV_DCO_MIN+1;
#ifdef CONFIG_AUDIO_PLL_USE
            if (tcc_find_audio_pclk(PCLKCTRL) == 0)
                return 0;
#endif
            break;
        default:
            return -1;
    }

    searchsrc = 0xFFFFFFFF;
    overclksrc = 0xFFFFFFFF;
    if (PCLKCTRL->md == PCLKCTRL_MODE_DCO) {
        if (PCLKCTRL->freq < div_max)
            dco_shift = 0;
        else if (PCLKCTRL->freq < div_max*2)  //  13.1072 MHz
            dco_shift = 1;
        else if (PCLKCTRL->freq < div_max*4)  //  26.2144 MHz
            dco_shift = 2;
        else if (PCLKCTRL->freq < div_max*8)  //  52.4288 MHz
            dco_shift = 3;
        else if (PCLKCTRL->freq < div_max*16) // 104.8596 MHz
            dco_shift = 4;
        else                         // 209.7152 MHz
            dco_shift = 5;

        for (i=0 ; i<MAX_CLK_SRC ; i++) {
            if (stClockSource[i] == 0 || stClockSource[i] == XIN_CLK_RATE)    // remove XIN clock source
                continue;
            if (stClockSource[i] < PCLKCTRL->freq)
                continue;
            div_100[i] = ((PCLKCTRL->freq*(div_max>>dco_shift))/(stClockSource[i]/100))<<dco_shift;
            if ((div_100[i]%100) > 50) {
                div[i] = div_100[i]/100 + 1;
                div_100[i] = 100 - (div_100[i]%100);
            }
            else {
                div[i] = div_100[i]/100;
                div_100[i] %= 100;
            }
            if (searchsrc == 0xFFFFFFFF)
                searchsrc = i;
            else {
                /* find similar clock */
                if (div_100[i] < div_100[searchsrc])
                    searchsrc = i;
            }
        }
        if (searchsrc == 0xFFFFFFFF) {
            if (overclksrc == 0xFFFFFFFF) {
                overclksrc = 0;
                for (i=1 ; i<MAX_CLK_SRC ; i++) {
                    if (stClockSource[i] > stClockSource[overclksrc])
                        overclksrc = i;
                }
            }
            searchsrc = overclksrc;
            div[searchsrc] = 1;
        }
    }
    else { /* Divider mode */
        for (i=0 ; i<MAX_CLK_SRC ; i++) {
            if (stClockSource[i] == 0)
                continue;
            if (stClockSource[i] < PCLKCTRL->freq)
                continue;
            div_100[i] = stClockSource[i]/(PCLKCTRL->freq/100);
            if (div_100[i] > div_max*100)
                div_100[i] = div_max*100;
            if ((div_100[i]%100) > 50) {
                div[i] = div_100[i]/100 + 1;
                div_100[i] = 100 - (div_100[i]%100);
            }
            else {
                div[i] = div_100[i]/100;
                div_100[i] %= 100;
            }
            if (searchsrc == 0xFFFFFFFF)
                searchsrc = i;
            else {
                /* find similar clock */
                if (div_100[i] < div_100[searchsrc])
                    searchsrc = i;
                /* find even division vlaue */
                else if(div_100[i] == div_100[searchsrc]) {
                    if (div[searchsrc]%2)
                        searchsrc = i;
                    else if (div[searchsrc] > div[i])
                        searchsrc = i;
                }
            }
        }
        if (searchsrc == 0xFFFFFFFF) {
            if (overclksrc == 0xFFFFFFFF) {
                overclksrc = 0;
                for (i=1 ; i<MAX_CLK_SRC ; i++) {
                    if (stClockSource[i] > stClockSource[overclksrc])
                        overclksrc = i;
                }
            }
            searchsrc = overclksrc;
            div[searchsrc] = 1;
        }
    }

    switch(searchsrc) {
        case 0: PCLKCTRL->sel = PCLKCTRL_SEL_PLL0; break;
        case 1: PCLKCTRL->sel = PCLKCTRL_SEL_PLL1; break;
        case 2: PCLKCTRL->sel = PCLKCTRL_SEL_PLL2; break;
        case 3: PCLKCTRL->sel = PCLKCTRL_SEL_PLL3; break;
#if (MAX_TCC_PLL > 4)
        case 4: PCLKCTRL->sel = PCLKCTRL_SEL_PLL4; break;
        case 5: PCLKCTRL->sel = PCLKCTRL_SEL_PLL5; break;
        case 6: PCLKCTRL->sel = PCLKCTRL_SEL_PLL0DIV; break;
        case 7: PCLKCTRL->sel = PCLKCTRL_SEL_PLL1DIV; break;
        case 8: PCLKCTRL->sel = PCLKCTRL_SEL_PLL2DIV; break;
        case 9: PCLKCTRL->sel = PCLKCTRL_SEL_PLL3DIV; break;
        case 10: PCLKCTRL->sel = PCLKCTRL_SEL_PLL4DIV; break;
        case 11: PCLKCTRL->sel = PCLKCTRL_SEL_PLL5DIV; break;
        case 12: PCLKCTRL->sel = PCLKCTRL_SEL_XIN; break;
#else
        case 4: PCLKCTRL->sel = PCLKCTRL_SEL_PLL0DIV; break;
        case 5: PCLKCTRL->sel = PCLKCTRL_SEL_PLL1DIV; break;
        case 6: PCLKCTRL->sel = PCLKCTRL_SEL_PLL2DIV; break;
        case 7: PCLKCTRL->sel = PCLKCTRL_SEL_PLL3DIV; break;
        case 8: PCLKCTRL->sel = PCLKCTRL_SEL_XIN; break;
#endif
        default: return -1;
    }

    if (PCLKCTRL->md == PCLKCTRL_MODE_DCO) {
        PCLKCTRL->div = div[searchsrc];
        if (PCLKCTRL->div > div_max/2)
            PCLKCTRL->freq = ((stClockSource[searchsrc]>>dco_shift)*(div_max-PCLKCTRL->div))/(div_max>>dco_shift);
        else
            PCLKCTRL->freq = ((stClockSource[searchsrc]>>dco_shift)*PCLKCTRL->div)/(div_max>>dco_shift);

        if (PCLKCTRL->div < div_min || PCLKCTRL->div > div_max)
            return -1;
    }
    else { /* Divider mode */
        PCLKCTRL->div = div[searchsrc];
        PCLKCTRL->freq = stClockSource[searchsrc]/PCLKCTRL->div;
        if (PCLKCTRL->div >= div_min && PCLKCTRL->div <= div_max)
            PCLKCTRL->div -= 1;
        else
            return -1;
    }
    return 0;
}

static inline tPCLKTYPE tcc_check_pclk_type(unsigned int periname)
{
    if (periname == PERI_HDMIA || periname == PERI_ADAI1 || periname == PERI_ADAM1 || periname == PERI_SPDIF1 ||
        periname == PERI_ADAI0 || periname == PERI_ADAM0 || periname == PERI_SPDIF0 || periname == PERI_ADC)
        return PCLKCTRL_TYPE_YYY;
    else
        return PCLKCTRL_TYPE_XXX;
}

unsigned int tca_ckc_setperi(unsigned int periname,unsigned int isenable, unsigned int freq)  /* freq(100Hz) */
{
    volatile unsigned   *PCLKCTRL = (volatile unsigned *)REG_PCLKCTRL+periname;
    tPCLKCTRL           nPCLKCTRL;
    tPCLKTYPE           type = tcc_check_pclk_type(periname);

    if (periname == PERI_HDMI)
        PCLKCTRL = &stHDMIReg;
    else if (periname == PERI_HDMIA)
        PCLKCTRL = &stHDMIAudReg;

    nPCLKCTRL.freq = freq;
    nPCLKCTRL.periname = periname;
    nPCLKCTRL.div = 0;
    nPCLKCTRL.md = PCLKCTRL_MODE_DCO;
    nPCLKCTRL.sel = PCLKCTRL_SEL_XIN;

    switch (type) {
        case PCLKCTRL_TYPE_YYY:
            if (nPCLKCTRL.periname == PERI_ADC) {
                nPCLKCTRL.md = PCLKCTRL_MODE_DIVIDER;
                nPCLKCTRL.sel = PCLKCTRL_SEL_XIN;
                nPCLKCTRL.div = (XIN_CLK_RATE+nPCLKCTRL.freq-1)/nPCLKCTRL.freq;
                nPCLKCTRL.freq= XIN_CLK_RATE/nPCLKCTRL.div;
                if (nPCLKCTRL.div > PCLKCTRL_DIV_MIN && nPCLKCTRL.div <= (PCLKCTRL_DIV_YYY_MAX+1))
                    nPCLKCTRL.div -= 1;
                else 
                    goto tca_ckc_setperi_failed;
            }
            else
            if (tcc_find_pclk(&nPCLKCTRL, type))
                goto tca_ckc_setperi_failed;
            break;

        case PCLKCTRL_TYPE_XXX:
            if (nPCLKCTRL.freq == XTIN_CLK_RATE) {
                if ((nPCLKCTRL.periname == PERI_LCD0 || nPCLKCTRL.periname == PERI_LCD1)) {
                    nPCLKCTRL.sel = PCLKCTRL_SEL_HDMIPCLK;
                    nPCLKCTRL.div = 0;
                }
                else {
                    nPCLKCTRL.sel = PCLKCTRL_SEL_XTIN;
                    nPCLKCTRL.div = 0;
                }
            }
#if !defined(CONFIG_HDMI_CLK_USE_INTERNAL_PLL)
            else if (nPCLKCTRL.periname == PERI_HDMI && nPCLKCTRL.freq == HDMI_CLK_RATE) {
                #if defined(CONFIG_HDMI_CLK_USE_XIN_24MHZ)
                nPCLKCTRL.sel = PCLKCTRL_SEL_XIN;
                nPCLKCTRL.div = 0;
                nPCLKCTRL.freq = XIN_CLK_RATE;
                #else
                nPCLKCTRL.sel = PCLKCTRL_SEL_HDMIXIN;
                nPCLKCTRL.div = 0;
                nPCLKCTRL.freq = HDMI_CLK_RATE;
                #endif /* CONFIG_HDMI_CLK_USE_XIN_24MHZ */
            }
#endif
            else if (nPCLKCTRL.freq == 120000 || nPCLKCTRL.freq == 60000 || nPCLKCTRL.freq == 40000
                  || nPCLKCTRL.freq == 30000 || nPCLKCTRL.freq <= 20000) {
                nPCLKCTRL.sel = PCLKCTRL_SEL_XIN;
                nPCLKCTRL.div = (XIN_CLK_RATE+nPCLKCTRL.freq-1)/nPCLKCTRL.freq;
                nPCLKCTRL.freq = XIN_CLK_RATE/nPCLKCTRL.div;
                if (nPCLKCTRL.div > PCLKCTRL_DIV_MIN && nPCLKCTRL.div <= (PCLKCTRL_DIV_XXX_MAX+1))
                    nPCLKCTRL.div -= 1;
                else 
                    goto tca_ckc_setperi_failed;
            }
            else {
                if (tcc_find_pclk(&nPCLKCTRL, type))
                    goto tca_ckc_setperi_failed;
            }
            break;
        default:
            goto tca_ckc_setperi_failed;
    }

    /* enable bit */
    {
        switch (isenable) {
            case CKC_DISABLE:
                nPCLKCTRL.en = 0;
                break;
            case CKC_ENABLE:
                nPCLKCTRL.en = 1;
                break;
            default:
                nPCLKCTRL.en = ((*(volatile unsigned *)PCLKCTRL) & (1<<PCLKCTRL_EN_SHIFT)) ? 1 : 0;
                break;
        }
    }

    tcc_pclkctrl_write(PCLKCTRL, nPCLKCTRL.md, nPCLKCTRL.en, nPCLKCTRL.sel, nPCLKCTRL.div, type);

    if (periname == PERI_HDMI || periname == PERI_HDMIA)
        *(volatile unsigned *)(REG_PCLKCTRL+periname*4) = *PCLKCTRL;
    
    return nPCLKCTRL.freq;

tca_ckc_setperi_failed:
    tcc_pclkctrl_write(PCLKCTRL, PCLKCTRL_MODE_DIVIDER, CKC_DISABLE, PCLKCTRL_SEL_XIN, 1, type);
    if (periname == PERI_HDMI || periname == PERI_HDMIA)
        *(volatile unsigned *)(REG_PCLKCTRL+periname*4) = *PCLKCTRL;
    return 0;
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getperi(unsigned int periname)
* DESCRIPTION :
* ***************************************************************************************/
unsigned int tca_ckc_getperi(unsigned int periname)
{
    volatile unsigned   *pPCLKCTRL = (volatile unsigned *)REG_PCLKCTRL+periname;
    tPCLKCTRL           nPCLKCTRL;
    tPCLKTYPE           type = tcc_check_pclk_type(periname);
    unsigned int        src_freq = 0, div_mask;

    if (periname == PERI_HDMI)
        pPCLKCTRL = &stHDMIReg;
    else if (periname == PERI_HDMIA)
        pPCLKCTRL = &stHDMIAudReg;

    nPCLKCTRL.en = (*(volatile unsigned *)pPCLKCTRL&(PCLKCTRL_EN_MASK<<PCLKCTRL_EN_SHIFT)) ? 1 : 0;
    if (nPCLKCTRL.en == 0)
        return 0;

    nPCLKCTRL.sel = (*(volatile unsigned *)pPCLKCTRL&(PCLKCTRL_SEL_MASK<<PCLKCTRL_SEL_SHIFT))>>PCLKCTRL_SEL_SHIFT;
    switch(nPCLKCTRL.sel) {
        case PCLKCTRL_SEL_PLL0 :
            src_freq =  tca_ckc_getpll(PLL_0);
            break;
        case PCLKCTRL_SEL_PLL1 :
            src_freq =  tca_ckc_getpll(PLL_1);
            break;
        case PCLKCTRL_SEL_PLL2 :
            src_freq =  tca_ckc_getpll(PLL_2);
            break;
        case PCLKCTRL_SEL_PLL3 :
            src_freq =  tca_ckc_getpll(PLL_3);
            break;
        case PCLKCTRL_SEL_XIN :
            src_freq =  XIN_CLK_RATE;
            break;
        case PCLKCTRL_SEL_PLL0DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_0);
            break;
        case PCLKCTRL_SEL_PLL1DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_1);
            break;
        case PCLKCTRL_SEL_PLL2DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_2);
            break;
        case PCLKCTRL_SEL_PLL3DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_3);
            break;
        case PCLKCTRL_SEL_XTIN:
            src_freq =  XTIN_CLK_RATE;
            break;
        /*
        case PCLKCTRL_SEL_HDMITMDS:
            src_freq =  ;
            break;
        case PCLKCTRL_SEL_HDMIPCLK:
            src_freq =  ;
            break;
        */
        case PCLKCTRL_SEL_HDMIXIN:
            src_freq =  HDMI_CLK_RATE;
            break;
        /*
        case PCLKCTRL_SEL_XINDIV:
            src_freq =  60000;
            break;
        case PCLKCTRL_SEL_XTINDIV:
            src_freq =  163;
            break;
        */
#if (MAX_TCC_PLL > 4)
        case PCLKCTRL_SEL_PLL4:
            src_freq =  tca_ckc_getpll(PLL_4);
            break;
        case PCLKCTRL_SEL_PLL5:
            src_freq =  tca_ckc_getpll(PLL_5);
            break;
        case PCLKCTRL_SEL_PLL4DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_4);
            break;
        case PCLKCTRL_SEL_PLL5DIV:
            src_freq =  tcc_ckc_getplldivder(PLL_5);
            break;
#endif
        default :
            return 0;
    }

    switch (type) {
        case PCLKCTRL_TYPE_XXX:
            div_mask = PCLKCTRL_DIV_XXX_MASK;
            nPCLKCTRL.md = PCLKCTRL_MODE_DIVIDER;
            break;
        case PCLKCTRL_TYPE_YYY:
            div_mask = PCLKCTRL_DIV_YYY_MASK;
            break;
        default:
            return 0;
    }
    nPCLKCTRL.freq = 0;
    nPCLKCTRL.div = (*(volatile unsigned *)pPCLKCTRL&(div_mask<<PCLKCTRL_DIV_SHIFT))>>PCLKCTRL_DIV_SHIFT;
    if (nPCLKCTRL.md == PCLKCTRL_MODE_DIVIDER)
       nPCLKCTRL.freq = src_freq/(nPCLKCTRL.div+1);
    else {
        if (nPCLKCTRL.div > (div_mask+1)/2)
            nPCLKCTRL.freq = (src_freq*((div_mask+1) - nPCLKCTRL.div)) / (div_mask+1);
        else
            nPCLKCTRL.freq = (src_freq*nPCLKCTRL.div) / (div_mask+1);
    }
    return nPCLKCTRL.freq;
}

int tca_ckc_pclk_enable(unsigned int periname, unsigned int enable)
{
    volatile unsigned   *PCLKCTRL = (volatile unsigned *)REG_PCLKCTRL+periname;

    if (periname == PERI_HDMI)
        PCLKCTRL = &stHDMIReg;
    else if (periname == PERI_HDMIA)
        PCLKCTRL = &stHDMIAudReg;

    if (enable)
        *(volatile unsigned *)PCLKCTRL = *(volatile unsigned *)PCLKCTRL | (1<<PCLKCTRL_EN_SHIFT);
    else
        *(volatile unsigned *)PCLKCTRL = *(volatile unsigned *)PCLKCTRL & ~(1<<PCLKCTRL_EN_SHIFT);

    if (periname == PERI_HDMI || periname == PERI_HDMIA)
        *(volatile unsigned *)(REG_PCLKCTRL+periname*4) = *PCLKCTRL;

    return 0;
}


/****************************************************************************************
* FUNCTION :void tca_ckc_setpmuippwdn( unsigned int sel , unsigned int ispwdn)
* DESCRIPTION :
*   - fbusname : IP Isolation index
*   - ispwdn : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_setippwdn( unsigned int sel, unsigned int ispwdn)
{
    switch(sel) {
//      case PMU_ISOL_OTP:    /* Controlled by PM */
//      case PMU_ISOL_RTC:    /* Controlled by PM */
//      case PMU_ISOL_PLL:    /* Controlled by PM */
//      case PMU_ISOL_ECID:   /* Controlled by PM */
        case PMU_ISOL_HDMI:
        case PMU_ISOL_VDAC:
        case PMU_ISOL_TSADC:
        case PMU_ISOL_USBHP:
        case PMU_ISOL_USBOP:
        case PMU_ISOL_LVDS:
            if (ispwdn)
                stIPPWDNReg |= (1<<sel);
            else
                stIPPWDNReg &= ~(1<<sel);

            pPMU->PMU_ISOL.nREG = stIPPWDNReg;
            break;
        default:
            return -1;
    }
    return 0;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setfbuspwdn( unsigned int fbusname , unsigned int ispwdn)
* DESCRIPTION :
*   - fbusname : CLKCTRL(n) index
*   - ispwdn : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_setfbuspwdn( unsigned int fbusname, unsigned int ispwdn)
{
    while(pPMU->PMU_PWRSTS.bREG.MAIN_STATE);

    switch (fbusname) {
        case FBUS_MEM:
            if (ispwdn) {
                do {
                    pPMU->PWRDN_MBUS.bREG.DATA = 1;
                } while (pPMU->PMU_PWRSTS.bREG.PD_MB == 0);
            }
            else {
                do {
                    pPMU->PWRUP_MBUS.bREG.DATA = 1;
                } while (pPMU->PMU_PWRSTS.bREG.PU_MB == 0);
            }
            break;
        case FBUS_DDI:
            if (ispwdn) {
                do {
                    pMEMBUSCFG->HCLKMASK.bREG.DBUS = 0;
                } while ((pMEMBUSCFG->MBUSSTS.bREG.DBUS0 | pMEMBUSCFG->MBUSSTS.bREG.DBUS1) == 1);

                pPMU->PMU_SYSRST.bREG.DB = 0;
                do {
                    pPMU->PWRDN_DBUS.bREG.DATA = 1;
                } while (pPMU->PMU_PWRSTS.bREG.PD_DB == 0);
            }
            else {
                do {
                    pPMU->PWRUP_DBUS.bREG.DATA = 1;
                } while (pPMU->PMU_PWRSTS.bREG.PU_DB == 0);
                pPMU->PMU_SYSRST.bREG.DB = 1;

                do {
                    pMEMBUSCFG->HCLKMASK.bREG.DBUS = 1;
                } while ((pMEMBUSCFG->MBUSSTS.bREG.DBUS0 & pMEMBUSCFG->MBUSSTS.bREG.DBUS1) == 0);
            }
            break;
        case FBUS_GPU:
            if (ispwdn) {
                do {
                    pMEMBUSCFG->HCLKMASK.bREG.GBUS = 0;
                } while (pMEMBUSCFG->MBUSSTS.bREG.GBUS == 1);

                pPMU->PMU_SYSRST.bREG.GB = 0;
                do {
                    pPMU->PWRDN_GBUS.bREG.DATA = 1;
                } while (pPMU->PMU_PWRSTS.bREG.PD_GB == 0);
            }
            else {
                do {
                    pPMU->PWRUP_GBUS.bREG.DATA = 1;
                } while (pPMU->PMU_PWRSTS.bREG.PU_GB == 0);
                pPMU->PMU_SYSRST.bREG.GB = 1;

                do {
                    pMEMBUSCFG->HCLKMASK.bREG.GBUS = 1;
                } while (pMEMBUSCFG->MBUSSTS.bREG.GBUS == 0);
            }
            break;
        case FBUS_VBUS:
            if (ispwdn) {
                do {pMEMBUSCFG->HCLKMASK.bREG.VBUS = 0;
                } while ((pMEMBUSCFG->MBUSSTS.bREG.VBUS0 | pMEMBUSCFG->MBUSSTS.bREG.VBUS1) == 1);

                pPMU->PMU_SYSRST.bREG.VB = 0;
                do {
                    pPMU->PWRDN_VBUS.bREG.DATA = 1;
                } while (pPMU->PMU_PWRSTS.bREG.PD_VB == 0);
            }
            else {
                do {
                    pPMU->PWRUP_VBUS.bREG.DATA = 1;
                } while (pPMU->PMU_PWRSTS.bREG.PU_VB == 0);
                pPMU->PMU_SYSRST.bREG.VB = 1;

                do {
                    pMEMBUSCFG->HCLKMASK.bREG.VBUS = 1;
                } while ((pMEMBUSCFG->MBUSSTS.bREG.VBUS0 & pMEMBUSCFG->MBUSSTS.bREG.VBUS1) == 0);
            }
            break;
        case FBUS_HSIO:
            if (ispwdn) {
                do {
                    pMEMBUSCFG->HCLKMASK.bREG.HSIOBUS = 0;
                } while (pMEMBUSCFG->MBUSSTS.bREG.HSIOBUS == 1);

                pPMU->PMU_SYSRST.bREG.HSB = 0;
                do {
                    pPMU->PWRDN_HSBUS.bREG.DATA = 1;
                } while (pPMU->PMU_PWRSTS.bREG.PD_HSB == 0);
            }
            else {
                do {
                    pPMU->PWRUP_HSBUS.bREG.DATA = 1;
                } while (pPMU->PMU_PWRSTS.bREG.PU_HSB == 0);
                pPMU->PMU_SYSRST.bREG.HSB = 1;

                do {
                    pMEMBUSCFG->HCLKMASK.bREG.HSIOBUS = 1;
                } while (pMEMBUSCFG->MBUSSTS.bREG.HSIOBUS == 0);
            }
            break;
        default:
            return -1;
    }

    return 0;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_getfbuspwdn( unsigned int fbusname)
* DESCRIPTION :
*   - fbusname : CLKCTRL(n) index
*   - return : 1:pwdn, 0:wkup
* ***************************************************************************************/
int tca_ckc_getfbuspwdn( unsigned int fbusname)
{
    int retVal = 0;

    switch (fbusname) {
        case FBUS_MEM:
            if (pPMU->PMU_PWRSTS.bREG.PU_MB)
                retVal = 1;
            break;
        case FBUS_DDI:
            if (pPMU->PMU_PWRSTS.bREG.PU_DB)
                retVal = 1;
            break;
        case FBUS_GPU:
            if (pPMU->PMU_PWRSTS.bREG.PU_GB)
                retVal = 1;
            break;
//        case FBUS_VCORE:
        case FBUS_VBUS:
            if (pPMU->PMU_PWRSTS.bREG.PU_VB)
                retVal = 1;
            break;
        case FBUS_HSIO:
            if (pPMU->PMU_PWRSTS.bREG.PU_HSB)
                retVal = 1;
            break;
        default :
            retVal = -1;
            break;
    }

     return retVal;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setswreset(unsigned int fbus_name, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setfbusswreset(unsigned int fbusname, unsigned int isreset)
{
    unsigned long old_value, ctrl_value;

    return 0;

    old_value = (0x1FF | ~(pCKC->SWRESET.nREG));

    switch (fbusname) {
        case FBUS_DDI:
            ctrl_value = 0x00000004;
            break;
        case FBUS_GPU:
            ctrl_value = 0x00000008;
            break;
        case FBUS_VBUS:
//        case FBUS_VCORE:  
            ctrl_value = 0x00000060;
            break;
        case FBUS_HSIO:
            ctrl_value = 0x00000080;
            break;
        default:
            return -1;
    }

    if (isreset)
        pCKC->SWRESET.nREG = old_value & ~ctrl_value;
    else
        pCKC->SWRESET.nREG = old_value | ctrl_value;

    return 0;
}

/****************************************************************************************
* FUNCTION :  int tca_ckc_setiopwdn(unsigned int sel, unsigned int ispwdn)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setiopwdn(unsigned int sel, unsigned int ispwdn)
{
    if (sel >= RB_MAX)
        return -1;

    if (pCKC->CLKCTRL4.bREG.EN == 0)
        return -2;

    if (sel >= 32) {
        if (ispwdn)
            pIOBUSCFG->HCLKEN1.nREG &= ~(0x1 << (sel-32));
        else
            pIOBUSCFG->HCLKEN1.nREG |= (0x1 << (sel-32));
    }
    else {
        if (ispwdn)
            pIOBUSCFG->HCLKEN0.nREG &= ~(0x1 << sel);
        else
            pIOBUSCFG->HCLKEN0.nREG |= (0x1 << sel);
    }

    return 0;
}

/****************************************************************************************
* FUNCTION :  int tca_ckc_getiobus(unsigned int sel)
* DESCRIPTION :
*   - return : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_getiopwdn(unsigned int sel)
{
    int retVal = 0;

    if (sel >= RB_MAX)
        return -1;

    if (pCKC->CLKCTRL4.bREG.EN == 0)
        return -2;

    if (sel >= 32) {
        if (pIOBUSCFG->HCLKEN1.nREG & (0x1 << (sel-32)))
            retVal = 0;
        else
            retVal = 1;
    }
    else {
        if (pIOBUSCFG->HCLKEN0.nREG & (0x1 << sel))
            retVal = 0;
        else
            retVal = 1;
    }

    return retVal;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_setioswreset(unsigned int sel, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setioswreset(unsigned int sel, unsigned int isreset)
{
    if (sel >= RB_MAX)
        return -1;

    if (pCKC->CLKCTRL4.bREG.EN == 0)
        return -2;

    if (sel >= 32) {
        if (isreset)
            pIOBUSCFG->HRSTEN1.nREG &= ~(0x1 << (sel-32));
        else
            pIOBUSCFG->HRSTEN1.nREG |= (0x1 << (sel-32));
    }
    else {
        if (isreset)
            pIOBUSCFG->HRSTEN0.nREG &= ~(0x1 << sel);
        else
            pIOBUSCFG->HRSTEN0.nREG |= (0x1 << sel);
    }

    return 0;
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setddipwdn(unsigned int sel , unsigned int ispwdn)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setddipwdn(unsigned int sel , unsigned int ispwdn)
{
    if (sel >= DDIBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL2.bREG.EN == 0)
        return -2;

    if (ispwdn)
        pDDIBUSCFG->PWDN.nREG &= ~(0x1 << sel);
    else
        pDDIBUSCFG->PWDN.nREG |= (0x1 << sel);

    return 0;
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getddipwdn(unsigned int sel)
* DESCRIPTION :
*   - return : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_getddipwdn(unsigned int sel)
{
    int retVal = 0;

    if (sel >= DDIBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL2.bREG.EN == 0)
        return -2;

    if (pDDIBUSCFG->PWDN.nREG & (0x1 << sel))
        retVal = 0;
    else
        retVal = 1;

    return retVal;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_setddiswreset(unsigned int sel, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setddiswreset(unsigned int sel, unsigned int isreset)
{
    if (sel >= DDIBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL2.bREG.EN == 0)
        return -2;

    if (isreset)
        pDDIBUSCFG->SWRESET.nREG &= ~(0x1 << sel);
    else
        pDDIBUSCFG->SWRESET.nREG |= (0x1 << sel);

    return 0;
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setgpupwdn(unsigned int sel , unsigned int ispwdn)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setgpupwdn(unsigned int sel , unsigned int ispwdn)
{
    if (sel >= GPUBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL3.bREG.EN == 0)
        return -2;

    if (ispwdn)
        pGPUBUSCFG->PWDN.nREG &= ~(0x1 << sel);
    else
        pGPUBUSCFG->PWDN.nREG |= (0x1 << sel);

    return 0;
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getgpupwdn(unsigned int sel)
* DESCRIPTION :
*   - return : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_getgpupwdn(unsigned int sel)
{
    int retVal = 0;

    if (sel >= GPUBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL3.bREG.EN == 0)
        return -2;

    if (pGPUBUSCFG->PWDN.nREG & (0x1 << sel))
        retVal = 0;
    else
        retVal = 1;

    return retVal;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_setgpuswreset(unsigned int sel, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setgpuswreset(unsigned int sel, unsigned int isreset)
{
    if (sel >= GPUBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL3.bREG.EN == 0)
        return -2;

    if (isreset)
        pGPUBUSCFG->SWRESET.nREG &= ~(0x1 << sel);
    else
        pGPUBUSCFG->SWRESET.nREG |= (0x1 << sel);

    return 0;
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setvideopwdn(unsigned int sel , unsigned int ispwdn)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setvideopwdn(unsigned int sel , unsigned int ispwdn)
{
    if (sel >= VIDEOBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL5.bREG.EN == 0)
        return -2;

    if (ispwdn)
        pVIDEOBUSCFG->PWDN.nREG &= ~(0x1 << sel);
    else
        pVIDEOBUSCFG->PWDN.nREG |= (0x1 << sel);
    
    return 0;
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getvideopwdn(unsigned int sel)
* DESCRIPTION :
*   - return : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_getvideopwdn(unsigned int sel)
{
    int retVal = 0;

    if (sel >= VIDEOBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL5.bREG.EN == 0)
        return -2;

    if (pVIDEOBUSCFG->PWDN.nREG & (0x1 << sel))
        retVal = 0;
    else
        retVal = 1;

    return retVal;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_setvideoswreset(unsigned int sel, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setvideoswreset(unsigned int sel, unsigned int isreset)
{
    if (sel >= VIDEOBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL5.bREG.EN == 0)
        return -2;

    if (isreset)
        pVIDEOBUSCFG->SWRESET.nREG &= ~(0x1 << sel);
    else
        pVIDEOBUSCFG->SWRESET.nREG |= (0x1 << sel);

    return 0;
}

/****************************************************************************************
* FUNCTION : void tca_ckc_sethsiopwdn(unsigned int sel , unsigned int ispwdn)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_sethsiopwdn(unsigned int sel , unsigned int ispwdn)
{
    if (sel >= HSIOBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL7.bREG.EN == 0)
        return -2;

    if (ispwdn)
        pHSIOBUSCFG->PWDN.nREG &= ~(0x1 << sel);
    else
        pHSIOBUSCFG->PWDN.nREG |= (0x1 << sel);

    return 0;
}

/****************************************************************************************
* FUNCTION : int tca_ckc_gethsiopwdn(unsigned int sel)
* DESCRIPTION : Power Down Register of DDI_CONFIG
*   - return : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_gethsiopwdn(unsigned int sel)
{
    int retVal = 0;

    if (sel >= HSIOBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL7.bREG.EN == 0)
        return -2;

    if (pHSIOBUSCFG->PWDN.nREG & (0x1 << sel))
        retVal = 0;
    else
        retVal = 1;

    return retVal;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_sethsioswreset(unsigned int sel, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_sethsioswreset(unsigned int sel, unsigned int isreset)
{
    if (sel >= HSIOBUS_MAX)
        return -1;

    if (pCKC->CLKCTRL7.bREG.EN == 0)
        return -2;

    if (isreset)
        pHSIOBUSCFG->SWRESET.nREG &= ~(0x1 << sel);
    else
        pHSIOBUSCFG->SWRESET.nREG |= (0x1 << sel);

    return 0;
}

int tca_ckc_fclk_enable(unsigned int fclk, unsigned int enable)
{
    volatile CLKCTRL_TYPE *pCLKCTRL;
    pCLKCTRL = (volatile CLKCTRL_TYPE *)((&pCKC->CLKCTRL0)+fclk);

    if (enable)
        pCLKCTRL->bREG.EN = 1;
    else
        pCLKCTRL->bREG.EN = 0;
    while(pCLKCTRL->bREG.CFGREQ);

    return 0;
}

int tca_ckc_fclk_is_enabled(unsigned int fclk)
{
    volatile CLKCTRL_TYPE *pCLKCTRL;
    pCLKCTRL = (volatile CLKCTRL_TYPE *)((&pCKC->CLKCTRL0)+fclk);

    if (pCLKCTRL->bREG.EN)
        return 1;

    return 0;
}

/****************************************************************************************
* EXPORT_SYMBOL clock functions for Linux
* ***************************************************************************************/
#if defined(_LINUX_)

EXPORT_SYMBOL(tca_ckc_init);
EXPORT_SYMBOL(tca_ckc_setpll);
EXPORT_SYMBOL(tca_ckc_getpll);
EXPORT_SYMBOL(tca_ckc_setfbusctrl);
EXPORT_SYMBOL(tca_ckc_getfbusctrl);
EXPORT_SYMBOL(tca_ckc_setperi);
EXPORT_SYMBOL(tca_ckc_getperi);
EXPORT_SYMBOL(tca_ckc_setippwdn);
EXPORT_SYMBOL(tca_ckc_setfbuspwdn);
EXPORT_SYMBOL(tca_ckc_getfbuspwdn);
EXPORT_SYMBOL(tca_ckc_setfbusswreset);
EXPORT_SYMBOL(tca_ckc_setiopwdn);
EXPORT_SYMBOL(tca_ckc_getiopwdn);
EXPORT_SYMBOL(tca_ckc_setioswreset);
EXPORT_SYMBOL(tca_ckc_setddipwdn);
EXPORT_SYMBOL(tca_ckc_getddipwdn);
EXPORT_SYMBOL(tca_ckc_setddiswreset);
EXPORT_SYMBOL(tca_ckc_setgpupwdn);
EXPORT_SYMBOL(tca_ckc_getgpupwdn);
EXPORT_SYMBOL(tca_ckc_setgpuswreset);
EXPORT_SYMBOL(tca_ckc_setvideopwdn);
EXPORT_SYMBOL(tca_ckc_getvideopwdn);
EXPORT_SYMBOL(tca_ckc_setvideoswreset);
EXPORT_SYMBOL(tca_ckc_sethsiopwdn);
EXPORT_SYMBOL(tca_ckc_gethsiopwdn);
EXPORT_SYMBOL(tca_ckc_sethsioswreset);
EXPORT_SYMBOL(tca_ckc_fclk_enable);
EXPORT_SYMBOL(tca_ckc_fclk_is_enabled);
EXPORT_SYMBOL(tca_ckc_pclk_enable);

#endif

/* end of file */
