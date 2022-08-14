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

#ifndef TCC_SPDIF_DTS_H
#define TCC_SPDIF_DTS_H

static const int dts_sampling_rate[] =
{
	0,
	8000,
	16000,
	32000,
	0,
	0,
	11025,
	22050,
	44100,
	0,
	0,
	12000,
	24000,
	48000,
	96000,
	192000
};

int dts_header_parse(unsigned char* hdr,spdif_header_info_s *hinfo, int dtsiv_mode, int packet_size);
int dts_header_parse_iv(unsigned char* hdr,spdif_header_info_s *hinfo, int dtsiv_mode, int packet_size);

#endif /* TCC_SPDIF_DTS_H */
