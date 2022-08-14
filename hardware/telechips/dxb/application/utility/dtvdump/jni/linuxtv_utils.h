/****************************************************************************
 *   FileName    : linuxtv_utils.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/

/****************************************************************************

  Revision History

 ****************************************************************************

 ****************************************************************************/
#ifndef     __LINUEXTV_UTILS_H__     
#define     __LINUEXTV_UTILS_H__

//#define     DUMP_FROM_DMX1 //While DTV running, dumping stream from demux1. in this case, we don't control tuner part.

int linuxtv_init(int *pid, int ipidnum);
int linuxtv_close(void);
int linuxtv_read(unsigned char *buff, int readsize);

#endif
