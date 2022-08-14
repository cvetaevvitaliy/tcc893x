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

#include "os.h"
        
void OS_SleepMs(UINT32 u32Msec)
{
	#if 0
	if ( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING ) {
	    // we can use the scheduler to do the delay
	    vTaskDelay((portTickType)u32Msec/portTICK_RATE_MS);
	}
	else
	{
	    // we have to use the CPU waisting methode
	    while ( u32Msec-- )
	    {
	        OS_SleepUs(1000);
	    }
	}
	#endif
}

void OS_SleepUs(UINT32 u32Usec) //range: 0..1000us via OS_ASSERT()
{
	//OS_ASSERT( u32Usec <= 1000 );

	//LowLevelSleepUs(u32Usec);
		{
			int i;
			for (i=0; i<u32Usec*0x100; i++)	{	/*__asm { nop; }*/	}
		}
}

UINT32 OS_GetTick(void)
{
    //return xTaskGetTickCount();
	return 0;
}

UINT32 OS_GetFrequency(void)
{
    //return configTICK_RATE_HZ;
	return 200000000;	//200mhz
}

