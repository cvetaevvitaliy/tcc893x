/****************************************************************************
 *   FileName    : nand_regs.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall 
constitute any express or implied warranty of any kind, including without limitation, any warranty 
of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright 
or other third party intellectual property right. No warranty is made, express or implied, 
regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of 
or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************/
#ifndef _NAND_REGS_H
#define _NAND_REGS_H

#if defined(NFC_VER_50)
#define NAND_IO_ECC_SEL								*(volatile unsigned long *)(tcc_p2v(0x98013010UL))
#define	NAND_BUS_SOFTWARE_RESET						*(volatile unsigned long *)(tcc_p2v(0x98006018UL))
#define	NAND_BUS_CLOCK_CTRL 						*(volatile unsigned long *)(tcc_p2v(0x98006014UL))
#define	NAND_BUS_CLOCK_CTRL1 						*(volatile unsigned long *)(tcc_p2v(0x98006060UL))
#define NAND_BUS_CLOCK_CTRL_ECC						Hw9
#define NAND_BUS_CLOCK_CTRL_NFC						Hw16

#define NAND_EXT_MEM_CONFIG_REG						*(volatile unsigned long *)(tcc_p2v(0x980130E0UL))

#define	NAND_IO_CKC_BUS_ECC							NAND_BUS_CLOCK_CTRL_ECC
#define	NAND_IO_CKC_BUS_NFC							NAND_BUS_CLOCK_CTRL_NFC
#define	NAND_IO_CKC_EnableBUS(X)					(BITSET(NAND_BUS_CLOCK_CTRL, (X)))
#define	NAND_IO_CKC_DisableBUS(X)					(BITCLR(NAND_BUS_CLOCK_CTRL, (X)))

#define NFC_CTRL_READY_INTR_ENABLE					(1<<31)
#define NFC_CTRL_PROGRAM_INTR_ENABLE				(1<<30)
#define NFC_CTRL_READ_INTR_ENABLE					(1<<29)
#define NFC_CTRL_DMA_REQUEST_ENABLE					(1<<28)
#define NFC_CTRL_FIFO_READY_TO_BURST_ACCESS			(1<<27)
#define NFC_CTRL_BUS_WIDTH_FIELD					(1<<26)
#define NFC_CTRL_BUS_WIDTH_8BIT						(0<<26)
#define NFC_CTRL_BUS_WIDTH_16BIT					(1<<26)
#define NFC_CTRL_CS_FIELD							(0xF<<22)
#define NFC_CTRL_CS3								(1<<25)
#define NFC_CTRL_CS2								(1<<24)
#define NFC_CTRL_CS1								(1<<23)
#define NFC_CTRL_CS0								(1<<22)
#define NFC_CTRL_RDY_RDY							(1<<21)
#define NFC_CTRL_BURST_SIZE_FIELD					(3<<19)
#define NFC_CTRL_BURST_SIZE_1						(0<<19)
#define NFC_CTRL_BURST_SIZE_2						(1<<19)
#define NFC_CTRL_BURST_SIZE_4						(2<<19)
#define NFC_CTRL_BURST_SIZE_8						(3<<19)
#define NFC_CTRL_PAGE_SIZE_FIELD					(7<<16)
#define NFC_CTRL_PAGE_SIZE_256W						(0<<16)
#define NFC_CTRL_PAGE_SIZE_512B						(1<<16)
#define NFC_CTRL_PAGE_SIZE_1024W					(2<<16)
#define NFC_CTRL_PAGE_SIZE_2048B					(3<<16)
#define NFC_CTRL_PAGE_SIZE_4096B					(4<<16)
#define NFC_CTRL_HIGH_BYTES_UNMASK					(0<<15)
#define NFC_CTRL_HIGH_BYTES_MASK					(1<<15)

#define NFC_CTRL1_DMA_ACK_ENABLE					(1<<31)
#define NFC_CTRL1_MDATA_COUNT_FIELD					(3<<24)
#define NFC_CTRL1_MDATA_COUNT(n)					(n<<24)

#define NFC_IRQ_READY_FLAG							(1<<6)
#define NFC_IRQ_PROGRAM_FLAG						(1<<5)
#define NFC_IRQ_READ_FLAG							(1<<4)
#define NFC_IRQ_READY_INTR							(1<<2)
#define NFC_IRQ_PROGRAM_INTR						(1<<1)
#define NFC_IRQ_READ_INTR							(1<<0)

#define ECC_CTRL_ECC12_DECODE_INTR_ENABLE			(1<<30)
#define ECC_CTRL_ECC8_DECODE_INTR_ENABLE			(1<<29)
#define ECC_CTRL_ECC4_DECODE_INTR_ENABLE			(1<<28)

#define ECC_CTRL_ECC_DISABLE						(0<<0)
#define ECC_CTRL_ECC_EN_SLC_ECC_ENCODE				(8<<0)
#define ECC_CTRL_ECC_EN_SLC_ECC_DECODE				(9<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC4_ENCODE				(10<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC4_DECODE				(11<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC8_ENCODE				(12<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC8_DECODE				(13<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC12_ENCODE			(14<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC12_DECODE			(15<<0)

#define ECC_IRQ_SLC_DECODE_END_FLAG					0x00400000
#define ECC_IRQ_MLC4_DECODE_END_FLAG				0x00100000
#define ECC_IRQ_MLC8_DECODE_END_FLAG				0x00040000
#define ECC_IRQ_MLC12_DECODE_END_FLAG				0x00010000

#define ECC_IRQ_MLC4_DECODE_END_INTR				0x00000010
#define ECC_IRQ_MLC8_DECODE_END_INTR				0x00000040
#define ECC_IRQ_MLC12_DECODE_END_INTR				0x00000001


#elif defined(NFC_VER_200) || defined(NFC_VER_300)

#define NFC_CTRL_CS_FIELD							(0xF<<28)
#define NFC_CTRL_CS3								(1<<31)
#define NFC_CTRL_CS2								(1<<30)
#define NFC_CTRL_CS1								(1<<29)
#define NFC_CTRL_CS0								(1<<28)
#if defined(NFC_VER_200)
#define NFC_CTRL_READ_DATA_CAPTURE_DELAY_FIELD		(0x7<<25)
#define NFC_CTRL_READ_DATA_CAPTURE_DELAY(delay)		(delay<<25)
#define NFC_CTRL_AUTO_READY_FLAG_WAIT_ENABLE		(1<<24)
#define NFC_CTRL_RDYCFG_FIELD						(3<<22)
#define NFC_CTRL_RDYCFG(n)							(n<<22)
#elif defined(NFC_VER_300)
#define NFC_CTRL_READ_DATA_CAPTURE_DELAY_FIELD		(0xF<<24)
#define NFC_CTRL_READ_DATA_CAPTURE_DELAY(delay)		(delay<<24)
#define NFC_CTRL_AUTO_READY_FLAG_WAIT_ENABLE		(1<<23)
#define NFC_CTRL_RDYCFG_FIELD						(3<<21)
#define NFC_CTRL_RDYCFG(n)							(n<<21)
#endif
#define NFC_CTRL_DMA_ACK_ENABLE						(1<<20)
#define NFC_CTRL_DMA_MODE_GDMA						(0<<19)
#define NFC_CTRL_DMA_MODE_NDMA						(1<<19)
#define NFC_CTRL_BURST_MODE_DATA_ONLY				(0<<18)
#define NFC_CTRL_BURST_MODE_DATA_N_SPARE			(1<<18)
#define NFC_CTRL_ECC_ENABLE							(1<<17)
#define NFC_CTRL_BURST_READ							(0<<16)
#define NFC_CTRL_BURST_PROGRAM						(1<<16)
#define NFC_CTRL_MDATA_COUNT_FIELD					(3<<14)
#define NFC_CTRL_MDATA_COUNT(n)						(n<<14)
#define NFC_CTRL_RDYSEL_3TO0						(0<<12)
#define NFC_CTRL_RDYSEL_0_ONLY						(1<<12)
#define NFC_CTRL_READY_FLAG_N_INTR_MASK				(0<<11)
#define NFC_CTRL_READY_FLAG_N_INTR_UNMASK			(1<<11)
#define NFC_CTRL_HIGH_BYTES_UNMASK					(0<<10)
#define NFC_CTRL_HIGH_BYTES_MASK					(1<<10)
#define NFC_CTRL_BUS_WIDTH_FIELD					(3<<8)
#define NFC_CTRL_BUS_WIDTH_8BIT						(0<<8)
#define NFC_CTRL_BUS_WIDTH_16BIT					(1<<8)
#define NFC_CTRL_BUS_WIDTH_32BIT					(2<<8)
#define NFC_CTRL_BURST_SIZE_FIELD					(3<<6)
#define NFC_CTRL_BURST_SIZE_1						(0<<6)
#define NFC_CTRL_BURST_SIZE_2						(1<<6)
#define NFC_CTRL_BURST_SIZE_4						(2<<6)
#define NFC_CTRL_BURST_SIZE_8						(3<<6)
#define NFC_CTRL_PAGE_SIZE_FIELD					(7<<3)
#define NFC_CTRL_PAGE_SIZE_256W						(0<<3)
#define NFC_CTRL_PAGE_SIZE_512B						(1<<3)
#define NFC_CTRL_PAGE_SIZE_1024W					(2<<3)
#define NFC_CTRL_PAGE_SIZE_2048B					(3<<3)
#define NFC_CTRL_PAGE_SIZE_4096B					(4<<3)
#define NFC_CTRL_PAGE_SIZE_8192B					(5<<3)

#define NFC_IRQCFG_READY_INTR_ENABLE				(1<<31)
#define NFC_IRQCFG_PROGRAM_INTR_ENABLE				(1<<30)
#define NFC_IRQCFG_READ_INTR_ENABLE					(1<<29)
#define NFC_IRQCFG_SPARE_BURST_WR_INTR_ENABLE		(1<<27)
#define NFC_IRQCFG_SPARE_BURST_RD_INTR_ENABLE		(1<<26)
#define NFC_IRQCFG_MLC_ECC_DECODE_INTR_ENABLE		(1<<23)

#define NFC_IRQ_READY_INTR							(1<<31)
#define NFC_IRQ_PROGRAM_INTR						(1<<30)
#define NFC_IRQ_READ_INTR							(1<<29)
#define NFC_IRQ_SPARE_BURST_WR_INTR					(1<<27)
#define NFC_IRQ_SPARE_BURST_RD_INTR					(1<<26)
#define NFC_IRQ_MLC_ECC_DECODE_INTR					(1<<23)
#define NFC_IRQ_READY_FLAG							(1<<15)
#define NFC_IRQ_PROGRAM_FLAG						(1<<14)
#define NFC_IRQ_READ_FLAG							(1<<13)
#define NFC_IRQ_SPARE_BURST_WR_FLAG					(1<<11)
#define NFC_IRQ_SPARE_BURST_RD_FLAG					(1<<10)
#define NFC_IRQ_MLC_ECC_DECODE_FLAG					(1<<7)
#define NFC_IRQ_MLC_ECC_ENCODE_FLAG					(1<<6)
#define NFC_IRQ_SLC_ECC_DECODE_FLAG					(1<<5)
#define NFC_IRQ_SLC_ECC_ENCODE_FLAG					(1<<4)
#define NFC_IRQ_ECC_ERROR_FLAG						(1<<3)

#define NFC_STATUS_READY_FLAG						(1<<31)
#define NFC_STATUS_FIFO_READY_TO_BURST_ACCESS		(1<<30)
#define NFC_STATUS_CURRENT_SUBPAGE_FIELD			(0x1F<<16)

#define NDMA_CTRL_SP_ECC_EN							(1<<15)
#define NDMA_CTRL_LOCK								(1<<11)
#define NDMA_CTRL_ARBITRATION_ON					(0<<10)
#define NDMA_CTRL_ARBITRATION_OFF					(1<<10)
#define NDMA_CTRL_BURST_SIZE_FIELD					(3<<6)
#define NDMA_CTRL_BURST_SIZE_1						(0<<6)
#define NDMA_CTRL_BURST_SIZE_2						(1<<6)
#define NDMA_CTRL_BURST_SIZE_4						(2<<6)
#define NDMA_CTRL_BURST_SIZE_8						(3<<6)
#define NDMA_CTRL_WORD_SIZE_FIELD					(3<<4)
#define NDMA_CTRL_WORD_SIZE_8BIT					(0<<4)
#define NDMA_CTRL_WORD_SIZE_16BIT					(1<<4)
#define NDMA_CTRL_WORD_SIZE_32BIT					(2<<4)
#define NDMA_CTRL_DMA_DONE_FLAG						(1<<3)
#define NDMA_CTRL_SEQUENCIAL_MODE_OFF				(0<<1)
#define NDMA_CTRL_SEQUENCIAL_MODE_ON				(1<<1)
#define NDMA_CTRL_ENABLE							(1<<0)

#define ECC_CTRL_ECC_DATASIZE_FIELD					(0x7FF<<ECC_SHIFT_DATASIZE)

#if defined(NFC_VER_200)
#define ECC_CTRL_MLC_ECC_CRRECTION_PASS_FIELD		(1<<5)
#define ECC_CTRL_MLC_ECC_CRRECTION_PASS_ENABLE		(1<<5)
#define ECC_CTRL_MODE_FIELD							(1<<4)
#define ECC_CTRL_MODE_REFER_TO_NAND_IO				(0<<4)
#define ECC_CTRL_MODE_REFER_TO_NFC_MEMORY			(1<<4)
#define ECC_CTRL_ECC_EN_FIELD						(0xF<<0)
#elif defined(NFC_VER_300)
#define ECC_CTRL_MLC_ECC_CRRECTION_PASS_FIELD		(1<<6)
#define ECC_CTRL_MLC_ECC_CRRECTION_PASS_ENABLE		(1<<6)
#define ECC_CTRL_MODE_FIELD							(1<<5)
#define ECC_CTRL_MODE_REFER_TO_NAND_IO				(0<<5)
#define ECC_CTRL_ECC_EN_FIELD						(0x1F<<0)
#define ECC_CTRL_MODE_REFER_TO_NFC_MEMORY			(1<<5)
#endif

#define ECC_CTRL_ECC_EN_SLC_ECC_ENCODE				(4<<0)
#define ECC_CTRL_ECC_EN_SLC_ECC_DECODE				(5<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC4_ENCODE				(6<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC4_DECODE				(7<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC6_ENCODE				(8<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC6_DECODE				(9<<0)

#if defined(NFC_VER_200)
#define ECC_CTRL_ECC_EN_MLC_ECC12_ENCODE			(10<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC12_DECODE			(11<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC16_ENCODE			(12<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC16_DECODE			(13<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC24_ENCODE			(14<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC24_DECODE			(15<<0)
#elif defined(NFC_VER_300)
#define ECC_CTRL_ECC_EN_MLC_ECC8_ENCODE				(10<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC8_DECODE				(11<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC12_ENCODE			(12<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC12_DECODE			(13<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC16_ENCODE			(14<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC16_DECODE			(15<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC24_ENCODE			(16<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC24_DECODE			(17<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC40_ENCODE			(18<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC40_DECODE			(19<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC60_ENCODE			(20<<0)
#define ECC_CTRL_ECC_EN_MLC_ECC60_DECODE			(21<<0)
#endif

#endif

#if defined(TCC89XX) || defined(TCC92XX) || defined(TCC800X)
#define ECC_SHIFT_DATASIZE						4
#elif defined(TCC93XX) || defined(TCC88XX) || defined(TCC892X) || defined(TCC801X) || defined(TCC8925S) || defined(TCC893X) || defined(TCC370X)
#define ECC_SHIFT_DATASIZE						16
#endif

#if defined(NFC_VER_200) // TCC88xx, TCC892x, TCC93xx
#define NDMA_ECC_ADDR_SHIFT						16
#elif defined(NFC_VER_300) // TCC8925S, TCC893X
#define NDMA_ECC_ADDR_SHIFT						14
#define NDMA_ECC_MODE_SHIFT						27
#endif

#define ECC_CSTATE_ECC_IDLE							(0x00000001)
#if defined(NFC_VER_200)
#define ECC_CSTATE_ERROR_ON_CURRENT_SUBPAGE			(0x08000000)
#elif defined(NFC_VER_300)
#define ECC_CSTATE_ERROR_ON_CURRENT_SUBPAGE			(0x00000010)
#endif

#if defined(NFC_VER_50)
typedef struct _NFC_V05_T
{
	volatile unsigned int   NFC_CMD;                // 0x00     W       NAND Flash Command Register
	volatile unsigned int   NFC_LADDR;              // 0x04     W       NAND Flash Linear Address Register
	volatile unsigned int   NFC_BADDR;              // 0x08     W       NAND Flash Block Address Register
	volatile unsigned int   NFC_SADDR;              // 0x08     W       NAND Flash Signal Address Register
	volatile unsigned int   NFC_WDATA;           	// 0x1x     R/W     NAND Flash Word Data Register
	volatile unsigned int	NOTDEFINE0[3];
	volatile unsigned int   NFC_LDATA;           	// 0x2x/3x R/W      NAND Flash Linear Data Register
	volatile unsigned int	NOTDEFINE1[7];			// 
	volatile unsigned int   NFC_SDATA;              // 0x40     R/W     NAND Flash Single Data Register
	volatile unsigned int	NOTDEFINE2[3];
	volatile unsigned int   NFC_CTRL;               // 0x50     R/W     NFC Control Register
	volatile unsigned int	NFC_PSTART;				// 0x54		W		NFC Program Start Register
	volatile unsigned int	NFC_RSTART;				// 0x58		W		NFC Read Start Register
	volatile unsigned int	NFC_DSIZE;				// 0x5C		R/W		NFC Data Size Register
	volatile unsigned int 	NFC_IRQ;				// 0x60		R/W		NFC Interrupt Request Register
	volatile unsigned int	NFC_RST;				// 0x64		W		NFC Controller Reset Register
	volatile unsigned int	NFC_CTRL1;				// 0x68		R/W		NFC Control Register1
	volatile unsigned int	NOTDEFINE3[1];
	volatile unsigned int	NFC_MDATA;				// 0x7X		R/W		NFC Multiple Data Register
} NFC_V05_T, *PNFC_V05_T;

typedef struct _ECC_V05_T
{
	volatile unsigned int   ECC_CTRL;				// 0x00     R/W     NAND Flash Command Register
	volatile unsigned int   ECC_BASE;				// 0x04		R/W		Base Address for ECC Calculation
	volatile unsigned int   ECC_MASK;				// 0x08		R/W	 	Address Mask for ECC MNT'g ARea
	volatile unsigned int   ECC_CLEAR;				// 0x0C		W		ECC Clear Register.
	volatile unsigned int	ECC_CODE0;				// 0x10  	R/W		1st  ECC Code Register
	volatile unsigned int	ECC_CODE1;				// 0x14  	R/W		2nd  ECC Code Register
	volatile unsigned int	ECC_CODE2;				// 0x18  	R/W		3rd  ECC Code Register
	volatile unsigned int	ECC_CODE3;				// 0x1C  	R/W		4th  ECC Code Register
	volatile unsigned int	ECC_CODE4;				// 0x20  	R/W		5th  ECC Code Register
	volatile unsigned int	ECC_CODE5;				// 0x24  	R/W		6th  ECC Code Register
	volatile unsigned int	ECC_CODE6;				// 0x28  	R/W		7th  ECC Code Register
	volatile unsigned int	ECC_CODE7;				// 0x2C  	R/W		8th  ECC Code Register
	volatile unsigned int	ECC_CODE8;				// 0x30  	R/W		9th  ECC Code Register
	volatile unsigned int	ECC_CODE9;				// 0x34  	R/W		10th  ECC Code Register
	volatile unsigned int	ECC_CODE10; 			// 0x38  	R/W		11th  ECC Code Register
	volatile unsigned int	ECC_CODE11; 			// 0x3C  	R/W		12th  ECC Code Register
	volatile unsigned int	ECC_CODE12; 			// 0x40  	R/W		13th  ECC Code Register
	volatile unsigned int	ECC_CODE13; 			// 0x44  	R/W		14th  ECC Code Register
	volatile unsigned int	ECC_CODE14; 			// 0x48  	R/W		15th  ECC Code Register
	volatile unsigned int	ECC_CODE15; 			// 0x4C  	R/W		16th  ECC Code register
	volatile unsigned int	ECC_EADDR[16]; 			// 0x50~8C 	R  		ECC Error Address Register0	
	volatile unsigned int	ECC_ERRNUM;				// 0x90		R/W		SLC ECC Error Number Register
	volatile unsigned int	ECC_IREQ;				// 0x94		R/W		ECC IREQ Register
	volatile unsigned int 	ECC_STATE;				// 0x98		R		ECC Current FSM State
	volatile unsigned int	ECC_CNT;				// 0x9C		R		ECC Current Data Input Count 
} ECC_V05_T, *PECC_V05_T;
#endif


#if defined(NFC_VER_200)
typedef struct _NFC_V2_T
{
	volatile unsigned int   NFC_CMD;                // 0x00     W       NAND Flash Command Register
	volatile unsigned int   NFC_LADDR;              // 0x04     W       NAND Flash Linear Address Register
	volatile unsigned int   NFC_SADDR;              // 0x08     W       NAND Flash Signal Address Register
	volatile unsigned int   NFC_SDATA;              // 0x0C     R/W     NAND Flash Single Data Register
	volatile unsigned int   NFC_WDATA;           	// 0x1x     R/W     NAND Flash Word Data Register
	volatile unsigned int	NOTDEFINE0[3];
	volatile unsigned int   NFC_LDATA;           	// 0x2x/3x R/W      NAND Flash Linear Data Register
	volatile unsigned int	NOTDEFINE1[7];			
	volatile unsigned int   NFC_MDATA;              // 0x40     R/W     NAND Flash Multiple Data Register
	volatile unsigned int   NFC_CACYC;              // 0x44     R/W     NAND Flash Command & Address Cycle Register
	volatile unsigned int   NFC_WRCYC;              // 0x48     R/W     NAND Flash Write Cycle Register
	volatile unsigned int   NFC_RECYC;              // 0x4C     R/W     NAND Flash Read Cycle Register
	volatile unsigned int   NFC_CTRL;               // 0x50     R/W     NFC Control Register
	volatile unsigned int   NFC_DSIZE;              // 0x54     R/W     NFC Data Size Register
	volatile unsigned int   NFC_SPARE;              // 0x58     R/W     NFC Spare Burst Transfer Control Register
	volatile unsigned int   NFC_PRSTART;            // 0x5C     R/W     NFC Burst Program/Read Start
	volatile unsigned int   NFC_RST;                // 0X60     R/W     NFC Reset Register
	volatile unsigned int   NFC_IRQCFG;             // 0x64     R/W     NFC Interrupt Request Control Register
	volatile unsigned int   NFC_IRQ;                // 0x68     R/W     NFC Interrupt Request Register
	volatile unsigned int   NFC_STATUS;             // 0x6C     R/W     NFC Stutus Register
	volatile unsigned int   NFC_RFWBASE;            // 0x70     R/W     NFC Read Flag Wait Base Word Register
	volatile unsigned int   NOTDEFINE2[35];         // 0x74~0xFC
	volatile unsigned int   NDMA_ADDR;              // 0X100    R/W     NFC DMA Source/Destination Register
	volatile unsigned int   NDMA_PARAM;             // 0X104    R/W     NFC DMA Parameter Register
	volatile unsigned int   NDMA_CADDR;             // 0X108    R       NFC DMA Current Address Register
	volatile unsigned int   NDMA_CTRL;              // 0x10C    R/W     NFC DMA Controll Register
	volatile unsigned int   NDMA_SPCTRL;            // 0x110    R/W     NFC DMA SUBPAGE Control Regiter
	volatile unsigned int   NDMA_STATUS;            // 0x114    R/W     NFC DMA Stutus Register
	volatile unsigned int   NOTDEFINE3[2];          // 0x118~011C
	volatile unsigned int   SP_CFG[32];             // 0x120    R/W     NFC Sub Page Configuration Register
} NFC_V2_T, *PNFC_V2_T;

typedef struct _ECC_V2_T
{
	volatile unsigned int   ECC_CTRL;				// 0x00     W       NAND Flash Command Register
	volatile unsigned int   ECC_ERRSTATUS;			// 0x04     W       NAND Flash Linear Address Register
	volatile unsigned int   ECC_BASE;				// 0x08			 Reserved
	volatile unsigned int   ECC_MASK;				// 0x0C			 Reserved
	volatile unsigned int   ECC_CLEAR;
	volatile unsigned int   ECC_RSTART;
	volatile unsigned int   ECC_STLDCTL;
	volatile unsigned int   ECC_CSTATE;
	volatile unsigned int	ECC_CODE0;				// 0x220  R/W	1st  ECC Code Register
	volatile unsigned int	ECC_CODE1;				// 0x224  R/W	2nd  ECC Code Register
	volatile unsigned int	ECC_CODE2;				// 0x228  R/W	3rd  ECC Code Register
	volatile unsigned int	ECC_CODE3;				// 0x22C  R/W	4th  ECC Code Register
	volatile unsigned int	ECC_CODE4;				// 0x230  R/W	5th  ECC Code Register
	volatile unsigned int	ECC_CODE5;				// 0x234  R/W	6th  ECC Code Register
	volatile unsigned int	ECC_CODE6;				// 0x238  R/W	7th  ECC Code Register
	volatile unsigned int	ECC_CODE7;				// 0x23C  R/W	8th  ECC Code Register
	volatile unsigned int	ECC_CODE8;				// 0x240  R/W	9th  ECC Code Register
	volatile unsigned int	ECC_CODE9;				// 0x244  R/W	10th  ECC Code Register
	volatile unsigned int	ECC_CODE10; 			// 0x248  R/W	11th  ECC Code Register
	volatile unsigned int	ECC_CODE11; 			// 0x24C  R/W	12th  ECC Code Register
	volatile unsigned int	ECC_CODE12; 			// 0x250  R/W	13th  ECC Code Register
	volatile unsigned int	ECC_CODE13; 			// 0x254  R/W	14th  ECC Code Register
	volatile unsigned int	ECC_CODE14; 			// 0x258  R/W	15th  ECC Code Register
	volatile unsigned int	ECC_CODE15; 			// 0x25C  R/W	16th  ECC Code register
	volatile unsigned int	ECC_EADDR[24]; 			// 0x260  R  	ECC Error Address Register0	
	volatile unsigned int	NOTDEFINE[848];			// 0x2C0 ~ 0xFFF
	volatile unsigned int	NFC_MEMORY[384];		// 0x1000 ~ 0x17FF NFC MEMORY for Spare Area [NDMA Designation Memory]
} ECC_V2_T, *PECC_V2_T;
#endif

#if defined(NFC_VER_300)
typedef struct _NFC_V3_T
{
	volatile unsigned int   NFC_CMD;                // 0x00     W       NAND Flash Command Register
	volatile unsigned int   NFC_LADDR;              // 0x04     W       NAND Flash Linear Address Register
	volatile unsigned int   NFC_SADDR;              // 0x08     W       NAND Flash Signal Address Register
	volatile unsigned int   NFC_SDATA;              // 0x0C     R/W     NAND Flash Single Data Register
	volatile unsigned int   NFC_WDATA;           	// 0x1x     R/W     NAND Flash Word Data Register
	volatile unsigned int	NOTDEFINE0[3];
	volatile unsigned int   NFC_LDATA;           	// 0x2x/3x R/W      NAND Flash Linear Data Register
	volatile unsigned int	NOTDEFINE1[7];			// 0x01x  R/W	NAND Flash Word Data Register
	volatile unsigned int   NFC_MDATA;              // 0x40     R/W     NAND Flash Multiple Data Register
	volatile unsigned int   NFC_CACYC;              // 0x44     R/W     NAND Flash Command & Address Cycle Register
	volatile unsigned int   NFC_WRCYC;              // 0x48     R/W     NAND Flash Write Cycle Register
	volatile unsigned int   NFC_RECYC;              // 0x4C     R/W     NAND Flash Read Cycle Register
	volatile unsigned int   NFC_CTRL;               // 0x50     R/W     NFC Control Register
	volatile unsigned int   NFC_DSIZE;              // 0x54     R/W     NFC Data Size Register
	volatile unsigned int   NFC_SPARE;              // 0x58     R/W     NFC Spare Burst Transfer Control Register
	volatile unsigned int   NFC_PRSTART;            // 0x5C     R/W     NFC Burst Program/Read Start
	volatile unsigned int   NFC_RST;                // 0X60     R/W     NFC Reset Register
	volatile unsigned int   NFC_IRQCFG;             // 0x64     R/W     NFC Interrupt Request Control Register
	volatile unsigned int   NFC_IRQ;                // 0x68     R/W     NFC Interrupt Request Register
	volatile unsigned int   NFC_STATUS;             // 0x6C     R/W     NFC Stutus Register
	volatile unsigned int   NFC_RFWBASE;            // 0x70     R/W     NFC Read Flag Wait Base Word Register
	volatile unsigned int	NFC_RAND_CTRL;			// 0x74		R/W		NFC Randomizer Control Register
	volatile unsigned int	NFC_RAND_STATUS;		// 0x78		R/W		NFC Randomizer Status Register
	volatile unsigned int	NFC_DDR_CTRL;			// 0x7C		R/W		NFC DDR NAND Control Register
	volatile unsigned int	NFC_DDR_CYCLE;			// 0x80		R/W		NFC DDR NAND Cycle Register
	volatile unsigned int   NOTDEFINE2[31];         // 0x84~0xFC
	volatile unsigned int   NDMA_ADDR;              // 0X100    R/W     NFC DMA Source/Destination Register
	volatile unsigned int   NDMA_PARAM;             // 0X104    R/W     NFC DMA Parameter Register
	volatile unsigned int   NDMA_CADDR;             // 0X108    R       NFC DMA Current Address Register
	volatile unsigned int   NDMA_CTRL;              // 0x10C    R/W     NFC DMA Controll Register
	volatile unsigned int   NDMA_SPCTRL;            // 0x110    R/W     NFC DMA SUBPAGE Control Regiter
	volatile unsigned int   NDMA_STATUS;            // 0x114    R/W     NFC DMA Stutus Register
	volatile unsigned int   NOTDEFINE3[2];          // 0x118~011C
	volatile unsigned int   SP_CFG[32];             // 0x120    R/W     NFC Sub Page Configuration Register
} NFC_V3_T, *PNFC_V3_T;

typedef struct _ECC_V3_T
{
	volatile unsigned int   ECC_CTRL;				// 0x00     W       NAND Flash Command Register
	volatile unsigned int   ECC_ERRSTATUS;			// 0x04     W       NAND Flash Linear Address Register
	volatile unsigned int   ECC_BASE;				// 0x08			 Reserved
	volatile unsigned int   ECC_MASK;				// 0x0C			 Reserved
	volatile unsigned int   ECC_CLEAR;
	volatile unsigned int   ECC_RSTART;
	volatile unsigned int   ECC_STLDCTL;
	volatile unsigned int   ECC_CSTATE0;
	volatile unsigned int   ECC_CSTATE1;
	volatile unsigned int	NOTDEFINE1[3];
	volatile unsigned int	ECC_CODE0;				// 0x230  R/W	1st  ECC Code Register
	volatile unsigned int	ECC_CODE1;				// 0x234  R/W	2nd  ECC Code Register
	volatile unsigned int	ECC_CODE2;				// 0x238  R/W	3rd  ECC Code Register
	volatile unsigned int	ECC_CODE3;				// 0x23C  R/W	4th  ECC Code Register
	volatile unsigned int	ECC_CODE4;				// 0x240  R/W	5th  ECC Code Register
	volatile unsigned int	ECC_CODE5;				// 0x244  R/W	6th  ECC Code Register
	volatile unsigned int	ECC_CODE6;				// 0x248  R/W	7th  ECC Code Register
	volatile unsigned int	ECC_CODE7;				// 0x24C  R/W	8th  ECC Code Register
	volatile unsigned int	ECC_CODE8;				// 0x250  R/W	9th  ECC Code Register
	volatile unsigned int	ECC_CODE9;				// 0x254  R/W	10th  ECC Code Register
	volatile unsigned int	ECC_CODE10; 			// 0x258  R/W	11th  ECC Code Register
	volatile unsigned int	ECC_CODE11; 			// 0x25C  R/W	12th  ECC Code Register
	volatile unsigned int	ECC_CODE12; 			// 0x260  R/W	13th  ECC Code Register
	volatile unsigned int	ECC_CODE13; 			// 0x264  R/W	14th  ECC Code Register
	volatile unsigned int	ECC_CODE14; 			// 0x268  R/W	15th  ECC Code Register
	volatile unsigned int	ECC_CODE15; 			// 0x26C  R/W	16th  ECC Code register
	volatile unsigned int	ECC_CODE16; 			// 0x270  R/W	11th  ECC Code Register
	volatile unsigned int	ECC_CODE17; 			// 0x274  R/W	12th  ECC Code Register
	volatile unsigned int	ECC_CODE18; 			// 0x278  R/W	13th  ECC Code Register
	volatile unsigned int	ECC_CODE19; 			// 0x27C  R/W	14th  ECC Code Register
	volatile unsigned int	ECC_CODE20; 			// 0x280  R/W	15th  ECC Code Register
	volatile unsigned int	ECC_CODE21; 			// 0x284  R/W	16th  ECC Code register
	volatile unsigned int	ECC_CODE22; 			// 0x288  R/W	11th  ECC Code Register
	volatile unsigned int	ECC_CODE23; 			// 0x28C  R/W	12th  ECC Code Register
	volatile unsigned int	ECC_CODE24; 			// 0x290  R/W	13th  ECC Code Register
	volatile unsigned int	ECC_CODE25; 			// 0x294  R/W	14th  ECC Code Register
	volatile unsigned int	ECC_CODE26; 			// 0x298  R/W	15th  ECC Code Register
	volatile unsigned int	NOTDEFINE2[1];				// 0x29C  NULL
	volatile unsigned int	ECC_EADDR[60]; 			// 0x2A0 ~ 0x390 R  	ECC Error Address Register
	#if defined(TCC8925S) || defined(TCC893X)
	volatile unsigned int	NOTDEFINE3[(0x400-0x390)/4];			// 0x400  ~  0x13FF 
	#else
	#error 
	#endif
	volatile unsigned int	NFC_MEMORY[1024];		// 0x1000 ~ 0x17FF NFC MEMORY for Spare Area [NDMA Designation Memory]
} ECC_V3_T, *PECC_V3_T;
#endif
#endif //_NAND_REGS_H