/****************************************************************************
   																				
Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/

#ifndef __CDK_PROF_H__
#define __CDK_PROF_H__

#include "cdk.h"

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
// profile index
//
#define PROF_IDX_DMX	0
#define PROF_IDX_FIO	1
#define PROF_IDX_ADEC	2
#define PROF_IDX_VDEC	3


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
// profile helper
//
#define PROF_INDEX_MAX	8

extern long		 giProfReadBytes;

extern void*			gpfDmxProfFile;
//extern unsigned char*	gszDmxProfFileName[256];
extern char	gszDmxProfFileName[256];
extern void*			gpfADecProfFile;
//extern unsigned char*	gszADecProfFileName[256];
extern char	gszADecProfFileName[256];
extern void*			gpfVDecProfFile;
//extern unsigned char*	gszVDecProfFileName[256];
extern char	gszVDecProfFileName[256];

// for cumulate mode
extern long glProfOutputCount[PROF_INDEX_MAX];


//#if defined(__GNUC__) && defined(ARM)
#if defined(__GNUC__)
/*-----------------------------------------------
 *	Linux 
-----------------------------------------------*/
#include <sys/time.h>
#define	PRT64I	"lld"

extern long long glProfStartTime[PROF_INDEX_MAX];
extern long long glProfEndTime[PROF_INDEX_MAX];
extern long long glProfResultTime[PROF_INDEX_MAX];
extern long long glCumulateTime[PROF_INDEX_MAX];

#define PROF_START(idx)\
{\
	struct timeval _tv;\
	gettimeofday(&_tv,0);\
	glProfStartTime[idx] = ((long long)_tv.tv_sec)*1000000 + ((long long)_tv.tv_usec);\
}

#define PROF_END(idx)\
{\
	struct timeval _tv;\
	gettimeofday(&_tv,0);\
	glProfEndTime[idx] = ((long long)_tv.tv_sec)*1000000 + ((long long)_tv.tv_usec);\
	glProfResultTime[idx] += (glProfEndTime[idx] - glProfStartTime[idx]);\
	glCumulateTime[idx] += glProfResultTime[idx];\
}

#define PROF_CLEAR(idx)				{glProfResultTime[idx] = 0;}
#define PROF_RESULT_TIME(idx)		glProfResultTime[idx]
#define PROF_CUMULATE_TIME(idx)		glCumulateTime[idx]
#define PROF_CONFIG()				{}

#elif defined(WIN32) || defined(_WIN32_WCE)
/*-----------------------------------------------
 *	WinCE
-----------------------------------------------*/
#include <Windows.h>
#define	PRT64I	"I64d"

extern double	gfProfDivider;
extern __int64	glProfStartTime[PROF_INDEX_MAX];
extern __int64	glProfEndTime[PROF_INDEX_MAX];
extern __int64	glProfResultTime[PROF_INDEX_MAX];
extern __int64	glCumulateTime[PROF_INDEX_MAX];

#define PROF_START(idx)\
{\
	LARGE_INTEGER count;\
	QueryPerformanceCounter(&count);\
	glProfStartTime[idx] = count.QuadPart;\
}

#define PROF_END(idx)\
{\
	LARGE_INTEGER count;\
	QueryPerformanceCounter(&count);\
	glProfEndTime[idx] = count.QuadPart;\
	glProfResultTime[idx] += (__int64)((double)(glProfEndTime[idx] - glProfStartTime[idx]) / gfProfDivider);\
	glCumulateTime[idx] += glProfResultTime[idx];\
}

#define PROF_CLEAR(idx)				{glProfResultTime[idx] = 0;}
#define PROF_RESULT_TIME(idx)		glProfResultTime[idx]
#define PROF_CUMULATE_TIME(idx)		glCumulateTime[idx]

#define PROF_CONFIG()\
{\
	LARGE_INTEGER freq;\
	QueryPerformanceFrequency(&freq);\
	gfProfDivider = (double)freq.QuadPart / 1000000;\
}

#else

#define PROF_START(idx)				{}
#define PROF_END(idx)				{}
#define PROF_CLEAR(idx)				{}
#define PROF_RESULT_TIME(idx)		0
#define PROF_CUMULATE_TIME(idx)		0
#define PROF_CONFIG()				{}

#endif

void cdk_set_prof_fread_func(unsigned int (*pfProfFreadFunc)(void*, unsigned int, unsigned int, void*) );
unsigned int cdk_fread_prof (void* pSrc, unsigned int uiSize, unsigned int uiCount, void* pf);


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
// profile main
//
/* profiler open */
#define PROF_OPEN(ptr_cdk)\
{\
	int _idx/*, _len*/;\
	/*char _szProfPrefix[256];*/\
	/*char *_pszPrefix = _szProfPrefix;*/\
	for(_idx = 0; _idx < PROF_INDEX_MAX; _idx++) {\
		glProfStartTime[_idx]	= 0;\
		glProfEndTime[_idx]		= 0;\
		glProfResultTime[_idx]	= 0;\
		glCumulateTime[_idx]	= 0;\
		glProfOutputCount[_idx] = 0;\
	}\
	/*_idx = 0;*/\
	/*_len = strlen(ptr_cdk->m_pcOpenFileName);*/\
	/*while( ptr_cdk->m_pcOpenFileName[--_len] != '.' );*/\
	/*while( _idx < _len ) _szProfPrefix[_idx] = ptr_cdk->m_pcOpenFileName[_idx++];*/\
	/*_szProfPrefix[_idx] = NULL;*/\
	/*sprintf(gszDmxProfFileName, "[%s]%s.dprof", ptr_cdk->m_pcOpenFileName+_len+1, _szProfPrefix);*/\
	/*sprintf(gszADecProfFileName, "[%s]%s.vprof", ptr_cdk->m_pcOpenFileName+_len+1, _szProfPrefix);*/\
	/*sprintf(gszVDecProfFileName, "[%s]%s.aprof", ptr_cdk->m_pcOpenFileName+_len+1, _szProfPrefix);*/\
	sprintf(gszDmxProfFileName, "%s.dprof", ptr_cdk->m_pcOpenFileName);\
	sprintf(gszADecProfFileName, "%s.aprof", ptr_cdk->m_pcOpenFileName);\
	sprintf(gszVDecProfFileName, "%s.vprof", ptr_cdk->m_pcOpenFileName);\
	if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_DMX ) {\
		gpfDmxProfFile = cdk_fopen( (void*)gszDmxProfFileName, "w");\
		if( gpfDmxProfFile == NULL )\
			DSTATUS( "[CDK_CORE] Demuxer profile report open failed (%s)\n", gszDmxProfFileName );\
		else {\
			if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_CUMULATE ) \
				fprintf(gpfDmxProfFile, "<< Demuxer Profile Report - cmulutive mode >> \n");\
			else\
				fprintf(gpfDmxProfFile, "<< Demuxer Profile Report >> \n");\
			fprintf(gpfDmxProfFile, \
				" \n"\
				" - CDK Version: %s \n"\
				" - Input: \"%s\" \n"\
				" \n"\
				"================================================================================\n",\
				CDK_VERSION_STRING,	ptr_cdk->m_pcOpenFileName);\
		}\
	}\
	if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_ADEC ) {\
		gpfADecProfFile = cdk_fopen( (void*)gszADecProfFileName, "w");\
		if( gpfADecProfFile == NULL )\
			DSTATUS( "[CDK_CORE] Audio profile report open failed = (%s)\n", gszADecProfFileName );\
		else {\
			if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_CUMULATE ) \
				fprintf(gpfADecProfFile, "<< Audio Decoder Profile Report - cmulutive mode >> \n");\
			else\
				fprintf(gpfADecProfFile, "<< Audio Decoder Profile Report >> \n");\
			fprintf(gpfADecProfFile, \
				" \n"\
				" - CDK Version: %s \n"\
				" - Input: \"%s\" \n"\
				" \n"\
				"================================================================================\n",\
				CDK_VERSION_STRING,	ptr_cdk->m_pcOpenFileName);\
		}\
	}\
	if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_VDEC ) {\
		gpfVDecProfFile = cdk_fopen( (void*)gszVDecProfFileName, "w");\
		if( gpfVDecProfFile == NULL )\
			DSTATUS( "[CDK_CORE] Video profile report open failed = (%s)\n", gszVDecProfFileName );\
		else {\
			if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_CUMULATE ) \
				fprintf(gpfVDecProfFile, "<< Video Decoder Profile Report - cmulutive mode >> \n");\
			else\
				fprintf(gpfVDecProfFile, "<< Video Decoder Profile Report >> \n");\
			fprintf(gpfVDecProfFile, \
				" \n"\
				" - CDK Version: %s \n"\
				" - Input: \"%s\" \n"\
				" \n"\
				"================================================================================\n",\
				CDK_VERSION_STRING,	ptr_cdk->m_pcOpenFileName);\
		}\
	}\
	PROF_CONFIG();\
}

/* profiler close */
#define PROF_CLOSE(ptr_cdk)\
{\
	if( (ptr_cdk->m_iPlayMode & PLAYMODE_PROF_CUMULATE) == 0 ) {\
		if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_DMX ) {\
			if( gpfDmxProfFile ) {\
				fprintf(gpfDmxProfFile, \
					"================================================================================\n"\
					"Total Demuxing Time: %"PRT64I" us \n"\
					"Total File Read Time: %"PRT64I" us \n",\
					PROF_CUMULATE_TIME( PROF_IDX_DMX ),\
					PROF_CUMULATE_TIME( PROF_IDX_FIO ));\
			}\
		}\
		if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_ADEC ) {\
			if( gpfADecProfFile ) {\
				fprintf(gpfADecProfFile, \
					"================================================================================\n"\
					"Total Audio Decoding Time: %"PRT64I" us \n",\
					PROF_CUMULATE_TIME( PROF_IDX_ADEC ));\
			}\
		}\
		if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_VDEC ) {\
			if( gpfVDecProfFile ) {\
				fprintf(gpfVDecProfFile, \
					"================================================================================\n"\
					"Total Video Decoding Time: %"PRT64I" us \n",\
					PROF_CUMULATE_TIME( PROF_IDX_VDEC ));\
			}\
		}\
	}\
	if( gpfDmxProfFile )\
		cdk_fclose(gpfDmxProfFile);\
	if( gpfADecProfFile )\
		cdk_fclose(gpfADecProfFile);\
	if( gpfVDecProfFile )\
		cdk_fclose(gpfVDecProfFile);\
}


/* Demuxer */
#define PROF_DMX_START(ptr_cdk)\
{\
	if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_DMX ) {\
		giProfReadBytes = 0;\
		PROF_CLEAR( PROF_IDX_FIO );\
		PROF_CLEAR( PROF_IDX_DMX );\
		PROF_START( PROF_IDX_DMX );\
	}\
}

#define PROF_DMX_END(ptr_cdk, dmx_out, ret_val)\
{\
	static int _prof_dmx_idx;\
	if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_DMX ) {\
		PROF_END( PROF_IDX_DMX );\
		if( gpfDmxProfFile ) {\
			if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_CUMULATE )	{\
				if( (dmx_out.m_iTimeStamp / 1000) > glProfOutputCount[PROF_IDX_DMX] ) {\
					fprintf(gpfDmxProfFile, \
						"[PTS: %08d ms][DmxTime: %19"PRT64I" us] \n",\
						dmx_out.m_iTimeStamp,\
						PROF_CUMULATE_TIME(PROF_IDX_DMX) );\
					PROF_CUMULATE_TIME(PROF_IDX_DMX) = 0;\
					glProfOutputCount[PROF_IDX_DMX] = dmx_out.m_iTimeStamp / 1000;\
				}\
			}\
			else {\
				if( dmx_out.m_iPacketType == AV_PACKET_VIDEO )			fprintf(gpfDmxProfFile, "[%08d][V]", _prof_dmx_idx++);\
				else if( dmx_out.m_iPacketType == AV_PACKET_AUDIO )	fprintf(gpfDmxProfFile, "[%08d][A]", _prof_dmx_idx++);\
				else if( dmx_out.m_iPacketType == AV_PACKET_SUBTITLE )	fprintf(gpfDmxProfFile, "[%08d][S]", _prof_dmx_idx++);\
				else												fprintf(gpfDmxProfFile, "[%08d][-]", _prof_dmx_idx++);\
				fprintf(gpfDmxProfFile, \
						"[ret: %d][PTS: %08d ms][Len: %07d][DMX: %07"PRT64I" us][FIO: %07"PRT64I" us (Read: %07ld)] \n",\
						ret_val,\
						dmx_out.m_iTimeStamp,\
						dmx_out.m_iPacketSize,\
						PROF_RESULT_TIME(PROF_IDX_DMX),\
						PROF_RESULT_TIME(PROF_IDX_FIO),\
						giProfReadBytes);\
			}\
		}\
	}\
}


/* Demuxer (file read) */
#define PROF_FIO_START(ptr_cdk)\
{\
	if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_DMX ) {\
		PROF_START( PROF_IDX_FIO );\
	}\
}

#define PROF_FIO_END(ptr_cdk)\
{\
	if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_DMX ) {\
		PROF_END( PROF_IDX_FIO );\
	}\
}


/* Audio Codec */
#define PROF_ADEC_START(ptr_cdk)\
{\
	if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_ADEC ) {\
		PROF_CLEAR( PROF_IDX_ADEC );\
		PROF_START( PROF_IDX_ADEC );\
	}\
}

#define PROF_ADEC_END(ptr_cdk, in_len, adec_out, ret_val, pdmx_out)\
{\
	static int _prof_adec_idx;\
	if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_ADEC ) {\
		PROF_END( PROF_IDX_ADEC );\
		if( gpfADecProfFile ) {\
			if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_CUMULATE )	{\
				if( ((pdmx_out)->m_iTimeStamp / 1000) > glProfOutputCount[PROF_IDX_ADEC] ) {\
					fprintf(gpfADecProfFile, \
						"[PTS: %08d ms][DecTime: %19"PRT64I" us] \n",\
						(pdmx_out)->m_iTimeStamp,\
						PROF_CUMULATE_TIME(PROF_IDX_ADEC) );\
					PROF_CUMULATE_TIME(PROF_IDX_ADEC) = 0;\
					glProfOutputCount[PROF_IDX_ADEC] = (pdmx_out)->m_iTimeStamp / 1000;\
				}\
			}\
			else\
				fprintf(gpfADecProfFile, \
					"[%08d][Len: %07d][ret: %d][DecTime: %08"PRT64I" us] \n",\
					_prof_adec_idx++, \
					in_len,\
					ret_val,\
					PROF_RESULT_TIME(PROF_IDX_ADEC));\
		}\
	}\
}


/* Video Codec */
#define PROF_VDEC_START(ptr_cdk)\
{\
	if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_VDEC ) {\
		PROF_CLEAR( PROF_IDX_VDEC );\
		PROF_START( PROF_IDX_VDEC );\
	}\
}

#define PROF_VDEC_END(ptr_cdk, codec_id, in_len, vdec_out, pdmx_out)\
{\
	static int _prof_vdec_idx;\
	int iPicType = vdec_out.m_DecOutInfo.m_iPicType;\
	int iPicStruct = gsVDecOutput.m_DecOutInfo.m_iPictureStructure;\
	if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_VDEC ) {\
		PROF_END( PROF_IDX_VDEC );\
		if( gpfVDecProfFile )  {\
			if( ptr_cdk->m_iPlayMode & PLAYMODE_PROF_CUMULATE )	{\
				if( ((pdmx_out)->m_iTimeStamp / 1000) > glProfOutputCount[PROF_IDX_VDEC] ) {\
					fprintf(gpfVDecProfFile, \
						"[PTS: %08d ms][DecTime: %19"PRT64I" us] \n",\
						(pdmx_out)->m_iTimeStamp,\
						PROF_CUMULATE_TIME(PROF_IDX_VDEC) );\
					PROF_CUMULATE_TIME(PROF_IDX_VDEC) = 0;\
					glProfOutputCount[PROF_IDX_VDEC] = (pdmx_out)->m_iTimeStamp / 1000;\
				}\
			}\
			else{\
				fprintf(gpfVDecProfFile, "[%08d]", _prof_vdec_idx++);\
				switch ( codec_id ) {\
				case STD_MPEG2 : \
					if( iPicType == PIC_TYPE_I )				fprintf(gpfVDecProfFile, "[I]" );\
					else if( iPicType == PIC_TYPE_P )			fprintf(gpfVDecProfFile, "[P]" );\
					else if( iPicType == PIC_TYPE_B )			fprintf(gpfVDecProfFile, "[B]" );\
					else										fprintf(gpfVDecProfFile, "[D]" );\
					break;\
				case STD_MPEG4 :\
					if( iPicType == PIC_TYPE_I )				fprintf(gpfVDecProfFile, "[I]" );\
					else if( iPicType == PIC_TYPE_P )			fprintf(gpfVDecProfFile, "[P]" );\
					else if( iPicType == PIC_TYPE_B )			fprintf(gpfVDecProfFile, "[B]" );\
					else if( iPicType == PIC_TYPE_B_PB )		fprintf(gpfVDecProfFile, "[B(PB)]" );\
					else										fprintf(gpfVDecProfFile, "[S]" );\
					break;\
				case STD_VC1 :\
					if( iPicStruct == 3)  {\
						if( (iPicType>>3) == PIC_TYPE_I )		fprintf(gpfVDecProfFile, "[I(TOP)   ]" );\
						else if( (iPicType>>3) == PIC_TYPE_P )	fprintf(gpfVDecProfFile, "[P(TOP)   ]" );\
						else if( (iPicType>>3) == 2 )			fprintf(gpfVDecProfFile, "[BI(TOP)  ]" );\
						else if( (iPicType>>3) == 3 )			fprintf(gpfVDecProfFile, "[B(TOP)   ]" );\
						else if( (iPicType>>3) == 4 )			fprintf(gpfVDecProfFile, "[SKIP(TOP)]" );\
						else									fprintf(gpfVDecProfFile, "[FORB(TOP)]" );\
						if( (iPicType&0x7) == PIC_TYPE_I )		fprintf(gpfVDecProfFile, "[I(BOT)   ]" );\
						else if( (iPicType&0x7) == PIC_TYPE_P )	fprintf(gpfVDecProfFile, "[P(BOT)   ]" );\
						else if( (iPicType&0x7) == 2 )			fprintf(gpfVDecProfFile, "[BI(BOT)  ]" );\
						else if( (iPicType&0x7) == 3 )			fprintf(gpfVDecProfFile, "[B(BOT)   ]" );\
						else if( (iPicType&0x7) == 4 )			fprintf(gpfVDecProfFile, "[SKIP(BOT)]" );\
						else									fprintf(gpfVDecProfFile, "[FORB(BOT)]" );\
					}\
					else{\
						if( (iPicType>>3) == PIC_TYPE_I )		fprintf(gpfVDecProfFile, "[I   ]" );\
						else if( (iPicType>>3) == PIC_TYPE_P )	fprintf(gpfVDecProfFile, "[P   ]" );\
						else if( (iPicType>>3) == 2 )			fprintf(gpfVDecProfFile, "[BI  ]" );\
						else if( (iPicType>>3) == 3 )			fprintf(gpfVDecProfFile, "[B   ]" );\
						else if( (iPicType>>3) == 4 )			fprintf(gpfVDecProfFile, "[SKIP]" );\
						else									fprintf(gpfVDecProfFile, "[FORB]" );\
					}\
					break;\
				default:\
					if( iPicType == PIC_TYPE_I )				fprintf(gpfVDecProfFile, "[I]" );\
					else if( iPicType == PIC_TYPE_P )			fprintf(gpfVDecProfFile, "[P]" );\
					else if( iPicType == PIC_TYPE_B )			fprintf(gpfVDecProfFile, "[B]" );\
					else										fprintf(gpfVDecProfFile, "[U]" );\
				}\
				fprintf(gpfVDecProfFile, \
						"[Len: %07d][Output: %d][DecState: %d][DecTime: %08"PRT64I" us] \n",\
						in_len,\
						gsVDecOutput.m_DecOutInfo.m_iOutputStatus,\
						gsVDecOutput.m_DecOutInfo.m_iDecodingStatus,\
						PROF_RESULT_TIME(PROF_IDX_VDEC));\
			}\
		}\
	}\
}

#endif //__CDK_PROF_H__