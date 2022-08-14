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
#define LOG_TAG	"DVB_MANAGER_CAS"
#include <utils/Log.h>

#include "tcc_dxb_interface_cipher.h"
#include "tcc_dxb_manager_cas.h"
#include "tcc_dxb_manager_alticas.h"

 
/*****************************************************************************************
define
*****************************************************************************************/
#if LOG_NDEBUG
#define DEBUG_PRINTF		ALOGE
#else
#define DEBUG_PRINTF		
#endif

#define REQUEST_ID	55555

/*****************************************************************************************
Interface API
*****************************************************************************************/
CA_section_callback CAS_Callback;

DxB_ERR_CODE tcc_manager_cas_section_notify(UINT32 ulDemuxId, UINT32 SectionHandle, UINT8 *pucBuf,  UINT32 length, UINT32 ulPid, UINT32 ulRequestId)
{
	DEBUG_PRINTF("In %s\n", __func__);

	CA_section	*pSection_cmd;
	pSection_cmd = TCC_malloc(sizeof(CA_section));

	pSection_cmd->handle = SectionHandle;
	pSection_cmd->length = length;
	pSection_cmd->pid = ulPid;
	pSection_cmd->buf = pucBuf;

	CAS_Callback(pSection_cmd , 0);
	return DxB_ERR_OK;
}

DxB_ERR_CODE tcc_manager_cas_alloc_buffer(UINT32 usBufLen, UINT8 **ppucBuf)
{
	DEBUG_PRINTF("In %s\n", __func__);
	*ppucBuf = TCC_malloc(usBufLen);
	return DxB_ERR_OK;
}

DxB_ERR_CODE tcc_manager_cas_free_buffer( UINT8 *pucBuf)
{
	DEBUG_PRINTF("In %s\n", __func__);
	TCC_free(pucBuf);
	return DxB_ERR_OK;
}

INT32 tcc_manager_cas_close (void)
{
	DEBUG_PRINTF("In %s\n", __func__);

	int	err= 0;

	err = TCC_DxB_CAS_Close();
	if(err !=0)
	{
		DEBUG_PRINTF("error [%s][%d] \n", __func__, __LINE__);
		return -1;
	}
	return 0;
}

INT32 tcc_manager_cas_kwy_swap (unsigned char *srckey, int size)
{
	DEBUG_PRINTF("In %s\n", __func__);

	unsigned char *destkey;
	unsigned int		i;

	destkey = TCC_malloc(size);

	for(i=0;i<=size; i+=4)
	{
		destkey[i] = srckey[i+3];
		destkey[i+1] = srckey[i+2];
		destkey[i+2] = srckey[i+1];
		destkey[i+3] = srckey[i];
	}
#if 0
	for(i=0; i<size; i+=4)
	{
		ALOGE("0x%02x 0x%02x 0x%02x 0x%02x\n", srckey[i+0], srckey[i+1], srckey[i+2], srckey[i+3]);
		ALOGE("0x%02x 0x%02x 0x%02x 0x%02x\n", destkey[i+0], destkey[i+1], destkey[i+2], destkey[i+3]);
	}
#endif
	memcpy(srckey, destkey, size);

	TCC_free(destkey);
	return 0;
}


/**************************************************************************
 FUNCTION NAME : CA_get_unique_id
 
 DESCRIPTION : TeleChips 의 CAS PL에서 고유의 식별자를 읽어온다. 
 
 INPUT	 :  CA_UINT32 unique_id_max_size : The maximum size of unique id
 OUTPUT  : CA_INT8* unique_id : The unique ID of TeleChips terminal
		     CA_UINT32 *unique_id_size : The size of unique
return  : 0 : success
		-1 : fail
**************************************************************************/
INT32 tcc_manager_cas_get_unique_id(INT8 *unique_id, UINT32 unique_id_max_size, UINT32 *unique_id_size)
{
#if 0
	if(unique_id_size<unique_id_max_size)
	{
		memset(	unique_id+unique_id_size, 0, unique_id_max_size- unique_id_size)
	}
#endif	
	unique_id = 0;
	unique_id_max_size =0;
	unique_id_size=0;

	return 0;
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
INT32 tcc_manager_cas_set_cw(ENCRYPTION_TYPE alg,
	ENCRYPTION_MODE_TYPE mode,
	UINT32 video_pid,
	UINT32 audio_pid,
	UINT32 descramble_key_len,
	UINT8 * descramble_key,
	UINT32 descramble_iv_len,
	UINT8 * descramble_iv,
	UINT32 flag)

{
	unsigned int	algorithm, operation_mode, algorithm_mode, Key_flag, multi2round=0;
	int	err= 0;

	DEBUG_PRINTF("%s %d alg = %d audio_pid = %d video_pid= %d flag = %d \n", __func__, __LINE__, alg, audio_pid, video_pid, flag);
	DEBUG_PRINTF("%s %d mode = %d descramble_key_len = %d descramble_iv_len= %d \n", __func__, __LINE__, mode, descramble_key_len,  descramble_iv_len);
	
	tcc_manager_cas_kwy_swap(descramble_key, descramble_key_len);

	if(alg == ALG_NONE)
	{
		err = TCC_DxB_CAS_Close();
		if(err !=0)
		{
			DEBUG_PRINTF("error [%s][%d] \n", __func__, __LINE__);
			return -1;
		}
	}
	else
	{
		switch(alg)
		{
			case ALG_AES:
				algorithm = TCC_CHIPHER_ALGORITM_AES;
				switch(descramble_key_len)
				{
					case ASE_128KEY_SIZE:
							algorithm_mode = TCC_CHIPHER_AES_128KEY;
						break;
					case ASE_192KEY_SIZE:
							algorithm_mode = TCC_CHIPHER_AES_192KEY;
						break;
					case ASE_256KEY_SIZE:
							algorithm_mode = TCC_CHIPHER_AES_256KEY_1;
						break;
					default:
						break;
				}
				break;
			case ALG_3DES:
				algorithm = TCC_CHIPHER_ALGORITM_DES;
				switch(descramble_key_len)
				{
					case DES_TRIPLE_2KEY_SIZE:
							algorithm_mode = TCC_CHIPHER_DES_TRIPLE_2KEY;
						break;
					case DES_TRIPLE_3KEY_SIZE:
							algorithm_mode = TCC_CHIPHER_DES_TRIPLE_3KEY;
						break;
					default:
						break;
				}
				break;
			case ALG_DES:
				algorithm = TCC_CHIPHER_ALGORITM_DES;
				switch(descramble_key_len)
				{
					case DES_SINGLE_KEY_SIZE:
							algorithm_mode = TCC_CHIPHER_DES_SINGLE;
						break;
					case DES_DOUBLE_KEY_SIZE:
							algorithm_mode = TCC_CHIPHER_DES_DOUBLE;
						break;
					default:
						break;
				}
				break;
			case ALG_DVB:
				break;
			default:
				break;
		}
	}
		
	switch(mode)
	{
		case MODE_ECB:
			operation_mode = TCC_CHIPHER_OPMODE_ECB;
			break;
		case MODE_CBC:
			operation_mode = TCC_CHIPHER_OPMODE_CBC;
			break;
		case MODE_CFB:
			operation_mode = TCC_CHIPHER_OPMODE_CFB;
			break;
		case MODE_OFB:
			operation_mode = TCC_CHIPHER_OPMODE_OFB;
			break;
		default:
			break;
	}

	err = TCC_DxB_CAS_Open(algorithm, operation_mode, algorithm_mode, multi2round);
	if(err !=0)
	{
		DEBUG_PRINTF("error [%s][%d] \n", __func__, __LINE__);
		return -1;
	}

	switch(flag)
	{
		case FLAG_EVEN:
			Key_flag = 0x02;
			break;
		case FLAG_ODD:
			Key_flag = 0x03;
			break;
		case FLAG_ALL:
			break;
		default:
			break;
	}

	err = TCC_DxB_CAS_SetKey(descramble_key, descramble_key_len, descramble_iv, descramble_iv_len, 0, 0, Key_flag);
	if(err !=0)
	{
		DEBUG_PRINTF("error [%s][%d] \n", __func__, __LINE__);
		return -1;
	}

	return 0;
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
INT32 tcc_manager_cas_start_packet_filtering (INT32* handle, 
	UINT32 pid, 
	INT8* table_id_mask, 
	INT8* table_id, 
	CA_section_callback func, 
	UINT32 buffsize)

{
	int     		iOnce, iCheckCRC;
	char	  ucValue[16];
	char	  ucMask[16];
	int			err;
	char	  ucExclusion[16];

	ucValue[0] = table_id; 
	ucMask[0] = table_id_mask;
	ucExclusion[0] = 0x00;

	CAS_Callback  = func;

	DEBUG_PRINTF("%s %d pid = %x table_id = 0x%x table_id_mask = 0x%x \n", __func__, __LINE__, pid, ucValue[0], ucMask[0]);

	iOnce = FALSE;
	iCheckCRC = 1;
	err = TCC_DxB_DEMUX_StartSectionFilterEx(0, pid, REQUEST_ID, iOnce, 1, ucValue, ucMask, ucExclusion, FALSE, 
													tcc_manager_cas_section_notify, 
													tcc_manager_cas_alloc_buffer,
													handle);		

	DEBUG_PRINTF("%s %d handle = %x  \n", __func__, __LINE__, handle);

	if(err)
	{
		ALOGE("error [%s][%d] err= %d \n", __func__, __LINE__, err);
		handle = -1;
		return -1;
	}
	
	return 0;
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
INT32 tcc_manager_cas_stop_packet_filtering (INT32 handle)
{
	int	err = 0 ;

	err = TCC_DxB_DEMUX_StopSectionFilterEx(0, handle);
	if(err)
	{
		DEBUG_PRINTF("error = %d  [%s][%d] \n",err,  __func__, __LINE__);
		return -1;
	}

	return 0;
}



