/****************************************************************************
One line to give the program's name and a brief idea of what it does.
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

#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <mach/bsp.h>

#include "tcc_adec_proc.h"
#include "adec.h"

#undef adec_dbg
#if 0
#define adec_dbg(f, a...)  printk("== adec_proc == " f, ##a)
#else
#define adec_dbg(f, a...)
#endif


static int ADEC_Init(ADEC_Init_InParam* inParam, ADEC_Init_OutParam* outParam)
{
    ARG_ADEC_CMD_IN InArg;
    ARG_ADEC_CMD_OUT OutArg;
	ADEC_Init_InParam *param;

    adec_dbg("\n%s:%d\n",__func__, __LINE__);
		
    memset(&InArg, 0x00, sizeof(ARG_CM3_CMD));
    memset(&OutArg, 0x00, sizeof(ARG_CM3_CMD));

    InArg.tInit.uiCmdId = HW_ADEC_INIT;

	memcpy(&InArg.tInit.inParam, inParam, sizeof(ADEC_Init_InParam));
    CM3_SEND_COMMAND(&InArg, &OutArg);
	memcpy(outParam, &OutArg.tInit.outParam, sizeof(ADEC_Init_OutParam));

    return OutArg.tInit.ret;
}

static int ADEC_Decode(ADEC_Decode_InParam* inParam, ADEC_Decode_OutParam* outParam)
{
    ARG_ADEC_CMD_IN InArg;
    ARG_ADEC_CMD_OUT OutArg;
	void * param;
    volatile unsigned int * pInputMem = (volatile unsigned int *)tcc_p2v(HwCORTEXM3_CODE_MEM_BASE);
    volatile unsigned int * pOutputMem = (volatile unsigned int *)tcc_p2v(HwCORTEXM3_DATA_MEM_BASE);

    adec_dbg("\n%s:%d\n",__func__, __LINE__);
		
    memset(&InArg, 0x00, sizeof(ARG_CM3_CMD));
    memset(&OutArg, 0x00, sizeof(ARG_CM3_CMD));

    InArg.tDecode.uiCmdId= HW_ADEC_DECODE;


	InArg.tDecode.inParam.uiStreamSize = inParam->uiStreamSize;
	InArg.tDecode.inParam.pStreamBuff = HwCORTEXM3_CODE_MEM_BASE;
	InArg.tDecode.inParam.pPcmBuff = HwCORTEXM3_DATA_MEM_BASE;

	copy_from_user(pInputMem, inParam->pStreamBuff, inParam->uiStreamSize);
    CM3_SEND_COMMAND(&InArg, &OutArg);
	memcpy(outParam, &OutArg.tDecode.outParam, sizeof(ADEC_Decode_OutParam));

	copy_to_user(inParam->pPcmBuff, pOutputMem, outParam->uiNumberOfChannel*outParam->uiSamplesPerChannel*sizeof(short));

    return OutArg.tDecode.ret;
}

static int ADEC_Close(void)
{
    ARG_CM3_CMD InArg;
    ARG_CM3_CMD OutArg;

    adec_dbg("\n%s:%d\n",__func__, __LINE__);
		
    memset(&InArg, 0x00, sizeof(ARG_CM3_CMD));
    memset(&OutArg, 0x00, sizeof(ARG_CM3_CMD));

    InArg.uiCmdId= HW_ADEC_CLOSE;
	
    CM3_SEND_COMMAND(&InArg, &OutArg);

    return OutArg.uiParam0;
}

static int ADEC_Flush(void)
{
    ARG_CM3_CMD InArg;
    ARG_CM3_CMD OutArg;

    adec_dbg("\n%s:%d\n",__func__, __LINE__);
		
    memset(&InArg, 0x00, sizeof(ARG_CM3_CMD));
    memset(&OutArg, 0x00, sizeof(ARG_CM3_CMD));

    InArg.uiCmdId= HW_ADEC_FLUSH;
	
    CM3_SEND_COMMAND(&InArg, &OutArg);

    return OutArg.uiParam0;
}


int ADEC_Proc(unsigned long arg)
{
    t_cm3_adec_cmd adec_cmd;
	int ret = -1;

    copy_from_user(&adec_cmd, (t_cm3_adec_cmd*)arg, sizeof(t_cm3_adec_cmd));
//    printk("input: iOpCode(0x%x), pParam1(0x%x), pParam2(0x%x)\n", adec_cmd.iOpCode, adec_cmd.pParam1, adec_cmd.pParam2);

	switch(adec_cmd.iOpCode)
	{
		case AUDIO_INIT:
		{
			ADEC_Init_InParam sADecInParam;
			ADEC_Init_OutParam sADecOutParam;
	        copy_from_user(&sADecInParam, adec_cmd.pParam1, sizeof(ADEC_Init_InParam));
	        ret = ADEC_Init(&sADecInParam, &sADecOutParam);						
	        copy_to_user(adec_cmd.pParam2, &sADecOutParam, sizeof(ADEC_Init_OutParam));      
		}
			break;
		case AUDIO_DECODE:
		{
			ADEC_Decode_InParam sADecInParam;
			ADEC_Decode_OutParam sADecOutParam;

	        copy_from_user(&sADecInParam, adec_cmd.pParam1, sizeof(ADEC_Decode_InParam));
	        ret = ADEC_Decode(&sADecInParam, &sADecOutParam);						
	        copy_to_user(adec_cmd.pParam2, &sADecOutParam, sizeof(ADEC_Decode_OutParam));      
		}
			break;
		case AUDIO_CLOSE:
		{
	        ret = ADEC_Close();						
		}
			break;
		case AUDIO_FLUSH:
		{
	        ret = ADEC_Flush();						
		}
			break;
		default:
			break;
	}

	return ret;	
}

