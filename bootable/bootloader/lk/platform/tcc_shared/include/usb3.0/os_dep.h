#ifndef _DWC_OS_DEP_H_
#define _DWC_OS_DEP_H_

#if 0		/* 012.08.13 */
#ifdef __cplusplus
extern "C" {
#endif
#endif /* 0 */



#if 0		/* 012.08.13 */
#ifndef uint64_t
#define uint64_t
	typedef unsigned long long		uint64_t;
#endif
#ifndef int32_t
#define int32_t
	typedef int int32_t;
#endif
#ifndef _uint32_t_
#define _uint32_t_
	typedef unsigned int uint32_t;
#endif
#ifndef u_int32_t
#define u_int32_t
	typedef unsigned int u_int32_t;
#endif
#ifndef _u_int16_t_
#define _u_int16_t_
	typedef unsigned short u_int16_t;
#endif
#ifndef uint16_t
#define uint16_t
	typedef unsigned short uint16_t;
#endif
#ifndef _u_int8_t_
#define _u_int8_t_
	typedef unsigned char u_int8_t;
#endif
#ifndef uint8_t
#define uint8_t
	typedef unsigned char uint8_t;
#endif
#ifndef _u_long_
#define _u_long_
	typedef unsigned long u_long;
#endif
#ifndef u_int
#define u_int
	typedef unsigned int u_int;
#endif
#ifndef u_char
#define u_char
	typedef unsigned char u_char;
#endif
#endif /* 0 */

#ifndef _dma_addr_t_
#define _dma_addr_t_
	typedef unsigned int dma_addr_t;
#endif
#ifndef NULL
#define NULL	0
#endif

#if 0		/* 012.08.13 */
/** @name Error Codes */
#define DWC_E_INVALID		EINVAL
#define DWC_E_NO_MEMORY		ENOMEM
#define DWC_E_NO_DEVICE		ENODEV
#define DWC_E_NOT_SUPPORTED	EOPNOTSUPP
#define DWC_E_TIMEOUT		ETIMEDOUT
#define DWC_E_BUSY			EBUSY
#define DWC_E_AGAIN			EAGAIN
#define DWC_E_RESTART		ERESTART
#define DWC_E_ABORT			ECONNABORTED
#define DWC_E_SHUTDOWN		ESHUTDOWN
#define DWC_E_NO_DATA		ENODATA
#define DWC_E_DISCONNECT	ECONNRESET
#define DWC_E_UNKNOWN		EINVAL
#define DWC_E_NO_STREAM_RES	ENOSR
#define DWC_E_COMMUNICATION	ECOMM
#define DWC_E_OVERFLOW		EOVERFLOW
#define DWC_E_PROTOCOL		EPROTO
#define DWC_E_IN_PROGRESS	EINPROGRESS
#define DWC_E_PIPE			EPIPE
#define DWC_E_IO			EIO
#define DWC_E_NO_SPACE		ENOSPC
#else
typedef enum
{
	DWC_E_INVALID = 0xB1000000,
	DWC_E_NO_MEMORY	,	
	DWC_E_NO_DEVICE		,
	DWC_E_NOT_SUPPORTED,
	DWC_E_TIMEOUT	,	
	DWC_E_BUSY			,
	DWC_E_AGAIN			,
	DWC_E_RESTART		,
	DWC_E_ABORT			,
	DWC_E_SHUTDOWN		,
	DWC_E_NO_DATA		,
	DWC_E_DISCONNECT	,
	DWC_E_UNKNOWN		,
	DWC_E_NO_STREAM_RES,
	DWC_E_COMMUNICATION,
	DWC_E_OVERFLOW		,
	DWC_E_PROTOCOL		,
	DWC_E_IN_PROGRESS	,
	DWC_E_PIPE			,
	DWC_E_IO			,
	DWC_E_NO_SPACE
} DWC_ERROR;

#endif /* 0 */

/** Compiler 'packed' attribute for structs */
#define UPACKED	__attribute__((__packed__))

#if 1
/** Type for DMA addresses */
typedef dma_addr_t		dwc_dma_t;
#define DWC_DMA_ADDR_INVALID	(~(dwc_dma_t)0)
#endif




//#define __iomem __attribute__((noderef, address_space(2)))

#define __iomem

typedef struct {
	int counter;
} atomic_t;

/**
 * The number of DMA Descriptors (TRBs) to allocate for each endpoint type.
 * NOTE: The driver currently supports more than 1 TRB for Isoc EPs only.
 * So the values for Bulk and Intr must be 1.
 */
#define DWC_NUM_BULK_TRBS	1
#define DWC_NUM_INTR_TRBS	1
#define DWC_NUM_ISOC_TRBS	256

#if 0		/* 012.08.13 */
#ifdef __cplusplus
}
#endif
#endif /* 0 */

#endif /* _DWC_OS_DEP_H_ */
