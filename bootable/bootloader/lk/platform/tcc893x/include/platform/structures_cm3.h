/*
 * Copyright (c) 2011 Telechips, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


/*
    Not finished block "SD, OVERLAY, AUDIO0/1, NFC, USBOTG"
*/

#ifndef _STRUCTURES_CM3_H_
#define _STRUCTURES_CM3_H_

/*******************************************************************************
*
*    TCC893x DataSheet PART 9 CM3 BUS
*
********************************************************************************/


/*******************************************************************************
*    MailBox Register Define (Base Addr = 0x790A0000[main], 0x790B0000[sub])
********************************************************************************/

typedef struct {
    unsigned DATA           :32;
} MBOX_DATA_IDX_TYPE;

typedef union {
	unsigned long			nREG;
	MBOX_DATA_IDX_TYPE      bREG;
}	MBOX_CTL_xxx_TYPE;

typedef struct {
	unsigned LEVEL			:2;
	unsigned                :2;
	unsigned IEN            :1;
	unsigned OEN            :1;
	unsigned FLUSH          :1;
	unsigned                :25;
}	MBOX_CTL_IDX_TYPE;

typedef union {
	unsigned long           nREG;
	MBOX_CTL_IDX_TYPE       bREG;
}	MBOX_CTL_016_TYPE;

typedef struct {
	unsigned TXD            :32;
}	MBOX_TXD_IDX_TYPE;

typedef union {
	unsigned long           nREG;
	MBOX_TXD_IDX_TYPE       bREG;
}	MBOX_CTL_TXD_TYPE;

typedef struct {
	unsigned RXD            :32;
}	MBOX_RXD_IDX_TYPE;

typedef union {
	unsigned long           nREG;
	MBOX_RXD_IDX_TYPE       bREG;
}	MBOX_CTL_RXD_TYPE;


typedef struct _MAILBOX {
  	MBOX_CTL_TXD_TYPE    uMBOX_TX0;	// 0x0000
	MBOX_CTL_TXD_TYPE    uMBOX_TX1;	// 0x0004
	MBOX_CTL_TXD_TYPE    uMBOX_TX2; 	// 0x0008
	MBOX_CTL_TXD_TYPE    uMBOX_TX3; 	// 0x000C
	MBOX_CTL_TXD_TYPE    uMBOX_TX4;	// 0x0010
	MBOX_CTL_TXD_TYPE    uMBOX_TX5;	// 0x0014
	MBOX_CTL_TXD_TYPE    uMBOX_TX6;	// 0x0018
	MBOX_CTL_TXD_TYPE    uMBOX_TX7;	// 0x001C
	MBOX_CTL_RXD_TYPE    uMBOX_RX0;	// 0x0020
	MBOX_CTL_RXD_TYPE    uMBOX_RX1;	// 0x0024
	MBOX_CTL_RXD_TYPE    uMBOX_RX2;	// 0x0028
	MBOX_CTL_RXD_TYPE    uMBOX_RX3;	// 0x002C
	MBOX_CTL_RXD_TYPE    uMBOX_RX4;	// 0x0030
	MBOX_CTL_RXD_TYPE    uMBOX_RX5;	// 0x0034
	MBOX_CTL_RXD_TYPE    uMBOX_RX6;	// 0x0038
	MBOX_CTL_RXD_TYPE    uMBOX_RX7;	// 0x003C
	MBOX_CTL_016_TYPE    uMBOX_CTL_016;	// 0x0040
	MBOX_CTL_xxx_TYPE    uMBOX_CTL_017;	// 0x0044
	MBOX_CTL_xxx_TYPE    uMBOX_CTL_018;	// 0x0048
	MBOX_CTL_xxx_TYPE    uMBOX_CTL_019;	// 0x004C
	MBOX_CTL_xxx_TYPE    uMBOX_CTL_020;	// 0x0050
} MAILBOX, *PMAILBOX;

/*******************************************************************************
*    Cortex M3 BUS Configuration Registers Define (Base Addr =  0x790F0000)
********************************************************************************/

typedef struct {
    unsigned NMI            :1;
	unsigned BIGEND         :1;
	unsigned DBGEN          :1;
	unsigned FIX_TYPE       :1;
	unsigned SLEEP_H        :1;
	unsigned STK            :1;
	unsigned CLK_CH         :1;
	unsigned MPU_DIS        :1;
	unsigned DAPEN          :1;
	unsigned COMP_EN        :1;
	unsigned DNOT           :1;
	unsigned                :11;
} CM3_CFG1_IDX_TYPE;

typedef union {
	unsigned long			nREG;
	CM3_CFG1_IDX_TYPE       bREG;
} CM3_CFG1_TYPE;

typedef struct {
    unsigned AUXFAULT       :32;
} CM3_CFG2_IDX_TYPE;

typedef union {
	unsigned long			nREG;
	CM3_CFG2_IDX_TYPE       bREG;
} CM3_CFG2_TYPE;

typedef struct {
    unsigned TSVALUEB       :32;
} CM3_CFG3_IDX_TYPE;

typedef union {
	unsigned long			nREG;
	CM3_CFG3_IDX_TYPE       bREG;
} CM3_CFG3_TYPE;

typedef struct {
    unsigned TSVALUEB       :16;
	unsigned                :16;
} CM3_CFG4_IDX_TYPE;

typedef union {
	unsigned long			nREG;
	CM3_CFG4_IDX_TYPE       bREG;
} CM3_CFG4_TYPE;

typedef struct {
    unsigned                :1;
	unsigned SF             :1;
	unsigned Cipher         :1;
	unsigned MBOX           :1;
	unsigned wdma           :1;
	unsigned rdma0          :1;
	unsigned rdma1          :1;
	unsigned pktg0          :1;
	unsigned pktg1          :1;
	unsigned pktg2          :1;
	unsigned pktg3          :1;
	unsigned V_IRQ          :1;
	unsigned V_FIQ          :1;
	unsigned AUDIO_DMA      :1;
	unsigned AUDIO          :1;
	unsigned                :1;
	unsigned P_FIQ          :1;
	unsigned                :5;
	unsigned P_IRQ          :1;
	unsigned                :9;
} CM3_IRQ_MASK_POL_IDX_TYPE;

typedef union {
	unsigned long				nREG;
	CM3_IRQ_MASK_POL_IDX_TYPE	bREG;
} CM3_IRQ_MASK_POL_TYPE;

typedef struct {
    unsigned CM3            :1;
	unsigned PERI           :1;
	unsigned CODE           :1;
	unsigned DATA           :1;
	unsigned MBOX           :1;
	unsigned                :27;
} CM3_HCLK_MASK_IDX_TYPE;

typedef union {
	unsigned long           nREG;
	CM3_HCLK_MASK_IDX_TYPE  bREG;
} CM3_HCLK_MASK_TYPE;

typedef struct {
    unsigned                :1;
	unsigned POR            :1;
	unsigned SYS            :1;
	unsigned                :29;
} CM3_RESET_IDX_TYPE;

typedef union {
	unsigned long           nREG;
	CM3_RESET_IDX_TYPE  bREG;
} CM3_RESET_TYPE;

typedef struct {
    unsigned STCLK_DIV_CFG  :18;
	unsigned                :14;
} CM3_STCLK_DIV_CFG_IDX_TYPE;

typedef union {
	unsigned long               nREG;
	CM3_STCLK_DIV_CFG_IDX_TYPE  bREG;
} CM3_STCLK_DIV_CFG_TYPE;

typedef struct {
    unsigned ECNT           :4;
	unsigned                :28;
} CM3_ECNT_IDX_TYPE;

typedef union {
	unsigned long            nREG;
	CM3_ECNT_IDX_TYPE        bREG;
} CM3_ECNT_TYPE;

typedef struct {
    unsigned                :1;
    unsigned A2XMOD         :3;
	unsigned                :28;
} CM3_A2X_IDX_TYPE;

typedef union {
	unsigned long            nREG;
	CM3_A2X_IDX_TYPE        bREG;
} CM3_A2X_TYPE;

typedef struct {
    unsigned REMAP_7        :4;
    unsigned REMAP_6        :4;
    unsigned REMAP_5        :4;
    unsigned REMAP_4        :4;
    unsigned REMAP_3        :4;
    unsigned REMAP_2        :4;
    unsigned DCODE          :4;
    unsigned ICODE          :4;
} CM3_REMAP0_IDX_TYPE;

typedef union {
	unsigned long            nREG;
	CM3_REMAP0_IDX_TYPE      bREG;
} CM3_REMAP0_TYPE;

typedef struct {
    unsigned REMAP_F        :4;
    unsigned REMAP_E        :4;
    unsigned REMAP_D        :4;
    unsigned REMAP_C        :4;
    unsigned REMAP_B        :4;
    unsigned REMAP_A        :4;
    unsigned REMAP_9        :4;
    unsigned REMAP_8        :4;
} CM3_REMAP1_IDX_TYPE;

typedef union {
	unsigned long            nREG;
	CM3_REMAP1_IDX_TYPE      bREG;
} CM3_REMAP1_TYPE;

typedef struct _CM3_TSD_CFG {
	CM3_CFG1_TYPE            CM3_CFG_1;	    // 0x00
	CM3_CFG2_TYPE            CM3_CFG_2;	    // 0x04
	CM3_CFG3_TYPE            CM3_CFG_3;	    // 0x08
	CM3_CFG4_TYPE            CM3_CFG_4;	    // 0x0C
	CM3_IRQ_MASK_POL_TYPE    IRQ_MASK_POL;	// 0x10
	CM3_HCLK_MASK_TYPE       HCLK_MASK;	    // 0x14
	CM3_RESET_TYPE           CM3_RESET;	    // 0x18
	unsigned                 :32;           // 0x1C
	CM3_STCLK_DIV_CFG_TYPE   STCLK_DIV_CFG; // 0x20
	unsigned                 :32;           // 0x24
	CM3_ECNT_TYPE            ECNT;          // 0x28
	unsigned                 :32;           // 0x2C
	unsigned                 :32;           // 0x30
	unsigned                 :32;           // 0x34
	CM3_A2X_TYPE             CM3_A2X;       // 0x38
	unsigned                 :32;           // 0x3C
	CM3_REMAP0_TYPE          REMAP0;        // 0x40
	CM3_REMAP1_TYPE          REMAP1;        // 0x44
} CM3_TSD_CFG, *PCM3_TSD_CFG;

#endif /* _STRUCTURES_CM3_H_ */
