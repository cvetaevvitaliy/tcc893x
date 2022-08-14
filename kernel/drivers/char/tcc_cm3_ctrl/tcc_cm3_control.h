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


#ifndef     _TCC_CM3_CONTROL_H_
#define     _TCC_CM3_CONTROL_H_

#define CM3_CTRL_DEV_FILE		"/dev/tcc_cm3_ctrl"
#define CM3_CTRL_DEV_NAME		"tcc_cm3_ctrl"
#define CM3_CTRL_DEV_MAJOR		221
#define CM3_CTRL_DEV_MINOR		0

#define IOCTL_CM3_CTRL_OFF		    0
#define IOCTL_CM3_CTRL_ON			1
#define IOCTL_CM3_CTRL_RESET    	2
#define IOCTL_CM3_CTRL_CMD			3

#define DEVICE_NAME	"mailbox"

#define MBOX_IRQ    INT_CM3MB

typedef struct{
    int iOpCode;
    int* pHandle;
    void* pParam1;
    void* pParam2; 
}t_cm3_avn_cmd;

typedef struct
{
    unsigned int uiCmdId;
    unsigned int uiParam0;
    unsigned int uiParam1;
    unsigned int uiParam2;
    unsigned int uiParam3;
    unsigned int uiParam4;
    unsigned int uiParam5;
    unsigned int uiParam6;
}ARG_CM3_CMD;

extern int CM3_SEND_COMMAND(ARG_CM3_CMD *pInARG, ARG_CM3_CMD *pOutARG);

#endif	//_TCC_CM3_CONTROL_H_

