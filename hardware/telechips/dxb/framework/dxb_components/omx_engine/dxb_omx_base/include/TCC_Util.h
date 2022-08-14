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

#ifndef	_TCC_UTIL_H__
#define	_TCC_UTIL_H__

/******************************************************************************
* include 
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

/******************************************************************************
* defines 
******************************************************************************/
#define STORAGE_NAND     "/storage/sdcard0"
#define STORAGE_SDCARD   "/storage/sdcard1"

#define DEVICE_NAME_LENGTH		80
#define MOUNT_DIRECTORY_LENGTH	80
#define FILESYSTEM_LENGTH		12

/******************************************************************************
* typedefs & structure
******************************************************************************/
struct f_size{
	long byteperblock;
	long blocks;
	long avail;
};

typedef struct _mountinfo {
	FILE *fp;			// 파일 스트림 포인터
	char devname[DEVICE_NAME_LENGTH];			// 장치 이름
	char mountdir[MOUNT_DIRECTORY_LENGTH];		// 마운트 디렉토리 이름
	char fstype[FILESYSTEM_LENGTH];				// 파일 시스템 타입
	struct f_size size;	// 파일 시스템의 총크기/사용율
} MOUNTP;


/******************************************************************************
* declarations
******************************************************************************/
MOUNTP *dfopen();
MOUNTP *dfget(MOUNTP *MP, int deviceType);
MOUNTP *dfgetex(MOUNTP *MP, unsigned char *pStrPath);
int dfclose(MOUNTP *MP);

#endif //_TCC_UTIL_H__
