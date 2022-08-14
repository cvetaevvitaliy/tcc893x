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

#include "tcc_avn_proc.h"
#include "adec.h"

#undef adec_dbg
#if 0
#define adec_dbg(f, a...)  printk("== adec_proc == " f, ##a)
#else
#define adec_dbg(f, a...)
#endif


static int CM3_AVN_TEST(int time_interval)
{

    adec_dbg("\n%s:%d\n",__func__, __LINE__);
	ARG_CM3_CMD InArg;
	ARG_CM3_CMD OutArg;	

    InArg.uiCmdId = HW_TIMER_TEST;
	InArg.uiParam0 = time_interval;

	memset(&OutArg, 0x00, sizeof(OutArg));
    CM3_SEND_COMMAND(&InArg, &OutArg);

    return OutArg.uiParam0;
}

int CM3_AVN_Proc(unsigned long arg)
{
    t_cm3_avn_cmd avn_cmd;
	int ret = -1;

    copy_from_user(&avn_cmd, (t_cm3_avn_cmd*)arg, sizeof(t_cm3_avn_cmd));
    printk("input: iOpCode(0x%x), pParam1(0x%x), pParam2(0x%x)\n", avn_cmd.iOpCode, avn_cmd.pParam1, avn_cmd.pParam2);

	switch(avn_cmd.iOpCode)
	{
		case HW_TIMER_TEST:
		{
	        ret = CM3_AVN_TEST((int)(avn_cmd.pParam1));						
		}
			break;
		default:
			break;
	}

	printk("CM3_AVN_Proc ret : 0x%x\n", ret);

	return ret;	
}

