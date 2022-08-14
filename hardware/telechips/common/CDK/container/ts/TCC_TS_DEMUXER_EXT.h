#ifndef __TCC_TS_DEMUXER_EXT_H__
#define __TCC_TS_DEMUXER_EXT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WCE)
typedef	signed __int64		tsd_s64_t;
typedef	unsigned __int64	tsd_u64_t;
#else
typedef	signed long long	tsd_s64_t;
typedef	unsigned long long	tsd_u64_t;
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Extensions
//

/*
	Tag of the extension fields
*/
#define TSDEXT_TAG_CONFIG_SCAN_ENDPTS	1
#define TSDEXT_TAG_MAX					2	

/*
	extension field header
*/
typedef struct tsdext_header_t
{
	unsigned long	ulExtTag;			// TSDEXT_ID_CONFIG_SCAN_ENDPTS
	long			lExtBytes;			// sizeof(tsdext_header_t) + ??
	void		   *pNextField;			// pointer of next header
} tsdext_header_t;


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Extension Fields
//
typedef struct tsdext_config_scan_endpts_t
{
	unsigned long	ulExtTag;			// TSDEXT_TAG_CONFIG_SCAN_ENDPTS
	long			lExtBytes;			// sizeof(tsdext_config_scan_endpts_t)
	void		   *pNextField;			// pointer of next header

	tsd_s64_t		llStartOffset;		// offset from end of file (positive value)
	tsd_s64_t		llMaxOffset;		// offset from end of file (positive value)
	tsd_s64_t		llStepSize;			// step size of backward file seek
} tsdext_config_scan_endpts_t;


#ifdef __cplusplus
}
#endif
#endif //__TCC_TS_DEMUXER_EXT_H__
