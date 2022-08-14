/* ------------------------------------------------------------------
 * Copyright (C) 2009-2010 Telechips 
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#ifndef _MEDIA_IO_WRAPPER_STAGEFRIGHT_H_
#define _MEDIA_IO_WRAPPER_STAGEFRIGHT_H_

#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaErrors.h>

// stagefright media file
class SF_MEDIA_FILE
{
	public:
		SF_MEDIA_FILE() : offset(0), size(0) {}
		~SF_MEDIA_FILE()
		{
			offset = 0;
			size = 0;
		}

		sp<DataSource>	file;	
		int64_t 		offset;
		off64_t			size;
};

void * WRAPPER_malloc(unsigned int size);
void WRAPPER_free(void * ptr);
void * WRAPPER_realloc(void * ptr, unsigned int size);
void * WRAPPER_memcpy(void* dest, const void* src, unsigned int size);
void * WRAPPER_memset(void* ptr, int val, unsigned int size);
void * WRAPPER_memmove(void* dest, const void* src, unsigned int size);
int WRAPPER_memcmp(const void* src1, const void* src2, unsigned int size);

void * WRAPPER_fopen(const char * name, const char * mode);
unsigned int WRAPPER_fread(void * ptr, unsigned int size, unsigned int count, void * fp);
int WRAPPER_fseek(void * fp, long offset, int origin);
int WRAPPER_sseek(void * fp, long offset, int origin);
long WRAPPER_ftell(void * fp);
int WRAPPER_fseek64(void * fp, int64_t offset, int origin);
int WRAPPER_sseek64(void * fp, int64_t offset, int origin);
int64_t WRAPPER_ftell64(void * fp);
int WRAPPER_fclose(void * fp);
unsigned int WRAPPER_fflush(void * fp);
unsigned int WRAPPER_feof(void * fp);
unsigned int WRAPPER_seof(void * fp);

#endif // _MEDIA_IO_WRAPPER_STAGEFRIGHT_H_

