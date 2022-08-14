#include "albumart_extractor.h"
#include "tcc_metadata_define.h"
#include <utils/Log.h>
#include <utils/String16.h>

//#define DEBUG
#ifdef DEBUG
#define LOGMSG(x...) LOGD(x)
#else
#define LOGMSG(x...)
#endif


// Win32 unicode size = 2, linux unicode size = 4
// src_size is size of Win32 unicode string, in bytes
/*
void UnicodeWin32ToLinux(char* pSrc, uint32_t src_size, oscl_wchar* pDst)
{
	for (uint32_t count = 0 ; count < src_size/WIN32_UNICODE_SIZE ; count++)
	{
		memcpy(pDst+count, pSrc+count*WIN32_UNICODE_SIZE, WIN32_UNICODE_SIZE);
	}
}
*/

bool TCCAlbumartExtractor::ExistWMAAlbumart(const sp<DataSource> &source)
{
    uint8_t buf[24]; // 24 is sum of GUID and Object Size
    uint8_t* pBuf;
    uint64_t object_size;
    uint32_t nObjects;
    uint64_t offset;

    ssize_t n = source->readAt(24, buf, 4);
    if (n < 0) return false;

    pBuf = buf;
    LoadQWORD(nObjects, pBuf);
    offset = 30; // start point of first object

    LOGMSG("number of objects %d", (uint32_t)nObjects);
    // find out Content Description Object & Extended Content Description Object
    for (uint32_t num_obj=0 ; num_obj < nObjects ; num_obj++)
    {
        n = source->readAt(offset, buf, 24); // read Object ID and Object size from file
        if (n < 0) return false;

        offset += 24;

        pBuf = buf;
        pBuf += 16; // skip GUID
        LoadQWORD(object_size, pBuf);

        if (strncmp(ExtendedContentDescriptionObjectID, (char*)buf, 16) == 0)
        {
            LOGMSG("exist ExtendedContentDescriptionObject");
            char* data;
            char* pData;
            uint16_t nContent;

            data = (char*)malloc(object_size-24);
            n = source->readAt(offset, data, object_size-24);
            if (n < 0) return false;

            offset += object_size-24;

            pData = data;
            LoadWORD(nContent, pData); // read number of extended tags

            uint16_t desc_name_length;
            uint16_t desc_val_length;
            String16 desc_name;

            for (uint16_t num_con=0 ; num_con < nContent ; num_con++)
            {
                LoadWORD(desc_name_length, pData);

                desc_name.setTo((const char16_t*)pData, desc_name_length/WIN32_UNICODE_SIZE);

                pData += desc_name_length;
                pData += 2; // skip description value type
                LoadWORD(desc_val_length, pData);

                if (desc_val_length > 0)
                {
                    if (desc_name == String16("WM/Picture"))
                    {
                        LOGMSG("exist album art tag");
                        ExtractWMAAlbumartCore(pData, desc_val_length);
                    }
                }

                pData += desc_val_length;
            }

            free(data);
        }
        else if (strncmp(HeaderExtensionObjectID, (char*)buf, 16) == 0)
        {
            LOGMSG("Header Extension Object in");
            uint8_t header_ext_obj[22];
            uint32_t header_ext_obj_size;
            uint8_t* pData;
            n = source->readAt(offset, header_ext_obj, 22);
            if (n < 0) return false;

            offset += 22;

            pData = &header_ext_obj[18];
            LoadDWORD(header_ext_obj_size, pData);

            LOGMSG("header extension object size %d", header_ext_obj_size);
            if (header_ext_obj[16] == 0x06) // it must be 6
            {
                uint8_t sub_obj_header[24];
                uint64_t sub_obj_size;

                while(header_ext_obj_size > 0)
                {
                    n = source->readAt(offset, sub_obj_header, 24);
                    if (n < 0) return false;

                    offset += 24;
                    pData = &sub_obj_header[16];
                    LoadQWORD(sub_obj_size, pData);

                    if (strncmp(MetadataLibraryObjectID, (char*)sub_obj_header, 16) == 0)
                    {
                        LOGMSG("Exist Metadata Library Object");
                        char* desc_data;
                        char* p;
                        uint16_t nContent;
                        uint16_t desc_name_length;
                        uint32_t desc_val_length;
                        String16 desc_name;

                        desc_data = (char*)malloc(sub_obj_size-24);
                        n = source->readAt(offset, desc_data, sub_obj_size-24);
                        if (n < 0) return false;

                        offset += sub_obj_size-24;

                        p = desc_data;
                        LoadWORD(nContent, p); // read number of tags in metadata library object

                        LOGMSG("Metadata Library Object has %d contents", nContent);
                        for (uint16_t num_con = 0 ; num_con < nContent ; num_con++)
                        {
                            p += 4; // skip language idx(2), stream number(2)

                            LoadWORD(desc_name_length, p);

                            p += 2; // skip data type(2)

                            LoadDWORD(desc_val_length, p);

                            desc_name.setTo((const char16_t*)p, desc_name_length/WIN32_UNICODE_SIZE);

                            p += desc_name_length;
                            if (desc_name == String16("WM/Picture"))
                            {
                                LOGMSG("exist album art tag");
                                ExtractWMAAlbumartCore(p, desc_val_length);
                            }

                            p += desc_val_length;
                        }

                        free(desc_data);
                    }
                    else
                    {
                        offset += sub_obj_size-24;
                    }
                    header_ext_obj_size -= (uint32_t)sub_obj_size;
                }
            }
            else
            {
                // skip remaining data
                offset += object_size-24-22;
            }
        }
        else
        {
            // skip other objects
            offset += object_size-24;
        }
    }

    if (pAlbumart && size > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void TCCAlbumartExtractor::ExtractWMAAlbumartCore(char* pSrc, uint32_t iSize)
{
    if (iSize == 0)
    {
        return;
    }

    uint32_t picture_size;
    uint8_t picture_type;

    LoadBYTE(picture_type, pSrc);
    LoadDWORD(picture_size, pSrc);

    LOGMSG("pic type %d, pic size %d", picture_type, picture_size);

    if (picture_size > 0)
    {
        uint32_t remained_header;
        remained_header = iSize - picture_size - 1 - 4; // 1 is type info, 4 is size info
        LOGMSG("remained_header1 %d", remained_header);

        uint32_t r_size;
        for (r_size = 0 ; r_size < remained_header ; r_size+=WIN32_UNICODE_SIZE)
        {
            if (pSrc[r_size] == 0 && pSrc[r_size+1] == 0) // NULL character is (0x00 0x00)
            {
                LOGMSG("picture mime type string size %d", r_size+WIN32_UNICODE_SIZE);
                //pAlbumart->iGraphicMimeType = (oscl_wchar*)calloc(r_size/WIN32_UNICODE_SIZE+1, sizeof(oscl_wchar)); // 1 is for NULL character
                //UnicodeWin32ToLinux(pSrc, r_size+WIN32_UNICODE_SIZE, pAlbumart->iGraphicMimeType);
                pSrc += r_size+WIN32_UNICODE_SIZE;
                break;
            }
        }
        remained_header -= r_size+2; // size of string + NULL character
        LOGMSG("remained_header2 %d", remained_header);
        for (r_size = 0 ; r_size < remained_header ; r_size+=WIN32_UNICODE_SIZE)
        {
            if (pSrc[r_size] == 0 && pSrc[r_size+1] == 0) // NULL character is (0x00 0x00)
            {
                LOGMSG("picture description string size %d", r_size+WIN32_UNICODE_SIZE);
                //pAlbumart->iGraphicDescription = (oscl_wchar*)calloc(r_size/WIN32_UNICODE_SIZE+1, sizeof(oscl_wchar)); // 1 is for NULL character
                //UnicodeWin32ToLinux(pSrc, r_size+WIN32_UNICODE_SIZE, pAlbumart->iGraphicDescription);
                pSrc += r_size+WIN32_UNICODE_SIZE;
                break;
            }
        }

        LOGMSG("%02x %02x %02x %02x %02x", pSrc[0], pSrc[1], pSrc[2], pSrc[3], pSrc[4]);
        pAlbumart = new uint8_t[picture_size];
        memcpy(pAlbumart, pSrc, picture_size);
        size = picture_size;
    }
}

