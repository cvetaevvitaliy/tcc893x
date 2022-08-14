#ifndef __TCC_GS_DECODER__
#define __TCC_GS_DECODER__

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Definitions
//
#define OBJECT_MAX			2


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Macros
//

	
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Exported Types
//
/* API return typs */
typedef	signed long		gsdresult_t;
typedef	void*			gsdhandle_t;

/* memory management functions */
typedef struct gsd_mem_funcs_t
{
	void* (*m_pfnMalloc			) ( unsigned int );									//!< malloc
	void  (*m_pfnFree			) ( void* );										//!< free
	int   (*m_pfnMemcmp			) ( const void*, const void*, unsigned int );		//!< memcmp
	void* (*m_pfnMemcpy			) ( void*, const void*, unsigned int );				//!< memcpy
	void* (*m_pfnMemmove		) ( void*, const void*, unsigned int );				//!< memmove
	void* (*m_pfnMemset			) ( void*, int, unsigned int );						//!< memset
	void* (*m_pfnRealloc		) ( void*, unsigned int );							//!< realloc
	int reserved[16-7];
} gsd_mem_funcs_t;

typedef enum gsd_outformat_t
{
	// planar YCbCr format (separated format)
	GSD_OUT_YCbCr420SEP,
	GSD_OUT_YCbCr422SEP,
	GSD_OUT_YCbCr444SEP,

	// planar YCbCr format + chrominance interleaved
	GSD_OUT_YCbCr420SEPCI_TYPE0,	// Y(Y0 Y1 Y2 Y4 ..) UV(Cb0 Cr0 Cb1 Cr1 ..)
	GSD_OUT_YCbCr422SEPCI_TYPE0,	// Y(Y0 Y1 Y2 Y4 ..) UV(Cb0 Cr0 Cb1 Cr1 ..)
	GSD_OUT_YCbCr420SEPCI_TYPE1,	// Y(Y0 Y1 Y2 Y4 ..) UV(Cr0 Cb0 Cr1 Cb1 ..)
	GSD_OUT_YCbCr422SEPCI_TYPE1,	// Y(Y0 Y1 Y2 Y4 ..) UV(Cr0 Cb0 Cr1 Cb1 ..)

	// packed YCbCr format (sequential format)
	GSD_OUT_YCbCr422SEQ_TYPE0,	//YUV2	(Y0 Cb0 Y1 Cr0 ...)
	GSD_OUT_YCbCr422SEQ_TYPE1,	//UYVY	(Cr0 Y0 Cb0 Y1 ...)

	// interleaved RGB format
	GSD_OUT_RGB8888,		//32bit ARGB	MSB(A0 R0 G0 B0)LSB
	GSD_OUT_RGB4444,		//16bit ARGB	MSB(A0 R0 G0 B0)LSB
	GSD_OUT_RGB565,			//16bit RGB		MSB(R0 G0 B0)LSB
	
} gsd_outformat_t;

typedef union gsd_chromakey_t
{
	struct {
		unsigned char	ucY;
		unsigned char	ucCb;
		unsigned char	ucCr;
		unsigned char	ucReserved;
	} stYCbCr;
	struct {
		unsigned char	ucR;
		unsigned char	ucG;
		unsigned char	ucB;
		unsigned char	ucReserved;
	} stRGB;
} gsd_chromakey_t;

typedef struct gsd_rgnrect_t
{
	long	lLeft;
	long	lTop;
	long	lRight;
	long	lBottom;
} gsd_rgnrect_t;

typedef struct gsd_object_info_t
{
	long			m_lOffsetH;
	long			m_lOffsetV;
	long			m_lWidth;
	long			m_lHeight;
	long			m_lAdjustWidth;
	unsigned char  *m_pbyOutputBuff;
	long			m_lOutputLength;
} gsd_object_info_t;


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//	
//	Main API
//
/*============================================================
	ROUTINE: 
		TCC_GS_DEC

	DESCRIPTION:
		Main function of the Graphic Stream Decoder API

	PARAMS:
		- opCode: operation code (defined below)
		- handle: GS Decoder handle
		- param1: operation parameter 1 (generally used as input)
		- param2: operation parameter 2 (generally used as output)

	RETURNS:
		- GSD_SUCCESS
		- GSD_FAILED
		- GSD_NONE:		Display buffer not updated
		- GSD_UPDATED:	Display buffer updated
============================================================*/
gsdresult_t TCC_GS_DEC(unsigned long opCode, gsdhandle_t handle, void *param1, void *param2);

/* Return values */
#define GSD_FAILED			-1
#define GSD_SUCCESS			0
#define GSD_NONE			1	// Display buffer not updated
#define GSD_UPDATED			2	// Display buffer updated

/* opCode constants */
#define GSDOP_OPEN			0
#define GSDOP_CLOSE			1
#define GSDOP_DECODE		2
#define GSDOP_RESET			3	// reset decoder status with frame seek
#define GSDOP_GET_LASTERROR	4


/*============================================================
	Usage - Decoder Instance Open
==============================================================
	PARAMS:
		- opCode:	GSDOP_OPEN
		- handle:	NULL
		- param1:	(gsd_init_t*)in
		- param2:	(gsd_info_t*)out

		param1:	in->m_stMemFuncs = <Memory Management Functions>;
				in->m_emOutFormat = GSD_OUT_YCbCr422SEQ_TYPE0 / GSD_OUT_YCbCr422SEQ_TYPE1 / .. ;
				in->m_stChromaKeyValue = {Y, Cb, Cr or R, G, B};
				in->m_lVideoWidth = 1920;	// video width from demuxer infomation
				in->m_lVideoHeight = 1080;	// video height from demuxer infomation
				in->m_ulOption = GSDOPT_DISPLAY_UPDATE_MODE | GSDOPT_ERASE_DISPLAY;	// default: 0

	RETURNS:
		- GSD_SUCCESS / GSD_FAILED
============================================================*/
/* ulOption */
#define GSDOPT_DISPLAY_UPDATE_MODE	0x0001	// whole display region is updated.
#define GSDOPT_ERASE_DISPLAY		0x0002	// Whole display region is erased before update on display update mode.
#define GSDOPT_16X_ADJUSTED_WIDTH	0x0004	// 

typedef struct gsd_init_t
{
	gsd_mem_funcs_t	m_stMemFuncs;			// Memory management functions
	gsd_outformat_t	m_emOutFormat;			// GS Decoder output format
	gsd_chromakey_t	m_stChromaKeyValue;		// Y/Cb/Cr value for Transparent Pixel (used for YCbCr output format)

	long			m_lVideoWidth;			// video plain width (no rescaled)
	long			m_lVideoHeight;			// video plain height (no rescaled)

	unsigned long	m_ulOption;				// GS decoder options
} gsd_init_t;

typedef struct gsd_info_t
{
	gsdhandle_t		m_hGSDecoder;
	unsigned long	m_ulOutputBuffLength;	// output buffer length
	long			m_lAdjustWidth;
} gsd_info_t;


/*============================================================
	Usage - Decoder Instance Close
==============================================================
	PARAMS:
		- opCode:	GSDOP_CLOSE
		- handle:	<TS DeMuxer Handle>
		- param1:	NULL
		- param2:	NULL

	RETURNS:
		- GSD_SUCCESS / GSD_FAILED
============================================================*/


/*============================================================
	Usage - Decoder Instance Reset
==============================================================
	PARAMS:
		- opCode:	GSDOP_RESET
		- handle:	<TS DeMuxer Handle>
		- param1:	NULL
		- param2:	NULL

	RETURNS:
		- GSD_SUCCESS / GSD_FAILED
============================================================*/


/*============================================================
	Usage - Graphics Stream Decode
==============================================================
	PARAMS:
		- opCode:	GSDOP_CLOSE
		- handle:	<TS DeMuxer Handle>
		- param1:	(gsd_decode_t*)in
		- param2:	(gsd_display_output_t* or gsd_object_output_t*)out

		param1:	in->m_pbyInputData = <Graphics Stream data>;
				in->m_ulInputLength = <length of the Stream data>;
				in->m_lTimeStamp = <timestamp of the Stream data>;	//from demuxer

	RETURNS:
		- GSD_SUCCESS / GSD_FAILED
============================================================*/
typedef struct gsd_decode_t
{
	unsigned char  *m_pbyInputData;
	unsigned long	m_ulInputLength;
	long			m_lTimeStamp;

	unsigned char  *m_pbyOutputBuff;			// buff size: gsd_info_t.ulPlainBuffLength
} gsd_decode_t;

typedef struct gsd_display_output_t
{
	gsd_rgnrect_t		m_stUpdateRegion;
	long				m_lTimeStamp;
} gsd_display_output_t;

typedef struct gsd_object_output_t
{
	gsd_object_info_t	m_astObjectInfo[OBJECT_MAX];
	long				m_lObjectTotal;
	long				m_lTimeStamp;
} gsd_object_output_t;


/*============================================================
	Usage - Get Last Error
==============================================================
	PARAMS:
		- opCode:	GSDOP_GET_LASTERROR
		- handle:	<TS DeMuxer Handle>
		- param1:	NULL
		- param2:	NULL

	RETURNS:
		- ERROR_CODE:	defined below
============================================================*/
/*============================================================
	ERROR_CODE constants
============================================================*/
// system error
#define GSDERR_BASE_SYSTEM_ERROR							0
#define GSDERR_SYSTEM_ERROR									(GSDERR_BASE_SYSTEM_ERROR - 1)
#define GSDERR_DECODER_ALLOCATION_FAILED					(GSDERR_BASE_SYSTEM_ERROR - 2)
#define GSDERR_OBJECT_BUFFER_ALLOCATION_FAILED				(GSDERR_BASE_SYSTEM_ERROR - 3)
#define GSDERR_OBJECT_BUFFER_REALLOCATION_FAILED			(GSDERR_BASE_SYSTEM_ERROR - 4)
#define GSDERR_RLE_OBJECT_BUFFER_ALLOCATION_FAILED			(GSDERR_BASE_SYSTEM_ERROR - 5)
#define GSDERR_RLE_OBJECT_BUFFER_REALLOCATION_FAILED		(GSDERR_BASE_SYSTEM_ERROR - 6)

// invalid function parameter
#define GSDERR_BASE_INVALID_FUNC_PARAM						-1000
#define GSDERR_INVALID_DECODER_HANDLE						(GSDERR_BASE_INVALID_FUNC_PARAM - 1)

// internal state error
#define GSDERR_BASE_INTERNAL_STATE							-2000
#define GSDERR_INVALID_PALETTE_ID							(GSDERR_BASE_INTERNAL_STATE - 1)
#define GSDERR_INVALID_OBJECT_ID							(GSDERR_BASE_INTERNAL_STATE - 2)
#define GSDERR_INVALID_STATE_COMPOSITION					(GSDERR_BASE_INTERNAL_STATE - 3)
#define GSDERR_INVALID_STATE_PALETTE						(GSDERR_BASE_INTERNAL_STATE - 4)
#define GSDERR_INVALID_STATE_OBJECT							(GSDERR_BASE_INTERNAL_STATE - 5)
#define GSDERR_OBJECT_DATA_NOT_STARTED						(GSDERR_BASE_INTERNAL_STATE - 6)
#define GSDERR_PREVIOUS_COMPOSITION_NOT_EXIST				(GSDERR_BASE_INTERNAL_STATE - 7)
#define GSDERR_COMPOSITION_IS_NULL							(GSDERR_BASE_INTERNAL_STATE - 8)
#define GSDERR_VALID_WINDOW_NOT_EXIST						(GSDERR_BASE_INTERNAL_STATE - 9)

#ifdef __cplusplus
}
#endif
#endif //__TCC_GS_DECODER__
