/****************************************************************************
 *   FileName    : dumpmain.c
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
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <pthread.h>

#include "tcc_socket_util.h"

#define PRINTF(msg...)	printf(msg)

#define SRVTYPE_NONE                0x00
#define SRVTYPE_DAB                 0x01
#define SRVTYPE_DABPLUS             0x02
#define SRVTYPE_DATA                0x03
#define SRVTYPE_DMB                 0x04
#define SRVTYPE_FIDC                0x05
#define SRVTYPE_RAWDATA             0x06
#define SRVTYPE_FIC                 0x07
#define SRVTYPE_FIC_WITH_ERR        0x08
#define EWS_ANNOUNCE_FLAG                   0x09
#define RECONFIGURATION_FLAG                0x0a
#define EWS_ANNOUNCE_RECONFIGURATION_FLAG   0x0b


#define TSIF_PACKETSIZE			(188)
#define TSIF_MAX_PACKETCOUNT	(8190)
#define TSIF_INT_PACKETCOUNT	(39)


#define	STREAM_BUFFER_SIZE		(TSIF_PACKETSIZE*10)

typedef	struct
{
	FILE *pFP;
	unsigned char *pucData;
	unsigned int uiDataSize;
	unsigned int isWorking;
}DUMP_ARG;

static 	TCSOCK_HANDLE gpSockHandleAV;	

void *dumpthread(void *arg)
{
	DUMP_ARG *pArg;
	pArg = (DUMP_ARG *)arg;
	pArg->isWorking = 1;
	fwrite(	pArg->pucData, 1, pArg->uiDataSize, pArg->pFP);
	pArg->isWorking = 0;
	return NULL;
}

int main(int argc, char *argv[])
{
	DUMP_ARG stArg;
	FILE *pfp;
	unsigned char *pucMSCDMABuf, *pucBuf;
	pthread_t thread_id;
	int iTSIFHandle;
	int iPrevCC, iCC; 
	int err=0;
	int iARGS[3];
	int i, bufferpos, iPID;
	unsigned char *pucFile;
	int ret, iDumpSizeKbyte, iTotalSaveSize;
	unsigned char ucService[7];

	unsigned char	IPStr[32];
	unsigned int		port;

	char	*USAGE	=
		"Usage: iptvdump ip, port, dumpsize, file\n"
		"       IP unit.\n"
		"       Port unit.\n"
		"       dumpsize is Kbyte unit.\n"
		"       ex)iptvdump 239.1.1.1 5000 1024 /nand/iptv.ts\n"
	;

	if(argc < 5)
	{
		PRINTF("%s", USAGE);
		return -1;
	}	
	pucMSCDMABuf = NULL;
	iTSIFHandle = -1;
	pfp = NULL;
	thread_id = NULL;
	memset(&stArg, 0x00, sizeof(DUMP_ARG));	

	strcpy(IPStr, argv[1]);
//	IPStr = argv[1];

	PRINTF("IP : %s \n", IPStr);

	port = atoi(argv[2]);
	iDumpSizeKbyte = atoi(argv[3]);
	pucFile = argv[4];
	

	pfp = fopen(pucFile,"wb");
	if(pfp == NULL)
	{
		PRINTF("CANNOT Make file [%s] !!!\n", pucFile);
		goto _error_return_;		
	}
	
	PRINTF("port[%d] OutputPath[%s] Size[%dKbyte]\n", port, pucFile, iDumpSizeKbyte);
	
	pucMSCDMABuf = malloc(STREAM_BUFFER_SIZE*2 + TSIF_PACKETSIZE); //*2 dubble buffer
	if( NULL == pucMSCDMABuf )
	{	
		PRINTF("error malloc\n");
		goto _error_return_;
	}

	gpSockHandleAV = TCSOCKUTIL_Open(IPStr, port);
	if(gpSockHandleAV == NULL)
	{
		PRINTF ("SOCKET Open Error !!!\n");
		goto _error_return_;		
	}			


	
	////////////////////////
	iPrevCC = -1;
	iTotalSaveSize = 0;
	PRINTF("Dump Started.....\n");
	//for(j=0;j<2;j++)
	bufferpos = 0;
	while(1)
	{			
		//PRINTF("buffer pos %d\n", bufferpos);
		for(i=0; i<STREAM_BUFFER_SIZE; i+=TSIF_PACKETSIZE)
		{
			pucBuf = pucMSCDMABuf+bufferpos*STREAM_BUFFER_SIZE+i;
			ret = TCSOCKUTIL_Read(gpSockHandleAV, pucBuf, TSIF_PACKETSIZE, 500);

			if( ret == TSIF_PACKETSIZE )
			{
			}
			else
			{
				PRINTF("Read Error %d\n", ret);
			}
		}
		/*Save pucMSCDMABuf+bufferpos*STREAM_BUFFER_SIZE
		*/
		PRINTF("@");
		if(pfp)
		{
			stArg.pFP = pfp;
			stArg.pucData = pucMSCDMABuf+bufferpos*STREAM_BUFFER_SIZE;
			stArg.uiDataSize = STREAM_BUFFER_SIZE;			
			while(1)
			{
				if(stArg.isWorking)
				{
					//PRINTF("Waiting Until thread close !!!\n");
					PRINTF("W");
					usleep(5000);
				}	
				else
					break;
			}
			
			stArg.isWorking = 0;
			ret = pthread_create(&thread_id, NULL, dumpthread, &stArg);
			if(ret != 0)
			{
				PRINTF("cann't create thread\n");			
				goto _error_return_;
			}
		}

		iTotalSaveSize += STREAM_BUFFER_SIZE;		
		if( (iTotalSaveSize/1024)  > iDumpSizeKbyte)
		{
			PRINTF("\nDump Finished.....\n");
			break;
		}
		bufferpos++;
		bufferpos = bufferpos&0x1;		
	}
	if(pfp)
		pthread_join(thread_id, NULL);

	///////////////////////
	if(gpSockHandleAV)
	{
		TCSOCKUTIL_Close(gpSockHandleAV);
		gpSockHandleAV = NULL;
	}
	
	if(pfp)
		fclose(pfp);
	
	if(pucMSCDMABuf)
		free(pucMSCDMABuf);

	PRINTF("Return With Success !!!\n");
    return 0;
_error_return_:
	if(gpSockHandleAV)
	{
		TCSOCKUTIL_Close(gpSockHandleAV);
		gpSockHandleAV = NULL;
	}
	
	if(pfp)
		fclose(pfp);
	
	if(pucMSCDMABuf)
		free(pucMSCDMABuf);
	PRINTF("Return With Failure !!!\n");
	return -1;
}
