/****************************************************************************
 *   FileName    : tcc317x_mailbox.c
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

#include "tcc317x_mailbox.h"
#include "tcc317x_register_control.h"
#include "tcpal_os.h"

extern TcpalSemaphore_t Tcc317xMailboxSema[TCC317X_MAX][TCC317X_MAX];
extern TcpalSemaphore_t Tcc317xOpMailboxSema[TCC317X_MAX][TCC317X_MAX];

#define MAXWAIT_MAILBOX 1000     /* about 1sec */
/* #define _USE_MAILBOX_DEBUG_MONITOR_ */

static I32U Tcc317xMailboxStatus (I32U input, I32U WFlag)
{
    static I32U stat;

    if (WFlag)
        stat = input;
    else
        return stat;

    return stat;
}

static void Tcc317xMailboxTx (Tcc317xHandle_t * _handle, I32S rw_flag, I32U cmd, I32U * data_array, I32U word_cnt,
                              I32S * pmailboxok)
{
    I32U      temp;
    I32U      i;
    I08U      wstat;
    TcpalTime_t CurrTime;

    pmailboxok[0] = 1;
    Tcc317xSetRegMailboxControl (_handle, TC3XREG_MAIL_INIT);

    temp = (MB_HOSTMAIL << 24) | (word_cnt << 20) | (rw_flag << 19) | (MB_ERR_OK << 16) | cmd;

    Tcc317xSetRegMailboxFifoWindow (_handle, (I08U *) (&temp), 4);

    for (i = 0; i < (unsigned int) word_cnt; i++)
    {
        Tcc317xSetRegMailboxFifoWindow (_handle, (I08U *) (&data_array[i]), 4);
    }

    Tcc317xMailboxStatus (cmd, 1);
    Tcc317xSetRegMailboxControl (_handle, TC3XREG_MAIL_HOSTMAILPOST);

    CurrTime = TcpalGetCurrentTimeCount_ms ();

    do
    {
        if (!(_handle->sysEnValue & TC3XREG_SYS_EN_DSP))        /* Exceptional Error */
        {
            TcpalPrintErr ((I08S*)"# [Error] MailBox - OP Disabled!!! \n");
            return;
        }

        if (TcpalGetTimeIntervalCount_ms (CurrTime) > MAXWAIT_MAILBOX)
        {
#if defined (_USE_MAILBOX_DEBUG_MONITOR_)
            I08U op_debug[3];
	    Tcc317xGetRegOpDebug (_handle, &op_debug[0], 3);
	    TcpalPrintErr((I08S *)
			  "[Error] MailBox Read Timeout[OP:0x%02x%02x%02x]!!! \n",
			  op_debug[0], op_debug[1],
			  op_debug[2]);
#else
  	    TcpalPrintErr ((I08S*)"# [Error] MailBox Write Timeout!!! \n");
#endif
            pmailboxok[0] = -1;
            return;
        }
        TcpalSleep (0);
        Tcc317xGetRegMailboxFifoWriteStatus (_handle, &wstat);
    }
    while (!(wstat & 0x01));
}

static I32S Tcc317xMailboxRx (Tcc317xHandle_t * _handle, mailbox_t * p_mailbox, I32U warmboot, I32S * pmailboxok)
{
    I08U      temp;
    I32S      i;
    I32S      total_word_num;
    I32S      total_byte_num;
    I32U      cmd;
    TcpalTime_t CurrTime;

    pmailboxok[0] = 1;

    i = 0;
    CurrTime = TcpalGetCurrentTimeCount_ms ();

    do
    {
        if (!(_handle->sysEnValue & TC3XREG_SYS_EN_DSP))        /* Exceptional Error */
        {
            TcpalPrintErr ((I08S*)"# [Error] MailBox - OP Disabled!!! \n");
            return -1;
        }

        Tcc317xGetRegMailboxFifoReadStatus (_handle, &temp);

        if (TcpalGetTimeIntervalCount_ms (CurrTime) > MAXWAIT_MAILBOX)
        {
#if defined (_USE_MAILBOX_DEBUG_MONITOR_)
	    I08U op_debug[3];
	    Tcc317xGetRegOpDebug (_handle, &op_debug[0], 3);
	    TcpalPrintErr((I08S *)
			  "[Error] MailBox Read Timeout[OP:0x%02x%02x%02x]!!! \n",
			  op_debug[0], op_debug[1],
			  op_debug[2]);
#else
            pmailboxok[0] = -1;
            TcpalPrintErr ((I08S*)"# [Error] MailBox Read Timeout!!! \n");
#endif
            return -1;
        }
        TcpalSleep (0);
    }
    while ((temp & 0xfc) < 3);

    if (warmboot)
    {
        Tcc317xGetRegMailboxFifoWindow (_handle, (I08U *) (&p_mailbox->data_array[0]), 1 << 2);
        return 0;
    }

    total_byte_num = (temp >> 2) & 0x3f;
    total_word_num = (total_byte_num >> 2);

    /* LSB First */
    Tcc317xGetRegMailboxFifoWindow (_handle, (I08U *) (&cmd), 1 << 2);
    Tcc317xGetRegMailboxFifoWindow (_handle, (I08U *) (&p_mailbox->data_array[0]), (total_word_num - 1) << 2);

    /* mark check */
    if ((cmd >> 24) != MB_SLAVEMAIL)
    {
        I32U      mailstat;

        TcpalPrintErr ((I08S*)"# Mailbox Error Cmd[0x%x] Total Byte Num[0x%x]\n", cmd, total_byte_num);

        for (i = 0; i < total_word_num - 1; i++)
        {
            TcpalPrintErr ((I08S*)"# data_array[%d][0x%x]\n", i, p_mailbox->data_array[i]);
        }

        mailstat = Tcc317xMailboxStatus (cmd, 0);
        TcpalPrintErr ((I08S*)"# [MBERR]Mark Error[0x%x][ori:0x%x]\n", cmd, mailstat);
        TcpalPrintErr ((I08S*)"# [MBERR]please check the header.\n");
        pmailboxok[0] = -1;
        return -1;
    }

    p_mailbox->cmd = cmd & 0xffff;
    p_mailbox->word_cnt = total_word_num - 1;
    p_mailbox->status = ((cmd >> 16) & 0x07);

    if (p_mailbox->status != 0)
    {
        TcpalPrintErr ((I08S*)"# [MBERR] Error Message : 0x%0x[cmd 0x%x][totalnum 0x%x]\n", p_mailbox->status, cmd,
                       total_word_num);
        pmailboxok[0] = -1;
        return -1;
    }

    return total_word_num - 1;
}

static I32S Tcc317xMailboxTxOnlySub (Tcc317xHandle_t * _handle, I32U cmd, I32U * data_array, I32S word_cnt)
{
    I32S      mailboxok;

    TcpalSemaphoreLock (&Tcc317xMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
    Tcc317xMailboxTx (_handle, MB_CMD_WRITE, cmd, data_array, word_cnt, &mailboxok);
    TcpalSemaphoreUnLock (&Tcc317xMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);

    if (mailboxok == -1)
    {
        TcpalPrintErr ((I08S*)"# [ERR] MailboxTX Error.\n");
        return -1;
    }

    return 1;
}

I32S Tcc317xMailboxTxOnly (Tcc317xHandle_t * _handle, I32U cmd, I32U * data_array, I32S word_cnt)
{
    I32S      ret;

    TcpalSemaphoreLock (&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
    if (_handle->mailboxErrorCounter > MAX_MAILBOX_RETRY)
    {
        TcpalPrintErr ((I08S*)"# [M] Critical Mailbox Control Error!!! Can't recover!!!\n");
        _handle->mailboxErrorCounter = 0;
        TcpalSemaphoreUnLock (&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
        return -1;
    }

    ret = Tcc317xMailboxTxOnlySub (_handle, cmd, data_array, word_cnt);

    if (ret == -1)
    {
        /* one more time set and give up */
        TcpalPrintErr ((I08S*)"# [M] Mailbox Retry\n");
        ret = Tcc317xMailboxTxOnlySub (_handle, cmd, data_array, word_cnt);
        if (ret == -1)
        {
            TcpalPrintErr ((I08S*)"# [ERR] Can't recover mailbox. Please Retune!\n");
            _handle->mailboxErrorCounter = 0;
            TcpalSemaphoreUnLock (&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
            return -1;
        }
    }

    _handle->mailboxErrorCounter = 0;
    TcpalSemaphoreUnLock (&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
    return 1;
}

static I32S Tcc317xMailboxTxRxSub (Tcc317xHandle_t * _handle, mailbox_t * p_mailbox, I32U cmd, I32U * data_array,
                                   I32S word_cnt, I32S warmboot)
{
    I32S      ret = 0;
    I32S      mailboxok;

    TcpalSemaphoreLock(&Tcc317xMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);

    if (cmd == MBPARA_SYS_WARMBOOT)
    {
        Tcc317xMailboxTx (_handle, MB_CMD_WRITE, cmd, data_array, word_cnt, &mailboxok);
    }
    else
    {
        Tcc317xMailboxTx (_handle, MB_CMD_READ, cmd, data_array, word_cnt, &mailboxok);
    }

    if (mailboxok == -1)
    {
        TcpalSemaphoreUnLock(&Tcc317xMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
        TcpalPrintErr ((I08S*)"# [ERR] MailboxTXRX Error. cmd[%x]\n", cmd);
        return -1;
    }
    else
    {
        ret = Tcc317xMailboxRx (_handle, p_mailbox, warmboot, &mailboxok);

        if (mailboxok == -1)
        {
            TcpalSemaphoreUnLock(&Tcc317xMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
            TcpalPrintErr ((I08S*)"# [ERR] MailboxTXRX Error. cmd[%x]\n", cmd);
            return -1;
        }
    }
    TcpalSemaphoreUnLock(&Tcc317xMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
    return ret;
}

I32S Tcc317xMailboxTxRx (Tcc317xHandle_t * _handle, mailbox_t * p_mailbox, I32U cmd, I32U * data_array, I32S word_cnt,
                         I32S warmboot)
{
    I32S      ret;

    TcpalSemaphoreLock (&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
    if (_handle->mailboxErrorCounter > MAX_MAILBOX_RETRY)
    {
        TcpalPrintErr ((I08S*)"# [M] Critical Mailbox Control Error!!! Can't recover!!!\n");
        _handle->mailboxErrorCounter = 0;
        TcpalSemaphoreUnLock (&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
        return -1;
    }

    ret = Tcc317xMailboxTxRxSub (_handle, p_mailbox, cmd, data_array, word_cnt, warmboot);

    if (ret == -1)
    {
        /* one more time set and give up */
        TcpalPrintErr ((I08S*)"# [M] Mailbox Retry\n");
        ret = Tcc317xMailboxTxRxSub (_handle, p_mailbox, cmd, data_array, word_cnt, warmboot);
        if (ret == -1)
        {
            TcpalPrintErr ((I08S*)"# [ERR] Can't recover mailbox. Please Retune!\n");
            _handle->mailboxErrorCounter = 0;
            TcpalSemaphoreUnLock (&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
            _handle->EventCallBack(_handle->moduleIndex, 
                                   _handle->diversityIndex, 
                                   TCC317X_EVENT_MAILBOX_FAIL);
            return -1;
        }
    }

    _handle->mailboxErrorCounter = 0;
    TcpalSemaphoreUnLock (&Tcc317xOpMailboxSema[_handle->moduleIndex][_handle->diversityIndex]);
    return ret;
}

I32U Tcc317xGetAccessMail (Tcc317xHandle_t * _handle)
{
    I32U      access_mail;

    Tcc317xGetRegMailboxFifoWindow (_handle, (I08U *) (&access_mail), 1 << 2);
    TcpalPrintLog ((I08S*)"# [%d][%d] AccessMail[0x%08x]\n", _handle->moduleIndex, _handle->diversityIndex, access_mail);
    return access_mail;
}

