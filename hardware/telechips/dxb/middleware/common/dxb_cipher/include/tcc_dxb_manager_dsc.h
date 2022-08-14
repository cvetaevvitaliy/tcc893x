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

/****************************************************************************

  Revision History

 ****************************************************************************/

#ifndef _TCC_DXB_MANAGER_DSC_H_
#define _TCC_DXB_MANAGER_DSC_H_

#include "tcc_dxb_interface_cipher.h"

typedef enum
{
	TCCMANAGER_DSC_ERR_NOERROR =0,
	TCCMANAGER_DSC_ERR_BADPARAMETER,
	TCCMANAGER_DSC_ERR_OUTOFMEMORY,
	TCCMANAGER_DSC_ERR_ERROR
}TCCMANAGER_DSC_ERROR;


typedef enum
{
	TCCMANAGER_DESC_DVB,
	TCCMANAGER_DESC_AES,
	TCCMANAGER_DESC_DES,
	TCCMANAGER_DESC_AES_CLEARMODE,	//CI+
	TCCMANAGER_DESC_DES_CLEARMODE,	//CI+
	TCCMANAGER_DESC_3DESABA,
	TCCMANAGER_DESC_MULTI2
} TCCMANAGER_DESC_TYPE;

typedef enum
{
	TCCMANAGER_DESC_ODD,
	TCCMANAGER_DESC_EVEN
} TCCMANAGER_DESC_ODD_EVEN;

typedef enum TCCMANAGER_DESC_AlgorithmVariant
{
	TCCMANAGER_DESC_AlgorithmVariant_eEcb,
	TCCMANAGER_DESC_AlgorithmVariant_eCbc,
	TCCMANAGER_DESC_AlgorithmVariant_eCounter,
	/* Add new algorithm variant definition before this line */
	TCCMANAGER_DESC_AlgorithmVariant_eMax
}   TCCMANAGER_DESC_AlgorithmVariant;

typedef struct TCCMANAGER_DESC_Multi2_Cbc
{
	unsigned int cbc_l;
	unsigned int cbc_r;	
} TCCMANAGER_DESC_Multi2_Cbc;

typedef struct TCCMANAGER_DESC_EncryptionSettings
{
	TCCMANAGER_DESC_AlgorithmVariant	algorithmVar;
	int				bIsUsed;
	unsigned short	multi2KeySelect;
	unsigned char	multi2Rounds;
	unsigned char	multi2SysKey[32];
} TCCMANAGER_DESC_EncryptionSettings;

typedef enum
{
	DSC_MODE_HW =0,
	DSC_MODE_SW_MULTI2,
	DSC_MODE_NONE,
}TCCMANAGER_DSC_MODE_TYPE;

int TSDEMUXDSCApi_ModeSet(TCCMANAGER_DSC_MODE_TYPE dsc_mode);
int TSDEMUXDSCApi_Open(unsigned int *pDescId, 
								TCCMANAGER_DESC_TYPE type, 
								TCCMANAGER_DESC_EncryptionSettings *encSetting,
								TCC_CIPHER_ModeSettings *CipherModeSetting);
int TSDEMUXDSCApi_SetPid(unsigned int DescId, unsigned short pid );
int TSDEMUXDSCApi_ClearPid(unsigned int DescId, unsigned short pid);
int TSDEMUXDSCApi_Close(unsigned int DescId);
int TSDEMUXDSCApi_SetKey
(
	unsigned int DescId,
	TCCMANAGER_DESC_ODD_EVEN parity,
	unsigned char *p_key_data,
	unsigned int key_length,
	unsigned char *p_init_vector,				// 사용하지 않는 경우 NULL
	unsigned int vector_length,				// 사용하지 않는 경우 '0'
	TCC_CIPHER_KeySettings *Keysetting);
int TSDEMUXDSCApi_Decrypt(unsigned int type, unsigned char *pBuf, unsigned int uiSize);
int TSDEMUXDSCApi_SetProtectionKey(	unsigned int DescId,unsigned char *pCWPK,unsigned int key_length);



#endif													  
