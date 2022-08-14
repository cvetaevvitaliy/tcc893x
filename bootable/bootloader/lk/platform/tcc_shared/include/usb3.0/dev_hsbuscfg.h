
#ifndef	_HSBUSCFG_H_
#define	_HSBUSCFG_H_

///////////////////////////////////////////////////////////////////
//
//	BUS CONTROLLER
//
#define	HSBUS_BCLK_ON		1
#define	HSBUS_BCLK_OFF		0

#define	HSBUS_SRST_ON		0
#define	HSBUS_SRST_OFF		1

typedef	struct	{
    unsigned    DMAX    : 1;    // [00]
    unsigned    GMACG   : 1;    // [01]
    //unsigned    GMAC1   : 1;    // [02]
    //unsigned    GMAC0   : 1;    // [03]
    unsigned    HSB_GMAC1   : 1;    // [02]
    unsigned    HSB_GMAC0   : 1;    // [03]
    unsigned    TSDMUX  : 1;    // [04]
    unsigned    ESW     : 1;    // [05]
    unsigned    U20H    : 1;    // [06]
    unsigned    TSIFG   : 1;    // [07]
    unsigned    TSIF2   : 1;    // [08]
    unsigned    TSIF1   : 1;    // [09]
    unsigned    TSIF0   : 1;    // [10]
    unsigned    GDMA    : 1;    // [11]
    unsigned    SATAG   : 1;    // [12]
    unsigned    SATAH   : 1;    // [13]
    unsigned    SATAD   : 1;    // [14]
    unsigned    MEM     : 1;    // [15]
    unsigned    BMCFG   : 1;    // [16]
    unsigned    HSB_RFX : 1;    // [17]
    unsigned    CIPHER  : 1;    // [18]
	unsigned	     	:13;
} HSIOBUS;

typedef	union {
	unsigned	nReg;
	HSIOBUS	bReg;
} HSIOBUS_U;

typedef	struct	{
      unsigned  SLN_IRAM    : 1;    //PWDATA[00];
      unsigned  PDN_IRAM    : 1;    //PWDATA[01];
      unsigned  SLN_GMAC0   : 1;    //PWDATA[02];
      unsigned  PDN_GMAC0   : 1;    //PWDATA[03];
      unsigned  SLN_GMAC1   : 1;    //PWDATA[04];
      unsigned  PDN_GMAC1   : 1;    //PWDATA[05];
      unsigned  SLN_U20H    : 1;    //PWDATA[06];
      unsigned  PDN_U20H    : 1;    //PWDATA[07];
      unsigned  SLN_RFX     : 1;    //PWDATA[08];
      unsigned  PDN_RFX     : 1;    //PWDATA[09];
      unsigned  SLN_NFC     : 1;    //PWDATA[10];
      unsigned  PDN_NFC     : 1;    //PWDATA[11];
      unsigned  PGEN_DMAX   : 1;    //PWDATA[12];
      unsigned  RET1N_DMAX  : 1;    //PWDATA[13];
      unsigned  PGEN_SATA   : 1;    //PWDATA[14];
      unsigned  RET1N_SATA  : 1;    //PWDATA[15];
      unsigned              : 16;
} HSB_MEM;
typedef	union {
	unsigned	nReg;
	HSB_MEM	bReg;
} HSB_MEM_U;

///////////////////////////////////////////////////////////////////
//
//	Memory Configuration
//
typedef	struct	{
    unsigned        RAM_RDW      : 2;
    unsigned                     :30;
} HSB_IMEMCFG;

typedef	union {
    unsigned		nReg;
    HSB_IMEMCFG bReg;
} HSB_IMEMCFG_U;

///////////////////////////////////////////////////////////////////
//
//	HSBUS Configuration
//
typedef	struct	{
    unsigned        A2X_GDMA     : 2;
    unsigned        A2X_TSIF     : 2;
    unsigned                     : 2;
    unsigned        A2X_U20H     : 2;
    unsigned        A2X_ESW      : 2;
    unsigned        A2X_TSDMUX   : 2;
    unsigned        A2X_GMAC     : 2;
    unsigned        A2X_SATA     : 2;
    unsigned        BWRAP_GDMA   : 1;
    unsigned        BWRAP_TSIF   : 1;
    unsigned                     : 1;
    unsigned        BWRAP_U20H   : 1;
    unsigned        BWRAP_ESW    : 1;
    unsigned        BWRAP_TSDMUX : 1;
    unsigned        BWRAP_GMAC   : 1;
    unsigned        BWRAP_SATA   : 1;
    unsigned                     : 4;
    unsigned        TZPROT_BM2x1 : 1;
    unsigned        TZPROT_BM8x3 : 3;
} HSB_CFG;

typedef	union {
    unsigned		nReg;
    HSB_CFG  	bReg;
} HSB_CFG_U;

///////////////////////////////////////////////////////////////////
//
//	Protection Control
//
typedef	struct	{
    unsigned        TSDMUX_ENC_SEL : 4;
    unsigned        TSDMUX_DEC_SEL : 4;
    unsigned        TSDMUX_ENC_EN  : 1;
    unsigned        TSDMUX_DEC_EN  : 1;
    unsigned                       : 6;
    unsigned        GDMA_ENC_SEL   : 4;
    unsigned        GDMA_DEC_SEL   : 4;
    unsigned        GDMA_ENC_EN    : 1;
    unsigned        GDMA_DEC_EN    : 1;
    unsigned                       : 6;
} HSB_KP;

typedef	union {
    unsigned		nReg;
    HSB_KP  	bReg;
} HSB_KP_U;

///////////////////////////////////////////////////////////////////
//
//	GMAC Configuration
//
#define MAC_REFCLK      0x0
#define MAC_RXCLK       0x1
#define MAC_TXCLK       0x2
#define MAC_PERICLK     0x3 

typedef	struct	{
    unsigned        CLKSEL_RX : 2;
    unsigned                  : 2;
    unsigned        DIV_RX    : 6;
    unsigned                  : 5;
    unsigned        REGSET_RX : 1;
    unsigned        CLKSEL_TX : 2;
    unsigned                  : 2;
    unsigned        DIV_TX    : 6;
    unsigned                  : 5;
    unsigned        REGSET_TX : 1;
} HSB_GMACCFG0;

typedef	union {
    unsigned		    nReg;
    HSB_GMACCFG0  	bReg;
} HSB_GMACCFG0_U;

#define GMII_MODE       0x0
#define RGMII_MODE      0x1
#define SGMII_MODE      0x2
#define TBI_MODE        0x3
#define RMII_MODE       0x4
#define RTBI_MODE       0x5 
#define MII_MODE        0x6 

typedef	struct	{
    unsigned        STS_SPEED : 2;
    unsigned                  :14;
    unsigned        TXCLKOEN  : 1;
    unsigned        SBD_FCTRL : 1;
    unsigned        INFSEL    : 3;
    unsigned                  :10;
    unsigned        CLKEN     : 1;
} HSB_GMACCFG1;

typedef	union {
    unsigned		    nReg;
    HSB_GMACCFG1  	bReg;
} HSB_GMACCFG1_U;

///////////////////////////////////////////////////////////////////
//
//	SATA PHY Control
//
//typedef	struct	{
//    unsigned        CTRL        : 8;     // [07:00]
//    unsigned        PORN        : 1;     // [08   ]
//    unsigned        RSTN_SEL    : 1;     // [09   ]
//    unsigned        CMU_RSTN    : 1;     // [10   ]
//    unsigned        PD          : 2;     // [12:11]
//    unsigned        RATE0       : 1;     // [13   ]
//    unsigned        SOCCLK_DIV1 : 1;     // [14   ]
//    unsigned        SOCCLK_DIV0 : 1;     // [15   ]
//    unsigned        CMU_PD      : 1;     // [16   ]
//    unsigned        FAST_SIM    : 1;     // [17   ]
//    unsigned                    : 2;     // [19:18]
//    unsigned        RATE1       : 1;     // [20   ]
//    unsigned        CTRL_SEL    : 1;     // [21   ]
//    unsigned        PD0_SEL     : 1;     // [22   ]
//    unsigned        PD1_SEL     : 1;     // [23   ]
//    unsigned        RATE_SEL    : 1;     // [24   ]
//    unsigned        DIV_SEL     : 1;     // [25   ]
//    unsigned                    : 6;     // [31:26]
//} HSB_SATAPHY_CTRL;

//typedef	union {
//    unsigned		        nReg;
//    HSB_SATAPHY_CTRL  	bReg;
//} HSB_SATAPHY_CTRL_U;

///////////////////////////////////////////////////////////////////
//
//	Cache Control
//
typedef	struct	{
    //0----------------------------------------
    unsigned        AWCACHES00_SEL : 4; // [03:00]
    unsigned        AWCACHES00     : 4; // [07:04]
    unsigned        ARCACHES00_SEL : 4; // [11:08]
    unsigned        ARCACHES00     : 4; // [15:12]
    unsigned        AWCACHES01_SEL : 4; // [19:16]
    unsigned        AWCACHES01     : 4; // [23:20]
    unsigned        ARCACHES01_SEL : 4; // [27:24]
    unsigned        ARCACHES01     : 4; // [31:28]
    //1----------------------------------------
    unsigned        AWCACHES02_SEL : 4; // [03:00]
    unsigned        AWCACHES02     : 4; // [07:04]
    unsigned        ARCACHES02_SEL : 4; // [11:08]
    unsigned        ARCACHES02     : 4; // [15:12]
    unsigned        AWCACHES03_SEL : 4; // [19:16]
    unsigned        AWCACHES03     : 4; // [23:20]
    unsigned        ARCACHES03_SEL : 4; // [27:24]
    unsigned        ARCACHES03     : 4; // [31:28]
    //2----------------------------------------
    unsigned        AWCACHES04_SEL : 4; // [03:00]
    unsigned        AWCACHES04     : 4; // [07:04]
    unsigned        ARCACHES04_SEL : 4; // [11:08]
    unsigned        ARCACHES04     : 4; // [15:12]
    unsigned        AWCACHES05_SEL : 4; // [19:16]
    unsigned        AWCACHES05     : 4; // [23:20]
    unsigned        ARCACHES05_SEL : 4; // [27:24]
    unsigned        ARCACHES05     : 4; // [31:28]
    //3----------------------------------------
    unsigned        AWCACHES06_SEL : 4; // [03:00]
    unsigned        AWCACHES06     : 4; // [07:04]
    unsigned        ARCACHES06_SEL : 4; // [11:08]
    unsigned        ARCACHES06     : 4; // [15:12]
    unsigned        AWCACHES07_SEL : 4; // [19:16]
    unsigned        AWCACHES07     : 4; // [23:20]
    unsigned        ARCACHES07_SEL : 4; // [27:24]
    unsigned        ARCACHES07     : 4; // [31:28]
    //4----------------------------------------
    unsigned        AWCACHES08_SEL : 4; // [03:00]
    unsigned        AWCACHES08     : 4; // [07:04]
    unsigned        ARCACHES08_SEL : 4; // [11:08]
    unsigned        ARCACHES08     : 4; // [15:12]
    unsigned        AWCACHES09_SEL : 4; // [19:16]
    unsigned        AWCACHES09     : 4; // [23:20]
    unsigned        ARCACHES09_SEL : 4; // [27:24]
    unsigned        ARCACHES09     : 4; // [31:28]
    //5----------------------------------------
    unsigned        AWCACHES10_SEL : 4; // [03:00]
    unsigned        AWCACHES10     : 4; // [07:04]
    unsigned        ARCACHES10_SEL : 4; // [11:08]
    unsigned        ARCACHES10     : 4; // [15:12]
    unsigned        AWCACHES11_SEL : 4; // [19:16]
    unsigned        AWCACHES11     : 4; // [23:20]
    unsigned        ARCACHES11_SEL : 4; // [27:24]
    unsigned        ARCACHES11     : 4; // [31:28]
    //6----------------------------------------
    unsigned        AWCACHES12_SEL : 4; // [03:00]
    unsigned        AWCACHES12     : 4; // [07:04]
    unsigned        ARCACHES12_SEL : 4; // [11:08]
    unsigned        ARCACHES12     : 4; // [15:12]
    unsigned        AWCACHES13_SEL : 4; // [19:16]
    unsigned        AWCACHES13     : 4; // [23:20]
    unsigned        ARCACHES13_SEL : 4; // [27:24]
    unsigned        ARCACHES13     : 4; // [31:28]
    //7----------------------------------------
    unsigned        AWCACHES14_SEL : 4; // [03:00]
    unsigned        AWCACHES14     : 4; // [07:04]
    unsigned        ARCACHES14_SEL : 4; // [11:08]
    unsigned        ARCACHES14     : 4; // [15:12]
    unsigned        reserved       :16; // [31:16]
    //----------------------------------------
} HSB_CACHECTRL;

typedef	union {
    unsigned		    nReg[8];
    HSB_CACHECTRL  	bReg;
} HSB_CACHECTRL_U;

///////////////////////////////////////////////////////////////////
//
//	USB30 PHY CFG
//
typedef	struct	{
    unsigned        MPLL_M         : 7; // CFG0[06:00]
    unsigned        REFCLKDIV      : 1; // CFG0[07:07]
    unsigned        LANE_EXTPCLK   : 1; // CFG0[08:08]
    unsigned        SSC_RANGE      : 3; // CFG0[11:09]
    unsigned        SSC_REFCLK_SEL : 9; // CFG0[20:12]
    unsigned        FSEL           : 6; // CFG0[26:21]
    unsigned        REFCLKSEL      : 2; // CFG0[28:27]
    unsigned        REF_SSP_EN     : 1; // CFG0[29:29]
    unsigned        REF_USE_PAD    : 1; // CFG0[30:30]
    unsigned        SSC_EN         : 1; // CFG0[31:31]
} HSB_USB30PHYCFG0;

typedef	struct	{
    unsigned        TESTBURNIN     : 1; // CFG1[00:00]
    unsigned        pipe_reset_n   : 1; // CFG1[01:01]
    unsigned        PORTRESET      : 1; // CFG1[02:02]
    unsigned        phy_reset      : 1; // CFG1[03:03]
    unsigned        ATERESET       : 1; // CFG1[04:04]
    unsigned        PIPE_PWRDN     : 2; // CFG1[06:05]
    unsigned        OVRD_PIPE_PWRDN: 1; // CFG1[07:07]
    unsigned        test_powerdown_hsp : 1; // CFG1[08:08]
    unsigned        test_powerdown_ssp : 1; // CFG1[09:09]
    unsigned        COMMONONN      : 1; // CFG1[10:10]
    unsigned        ALTCLK_EN      : 1; // CFG1[11:11]
    unsigned        ALTCLK_SEL     : 1; // CFG1[12:12]
    unsigned        MPLL_SSCCLK_EN : 1; // CFG1[13:13]
    unsigned                       :18; // CFG1[31:14]
} HSB_USB30PHYCFG1;

typedef	struct	{
    unsigned        TXVBOOST       : 3; // CFG2[02:00]
    unsigned        TXDEEMPH       : 2; // CFG2[04:03]
    unsigned        OVRD_TXDEEMPH  : 1; // CFG2[05:05]
    unsigned        TXVREFTUNE     : 4; // CFG2[09:06]
    unsigned        TXRISETUNE     : 2; // CFG2[11:10]
    unsigned        TXRESTUNE      : 2; // CFG2[13:12]
    unsigned        TXPREEMPPTUNE  : 1; // CFG2[14:14]
    unsigned        TXPREEMPATUNE  : 2; // CFG2[16:15]
    unsigned        TXHSXVTUNE     : 2; // CFG2[18:17]
    unsigned        TXFSLSTUNE     : 4; // CFG2[22:19]
    unsigned        SQRXTUNE       : 3; // CFG2[25:23]
    unsigned        OTGTUNE        : 3; // CFG2[28:26]
    unsigned        COMPDISTUNE    : 3; // CFG2[31:29]
} HSB_USB30PHYCFG2;

typedef	struct	{
    unsigned        LOS_BIAS       : 3; // CFG3[02:00]
    unsigned        TXTERM_OFFSET  : 5; // CFG3[07:03]
    unsigned        TXSWING        : 7; // CFG3[14:08]
    unsigned        TXDEEMPH6DB    : 6; // CFG3[20:15]
    unsigned        TXDEEMPH3P5DB  : 6; // CFG3[26:21]
    unsigned        LOSLEVEL       : 5; // CFG3[31:27]
} HSB_USB30PHYCFG3;

typedef	struct	{
    unsigned        XCVRSEL        : 2; // CFG4[01:00]
    unsigned        OVRD_XCVRSEL   : 1; // CFG4[02:02]
    unsigned        WORDIF         : 1; // CFG4[03:03]
    unsigned        OVRD_WORDIF    : 1; // CFG4[04:04]
    unsigned        VBUSVLDEXT     : 1; // CFG4[05:05]
    unsigned        OVRD_VBUSVLDEXT: 1; // CFG4[06:06]
    unsigned        TXBITSTFH      : 1; // CFG4[07:07]
    unsigned        OVRD_TXBITSTFH : 1; // CFG4[08:08]
    unsigned        TXBITSTF       : 1; // CFG4[09:09]
    unsigned        OVRD_TXBITSTF  : 1; // CFG4[10:10]
    unsigned        TERMSEL        : 1; // CFG4[11:11]
    unsigned        OVRD_TERMSEL   : 1; // CFG4[12:12]
    unsigned        SUSPENDM       : 1; // CFG4[13:13]
    unsigned        OVRD_SUSPENDM  : 1; // CFG4[14:14]
    unsigned        SLEEPM         : 1; // CFG4[15:15]
    unsigned        OVRD_SLEEPM    : 1; // CFG4[16:16]
    unsigned        OPMODE         : 2; // CFG4[18:17]
    unsigned        OVRD_OPMODE    : 1; // CFG4[19:19]
    unsigned        IDPU           : 1; // CFG4[20:20]
    unsigned        OVRD_IDPU      : 1; // CFG4[21:21]
    unsigned        DRVVBUS        : 1; // CFG4[22:22]
    unsigned        OVRD_DRVVBUS   : 1; // CFG4[23:23]
    unsigned        DPPD           : 1; // CFG4[24:24]
    unsigned        OVRD_DPPD      : 1; // CFG4[25:25]
    unsigned        DMPD           : 1; // CFG4[26:26]
    unsigned        OVRD_DMPD      : 1; // CFG4[27:27]
    unsigned                       : 4; // CFG4[31:28]
} HSB_USB30PHYCFG4;

//typedef	struct	{
//    unsigned        FSTXENB        : 1; // CFG5[00:00]
//    unsigned        FSXCVROWNER    : 1; // CFG5[01:01]
//    unsigned        FSSE0EXT       : 1; // CFG5[02:02]
//    unsigned        FSDATAEXT      : 1; // CFG5[03:03]
//    unsigned        BYPASSDMEN     : 1; // CFG5[04:04]
//    unsigned        BYPASSDPEN     : 1; // CFG5[05:05]
//    unsigned        BYPASSDMDATA   : 1; // CFG5[06:06]
//    unsigned        BYPASSDPDATA   : 1; // CFG5[07:07]
//    unsigned        CHRGSEL        : 1; // CFG5[08:08]
//    unsigned        ACAENB         : 1; // CFG5[09:09]
//    unsigned        VBUSVLDEXTSEL  : 1; // CFG5[10:10]
//    unsigned        BYPASSSEL      : 1; // CFG5[11:11]
//    unsigned        VDATSRCENB     : 1; // CFG5[12:12]
//    unsigned        VDATDETENB     : 1; // CFG5[13:13]
//    unsigned        DCDENB         : 1; // CFG5[14:14]
//    unsigned        OTGDISABLE     : 1; // CFG5[15:15]
//    unsigned        VATESTENB      : 1; // CFG5[16:16]
//    unsigned        LOOPBACKENB    : 1; // CFG5[17:17]
//    unsigned        ADPPRBENB      : 1; // CFG5[18:18]
//    unsigned        OVRD_ADPPRBENB : 1; // CFG5[19:19]
//    unsigned        RETENB         : 1; // CFG5[20:20]
//    unsigned        RTUNE_REQ      : 1; // CFG5[21:21]
//    unsigned        TX2RXLB        : 1; // CFG5[22:22]
//    unsigned                       : 9; // CFG5[31:23]
//} HSB_USB30PHYCFG5;

typedef	struct	{
    unsigned                       : 10; // CFG5[09:00]
    unsigned        VBUSVLDEXTSEL  : 1; // CFG5[10:10]
    unsigned                       : 4; // CFG5[14:11]
    unsigned        OTGDISABLE     : 1; // CFG5[15:15]
    unsigned                       : 1; // CFG5[16:16]
    unsigned        LOOPBACKENB    : 1; // CFG5[17:17]
    unsigned                       : 2; // CFG5[19:18]
    unsigned        RETENB         : 1; // CFG5[20:20]
    unsigned        RTUNE_REQ      : 1; // CFG5[21:21]
    unsigned        TX2RXLB        : 1; // CFG5[22:22]
    unsigned                       : 7; // CFG5[29:23]
    unsigned        WF_TB          : 1; // CFG5[30:30]
    unsigned        WF_SOC         : 1; // CFG5[31:31]
} HSB_USB30PHYCFG5;

typedef	union {
    unsigned		        nReg;
    HSB_USB30PHYCFG0    bReg;
} HSB_USB30PHYCFG0_U;

typedef	union {
    unsigned		        nReg;
    HSB_USB30PHYCFG1    bReg;
} HSB_USB30PHYCFG1_U;

typedef	union {
    unsigned		        nReg;
    HSB_USB30PHYCFG2    bReg;
} HSB_USB30PHYCFG2_U;

typedef	union {
    unsigned		        nReg;
    HSB_USB30PHYCFG3    bReg;
} HSB_USB30PHYCFG3_U;

typedef	union {
    unsigned		        nReg;
    HSB_USB30PHYCFG4    bReg;
} HSB_USB30PHYCFG4_U;

typedef	union {
    unsigned		        nReg;
    HSB_USB30PHYCFG5    bReg;
} HSB_USB30PHYCFG5_U;

///////////////////////////////////////////////////////////////////
//
//	USB30 LINK CFG
//
typedef	struct	{
    unsigned        SUSCLK_DIV     : 4; // CFG0[03:00]
    unsigned        SUSCLK_DIV_EN  : 1; // CFG0[04:04]
    unsigned        BENDIAN        : 1; // CFG0[05:05]
    unsigned        PME_EN         : 1; // CFG0[06:06]
    unsigned        HUB_OVRCRNT    : 2; // CFG0[08:07]
    unsigned        U2PORT_ENB     : 1; // CFG0[09:09]
    unsigned        U3PORT_ENB     : 1; // CFG0[10:10]
    unsigned        PPC_PRSNT      : 1; // CFG0[11:11]
    unsigned        FLADJ          : 6; // CFG0[17:12]
    unsigned        FILTER_BYPASS  : 4; // CFG0[21:18]
    unsigned        XHCI_REV       : 1; // CFG1[22:22]
    unsigned        PIPE_DW        : 2; // CFG1[24:23]
    unsigned        vcc_reset_n    : 1; // CFG1[25:25]
    unsigned        vaux_reset_n   : 1; // CFG1[26:26]
    unsigned        HUB_PERM_ATC   : 2; // CFG1[28:27]
    unsigned        HOST_MSI_EN    : 1; // CFG1[29:29]
    unsigned        USB20ONLY      : 1; // CFG1[30:30]
    //unsigned        IRQ            : 1; // CFG1[27:27]
    //unsigned        ACK            : 1; // CFG1[28:28]
    //unsigned                       : 3; // CFG1[31:29]
    unsigned        BME            : 1; // CFG1[31:31]
} HSB_USB30LINKCFG;

typedef	union {
    unsigned		        nReg;
    HSB_USB30LINKCFG    bReg;
} HSB_USB30LINKCFG_U;

///////////////////////////////////////////////////////////////////
//
//	TOP
//
typedef	struct {
    HSIOBUS_U           uCLKMASK       ;    // 00    
    HSIOBUS_U           uSWRESET       ;    // 01
    HSB_MEM_U           uMEMPWRDN      ;    // 02
    HSB_IMEMCFG_U       uIMEMCFG       ;    // 03
    unsigned                uUSB20H_CFG    ;    // 04
    unsigned                uUSB20H_CFG0   ;    // 05
    unsigned                uUSB20H_CFG1   ;    // 06
    unsigned                uUSB20H_CFG2   ;    // 07
    unsigned                uUSB20H_STS    ;    // 08
    unsigned                uASD_IOMUX     ;    // 09
	unsigned		        undef0[2]      ;    // 11:10
    unsigned                uSATA_PHY_STS  ;    // 12
    unsigned                uSATA_HLINK_STS;    // 13
    unsigned                uSATA_DLINK_STS;    // 14
	unsigned		        undef1         ;    // 15
    //HSB_SATAPHY_CTRL_U  uSATA_PHY_CTRL ;    // 16
    unsigned                undef5         ;
    unsigned                uSATA_LINK_CTRL;    // 17
    unsigned                uSATA_MUX_CTRL ;    // 18
	unsigned		        undef2         ;    // 19
    HSB_CFG_U           uHSB_CFG       ;    // 20
    HSB_KP_U            uHSB_KP        ;    // 21
	unsigned		        undef3[2]      ;    // 23:22
    HSB_GMACCFG0_U      uGMAC0_CFG0    ;    // 24
    HSB_GMACCFG1_U      uGMAC0_CFG1    ;    // 25
    HSB_GMACCFG0_U      uGMAC1_CFG0    ;    // 26
    HSB_GMACCFG1_U      uGMAC1_CFG1    ;    // 27
    unsigned                uESW_CFG       ;    // 28
	unsigned		        undef4[3]      ;    // 31:29
    HSB_CACHECTRL_U     uCACHECTRL     ;    // 39:32
    unsigned                uAXIPWRCTRL    ;    // 40
    HSB_USB30PHYCFG0_U  uUSB30PHY_CFG0 ;    // 41
    HSB_USB30PHYCFG1_U  uUSB30PHY_CFG1 ;    // 42
    HSB_USB30PHYCFG2_U  uUSB30PHY_CFG2 ;    // 43
    HSB_USB30PHYCFG3_U  uUSB30PHY_CFG3 ;    // 44
    HSB_USB30PHYCFG4_U  uUSB30PHY_CFG4 ;    // 45
    HSB_USB30PHYCFG5_U  uUSB30PHY_CFG5 ;    // 46
    HSB_USB30LINKCFG_U  uUSB30LINK_CFG ;    // 47
}	HSBUSCFG;

#endif
// vim:ts=4:et:sw=4:sts=4
