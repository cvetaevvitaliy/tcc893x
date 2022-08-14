/**

  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

#ifndef	_TCC_FILE_H__
#define	_TCC_FILE_H__

/******************************************************************************
* include 
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************
* typedefs & structure
******************************************************************************/


/******************************************************************************
* defines 
******************************************************************************/

#define	TCC_LINUX_FILE_SYTEM

//#define	TCC_FILE_DEBUG	// for Telechips File Systme Debugging 



#ifdef TCC_LINUX_FILE_SYTEM
#define	TCC_FILE			FILE					// File Handler

#define	TCC_SEEK_SET		SEEK_SET				// File Seek 
#define	TCC_SEEK_CUR		SEEK_CUR			//
#define	TCC_SEEK_END		SEEK_END			//

#define	TCC_FILE_OK			0
#define	TCC_FILE_ERROR		-1					
#define	TCC_FILE_EOF		EOF
#else

#endif



/******************************************************************************
* globals
******************************************************************************/

/******************************************************************************
* locals
******************************************************************************/


/******************************************************************************
* declarations
******************************************************************************/
TCC_FILE *TCC_fopen(char *path,char *mode);
int TCC_fclose(TCC_FILE *stream);
int TCC_fseek(TCC_FILE *stream, long offset, int whence);
long  TCC_ftell(TCC_FILE *stream);
void  TCC_rewind(TCC_FILE *stream);
int TCC_feof(TCC_FILE *stream);
int TCC_fgetc(TCC_FILE *stream);
char *TCC_fgets(char *s, int size, TCC_FILE *stream);
int TCC_fputs(char *s, TCC_FILE *stream);
int TCC_fread(char *s, int size_t, int size, TCC_FILE *stream);
int TCC_fwrite(char *s, int size_t, int size, TCC_FILE *stream);
void TCC_fStartMeasure(void);
unsigned long long TCC_fGetMeasure(unsigned long long *min, unsigned long long *max);
#endif //_TCC_FILE_H__
