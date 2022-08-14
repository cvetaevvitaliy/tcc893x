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

#ifndef TCC_SPDIF_PARSE_H
#define TCC_SPDIF_PARSE_H

#include "spdif_defs.h"

#define DTS_IV_START_CODE_SIZE 10

typedef enum
{
	FORMAT_AC3 = 1,
	FORMAT_AC3_TRUE_HD,
	FORMAT_AC3_DDP,
	FORMAT_DTS,
	FORMAT_DTS_HD,
	FORMAT_MAX
}SPDIF_CODEC_TYPE;

typedef struct
{
	unsigned char* buf;
	spdif_header_info_s hdr;
	int use_header;
	int spdif_bs;
	int spdif_frame_size;
}spdif_parse_type_s;

extern void spdif_parse_init(void);
extern void spdif_parse_deinit(void);
extern int spdif_parser_frame(unsigned char* frame, int size,int codec_type,int usr_mode,int dts_mode);
extern int spdif_make_header(unsigned long param, unsigned short type, unsigned short len);
extern unsigned int spdif_parse_get_frame_size(void);
extern unsigned char* spdif_parse_get_buf(void);
extern unsigned int spdif_parse_get_endian(void);


#endif /* TCC_SPDIF_PARSE_H */
