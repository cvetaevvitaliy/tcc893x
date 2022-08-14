/******************************************************************************
*
*  (C)Copyright All Rights Reserved by Telechips Inc.
*
*  This material is confidential and shall remain as such.
*  Any unauthorized use, distribution, reproduction is strictly prohibited.
*
*******************************************************************************
*
*  File Name   : Vioc_config.h
*
*  Description :
*
*******************************************************************************
*
*  yyyy/mm/dd     ver            descriptions                Author
*	---------	--------   ---------------       -----------------
*    2011/08/08     0.1            created                      hskim
*******************************************************************************/

#ifndef __VIOC_CONFIG_H__
#define	__VIOC_CONFIG_H__

#include <platform/reg_physical.h>
#include <platform/globals.h>

#define	VIOC_SC0						(0)
#define	VIOC_SC1						(1)
#define	VIOC_SC2						(2)
#define	VIOC_SC3						(3)
		#define	VIOC_SC_RDMA_00			(00)
		#define	VIOC_SC_RDMA_01			(01)
		#define	VIOC_SC_RDMA_02			(02)
		#define	VIOC_SC_RDMA_03			(03)
		#define	VIOC_SC_RDMA_04			(04)
		#define	VIOC_SC_RDMA_05			(05)
		#define	VIOC_SC_RDMA_06			(06)
		#define	VIOC_SC_RDMA_07			(07)
#define	VIOC_VIQE						(4)
#define	VIOC_DEINTLS					(5)

#define	VIOC_DEVICE_INVALID     		(-2)
#define	VIOC_DEVICE_BUSY				(-1)
#define	VIOC_DEVICE_CONNECTED			( 0)

#define VIOC_PATH_DISCONNECTED			(0)
#define VIOC_PATH_CONNECTING  			(1)
#define VIOC_PATH_CONNECTED   			(2)
#define VIOC_PATH_DISCONNECTING			(3)

typedef struct{
	unsigned int enable;
	unsigned int connect_statue;
	unsigned int connect_device;
}VIOC_PlugInOutCheck;

/* Interface APIs */
extern int VIOC_CONFIG_PlugIn(unsigned int nType, unsigned int nValue);
extern int VIOC_CONFIG_PlugOut(unsigned int nType);
#endif



