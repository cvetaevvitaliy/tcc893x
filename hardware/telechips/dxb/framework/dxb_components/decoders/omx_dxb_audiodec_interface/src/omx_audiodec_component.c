/**

  Copyright (C) 2007-2008  STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

#include <omxcore.h>
#include <omx_base_audio_port.h>

#include <omx_audiodec_component.h>
#include "omx_aacdec_component.h"
#include "omx_mp2dec_component.h"
#include "omx_ac3dec_component.h"
#include "omx_spdif_component.h"
#include <tccaudio.h>
#include <OMX_TCC_Index.h>
#include "TCCFile.h"
#include "TCCMemory.h"
#include "cdk_sys.h"

#include "mute_strategy.h"

#ifdef HAVE_ANDROID_OS
#define USE_EXTERNAL_BUFFER 0
#define LOG_TAG	"OMX_TCC_AUDIODEC"
#include <utils/Log.h>

//#define DEBUG_ON
#ifdef DEBUG_ON
#define DBUG_MSG(x...)	ALOGD(x)
#else
#define DBUG_MSG(x...)
#endif

//#define MEM_DEBUG
#ifdef MEM_DEBUG
#define LOGMSG(x...) ALOGD(x)
#else
#define LOGMSG(x...)
#endif

// for APE -------------------------------------------
#define CODEC_SPECIFIC_MARKER 0x12345678
#define CODEC_SPECIFIC_MARKER_OFFSET 8
#define CODEC_SPECIFIC_INFO_OFFSET 4

typedef struct ape_dmx_exinput_t
{
	unsigned int m_uiInputBufferByteOffset;
	unsigned int m_uiCurrentFrames;
} ape_dmx_exinput_t;
// for APE -------------------------------------------

#endif /* HAVE_ANDROID_OS */

#define		BUFFER_INDICATOR	   "TDXB"
#define		BUFFER_INDICATOR_SIZE  4

typedef struct _tagSignalQuality{
	unsigned int	SNR;
	unsigned int	PCBER;
	float			RSBER;
	double 		RSSI;
}tSignalQuality,*PSIGNALQUALITY;

OMX_S16 iTotalHandle = TOTAL_AUDIO_TRACK;

#define SPDIF_BUFFER_SIZE      (1024 * 1024)
#include <cutils/properties.h>
#include <spdif_parse.h>

enum
{
	AUDIO_OUTPUT_HDMI_STEREO = 0,
	AUDIO_OUTPUT_HDMI_PASS_THROUGH = 4,
	AUDIO_OUTPUT_SPDIF_STEREO = 1,
	AUDIO_OUTPUT_SPDIF_PASS_THROUGH = 2,
};

typedef long long (*pfnDemuxGetSTC)(int itype, long long lpts, unsigned int index, int log);
static pfnDemuxGetSTC gfnDemuxGetSTCValue = NULL;

//#define	USE_PCM_GAIN
#ifdef USE_PCM_GAIN
float gPCM_GAIN = 1.0;
#endif

enum {
	TCCDXB_JAPAN = 0,
	TCCDXB_BRAZIL
};

int guiSamplingRate = 48000; //default 48000
int guiChannels = 2; //default stereo

#define	AUDIOMONITOR_TIME_DISCONTINUITY	500	// unit: ms
struct _tagAudioMonitor {
	long long lastTimestamp;	// unit: ms
	int lastDuration;			// unit: ms
	int flag;					// 1: on-going, 0-stop
} stAudioMonitor[2], *pAudioMonitor;

void AudioMonInit (void)
{
	ALOGD("In %s", __func__);

	int i;
	for (i=0; i < 2; i++) {
		stAudioMonitor[i].lastTimestamp = 0;
		stAudioMonitor[i].lastDuration = 0;
		stAudioMonitor[i].flag = 0;
	}
}
void AudioMonRun(unsigned int uiDecoderID, int start_stop)
{
	ALOGD("In %s(%d), flag=%d", __func__, uiDecoderID, start_stop);

	if (uiDecoderID < 2) {
		pAudioMonitor = &stAudioMonitor[uiDecoderID];
		if (start_stop)
			pAudioMonitor->flag = 1;
		else
			pAudioMonitor->flag = 0;
	}
}
int AudioMonGetState (unsigned int uiDecoderID)
{
	if (uiDecoderID < 2) {
		pAudioMonitor = &stAudioMonitor[uiDecoderID];
		return pAudioMonitor->flag;
	} else
		return 0;
}
void AudioMonSet (unsigned int uiDecoderID, long long pts, int duration)
{
	if (uiDecoderID < 2) {
		pAudioMonitor = &stAudioMonitor[uiDecoderID];

		pAudioMonitor->lastTimestamp = pts;
		pAudioMonitor->lastDuration = duration;
	}
}
int AudioMonCheckContinuity (unsigned int uiDecoderID, long long cur_pts)
{
	int	rtn = 0;
	long long expected_timestamp;

	if (uiDecoderID < 2) {
		pAudioMonitor = &stAudioMonitor[uiDecoderID];
		if (pAudioMonitor->flag) {
			if(pAudioMonitor->lastTimestamp < cur_pts) {
				expected_timestamp = pAudioMonitor->lastTimestamp + (long long)pAudioMonitor->lastDuration;
				if ((expected_timestamp + AUDIOMONITOR_TIME_DISCONTINUITY) < cur_pts)
					rtn = 1;
			}
		}
	}
	return rtn;
}

void AudioSettingChangeRequest(OMX_COMPONENTTYPE *openmaxStandComp, int value);

// --------------------------------------------------
// memory management functions
// --------------------------------------------------
void * WRAPPER_malloc(unsigned int size)
{
	LOGMSG("malloc size %d", size);
	void * ptr = malloc(size);
	LOGMSG("malloc ptr %x", ptr);
	return ptr;
}

void * WRAPPER_calloc(unsigned int size, unsigned int count)
{
	LOGMSG("calloc size %d, count %d", size, count);
	void * ptr = calloc(size, count);
	LOGMSG("calloc ptr %x", ptr);
	return ptr;
}
void WRAPPER_free(void * ptr)
{
	LOGMSG("free %x", ptr);
	free(ptr);
	LOGMSG("free out");
}

void * WRAPPER_realloc(void * ptr, unsigned int size)
{
	LOGMSG("realloc ptr %x, size %d", ptr, size);
	return realloc(ptr, size);
}

void * WRAPPER_memcpy(void* dest, const void* src, unsigned int size)
{
	LOGMSG("memcpy dest ptr %x, src ptr %x, size %d", dest, src, size);
	return memcpy(dest, src, size);
}

void WRAPPER_memset(void* ptr, int val, unsigned int size)
{
	LOGMSG("memset ptr %x, val %d, size %d", ptr, val, size);
	memset(ptr, val, size);
}

void * WRAPPER_memmove(void* dest, const void* src, unsigned int size)
{
	LOGMSG("memmove dest %x, src %x, size %d", dest, src, size);
	return memmove(dest, src, size);
}

// ---------------------------------------------------------
// ---------------------------------------------------------

void AudioInfo_print(cdmx_info_t *info)
{
	DBUG_MSG("Audio Info Start=====================================>");
	
	DBUG_MSG("common!!");
	DBUG_MSG("m_iTotalNumber = %d", info->m_sAudioInfo.m_iTotalNumber);
	DBUG_MSG("m_iSamplePerSec = %d", info->m_sAudioInfo.m_iSamplePerSec);
	DBUG_MSG("m_iBitsPerSample = %d", info->m_sAudioInfo.m_iBitsPerSample);
	DBUG_MSG("m_iChannels = %d", info->m_sAudioInfo.m_iChannels);
	DBUG_MSG("m_iFormatId = %d", info->m_sAudioInfo.m_iFormatId);
	DBUG_MSG("--------------------------------------------------------------------");
	
	DBUG_MSG("extra info (common)!!");
	DBUG_MSG("m_pExtraData = 0x%x", info->m_sAudioInfo.m_pExtraData);
	DBUG_MSG("m_iExtraDataLen = %d", info->m_sAudioInfo.m_iExtraDataLen);
	DBUG_MSG("--------------------------------------------------------------------");

	DBUG_MSG("mp4 info!!");
	DBUG_MSG("m_iFramesPerSample = %d", info->m_sAudioInfo.m_iFramesPerSample);
	DBUG_MSG("m_iTrackTimeScale = %d", info->m_sAudioInfo.m_iTrackTimeScale);
	DBUG_MSG("m_iSamplesPerPacket = 0x%x", info->m_sAudioInfo.m_iSamplesPerPacket);
	DBUG_MSG("--------------------------------------------------------------------");

	DBUG_MSG("AVI info!!");
	DBUG_MSG("m_iNumAudioStream = %d", info->m_sAudioInfo.m_iNumAudioStream);
	DBUG_MSG("m_iCurrAudioIdx = %d", info->m_sAudioInfo.m_iCurrAudioIdx);
	DBUG_MSG("m_iBlockAlign = %d", info->m_sAudioInfo.m_iBlockAlign);
	DBUG_MSG("--------------------------------------------------------------------");

	DBUG_MSG("ASF info skip!!");
	DBUG_MSG("--------------------------------------------------------------------");

	DBUG_MSG("MPG info!!");
	DBUG_MSG("m_iBitRate = %d", info->m_sAudioInfo.m_iBitRate);
	DBUG_MSG("--------------------------------------------------------------------");

	DBUG_MSG("RM info!!");
	DBUG_MSG("ulActualRate = %d", info->m_sAudioInfo.m_iActualRate);
	DBUG_MSG("usAudioQuality = %d", info->m_sAudioInfo.m_sAudioQuality);
	DBUG_MSG("usFlavorIndex = %d", info->m_sAudioInfo.m_sFlavorIndex);
	DBUG_MSG("ulBitsPerFrame = %d", info->m_sAudioInfo.m_iBitsPerFrame);
	DBUG_MSG("ulGranularity = %d", info->m_sAudioInfo.m_iGranularity);
	DBUG_MSG("ulOpaqueDataSize = %d", info->m_sAudioInfo.m_iOpaqueDataSize);
	DBUG_MSG("pOpaqueData = 0x%x", info->m_sAudioInfo.m_pOpaqueData);
	DBUG_MSG("ulSamplesPerFrame = %d", info->m_sAudioInfo.m_iSamplesPerFrame);
	DBUG_MSG("frameSizeInBits = %d", info->m_sAudioInfo.m_iframeSizeInBits);
	DBUG_MSG("ulSampleRate = %d", info->m_sAudioInfo.m_iSampleRate);
	DBUG_MSG("cplStart = %d", info->m_sAudioInfo.m_scplStart);
	DBUG_MSG("cplQBits = %d", info->m_sAudioInfo.m_scplQBits);
	DBUG_MSG("nRegions = %d", info->m_sAudioInfo.m_snRegions);
	DBUG_MSG("dummy = %d", info->m_sAudioInfo.m_sdummy);
	DBUG_MSG("m_iFormatId = %d", info->m_sAudioInfo.m_iFormatId);
	DBUG_MSG("--------------------------------------------------------------------");

	DBUG_MSG("for Audio Only Path!!");
	DBUG_MSG("m_iMinDataSize = %d", info->m_sAudioInfo.m_iMinDataSize);
	DBUG_MSG("Audio Info End =====================================>");
}

void cdk_callback_func_init( void)
{
	cdk_malloc		= malloc;
	cdk_nc_malloc	= malloc;
	cdk_free		= free;
	cdk_nc_free		= free;
	cdk_memcpy		= memcpy;
	cdk_memset		= memset;
	cdk_realloc		= realloc;
	cdk_memmove		= memmove;

	cdk_sys_malloc_physical_addr = NULL;
	cdk_sys_free_physical_addr	 = NULL;
	cdk_sys_malloc_virtual_addr	 = NULL;
	cdk_sys_free_virtual_addr	 = NULL;

	cdk_fopen		= fopen;
	cdk_fread		= fread;
	cdk_fseek		= fseek;
	cdk_ftell		= ftell;
	cdk_fwrite		= fwrite;
	cdk_fclose		= fclose;
	cdk_unlink		= NULL;

	cdk_fseek64		= fseek;
	cdk_ftell64		= ftell;
	return;
}

OMX_U32 CheckAudioStart(OMX_S16 nDevID, OMX_COMPONENTTYPE * openmaxStandComp)
{
	OMX_U8 ucCount, i;
	OMX_U32 ulAudioStart;
	OMX_U32 ulAudioFormat;
	OMX_U32 retry_count = 50;
	
	omx_audiodec_component_PrivateType *omx_private = openmaxStandComp->pComponentPrivate;
    if(omx_private->pAudioDecPrivate[nDevID].stAudioStart.nState != TCC_DXBAUDIO_OMX_Dec_None)
    {
        ulAudioStart = omx_private->pAudioDecPrivate[nDevID].stAudioStart.nState;
        ulAudioFormat = omx_private->pAudioDecPrivate[nDevID].stAudioStart.nFormat;
        omx_private->pAudioDecPrivate[nDevID].stAudioStart.nState = TCC_DXBAUDIO_OMX_Dec_None;
        omx_private->pAudioDecPrivate[nDevID].stAudioStart.nFormat = 0;

        pthread_mutex_lock(&omx_private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
        if(ulAudioStart == TCC_DXBAUDIO_OMX_Dec_Stop)
        {
            omx_private->pAudioDecPrivate[nDevID].bAudioStarted = OMX_FALSE;

		    while(retry_count > 0){
			    if(omx_private->pAudioDecPrivate[nDevID].bAudioInDec == OMX_FALSE){
				    break;
			    }
			    usleep(1000);
			    retry_count--;
		    }

            DBUG_MSG("%s - TCC_DXBAUDIO_OMX_Dec_Stop\n", __func__);

			AudioMonRun(nDevID, 0);
        }
        else if(ulAudioStart == TCC_DXBAUDIO_OMX_Dec_Start)
        {
            OMX_ERRORTYPE err = OMX_ErrorNone;  
            omx_audiodec_component_LibDeinit (nDevID, omx_private);

            if(ulAudioFormat == TCC_DXBAUDIO_OMX_Codec_AC3)
            {
                if(omx_private->pAudioDecPrivate[nDevID].audio_coding_type != OMX_AUDIO_CodingAC3)
                {
					if(omx_private->iSPDIFMode == AUDIO_OUTPUT_HDMI_PASS_THROUGH
						|| omx_private->iSPDIFMode == AUDIO_OUTPUT_SPDIF_PASS_THROUGH) {
						
						omx_private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingAC3;
						omx_private->pAudioDecPrivate[nDevID].iAdecType = AUDIO_ID_AC3;
					}
					else
                    	err = omx_audiodec_component_Change_Default_AC3dec(nDevID, openmaxStandComp, AUDIO_DEC_AC3_NAME);
                }
            }
			else if(ulAudioFormat == TCC_DXBAUDIO_OMX_Codec_DDP)
			{
				if(omx_private->pAudioDecPrivate[nDevID].audio_coding_type != OMX_AUDIO_CodingAC3)
                {
					if(omx_private->iSPDIFMode == AUDIO_OUTPUT_HDMI_PASS_THROUGH
						|| omx_private->iSPDIFMode == AUDIO_OUTPUT_SPDIF_PASS_THROUGH) {
						
						omx_private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingAC3;
						omx_private->pAudioDecPrivate[nDevID].iAdecType = AUDIO_ID_DDP;
					}
					else
                    	err = omx_audiodec_component_Change_Default_AC3dec(nDevID, openmaxStandComp, AUDIO_DEC_AC3_NAME);
                }
            }
            else if(ulAudioFormat == TCC_DXBAUDIO_OMX_Codec_AAC_ADTS)
            {
                if(omx_private->pAudioDecPrivate[nDevID].audio_coding_type != OMX_AUDIO_CodingAAC)
                {
                    err = omx_audiodec_component_Change_AACdec(nDevID, openmaxStandComp, AUDIO_DEC_AAC_NAME, AUDIO_SUBTYPE_AAC_ADTS);
                }
                else
                {
                    if(omx_private->pAudioDecPrivate[nDevID].cdmx_info.m_sAudioInfo.m_ulSubType != AUDIO_SUBTYPE_AAC_ADTS)
                        err = omx_audiodec_component_Change_AACdec(nDevID, openmaxStandComp, AUDIO_DEC_AAC_NAME, AUDIO_SUBTYPE_AAC_ADTS);
                }
            }
            else if(ulAudioFormat == TCC_DXBAUDIO_OMX_Codec_AAC_LATM)
            {
                if(omx_private->pAudioDecPrivate[nDevID].audio_coding_type != OMX_AUDIO_CodingAAC)
                {
                    err = omx_audiodec_component_Change_AACdec(nDevID, openmaxStandComp, AUDIO_DEC_AAC_NAME, AUDIO_SUBTYPE_AAC_LATM);
                }
                else
                {
                    if(omx_private->pAudioDecPrivate[nDevID].cdmx_info.m_sAudioInfo.m_ulSubType != AUDIO_SUBTYPE_AAC_LATM)
                        err = omx_audiodec_component_Change_AACdec(nDevID, openmaxStandComp, AUDIO_DEC_AAC_NAME, AUDIO_SUBTYPE_AAC_LATM);
                }
            }
            else if(ulAudioFormat == TCC_DXBAUDIO_OMX_Codec_AAC_PLUS)
            {
                if(omx_private->pAudioDecPrivate[nDevID].audio_coding_type != OMX_AUDIO_CodingAAC)
                {
                    err = omx_audiodec_component_Change_AACdec(nDevID, openmaxStandComp, AUDIO_DEC_AAC_NAME, AUDIO_SUBTYPE_NONE);
                }
                else
                {
                    if(omx_private->pAudioDecPrivate[nDevID].cdmx_info.m_sAudioInfo.m_ulSubType != AUDIO_SUBTYPE_NONE)
                        err = omx_audiodec_component_Change_AACdec(nDevID, openmaxStandComp, AUDIO_DEC_AAC_NAME, AUDIO_SUBTYPE_NONE);
                }
            }
            else 
            {
                if(omx_private->pAudioDecPrivate[nDevID].audio_coding_type != OMX_AUDIO_CodingMP2)
                    err = omx_audiodec_component_Change_MP2dec(nDevID, openmaxStandComp, AUDIO_DEC_MP2_NAME);
            }
            
            omx_audiodec_component_LibInit (nDevID, omx_private);
            if(err == OMX_ErrorNone)
                omx_private->pAudioDecPrivate[nDevID].bAudioStarted = OMX_TRUE;
            else
                omx_private->pAudioDecPrivate[nDevID].bAudioStarted = OMX_FALSE;

            omx_private->pAudioDecPrivate[nDevID].bAudioPaused = OMX_FALSE;

            DBUG_MSG("%s - TCC_DXBAUDIO_OMX_Dec_Start, Format=0x%x\n", __func__, ulAudioFormat);

			AudioMonInit();
        }
        pthread_mutex_unlock(&omx_private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
    }
	return 0;
}

OMX_ERRORTYPE omx_audiodec_component_LibInit(OMX_S16 nDevID, omx_audiodec_component_PrivateType* omx_audiodec_component_Private) 
{
	cdk_callback_func_init();
	omx_audiodec_component_Private->isNewBuffer = 1;
	omx_audiodec_component_Private->pAudioDecPrivate[nDevID].bPrevDecFail = OMX_FALSE;
	omx_audiodec_component_Private->pAudioDecPrivate[nDevID].iNumOfSeek = 0;
	omx_audiodec_component_Private->pAudioDecPrivate[nDevID].iStartTS = 0;
	omx_audiodec_component_Private->pAudioDecPrivate[nDevID].iSamples = 0;

	memset(omx_audiodec_component_Private->pAudioDecPrivate[nDevID].gucAudioInitBuffer, 0x0, AUDIO_INIT_BUFFER_SIZE);

	omx_spdif_component_LibInit(nDevID, omx_audiodec_component_Private);

	omx_audiodec_component_Private->pAudioDecPrivate[nDevID].guiAudioInitBufferIndex = 0;
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_audiodec_component_LibDeinit(OMX_S16 nDevID, omx_audiodec_component_PrivateType* omx_audiodec_component_Private) 
{
	omx_spdif_component_LibDeinit(nDevID, omx_audiodec_component_Private);
	if( omx_audiodec_component_Private->pAudioDecPrivate[nDevID].decode_ready == OMX_TRUE )
    {
    	if( cdk_adec_close(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].cdk_core, &omx_audiodec_component_Private->pAudioDecPrivate[nDevID].gsADec) < 0 )
	    {
		    DBUG_MSG("Audio DEC close error\n");
    		return OMX_ErrorHardware; 	  
	    }
    }
	omx_audiodec_component_Private->pAudioDecPrivate[nDevID].decode_ready = OMX_FALSE;
	omx_audiodec_component_Private->bDABMode = OMX_FALSE;
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_audiodec_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp, internalRequestMessageType *message)  
{
	OMX_S16 nDevID;
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = (omx_audiodec_component_PrivateType*)openmaxStandComp->pComponentPrivate;  
	OMX_ERRORTYPE err;
	OMX_STATETYPE eCurrentState = omx_audiodec_component_Private->state;
	DBUG_MSG("In %s\n", __func__);

	/** Execute the base message handling */
	err = omx_base_component_MessageHandler(openmaxStandComp, message);

	if (message->messageType == OMX_CommandStateSet){ 
		if ((message->messageParam == OMX_StateExecuting) && (eCurrentState == OMX_StateIdle)) {
			for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++) {
				err = omx_audiodec_component_LibInit(nDevID, omx_audiodec_component_Private);
			    memset(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart,0x0, sizeof(AudioStartInfo));
        		pthread_mutex_init(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex, NULL);
			}
			if(err!=OMX_ErrorNone) { 
				ALOGE("In %s Audio decoder library Init Failed Error=%x\n",__func__,err); 
				return err;
			}
		} else if ((message->messageParam == OMX_StateIdle) && (eCurrentState == OMX_StateExecuting || eCurrentState == OMX_StatePause)) {
			for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++) {
				err = omx_audiodec_component_LibDeinit(nDevID, omx_audiodec_component_Private);
				pthread_mutex_destroy(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
			}
			if(err!=OMX_ErrorNone) { 
				ALOGE("In %s Audio decoder library Deinit Failed Error=%x\n",__func__,err); 
				return err;
			}
		}
	}
	return err;  

}

OMX_ERRORTYPE omx_audiodec_component_GetExtensionIndex(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,
  OMX_OUT OMX_INDEXTYPE* pIndexType)
{    

	DBUG_MSG("In  %s \n",__func__);
	if(strcmp(cParameterName,TCC_AUDIO_FILE_OPEN_STRING) == 0) {
		*pIndexType = OMX_IndexVendorParamFileOpen;  
	} else if(strcmp(cParameterName,TCC_AUDIO_POWERSPECTUM_STRING) == 0){
		*pIndexType = OMX_IndexVendorConfigPowerSpectrum;  
	} else if(strcmp(cParameterName,TCC_AUDIO_MEDIA_INFO_STRING) == 0){
		*pIndexType = OMX_IndexVendorParamMediaInfo;
	}else if(strcmp(cParameterName,TCC_AUDIO_ENERGYVOLUME_STRING) == 0){
		*pIndexType = OMX_IndexVendorConfigEnergyVolume;
	}else{
		return OMX_ErrorBadParameter;
	}

  return OMX_ErrorNone;  
}

/** The destructor */
OMX_ERRORTYPE omx_audiodec_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp) 
{

	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_U32 i;
	char value[PROPERTY_VALUE_MAX];

	if(omx_audiodec_component_Private->pAudioDecPrivate) {
		AudioSettingChangeRequest(openmaxStandComp, omx_audiodec_component_Private->iSPDIFMode);
		TCC_free(omx_audiodec_component_Private->pAudioDecPrivate);
	}
	omx_audiodec_component_Private->pAudioDecPrivate = NULL;

	/* frees port/s */
	if (omx_audiodec_component_Private->ports) 
	{
		for (i=0; i < omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
		{
			if(omx_audiodec_component_Private->ports[i])
				omx_audiodec_component_Private->ports[i]->PortDestructor(omx_audiodec_component_Private->ports[i]);
		}

		TCC_free(omx_audiodec_component_Private->ports);
		omx_audiodec_component_Private->ports=NULL;
	}

	DBUG_MSG("Destructor of the Audio decoder component is called\n");

	omx_audiodec_component_Private->audio_decode_state = AUDIO_MAX_EVENT;
	
	//omx_mp3dec_component_Private->audio_valid_info = 0;
	
	omx_base_filter_Destructor(openmaxStandComp);

	return OMX_ErrorNone;

}

/** this function sets the parameter values regarding audio format & index */
OMX_ERRORTYPE omx_audiodec_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure)
{
	OMX_S32   *piArg;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_AUDIO_PARAM_PORTFORMATTYPE *pAudioPortFormat;
	OMX_AUDIO_PARAM_PCMMODETYPE* pAudioPcmMode;
	OMX_PARAM_COMPONENTROLETYPE * pComponentRole;
	OMX_AUDIO_CONFIG_SEEKTYPE* gettime;
	OMX_AUDIO_CONFIG_INFOTYPE *info;

	omx_base_audio_PortType *port;
	OMX_U32 portIndex;

	/* Check which structure we are being fed and make control its header */
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = openmaxStandComp->pComponentPrivate;
	omx_aacdec_component_PrivateType* AACPrivate;
	omx_ac3dec_component_PrivateType* AC3Private;
	omx_apedec_component_PrivateType* APEPrivate;
	omx_dtsdec_component_PrivateType* DTSPrivate;
	omx_flacdec_component_PrivateType* FLACPrivate;
	omx_mp2dec_component_PrivateType* MP2Private;
	omx_mp3dec_component_PrivateType* MP3Private;
	omx_radec_component_PrivateType* RAPrivate;
	omx_vorbisdec_component_PrivateType* VORBISPrivate;
	omx_wmadec_component_PrivateType* WMAPrivate;
	omx_wavdec_component_PrivateType* WAVPrivate;

	OMX_AUDIO_PARAM_AACPROFILETYPE* pAudioAac;
	OMX_AUDIO_PARAM_AC3TYPE * pAudioAc3;
	OMX_AUDIO_PARAM_APETYPE * pAudioApe;
	OMX_AUDIO_PARAM_DTSTYPE * pAudioDts;
	OMX_AUDIO_PARAM_FLACTYPE * pAudioFlac;
	OMX_AUDIO_PARAM_MP2TYPE * pAudioMp2;
	OMX_AUDIO_PARAM_MP3TYPE * pAudioMp3;
	OMX_AUDIO_PARAM_RATYPE * pAudioRa;
	OMX_AUDIO_PARAM_VORBISTYPE * pAudioVorbis;
	OMX_AUDIO_PARAM_WMATYPE * pAudioWma;

	OMX_S16 nDevID;
	
	if (ComponentParameterStructure == NULL) 
	{
		return OMX_ErrorBadParameter;
	}

	DBUG_MSG("   Setting parameter 0x%x\n", nParamIndex);

	switch(nParamIndex) 
	{
		case OMX_IndexParamAudioPortFormat:
			pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE*)ComponentParameterStructure;
			portIndex = pAudioPortFormat->nPortIndex;
			/*Check Structure Header and verify component state*/
			err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("In %s Parameter Check Error=%x\n",__func__,err); 
				break;
			}
			if (portIndex <= 2)
			{
				port = (omx_base_audio_PortType *) omx_audiodec_component_Private->ports[portIndex];
				memcpy(&port->sAudioParam, pAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
			} 
			else
			{
				err = OMX_ErrorBadPortIndex;
			}
		break;

		case OMX_IndexParamStandardComponentRole:
			pComponentRole = (OMX_PARAM_COMPONENTROLETYPE*)ComponentParameterStructure;
			if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_TCC_AAC_ROLE)) 
			{
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++)
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingAAC;
			}  
			else if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_AC3_ROLE)) 
			{
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++)
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingAC3;
			}  
			else if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_APE_ROLE)) 
			{
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++)
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingAPE;
			}  
			else if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_DTS_ROLE)) 
			{
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++)
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingDTS;
			}  
			else if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_FLAC_ROLE)) 
			{
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++)
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingFLAC;
			}  
			else if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_MP2_ROLE)) 
			{
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++)
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingMP2;
			}  
			else if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_MP3_ROLE)) 
			{
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++)
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingMP3;
			}  
			else if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_RA_ROLE)) 
			{
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++)
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingRA;
			}  
			else if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_VORBIS_ROLE)) 
			{
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++)
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingVORBIS;
			}  
			else if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_PCM_ROLE)) 
			{
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++)
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingPCM;
			}  
			else if (!strcmp( (char*) pComponentRole->cRole, AUDIO_DEC_TCC_WMA_ROLE)) 
			{
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++)
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type = OMX_AUDIO_CodingWMA;
			}  
			else
			{
				err = OMX_ErrorBadParameter;
			}
		break;
		
		// --------------------------------------------------------------------------------
		// codec specific parameters -------------------------------------------------------

		case OMX_IndexParamAudioAac:
			pAudioAac = (OMX_AUDIO_PARAM_AACPROFILETYPE*) ComponentParameterStructure;
			portIndex = pAudioAac->nPortIndex;
			err = omx_base_component_ParameterSanityCheck(hComponent,portIndex,pAudioAac,sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
			
			AACPrivate = (omx_aacdec_component_PrivateType *)omx_audiodec_component_Private;
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("In %s Parameter Check Error=%x\n",__func__,err); 
				break;
			}
			if (pAudioAac->nPortIndex == 0) 
			{
				memcpy(&AACPrivate->pAudioAac, pAudioAac, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
			}  
			else
			{
				err = OMX_ErrorBadPortIndex;
			}			
		break;
		
		case OMX_IndexParamAudioAC3:
			pAudioAc3 = (OMX_AUDIO_PARAM_AC3TYPE*) ComponentParameterStructure;
			portIndex = pAudioAc3->nPortIndex;
			err = omx_base_component_ParameterSanityCheck(hComponent,portIndex,pAudioAc3,sizeof(OMX_AUDIO_PARAM_AC3TYPE));
	
			AC3Private = (omx_aacdec_component_PrivateType *)omx_audiodec_component_Private;
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("[DBG_AC3]  ==> In %s Parameter Check Error=%x\n",__func__,err); 
				break;
			}
			if (pAudioAc3->nPortIndex == 0) 
			{
				memcpy(&AC3Private->pAudioAc3, pAudioAc3, sizeof(OMX_AUDIO_PARAM_AC3TYPE));
			}  
			else
			{
				err = OMX_ErrorBadPortIndex;
			}			
		break;	

		case OMX_IndexParamAudioDTS:
			pAudioDts = (OMX_AUDIO_PARAM_DTSTYPE*) ComponentParameterStructure;
			portIndex = pAudioDts->nPortIndex;
			err = omx_base_component_ParameterSanityCheck(hComponent,portIndex,pAudioDts,sizeof(OMX_AUDIO_PARAM_DTSTYPE));
			
			DTSPrivate = (omx_dtsdec_component_PrivateType *)omx_audiodec_component_Private;
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("In %s Parameter Check Error=%x\n",__func__,err); 
				break;
			}
			if (pAudioDts->nPortIndex == 0) 
			{
				memcpy(&DTSPrivate->pAudioDts, pAudioDts, sizeof(OMX_AUDIO_PARAM_DTSTYPE));
			}  
			else
			{
				err = OMX_ErrorBadPortIndex;
			}			
		break;		
		
		case OMX_IndexParamAudioMp2:
			pAudioMp2 = (OMX_AUDIO_PARAM_MP2TYPE*) ComponentParameterStructure;
			portIndex = pAudioMp2->nPortIndex;
			err = omx_base_component_ParameterSanityCheck(hComponent,portIndex,pAudioMp2,sizeof(OMX_AUDIO_PARAM_MP2TYPE));
			
			MP2Private = (omx_mp2dec_component_PrivateType *)omx_audiodec_component_Private;
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("In %s Parameter Check Error=%x\n",__func__,err); 
				break;
			}
			if (pAudioMp2->nPortIndex == 0) 
			{
				memcpy(&MP2Private->pAudioMp2, pAudioMp2, sizeof(OMX_AUDIO_PARAM_MP2TYPE));
			}  
			else
			{
				err = OMX_ErrorBadPortIndex;
			}			
		break;

		case OMX_IndexParamAudioMp3:
			pAudioMp3 = (OMX_AUDIO_PARAM_MP3TYPE*) ComponentParameterStructure;
			portIndex = pAudioMp3->nPortIndex;
			err = omx_base_component_ParameterSanityCheck(hComponent,portIndex,pAudioMp3,sizeof(OMX_AUDIO_PARAM_MP3TYPE));

			MP3Private = (omx_mp3dec_component_PrivateType *)omx_audiodec_component_Private;
			if(err!=OMX_ErrorNone)
			{ 
				DBUG_MSG("In %s Parameter Check Error=%x\n",__func__,err); 
				break;
			}
			if (pAudioMp3->nPortIndex == 0) 
			{
				memcpy(&MP3Private->pAudioMp3, pAudioMp3, sizeof(OMX_AUDIO_PARAM_MP3TYPE));
			}  
			else
			{
				err = OMX_ErrorBadPortIndex;
			}			
		break;


		// codec specific ---------------------------------------------------------------
		// ------------------------------------------------------------------------------

		case OMX_IndexVendorAudioExtraData:
		{
			OMX_VENDOR_EXTRADATATYPE* pExtradata;	
			pExtradata = (OMX_VENDOR_EXTRADATATYPE*)ComponentParameterStructure;
			
			if (pExtradata->nPortIndex <= 2) {
				/** copy the extradata in the codec context private structure */
////				memcpy(&omx_mp3dec_component_Private->audioinfo, pExtradata->pData, sizeof(rm_audio_info));
			} else {
				err = OMX_ErrorBadPortIndex;
			}
		}
		break;

		case OMX_IndexVendorParamMediaInfo:
			info = (OMX_AUDIO_CONFIG_INFOTYPE*) ComponentParameterStructure;
			
			guiSamplingRate = info->nSamplingRate;
			guiChannels = info->nChannels;
			//omx_mp3dec_component_Private->pAudioMp3.nChannels = info->nChannels;
			//omx_mp3dec_component_Private->pAudioMp3.nBitRate = info->nBitPerSample;
			//omx_mp3dec_component_Private->pAudioMp3.nSampleRate = info->nSamplingRate;
			break;
		

		case OMX_IndexParamAudioSetStart:
			{
				OMX_AUDIO_PARAM_STARTTYPE *pStartInfo;

				pStartInfo = (OMX_AUDIO_PARAM_STARTTYPE*) ComponentParameterStructure;
				if(pStartInfo->nDevID >= iTotalHandle)
					break;

                omx_audiodec_component_Private->pAudioDecPrivate[pStartInfo->nDevID].stAudioStart.nFormat = pStartInfo->nAudioFormat;
                if(pStartInfo->bStartFlag)
                {
                    omx_audiodec_component_Private->pAudioDecPrivate[pStartInfo->nDevID].stAudioStart.nState = TCC_DXBAUDIO_OMX_Dec_Start;
                }
                else
                {
                    omx_audiodec_component_Private->pAudioDecPrivate[pStartInfo->nDevID].stAudioStart.nState = TCC_DXBAUDIO_OMX_Dec_Stop;
                }
                CheckAudioStart(pStartInfo->nDevID, openmaxStandComp);
			}
			break;

		case OMX_IndexParamAudioSetDABMode:
			{
				omx_audiodec_component_Private->bDABMode = OMX_TRUE;
			}
			break;

		case OMX_IndexParamAudioSetGetSignalStrength:
			{
				omx_audiodec_component_Private->callbackfunction = (void *)ComponentParameterStructure;
			}
			break;

		case OMX_IndexVendorParamDxBSetBitrate:
			{
				OMX_S16 nDevID;
				for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++) {
					omx_audiodec_component_Private->pAudioDecPrivate[nDevID].cdmx_info.m_sAudioInfo.m_iBitRate = ComponentParameterStructure;
				}
			}
			break;

		case OMX_IndexVendorParamAudioOutputCh:
			{
				OMX_U32 *ulDemuxId = (OMX_U32 *)ComponentParameterStructure;
				omx_audiodec_component_Private->outDevID = *ulDemuxId;
			}
			break;

		case OMX_IndexVendorParamAudioAacDualMono:
			{
				TCC_DXBAUDIO_DUALMONO_TYPE	*paudio_sel;

				paudio_sel = (TCC_DXBAUDIO_DUALMONO_TYPE *)ComponentParameterStructure;
				AACPrivate = (omx_aacdec_component_PrivateType *)omx_audiodec_component_Private;
				AACPrivate->eAudioDualMonoSel = *paudio_sel;
				if ((AACPrivate->eAudioDualMonoSel != TCC_DXBAUDIO_DUALMONO_Left)
					&& (AACPrivate->eAudioDualMonoSel != TCC_DXBAUDIO_DUALMONO_Right)
					&& (AACPrivate->eAudioDualMonoSel != TCC_DXBAUDIO_DUALMONO_LeftNRight)
				)
					AACPrivate->eAudioDualMonoSel = TCC_DXBAUDIO_DUALMONO_LeftNRight;
			}
			break;
			
		case OMX_IndexVendorParamStereoMode:
			{
				TCC_DXBAUDIO_DUALMONO_TYPE *peStereoMode = (TCC_DXBAUDIO_DUALMONO_TYPE *)ComponentParameterStructure;
				omx_audiodec_component_Private->eStereoMode = *peStereoMode;
			}
			break;

		case OMX_IndexVendorParamSetAudioPause:
			{
				OMX_U32 ulDemuxId;
				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulDemuxId = (OMX_U32) piArg[0];
				omx_audiodec_component_Private->pAudioDecPrivate[ulDemuxId].bAudioPaused = (OMX_BOOL) piArg[1];
			}
			break;

#ifdef  SUPPORT_PVR
		case OMX_IndexVendorParamPlayStartRequest:
			{
				OMX_S32 ulPvrId;
				OMX_S8 *pucFileName;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];
				pucFileName = (OMX_S8 *) piArg[1];

				pthread_mutex_lock(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
				if ((omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags & PVR_FLAG_START) == 0)
				{
					omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags = PVR_FLAG_START | PVR_FLAG_CHANGED;
				}
				pthread_mutex_unlock(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
			}
			break;

		case OMX_IndexVendorParamPlayStopRequest:
			{
				OMX_S32 ulPvrId;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];

				pthread_mutex_lock(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
				if (omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags & PVR_FLAG_START)
				{
					omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags = PVR_FLAG_CHANGED;
				}
				pthread_mutex_unlock(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
			}
			break;

		case OMX_IndexVendorParamSeekToRequest:
			{
				OMX_S32 ulPvrId, nSeekTime;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];
				nSeekTime = (OMX_S32) piArg[1];

				pthread_mutex_lock(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
				if (omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags & PVR_FLAG_START)
				{
					if (nSeekTime < 0)
					{
						if (omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags & PVR_FLAG_TRICK)
						{
							omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags &= ~PVR_FLAG_TRICK;
							omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags |= PVR_FLAG_CHANGED;
						}
					}
					else
					{
						if ((omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags & PVR_FLAG_TRICK) == 0)
						{
							omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags |= (PVR_FLAG_TRICK | PVR_FLAG_CHANGED);
						}
					}
				}
				pthread_mutex_unlock(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
			}
			break;

		case OMX_IndexVendorParamPlaySpeed:
			{
				OMX_S32 ulPvrId, nPlaySpeed;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];
				nPlaySpeed = (OMX_S32) piArg[1];

				pthread_mutex_lock(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
				if (omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags & PVR_FLAG_START)
				{
					if (nPlaySpeed == 1)
					{
						if (omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags & PVR_FLAG_TRICK)
						{
							omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags &= ~PVR_FLAG_TRICK;
							omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags |= PVR_FLAG_CHANGED;
						}
					}
					else
					{
						if ((omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags & PVR_FLAG_TRICK) == 0)
						{
							omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags |= (PVR_FLAG_TRICK | PVR_FLAG_CHANGED);
						}
					}
				}
				pthread_mutex_unlock(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
			}
			break;

		case OMX_IndexVendorParamPlayPause:
			{
				OMX_S32 ulPvrId, nPlayPause;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];
				nPlayPause = (OMX_S32) piArg[1];

				pthread_mutex_lock(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
				if (omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags & PVR_FLAG_START)
				{
					if (nPlayPause == 0)
					{
						omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags |= PVR_FLAG_PAUSE;
					}
					else
					{
						omx_audiodec_component_Private->pAudioDecPrivate[ulPvrId].nPVRFlags &= ~PVR_FLAG_PAUSE;
					}
				}
				pthread_mutex_unlock(&omx_audiodec_component_Private->pAudioDecPrivate[nDevID].stAudioStart.mutex);
			}
			break;
#endif//SUPPORT_PVR

		case OMX_IndexVendorParamDxBGetSTCFunction:
			{
				gfnDemuxGetSTCValue = (pfnDemuxGetSTC)ComponentParameterStructure;
			}
			break;
		case OMX_IndexVendorParamAudioIsSupportCountry:
			{
				OMX_U32 uiDevId, uiCountry;
				OMX_U32 *piArg;
				piArg = (OMX_U32*) ComponentParameterStructure;
				uiDevId = piArg[0];
				uiCountry = piArg[1];
				omx_audiodec_component_Private->uiCountry = uiCountry;
			}
			break;

		default: /*Call the base component function*/
			return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);

	}

	if(err != OMX_ErrorNone)
		ALOGE("ERROR %s :: nParamIndex = 0x%x, error(0x%x)", __func__, nParamIndex, err);

	return err;
}  

/** this function gets the parameters regarding audio formats and index */
OMX_ERRORTYPE omx_audiodec_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure)
{

	OMX_AUDIO_PARAM_PORTFORMATTYPE *pAudioPortFormat;  
	OMX_AUDIO_PARAM_PCMMODETYPE *pAudioPcmMode;
	OMX_PARAM_COMPONENTROLETYPE * pComponentRole;
	OMX_AUDIO_CONFIG_GETTIMETYPE *gettime;
    OMX_AUDIO_CONFIG_INFOTYPE *info;
	omx_base_audio_PortType *port;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_AUDIO_SPECTRUM_INFOTYPE *power;
	OMX_AUDIO_ENERGY_VOLUME_INFOTYPE *energyvolume;
	int i;

	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = openmaxStandComp->pComponentPrivate;
	omx_aacdec_component_PrivateType* AACPrivate;
	omx_ac3dec_component_PrivateType* AC3Private;
	omx_apedec_component_PrivateType* APEPrivate;
	omx_dtsdec_component_PrivateType* DTSPrivate;
	omx_flacdec_component_PrivateType* FLACPrivate;
	omx_mp2dec_component_PrivateType* MP2Private;
	omx_mp3dec_component_PrivateType* MP3Private;
	omx_radec_component_PrivateType* RAPrivate;
	omx_vorbisdec_component_PrivateType* VORBISPrivate;
	omx_wmadec_component_PrivateType* WMAPrivate;
	omx_wavdec_component_PrivateType* WAVPrivate;

	OMX_AUDIO_PARAM_AACPROFILETYPE* pAudioAac;
	OMX_AUDIO_PARAM_AC3TYPE * pAudioAc3;
	OMX_AUDIO_PARAM_APETYPE * pAudioApe;
	OMX_AUDIO_PARAM_DTSTYPE * pAudioDts;
	OMX_AUDIO_PARAM_FLACTYPE * pAudioFlac;
	OMX_AUDIO_PARAM_MP2TYPE * pAudioMp2;
	OMX_AUDIO_PARAM_MP3TYPE * pAudioMp3;
	OMX_AUDIO_PARAM_RATYPE * pAudioRa;
	OMX_AUDIO_PARAM_VORBISTYPE * pAudioVorbis;
	OMX_AUDIO_PARAM_WMATYPE * pAudioWma;

	OMX_S16 nDevID = 0;

	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}
	DBUG_MSG("   Getting parameter 0x%x\n", nParamIndex);
	/* Check which structure we are being fed and fill its header */

	switch(nParamIndex) 
	{
		case OMX_IndexParamAudioInit:
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE))) != OMX_ErrorNone) 
			{ 
			  break;
			}
			memcpy(ComponentParameterStructure, &omx_audiodec_component_Private->sPortTypesParam, sizeof(OMX_PORT_PARAM_TYPE));
			break;    

		case OMX_IndexParamAudioPortFormat:
			pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE*)ComponentParameterStructure;
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			if (pAudioPortFormat->nPortIndex <= 2)
			{
				port = (omx_base_audio_PortType *)omx_audiodec_component_Private->ports[pAudioPortFormat->nPortIndex];
				memcpy(pAudioPortFormat, &port->sAudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
			} 
			else 
			{
				return OMX_ErrorBadPortIndex;
			}
			break;    

		case OMX_IndexParamAudioPcm:
			pAudioPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE*)ComponentParameterStructure;
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			if (pAudioPcmMode->nPortIndex > 2)
			{
				return OMX_ErrorBadPortIndex;
			}
			
			if (pAudioPcmMode->nPortIndex < 2) // input is PCM
			{
				WAVPrivate = (omx_wavdec_component_PrivateType *)omx_audiodec_component_Private;
				memcpy(pAudioPcmMode, &WAVPrivate->pAudioPcm, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
			}  
			else // output is PCM
			{
				memcpy(pAudioPcmMode, &omx_audiodec_component_Private->pAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
			}
			break;
		
		// --------------------------------------------------------------------------------
		// codec specific parameters -------------------------------------------------------

		case OMX_IndexParamAudioAac:
			pAudioAac = (OMX_AUDIO_PARAM_AACPROFILETYPE*)ComponentParameterStructure;
			AACPrivate = (omx_aacdec_component_PrivateType *)omx_audiodec_component_Private;
			if (pAudioAac->nPortIndex > 1)
			{
				return OMX_ErrorBadPortIndex;
			}
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			memcpy(pAudioAac, &AACPrivate->pAudioAac, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
			break;
		
		case OMX_IndexParamAudioAC3: // parameter read(channel,samplerate etc...)
			pAudioAc3 = (OMX_AUDIO_PARAM_AC3TYPE*)ComponentParameterStructure;
			AC3Private = (omx_ac3dec_component_PrivateType *)omx_audiodec_component_Private;
			if (pAudioAc3->nPortIndex > 1) 
			{
				return OMX_ErrorBadPortIndex;
			}
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_AC3TYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			memcpy(pAudioAc3, &AC3Private->pAudioAc3, sizeof(OMX_AUDIO_PARAM_AC3TYPE));
			break;
			
		case OMX_IndexParamAudioDTS:
			pAudioDts = (OMX_AUDIO_PARAM_DTSTYPE*)ComponentParameterStructure;
			DTSPrivate = (omx_dtsdec_component_PrivateType *)omx_audiodec_component_Private;
			if (pAudioDts->nPortIndex > 1) 
			{
				return OMX_ErrorBadPortIndex;
			}
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_DTSTYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			memcpy(pAudioDts, &DTSPrivate->pAudioDts, sizeof(OMX_AUDIO_PARAM_DTSTYPE));
			break;
			
		case OMX_IndexParamAudioMp2:
			pAudioMp2 = (OMX_AUDIO_PARAM_MP2TYPE*)ComponentParameterStructure;
			MP2Private = (omx_mp2dec_component_PrivateType *)omx_audiodec_component_Private;
			if (pAudioMp2->nPortIndex > 1) 
			{
				return OMX_ErrorBadPortIndex;
			}
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_MP2TYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			memcpy(pAudioMp2, &MP2Private->pAudioMp2, sizeof(OMX_AUDIO_PARAM_MP2TYPE));
			break;

		case OMX_IndexParamAudioMp3:
			pAudioMp3 = (OMX_AUDIO_PARAM_MP3TYPE*)ComponentParameterStructure;
			MP3Private = (omx_mp3dec_component_PrivateType *)omx_audiodec_component_Private;
			if (pAudioMp3->nPortIndex > 1) 
			{
				return OMX_ErrorBadPortIndex;
			}
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_MP3TYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			memcpy(pAudioMp3, &MP3Private->pAudioMp3, sizeof(OMX_AUDIO_PARAM_MP3TYPE));
			break;
		
		
		// codec specific ---------------------------------------------------------------
		// ------------------------------------------------------------------------------
		case OMX_IndexParamStandardComponentRole:
			pComponentRole = (OMX_PARAM_COMPONENTROLETYPE*)ComponentParameterStructure;
			if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone) 
			{ 
				break;
			}
			
			if (omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type == OMX_AUDIO_CodingAAC) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_TCC_AAC_ROLE);
			}
			else if (omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type == OMX_AUDIO_CodingAC3) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_AC3_ROLE);
			}
			else if (omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type == OMX_AUDIO_CodingAPE) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_APE_ROLE);
			}
			else if (omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type == OMX_AUDIO_CodingDTS) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_DTS_ROLE);
			}
			else if (omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type == OMX_AUDIO_CodingFLAC) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_FLAC_ROLE);
			}
			else if (omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type == OMX_AUDIO_CodingMP2) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_MP2_ROLE);
			}
			else if (omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type == OMX_AUDIO_CodingMP3) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_MP3_ROLE);
			}
			else if (omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type == OMX_AUDIO_CodingRA) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_RA_ROLE);
			}
			else if (omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type == OMX_AUDIO_CodingVORBIS) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_VORBIS_ROLE);
			}
			else if (omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type == OMX_AUDIO_CodingPCM) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_PCM_ROLE);
			}
			else if (omx_audiodec_component_Private->pAudioDecPrivate[nDevID].audio_coding_type == OMX_AUDIO_CodingWMA) 
			{
				strcpy( (char*) pComponentRole->cRole, AUDIO_DEC_TCC_WMA_ROLE);
			}
			else
			{
				strcpy( (char*) pComponentRole->cRole,"\0");;
			}
			break;

#ifdef HAVE_ANDROID_OS
			case PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX:
			{
				PV_OMXComponentCapabilityFlagsType *pCap_flags =
					(PV_OMXComponentCapabilityFlagsType *) ComponentParameterStructure;
				if (NULL == pCap_flags) {
					return OMX_ErrorBadParameter;
				}
		
				memset(pCap_flags, 0x00, sizeof(PV_OMXComponentCapabilityFlagsType));
				pCap_flags->iIsOMXComponentMultiThreaded = OMX_TRUE;
				pCap_flags->iOMXComponentSupportsExternalInputBufferAlloc 	= OMX_TRUE; //ZzaU::to use MovableInputBuffers
				pCap_flags->iOMXComponentSupportsExternalOutputBufferAlloc 	= OMX_FALSE;
				pCap_flags->iOMXComponentSupportsMovableInputBuffers		= OMX_TRUE;
				pCap_flags->iOMXComponentSupportsPartialFrames				= OMX_TRUE; //ZzaU::PartialFrames must enable to use MovableInputBuffers.
//				pCap_flags->iOMXComponentUsesNALStartCodes					= OMX_FALSE;
//				pCap_flags->iOMXComponentUsesFullAVCFrames					= OMX_FALSE;			
//				pCap_flags->iOMXComponentCanHandleIncompleteFrames			= OMX_FALSE;
			}
			break;
#endif

		case OMX_IndexVendorParamDxbGetAudioType:
		{
			int *piParamDxbGetAudioType;
			piParamDxbGetAudioType = (int *)ComponentParameterStructure;
			// [0] = decoder id
			// [1] = return for channel no
			// [2] = return for audio mode
			int iDecoderID = *piParamDxbGetAudioType;

			//default value - undefined
			*(piParamDxbGetAudioType+1) = 0;
			*(piParamDxbGetAudioType+2) = 0;

			if (iDecoderID < (int)iTotalHandle) {
				if (omx_audiodec_component_Private->pAudioDecPrivate[iDecoderID].decode_ready == OMX_TRUE) {
					if (omx_audiodec_component_Private->pAudioDecPrivate[iDecoderID].cdmx_out.m_iDecodedSamples > 0) {
						*(piParamDxbGetAudioType+1) = omx_audiodec_component_Private->pAudioDecPrivate[iDecoderID].gsADec.gsADecInput.m_uiNumberOfChannel;
						*(piParamDxbGetAudioType+2) = omx_audiodec_component_Private->pAudioDecPrivate[iDecoderID].gsADec.gsADecOutput.m_eChannelType;
					}
				}
			}
			break;
		}
		default: /*Call the base component function*/
			return omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);

	}

	return OMX_ErrorNone;

}

//#define	DUMP_INPUT_TO_FILE
//#define	DUMP_OUTPUT_TO_FILE
#if defined(DUMP_INPUT_TO_FILE) || defined(DUMP_OUTPUT_TO_FILE)
int	gDumpHandle = 0;
FILE *gFileInput, *gFileOutput;
int gInputWriteSize = 0, gOutputWriteSize=0;
void IODumpFileInit(void)
{
#ifdef 	DUMP_INPUT_TO_FILE	
	gFileInput = fopen("/sdcard/audioinput1.mp2", "wb");
	gInputWriteSize = 0;
	ALOGD("File Handle %d", gFileInput);
	if(gFileInput == NULL)
	{
		ALOGE("audio input file dump error %d",gFileInput);
	}
#endif

#ifdef 	DUMP_OUTPUT_TO_FILE	
	gFileOutput = fopen("/sdcard/audioinput1.pcm", "wb");
	gOutputWriteSize = 0;
	ALOGD("File Handle %d", gFileOutput);
	if(gFileOutput == NULL)
	{
		ALOGE("audio output file dump error %d",gFileOutput);
	}
#endif
}

int IODumpFileWriteInputData(char *data, int size)
{
	if( gFileInput )
	{
		fwrite(data, 1, size, gFileInput);
		ALOGD("INPUT : write size %d - handle %d\n", size, gFileInput);
		gInputWriteSize += size;
		if(gInputWriteSize > 300*1024)
		{
			fclose(gFileInput);
			sync();
			gFileInput = NULL;
			ALOGD("INPUT : File Saved size = %d", gInputWriteSize);			
		}
		return size;
	}
	else
		return 0;
}

int IODumpFileWriteOutputData(char *data, int size)
{
	if( gFileOutput )
	{
		fwrite(data, 1, size, gFileOutput);
		ALOGD("OUTPUT : write size %d - handle %d\n", size, gFileOutput);
		gOutputWriteSize += size;
		if(gOutputWriteSize > 2*1024*1024)
		{
			fclose(gFileOutput);
			sync();
			gFileOutput = NULL;
			ALOGD("OUTPUT : File Saved size = %d", gOutputWriteSize);			
		}
		return size;
	}
	else
		return 0;
}
#endif

void AudioSettingChangeRequest(OMX_COMPONENTTYPE *openmaxStandComp, int value)
{
	omx_audiodec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;
	(*(pPrivate->callbacks->EventHandler)) (openmaxStandComp,
		pPrivate->callbackData,
		OMX_EventDynamicResourcesAvailable,
		value, NULL, NULL);
}

#ifdef  SUPPORT_PVR
static void SetPVRFlags(omx_audiodec_component_PrivateType *omx_private, OMX_U8 uiDecoderID, OMX_BUFFERHEADERTYPE * pOutputBuffer)
{
	if (omx_private->pAudioDecPrivate[uiDecoderID].nPVRFlags & PVR_FLAG_START)
	{
		pOutputBuffer->nFlags |= OMX_BUFFERFLAG_FILE_PLAY;

		if (omx_private->pAudioDecPrivate[uiDecoderID].nPVRFlags & PVR_FLAG_TRICK)
		{
			pOutputBuffer->nFlags |= OMX_BUFFERFLAG_TRICK_PLAY;
		}
		else
		{
			pOutputBuffer->nFlags &= ~OMX_BUFFERFLAG_TRICK_PLAY;
		}
	}
	else
	{
		pOutputBuffer->nFlags &= ~OMX_BUFFERFLAG_FILE_PLAY;
		pOutputBuffer->nFlags &= ~OMX_BUFFERFLAG_TRICK_PLAY;
	}
}

static int CheckPVR(omx_audiodec_component_PrivateType *omx_private, OMX_U8 uiDecoderID, OMX_U32 ulInputBufferFlags)
{
	OMX_U32 iPVRState, iBufferState;

	iPVRState = (omx_private->pAudioDecPrivate[uiDecoderID].nPVRFlags & PVR_FLAG_START) ? 1 : 0;
	iBufferState = (ulInputBufferFlags & OMX_BUFFERFLAG_FILE_PLAY) ? 1 : 0;
	if (iPVRState != iBufferState)
	{
		return 1; // Skip
	}

	iPVRState = (omx_private->pAudioDecPrivate[uiDecoderID].nPVRFlags & PVR_FLAG_TRICK) ? 1 : 0;
	iBufferState = (ulInputBufferFlags & OMX_BUFFERFLAG_TRICK_PLAY) ? 1 : 0;
	if (iPVRState != iBufferState)
	{
		return 1; // Skip
	}

	if (omx_private->pAudioDecPrivate[uiDecoderID].nPVRFlags & PVR_FLAG_CHANGED)
	{
		omx_private->pAudioDecPrivate[uiDecoderID].nPVRFlags &= ~PVR_FLAG_CHANGED;

		omx_private->pAudioDecPrivate[uiDecoderID].iNumOfSeek = 0;
		omx_private->pAudioDecPrivate[uiDecoderID].iStartTS = 0;
		omx_private->pAudioDecPrivate[uiDecoderID].iSamples = 0;
	}

	return 0; // Decoding
}
#endif//SUPPORT_PVR

void omx_audiodec_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* inputbuffer, OMX_BUFFERHEADERTYPE* outputbuffer)
{
	
	omx_audiodec_component_PrivateType* pPrivate = openmaxStandComp->pComponentPrivate;
	omx_aacdec_component_PrivateType* pAACPrivate;

	OMX_S32 ret	= 0;
	OMX_U8* input_ptr;
	ape_dmx_exinput_t *codecSpecific = 0;
	OMX_S16 iSeekFlag = 0;
	OMX_U32 uiDemuxID = 0, uiDecoderID = 0;
	TCC_DXBAUDIO_DUALMONO_TYPE eStereoMode;
	OMX_S8 value[PROPERTY_VALUE_MAX];

#if 0
	property_get("tcc.dxb.audiodecoding", value, "0");
	sscanf(value, "%d", &pPrivate->outDevID);
#endif

	if( memcmp(inputbuffer->pBuffer,BUFFER_INDICATOR,BUFFER_INDICATOR_SIZE)==0)
	{
		memcpy(&input_ptr, inputbuffer->pBuffer + BUFFER_INDICATOR_SIZE, 4);	
		if(inputbuffer->pBuffer[BUFFER_INDICATOR_SIZE+4] == 0xAA)
		{
			uiDemuxID = inputbuffer->pBuffer[BUFFER_INDICATOR_SIZE+5];
			uiDecoderID = inputbuffer->pBuffer[BUFFER_INDICATOR_SIZE+6];
			//ALOGD("%s:Demuxer[%d]Decoder[%d]",__func__, uiDemuxID, uiDecoderID);
		}
	}else{
		input_ptr = inputbuffer->pBuffer;
		uiDecoderID= inputbuffer->pBuffer[inputbuffer->nFilledLen];
	}

#ifdef  SUPPORT_PVR
	pthread_mutex_lock(&pPrivate->pAudioDecPrivate[uiDecoderID].stAudioStart.mutex);
	if (CheckPVR(pPrivate, uiDecoderID, inputbuffer->nFlags))
	{
		inputbuffer->nFilledLen = 0;
		outputbuffer->nFilledLen = 0;
		pthread_mutex_unlock(&pPrivate->pAudioDecPrivate[uiDecoderID].stAudioStart.mutex);
		return;
	}
	pthread_mutex_unlock(&pPrivate->pAudioDecPrivate[uiDecoderID].stAudioStart.mutex);
#endif//SUPPORT_PVR

//	CheckAudioStart(uiDecoderID, openmaxStandComp);

	if(pPrivate->pAudioDecPrivate[uiDecoderID].bAudioStarted == OMX_FALSE)
	{
		inputbuffer->nFilledLen = 0;
		outputbuffer->nFilledLen = 0;
		return;
	}

	switch (pPrivate->iSPDIFMode)
	{
		case AUDIO_OUTPUT_SPDIF_PASS_THROUGH:
		property_get("persist.sys.spdif_setting", value, "0");
		if( pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3
			|| pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingDTS ) {
				if (AUDIO_OUTPUT_SPDIF_PASS_THROUGH != atoi(value)) {
					AudioSettingChangeRequest(openmaxStandComp, AUDIO_OUTPUT_SPDIF_PASS_THROUGH);
			}
			omx_spdif_component_BufferMgmtCallback(openmaxStandComp, inputbuffer, outputbuffer);
			return;
			} else if (AUDIO_OUTPUT_SPDIF_STEREO != atoi(value)) {
				AudioSettingChangeRequest(openmaxStandComp, AUDIO_OUTPUT_SPDIF_STEREO);
		}
			break;
		case AUDIO_OUTPUT_HDMI_PASS_THROUGH:
			property_get("persist.sys.spdif_setting", value, "0");
			if( pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3
				|| pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingDTS ) {
				if (AUDIO_OUTPUT_HDMI_PASS_THROUGH != atoi(value)) {
					AudioSettingChangeRequest(openmaxStandComp, AUDIO_OUTPUT_HDMI_PASS_THROUGH);
		}
				omx_spdif_component_BufferMgmtCallback(openmaxStandComp, inputbuffer, outputbuffer);
				return;
			} else if (AUDIO_OUTPUT_HDMI_STEREO != atoi(value)) {
				AudioSettingChangeRequest(openmaxStandComp, AUDIO_OUTPUT_HDMI_STEREO);
			}
			break;
	}

	outputbuffer->nFilledLen = 0;
	outputbuffer->nOffset = 0;
	outputbuffer->nDecodeID = uiDecoderID;

	DBUG_MSG("BufferMgmtCallback IN inLen = %ld, Flags = 0x%x", inputbuffer->nFilledLen, inputbuffer->nFlags);	
	//if((inputbuffer->nFlags & OMX_BUFFERFLAG_CODECCONFIG) && pPrivate->pAudioDecPrivate[uiDecoderID].decode_ready == OMX_FALSE)
	if( pPrivate->pAudioDecPrivate[uiDecoderID].decode_ready == OMX_FALSE )
	{
		memset(&pPrivate->pAudioDecPrivate[uiDecoderID].gsADec, 0, sizeof(ADEC_VARS));
		pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate = guiSamplingRate;//48000; //default 48000
		pPrivate->pAudioPcmMode[uiDecoderID].nChannels = guiChannels; //default stereo

		if(pPrivate->bDABMode == OMX_TRUE)
		{
			//pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecInit.m_unAudioCodecParams.m_unMP2.m_iDABMode = 1;
			pPrivate->pAudioDecPrivate[uiDecoderID].cdk_core.m_iAudioProcessMode = AUDIO_BROADCAST_MODE;
			DAB_Mute_Ctrl_Init();
			ALOGD("DAB_Mute_Ctrl_Init");
		}
		else
		{
			//pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecInit.m_unAudioCodecParams.m_unMP2.m_iDABMode = 0;
			pPrivate->pAudioDecPrivate[uiDecoderID].cdk_core.m_iAudioProcessMode = AUDIO_NORMAL_MODE;
		}

		pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_iBitsPerSample = pPrivate->pAudioPcmMode[uiDecoderID].nBitPerSample;
		pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_iSamplePerSec = pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate;
		pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_iChannels = pPrivate->pAudioPcmMode[uiDecoderID].nChannels;		
		pPrivate->pAudioDecPrivate[uiDecoderID].iCtype = CONTAINER_TYPE_AUDIO;//CONTAINER_TYPE_TS;//CONTAINER_TYPE_AUDIO;
		pPrivate->iGuardSamples = 0;


		if( pPrivate->pAudioDecPrivate[uiDecoderID].guiAudioInitBufferIndex+inputbuffer->nFilledLen > AUDIO_INIT_BUFFER_SIZE)
		{
			memset(pPrivate->pAudioDecPrivate[uiDecoderID].gucAudioInitBuffer, 0x0, AUDIO_INIT_BUFFER_SIZE);
			pPrivate->pAudioDecPrivate[uiDecoderID].guiAudioInitBufferIndex = 0;
		}

		memcpy(pPrivate->pAudioDecPrivate[uiDecoderID].gucAudioInitBuffer+pPrivate->pAudioDecPrivate[uiDecoderID].guiAudioInitBufferIndex, input_ptr, inputbuffer->nFilledLen);
		pPrivate->pAudioDecPrivate[uiDecoderID].guiAudioInitBufferIndex += inputbuffer->nFilledLen;
		
		pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_pExtraData = pPrivate->pAudioDecPrivate[uiDecoderID].gucAudioInitBuffer;
		pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_iExtraDataLen = pPrivate->pAudioDecPrivate[uiDecoderID].guiAudioInitBufferIndex;		
		if( pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingMP2 )
			pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_iFormatId = AV_AUDIO_MP2;
		else if( pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAC3 )
			pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_iFormatId = AV_AUDIO_AC3;
		else if( pPrivate->pAudioDecPrivate[uiDecoderID].audio_coding_type == OMX_AUDIO_CodingAAC )
		{
			pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_iFormatId = AV_AUDIO_AAC;
			if( pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_ulSubType == AUDIO_SUBTYPE_NONE )
			{
				pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_pExtraData = NULL;
				pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_iExtraDataLen = 0;	
			}
		}	
		
		pPrivate->pAudioDecPrivate[uiDecoderID].cdk_core.m_pOutWav = NULL;
	#ifdef _TCC8920_
		if (uiDecoderID == 0 && pPrivate->uiCountry == TCCDXB_BRAZIL && pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_AAC)
			pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecInit.m_unAudioCodecParams.m_unAAC.m_uiChannelMasking = 1;
	#endif
		if( cdk_adec_init(&pPrivate->pAudioDecPrivate[uiDecoderID].cdk_core,
							&pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info, 
							pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType,		// AUDIO_ID
							pPrivate->pAudioDecPrivate[uiDecoderID].iCtype,			// CONTAINER_TYPE
							pPrivate->pAudioDecPrivate[uiDecoderID].cb_function,
							&pPrivate->pAudioDecPrivate[uiDecoderID].gsADec) < 0 )	// Audio decoder function
		{
			ALOGE("Audio[%d] DEC init error.", uiDecoderID);
			inputbuffer->nFlags &= ~OMX_BUFFERFLAG_CODECCONFIG;
			// to skip all audio data
			inputbuffer->nFilledLen = 0;
			return; 	  
		}

		pPrivate->pAudioDecPrivate[uiDecoderID].decode_ready  = OMX_TRUE;
		pPrivate->isNewBuffer = 1;
		outputbuffer->nFilledLen = 0;
		inputbuffer->nFilledLen = 0;
		pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate = 0;
		pPrivate->pAudioPcmMode[uiDecoderID].nChannels = 0;
		DBUG_MSG("Audio[%d] DEC initialized.", uiDecoderID);

#if defined(DUMP_INPUT_TO_FILE) || defined(DUMP_OUTPUT_TO_FILE)
		if (gDumpHandle == uiDecoderID)
			IODumpFileInit();
#endif
		return;
	}

	pPrivate->pAudioDecPrivate[uiDecoderID].bAudioInDec = OMX_TRUE;

	if(pPrivate->pAudioDecPrivate[uiDecoderID].iCtype != CONTAINER_TYPE_AUDIO)
	{
		// if previous decoding failed, silence should be inserted 
		if(pPrivate->pAudioDecPrivate[uiDecoderID].bPrevDecFail == OMX_TRUE)
		{
			if(/*pPrivate->outDevID==uiDecoderID*/OMX_TRUE) {
				OMX_TICKS samples;
				samples = (outputbuffer->nTimeStamp - pPrivate->pAudioDecPrivate[uiDecoderID].iPrevTS) * pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate / 1000000;
				outputbuffer->nFilledLen = pPrivate->pAudioPcmMode[uiDecoderID].nChannels * samples * sizeof(short);
				memset(outputbuffer->pBuffer, 0, outputbuffer->nFilledLen);
			}
			else {
				outputbuffer->nFilledLen = 0;
			}

			pPrivate->pAudioDecPrivate[uiDecoderID].bPrevDecFail = OMX_FALSE;
			inputbuffer->nFilledLen = 0;
#ifdef PCM_INFO_SIZE
			if(outputbuffer->nFilledLen > 0) {
				memcpy(outputbuffer->pBuffer+outputbuffer->nFilledLen, &pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate, 4);
				outputbuffer->nFilledLen += 4;
				memcpy(outputbuffer->pBuffer+outputbuffer->nFilledLen, &pPrivate->pAudioPcmMode[uiDecoderID].nChannels, 4);
				outputbuffer->nFilledLen += 4;
				*(int *)(outputbuffer->pBuffer+outputbuffer->nFilledLen) = 0;
				outputbuffer->nFilledLen += 4;
			}
#endif
			pPrivate->pAudioDecPrivate[uiDecoderID].bAudioInDec = OMX_FALSE;
			return;
		}
	}

	if(inputbuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME)
	{
		iSeekFlag = 1;
		pPrivate->pAudioDecPrivate[uiDecoderID].iStartTS = inputbuffer->nTimeStamp;
		pPrivate->pAudioDecPrivate[uiDecoderID].iSamples = 0;
		pPrivate->pAudioDecPrivate[uiDecoderID].iNumOfSeek++;

		if (pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_APE)
		{
			OMX_U8* p = input_ptr;

			if ( *((OMX_U32*)(p+inputbuffer->nFilledLen-CODEC_SPECIFIC_MARKER_OFFSET)) == CODEC_SPECIFIC_MARKER)
			{
				codecSpecific = (ape_dmx_exinput_t *)(*((OMX_U32*)(p+inputbuffer->nFilledLen-CODEC_SPECIFIC_INFO_OFFSET)));
				inputbuffer->nFilledLen -= CODEC_SPECIFIC_MARKER_OFFSET;
			}
			else
			{
				ALOGE("No codec specific data");
			}
		}
	}

#if defined(DUMP_INPUT_TO_FILE)
	if (gDumpHandle == uiDecoderID)
		IODumpFileWriteInputData(input_ptr, inputbuffer->nFilledLen);
#endif
	
	pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_uiUseCodecSpecific = codecSpecific;

	/* Decode the block */
	pPrivate->pAudioDecPrivate[uiDecoderID].cdk_core.m_pOutWav = outputbuffer->pBuffer;
	
	pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_pPacketData = input_ptr;
	pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_iPacketSize = inputbuffer->nFilledLen;

	if (0)
	{
		OMX_U8* p = input_ptr;
		ALOGD("------------------------------");
		ALOGD("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		ALOGD("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", p[0+16], p[1+16], p[2+16], p[3+16], p[4+16], p[5+16], p[6+16], p[7+16], p[8+16], p[9+16], p[10+16], p[11+16], p[12+16], p[13+16], p[14+16], p[15+16]);
		ALOGD("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", p[0+32], p[1+32], p[2+32], p[3+32], p[4+32], p[5+32], p[6+32], p[7+32], p[8+32], p[9+32], p[10+32], p[11+32], p[12+32], p[13+32], p[14+32], p[15+32]);
		ALOGD("------------------------------");
	}
	ret = cdk_adec_decode(&pPrivate->pAudioDecPrivate[uiDecoderID].cdk_core,
								&pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info,
								&pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out,
								pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType,
								iSeekFlag,
								0, // alsa_status
								&pPrivate->pAudioDecPrivate[uiDecoderID].gsADec);

	if (0) {
		//check if a pts of audio is continuous or not
		int duration;
		int continuity;
		if(pPrivate->bDABMode != OMX_TRUE && ret >= 0) {
			duration = (pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_iDecodedSamples * 1000 ) / pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_iSamplePerSec;
			if (AudioMonGetState(uiDecoderID) == 0) {
				//not yet run
				AudioMonSet(uiDecoderID, inputbuffer->nTimeStamp / 1000, duration);
				AudioMonRun(uiDecoderID, 1);
			} else {
				continuity = AudioMonCheckContinuity(uiDecoderID, inputbuffer->nTimeStamp / 1000);
				if (continuity) {
					ALOGE("In %s, audio id=% found discontinuity", __func__, uiDecoderID);

					ALOGE("[ASDF]*** found discontinuity ***");
				}
				AudioMonSet(uiDecoderID, inputbuffer->nTimeStamp / 1000, duration);
			}
		}
	}

	if(pPrivate->bDABMode == OMX_TRUE)
	{
		OMX_U32 dec_error;
		OMX_S32 *pcm = (OMX_S32 *)outputbuffer->pBuffer;
		OMX_S16 left[1152];
		OMX_S16 right[1152];

		tSignalQuality sq;
		if(pPrivate->callbackfunction != NULL)
			pPrivate->callbackfunction((void*)&sq);

		//ALOGD("SNR:  %d", sq.SNR);
		//ALOGD("PCBER:%d", sq.PCBER);
		//ALOGD("RSSI :%f", sq.RSSI);

		for(dec_error=0; dec_error<1152; dec_error++)
		{
			left[dec_error] = (OMX_S16)((pcm[dec_error] & 0xFFFF0000) >> 16);
			right[dec_error] = (OMX_S16)(pcm[dec_error] & 0x0000FFFF);
		}

		if(ret >= 0) {
			dec_error = 0;
		}
		else {
			dec_error = -1;
			for(dec_error=0; dec_error<1152; dec_error++)
			{
				left[dec_error] = 0x00;
				right[dec_error] = 0x00;
			}
		}

		DAB_Mute_Control_byMP2(left, right, dec_error, pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_info.m_sAudioInfo.m_iBitRate, 1152);

		for(dec_error=0; dec_error<1152; dec_error++)
		{
			pcm[dec_error] = (((OMX_S32)left[dec_error] << 16) & 0xFFFF0000) | ((OMX_S32)right[dec_error] & 0x0000FFFF);
		}

		ret = 0;

		outputbuffer->nFilledLen = dec_error*4;

		if(pPrivate->callbackfunction != NULL && sq.PCBER >= 8500)
		{
			memset(outputbuffer->pBuffer, 0, outputbuffer->nFilledLen);
		}

		if(((pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate != pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_eSampleRate) ||
		     ( pPrivate->pAudioPcmMode[uiDecoderID].nChannels!=pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_uiNumberOfChannel)) && pPrivate->outDevID==uiDecoderID)
		{
			ALOGE("SamplingRate[%d]Channel[%d] \n",pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_eSampleRate, pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_uiNumberOfChannel);						
			pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate = pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_eSampleRate;
			pPrivate->pAudioPcmMode[uiDecoderID].nChannels = pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_uiNumberOfChannel;			
			/*Send Port Settings changed call back*/
			(*(pPrivate->callbacks->EventHandler))
			(openmaxStandComp,
			pPrivate->callbackData,
			OMX_EventPortSettingsChanged, /* The command was completed */
			0, 
			2, /* This is the output port index */
			NULL);
		}

		DBUG_MSG("Audio DEC Success. nTimeStamp = %lld", outputbuffer->nTimeStamp);
		pPrivate->pAudioDecPrivate[uiDecoderID].bPrevDecFail = OMX_FALSE;
	}
	else if (ret >= 0)
	{
		omx_aacdec_component_PrivateType* pAACPrivate;
		OMX_U32 *pbuf = (OMX_U32 *)outputbuffer->pBuffer;
		OMX_U32  pcm;
		OMX_U16  lpcm, rpcm;		
		int count;

   		if(((pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate != pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_eSampleRate) ||
		     ( pPrivate->pAudioPcmMode[uiDecoderID].nChannels!=pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_uiNumberOfChannel)) && /*pPrivate->outDevID==uiDecoderID*/OMX_TRUE)
		{	
			ALOGE("SamplingRate[%d]Channel[%d] \n",pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_eSampleRate, pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_uiNumberOfChannel);									
			pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate = pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_eSampleRate;
			pPrivate->pAudioPcmMode[uiDecoderID].nChannels = pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_uiNumberOfChannel;			
			/*Send Port Settings changed call back*/
			(*(pPrivate->callbacks->EventHandler))
			(openmaxStandComp,
			pPrivate->callbackData,
			OMX_EventPortSettingsChanged, /* The command was completed */
			0, 
			2, /* This is the output port index */
			NULL);
		}

		outputbuffer->nFilledLen = pPrivate->pAudioPcmMode[uiDecoderID].nChannels * pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_iDecodedSamples * sizeof(short);
		if(outputbuffer->nFilledLen == 0)
		{
			outputbuffer->nFilledLen = 0;
			inputbuffer->nFilledLen = 0;

			pPrivate->pAudioDecPrivate[uiDecoderID].bAudioInDec = OMX_FALSE;
			
			return;		
		}

		eStereoMode = pPrivate->eStereoMode;

		if (pPrivate->pAudioDecPrivate[uiDecoderID].gsADec.gsADecOutput.m_eChannelType == TCAS_CH_DUAL)
		{
			pAACPrivate = openmaxStandComp->pComponentPrivate;
			eStereoMode = pAACPrivate->eAudioDualMonoSel;

			switch (eStereoMode)
			{
				case TCC_DXBAUDIO_DUALMONO_Left:
					for(count=0; count < pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_iDecodedSamples; count++)
					{
						pcm = (OMX_U32) pbuf[count];
						lpcm = (OMX_U16)(pcm & 0x0000FFFF);
						rpcm = (OMX_U16)((pcm & 0xFFFF0000) >> 16);
						pcm = ((OMX_U32)lpcm << 16) | ((OMX_U32)lpcm);
						pbuf[count] = pcm;
					}
					break;
				case TCC_DXBAUDIO_DUALMONO_Right:
					for(count=0; count < pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_iDecodedSamples; count++)
					{
						pcm = (OMX_U32) pbuf[count];
						lpcm = (OMX_U16)(pcm & 0x0000FFFF);
						rpcm = (OMX_U16)((pcm & 0xFFFF0000) >> 16);
						pcm = ((OMX_U32)rpcm << 16) | ((OMX_U32)rpcm);
						pbuf[count] = pcm;
					}
					break;
				case TCC_DXBAUDIO_DUALMONO_Mix:
					for(count=0; count < pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_iDecodedSamples; count++)
					{
						pcm = (OMX_U32) pbuf[count];
						lpcm = (OMX_U16)(pcm & 0x0000FFFF) >> 1;
						rpcm = (OMX_U16)((pcm & 0xFFFF0000) >> 17);
						lpcm += rpcm;
						pcm = ((OMX_U32)lpcm << 16) | ((OMX_U32)lpcm);
						pbuf[count] = pcm;
					}
					break;
				case TCC_DXBAUDIO_DUALMONO_LeftNRight:
				default:
					/* empty intentionally*/
					break;
			}
		}

	#ifdef	USE_PCM_GAIN
	{
		signed short *pl, *pr;

		#if 0 
		/*--- test code start ---*/
		unsigned int uLength;
		float newgain;
		uLength = property_get("dxb.pcm.gain", value, "");
		if (uLength) {
			uLength = atoi(value);
			if (uLength != 0) {
				newgain = (float) ((float)uLength / 10);
				if (newgain != gPCM_GAIN) {
					LOGE("*** pcm gain: %f -> %f ***", gPCM_GAIN, newgain);
					gPCM_GAIN = newgain;
				}
			}
		}
		/*--- test code end ---*/
		#endif

		for (count=0; count < pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_iDecodedSamples; count++)
		{
			pl = (signed short *)&pbuf[count];
			pr = pl+1;
			*pl = (signed short int)((float)*pl * gPCM_GAIN);
			*pr = (signed short int)((float)*pr * gPCM_GAIN);
		}
	}
	#endif

		if (pPrivate->pAudioDecPrivate[uiDecoderID].iCtype != CONTAINER_TYPE_AUDIO)
		{
			if (pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_WMA)
			{
//				outputbuffer->nTimeStamp = pPrivate->pAudioDecPrivate[uiDecoderID].iStartTS + ((OMX_TICKS)pPrivate->pAudioDecPrivate[uiDecoderID].iSamples * 1000000 + (pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate >> 1)) / pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate;
				;
			}

			// to reduce peak noise, decoded samples which are corresponding to iGuardSamples are set to 0 after seek
			// 'iNumOfSeek > 1' means that the first trial of seek is done actually
			if (pPrivate->pAudioDecPrivate[uiDecoderID].iSamples < pPrivate->iGuardSamples && pPrivate->pAudioDecPrivate[uiDecoderID].iNumOfSeek > 1)
			{
				memset(outputbuffer->pBuffer, 0, outputbuffer->nFilledLen);
			}
			pPrivate->pAudioDecPrivate[uiDecoderID].iSamples += (OMX_TICKS)pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_iDecodedSamples;
		}
		else
		{
			if (pPrivate->pAudioDecPrivate[uiDecoderID].iAdecType == AUDIO_ID_APE)
			{
				outputbuffer->nTimeStamp = pPrivate->pAudioDecPrivate[uiDecoderID].iStartTS + ((OMX_TICKS)pPrivate->pAudioDecPrivate[uiDecoderID].iSamples * 1000000 + (pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate >> 1)) / pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate;
				pPrivate->pAudioDecPrivate[uiDecoderID].iSamples += (OMX_TICKS)pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_iDecodedSamples;
			}
			else
			{
				if (outputbuffer->nTimeStamp == 0 || pPrivate->pAudioDecPrivate[uiDecoderID].iStartTS == outputbuffer->nTimeStamp)
				{
					outputbuffer->nTimeStamp = pPrivate->pAudioDecPrivate[uiDecoderID].iStartTS + (pPrivate->pAudioDecPrivate[uiDecoderID].iSamples * 1000000ll / pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate);
				}
				else
				{
					pPrivate->pAudioDecPrivate[uiDecoderID].iStartTS = outputbuffer->nTimeStamp;
					pPrivate->pAudioDecPrivate[uiDecoderID].iSamples = 0;
				}
				pPrivate->pAudioDecPrivate[uiDecoderID].iSamples += (OMX_TICKS)pPrivate->pAudioDecPrivate[uiDecoderID].cdmx_out.m_iDecodedSamples;
			}
		}


		if (0)
		{
			FILE* fp;
			fp = fopen("/nand/decout.txt", "ab");
			fwrite(outputbuffer->pBuffer, 1, outputbuffer->nFilledLen, fp);
			fclose(fp);
		}
		DBUG_MSG("Audio DEC Success. nTimeStamp = %lld", outputbuffer->nTimeStamp);
		pPrivate->pAudioDecPrivate[uiDecoderID].bPrevDecFail = OMX_FALSE;
	}
	else
	{
		ALOGE( "cdk_audio_dec_run error[%d]: %ld", uiDecoderID, ret );
		pPrivate->pAudioDecPrivate[uiDecoderID].iPrevTS = outputbuffer->nTimeStamp;
		pPrivate->pAudioDecPrivate[uiDecoderID].bPrevDecFail = OMX_TRUE;
	}

#if defined(DUMP_OUTPUT_TO_FILE)
IODumpFileWriteOutputData(outputbuffer->pBuffer, outputbuffer->nFilledLen);
#endif

	pPrivate->isNewBuffer = 1;
	inputbuffer->nFilledLen = 0;

	if(pPrivate->pAudioDecPrivate[uiDecoderID].bAudioPaused == OMX_TRUE)
	{
		outputbuffer->nFilledLen = 0;
	}
#ifdef PCM_INFO_SIZE
	if(outputbuffer->nFilledLen > 0) {
		memcpy(outputbuffer->pBuffer+outputbuffer->nFilledLen, &pPrivate->pAudioPcmMode[uiDecoderID].nSamplingRate, 4);
		outputbuffer->nFilledLen += 4;
		memcpy(outputbuffer->pBuffer+outputbuffer->nFilledLen, &pPrivate->pAudioPcmMode[uiDecoderID].nChannels, 4);
		outputbuffer->nFilledLen += 4;
		*(int *)(outputbuffer->pBuffer+outputbuffer->nFilledLen) = 0;
		outputbuffer->nFilledLen += 4;
	}
#endif

#ifdef SUPPORT_PVR
	if (outputbuffer->nFilledLen > 0)
	{
		SetPVRFlags(pPrivate, uiDecoderID, outputbuffer);
	}
#endif//SUPPORT_PVR

	pPrivate->pAudioDecPrivate[uiDecoderID].bAudioInDec = OMX_FALSE;
}

OMX_ERRORTYPE omx_dxb_audiodec_default_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName) 
{

	OMX_ERRORTYPE err = OMX_ErrorNone;  
	omx_dxb_audio_default_component_PrivateType* omx_audiodec_component_Private;
	omx_base_audio_PortType *inPort,*outPort;
	OMX_U32 i;
	char value[PROPERTY_VALUE_MAX];

	if (!openmaxStandComp->pComponentPrivate) 
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_dxb_audio_default_component_PrivateType));

		if(openmaxStandComp->pComponentPrivate==NULL)  
		{
			return OMX_ErrorInsufficientResources;
		}
	} 
	else 
	{
	    DBUG_MSG("In %s, Error Component %x Already Allocated\n", 
	              __func__, (int)openmaxStandComp->pComponentPrivate);
	}

	omx_audiodec_component_Private = openmaxStandComp->pComponentPrivate;
	omx_audiodec_component_Private->ports = NULL;

	property_get("persist.sys.spdif_setting", value, "0");
	omx_audiodec_component_Private->iSPDIFMode = atoi(value);

	/** we could create our own port structures here
	* fixme maybe the base class could use a "port factory" function pointer?  
	*/
	err = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

	DBUG_MSG("constructor of MP2 decoder component is called\n");

	/* Domain specific section for the ports. */  
	/* first we set the parameter common to both formats */
	/* parameters related to input port which does not depend upon input audio format    */
	/* Allocate Ports and call port constructor. */  

	omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
  	omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 3;

	if (omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_audiodec_component_Private->ports) 
	{
	    omx_audiodec_component_Private->ports = TCC_calloc(omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_PortType *));
	    if (!omx_audiodec_component_Private->ports) 
	    {
	  	    return OMX_ErrorInsufficientResources;
	    }
	    for (i=0; i < omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) 
	    {
		      omx_audiodec_component_Private->ports[i] = TCC_calloc(1, sizeof(omx_base_audio_PortType));
		      if (!omx_audiodec_component_Private->ports[i]) 
		      {
		        	return OMX_ErrorInsufficientResources;
		      }
	    }
	}

	base_audio_port_Constructor(openmaxStandComp, &omx_audiodec_component_Private->ports[0], 0, OMX_TRUE); // input
	base_audio_port_Constructor(openmaxStandComp, &omx_audiodec_component_Private->ports[1], 1, OMX_TRUE); // input
	base_audio_port_Constructor(openmaxStandComp, &omx_audiodec_component_Private->ports[2], 2, OMX_FALSE); // output
		/** parameters related to input port */
	inPort = (omx_base_audio_PortType *) omx_audiodec_component_Private->ports[0];
	inPort->sPortParam.nBufferSize = AUDIO_DXB_IN_BUFFER_SIZE;	
	inPort->sPortParam.nBufferCountMin = 4;
	inPort->sPortParam.nBufferCountActual = 4;

	inPort = (omx_base_audio_PortType *) omx_audiodec_component_Private->ports[1];
	inPort->sPortParam.nBufferSize = AUDIO_DXB_IN_BUFFER_SIZE;	
	inPort->sPortParam.nBufferCountMin = 4;
	inPort->sPortParam.nBufferCountActual = 4;

	outPort = (omx_base_audio_PortType *) omx_audiodec_component_Private->ports[2];
	outPort->sPortParam.nBufferSize = AUDIO_DXB_OUT_BUFFER_SIZE;
	outPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
	outPort->sAudioParam.eEncoding = OMX_AUDIO_CodingPCM;
	outPort->sPortParam.nBufferCountMin = 8;
	outPort->sPortParam.nBufferCountActual = 8;	


	/** general configuration irrespective of any audio formats */
	/**  setting values of other fields of omx_maddec_component_Private structure */
	omx_audiodec_component_Private->BufferMgmtFunction = omx_twoport_filter_component_BufferMgmtFunction;
	omx_audiodec_component_Private->BufferMgmtCallback = omx_audiodec_component_BufferMgmtCallback;
	omx_audiodec_component_Private->messageHandler = omx_audiodec_component_MessageHandler;
	omx_audiodec_component_Private->destructor = omx_audiodec_component_Destructor;
	openmaxStandComp->SetParameter = omx_audiodec_component_SetParameter;
	openmaxStandComp->GetParameter = omx_audiodec_component_GetParameter;
	openmaxStandComp->GetExtensionIndex = omx_audiodec_component_GetExtensionIndex;

	OMX_S16 nDevID;
	omx_audiodec_component_Private->pAudioDecPrivate = TCC_calloc(iTotalHandle, sizeof(OMX_AUDIO_DEC_PRIVATE_DATA));
	for(nDevID=0 ; nDevID<iTotalHandle ; nDevID++) {
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].decode_ready = OMX_FALSE;
	
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_iAudioProcessMode = AUDIO_BROADCAST_MODE; /* decoded pcm mode */
	
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback = &(omx_audiodec_component_Private->callback_func);
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfMalloc   = (void * (*)(unsigned int))malloc;
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfRealloc  = (void * (*)(void*, unsigned int))realloc;
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfFree	  = (void (*)(void*))free;
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfMemcpy   = (void * (*)(void*, const void*, unsigned int))memcpy;
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfMemmove  = (void * (*)(void*, const void*, unsigned int))memmove;
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].cdk_core.m_psCallback->m_pfMemset   = (void (*)(void*, int, unsigned int))memset;
		
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].bAudioStarted = OMX_TRUE;
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].bAudioPaused = OMX_FALSE;
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].bAudioInDec = OMX_FALSE;

#ifdef  SUPPORT_PVR
		omx_audiodec_component_Private->pAudioDecPrivate[nDevID].nPVRFlags = 0;
#endif//SUPPORT_PVR
	}

	omx_audiodec_component_Private->eStereoMode = TCC_DXBAUDIO_DUALMONO_LeftNRight;
	omx_audiodec_component_Private->callbackfunction = NULL;

	return err;
}

OMX_ERRORTYPE OMX_DXB_AudioDec_Default_ComponentInit(OMX_COMPONENTTYPE *openmaxStandComp)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	err = omx_dxb_audiodec_default_component_Constructor(openmaxStandComp,AUDIO_DEC_BASE_NAME);
	return err;
}
