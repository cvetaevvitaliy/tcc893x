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

#ifndef	_TCC_DXB_INTERFACE_CIPHER_H_
#define	_TCC_DXB_INTERFACE_CIPHER_H_

/***************************************************************
Macro definition
***************************************************************/


/***************************************************************
Type definition
***************************************************************/
/* CIPHER IOCTL Command */
enum
{
	TCC_CHIPHER_IOCTL_SET_ALGORITHM = 0x100,
	TCC_CHIPHER_IOCTL_SET_KEY,
	TCC_CHIPHER_IOCTL_SET_VECTOR,
	TCC_CHIPHER_IOCTL_ENCRYPT,
	TCC_CHIPHER_IOCTL_DECRYPT,
	TCC_CHIPHER_IOCTL_CLOSE,
	TCC_CHIPHER_IOCTL_MAX,
};

/* The Algorithm of the cipher block */
enum TCC_CHIPHER_ALGORITHM_TYPE
{
	TCC_CHIPHER_ALGORITM_AES = 0,
	TCC_CHIPHER_ALGORITM_DES,
	TCC_CHIPHER_ALGORITM_MULTI2,
	TCC_CHIPHER_ALGORITM_MULTI2_1,
	TCC_CHIPHER_ALGORITM_MAX
};

/* The Algorithm of the operation mode */
enum TCC_CHIPHER_OPERATION_MODE_TYPE
{
	TCC_CHIPHER_OPMODE_ECB = 0,
	TCC_CHIPHER_OPMODE_CBC,
	TCC_CHIPHER_OPMODE_CFB,
	TCC_CHIPHER_OPMODE_OFB,
	TCC_CHIPHER_OPMODE_CTR,
	TCC_CHIPHER_OPMODE_CTR_1,
	TCC_CHIPHER_OPMODE_CTR_2,
	TCC_CHIPHER_OPMODE_CTR_3,
	TCC_CHIPHER_OPMODE_MAX
};

/* The Key Option for AES */ 
enum TCC_CHIPHER_AES_MODE_TYPE
{
	TCC_CHIPHER_AES_128KEY =0,
	TCC_CHIPHER_AES_192KEY,
	TCC_CHIPHER_AES_256KEY_1,
	TCC_CHIPHER_AES_256KEY_2,
	TCC_CHIPHER_AES_KEYMAX,
};

/* The Key Option for DES */ 
enum TCC_CHIPHER_DES_MODE_TYPE
{
	TCC_CHIPHER_DES_SINGLE =0,
	TCC_CHIPHER_DES_DOUBLE,
	TCC_CHIPHER_DES_TRIPLE_2KEY,
	TCC_CHIPHER_DES_TRIPLE_3KEY,
	TCC_CHIPHER_DES_KEYMAX,
};

/* The Parity bit Option for DES */ 
enum TCC_CHIPHER_DES_PARITY_TYPE
{
	TCC_CHIPHER_DES_LSB_PRT =0,
	TCC_CHIPHER_DES_MSB_PRT,
	TCC_CHIPHER_DES_PARITYMAX,
};

/* The Key Option for Multi2 */ 
enum TCC_CHIPHER_MILTI2_KEY_TYPE
{
	TCC_CHIPHER_KEY_MULTI2_DATA = 0,
	TCC_CHIPHER_KEY_MULTI2_SYSTEM,
	TCC_CHIPHER_KEY_MULTI2_MAX
};


typedef struct
{
	unsigned int	uOperationMode;
	unsigned int	uAlgorithm;
	unsigned int	uArgument1;
	unsigned int	uArgument2;
} stCIPHER_ALGORITHM;

typedef struct
{
	unsigned char	 *pucData;
	unsigned int		 uLength;
	unsigned int		 uOption;
} stCIPHER_KEY;


typedef struct
{
	unsigned char	 *pucData;
	unsigned int		 uLength;
	unsigned int		 uOption;
} stCIPHER_SYSTEMKEY;


typedef struct
{
	unsigned char	 *pucData;
	unsigned int		 uLength;
} stCIPHER_VECTOR;


typedef struct
{
	unsigned char 	*pucSrcAddr;
	unsigned char 	*pucDstAddr;
	unsigned int		uLength;
} stCIPHER_ENCRYPTION;


/**
algorithm			: AES, DES, Multi2 	refer to ALGORITHM_TYPE
operation_mode		: ECB, CBC, ... 		refer to OPERATION_MODE_TYPE
algorithm_mode		: AES		-> refer to AES_MODE_TYPE
						DES	-> refer to DES_MODE_TYPE
						Multi2 	-> not used
multi2Rounds		: Multi2		-> round_count (Variable)
						AES, DES  -> not  used
**/
typedef struct TCC_CIPHER_ModeSettings
{
	unsigned int		algorithm;		
	unsigned int		operation_mode;
	unsigned int		algorithm_mode;
	unsigned int		multi2Rounds;
} TCC_CIPHER_ModeSettings;
 
/**
p_key_data			: data key
key_length			: data key_length
p_init_vector		: initial vector 
vector_length		: vector length
p_Systemkey_data	: system key data (used to only multi2)
Systemkey_length	: system key_length(used to only multi2)
**/
 typedef struct TCC_CIPHER_KeySettings
 {
	 unsigned char *p_key_data;
	 unsigned int	 key_length;
	 unsigned char *p_init_vector;				 // 사용하지 않는 경우 NULL
	  unsigned int vector_length;				 // 사용하지 않는 경우 '0'
	 unsigned char *p_Systemkey_data;				 // 사용하지 않는 경우 NULL
	 unsigned int	 Systemkey_length;			 // 사용하지 않는 경우 '0'
	 unsigned int Key_flag;			 		// 10: even key, 11:odd key
} TCC_CIPHER_KeySettings;

/***************************************************************
Function definition
***************************************************************/
int Cipher_open(TCC_CIPHER_ModeSettings *CipherModeSetting);
int Cipher_close();
int Cipher_SetKey(TCC_CIPHER_KeySettings *Keysetting);
int Cipher_Run(unsigned char* ucSrcData, unsigned char * ucDstData1, unsigned int Src_length, unsigned int mode);

#endif


