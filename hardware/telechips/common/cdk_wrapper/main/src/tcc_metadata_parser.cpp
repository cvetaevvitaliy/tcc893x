#include "tcc_metadata_parser.h"
#include "tcc_metadata_define.h"
#include <utils/Log.h>
#include <utils/String8.h>
#include <cdk.h>
#include <cdk_core.h>
#include <cdk_audio.h>
#include "stdlib.h"

//#define DEBUG
#ifdef DEBUG
#define LOG_TAG	"TCCMetadataParser"
#define LOGMSG(x...) LOGD(x)
#else
#define LOGMSG(x...)
#endif

int32_t TCCMetadataParser::ParseMetadata(int32_t iAudFmt, void* aFile)
{
	if (aFile == NULL)
		return -1;

#if 0
	if (!aFile->IsOpen())
	{
		LOGE("file not opened!");
		return -1;
	}
#endif

	iFile = aFile;

	switch(iAudFmt)
	{
		case AUDIO_ID_FLAC:
			ParseFLAC();
			break;
		case AUDIO_ID_WMA:
			ParseWMA();
			break;
		case AUDIO_ID_AAC:
		default:
			break;
	}

	return 0;
}

TCCMetadata* TCCMetadataParser::GetMetadata()
{
	return &cData;
}

#define FLAC_BLOCK_TYPE_STREAMINFO 		0
#define	FLAC_BLOCK_TYPE_PADDING			1
#define	FLAC_BLOCK_TYPE_APPLICATION		2
#define FLAC_BLOCK_TYPE_SEEKTABLE		3
#define FLAC_BLOCK_TYPE_VORBIS_COMMENT	4
#define	FLAC_BLOCK_TYPE_CUESHEET		5
#define	FLAC_BLOCK_TYPE_PICTURE			6

void TCCMetadataParser::ParseFLAC()
{
	LOGMSG("%s in", __func__);
	int32_t return_offset;

	//return_offset = iFile->Tell();
	return_offset = cdk_ftell64(iFile);

	// skip id3 tag
	cdk_fseek64(iFile, 0, SEEK_SET);
	unsigned int id3tagsize = 0;
	char id3header[10];
	
	cdk_fread(id3header, 1, 10, iFile);
	if (!strncmp(id3header, "ID3", 3)) {
		id3tagsize = (id3header[6] << 21) | (id3header[7] << 14) | (id3header[8] <<  7) | (id3header[9] <<  0);
		id3tagsize += 10;
	}

	// 42 is sum of "fLaC", size of METADATA_BLOCK_HEADER = 4 and METADATA_BLOCK_STREAMINFO = 34
	//iFile->Seek(42, Oscl_File::SEEKSET);
	cdk_fseek64(iFile, id3tagsize + 42, SEEK_SET);

	uint8_t metadata_type;
	bool isLastBlock;
	uint32_t metadata_size;
	uint8_t metadata_size_buf[3];

	while(1)
	{
		cdk_fread(&metadata_type, 1, 1, iFile);
		cdk_fread(metadata_size_buf, 1, 3, iFile);
		metadata_size = ((uint32_t)metadata_size_buf[0] << 16) + ((uint32_t)metadata_size_buf[1] << 8) + (uint32_t)metadata_size_buf[2];

		if (metadata_type & 0x80)
		{
			isLastBlock = true;
		}
		else
		{
			isLastBlock = false;
		}
	
		metadata_type &= 0x7f;

		if (metadata_type != FLAC_BLOCK_TYPE_VORBIS_COMMENT)
		{
			if (isLastBlock)
			{
				break;
			}
			else
			{
				cdk_fseek64(iFile, metadata_size, SEEK_CUR);
				continue;
			}
		}
		else // this block is VORBIS_COMMENT
		{
			uint8_t* pMetadata;
			pMetadata = (uint8_t*)malloc(metadata_size);

			cdk_fread(pMetadata, 1, metadata_size, iFile);

			uint32_t vendor_length;
			uint32_t user_comment_list_length;

			uint8_t* p = pMetadata;
			LoadDWORD(vendor_length, p);
			p += vendor_length; // skip vendor data

			LoadDWORD(user_comment_list_length, p); // read number of tags

			uint32_t length;
			for (uint32_t i = 0 ; i < user_comment_list_length ; i++)
			{
				LoadDWORD(length, p);

				if (strncasecmp((const char*)p, "artist", 6) == 0 || strncasecmp((const char*)p, "ARTIST", 6) == 0)
				{
					LOGMSG("exist artist tag");
					p += 7; // "artist="
					length -= 7;
					String16 str16((const char*)p, length);
					cData.sArtist = str16;
					p += length;
				}
				else if (strncasecmp((const char*)p, "album", 5) == 0 || strncasecmp((const char*)p, "ALBUM", 5) == 0)
				{
					LOGMSG("exist album tag");
					p += 6; // "album="
					length -= 6;
					String16 str16((const char*)p, length);
					cData.sAlbum = str16;
					p += length;
				}
				else if (strncasecmp((const char*)p, "title", 5) == 0 || strncasecmp((const char*)p, "TITLE", 5) == 0)
				{
					LOGMSG("exist title tag");
					p += 6; // "title="
					length -= 6;
					String16 str16((const char*)p, length);
					cData.sTitle = str16;
					p += length;
				}
				else if (strncasecmp((const char*)p, "tracknumber", 11) == 0 || strncasecmp((const char*)p, "TRACKNUMBER", 11) == 0)
				{
					LOGMSG("exist tracknumber tag");
					p += 12; // "tracknumber="
					length -= 12;
					String16 str16((const char*)p, length);
					cData.sTrack = str16;
					p += length;
				}
				else if (strncasecmp((const char*)p, "genre", 5) == 0 || strncasecmp((const char*)p, "GENRE", 5) == 0)
				{
					LOGMSG("exist genre tag");
					p += 6; // "genre="
					length -= 6;
					String16 str16((const char*)p, length);
					cData.sGenre = str16;
					p += length;
				}
				else if (strncasecmp((const char*)p, "description", 11) == 0 || strncasecmp((const char*)p, "DESCRIPTION", 11) == 0)
				{
					LOGMSG("exist description tag");
					p += 12; // "description="
					length -= 12;
					String16 str16((const char*)p, length);
					cData.sDescription = str16;
					p += length;
				}
				else if (strncasecmp((const char*)p, "date", 4) == 0 || strncasecmp((const char*)p, "DATE", 4) == 0)
				{
					LOGMSG("exist date tag");
					p += 5; // "date="
					length -= 5;
					String16 str16((const char*)p, length);
					cData.sYear = str16;
					p += length;
				}
				else
				{
					p += length; // unrecognized tags
				}
			}

			free(pMetadata);
			break;
		}
	}

	cdk_fseek64(iFile, return_offset, SEEK_SET);
}

#define DATA_TYPE_UNICODE		0
#define DATA_TYPE_BYTE_ARRAY	1
#define DATA_TYPE_BOOL			2	
#define DATA_TYPE_DWORD			3	
#define DATA_TYPE_QWORD			4	
#define DATA_TYPE_WORD			5	

void TCCMetadataParser::ParseWMA()
{
	int32_t return_offset;

	return_offset = cdk_ftell64(iFile);

	uint8_t buf[24]; // 24 is sum of GUID and Object Size
	uint8_t* pBuf;
	uint64_t object_size;
	uint32_t nObjects;

	cdk_fseek64(iFile, 24, SEEK_SET); // start point of number of objects 
	cdk_fread(buf, 1, 4, iFile);
	pBuf = buf;
	LoadQWORD(nObjects, pBuf);
	cdk_fseek64(iFile, 30, SEEK_SET); // start point of first object

	LOGMSG("number of objects %d", (uint32_t)nObjects);
	// find out Content Description Object & Extended Content Description Object
	for (uint32_t num_obj=0 ; num_obj < nObjects ; num_obj++)
	{
		cdk_fread(buf, 1, 24, iFile); // read Object ID and Object size from file

		pBuf = buf;
		pBuf += 16; // skip GUID
		LoadQWORD(object_size, pBuf);

		if (strncmp(ContentDescriptionObjectID, (char*)buf, 16) == 0)
		{
			char* data;
			char* pData;
			char* pValue;
			uint32_t length;
			uint32_t count;

			data = (char*)malloc(object_size-24); // exact data size is object size minus 24
			cdk_fread(data, 1, object_size-24, iFile);

			pData = data;
			LoadWORD(length, pData); // length is in bytes

			pValue = pData + 8; // content start point, 4 length info are remained, each one has 2 bytes size

			if (length > 0)
			{
				LOGMSG("exist title tag");
				cData.sTitle.setTo((const char16_t*)pValue, length/WIN32_UNICODE_SIZE);
			}

			pValue += length;
			LoadWORD(length, pData);

			if (length > 0)
			{
				LOGMSG("exist author tag");
				cData.sAuthor.setTo((const char16_t*)pValue, length/WIN32_UNICODE_SIZE);
			}

			pValue += length;
			LoadWORD(length, pData);

			if (length > 0)
			{
				LOGMSG("exist copyright tag");
				cData.sCopyright.setTo((const char16_t*)pValue, length/WIN32_UNICODE_SIZE);
			}

			pValue += length;
			LoadWORD(length, pData);

			if (length > 0)
			{
				LOGMSG("exist description tag");
				cData.sDescription.setTo((const char16_t*)pValue, length/WIN32_UNICODE_SIZE);
			}

			pValue += length;
			LoadWORD(length, pData);

			if (length > 0)
			{
				LOGMSG("exist rating tag");
				cData.sRating.setTo((const char16_t*)pValue, length/WIN32_UNICODE_SIZE);
			}

			free(data);
		}
		else if (strncmp(ExtendedContentDescriptionObjectID, (char*)buf, 16) == 0)
		{
			LOGMSG("exist ExtendedContentDescriptionObject");
			char* data;
			char* pData;
			uint16_t nContent; 

			data = (char*)malloc(object_size-24);
			cdk_fread(data, 1, object_size-24, iFile);

			pData = data;
			LoadWORD(nContent, pData); // read number of extended tags

			uint16_t desc_name_length;
			uint16_t desc_val_length;
			uint16_t desc_val_type;
			String16 desc_name;
			
			for (uint16_t num_con=0 ; num_con < nContent ; num_con++)
			{
				LoadWORD(desc_name_length, pData);

				desc_name.setTo((const char16_t*)pData, desc_name_length/WIN32_UNICODE_SIZE);

				pData += desc_name_length;
				LoadWORD(desc_val_type, pData);
				LoadWORD(desc_val_length, pData);

				if (desc_val_length > 0)
				{
					if (desc_name == String16("WM/Year"))
					{
						LOGMSG("exist year tag");
						char year_string[40];
						memset(year_string, 0, 40);

						switch (desc_val_type) {
							case DATA_TYPE_UNICODE: 
								cData.sYear.setTo((const char16_t*)pData, desc_val_length/WIN32_UNICODE_SIZE);
								break;
							case DATA_TYPE_WORD: 
								{
									int16_t year;
									LoadWORD(year, pData);

									sprintf(year_string, "%d", year);
									String16 year16(year_string, 40);	
									cData.sYear.setTo(year16);
									pData -= sizeof(int16_t);
								}
								break;
							case DATA_TYPE_DWORD: 
								{
									int32_t year;
									LoadDWORD(year, pData);

									sprintf(year_string, "%d", year);
									String16 year16(year_string, 40);	
									cData.sYear.setTo(year16);
									pData -= sizeof(int32_t);
								}
								break;
							case DATA_TYPE_QWORD: 
								{
									int64_t year;
									LoadQWORD(year, pData);

									sprintf(year_string, "%lld", year);
									String16 year16(year_string, 40);	
									cData.sYear.setTo(year16);
									pData -= sizeof(int64_t);
								}
								break;
							default:
								break;
						}
					}
					else if (desc_name == String16("WM/AlbumTitle"))
					{
						LOGMSG("exist album tag");
						cData.sAlbum.setTo((const char16_t*)pData, desc_val_length/WIN32_UNICODE_SIZE);
					}
					else if (desc_name == String16("WM/Genre"))
					{
						LOGMSG("exist genre tag");
						cData.sGenre.setTo((const char16_t*)pData, desc_val_length/WIN32_UNICODE_SIZE);
					}
					else if (desc_name == String16("WM/Composer"))
					{
						LOGMSG("exist composer tag");
						cData.sComposer.setTo((const char16_t*)pData, desc_val_length/WIN32_UNICODE_SIZE);
					}
					else if (desc_name == String16("WM/Track") || desc_name == String16("WM/TrackNumber"))
					{
						if (cData.sTrack.size() == 0) {
							LOGMSG("exist track tag");
							char track_string[40];
							memset(track_string, 0, 40);

							switch (desc_val_type) {
								case DATA_TYPE_UNICODE:
									cData.sTrack.setTo((const char16_t*)pData, desc_val_length/WIN32_UNICODE_SIZE);
									break;
								case DATA_TYPE_WORD:
									{
										int16_t track_num;
										LoadWORD(track_num, pData);
										if (track_num >= 0) {
											sprintf(track_string, "%d", track_num);
											String16 track16(track_string, 40);	
											cData.sTrack.setTo(track16);
										}

										pData -= sizeof(int16_t);
									}
									break;
								case DATA_TYPE_DWORD:
									{
										int32_t track_num;
										LoadDWORD(track_num, pData);
										if (track_num >= 0) {
											sprintf(track_string, "%d", track_num);
											String16 track16(track_string, 40);	
											cData.sTrack.setTo(track16);
										}

										pData -= sizeof(int32_t);
									}
									break;
								case DATA_TYPE_QWORD:
									{
										int64_t track_num;
										LoadQWORD(track_num, pData);
										if (track_num >= 0) {
											sprintf(track_string, "%lld", track_num);
											String16 track16(track_string, 40);	
											cData.sTrack.setTo(track16);
										}

										pData -= sizeof(int64_t);
									}
									break;
								default:
									break;
							}
						}
					}
				}

				pData += desc_val_length;
			}

			free(data);
		}	
		else
		{
			// skip other objects
			cdk_fseek64(iFile, (uint32_t)object_size-24, SEEK_CUR);
		}
	}
	
	cdk_fseek64(iFile, return_offset, SEEK_SET);
}

