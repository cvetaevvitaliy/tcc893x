#ifndef _TCC_METADATA_PARSER_H
#define _TCC_METADATA_PARSER_H

#include <utils/String16.h>

using namespace android;

class TCCMetadata
{
	public:
		TCCMetadata() {}

		String16 sArtist;
		String16 sAlbum;
		String16 sTitle;
		String16 sTrack;
		String16 sGenre;
		String16 sComment;
		String16 sAuthor;
		String16 sDescription;
		String16 sRating;
		String16 sComposer;
		String16 sCopyright;
		String16 sYear;
};

class TCCMetadataParser
{
	public:
		TCCMetadataParser() : iFile(NULL) {}

		~TCCMetadataParser()
		{
			iFile = NULL;
		}

		int32_t ParseMetadata(int32_t iAudFmt, void* aFile);
		TCCMetadata* GetMetadata();

	private:

		void ParseFLAC();
		void ParseWMA();
		TCCMetadata	cData;
		void* iFile;
};

#endif // _TCC_METADATA_PARSER_H

