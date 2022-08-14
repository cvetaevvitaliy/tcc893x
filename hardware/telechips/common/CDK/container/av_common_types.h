#ifndef _AV_COMMON_TYPES_H_
#define _AV_COMMON_TYPES_H_

/*
// MicorSoft Visual C++
#if (defined (_WIN32) && !defined (_WIN32_WCE)) || (defined (__WINS__) && defined (_SYMBIAN)) || (defined (WINCE_EMULATOR)) || (defined (_OPENWAVE_SIMULATOR))

// x86 GCC
#elif defined(__GNUC__) && (defined(__i386__) || defined(__amd64__)) || (defined (_SOLARIS) && !defined (__GNUC__) && defined(_SOLARISX86))

// MicroSoft Embedded Visual C++
#elif defined (_WIN32) && defined (_WIN32_WCE) && defined (ARM)

#endif
*/

/*
 ============================================================
 *
 *	Type Definition
 *
 ============================================================
*/
#if defined(_WIN32_WCE) || defined(_WIN32)
	typedef signed __int64		S64;
	typedef unsigned __int64	U64;
#else
	typedef signed long long	S64;
	typedef unsigned long long	U64;
#endif

typedef unsigned long	av_handle_t;
typedef long			av_result_t;


/*
============================================================
*
*	Common Structures Definition
*
============================================================
*/

//! Memory Management Functions 
typedef struct av_memory_func_t
{
	void* (*m_pfnMalloc			) ( unsigned int );									//!< malloc
	void  (*m_pfnFree			) ( void* );										//!< free
	int   (*m_pfnMemcmp			) ( const void*, const void*, unsigned int );		//!< memcmp
	void* (*m_pfnMemcpy			) ( void*, const void*, unsigned int );				//!< memcpy
	void* (*m_pfnMemmove		) ( void*, const void*, unsigned int );				//!< memmove
	void* (*m_pfnMemset			) ( void*, int, unsigned int );						//!< memset
	void* (*m_pfnRealloc		) ( void*, unsigned int );							//!< realloc
	int reserved[16-7];																//!< Reserved for future 
} av_memory_func_t;

//! File Management Functions
typedef struct av_file_func_t
{
	void*		 (*m_pfnFopen	) ( const char*, const char* );						//!< fopen
	unsigned int (*m_pfnFread	) ( void*, unsigned int, unsigned int, void* );		//!< fread
	int			 (*m_pfnFseek	) ( void*, long, int );								//!< fseek 32bit io
	long		 (*m_pfnFtell	) ( void* );										//!< ftell 32bit io
	int			 (*m_pfnFclose	) ( void* );										//!< fclose
	unsigned int (*m_pfnFeof	) ( void* );										//!< feof
	unsigned int (*m_pfnFflush	) ( void* );										//!< fflush
	unsigned int (*m_pfnFwrite	) ( const void*, unsigned int, unsigned int, void* );//!< fwrite (Muxer only)
	int			 (*m_pfnUnlink	) ( const char* );									//!< unlink (Muxer only)
	
	// 64bit io
	int			 (*m_pfnFseek64	) ( void*, S64, int );								//!< fseek 64bit io
	S64			 (*m_pfnFtell64	) ( void* );										//!< ftell 64bit io
	int reserved[16-11];															//!< Reserved for future 
} av_file_func_t;

#endif//_AV_COMMON_TYPES_H_
