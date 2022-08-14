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

#include "cdk_wrapper_stagefright.h"
#include "media_io_wrapper_stagefright.h"
#include "utils/Log.h"

#define LOG_TAG "TCC_MEDIA_IO_WRAPPER"

//#define MEM_DEBUG
#ifdef MEM_DEBUG
#define MEM_LOGMSG(x...) ALOGD(x)
#else
#define MEM_LOGMSG(x...) ((void)0)
#endif

//#define FILE_DEBUG
#ifdef FILE_DEBUG
#define FILE_LOGMSG(...) ALOGD(__VA_ARGS__)
#else
#define FILE_LOGMSG(...) ((void)0)
#endif

#ifdef FILE_DEBUG
void PrintHexDataFrontRear(char * pBuffer, int nLength, char* pTag)
{
	FILE_LOGMSG("[%s:%08d] 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x "
	     "~ 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", pTag, nLength
			, pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]
			, pBuffer[4], pBuffer[5], pBuffer[6]
			, pBuffer[7], pBuffer[8], pBuffer[9]
			, pBuffer[nLength - 10], pBuffer[nLength - 9], pBuffer[nLength - 8]
			, pBuffer[nLength - 7], pBuffer[nLength - 6], pBuffer[nLength - 5]
			, pBuffer[nLength - 4], pBuffer[nLength - 3], pBuffer[nLength - 2], pBuffer[nLength - 1]
		);
}
#endif

void * WRAPPER_malloc(unsigned int size)
{
	MEM_LOGMSG("malloc size %d", size);
	void * ptr = malloc(size);
	MEM_LOGMSG("malloc ptr %x", ptr);
	return ptr;
}

void WRAPPER_free(void * ptr)
{
	MEM_LOGMSG("free %x", ptr);
	free(ptr);
	MEM_LOGMSG("free out");
}

void * WRAPPER_realloc(void * ptr, unsigned int size)
{
	MEM_LOGMSG("realloc ptr %x, size %d", ptr, size);
	return realloc(ptr, size);
}

void * WRAPPER_memcpy(void* dest, const void* src, unsigned int size)
{
	MEM_LOGMSG("memcpy dest ptr %x, src ptr %x, size %d", dest, src, size);
	return memcpy(dest, src, size);
}

void * WRAPPER_memset(void* ptr, int val, unsigned int size)
{
	MEM_LOGMSG("memset ptr %x, val %d, size %d", ptr, val, size);
	return memset(ptr, val, size);
}

void * WRAPPER_memmove(void* dest, const void* src, unsigned int size)
{
	MEM_LOGMSG("memmove dest %x, src %x, size %d", dest, src, size);
	return memmove(dest, src, size);
}

int WRAPPER_memcmp(const void* src1, const void* src2, unsigned int size)
{
	MEM_LOGMSG("memcmp src1 %x, src2 %x, size %d", src1, src2, size);
	return memcmp(src1, src2, size);
}

void * WRAPPER_fopen(const char * name, const char * mode)
{
	SF_MEDIA_FILE* fp;
	uint32_t ptr;
	char* p;
	ptr = strtoul(name, &p, 16);
	fp = (SF_MEDIA_FILE*)(ptr);

	FILE_LOGMSG("%s copy datasource pointer", __func__);
	SF_MEDIA_FILE* file = new SF_MEDIA_FILE();
	FILE_LOGMSG("new file ptr %x", file);

	file->file = fp->file;
	file->offset = 0;
	file->size = fp->size;

	return (void *)(file);
}

unsigned int WRAPPER_fread(void * ptr, unsigned int size, unsigned int count, void * fp)
{
	SF_MEDIA_FILE* file = (SF_MEDIA_FILE*)fp;
#ifdef FILE_DEBUG
	FILE_LOGMSG("%s file pointer %x, file offset %lld, size %d, count %d", __func__, fp, file->offset, size, count);
#endif

	ssize_t ret;

	ret	= file->file->readAt(file->offset, ptr, size*count);

	if(ret <= 0)
		return 0;

	file->offset += (off64_t)ret;

	FILE_LOGMSG("fread out, read %d objects", ret/size);
#ifdef FILE_DEBUG
    PrintHexDataFrontRear((char *)ptr, ret/size, "fread out");
#endif
	return ret/size; 
}

int WRAPPER_fseek(void * fp, long offset, int origin)
{
	SF_MEDIA_FILE* file = (SF_MEDIA_FILE*)fp;
#ifdef FILE_DEBUG
	FILE_LOGMSG("%s file pointer %x, current file offset %d, target offset %d, origin %d", __func__, fp, (long)file->offset, offset, origin);
#endif

	off_t sum;

	switch (origin)
	{
		case SEEK_SET : 
			if (offset >= 0 && offset <= (off_t)file->size)
			{
				file->offset = offset; 
				return 0;
			}
			break;

		case SEEK_CUR : 
			sum = file->offset + offset;
			if (sum >= 0 && sum <= (off_t)file->size)
			{
				file->offset = sum; 
				return 0;
			}
			break;

		case SEEK_END : 
			sum = (off_t)file->size + offset;
			if (sum >= 0 && sum <= (off_t)file->size)
			{
				file->offset = sum; 
				return 0;
			}
			break;

		default: break;
	}

	return -1;
}

int WRAPPER_sseek(void * fp, long offset, int origin)
{
	SF_MEDIA_FILE* file = (SF_MEDIA_FILE*)fp;
#ifdef FILE_DEBUG
	FILE_LOGMSG("%s file pointer %x, current file offset %d, target offset %d, origin %d", __func__, fp, (long)file->offset, offset, origin);
#endif

	off_t sum;

	switch (origin)
	{
		case SEEK_SET : 
			if (offset >= 0)
			{
				file->offset = offset; 
				return 0;
			}
			break;

		case SEEK_CUR : 
			sum = file->offset + offset;
			if (sum >= 0)
			{
				file->offset = sum; 
				return 0;
			}
			break;

		case SEEK_END : 
			sum = (off_t)file->size + offset;
			if (sum >= 0)
			{
				file->offset = sum; 
				return 0;
			}
			break;

		default: break;
	}

	return -1;
}

long WRAPPER_ftell(void * fp)
{
	FILE_LOGMSG("%s file pointer %x", __func__, fp);
	SF_MEDIA_FILE* file = (SF_MEDIA_FILE*)fp;

	return (long)file->offset;
}

int WRAPPER_fseek64(void * fp, int64_t offset, int origin)
{
	SF_MEDIA_FILE* file = (SF_MEDIA_FILE*)fp;
#ifdef FILE_DEBUG
	FILE_LOGMSG("%s file pointer %x, current file offset %lld, target offset %lld, origin %d", __func__, fp, file->offset, offset, origin);
#endif
	
	loff_t sum;

	switch (origin)
	{
		case SEEK_SET : 
			if (offset >= 0 && offset <= file->size)
			{
				file->offset = offset; 
				return 0;
			}
			break;

		case SEEK_CUR : 
			sum = file->offset + offset;
			if (sum >= 0 && sum <= file->size)
			{
				file->offset = sum; 
				return 0;
			}
			break;

		case SEEK_END : 
			sum = file->size + offset;
			if (sum >= 0 && sum <= file->size)
			{
				file->offset = sum; 
				return 0;
			}
			break;

		default: break;
	}

	return -1;
}

int WRAPPER_sseek64(void * fp, int64_t offset, int origin)
{
	SF_MEDIA_FILE* file = (SF_MEDIA_FILE*)fp;
#ifdef FILE_DEBUG
	FILE_LOGMSG("%s file pointer %x, current file offset %lld, target offset %lld, origin %d", __func__, fp, file->offset, offset, origin);
#endif
	
	loff_t sum;

	switch (origin)
	{
		case SEEK_SET : 
			if (offset >= 0)
			{
				file->offset = offset; 
				return 0;
			}
			break;

		case SEEK_CUR : 
			sum = file->offset + offset;
			if (sum >= 0)
			{
				file->offset = sum; 
				return 0;
			}
			break;

		case SEEK_END : 
			sum = file->size + offset;
			if (sum >= 0)
			{
				file->offset = sum; 
				return 0;
			}
			break;

		default: break;
	}

	return -1;
}

int64_t WRAPPER_ftell64(void * fp)
{
	FILE_LOGMSG("%s file pointer %x", __func__, fp);
	
	SF_MEDIA_FILE* file = (SF_MEDIA_FILE*)fp;

	return (int64_t)file->offset;
}

int WRAPPER_fclose(void * fp)
{
	FILE_LOGMSG("%s file pointer %x", __func__, fp);

	SF_MEDIA_FILE* file = (SF_MEDIA_FILE*)fp;
	
	if (file)
	{
		delete file;
	}
	return 0;
}

unsigned int WRAPPER_fflush(void * fp)
{
	FILE_LOGMSG("%s file pointer %x", __func__, fp);
	return 0;
}

unsigned int WRAPPER_feof(void * fp)
{
	FILE_LOGMSG("%s file pointer %x", __func__, fp);
	SF_MEDIA_FILE* file = (SF_MEDIA_FILE*)fp;

	if (file->offset >= file->size)
		return 1;
	else
		return 0;
}

unsigned int WRAPPER_seof(void * fp)
{
    if (fp == NULL) {
        FILE_LOGMSG("WRAPPER_seof: Nullified fp");
        return 1;
    }

	SF_MEDIA_FILE* file = (SF_MEDIA_FILE*)fp;
    if (file->size > 0) {
        if (file->offset >= file->size) {
            FILE_LOGMSG("WRAPPER_seof: offset overflowed");
            return 1;
        } else {
            FILE_LOGMSG("WRAPPER_seof: Not yet ...");
            return 0;
        }
    } else {
        FILE_LOGMSG("WRAPPER_seof: we don't know eof now. (invalid file size)");
        return 0;
    }
}

// -----------------------------------------
// this functions use internal offset 
// -----------------------------------------

#if 0
void * WRAPPER_fopen_with_offset(const char * name, const char * mode)
{
	MEDIA_FF_FILE* fp;
	uint32 ptr;
	char* p;
	ptr = strtol(name, &p, 16);
	fp = (MEDIA_FF_FILE*)(ptr);

	FILE_LOGMSG("%s copy file handle", __func__);
	MEDIA_FF_FILE* file = OSCL_NEW(MEDIA_FF_FILE, ());
	FILE_LOGMSG("new file ptr %x", file);

	file->pvfile.Copy(fp->pvfile);
	file->offset = 0;

	TOsclFileOffset fileSize;
	MediaParserUtils::getCurrentFileSize(file, fileSize);
	file->fileSize = fileSize;
	MediaParserUtils::seekFromStart(file, 0);

	return (void *)(file);
}

unsigned int WRAPPER_fread_with_offset(void * ptr, unsigned int size, unsigned int count, void * fp)
{
	MEDIA_FF_FILE* file = (MEDIA_FF_FILE*)fp;
#ifdef FILE_DEBUG
	FILE_LOGMSG("%s file pointer %x, file offset %lld, size %d, count %d", __func__, fp, file->offset, size, count);
#endif

	file->pvfile.Seek(file->offset, Oscl_File::SEEKSET);

	uint32 ret = file->pvfile.Read(ptr, size, count);

	file->offset = file->pvfile.Tell();

	FILE_LOGMSG("fread out, read size %d", ret);
	return ret; 
}

int WRAPPER_fseek_with_offset(void * fp, long offset, int origin)
{
	MEDIA_FF_FILE* file = (MEDIA_FF_FILE*)fp;
#ifdef FILE_DEBUG
	FILE_LOGMSG("%s file pointer %x, current file offset %d, target offset %d, origin %d", __func__, fp, file->offset, offset, origin);
#endif

	file->pvfile.Seek(file->offset, Oscl_File::SEEKSET);
	
	int32 ret;
	Oscl_File::seek_type type;
	switch (origin)
	{
		case SEEK_SET : type = Oscl_File::SEEKSET; break;
		case SEEK_CUR : type = Oscl_File::SEEKCUR; break;
		case SEEK_END : type = Oscl_File::SEEKEND; break;
		default: return -1; break;
	}
	ret = file->pvfile.Seek(offset, type);
		
	file->offset = file->pvfile.Tell();

	if (ret == 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

long WRAPPER_ftell_with_offset(void * fp)
{
	FILE_LOGMSG("%s file pointer %x", __func__, fp);
	MEDIA_FF_FILE* file = (MEDIA_FF_FILE*)fp;

	file->pvfile.Seek(file->offset, Oscl_File::SEEKSET);

	return file->pvfile.Tell();
}

#ifdef __USE_FILE_OFFSET64
int WRAPPER_fseek64_with_offset(void * fp, int64 offset, int origin)
{
	MEDIA_FF_FILE* file = (MEDIA_FF_FILE*)fp;
#ifdef FILE_DEBUG
	FILE_LOGMSG("%s file pointer %x, current file offset %lld, target offset %lld, origin %d", __func__, fp, file->offset, offset, origin);
#endif
	
	file->pvfile.Seek(file->offset, Oscl_File::SEEKSET);

	int32 ret;
	Oscl_File::seek_type type;
	switch (origin)
	{
		case SEEK_SET : type = Oscl_File::SEEKSET; break;
		case SEEK_CUR : type = Oscl_File::SEEKCUR; break;
		case SEEK_END : type = Oscl_File::SEEKEND; break;
		default: return -1; break;
	}
	ret = file->pvfile.Seek(offset, type);

	file->offset = file->pvfile.Tell();

	if (ret == 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int64 WRAPPER_ftell64_with_offset(void * fp)
{
	FILE_LOGMSG("%s file pointer %x", __func__, fp);
	
	MEDIA_FF_FILE* file = (MEDIA_FF_FILE*)fp;

	file->pvfile.Seek(file->offset, Oscl_File::SEEKSET);
	
	return file->pvfile.Tell();
}
#endif

int WRAPPER_fclose_with_offset(void * fp)
{
	FILE_LOGMSG("%s file pointer %x", __func__, fp);

	// do not close file here. it will be closed in TCC parser.
	// return TCC_CDK_Wrapper::iFilePtr->Close();
	MEDIA_FF_FILE* file = (MEDIA_FF_FILE*)fp;
	
	if (file)
	{
		OSCL_DELETE(file);
	}

	return 0;
}

unsigned int WRAPPER_fflush_with_offset(void * fp)
{
	FILE_LOGMSG("%s file pointer %x", __func__, fp);
	return ((MEDIA_FF_FILE*)fp)->pvfile.Flush();
}

unsigned int WRAPPER_feof_with_offset(void * fp)
{
	FILE_LOGMSG("%s file pointer %x", __func__, fp);
	TOsclFileOffset remain = 0;
	MEDIA_FF_FILE* file = (MEDIA_FF_FILE*)fp;

	file->pvfile.Seek(file->offset, Oscl_File::SEEKSET);
	
	file->pvfile.GetRemainingBytes(remain);

	if (remain)
		return 0;
	else
		return 1;
}
#endif
