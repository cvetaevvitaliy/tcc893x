#ifndef __SECURE_AV_PARSER_H__
#define __SECURE_AV_PARSER_H__


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//
//	constant values
//
#define TRUE				1
#define FALSE				0
#define NULL				0

#define VIDEO_TYPE_MPEG12				101
#define VIDEO_TYPE_MPEG4				102
#define VIDEO_TYPE_AVC					103
#define VIDEO_TYPE_MVC					104
#define VIDEO_TYPE_VC1					105
#define VIDEO_TYPE_AVS					106

#define AUDIO_TYPE_MPEG12				201
#define AUDIO_TYPE_DOLBY_DIGITAL		202
#define AUDIO_TYPE_DOLBY_DIGITAL_PLUS	203
#define AUDIO_TYPE_DTS					204
#define AUDIO_TYPE_AAC_ADTS				205
#define AUDIO_TYPE_AAC_LATM				206
#define AUDIO_TYPE_HDMV_LPCM			207
#define AUDIO_TYPE_DVD_LPCM				208

#define AUDIO_SUBTYPE_AC3_LOSSLESS		3
#define AUDIO_SUBTYPE_AC3_PLUS			4
#define AUDIO_SUBTYPE_LPCM_HDMV			9
#define AUDIO_SUBTYPE_LPCM_DVD			10

#define SECURE_TYPE_HDCP			1



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//
//	Encryption info parameter
//

#if defined(WIN32) || defined(_WIN32) || defined(WIN32_WCE) || defined(_WIN32_WCE)
typedef unsigned __int64	SEC_U64;
typedef __int64				SEC_S64;
#else
typedef unsigned long long	SEC_U64;
typedef long long			SEC_S64;
#endif

/* HDCP (for SECURE_TYPE_HDCP type) */
//typedef struct hdcp_secure_info_t
//{
	//unsigned long	ulStreamCounter;
	//SEC_U64			ullInputCounter;
//} hdcp_secure_info_t;


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//
//	Secure audio/video parser
//

/* op code*/
#define SECOP_PARSE_VIDEO_HEADER	1
#define SECOP_PARSE_AUDIO_HEADER	2
#define SECOP_IS_VIDEO_KEYFRAME		3

/* Secure parser process main function */
int SecureParserProcess(unsigned long ulOpCode, void *param1, void *param2);



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//
//	Secure parser parameters
//

/* param1 for opcode "SECOP_PARSE_VIDEO_HEADER", "SECOP_PARSE_AUDIO_HEADER" and "SECOP_IS_VIDEO_KEYFRAME" */
typedef struct sec_parser_input_t
{
	unsigned long	ulStreamType; 
	unsigned char	*pbyEsBuffer;
	long			lEsLength;

	unsigned long	ulSecureType;
	void			*pSecureInfo;
} sec_parser_input_t;

/* param2 for opcode "SECOP_PARSE_VIDEO_HEADER" and "SECOP_IS_VIDEO_KEYFRAME" */
typedef struct sec_video_info_t
{
	long	lWidth;
	long	lHeight;
	long	lFrameRate;
	long	lFourCC;

	long	lAspectRatio;
	long	lBitRate;
	int		bIsInterlaced;
	int		bAvcAudUsed;
} sec_video_info_t;

/* param2 for opcode "SECOP_PARSE_AUDIO_HEADER" */
typedef struct sec_audio_info_t
{
	long	lFormatTag;
	long	lSubType;
	long	lSamplePerSec;
	long	lBitPerSample;
	long	lBitRate;					// Kbps
	long	lChannels;
	long	lFrameLength;
	long	lSearchPos;	
	int		bExtraExist;
} sec_audio_info_t;



#endif //__SECURE_AV_PARSER_H__