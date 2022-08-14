/****************************************************************************
 *   FileName    : tcc317x_mailbox.h
 *   Description : mailbox control Function
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

#ifndef __TCC317X_MAILBOX_H__
#define __TCC317X_MAILBOX_H__

#include "tcc317x_common.h"
#include "tcc317x_core.h"

#define MAX_MAILBOX_RETRY (2)   /* continuously */

#define MB_HOSTMAIL		0x47
#define MB_SLAVEMAIL		0x74

#define MB_ERR_OK		0x00
#define MB_ERR_CMD		0x01
#define MB_ERR_PARA		0x02

#define MBCMD_SYS				(0x00<<11)
#define MBPARA_SYS_WARMBOOT 			(MBCMD_SYS | 0x00)
#define MBPARA_SYS_START 			(MBCMD_SYS | 0x01)
#define MBPARA_SYS_SYNC 			(MBCMD_SYS | 0x02)

#define MBPARA_SYS_DAB_STREAM_SET 		(MBCMD_SYS | 0x0D)
#define MBPARA_SYS_DAB_MCI_UPDATE 		(MBCMD_SYS | 0x0c)
#define MBPARA_SYS_DAB_DP_FLT 			(MBCMD_SYS | 0x0f)
#define MBPARA_SYS_DAB_RESYNC_PARAM 		(MBCMD_SYS | 0x10)
#define MBPARA_SYS_DAB_RESYNC_RESULT 		(MBCMD_SYS | 0x11)
#define MBPARA_SYS_DAB_RF_FREQ_NO 		(MBCMD_SYS | 0x20)
#define MBPARA_SYS_DAB_RF_FREQ 			(MBCMD_SYS | 0x21)
#define MBPARA_SYS_DAB_ASM_VER 			(MBCMD_SYS | 0xff)

#define MBCMD_AGC_DAB				(0x01<<11)
#define MBCMD_AGC_DAB_CFG			(MBCMD_AGC_DAB | 0)
#define MBCMD_AGC_DAB_GAIN			(MBCMD_AGC_DAB | 1)
#define MBCMD_AGC_DAB_JAM			(MBCMD_AGC_DAB | 2)
#define MBCMD_AGC_DAB_3				(MBCMD_AGC_DAB | 3)
#define MBCMD_AGC_DAB_4				(MBCMD_AGC_DAB | 4)
#define MBCMD_AGC_DAB_5				(MBCMD_AGC_DAB | 5)
#define MBCMD_AGC_DAB_6				(MBCMD_AGC_DAB | 6)
#define MBCMD_AGC_DAB_7				(MBCMD_AGC_DAB | 7)
#define MBCMD_AGC_DAB_8				(MBCMD_AGC_DAB | 8)
#define MBCMD_AGC_DAB_9				(MBCMD_AGC_DAB | 9)
#define MBCMD_AGC_DAB_A				(MBCMD_AGC_DAB | 10)
#define MBCMD_AGC_DAB_B				(MBCMD_AGC_DAB | 11)
#define MBCMD_AGC_DAB_C				(MBCMD_AGC_DAB | 12)
#define MBCMD_AGC_DAB_D				(MBCMD_AGC_DAB | 13)
#define MBCMD_AGC_DAB_E				(MBCMD_AGC_DAB | 14)
#define MBCMD_AGC_DAB_F				(MBCMD_AGC_DAB | 15)

#define MBCMD_FP_DAB				(0x02 << 11)
#define MBCMD_FP_DAB_CFG			(MBCMD_FP_DAB | 0)
#define MBCMD_FP_DAB_IIR			(MBCMD_FP_DAB | 1)

#define MBCMD_TII				(0x03 << 11)
#define MBCMD_TII_CFG				(MBCMD_TII | 0x01)
#define MBCMD_TII_RESULT_1			(MBCMD_TII | 0x02)
#define MBCMD_TII_RESULT_2			(MBCMD_TII | 0x03)

#define MBCMD_CTO_DAB				(0x04<<11)
#define MBCMD_CTO_DAB_PARAM			(MBCMD_CTO_DAB | 0x00)
#define MBCMD_CTO_DAB_RESULT			(MBCMD_CTO_DAB | 0x01)

#define MBCMD_CFO_DAB				(0x06<<11)
#define MBCMD_CFO_DAB_PARAM			(MBCMD_CFO_DAB | 0x00)
#define MBCMD_CFO_DAB_RESULT			(MBCMD_CFO_DAB | 0x01)

#define MBCMD_FFO_DAB				(0x07<<11)
#define MBCMD_FTO_DAB				(0x0A<<11)

#define MBCMD_SFE_DAB				(0x0B<<11)
#define MBCMD_SFE_DAB_PARAM			(MBCMD_SFE_DAB | 0x00)
#define MBCMD_SFE_DAB_RESULT			(MBCMD_SFE_DAB | 0x01)

#define MBCMD_CIR_DAB				(0x0C<<11)
#define MBCMD_CIR_DAB_PARAM			(MBCMD_CIR_DAB | 0x00)
#define MBCMD_CIR_DAB_RESULT			(MBCMD_CIR_DAB | 0x01)

#define MBCMD_DP_DAB				(0x10<<11)
#define MBCMD_DP_DAB_RESULT			(MBCMD_DP_DAB | 0x00)
#define MBCMD_DP_DAB_CFG1			(MBCMD_DP_DAB | 0x01)
#define MBCMD_DP_DAB_CFG2			(MBCMD_DP_DAB | 0x02)

#define MBCMD_DEBUG_DAB				(0x1F<<11)

#define MBPARA_SYS_VERSION 	        	(MBCMD_SYS | 0xFF)

/* Freqset */
#define MBPARA_SYS_NUM_FREQ 			(MBCMD_SYS | 0x20)
#define MBPARA_SYS_FREQ_0_6    			(MBCMD_SYS | 0x21)
#define MBPARA_SYS_FREQ_7_13   			(MBCMD_SYS | 0x22)
#define MBPARA_SYS_FREQ_14_20  			(MBCMD_SYS | 0x23)
#define MBPARA_SYS_FREQ_21_27  			(MBCMD_SYS | 0x24)

#define MBCMD_SEL_CH_INFO			(0x13<<11)
#define MBPARA_SEL_CH_INFO_PRAM 		(MBCMD_SEL_CH_INFO | 0x00)
#define MBPARA_SEL_CH_INFO_RESULT 		(MBCMD_SEL_CH_INFO | 0x01)

#define MBCMD_VARI_OSC				(0x15<<11)
#define MBCMD_VARI_OSC_SPUR 	    		(MBCMD_VARI_OSC | 0x00)
#define MBCMD_VARI_OSC_RCSTEP 	    		(MBCMD_VARI_OSC | 0x01)
#define MBCMD_VARI_OSC_RFPLL 	    		(MBCMD_VARI_OSC | 0x02)

/*
    Mailbox protocol
    dir : 8bit
    size : 4bit
    RW
    error/ok : 12bit
    cmd : 5bit
    param : 3bit
*/

#define MB_CMD_READ	0
#define MB_CMD_WRITE	1

I32S Tcc317xMailboxTxOnly (Tcc317xHandle_t * _handle, I32U cmd, I32U *data_array, I32S word_cnt);
I32S Tcc317xMailboxTxRx (Tcc317xHandle_t * _handle, mailbox_t * p_mailbox, I32U cmd, I32U *data_array, I32S word_cnt, I32S warmboot);
I32U Tcc317xGetAccessMail (Tcc317xHandle_t * _handle);

#endif
