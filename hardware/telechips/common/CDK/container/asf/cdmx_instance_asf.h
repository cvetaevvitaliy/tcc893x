/****************************************************************************
 *   FileName    : cdmx_instance_asf.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information¡¯s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#ifndef _CDMX_INSTANCE_ASF_H_
#define _CDMX_INSTANCE_ASF_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 
// asf instance
//
typedef struct asf_dmx_inst_t
{
	// common
	cdmx_ctrl_t				stCdmxCtrl;
	cdmx_buff_t				stCdmxBuff;
	cdmx_seq_header_t		stSeqHeader;

	// for ASF demuxer
	av_dmx_handle_t			hDmxHandle;
	asf_dmx_init_t			stDmxInit;
	asf_dmx_info_t			stDmxInfo;
	asf_dmx_getstream_t		stDmxGetStream;
	asf_dmx_outstream_t		stDmxOutStream;
	asf_dmx_seek_t			stDmxSeek;
	av_result_t (*pfnDmxLib) ( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
	void					*pvLibHandle;

} asf_dmx_inst_t;

#endif//_CDMX_INSTANCE_ASF_H_

