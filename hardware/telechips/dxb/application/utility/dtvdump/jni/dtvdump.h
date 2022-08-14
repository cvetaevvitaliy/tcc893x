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
#ifndef     __DXB_DUMP_H__     
#define     __DXB_DUMP_H__

#ifdef __cplusplus
extern "C" {
#endif

	int DxbDumpInit(int iDxbType, int iFreq, int iBW, int iVoltage, int iMOD);
	int DxbDumpDeinit();
	int DxbDumpStart(int iPidNum, int *piPidTable, const char *pcFile, int iDumpSize);
	int DxbDumpStop();
	int SockDumpStart(const char * iIp, int iPort, const char * pcFile, int iDumpSize);
	int GetDumpSize();
	int GetDiskFreeSpace(const char *pcPath);

#ifdef __cplusplus
}
#endif

#endif
