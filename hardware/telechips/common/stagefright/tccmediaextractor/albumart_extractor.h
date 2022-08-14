#ifndef _ALBUMART_EXTRACTOR_H
#define _ALBUMART_EXTRACTOR_H

#include <media/stagefright/DataSource.h>

using namespace android;

class TCCAlbumartExtractor
{
	public:
		TCCAlbumartExtractor() : pAlbumart(NULL), size(0) {}

		~TCCAlbumartExtractor()
		{
			if (pAlbumart)
			{
				delete[] pAlbumart;
				pAlbumart = NULL;
			}
		}

		bool ExistWMAAlbumart(const sp<DataSource> &source);
		void GetWMAAlbumart(uint32_t &aSize, uint8_t* &pData) const
		{
			aSize = size;
			pData = pAlbumart;
		}

	private:

		void ExtractWMAAlbumartCore(char* pSrc, uint32_t iSize);
		uint8_t *pAlbumart;
		uint32_t size;
};

#endif // _ALBUMART_EXTRACTOR_H

