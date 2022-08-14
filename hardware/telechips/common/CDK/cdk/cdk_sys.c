/****************************************************************************
 *   FileName    : cdk_sys.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided “AS IS” and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information’s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
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

sys_malloc_physical_addr_func*	cdk_sys_malloc_physical_addr = NULL;
sys_free_physical_addr_func*	cdk_sys_free_physical_addr = NULL;
sys_malloc_virtual_addr_func*	cdk_sys_malloc_virtual_addr = NULL;
sys_free_virtual_addr_func*		cdk_sys_free_virtual_addr = NULL;

malloc_func*	cdk_malloc	=NULL;
malloc_func*	cdk_nc_malloc=NULL;
realloc_func*	cdk_realloc	=NULL;
free_func*		cdk_free	=NULL;
free_func*		cdk_nc_free	=NULL;
memcpy_func*	cdk_memcpy	=NULL;
memset_func*	cdk_memset	=NULL;
memmove_func*	cdk_memmove	=NULL;

fopen_func*		cdk_fopen	=NULL;
fread_func*		cdk_fread	=NULL;
fseek_func*		cdk_fseek	=NULL;
ftell_func*		cdk_ftell	=NULL;
fseek64_func*	cdk_fseek64	=NULL;
ftell64_func*	cdk_ftell64	=NULL;
fwrite_func*	cdk_fwrite	=NULL;
fclose_func*	cdk_fclose	=NULL;
unlink_func*	cdk_unlink	=NULL;
feof_func*		cdk_feof	=NULL;
fflush_func*	cdk_fflush	=NULL;

unsigned int 
cdk_strlen( const char* pStr )
{
	int i = 0;
	while( *pStr )
	{
		i++; pStr++;
	}
	return i;
}

int
cdk_strcmp( const char* pSrc, const char* pDst )
{
	int hit = 0;

	while( *pSrc != 0 && *pDst != 0 )
	{
		hit = 1;
		if( *pSrc++ != *pDst++ ) 
		{
			break;
		}
	}
	
	if( hit )
	{
		return ( *(--pSrc) - *(--pDst) );
	}
	else
	{
		return ( *(pSrc) - *(pDst) );
	}
}

int
cdk_strncmp( const char* pSrc1, const char* pSrc2, unsigned int Len )
{
	unsigned int i;
	for( i = 0; i < Len && *pSrc1 == *pSrc2; i++, pSrc1++, pSrc2++ )
	{
		if( *pSrc1 == '\0' )
			return 0;
	}

	if( i == Len )
		return 0;

	return (*pSrc1 - *pSrc2);
}

char*
cdk_strstr( const char *pStr1, const char* pStr2 )
{
	char *p1 = (char *)pStr1;
	char *p2, *p3;

	for( ; *p1; p1++ )
	{
		for( p2 = p1, p3 = (char *)pStr2; *p3 && *p2 == *p3; p2++, p3++ );
		if( !(*p3) )
		{ 
			return (p1);
		}
	}
	return (char *)NULL;
}

int cdk_toupper(int ch) 
{
	if ( (unsigned int)(ch - 'a') < 26u )
		ch += 'A' - 'a';
	return ch;
}


char* cdk_strcpy(char* dest, const char* source) 
{ 
	char *p=dest; 
	while(*source){ 
		*dest++ = *source++; 
	} 
	*dest='\0'; 
	return p; 
} 


char* cdk_strcat(char* dest, const char* source) 
{ 
	char *p=dest; 
	while(*dest) dest++; 
	while(*source){ 
		*dest++ = *source++; 
	} 
	*dest='\0'; 
	return p; 
} 


char* cdk_strchr( char *str, char c )
{
	//문자열 str안에 문자 c가 처음 나타나는 위치의 주소를 return하고
	//문자 c가 없으면 NULL을 return하는 함수
	while( *str )
	{
		if (*str == c) return str;
		str++;
	}
	return NULL;
}

