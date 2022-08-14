/****************************************************************************
 *   FileName    : linuxtv_utils.c
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
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dmx.h>
#include <video.h>
#include "linuxtv_utils.h"

//#define     TEST_SECTIONFILER
#define  LOG_TAG    "libdtvdump"

#include <utils/Log.h>

#define     PRINTF  printf
#define DVR "/dev/dvb0.dvr0"

#ifdef      DUMP_FROM_DMX1
#define DMX "/dev/dvb0.demux1"
#else
#define DMX "/dev/dvb0.demux0"
#endif


#define PID_PAT				0x0000
#define PID_PMT				0x0002
#define	PID_PMT_1SEG		0x1FC8
#define PID_NIT 			0x0010
#define PID_SDT				0x0011
#define PID_EIT				0x0012
#define PID_TDT_TOT			0x0014

typedef enum
{
   PAT_ID,                    /* 0x00   program_association_section                             */
   CAT_ID,                    /* 0x01   conditional access_section                              */
   PMT_ID,                    /* 0x02   program_map_section                                     */
   
/* skott Add 2005.9.14  start */
   BIFS_ID			= 0x04,   /* 0x04   BIFS command section                                    */
   ODT_ID			= 0x05,   /* 0x05   Object description section                              */
/* skott Add 2005.9.14  end */

	DSMCC_R1_ID			= 0x3A, /* Multi-protocol capsule                                       */
	DSMCC_DII_MSG_ID	= 0x3B, /* U-N message includeing DII                                   */
	DSMCC_DDB_MSG_ID	= 0x3C, /* Same as in the left column                                   */
	DSMCC_SD_ID			= 0x3D, /* Same as in the left column                                   */
	DSMCC_PD_ID			= 0x3E, /* Same as in the left column                                   */
	DSMCC_R2_ID			= 0x3F, /* Same as in the left column                                   */

   NIT_A_ID          = 0x40,  /* 0x40  network_information_section - actual_network             */
   NIT_O_ID          = 0x41,  /* 0x41  network_information_section - other_network              */
   SDT_A_ID          = 0x42,  /* 0x42  service_description_section - actual_ts                  */
   SDT_O_ID          = 0x46,  /* 0x46  service_description_section - other_ts                   */
   BAT_ID            = 0x4A,  /* 0x4A  bouquet_association_section                              */
   EIT_PF_A_ID       = 0x4E,  /* 0x4E  event_information_section - actual_ts, present/following */
   EIT_PF_O_ID       = 0x4F,  /* 0x4F  event_information_section - other_ts, present/following  */
   EIT_SA_0_ID       = 0x50,  /* 0x50-0x5F   event_information_section - actual_ts, schedule    */
   EIT_SA_1_ID,
   EIT_SA_2_ID,
   EIT_SA_3_ID,
   EIT_SA_4_ID,
   EIT_SA_5_ID,
   EIT_SA_6_ID,
   EIT_SA_7_ID,
   EIT_SA_8_ID,
   EIT_SA_9_ID,
   EIT_SA_A_ID,
   EIT_SA_B_ID,
   EIT_SA_C_ID,
   EIT_SA_D_ID,
   EIT_SA_E_ID,
   EIT_SA_F_ID,
   EIT_SO_0_ID       = 0x60,  /* 0x60-0x6F   event_information_section - other_ts, schedule     */
   EIT_SO_1_ID,
   EIT_SO_2_ID,
   EIT_SO_3_ID,
   EIT_SO_4_ID,
   EIT_SO_5_ID,
   EIT_SO_6_ID,
   EIT_SO_7_ID,
   EIT_SO_8_ID,
   EIT_SO_9_ID,
   EIT_SO_A_ID,
   EIT_SO_B_ID,
   EIT_SO_C_ID,
   EIT_SO_D_ID,
   EIT_SO_E_ID,
   EIT_SO_F_ID,
   TDT_ID            = 0x70,  /* 0x70  time_date_section                */
   RST_ID            = 0x71,  /* 0x71  running_status_section           */
   ST_ID             = 0x72,  /* 0x72  stuffing_table                   */
   TOT_ID            = 0x73,  /* 0x73  time_offset_section              */

   ECM_0_ID					 = 0x82,
   ECM_1_ID					 = 0x83,
   EMM_0_ID					 = 0x84,
   EMM_1_ID					 = 0x85,

   SDTT_ID 		= 0xC3,
   BIT_ID		= 0xC4,
   CDT_ID		= 0xC8

}  MPEG_TABLE_IDS;

#define FRONTEND_0 "/dev/dvb0.frontend0"
static int ghDemux1;
static int gFrontend = -1;
static int *pidtemp;
static int ipidnumtemp;
int linuxtv_init(int *pid, int ipidnum)
{
    int i = 0;
	struct dmx_pes_filter_params pesFilterParams;
	struct dmx_sct_filter_params sctFilterParams;
	int iCheckCRC = 1, iRepeat = 1;
	pidtemp = pid;
	ipidnumtemp = ipidnum;

#ifndef      DUMP_FROM_DMX1	
//open Dummy FRONTEND_0, We download f/w to CM3 at this time. 
    gFrontend = open(FRONTEND_0, O_RDWR | O_NONBLOCK);
	if (gFrontend < 0) {
		PRINTF("CANNOT Open frontend driver");
	}
#endif
	memset(&pesFilterParams, 0, sizeof(struct dmx_pes_filter_params));
	memset(&sctFilterParams, 0, sizeof(struct dmx_sct_filter_params));	
  	if ((ghDemux1 = open(DMX, O_RDWR | O_NONBLOCK)) < 0)
    {
        return 1;
    }

    if (ioctl(ghDemux1, DMX_SET_BUFFER_SIZE, 20*1024*1024) < 0) {
			PRINTF("Failed to set buffer size!![%d]", ghDemux1);
	}

#ifdef      TEST_SECTIONFILER    
    sctFilterParams.pid = PID_EIT;
	sctFilterParams.timeout = 0;
	sctFilterParams.flags = 0;//DMX_IMMEDIATE_START, DMX_CHECK_CRC, DMX_ONESHOT
	if (iCheckCRC)
		sctFilterParams.flags |= DMX_CHECK_CRC;
	if (iRepeat == 0)
		sctFilterParams.flags |= DMX_ONESHOT;

	for (i=0 ; i < 1; i++) {
		sctFilterParams.filter.filter[i] = EIT_SA_0_ID;
		sctFilterParams.filter.mask[i] = 0xf0;
		sctFilterParams.filter.mode[i] = 0x00;
	}

   	if (ioctl(ghDemux1, DMX_SET_FILTER, &sctFilterParams) < 0) {
		PRINTF("Failed to set section filter!![%d]", ghDemux1);
	}
#else
    
    pesFilterParams.pid = pid[i];
    pesFilterParams.pes_type = DMX_PES_OTHER;
    pesFilterParams.input = DMX_IN_FRONTEND;
    pesFilterParams.output = DMX_OUT_TSDEMUX_TAP; //DMX_OUT_DECODER, DMX_OUT_TAP
    pesFilterParams.flags = 0;//DMX_IMMEDIATE_START

    if (ioctl(ghDemux1, DMX_SET_PES_FILTER, &pesFilterParams) < 0) {
        PRINTF("Failed to set pes filter!![%d]", ghDemux1);
    }

    for(i=1; i<ipidnum; i++){

        if (ioctl(ghDemux1, DMX_ADD_PID, &pid[i]) < 0) {
          PRINTF("Failed to add pid!![%d]", ghDemux1);
        }
    }
#endif	

    if (ioctl(ghDemux1, DMX_START, 0) < 0) {
		PRINTF("Failed to start filter!![%d]", ghDemux1);
	}
    PRINTF("[%s:%d]\n", __func__, __LINE__);
    return 0;
}

int linuxtv_close(void)
{
    PRINTF("[%s:%d]\n", __func__, __LINE__);
    ioctl(ghDemux1, DMX_STOP, 0);
    close(ghDemux1);    
    if (gFrontend >= 0) {
		close(gFrontend);
		gFrontend = -1;
	}
	sync();
    return 0;
}


int linuxtv_read(unsigned char *buff, int readsize)
{
    int size = 0;
   	struct pollfd pfd;

    pfd.fd = ghDemux1;
	pfd.events = POLLIN;
	if (poll(&pfd, 1, 1000)) 
	{
	    if (pfd.revents & POLLIN) 
        {
            size = read(ghDemux1, buff, readsize);
            if(size < 0)
                PRINTF("[%s:%d] erro : %s \n", __func__, __LINE__, strerror(errno));
        }
    }
    else 
    {
    	linuxtv_close();
    	linuxtv_init(pidtemp, ipidnumtemp);
    }
    return size;
}
