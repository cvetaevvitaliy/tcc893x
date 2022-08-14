/****************************************************************************
 *   FileName    : cdk_sys.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¡°AS IS¡± and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information¡¯s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#ifndef _CDK_SYS_H_
#define _CDK_SYS_H_

#include "cdk_port.h"

/************************************************************************/
/*						                                                */
/* CDK Funcs			                                                */
/*						                                                */
/************************************************************************/
typedef void*		 (malloc_func	) ( unsigned int );
typedef void		 (free_func		) ( void* );
typedef void*		 (memcpy_func	) ( void*, const void*, unsigned int );
typedef void*		 (memset_func	) ( void*, int, unsigned int );
typedef void*		 (realloc_func  ) ( void*, unsigned int );
typedef void*		 (memmove_func	) ( void*, const void*, unsigned int );
typedef void*		 (fopen_func	) (const char *, const char *);
typedef unsigned int (fread_func	) (void*, unsigned int, unsigned int, void* );		
typedef int			 (fseek_func	) (void*, long, int );								
typedef long		 (ftell_func	) (void* );											
typedef int			 (fseek64_func	) (void*, int64_t, int );							
typedef int64_t		 (ftell64_func	) (void* );											
typedef unsigned int (fwrite_func	) (const void*, unsigned int, unsigned int, void*);	
typedef int			 (fclose_func	) (void *);											
typedef int			 (unlink_func	) (const char *);
typedef unsigned int (feof_func		) (void *);											
typedef unsigned int (fflush_func	) (void *);											

typedef void* (sys_malloc_physical_addr_func)	(unsigned int);
typedef void  (sys_free_physical_addr_func)		(void*, unsigned int);
typedef void* (sys_malloc_virtual_addr_func)	(int*, unsigned int, unsigned int);
typedef void  (sys_free_virtual_addr_func)		(int*, unsigned int, unsigned int);

extern sys_malloc_physical_addr_func*	cdk_sys_malloc_physical_addr;
extern sys_free_physical_addr_func*		cdk_sys_free_physical_addr;
extern sys_malloc_virtual_addr_func*	cdk_sys_malloc_virtual_addr;
extern sys_free_virtual_addr_func*		cdk_sys_free_virtual_addr;

extern malloc_func*		cdk_malloc;
extern malloc_func*		cdk_nc_malloc;
extern realloc_func*	cdk_realloc;
extern free_func*		cdk_free;
extern free_func*		cdk_nc_free;
extern memcpy_func*		cdk_memcpy;
extern memset_func*		cdk_memset;
extern memmove_func*	cdk_memmove;

extern fopen_func*		cdk_fopen;
extern fread_func*		cdk_fread;
extern fseek_func*		cdk_fseek;
extern ftell_func*		cdk_ftell;
extern fseek64_func*	cdk_fseek64;
extern ftell64_func*	cdk_ftell64;
extern fwrite_func*		cdk_fwrite;
extern fclose_func*		cdk_fclose;
extern unlink_func*		cdk_unlink;
extern feof_func*		cdk_feof;
extern fflush_func*		cdk_fflush;

unsigned int cdk_strlen( const char* pStr );
int cdk_strcmp( const char* pSrc, const char* pDst );
int cdk_strncmp( const char* pSrc1, const char* pSrc2, unsigned int Len );
char* cdk_strstr( const char *pStr1, const char* pStr2 );
int cdk_toupper(int ch);	
char* cdk_strcpy(char* dest, const char* source);
char* cdk_strcat(char* dest, const char* source);
char* cdk_strchr( char *str, char c );

#endif//_CDK_SYS_H_
