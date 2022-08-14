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

#ifndef	_TCC_DXB_INTERFACE_ALTICAS_H_
#define	_TCC_DXB_INTERFACE_ALTICAS_H_

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
Header Files
********************************************************************/

/***************************************************************
Macro definition
***************************************************************/


/***************************************************************
Type definition
***************************************************************/
#define CA_8_BIT_TYPE char /*Type char must be 8 bits long.*/
#define CA_16_BIT_TYPE short /*Type short int must be 16 bits long.*/
#define CA_32_BIT_TYPE int /*Type long int must be 32 bits long.*/

/*Signed and unsigned int types.*/
typedef signed CA_8_BIT_TYPE CA_INT8;
typedef unsigned CA_8_BIT_TYPE CA_UINT8;
typedef signed CA_16_BIT_TYPE CA_INT16;
typedef unsigned CA_16_BIT_TYPE CA_UINT16;
typedef signed CA_32_BIT_TYPE CA_INT32;
typedef unsigned CA_32_BIT_TYPE CA_UINT32;

#define FLAG_EVEN 0
#define FLAG_ODD 1
#define FLAG_ALL 2 // even & odd

typedef enum ENCRYPTION_TYPE {
	ALG_NONE = 0, // Descrambling stop
	ALG_3DES,
	ALG_DES,
	ALG_AES,
	ALG_DVB
}ENCRYPTION_TYPE; // 모든 알고리즘을 다 제공할 필요는 없음 (DVB, AES 필수)

typedef enum ENCRYPTION_MODE_TYPE {
	MODE_ECB = 0,
	MODE_CBC,
	MODE_CFB,
	MODE_OFB
}ENCRYPTION_MODE_TYPE; // 모든 알고리즘을 다 제공할 필요는 없음 (ECB 모드 필수)

typedef enum CA_EVENT {
	CA_SECTION_AVAIL = 0,
	CA_SECTION_FAILED // An invalid section arrived
}CA_EVENT;

typedef struct CA_section {
	CA_UINT32 handle;
	CA_UINT32 pid;
	CA_UINT32 length;
	CA_INT8 *buf;
} CA_section;

typedef void(* CA_section_callback) (CA_section *section, CA_EVENT event);

/***************************************************************
Function definition
***************************************************************/

/**************************************************************************
 FUNCTION NAME : CA_init_cas
 
 DESCRIPTION : 이 함수는 altiCAS에서 구현되고 TeleChips 의 CAS PL에서 altiCAS초기화를 위해 호출되는 함수이다. 
 
 INPUT	 :  NONE
 OUTPUT  : NONE
 return  : 	NONE
**************************************************************************/
extern void CA_init_cas();
extern CA_INT32 CA_get_unique_id(CA_INT8 *unique_id, CA_UINT32 unique_id_max_size, CA_UINT32 *unique_id_size);
extern CA_INT32 CA_set_cw(ENCRYPTION_TYPE alg, ENCRYPTION_MODE_TYPE mode,CA_UINT32 video_pid,CA_UINT32 audio_pid,CA_UINT32 descramble_key_len,CA_UINT8 * descramble_key,CA_UINT32 descramble_iv_len,CA_UINT8 * descramble_iv,CA_UINT32 flag);

/**************************************************************************
 FUNCTION NAME : CA_channel_change
 
 DESCRIPTION : 이 함수는 altiCAS에서 구현되고 TeleChips 의 CAS PL에서 채널이 전환되거나 PMT의 정보가 바뀌는 경우 altiCAS 에게 알려주기 위해 호출하는 함수이다.
				. channel_id는 PMT 내의 program_number와 같은 unique channel id를 나타내야 한다.
 
 INPUT	 :  . CA_INT32 channel_id : The unique channel id
			. CA_INT8* pmt_data : The pointer of PMT data
			. CA_UINT32 pmt_data_len : The length of PMT data
 OUTPUT  : NONE
 return  : 	0 : success
			-1 : fail
**************************************************************************/
extern CA_INT32 CA_channel_change (unsigned int service_id, void * pmt_data);

extern CA_INT32 CA_start_packet_filtering (CA_INT32* handle, CA_UINT32 pid, CA_INT8* table_id_mask, CA_INT8* table_id, CA_section_callback func, CA_UINT32 buffsize);
extern CA_INT32 CA_stop_packet_filtering (CA_INT32 handle);

#ifdef __cplusplus
}
#endif


#endif

