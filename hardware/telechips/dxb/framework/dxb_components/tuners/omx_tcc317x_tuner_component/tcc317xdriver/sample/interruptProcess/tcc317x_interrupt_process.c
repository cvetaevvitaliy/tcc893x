/****************************************************************************
 *   FileName    : tcc317x_interrupt_process.c
 *   Description : TCC317X Interrupt Process
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

#define _PleaseInsertYourCode_

#include "tcc317x_common.h"
#include "tcc317x_api.h"
#include "tcpal_os.h"
#include "tcc317x_stream_process.h"

#if defined (USE_TELECHIPS_TCC79_MUSE_NUCLEUS_ADAPT)
#include "tcc317x_muse.h"

extern void TC317X_IO_BB_INT_Ctrl (I32S _moduleIndex, I32S _command);
extern I32S TC317X_IO_QUE_Receive (void *queue, void *message, I32U suspend);

extern void      *m_TC3X_QUE_INTERRUPT0;
extern void      *m_TC3X_QUE_INTERRUPT1;

#endif

I08U Tcc317xStreamData[2][TCC317X_STREAM_THRESHOLD];

void Tcc317xInterruptTask ()
{
    /* INT Level Mode */
    I08U irqErr = 0;
    I32S moduleIndex = 0;
    #if defined (USE_TELECHIPS_TCC79_MUSE_NUCLEUS_ADAPT)
    I32U key;
	MsgInterruptCommand_t *msg;
    #endif

	for (;;)
	{
        /* wait interrupt handler queue */
        #if defined (USE_TELECHIPS_TCC79_MUSE_NUCLEUS_ADAPT)
        if(m_TC3X_QUE_INTERRUPT0==NULL)
        {
            continue;
        }
		TC317X_IO_QUE_Receive (m_TC3X_QUE_INTERRUPT0, &key, 0xFFFFFFFFUL);
		msg = (MsgInterruptCommand_t *) key;

		if(msg->usCommand==0xABCD)
		{
		    /* task end */
		    continue;
        }
		if((msg->usCommand & 0xFF00)!=0x3700)
		{
		    /* task skip */
		    continue;
        }
        #else
        _PleaseInsertYourCode_;
        #endif 
	
        /* Read BB Interrupt Status */
        Tcc317xApiGetIrqError(moduleIndex, &irqErr);

        /* Tcc317x IRQ Clear */
        Tcc317xApiIrqClear(moduleIndex);

        /* host interrupt pause */
        #if defined (USE_TELECHIPS_TCC79_MUSE_NUCLEUS_ADAPT)
        TC317X_IO_BB_INT_Ctrl (moduleIndex, CMD_BB_INT_DISABLE);
        #else
        _PleaseInsertYourCode_;
        #endif 

        /* Stream Interrupt */
        if (irqErr)
        {
            TcpalPrintErr ((I08S*)"# FIFO Overflow!!!, %02X!\n", irqErr);
            /* fifo reset */
            Tcc317xApiStreamStop(moduleIndex);
            Tcc317xApiStreamStart(moduleIndex);
        }
        else
        {
            Tcc317xApiStreamRead(moduleIndex, &Tcc317xStreamData[0][0], TCC317X_STREAM_THRESHOLD);
            Tcc317xStreamParsing(moduleIndex, &Tcc317xStreamData[0][0], TCC317X_STREAM_THRESHOLD);
        }

        /* host interrupt start */
        #if defined (USE_TELECHIPS_TCC79_MUSE_NUCLEUS_ADAPT)
        TC317X_IO_BB_INT_Ctrl (moduleIndex, CMD_BB_INT_ENABLE);
        #else
        _PleaseInsertYourCode_;
        #endif 
    }
}

void Tcc317xInterruptTaskDual ()
{
    /* INT Level Mode */
    I08U irqErr = 0;
    I32S moduleIndex = 1;
    #if defined (USE_TELECHIPS_TCC79_MUSE_NUCLEUS_ADAPT)
    I32U key;
	MsgInterruptCommand_t *msg;
    #endif

	for (;;)
	{
        /* wait interrupt handler queue */
        #if defined (USE_TELECHIPS_TCC79_MUSE_NUCLEUS_ADAPT)
		TC317X_IO_QUE_Receive (m_TC3X_QUE_INTERRUPT1, &key, 0xFFFFFFFFUL);
		msg = (MsgInterruptCommand_t *) key;
		if((msg->usCommand & 0xFF00)!=0x3700)
		{
		    /* task end */
		    break;
        }
        #else
        _PleaseInsertYourCode_;
        #endif 
	
        /* Read BB Interrupt Status */
        Tcc317xApiGetIrqError(moduleIndex, &irqErr);

        /* Tcc317x IRQ Clear */
        Tcc317xApiIrqClear(moduleIndex);

        /* host interrupt pause */
        #if defined (USE_TELECHIPS_TCC79_MUSE_NUCLEUS_ADAPT)
        TC317X_IO_BB_INT_Ctrl (moduleIndex, CMD_BB_INT_DISABLE);
        #else
        _PleaseInsertYourCode_;
        #endif 

        /* Stream Interrupt */
        if (irqErr)
        {
            TcpalPrintErr ((I08S*)"# FIFO Overflow!!!, %02X!\n", irqErr);
            /* fifo reset */
            Tcc317xApiStreamStop(moduleIndex);
            Tcc317xApiStreamStart(moduleIndex);
        }
        else
        {
            Tcc317xApiStreamRead(moduleIndex, &Tcc317xStreamData[1][0], TCC317X_STREAM_THRESHOLD);
            Tcc317xStreamParsing(moduleIndex, &Tcc317xStreamData[1][0], TCC317X_STREAM_THRESHOLD);
        }

        /* host interrupt start */
        #if defined (USE_TELECHIPS_TCC79_MUSE_NUCLEUS_ADAPT)
        TC317X_IO_BB_INT_Ctrl (moduleIndex, CMD_BB_INT_ENABLE);
        #else
        _PleaseInsertYourCode_;
        #endif 
    }
}
