/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/

#include "TC3X_IO.h"
#include "TC3X_GPIO.h"
#include "../TC3X_Common.h"

#include <stdlib.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/ioctl.h>
#include "tcc_dxb_control.h"

//#define T801

// -----------------------------------------------------------------
// Resource for TC3X Driver
//

TC_SEMAPHORE TC3X_SEM_IF[MAX_TCC3X];                     // interface semaphore
#if defined (STDDEF_DVBH)
TC_SEMAPHORE TC3X_SEM_RSDECODING[MAX_TCC3X];             // for mpefec schedule semaphore
#endif // STDDEF_DVBH
TC_SEMAPHORE TC3X_SEM_MAILBOX[MAX_TCC3X];                // MailBox semaphore
TC_SEMAPHORE TC3X_SEM_OP_MAILBOX[MAX_TCC3X];           // OP&Mailbox semaphore

#if defined (STDDEF_DVBH)
TC_QUEUE  TC3X_QUE_TIMESLICING[MAX_TCC3X];               // que for time slicing process 
TC_QUEUE  TC3X_QUE_MPEFEC[MAX_TCC3X];                    // que for mpefec process
TC_QUEUE  TC3X_QUE_DVBH_MANAGE[MAX_TCC3X];               // que for mpefec params process, ts mpep
#endif // STDDEF_DVBH
TC_QUEUE  TC3X_QUE_INTERRUPT[MAX_TCC3X];                 // que for interrupt process 

#if defined (STDDEF_DVBH)
TC_TASK   TC3X_TASK_RC_DVBH_MANAGE[MAX_TCC3X];           // task main task (dvb-h service set, mpep...)
TC_TASK   TC3X_TASK_RC_MPEFEC[MAX_TCC3X];                // task mpefec scheduling
TC_TASK   TC3X_TASK_RC_TIMESLICING[MAX_TCC3X];           // task time slicing
#endif // STDDEF_DVBH
TC_TASK   TC3X_TASK_RC_INTERRUPT[MAX_TCC3X];             // task interrupt

//TC_HISR HISR_TC3X_SPI;                                   // hisr for spi
//TC_HISR HISR_TC3X_INT;                                   // hisr for ext2
//#if defined (STDDEF_DVBH)
//TC_HISR HISR_TC3X_TIMER_5M;                              // hisr for time slicing process
//#endif // STDDEF_DVBH

FN_v_ii   fnLISR_SPI = (FN_v_ii) TC3X_IO_UTIL_dummy_function10;
FN_v_i    fnHISR_SPI = (FN_v_i) TC3X_IO_UTIL_dummy_function2;
#if defined (STDDEF_DVBH)
FN_v_i    fnHISR_5mTIMER0 = (FN_v_i) TC3X_IO_UTIL_dummy_function2;
FN_v_i    fnHISR_5mTIMER1 = (FN_v_i) TC3X_IO_UTIL_dummy_function2;
#endif // STDDEF_DVBH
FN_v_i    fnHISR_BBINT0 = (FN_v_i) TC3X_IO_UTIL_dummy_function2;
FN_v_i    fnHISR_BBINT1 = (FN_v_i) TC3X_IO_UTIL_dummy_function2;

FN_v_i    fnLISR_BBINT0 = (FN_v_i) TC3X_IO_UTIL_dummy_function2;
FN_v_i    fnLISR_BBINT1 = (FN_v_i) TC3X_IO_UTIL_dummy_function2;


static int giTccDXBCtrlFD = -1;

// -----------------------------------------------------------------
// Local Define
//
// -----------------------------------------------------------------
// Extern symbol
//
extern void *pTCC3X_Driver0;
extern void *pTCC3X_Driver1;
extern tSTDInfo  g_tSTDInfo[MAX_TCC3X];
extern void *pg_tSTDInfo0;
extern void *pg_tSTDInfo1;


// -----------------------------------------------------------------
// Set Host Prepare Setting Stuff
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_Host_Preset
//    Description : Perform the jobs requested at Host before running Driver.
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_Host_Preset (int moduleidx)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_GET_TIMECNT_us
//    Description : Return current time count
//    Input : 
//    
//    Output : 
//      time count us
//    Remark : 
//--------------------------------------------------------------------------
unsigned int TC3X_IO_GET_TIMECNT_us ()
{
    return 0;
}


#if defined(USE_ANDROID) || defined(USE_LINUX)
long long TC3X_IO_GET_TIMECNT_ms ()
{
    long long tickcount = 0;
    struct timeval tv;           
    gettimeofday (&tv, NULL); 
    tickcount = (long long) tv.tv_sec * 1000 + tv.tv_usec / 1000;     
    return tickcount;
}
#endif

// -----------------------------------------------------------------
// Set 78/79 Int Handler Set
// Linux - Don't care

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_Set_SPIHandler
//    Description : It is required only when using SPI/STS. Connect functions for SPI/STS Interrupt Handler.
//    Input : 
//      moduleidx : module index
//      BB_SPI_SLV_LISR : spi handler lisr
//      BB_SPI_SLV_HISR : spi handler hisr
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_Set_SPIHandler (int moduleidx, FN_v_ii BB_SPI_SLV_LISR, FN_v_i BB_SPI_SLV_HISR)
{
    fnLISR_SPI = BB_SPI_SLV_LISR;
    fnHISR_SPI = BB_SPI_SLV_HISR;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_Set_BBIntHandler
//    Description : Baseband interrupt handler. Connect functions for baseband interrupt handler.
//    Input : 
//      moduleidx : module index
//      BB_INT_LISR : bb interrupt handler lisr
//      BB_INT_HISR : bb interrupt handler hisr
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_Set_BBIntHandler (int moduleidx, FN_v_i BB_INT_LISR, FN_v_i BB_INT_HISR)
{
    switch (moduleidx)
    {
    case 0:
        fnLISR_BBINT0 = BB_INT_LISR;
        fnHISR_BBINT0 = BB_INT_HISR;
        break;
    case 1:
        fnLISR_BBINT1 = BB_INT_LISR;
        fnHISR_BBINT1 = BB_INT_HISR;
        break;
    default:
        PRINTF_LV_0 ("[ERROR] TC3X_IO_Set_BBINTHandler - Please insert code\n");
        break;
    }
}

#if defined (STDDEF_DVBH)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_Set_5mTimerHandler
//    Description : Set 5milli second timer interrupt handler
//    Input : 
//      moduleidx : module index
//      BB_HISR_5mTIMER : 5m timer interrupt hisr handler
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_Set_5mTimerHandler (int moduleidx, FN_v_i BB_HISR_5mTIMER)
{
    switch (moduleidx)
    {
    case 0:
        fnHISR_5mTIMER0 = BB_HISR_5mTIMER;
        break;
    case 1:
        fnHISR_5mTIMER1 = BB_HISR_5mTIMER;
        break;
    default:
        PRINTF_LV_0 ("[ERROR] TC3X_IO_Set_5mTimerHandler - Please insert code\n");
        break;
    }
}
#endif // STDDEF_DVBH

int TCC_GPIO_Open_TC3X(void)
{

	int iBoardType, err;
	ST_CTRLINFO_ARG stCtrl;
	giTccDXBCtrlFD = open(DXB_CTRL_DEV_FILE, O_RDWR | O_NDELAY);
	if(giTccDXBCtrlFD<0)
	{
		 PRINTF_LV_0 ("%s CANNOT open %s :: %d\n", __func__, DXB_CTRL_DEV_FILE, giTccDXBCtrlFD);
		return 1;
	}	
	iBoardType = BOARD_DXB_TCC3510;
	err = ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_SET_BOARD, &iBoardType);
	if(err != 0) 		
	{	close(giTccDXBCtrlFD);	
		giTccDXBCtrlFD = -1;
		 PRINTF_LV_0 ("%s ioctl(0x%x) error :: %d\n", __func__, IOCTL_DXB_CTRL_SET_BOARD, err);
		return 1;
	}

	err = ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_GET_CTLINFO, &stCtrl);
	if(err != 0) 		
	{	close(giTccDXBCtrlFD);	
		giTccDXBCtrlFD = -1;
		 PRINTF_LV_0 ("%s ioctl(0x%x) error :: %d\n", __func__, IOCTL_DXB_CTRL_GET_CTLINFO, err);
		return 1;
	}

    return 0;
}

void TCC_GPIO_Close_TC3X(void)
{

	close(giTccDXBCtrlFD);	
	giTccDXBCtrlFD = -1;
	

}

// -----------------------------------------------------------------
// Power Control
//
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_PW_RESET
//    Description : Power Reset to reset Baseband
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_PW_RESET (int moduleidx)
{
    unsigned int deviceIdx;
    int ret = 0;
    ret = TCC_GPIO_Open_TC3X();

    if(ret != 0)
    {
        PRINTF_LV_0 ("[TC3X_IO_PW_ON ] GPIO open failed!\n");
        return;
    }
    
    switch (moduleidx)
    {
    case 0:
        deviceIdx = 0;
		ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_RESET, &deviceIdx);
        if(g_tSTDInfo[moduleidx].NumOfDemodule>1)
        {
            deviceIdx = 1;
            ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_RESET, &deviceIdx);
        }
		break;

    case 1:
        deviceIdx = 1;
		ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_RESET, &deviceIdx);
		break;
    default:
        PRINTF_LV_0 ("[TC3X_IO_PW_RESET Error] Please insert code \n");
        break;
    }
    TCC_GPIO_Close_TC3X();
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_PW_ON
//    Description : power on
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_PW_ON (int moduleidx)
{
    unsigned int deviceIdx;
    int ret = 0;
    
    ret = TCC_GPIO_Open_TC3X();

    if(ret != 0) 
    {
        PRINTF_LV_0 ("[TC3X_IO_PW_ON ] GPIO open failed!\n");
        return;
    }
    switch (moduleidx)
    {
    case 0: // E3
        deviceIdx = 0;
		ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_ON, &deviceIdx);

        if(g_tSTDInfo[moduleidx].NumOfDemodule>1)
        {
            deviceIdx = 1;
    	    ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_ON, &deviceIdx);
    	}
		break;
    case 1: // D12
        deviceIdx = 1;
		ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_ON, &deviceIdx);
		break;

    default:
        PRINTF_LV_0 ("[TC3X_IO_PW_RESET Error] Please insert code \n");
        break;
    }
    
    TCC_GPIO_Close_TC3X();
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_PW_DN
//    Description : power down
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_PW_DN (int moduleidx)
{
    unsigned int deviceIdx;
    int ret = 0;
    ret = TCC_GPIO_Open_TC3X();

    if(ret != 0)
    {
        PRINTF_LV_0 ("[TC3X_IO_PW_ON ] GPIO open failed!\n");
        return;
    }
    

    switch (moduleidx)
    {
    case 0:
        deviceIdx = 0;
		ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_OFF, &deviceIdx);
        if(g_tSTDInfo[moduleidx].NumOfDemodule>1)
        {
            deviceIdx = 1;
    		ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_OFF, &deviceIdx);
    	}
		break;

    case 1:
        deviceIdx = 1;
		ioctl(giTccDXBCtrlFD, IOCTL_DXB_CTRL_OFF, &deviceIdx);
		break;

    default:
        PRINTF_LV_0 ("[TC3X_IO_PW_RESET Error] Please insert code \n");
        break;
    }

    TCC_GPIO_Close_TC3X();
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_Set_AntennaPower
//    Description : set a mode for antenna power control
//    Input : 
//      arg : 0-off, 1-on, 2-auto
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_Set_AntennaPower(int arg)
{
	return 0;
}

//----------------------------------------------------------------------------------------
//
//  Sleep Function Define
//
//----------------------------------------------------------------------------------------
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_Sleep
//    Description : sleep
//    Input : 
//      ms : milli second
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_Sleep (unsigned int ms)
{
    // in TCC79 SDK, min tick time is 5ms.
    usleep (ms*1000);
}

//----------------------------------------------------------------------------------------
//
//  Interface Semaphore Control - used TC3X
//
//----------------------------------------------------------------------------------------
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_IF_SemaphoreInit
//    Description : initialize interface semaphore
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_IF_SemaphoreInit (int moduleidx)
{   // I2C share if line. so semaphore use only tc3x_sem_if0
    if(g_tSTDInfo[moduleidx].MainIO==TC3X_IF_I2C)
    {
        if(moduleidx==0)
        {
            if(pTCC3X_Driver1)
                return; // already inited
        }
        else
        {
            if(pTCC3X_Driver0)
                return; // already inited
        }
        sem_init(&TC3X_SEM_IF[moduleidx], 0, 1);
    }
    else
    {
        sem_init(&TC3X_SEM_IF[moduleidx], 0, 1);
    }
}

#if defined (STDDEF_DVBH)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_RS_SemaphoreInit
//    Description : initialize rs decode routine semaphore
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_RS_SemaphoreInit (int moduleidx)
{
}
#endif // STDDEF_DVBH

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_MailBox_SemaphoreInit
//    Description : initialize mailbox semaphore
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_MailBox_SemaphoreInit (int moduleidx)
{
    sem_init(&TC3X_SEM_MAILBOX[moduleidx], 0, 1);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_OP_Mailbox_SemaphoreInit
//    Description : initialize mailbox semaphore
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_OP_Mailbox_SemaphoreInit (int moduleidx)
{
    sem_init (&TC3X_SEM_OP_MAILBOX[moduleidx], 0, 1);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_IF_SemaphoreDelete
//    Description : delete interface semaphore
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_IF_SemaphoreDelete (int moduleidx)
{   // I2C share if line. so semaphore use only tc3x_sem_if0
    if(g_tSTDInfo[moduleidx].MainIO==TC3X_IF_I2C)
    {
        if(moduleidx==0)
        {
            if(pTCC3X_Driver1)
                return; // already inited
        }
        else
        {
            if(pTCC3X_Driver0)
                return; // already inited
        }
        sem_destroy(&TC3X_SEM_IF[moduleidx]);
    }
    else
    {
        sem_destroy(&TC3X_SEM_IF[moduleidx]);
    }
}

#if defined (STDDEF_DVBH)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_RS_SemaphoreDelete
//    Description : delete rs decoding routine semaphore
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_RS_SemaphoreDelete (int moduleidx)
{
}
#endif // STDDEF_DVBH

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_MailBox_SemaphoreDelete
//    Description : delete mailbox semaphore
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_MailBox_SemaphoreDelete (int moduleidx)
{
    sem_destroy(&TC3X_SEM_MAILBOX[moduleidx]);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_OP_Mailbox_SemaphoreDelete
//    Description : delete mailbox semaphore
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_OP_Mailbox_SemaphoreDelete (int moduleidx)
{
    sem_destroy (&TC3X_SEM_OP_MAILBOX[moduleidx]);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_IF_LOCK
//    Description : interface lock
//    Input : 
//      moduleidx : module index
//    Output : 
//      don't care
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_IF_LOCK (int moduleidx)
{   // I2C share if line. so semaphore use only tc3x_sem_if0
    if(g_tSTDInfo[moduleidx].MainIO==TC3X_IF_I2C)
    {
        sem_wait(&TC3X_SEM_IF[0]);
    }
    else
    {
        sem_wait(&TC3X_SEM_IF[moduleidx]);
    }
    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_IF_UnLOCK
//    Description : interface unlock
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_IF_UnLOCK (int moduleidx)
{   // I2C share if line. so semaphore use only tc3x_sem_if0
    if(g_tSTDInfo[moduleidx].MainIO==TC3X_IF_I2C)
    {
        sem_post(&TC3X_SEM_IF[0]);
    }
    else
    {
        sem_post(&TC3X_SEM_IF[moduleidx]);
    }
    return 0;
}

#if defined (STDDEF_DVBH)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_RS_LOCK
//    Description : rs lock
//    Input : 
//      moduleidx : module index
//    Output : 
//      don't care
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_RS_LOCK (int moduleidx)
{
    return 0;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_RS_UnLOCK
//    Description : rs unlock
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_RS_UnLOCK (int moduleidx)
{
    return 0;
}
#endif // STDDEF_DVBH

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_MailBox_LOCK
//    Description : mailbox lock
//    Input : 
//      moduleidx : module index
//    Output : 
//      don't care
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_MailBox_LOCK (int moduleidx)
{
    return sem_wait(&TC3X_SEM_MAILBOX[moduleidx]);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_MailBox_UnLOCK
//    Description : mailbox unlock
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_MailBox_UnLOCK (int moduleidx)
{
    return sem_post(&TC3X_SEM_MAILBOX[moduleidx]);
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_OP_Mailbox_LOCK
//    Description : Monitoring lock
//    Input : 
//      moduleidx : module index
//    Output : 
//      don't care
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_OP_Mailbox_LOCK (int moduleidx)
{
    return sem_wait(&TC3X_SEM_OP_MAILBOX[moduleidx]);
}


//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_OP_Mailbox_UnLOCK
//    Description : Monitoring unlock
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_OP_Mailbox_UnLOCK (int moduleidx)
{
    return sem_post(&TC3X_SEM_OP_MAILBOX[moduleidx]);
}

//----------------------------------------------------------------------------------------
//
//  Que Control
//
//----------------------------------------------------------------------------------------
#if defined (STDDEF_DVBH)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_QUE_TIMES_Create
//    Description : create timeslicing message queue
//    Input : 
//      moduleidx : module index
//    Output : 
//      0 : fail / others : message queue pointer
//    Remark : 
//--------------------------------------------------------------------------
void   *TC3X_IO_QUE_TIMES_Create (int moduleidx)
{
    return &TC3X_QUE_TIMESLICING[moduleidx];
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_QUE_TIMES_Delete
//    Description : delete timeslicing message queue
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_QUE_TIMES_Delete (int moduleidx)
{
}
#endif // STDDEF_DVBH

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_QUE_INT_Create
//    Description : create interrupt message queue
//    Input : 
//      moduleidx : module index
//    Output : 
//      0 : fail / others : message queue pointer
//    Remark : 
//--------------------------------------------------------------------------
void     *TC3X_IO_QUE_INT_Create (int moduleidx)
{
    return &TC3X_QUE_INTERRUPT[moduleidx];
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_QUE_INT_Delete
//    Description : delete interrupt message queue
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_QUE_INT_Delete (int moduleidx)
{
    TC3X_QUE_INTERRUPT[moduleidx] = 0;
}

#if defined (STDDEF_DVBH)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_QUE_RS_Create
//    Description : create rs message queue
//    Input : 
//      moduleidx : module index
//    Output : 
//      0 : fail / others : message queue pointer
//    Remark : 
//--------------------------------------------------------------------------
void   *TC3X_IO_QUE_RS_Create (int moduleidx)
{
    return &TC3X_QUE_MPEFEC[moduleidx];
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_QUE_RS_Delete
//    Description : delete rs message queue
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_QUE_RS_Delete (int moduleidx)
{
}


//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_QUE_MPE_Create
//    Description : create mpe message queue
//    Input : 
//      moduleidx : module index
//    Output : 
//      0 : fail / others : message queue pointer
//    Remark : 
//--------------------------------------------------------------------------
void   *TC3X_IO_QUE_MPE_Create (int moduleidx)
{
    return &TC3X_QUE_DVBH_MANAGE[moduleidx];
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_QUE_MPE_Delete
//    Description : delete mpe message queue
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_QUE_MPE_Delete (int moduleidx)
{
}
#endif // STDDEF_DVBH

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_QUE_Reset
//    Description : reset queue
//    Input : 
//      queue
//    Output : 
//      0 : fail / others : success
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_QUE_Reset (void *queue)
{
    return 1;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_QUE_Send
//    Description : send queue
//    Input : 
//      queue
//      message
//    Output : 
//      0 : fail / others : success
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_QUE_Send (void *queue, void *message)
{
    return 1;
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_QUE_Receive
//    Description : receice queue
//    Input : 
//      queue
//      message
//      suspend
//    Output : 
//      0 : fail / others : success
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_QUE_Receive (void *queue, void *message, unsigned int suspend)
{
    return 1;
}

//----------------------------------------------------------------------------------------
//
//  Task Control
//
//----------------------------------------------------------------------------------------
#if defined (STDDEF_DVBH)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_DVBH_Manage_Task_Create
//    Description : create dvbh manage task
//    Input : 
//      moduleidx : module index
//      pTC3XTask_DVBH_Manage : function pointer
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_DVBH_Manage_Task_Create (int moduleidx, void (*pTC3XTask_DVBH_Manage) (void))      // for dvbh
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_DVBH_Manage_Task_Delete
//    Description : delete dvbh manage task
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_DVBH_Manage_Task_Delete (int moduleidx)       // for dvbh
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_MPEFECTask_Create
//    Description : create mpefec manage task
//    Input : 
//      moduleidx : module index
//      pTC3XTask_MPEFEC : function pointer
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_MPEFECTask_Create (int moduleidx, void (*pTC3XTask_MPEFEC) (void)) // for dvbh
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_MPEFECTask_Delete
//    Description : delete mpefec manage task
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_MPEFECTask_Delete (int moduleidx)             // for dvbh
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_TimeSlicingTask_Create
//    Description : create timeslicing manage task
//    Input : 
//      moduleidx : module index
//      pTC3XTask_TimeSlicing : function pointer
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_TimeSlicingTask_Create (int moduleidx, void (*pTC3XTask_TimeSlicing) (void))       // for dvbh
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_TimeSlicingTask_Delete
//    Description : delete timeslicing manage task
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_TimeSlicingTask_Delete (int moduleidx)        // for dvbh
{
}
#endif // STDDEF_DVBH

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_InterruptTask_Create
//    Description : create interrupt manage task
//    Input : 
//      moduleidx : module index
//      pTC3XTask_Interrupt : function pointer
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_InterruptTask_Create (int moduleidx, void (*pTC3XTask_Interrupt) (void))
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_InterruptTask_Delete
//    Description : delete interrupt manage task
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_InterruptTask_Delete (int moduleidx)
{
}

//----------------------------------------------------------------------------------------
//
//  HISR Control
//
//----------------------------------------------------------------------------------------

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_SPI_Create
//    Description : create spi hisr interrupt handler
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_SPI_Create (int moduleidx)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_SPI_Delete
//    Description : delete spi hisr interrupt handler
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_SPI_Delete (int moduleidx)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_BB_INT_Create
//    Description : create baseband hisr interrupt handler
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_BB_INT_Create (int moduleidx)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_BB_INT_Delete
//    Description : delete baseband hisr interrupt handler
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_BB_INT_Delete (int moduleidx)
{
}

#if defined (STDDEF_DVBH)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_TC3X_TIMER_5M_Create
//    Description : create 5milli second hisr interrupt handler
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_TC3X_TIMER_5M_Create (int moduleidx)     // for dvbh
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_TC3X_TIMER_5M_Delete
//    Description : delete 5milli second hisr interrupt handler
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_TC3X_TIMER_5M_Delete (int moduleidx)     // for dvbh
{
}
#endif // STDDEF_DVBH

//----------------------------------------------------------------------------------------
//
//  BB Interrupt Setting (TCC79X use external interrupt 2)
//
//----------------------------------------------------------------------------------------

// TC3X case : if stream io use gpsb interrupt(SPI,STS,..), it use edge mode.
//                other case use level mode.

// Baseband Interrupt Host Control
//void TC3X_IO_BB_INT_Ctrl(int command, int EdgeMode, int ActiveHigh)
//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_BB_INT_Ctrl
//    Description : baseband interrupt control function
//    Input : 
//      moduleidx : module index
//      command : command
//      trigmode : trigger mode
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_BB_INT_Ctrl (int moduleidx, int command, int TrigMode)
{
    switch (command)
    {
    case CMD_BB_INT_INIT:
        break;

    case CMD_BB_INT_ENABLE:
        break;

    case CMD_BB_INT_DISABLE:
        break;

    case CMD_BB_INT_PAUSE:
        break;

    case CMD_BB_INT_RESUME:
        break;

    default:
        break;
    }
}

//----------------------------------------------------------------------------------------
//
//  SDIO Interrupt Control (Active Low, Leveled)
//
//----------------------------------------------------------------------------------------

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_SDIO_INT_Ctrl
//    Description : sdio interrupt control function
//    Input : 
//      moduleidx : module index
//      command : command
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_SDIO_INT_Ctrl (int moduleidx, int command)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_Stream_SPI_Ctrl
//    Description : spi control function
//    Input : 
//      moduleidx : module index
//      command : command
//      pStreamBuff : stream buffer pointer
//      PktSize : packet size
//      PktNum : packet number
//      StreamIO : stream io
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_Stream_SPI_Ctrl (int moduleidx, int command, unsigned char **pStreamBuff, int PktSize, int PktNum, int StreamIO)
{
}

#if defined (STDDEF_DVBH)
//----------------------------------------------------------------------------------------
//
//  Host Interrupt Handler - Timer (5ms)
//
//----------------------------------------------------------------------------------------

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_ACTIVE_HISR_5mTimer
//    Description : activate 5milli second timer hisr handler
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_ACTIVE_HISR_5mTimer (int moduleidx)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_LISR_5mTimer_Handler0
//    Description : 5milli second lisr interrupt handler for bb #0
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_LISR_5mTimer_Handler0 (void)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_LISR_5mTimer_Handler1
//    Description : 5milli second lisr interrupt handler for bb #1
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_LISR_5mTimer_Handler1 (void)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_5mTimer_Handler0
//    Description : 5milli second hisr interrupt handler for bb #0
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_5mTimer_Handler0 (void)
{
    // call timer Interrupt HISR Handler Processing routine.
    fnHISR_5mTIMER0 (0); // call TC3X_HISR_5mTime
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_5mTimer_Handler1
//    Description : 5milli second hisr interrupt handler for bb #1
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_5mTimer_Handler1 (void)
{
    // call timer Interrupt HISR Handler Processing routine.
    fnHISR_5mTIMER1 (1); // call TC3X_HISR_5mTime
}

#endif // STDDEF_DVBH

//----------------------------------------------------------------------------------------
//
//  Host Interrupt Handler - SPI
//
//----------------------------------------------------------------------------------------

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_ACTIVE_HISR_SPI
//    Description : activate spi interrupt hisr handler
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_ACTIVE_HISR_SPI (int moduleidx)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_LISR_SPI_Handler0
//    Description : baseband #0 SPI lisr handler
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_LISR_SPI_Handler0 (void)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_LISR_SPI_Handler1
//    Description : baseband #1 SPI lisr handler
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_LISR_SPI_Handler1 (void)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_SPI_Handler0
//    Description : baseband #0 SPI hisr handler
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_SPI_Handler0 (void)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_SPI_Handler1
//    Description : baseband #1 SPI hisr handler
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_SPI_Handler1 (void)
{
}

//----------------------------------------------------------------------------------------
//
//  Host Interrupt Handler - BB Interrupt
//
//----------------------------------------------------------------------------------------

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_ACTIVE_HISR_BB_INT
//    Description : activate baseband interrupt hisr handler
//    Input : 
//      moduleidx : module index
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_ACTIVE_HISR_BB_INT (int moduleidx)
{
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_LISR_BB_INT_Handler0
//    Description : baseband #0 interrupt handler lisr
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_LISR_BB_INT_Handler0 (void)
{
    // call BB Interrupt LISR Handler Processing routine.
    fnLISR_BBINT0 (0);  // call TC3X_LISR_BBINT
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_LISR_BB_INT_Handler1
//    Description : baseband #1 interrupt handler lisr
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_LISR_BB_INT_Handler1 (void)
{
    // call BB Interrupt LISR Handler Processing routine.
    fnLISR_BBINT1 (1);  // call TC3X_LISR_BBINT
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_BB_INT_Handler0
//    Description : baseband #0 interrupt handler hisr
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_BB_INT_Handler0 (void)
{
    // call BB Interrupt HISR Handler Processing routine.
    fnHISR_BBINT0 (0);  // call TC3X_HISR_BBINT
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_HISR_BB_INT_Handler1
//    Description : baseband #1 interrupt handler hisr
//    Input : 
//    
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
void TC3X_IO_HISR_BB_INT_Handler1 (void)
{
    // call BB Interrupt HISR Handler Processing routine.
    fnHISR_BBINT1 (1);  // call TC3X_HISR_BBINT
}

//--------------------------------------------------------------------
// Memory Control
//--------------------------------------------------------------------

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_malloc
//    Description : memory allocation
//    Input : 
//      size : size
//    Output : 
//      memory pointer
//    Remark : 
//--------------------------------------------------------------------------
void     *TC3X_IO_malloc (unsigned int size)
{
    return (malloc (size));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_memset
//    Description : memory set 
//    Input : 
//      p : pointer
//      init : init value
//      size : size
//    Output : 
//      memory pointer
//    Remark : 
//--------------------------------------------------------------------------
void     *TC3X_IO_memset (void *p, int init, int size)
{
    return (memset (p, init, size));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_memcpy
//    Description : memory copy 
//    Input : 
//      des : destination
//      src : source
//      size : size
//    Output : 
//      memory pointer
//    Remark : 
//--------------------------------------------------------------------------
void     *TC3X_IO_memcpy (void *des, void *src, int size)
{
    return (memcpy (des, src, size));
}

//--------------------------------------------------------------------------
//    Function name : 
//        TC3X_IO_free
//    Description : memory free
//    Input : 
//      p : memory pointer
//    Output : 
//    
//    Remark : 
//--------------------------------------------------------------------------
int TC3X_IO_free (void *p)
{
    free (p);
    return 0;
}

