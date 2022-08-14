/**

  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/
/****************************************************************************
 *   FileName    : video_user_data.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/

#ifndef __VIDEO_USER_DATA__
#define __VIDEO_USER_DATA__

#if 0//     MOVE_VPU_IN_KERNEL
#define VIDEO_USER_DATA_PROCESS
#endif

#define MAX_USERDATA 4
#define MAX_USERDATA_LIST_ARRAY 16

typedef struct _video_userdata_t
{
	unsigned char *pData;
	int            iDataSize;
} video_userdata_t;

typedef struct _video_userdata_list_s_
{
	int iIndex;
	int iValidCount;
	unsigned char fDiscontinuity;
	unsigned long long ullPTS;
	int iUserDataNum;
	video_userdata_t arUserData[MAX_USERDATA];
} video_userdata_list_t;

extern int UserDataCtrl_Clear( video_userdata_list_t *pUserDataList );
extern int UserDataCtrl_Init( video_userdata_list_t *pUserDataListArray );
extern int UserDataCtrl_ResetAll( video_userdata_list_t *pUserDataListArray );
extern int UserDataCtrl_Put( video_userdata_list_t *pUserDataListArray, int iIndex,
                             unsigned long long ullPTS, unsigned char fDiscontinuity, video_userdata_list_t *pUserDataList );
extern int UserDataCtrl_Get( video_userdata_list_t *pUserDataListArray, int iIndex,
							 video_userdata_list_t **ppUserDataList );

#endif //__VIDEO_USER_DATA__
