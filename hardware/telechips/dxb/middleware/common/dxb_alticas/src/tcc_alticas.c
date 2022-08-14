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

#define LOG_NDEBUG 0
#include <utils/Log.h>

#include "tcc_dxb_manager_alticas.h"
#include "tcc_alticas.h"

 
/*****************************************************************************************
define
*****************************************************************************************/
#define DEBUG_PRINTF		//ALOGE

/*****************************************************************************************
Interface API
*****************************************************************************************/

/**************************************************************************
 FUNCTION NAME : CA_get_unique_id
 
 DESCRIPTION : TeleChips 의 CAS PL에서 고유의 식별자를 읽어온다. 
 
 INPUT	 :  CA_UINT32 unique_id_max_size : The maximum size of unique id
 OUTPUT  : CA_INT8* unique_id : The unique ID of TeleChips terminal
		     CA_UINT32 *unique_id_size : The size of unique
return  : 0 : success
		-1 : fail
**************************************************************************/
CA_INT32 CA_get_unique_id(CA_INT8 *unique_id, CA_UINT32 unique_id_max_size, CA_UINT32 *unique_id_size)
{
	int	err;
	err = tcc_manager_cas_get_unique_id(unique_id, unique_id_max_size, unique_id_size);
	return err;	
}

/**************************************************************************
 FUNCTION NAME : CA_set_cw
 
 DESCRIPTION : TeleChips 의 CAS PL에 CW를 설정하기 위해 사용한다.
				. ENCRYPTION_TYPE alg == ALG_NONE 일 경우 현재 진행하고 있는 Descrambling 작업을 멈춘다.
				. descramble_iv_len == 0인 경우 Initial Vector는 사용하지 않는다. 즉, descramble_iv = NULL이 된다.
				 Packet descrambling은 MPEG2 TS packet header의 scrambling control 필드를 보고 판단한다.
				<Scrambling control value (2 bits): The following per DVB spec[1]>
					Value				Description
					00				Not scrambled
					01				Reserved for future use
					10				Scrambled with even key
					11				Scrambled with odd key
 INPUT	 :  . ENCRYPTION_TYPE alg : Encryption type for descrambling
			. ENCRYPTION_MODE_TYPE mode : Encryption mode for descrambling
			. CA_UNIT32 video_pid : The video pid for descrambling
			. CA_UNIT32 audio_pid : The audio pid for descrambling
			. CA_UNIT32 descramble_key_len : The key length for descrambling
			. CA_UINT8* descramble_key : The key for descrambling
			. CA_UNIT32 descramble_iv_len : The length of IV (Initial Vector) for descrambling // Default Setting : 0
			. CA_UINT8* descramble_iv : The IV(Initial Vector) for descrambling
			. CA_UNIT32 flag : EVEN, ODD 지정
 OUTPUT  : NONE
return  : 0 : success
		-1 : fail
**************************************************************************/
CA_INT32 CA_set_cw(ENCRYPTION_TYPE alg,
	ENCRYPTION_MODE_TYPE mode,
	CA_UINT32 video_pid,
	CA_UINT32 audio_pid,
	CA_UINT32 descramble_key_len,
	CA_UINT8 * descramble_key,
	CA_UINT32 descramble_iv_len,
	CA_UINT8 * descramble_iv,
	CA_UINT32 flag)

{
	int err;
	err = tcc_manager_cas_set_cw(alg, mode, video_pid, audio_pid, descramble_key_len, descramble_key, descramble_iv_len, descramble_iv, flag);
	return err;	
}


/**************************************************************************
 FUNCTION NAME : CA_start_packet_filtering
 
 DESCRIPTION : 이 함수는 altiCAS 에서 필요한 Packet Section을 요청하기 위해 Callback API를 등록하는 함수이다.
				. table_id_mask와 table_id는 1byte로 다음과 같이 동작을 한다.
				(Section_data_buf[0] & table_id_mask == table_id)
				. 출력되는 handle은 현재 사용되고 있는 다른 handle과 중복되지 않는 유일한 값이어야 한다.
				. 만약 해당 함수의 반환 값이 -1(fail)인 경우 handle 의 값은 -1로 한다.
 INPUT	 :  . CA_UINT32 pid : The PID of a section to be filtered
			. CA_INT8 *table_id_mask : The table id mask
			. CA_INT8* table_id : The table id of a section to be filtered
			. CA_section_callback func : The callback function which will be invoked when the requested section is arrived
			. CA_UINT32 buffsize : The maximum buffer size of section to be filtered
 OUTPUT  :CA_INT32* handle : The section request handle to be set by this function
return  : 0 : success
		-1 : fail
**************************************************************************/
CA_INT32 CA_start_packet_filtering (CA_INT32* handle, 
	CA_UINT32 pid, 
	CA_INT8* table_id_mask, 
	CA_INT8* table_id, 
	CA_section_callback func, 
	CA_UINT32 buffsize)

{
	int err;
	err = tcc_manager_cas_start_packet_filtering(handle, pid, table_id_mask, table_id, func, buffsize);
	return err;
}

/**************************************************************************
 FUNCTION NAME : CA_stop_packet_filtering
 
 DESCRIPTION : 이 함수는 altiCAS가 요청한 Callback API를 취소하는 함수이다. 
 					이 함수가 호출된 이후에는 등록한 Callback API가 호출되어서는 안 된다.
 INPUT	 : CA_INT32 handle : The handle of the section request to be canceled
 OUTPUT : NONE
return  : 0 : success
		-1 : fail
**************************************************************************/
CA_INT32 CA_stop_packet_filtering (CA_INT32 handle)
{
	int	err = 0 ;
	err = tcc_manager_cas_stop_packet_filtering(handle);
	return err;
}



