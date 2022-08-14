/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

#ifndef _TS_PARSE_H_
#define _TS_PARSE_H_

typedef unsigned int	BOOL;
typedef unsigned char	U8;
typedef signed char		S8;
typedef unsigned short	U16;
typedef signed short	S16;
typedef unsigned int	U32;
typedef signed int		S32;
typedef void			VOID;

#ifdef WIN32
#define	__func__		__FUNCTION__
#define LOGE(...) printf(__VA_ARGS__)
#define LOGD(...) printf(__VA_ARGS__)
#endif

#ifndef NULL
	#define NULL 	(0)
#endif

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#define PRINTF

extern void* TCC_malloc (unsigned int iSize);
extern int TCC_free(void *pvPtr);

enum
{
	AUDIO_PES_PACKET = 0,
	VIDEO_PES_PACKET, 
	RA_HEADER_PES_PACKET,	// used for RMVB
	PRIVATE_PES_PACKET,
	TELETEXT_PES_PACKET,
	DSMCC_PACKET,
	OTHER_PACKET,

	LAST_PES_PACKET
};

typedef enum
{
	SCAN_NONE_PID = 0x00,
	SCAN_PAT_PID  = 0x01, 
	SCAN_PMT_PID  = 0x02,
	SCAN_SDT_PID  = 0x04,
	SCAN_DONE_PID = 0x08,
	SCAN_STATUS_MAX
} SCAN_STATUS;

#endif	// _TS_PARSE_H_

/* end of file */

