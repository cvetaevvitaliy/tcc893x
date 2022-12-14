/****************************************************************************
 *	 TCC Version 0.6
 *	 Copyright (c) telechips, Inc.
 *	 ALL RIGHTS RESERVED
 *
****************************************************************************/

#ifndef SZ_1K
#define SZ_1K		(1024)
#endif

#ifndef SZ_1M
#define SZ_1M		(1024*1024)
#endif

#define MEM_PHYS_OFFSET						(0x80000000)

#define ARRAY_MBYTE(x)		((((x) + (SZ_1M-1))>> 20) << 20)

#define TOTAL_SDRAM_SIZE					(TCC_MEM_SIZE * SZ_1M)

#if defined(PLATFORM_TCC892X)
	#if defined(TARGET_BOARD_STB)
		#include "tcc_used_mem_tcc8920st.h"
	#else
		#include "tcc_used_mem_tcc892x.h"
	#endif
#elif defined(PLATFORM_TCC893X)
	#if defined(TARGET_BOARD_STB)
		#include "tcc_used_mem_tcc8930st.h"
	#else
		#include "tcc_used_mem_tcc893x.h"
	#endif
#else
	#error No memory map is specified!
#endif
