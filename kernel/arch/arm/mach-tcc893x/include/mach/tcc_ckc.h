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


#ifndef _PLATFORM_TCC_CKC_H_
#define _PLATFORM_TCC_CKC_H_


//#define XIN_CLK_RATE    120000    // 12MHz
#define XIN_CLK_RATE    240000  // 24MHz
#define XTIN_CLK_RATE   327     // 32.768kHz
#define HDMI_CLK_RATE   270000

#define CKC_DISABLE     0
#define CKC_ENABLE      1
#define CKC_NOCHANGE    2


/* PLL channel index */
enum {
    PLL_0=0,
    PLL_1,
    PLL_2,
    PLL_3,
    PLL_4,
    PLL_5,
};

/* PLL Clock Source */
enum{
    PLLSRC_XIN=0,
    PLLSRC_HDMIXI,
    PLLSRC_EXTCLK0,
    PLLSRC_EXTCLK1,
    PLLSRC_MAX
};

/* CLKCTRL channel index */
enum {
    FBUS_CPU = 0,       // CLKCTRL0
    FBUS_MEM,           // CLKCTRL1
    FBUS_DDI,           // CLKCTRL2
    FBUS_GPU,           // CLKCTRL3
    FBUS_IO,            // CLKCTRL4
    FBUS_VBUS,          // CLKCTRL5
    FBUS_VCORE,         // CLKCTRL6
    FBUS_HSIO,          // CLKCTRL7
    FBUS_SMU,           // CLKCTRL8
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
    FBUS_G2D,           // CLKCTRL9
    FBUS_CM3,           // CLKCTRL10
#endif
};

/* Peripheral Clocks */
enum {/* Peri. Name */
    PERI_TCX = 0,       // 0    // 0x80
    PERI_TCT,
    PERI_TCZ,
    PERI_LCD0,
    PERI_LCDSI0,
    PERI_LCD1,          // 5
    PERI_LCDSI1,
    PERI_RESERVED07,
    PERI_LCDTIMER,
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    PERI_JPEG,
#else
    PERI_RESERVED09,
#endif
    PERI_RESERVED10,    // 10
    PERI_RESERVED11,
    PERI_GMAC,
    PERI_USBOTG,
    PERI_RESERVED14,
    PERI_OUT0,          // 15
    PERI_USB20H,
    PERI_HDMI,
    PERI_HDMIA,
    PERI_OUT1,
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    PERI_EHI,
#else
    PERI_REMOTE,        // 20
#endif
    PERI_SDMMC0,
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    PERI_RESERVED22,
#else
    PERI_SDMMC1,
#endif
    PERI_SDMMC2,
    PERI_SDMMC3,
    PERI_ADAI1,         // 25
    PERI_ADAM1,
    PERI_SPDIF1,
    PERI_ADAI0,
    PERI_ADAM0,
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    PERI_RESERVED30,
#else
    PERI_SPDIF0,        // 30
#endif
    PERI_PDM,
    PERI_RESERVED32,
    PERI_ADC,
    PERI_I2C0,
    PERI_I2C1,          // 35
    PERI_I2C2,
    PERI_I2C3,
    PERI_UART0,
    PERI_UART1,
    PERI_UART2,         // 40
    PERI_UART3,
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    PERI_RESERVED42,
    PERI_RESERVED43,
    PERI_RESERVED44,
    PERI_RESERVED45,
#else
    PERI_UART4,
    PERI_UART5,
    PERI_UART6,
    PERI_UART7,         // 45
#endif
    PERI_GPSB0,
    PERI_GPSB1,
    PERI_GPSB2,
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
    PERI_GPSB3,
    PERI_GPSB4,         // 50
    PERI_GPSB5,
    PERI_CAN0,
    PERI_CAN1,
    PERI_OUT2,
    PERI_OUT3,          // 55
    PERI_OUT4,
    PERI_OUT5,
    PERI_TSTX0,
    PERI_TSTX1,
    PERI_PKTGEN0,       // 60
    PERI_PKTGEN1,
    PERI_PKTGEN2,
    PERI_PKTGEN3,
#endif
    PERI_MAX,
};

/* I/O Bus pwdn/swreset */
enum {
    RB_EHI=0,                   //  0
    RB_MPEFEC,                  //  1
    RB_SDMMC0CONTROLLER,        //  2
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    RB_RESERVED03,
#else
    RB_SDMMC1CONTROLLER,        //  3
#endif
    RB_SDMMC2CONTROLLER,        //  4
    RB_SDMMC3CONTROLLER,        //  5
    RB_SDMMCCHANNELCONTROLLER,  //  6
    RB_DMA0,                    //  7
    RB_DMA1,                    //  8
    RB_DMA2,                    //  9
    RB_DMACONTROLLER,           // 10
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    RB_OVERLAYMAXER,            // 11
#endif
    RB_PWM=12,                  // 12
    RB_SMC,                     // 13
    RB_RTC=15,                  // 15
    RB_REMOTECONTROLLER,        // 16
    RB_TSADC,                   // 17
    RB_EDICONFIGURATION,        // 18
    RB_EDICONTROLLER,           // 19
    RB_SHOULDBE1,               // 20
    RB_AUDIO0_MCADMA,           // 21
    RB_AUDIO0_MCDAI,            // 22
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    RB_RESERVED23,
#else
    RB_AUDIO0_MCSPDIF,          // 23
#endif
    RB_AUDIO0_CONTROLLER,       // 24
    RB_AUDIO1_ADMA,             // 25
    RB_AUDIO1_DAI,              // 26
    RB_AUDIO1_SPDIF,            // 27
    RB_AUDIO1_CONTROLLER,       // 28
    RB_I2C_MASTER0,             // 29
    RB_I2C_MASTER1,             // 30
    RB_I2C_MASTER2,             // 31
    RB_I2C_MASTER3,             //  0 32
    RB_I2C_SLAVE0,              //  1 33
    RB_I2C_SLAVE1,              //  2 34
    RB_I2C_CONTROLLER,          //  3 35
    RB_UART0,                   //  4 36
    RB_UART1,                   //  5 37
    RB_UART2,                   //  6 38
    RB_UART3,                   //  7 39
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    RB_RESERVED40,
    RB_RESERVED41,
    RB_RESERVED42,
    RB_RESERVED43,
#else
    RB_UART4,                   //  8 40
    RB_UART5,                   //  9 41
    RB_UART6,                   // 10 42
    RB_UART7,                   // 11 43
#endif
    RB_UART_CONTROLLER,         // 12 44
    RB_IDE,                     // 13 45
    RB_NFC,                     // 14 46
    RB_TSIF0,                   // 15 47
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    RB_RESERVED48,
    RB_RESERVED49,
#else
    RB_TSIF1,                   // 16 48
    RB_TSIF2,                   // 17 49
#endif
    RB_TSIF_CONTROLLER,         // 18 50
    RB_GPSB0,                   // 19 51
    RB_GPSB1,                   // 20 52
    RB_GPSB2,                   // 21 53
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    RB_RESERVED54,
    RB_RESERVED55,
    RB_RESERVED56,
#else
    RB_GPSB3,                   // 22 54
    RB_GPSB4,                   // 23 55
    RB_GPSB5,                   // 24 56
#endif
    RB_GPSB_CONTROLLER,         // 25 57
    RB_USB20OTG,                // 26 58
    RB_MAX,
};

/* Display Bus pwdn/swreset */
enum{
    DDIBUS_VIOC = 0,
    DDIBUS_NTSCPAL,
    DDIBUS_HDMI,
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
    DDIBUS_G2D,
#endif
    DDIBUS_MAX,
};

/* Graphic Bus pwdn/swreset */
enum{
    GPUBUS_GRP = 0,
    GPUBUS_MAX,
};

/* Video Bus pwdn/swreset */
enum{
    VIDEOBUS_JENC = 0,
    VIDEOBUS_COD = 2,
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    VIDEOBUS_BUS,
#endif
    VIDEOBUS_MAX,
};

/* High-Speed I/O Bus pwdn/swreset */
enum{
    HSIOBUS_DMAX = 0,
    HSIOBUS_GMAC = 2,
    HSIOBUS_USB20H = 6,
    HSIOBUS_HSIOCFG = 16,
    HSIOBUS_CIPHER = 18,
    HSIOBUS_MAX,
};

/* Cortex-m3 hclk */
enum{
    M3HCLK_CM3 = 0,
    M3HCLK_PERI,
    M3HCLK_CODE,
    M3HCLK_DATA,
    M3HCLK_MBOX,
    M3HCLK_MAX
};

/* Cortex-m3 reset */
enum{
    M3RESET_POR = 1,
    M3RESET_SYS,
    M3RESET_MAX
};

/* IP Isolation Control Register (PMU_ISOL: 0x7440005C) */
enum{                           // bit Name
    PMU_ISOL_OTP = 0,
    PMU_ISOL_RTC,
    PMU_ISOL_PLL,
    PMU_ISOL_ECID,
    PMU_ISOL_HDMI,
    PMU_ISOL_VDAC = 6,
    PMU_ISOL_TSADC,
    PMU_ISOL_USBHP,
    PMU_ISOL_USBOP,
    PMU_ISOL_LVDS = 11,
    PMU_ISOL_MAX
};

typedef struct {
    unsigned int    fpll;
    unsigned int    en;
    unsigned int    p;
    unsigned int    m;
    unsigned int    s;
    unsigned int    vsel;
    unsigned int    src;
} tPMS;

typedef struct {
    unsigned int    freq;
    unsigned int    en;
    unsigned int    config;
    unsigned int    sel;
} tCLKCTRL;

typedef struct {
    unsigned int    periname;
    unsigned int    freq;
    unsigned int    md;
    unsigned int    en;
    unsigned int    sel;
    unsigned int    div;
} tPCLKCTRL;

extern void tca_ckc_init(void);

extern inline int   tcc_find_pms(tPMS *PLL, unsigned int src);
extern unsigned int tca_ckc_setpll(unsigned int fpll, unsigned int ch, unsigned int src);  /* pll(100Hz) */
extern unsigned int tca_ckc_getpll(unsigned int ch);
extern unsigned int tca_ckc_setfbusctrl(unsigned int clkname, unsigned int isenable, unsigned int freq);  /* freq(100Hz) */
extern unsigned int tca_ckc_getfbusctrl(unsigned int clkname);
extern unsigned int tca_ckc_setperi(unsigned int periname,unsigned int isenable, unsigned int freq);  /* freq(100Hz) */
extern unsigned int tca_ckc_getperi(unsigned int periname);

extern int tca_ckc_setippwdn( unsigned int sel, unsigned int ispwdn);
extern int tca_ckc_setfbuspwdn( unsigned int fbusname, unsigned int ispwdn);
extern int tca_ckc_getfbuspwdn( unsigned int fbusname);
extern int tca_ckc_setfbusswreset(unsigned int fbusname, unsigned int isreset);

extern int tca_ckc_setiopwdn(unsigned int sel, unsigned int ispwdn);
extern int tca_ckc_getiopwdn(unsigned int sel);
extern int tca_ckc_setioswreset(unsigned int sel, unsigned int isreset);

extern int tca_ckc_setddipwdn(unsigned int sel , unsigned int ispwdn);
extern int tca_ckc_getddipwdn(unsigned int sel);
extern int tca_ckc_setddiswreset(unsigned int sel, unsigned int isreset);

extern int tca_ckc_setgpupwdn(unsigned int sel , unsigned int ispwdn);
extern int tca_ckc_getgpupwdn(unsigned int sel);
extern int tca_ckc_setgpuswreset(unsigned int sel, unsigned int isreset);

extern int tca_ckc_setvideopwdn(unsigned int sel , unsigned int ispwdn);
extern int tca_ckc_getvideopwdn(unsigned int sel);
extern int tca_ckc_setvideoswreset(unsigned int sel, unsigned int isreset);

extern int tca_ckc_sethsiopwdn(unsigned int sel , unsigned int ispwdn);
extern int tca_ckc_gethsiopwdn(unsigned int sel);
extern int tca_ckc_sethsioswreset(unsigned int sel, unsigned int isreset);

extern int tca_ckc_fclk_enable(unsigned int fclk, unsigned int enable);
extern int tca_ckc_fclk_is_enabled(unsigned int fclk);
extern int tca_ckc_pclk_enable(unsigned int pclk, unsigned int enable);

extern void tcc_ckc_restore_misc_regs(void);

#endif /* _PLATFORM_TCC_CKC_H_ */
