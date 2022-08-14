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

#ifndef	_TCC_DXB_MANAGER_ALTICAS_H_
#define	_TCC_DXB_MANAGER_ALTICAS_H_

/********************************************************************
Header Files
********************************************************************/
#include "tcc_alticas.h"
#include "tcc_dxb_interface_type.h"

/***************************************************************
Macro definition
***************************************************************/


/***************************************************************
Type definition
***************************************************************/


/***************************************************************
Function definition
***************************************************************/

extern DxB_ERR_CODE tcc_manager_cas_section_notify(UINT32 ulDemuxId, UINT32 SectionHandle, UINT8 *pucBuf,  UINT32 ulPid, UINT32 ulRequestId, UINT32 length);
extern DxB_ERR_CODE tcc_manager_cas_alloc_buffer(UINT32 usBufLen, UINT8 **ppucBuf);
extern DxB_ERR_CODE tcc_manager_cas_free_buffer( UINT8 *pucBuf);
extern INT32 tcc_manager_cas_get_unique_id(INT8 *unique_id, UINT32 unique_id_max_size, UINT32 *unique_id_size);
extern INT32 tcc_manager_cas_set_cw(ENCRYPTION_TYPE alg,
	ENCRYPTION_MODE_TYPE mode,
	UINT32 video_pid,
	UINT32 audio_pid,
	UINT32 descramble_key_len,
	UINT8 * descramble_key,
	UINT32 descramble_iv_len,
	UINT8 * descramble_iv,
	UINT32 flag);
extern INT32 tcc_manager_cas_start_packet_filtering (INT32* handle, 
	UINT32 pid, 
	INT8* table_id_mask, 
	INT8* table_id, 
	CA_section_callback func, 
	UINT32 buffsize);
extern INT32 tcc_manager_cas_stop_packet_filtering (INT32 handle);


#endif

