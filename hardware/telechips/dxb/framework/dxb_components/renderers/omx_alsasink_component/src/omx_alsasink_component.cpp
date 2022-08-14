/**
  OpenMAX ALSA sink component. This component is an audio sink that uses ALSA library.

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

#ifdef HAVE_ANDROID_OS
#define LOG_TAG "OMX_AudioSink"
#include <utils/Log.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <omxcore.h>
#include <omx_base_audio_port.h>
#include <omx_base_component.h>
#include <omx_base_sink.h>
#include <omx_alsasink_component.h>
#include <tccaudio.h>
#include <tcc_video_common.h>
#include <OMX_TCC_Index.h>
#include "tcc_dxb_interface_demux.h"
#include <system/audio.h>

#ifdef HAVE_ANDROID_OS
#undef MONO
#include <media/AudioTrack.h>
#include <cutils/properties.h>
#endif

#include <hardware_legacy/SPDIFHardwareInterface.h>

//#define	CHANGE_AUDIO_SYSTEM_SAMPLING_RATE // ReverseJ - B060961
#include "SimpleBuffer.h"

typedef long long			(*pfnDemuxGetSTC)(int itype, long long lpts, unsigned int index, int log);
static pfnDemuxGetSTC gfnDemuxGetSTCValue = NULL;
static unsigned int guiSTCDelay = 0; //usec

static uint32_t afLatency = 0;


/** Maximum Number of AlsaSink Instance*/
#define MAX_COMPONENT_ALSASINK 1
#define MAX_AUDIOTRACK_FRAMECOUNT   16384//1024 * 50/*default 8192 */
#define AUDIOTRACK_FRAMECOUNT_UNITSIZE	(MAX_AUDIOTRACK_FRAMECOUNT/4)
#define AUDIOTRACK_MIN_BUFFER_RATIO	3

#define DISABLE_AUDIO_OUTPUT 0
#define ENABLE_AUDIO_OUTPUT 1

/* Default Values of Time gaps for check pattern #1 to sync PTS & STC */
#define DEFAULT_SYNC_PATTERN1_PTSSTC_WAITTIME_GAP			32		// 32mS
#define DEFAULT_SYNC_PATTERN1_PTSSTC_DROPTIME_GAP			(-32)	// -32mS
#define DEFAULT_SYNC_PATTERN1_PTSSTC_JUMPTIME_GAP			5000	// 5000mS

/** Number of AlsaSink Instance*/
static OMX_U32 noAlsasinkInstance = 0;

static OMX_BOOL balsasinkbypass = OMX_FALSE;
static int balsasinkStopRequested = 0; //bit0:Stop Requested, bit1:Start Requested

//#define FILE_OUT_ALSA
#ifdef FILE_OUT_ALSA
/*for checking whether PCM data is valid or not*/
static FILE *pOutA;
static int gAlsaFileSize = 0;
#endif

using namespace android;

int omx_alsasink_spdif_open(int sampling_rate, int channels);
int omx_alsasink_spdif_close();
int omx_alsasink_spdif_write(unsigned char *data, int data_size);
int omx_alsasink_spdif_start();

#define     SPDIF_BUFFER_SIZE       MAX_AUDIOTRACK_FRAMECOUNT//(8*1024)//(6144)//(8*1024)    
#define     SIMPLE_FIFO_FOR_SPDIF
#ifdef      SIMPLE_FIFO_FOR_SPDIF
static tsem_t* 	gpSPDIFBufSema = NULL;
static SRBUFFER gSPDIFFifo;
static char		*gpSPDIFBuf = NULL;

sp<AudioTrack> mAudioTrack[TOTAL_AUDIO_TRACK] = NULL;
SPDIFHardwareInterface *mSpdifHardware = NULL;

class SPDIFBufferThread : public Thread
{
    public:
        SPDIFBufferThread(OMX_COMPONENTTYPE *h_component);
        virtual ~SPDIFBufferThread();

    private:
        OMX_COMPONENTTYPE *m_h_component;
        virtual bool threadLoop();
};

SPDIFBufferThread::SPDIFBufferThread(OMX_COMPONENTTYPE *h_component) 
{
    m_h_component = h_component;
}

SPDIFBufferThread::~SPDIFBufferThread() 
{
}

bool SPDIFBufferThread::threadLoop() 
{
    int read_data, uiSize;	
    unsigned char *pucData = NULL;//[SPDIF_BUFFER_SIZE];
    uiSize = SPDIF_BUFFER_SIZE;
	pucData = (unsigned char *)malloc(uiSize);
    while(!exitPending()) 
    {
       	tsem_down(gpSPDIFBufSema);
	    read_data = QGetDataEX(&gSPDIFFifo, (char *)pucData, uiSize);
    	tsem_up(gpSPDIFBufSema);    	
    	if(read_data == 0)
	    {
		    //LOGD("%s SIZE[%d] :: [ERROR]There are no data to read !!!!\n", __func__, uiSize);
    		usleep(5000);
	    }
        else
        {
            omx_alsasink_spdif_write(pucData, uiSize);
            //ALOGD("%s SIZE[%d] :: Reading OK !!!!\n", __func__, uiSize);
        }
    }
	if(pucData)
		free(pucData);
    return false;
}
static SPDIFBufferThread *gpSPDIFBufferThread = NULL;
#endif

static OMX_ERRORTYPE omx_alsasink_component_DoStateSet(
	OMX_COMPONENTTYPE *h_component, 
	OMX_U32 ui_dest_state);

int omx_alsasink_spdif_open(int sampling_rate, int channels)
{
	if (mSpdifHardware != NULL)
		omx_alsasink_spdif_close();

	mSpdifHardware = createSPDIFHardware();
	if (mSpdifHardware != NULL)
	{
		mSpdifHardware->setChannelCount(channels);
		mSpdifHardware->setFrameSize(channels*2);
		mSpdifHardware->setSampleRate(sampling_rate);
	}
	DEBUG(DEFAULT_MESSAGES, "spdif_open");
	return 0;
}

int omx_alsasink_spdif_close()
{
	if (mSpdifHardware != NULL)
	{
		mSpdifHardware->stop();
		delete mSpdifHardware;
		mSpdifHardware = NULL;
	}
	DEBUG(DEFAULT_MESSAGES, "spdif_close");
	return 0;
}

int omx_alsasink_spdif_write(unsigned char *data, int data_size)
{
	int ret = -1;
	if (mSpdifHardware != NULL)
	{
		ret = mSpdifHardware->write(data, data_size);
	}
	return ret;
}

int omx_alsasink_spdif_start()
{
	if (mSpdifHardware != NULL)
	{
		mSpdifHardware->start();
	}
	DEBUG(DEFAULT_MESSAGES, "spdif_start");
	return 0;
}

int omx_alsasink_spdif_stop()
{
	if (mSpdifHardware != NULL)
	{
		mSpdifHardware->stop();
	}
	DEBUG(DEFAULT_MESSAGES, "spdif_stop");
	return 0;
}

OMX_ERRORTYPE omx_alsasink_no_clockcomp_component_Constructor (OMX_COMPONENTTYPE * openmaxStandComp, OMX_STRING cComponentName)
{
	int       i;
	int       err;
	int       omxErr;
	omx_base_audio_PortType *pPort;
	omx_alsasink_component_PrivateType *omx_alsasink_component_Private;
	char     *pcm_name;

	if (!openmaxStandComp->pComponentPrivate)
	{
		openmaxStandComp->pComponentPrivate = TCC_calloc (1, sizeof (omx_alsasink_component_PrivateType));
		if (openmaxStandComp->pComponentPrivate == NULL)
		{
			return OMX_ErrorInsufficientResources;
		}
	}

#ifdef FILE_OUT_ALSA
	pOutA = fopen ("/sdcard/dvbt_audio.pcm", "wb");
	if (pOutA == NULL)
		DEBUG (DEB_LEV_ERR, "Cannot make alsa dump File\n");
	else
		DEBUG (DEB_LEV_SIMPLE_SEQ, "Making alsa dump File success\n");
	gAlsaFileSize = 0;
#endif

	omx_alsasink_component_Private = (omx_alsasink_component_PrivateType *) openmaxStandComp->pComponentPrivate;

	omx_alsasink_component_Private->ports = NULL;

	omxErr = omx_base_sink_Constructor (openmaxStandComp, cComponentName);
	if (omxErr != OMX_ErrorNone)
	{
		return OMX_ErrorInsufficientResources;
	}

	omx_alsasink_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
	omx_alsasink_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 1;

  /** Allocate Ports and call port constructor. */
	if ((omx_alsasink_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts)
		&& !omx_alsasink_component_Private->ports)
	{
		omx_alsasink_component_Private->ports =
			(omx_base_PortType **)TCC_calloc (omx_alsasink_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts,
						sizeof (omx_base_PortType *));
		if (!omx_alsasink_component_Private->ports)
		{
			return OMX_ErrorInsufficientResources;
		}
		omx_alsasink_component_Private->ports[OMX_BASE_SINK_INPUTPORT_INDEX] =
			(omx_base_PortType *) TCC_calloc (1, sizeof (omx_base_audio_PortType));
		if (!omx_alsasink_component_Private->ports[0])
		{
			return OMX_ErrorInsufficientResources;
		}
		base_audio_port_Constructor (openmaxStandComp, &omx_alsasink_component_Private->ports[0], 0, OMX_TRUE);
	}

	pPort = (omx_base_audio_PortType *) omx_alsasink_component_Private->ports[OMX_BASE_SINK_INPUTPORT_INDEX];

	// set the pPort params, now that the ports exist  
  /** Domain specific section for the ports. */
	pPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
	/*Input pPort buffer size is equal to the size of the output buffer of the previous component */
	pPort->sPortParam.nBufferSize = AUDIO_DXB_OUT_BUFFER_SIZE;
	pPort->sPortParam.nBufferCountMin = 8;
	pPort->sPortParam.nBufferCountActual = 8;

	/* Initializing the function pointers */
	omx_alsasink_component_Private->BufferMgmtCallback = omx_alsasink_component_BufferMgmtCallback;
	omx_alsasink_component_Private->messageHandler = omx_alsasink_component_MessageHandler;
	omx_alsasink_component_Private->DoStateSet = omx_alsasink_component_DoStateSet;
	omx_alsasink_component_Private->destructor = omx_alsasink_component_Destructor;

	setHeader (&pPort->sAudioParam, sizeof (OMX_AUDIO_PARAM_PORTFORMATTYPE));
	pPort->sAudioParam.nPortIndex = 0;
	pPort->sAudioParam.nIndex = 0;
	pPort->sAudioParam.eEncoding = OMX_AUDIO_CodingPCM;

	/* OMX_AUDIO_PARAM_PCMMODETYPE */
	for(i=0; i<TOTAL_AUDIO_TRACK; i++)
	{
		setHeader (&omx_alsasink_component_Private->sPCMModeParam[i], sizeof (OMX_AUDIO_PARAM_PCMMODETYPE));
		omx_alsasink_component_Private->sPCMModeParam[i].nPortIndex = 0;
		omx_alsasink_component_Private->sPCMModeParam[i].nChannels = 2;
		omx_alsasink_component_Private->sPCMModeParam[i].eNumData = OMX_NumericalDataSigned;
		omx_alsasink_component_Private->sPCMModeParam[i].eEndian = OMX_EndianLittle;
		omx_alsasink_component_Private->sPCMModeParam[i].bInterleaved = OMX_TRUE;
		omx_alsasink_component_Private->sPCMModeParam[i].nBitPerSample = 16;
		omx_alsasink_component_Private->sPCMModeParam[i].nSamplingRate = 48000;
		omx_alsasink_component_Private->sPCMModeParam[i].ePCMMode = OMX_AUDIO_PCMModeLinear;
		omx_alsasink_component_Private->sPCMModeParam[i].eChannelMapping[0] = OMX_AUDIO_ChannelNone;
		omx_alsasink_component_Private->enable_output_audio_ch_index[i] = DISABLE_AUDIO_OUTPUT;
		omx_alsasink_component_Private->setVolumeValue[i] = 100;
	}

	noAlsasinkInstance++;
	if (noAlsasinkInstance > MAX_COMPONENT_ALSASINK)
	{
		return OMX_ErrorInsufficientResources;
	}

	/*6144 is alsa buffer size
	*/
	omx_alsasink_component_Private->mSPDIF_nChannels = 0;
	omx_alsasink_component_Private->mSPDIF_nSamplingRate = 0;

#ifdef SIMPLE_FIFO_FOR_SPDIF
	memset(&gSPDIFFifo, 0, sizeof(SRBUFFER));
	gpSPDIFBuf = (char *)malloc(1024*1024);
	if(gpSPDIFBuf == NULL)
		return OMX_ErrorHardware;

	QInitBuffer(&gSPDIFFifo, (1024*1024), gpSPDIFBuf);
	gpSPDIFBufSema = (tsem_t*)TCC_calloc(1,sizeof(tsem_t));
	if(gpSPDIFBufSema == NULL )
	{
		free(gpSPDIFBuf);
		return OMX_ErrorHardware;
	}        	
	tsem_init(gpSPDIFBufSema, 1);
	gpSPDIFBufferThread = new SPDIFBufferThread(openmaxStandComp);        
#endif //SIMPLE_FIFO_FOR_SPDIF

#ifdef CHANGE_AUDIO_SYSTEM_SAMPLING_RATE
	AudioSystem::getOutputSamplingRate(
		&omx_alsasink_component_Private->iAudioSystemSamplingRate, 
		AUDIO_STREAM_MUSIC);

	ALOGD("[ALSASINK] Current output sampling rate : %d(Hz)\n", 
		omx_alsasink_component_Private->iAudioSystemSamplingRate);

	if(omx_alsasink_component_Private->iAudioSystemSamplingRate != 48000)
	{
		omx_alsasink_component_Private->iCurrentSamplingRate = 48000;

		ALOGD("[ALSASINK] Set output sampling rate %d(Hz) to %d(Hz)\n", 
			omx_alsasink_component_Private->iAudioSystemSamplingRate, 
			omx_alsasink_component_Private->iCurrentSamplingRate);
		AudioSystem::setForceSamplingRate(omx_alsasink_component_Private->iCurrentSamplingRate);
	}

	AudioTrack::getMinFrameCount(&omx_alsasink_component_Private->mAudioTrack_MinFrameCount, AUDIO_STREAM_MUSIC, 48000);
	ALOGD("MinFrameCount[%d]", omx_alsasink_component_Private->mAudioTrack_MinFrameCount);

	omx_alsasink_component_Private->alsa_configure_request = FALSE;
#endif

	openmaxStandComp->SetParameter = omx_alsasink_component_SetParameter;
	openmaxStandComp->GetParameter = omx_alsasink_component_GetParameter;

	openmaxStandComp->GetConfig = omx_alsasink_component_GetConfig;
	openmaxStandComp->SetConfig = omx_alsasink_component_SetConfig;

	/* Write in the default parameters */
	omx_alsasink_component_Private->AudioPCMConfigured = 0;
	omx_alsasink_component_Private->eState = OMX_TIME_ClockStateStopped;
	omx_alsasink_component_Private->xScale = 1 << 16;
//	omx_alsasink_component_Private->hw_params = NULL;
//	omx_alsasink_component_Private->playback_handle = NULL;	
	omx_alsasink_component_Private->bAudioOutStartSyncWithVideo = OMX_FALSE;
	omx_alsasink_component_Private->iPatternToCheckPTSnSTC = 0;
	omx_alsasink_component_Private->iAudioSyncWaitTimeGap = DEFAULT_SYNC_PATTERN1_PTSSTC_WAITTIME_GAP;
	omx_alsasink_component_Private->iAudioSyncDropTimeGap = DEFAULT_SYNC_PATTERN1_PTSSTC_DROPTIME_GAP;
	omx_alsasink_component_Private->iAudioSyncJumpTimeGap = DEFAULT_SYNC_PATTERN1_PTSSTC_JUMPTIME_GAP;
	
	// iAudioMuteConfig
	// 0 : use Mute function of AutioTrack
	// 1 : disable pcm output
	omx_alsasink_component_Private->iAudioMuteConfig = 0;
	//omx_alsasink_component_Private->iAudioMute_PCMDisable = 0;

	gfnDemuxGetSTCValue = NULL;
	balsasinkbypass = OMX_FALSE;	
	balsasinkStopRequested = 0;	
	guiSTCDelay = 20000;
	
#ifdef  SUPPORT_PVR
	omx_alsasink_component_Private->nPVRFlags = 0;
#endif//SUPPORT_PVR

	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_alsasink_no_clockcomp_component_Init (OMX_COMPONENTTYPE * openmaxStandComp)
{
	return (omx_alsasink_no_clockcomp_component_Constructor (openmaxStandComp, ALSA_SINK_NAME));
}

/** The Destructor 
 */
OMX_ERRORTYPE omx_alsasink_component_Destructor (OMX_COMPONENTTYPE * openmaxStandComp)
{
	omx_alsasink_component_PrivateType *omx_alsasink_component_Private = (omx_alsasink_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	OMX_U32   i;
 
#ifdef      SIMPLE_FIFO_FOR_SPDIF
    if(gpSPDIFBufferThread)
    {
        gpSPDIFBufferThread->requestExit();
        usleep(5000);
    }

    if( gpSPDIFBuf )
		free(gpSPDIFBuf);
	memset(&gSPDIFFifo, 0, sizeof(SRBUFFER));
	if(gpSPDIFBufSema)
	{
		tsem_deinit(gpSPDIFBufSema);
		TCC_free(gpSPDIFBufSema);
		gpSPDIFBufSema = NULL;
	}	    
#endif
    omx_alsasink_spdif_close();

	#ifdef CHANGE_AUDIO_SYSTEM_SAMPLING_RATE
	{
		if(omx_alsasink_component_Private->iAudioSystemSamplingRate 
			!= omx_alsasink_component_Private->iCurrentSamplingRate)
		{
			ALOGD("[ALSASINK] Set output sampling rate %d(Hz) to %d(Hz)\n", 
				omx_alsasink_component_Private->iCurrentSamplingRate, 
				omx_alsasink_component_Private->iAudioSystemSamplingRate);
			AudioSystem::setForceSamplingRate(omx_alsasink_component_Private->iAudioSystemSamplingRate);
		}
	}
	#endif // CHANGE_AUDIO_SYSTEM_SAMPLING_RATE

	for(i=0; i<TOTAL_AUDIO_TRACK; i++)
	{
		if(mAudioTrack[i] != NULL){
			if (!mAudioTrack[i]->stopped())
			{
				mAudioTrack[i]->stop ();
			}
			DEBUG (DEFAULT_MESSAGES,"Delete audioTrack.\n");
			mAudioTrack[i].clear();
		}
 	}

	DEBUG (DEFAULT_MESSAGES,"Free ports & comp.\n");
	/* frees port/s */
	if (omx_alsasink_component_Private->ports)
	{
		for (i = 0; i < (omx_alsasink_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts +
						 omx_alsasink_component_Private->sPortTypesParam[OMX_PortDomainOther].nPorts); i++)
		{
			if (omx_alsasink_component_Private->ports[i])
				omx_alsasink_component_Private->ports[i]->PortDestructor (omx_alsasink_component_Private->ports[i]);
		}
		TCC_free (omx_alsasink_component_Private->ports);
		omx_alsasink_component_Private->ports = NULL;
	}

	noAlsasinkInstance--;

	return omx_base_sink_Destructor (openmaxStandComp);

}

#ifdef  SUPPORT_PVR
static int CheckPVR(omx_alsasink_component_PrivateType *omx_private, OMX_U32 ulInputBufferFlags)
{
	OMX_U32 iPVRState, iBufferState;

	iPVRState = (omx_private->nPVRFlags & PVR_FLAG_START) ? 1 : 0;
	iBufferState = (ulInputBufferFlags & OMX_BUFFERFLAG_FILE_PLAY) ? 1 : 0;
	if (iPVRState != iBufferState)
	{
		return 1; // skip
	}

	iPVRState = (omx_private->nPVRFlags & PVR_FLAG_TRICK) ? 1 : 0;
	iBufferState = (ulInputBufferFlags & OMX_BUFFERFLAG_TRICK_PLAY) ? 1 : 0;
	if (iPVRState != iBufferState)
	{
		return 1; // skip
	}

	if (ulInputBufferFlags & OMX_BUFFERFLAG_TRICK_PLAY)
	{
		return 1; // skip
	}

	if (omx_private->nPVRFlags & PVR_FLAG_PAUSE)
	{
		return 2; // pause
	}

	return 0; // OK
}
#endif//SUPPORT_PVR

/** 
 * This function plays the input buffer. When fully consumed it returns.
 */
void omx_alsasink_component_BufferMgmtCallback (OMX_COMPONENTTYPE * openmaxStandComp,
												OMX_BUFFERHEADERTYPE * inputbuffer)
{
    OMX_U32   frameSize;
    OMX_U32   uiBufferLen = inputbuffer->nFilledLen;
    OMX_S32   written;
    OMX_BOOL  allDataSent;
    uint32_t  position = 0;
    OMX_U32   latency = 0;
    OMX_U64   uiInputBufferTime = 0;
    OMX_S32   nSpdifMode = 0;
    OMX_U32   nSamplingRate = 0;
    OMX_U32   nChannels = 0;
	ssize_t BufferSize = 0;
	char audio_type[PROPERTY_VALUE_MAX];
	char hdmi_mode[PROPERTY_VALUE_MAX];
	OMX_S32   ret;

    omx_alsasink_component_PrivateType *omx_alsasink_component_Private =
        (omx_alsasink_component_PrivateType *) openmaxStandComp->pComponentPrivate;

#ifdef  SUPPORT_PVR
	ret = CheckPVR(omx_alsasink_component_Private, inputbuffer->nFlags);
	switch (ret)
	{
	case 1: // skip
		inputbuffer->nFilledLen = 0;
		return;
	case 2: // pause
		usleep(1000);
		return;
	}
#endif//SUPPORT_PVR

    if(balsasinkStopRequested & 0x1) {
        if(balsasinkStopRequested == 0x1)
        {   //No Start Operation
            inputbuffer->nFilledLen = 0;
            return;
        }
        if(balsasinkStopRequested & 0x2) //if Start happens
        {
            balsasinkStopRequested &= ~0x1; //Clear Stop flag
        }    
    }
   
    if (omx_alsasink_component_Private->enable_output_audio_ch_index[inputbuffer->nDecodeID] == DISABLE_AUDIO_OUTPUT) {
        inputbuffer->nFilledLen = 0;
        return;
    }

#ifdef PCM_INFO_SIZE
    if (uiBufferLen <= PCM_INFO_SIZE) {
        inputbuffer->nFilledLen = 0;
        return;
    }

    uiBufferLen -= 4;
    memcpy(&nSpdifMode, inputbuffer->pBuffer + uiBufferLen, 4);
    uiBufferLen -= 4;
    memcpy(&nChannels, inputbuffer->pBuffer + uiBufferLen, 4);
    uiBufferLen -= 4;
    memcpy(&nSamplingRate, inputbuffer->pBuffer + uiBufferLen, 4);
#endif

	property_get("persist.sys.spdif_setting", hdmi_mode, "0");
	property_get("tcc.hdmi.audio_type", audio_type, "0");

	if(!strcmp(hdmi_mode, "4")) //HDMI PassThrough
	{
		if(!strcmp(audio_type, "2")) //AC3
		{
			BufferSize = 8 * 1024;
			latency = (BufferSize * 1000 * 2 ) / (nSamplingRate * 2 * 2) + 400;
		}

  /* Broadcasting stream not exist dts audio type 
  *
  *		else if(!strcmp(audio_type, "3")) //DTS
  *		{
  *			if(nSamplingRate > 190000)
  *				BufferSize = 16 * 1024 * 2 * nChannels;
  *			else
  *				BufferSize = 8 * 1024 * 2 * nChannels;
  *
  *			latency = (BufferSize * 1000) / (nSamplingRate * 8 * 2) + 300;
  *		}
  *
  *
  *		else //DDP
  *		{
  *			BufferSize = 256 * 1024;
  *			latency = (BufferSize * 1000 * 2) / (nSamplingRate * 8 * 2);
  *		}
  *	}
  *	else if(!strcmp(hdmi_mode, "2")) //SPDIF PassThrough
  *	{
  *		BufferSize = 8 * 1024;
  *		latency = (BufferSize * 1000 * 2) / (nSamplingRate * 2 * 2);
  *	}
  *	else // Stereo
  *	{	
  *		BufferSize = 8 * 1024;
  		latency = (BufferSize * 1000) / (nSamplingRate * 8 * 2);
  */}	

    if(inputbuffer->nTimeStamp)
        uiInputBufferTime = inputbuffer->nTimeStamp + guiSTCDelay;

    if(gfnDemuxGetSTCValue && uiInputBufferTime && !balsasinkbypass) {
        long long llSTC, llPTS;
		/*In This case We don't use clock components.
		* At this moments We need to compare between STC(Sytem Time Clock) and PTS (Present Time Stamp)
		* STC : ms, PTS : us
		*/
        llPTS = uiInputBufferTime/1000; //557 is Android Audio system delay
        if (nSpdifMode < 2) {
	    if (mAudioTrack[inputbuffer->nDecodeID] != NULL) {
                latency = (1000 * mAudioTrack[inputbuffer->nDecodeID]->frameCount() / mAudioTrack[inputbuffer->nDecodeID]->getSampleRate());
                //latency = mAudioTrack[inputbuffer->nDecodeID]->latency_ex();
                //latency = mAudioTrack[inputbuffer->nDecodeID]->latency_ex() + mAudioTrack[inputbuffer->nDecodeID]->latency() - (1000 * mAudioTrack[inputbuffer->nDecodeID]->frameCount() / mAudioTrack[inputbuffer->nDecodeID]->getSampleRate());
	/*
                ALOGE("[latency] = %d:%d:%d:%d:%d:%d",  latency,
														audioTrack[inputbuffer->nDecodeID]->latency_ex(),
														audioTrack[inputbuffer->nDecodeID]->latency(),
														audioTrack[inputbuffer->nDecodeID]->frameCount(),
														audioTrack[inputbuffer->nDecodeID]->getSampleRate(),
														(1000 * audioTrack[inputbuffer->nDecodeID]->frameCount() / audioTrack[inputbuffer->nDecodeID]->getSampleRate()));
	*/
            }
        }
        llPTS-=latency;
        /* itype 0:Audio, 1:Video Other :: Get STC Value		
        * ret :: -1:drop frame, 0:display frame, 1:wait frame
        */		
        llSTC = gfnDemuxGetSTCValue(DxB_SYNC_AUDIO, llPTS, inputbuffer->nDecodeID, 1); //
        if (llSTC == DxB_SYNC_WAIT || llSTC == DxB_SYNC_LONG_WAIT) {				
            usleep(5000); //Wait
            return;
        } else if (llSTC == DxB_SYNC_DROP) {
            inputbuffer->nFilledLen = 0;
            return;
        }
    }

    if(	(*(omx_alsasink_component_Private->callbacks->EventHandler)) (openmaxStandComp,
            omx_alsasink_component_Private->callbackData,
            OMX_EventDynamicResourcesAvailable,
            uiBufferLen, (OMX_U32)(uiInputBufferTime/1000), inputbuffer->pBuffer) == OMX_ErrorNone)
    {
        inputbuffer->nFilledLen = 0;
        return;
    }

    if(nSamplingRate != 0 && omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nSamplingRate != nSamplingRate)
    {
        omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nSamplingRate = nSamplingRate;
        omx_alsasink_component_Private->alsa_configure_request = TRUE;
    }
    if(nChannels != 0 && omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nChannels != nChannels)
    {
        omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nChannels = nChannels;
        omx_alsasink_component_Private->alsa_configure_request = TRUE;
    }

    inputbuffer->nFilledLen = 0;

    if (nSpdifMode >= 2)
	{
#ifdef      SIMPLE_FIFO_FOR_SPDIF
        tsem_down(gpSPDIFBufSema);
        written = QPutData(&gSPDIFFifo,(char *)inputbuffer->pBuffer,(long)uiBufferLen);
        tsem_up(gpSPDIFBufSema);
        if(written == 0) {
            ALOGD("%s SIZE[%d] :: [ERROR]There are no Space in Fifo !!!!\n", __func__, uiBufferLen);
        } else {
            //ALOGD("%s SIZE[%d]Valid[%d] :: Writing OK !!!!\n", __func__, written, QHowManyData(&gSPDIFFifo));
        }
#endif
        if( (omx_alsasink_component_Private->mSPDIF_nSamplingRate != omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nSamplingRate) ||
                (omx_alsasink_component_Private->mSPDIF_nChannels != omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nChannels) ) {
            omx_alsasink_component_Private->mSPDIF_nSamplingRate = omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nSamplingRate;
            omx_alsasink_component_Private->mSPDIF_nChannels = omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nChannels;
            omx_alsasink_spdif_close();
            omx_alsasink_spdif_open(omx_alsasink_component_Private->mSPDIF_nSamplingRate, omx_alsasink_component_Private->mSPDIF_nChannels);
            omx_alsasink_spdif_start();
#ifdef      SIMPLE_FIFO_FOR_SPDIF
            if(gpSPDIFBufferThread)
                gpSPDIFBufferThread->run("SPDIF Buffering Thread", PRIORITY_URGENT_DISPLAY);
#endif
        }
        return;
    }
    else if ( omx_alsasink_component_Private->mSPDIF_nSamplingRate && omx_alsasink_component_Private->mSPDIF_nChannels)
    {
        omx_alsasink_component_Private->mSPDIF_nSamplingRate = 0;
        omx_alsasink_component_Private->mSPDIF_nChannels = 0;
        omx_alsasink_spdif_close();
        omx_alsasink_component_Private->alsa_configure_request = TRUE;
    }

    if(mAudioTrack[inputbuffer->nDecodeID] != NULL) {
        mAudioTrack[inputbuffer->nDecodeID]->getPosition(&position);
        //max : 4294967295
        //max-48000*60s*10
        if( (position>4266167295) || omx_alsasink_component_Private->alsa_configure_request) {
            float left,right;
            int iChannels;

            if (!mAudioTrack[inputbuffer->nDecodeID]->stopped())
            {
                mAudioTrack[inputbuffer->nDecodeID]->stop ();
            }
            DEBUG (DEFAULT_MESSAGES,"Delete audioTrack.\n");
            mAudioTrack[inputbuffer->nDecodeID].clear();
		
#ifdef CHANGE_AUDIO_SYSTEM_SAMPLING_RATE
            {
                if(omx_alsasink_component_Private->iCurrentSamplingRate != omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nSamplingRate)
                {
                    ALOGD("[ALSASINK] Set output sampling rate %d(Hz) to %d(Hz)\n", 
                        omx_alsasink_component_Private->iCurrentSamplingRate, 
                        omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nSamplingRate);

                    AudioSystem::setForceSamplingRate(omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nSamplingRate);
                    omx_alsasink_component_Private->iCurrentSamplingRate = omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nSamplingRate;
                }
            }
#endif //CHANGE_AUDIO_SYSTEM_SAMPLING_RATE

            AudioTrack::getMinFrameCount(&omx_alsasink_component_Private->mAudioTrack_MinFrameCount, AUDIO_STREAM_MUSIC, omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nSamplingRate);
            ALOGD("MinFrameCount[%d]", omx_alsasink_component_Private->mAudioTrack_MinFrameCount);

            omx_alsasink_component_Private->alsa_configure_request = FALSE;
        }
    }

    if (mAudioTrack[inputbuffer->nDecodeID] == NULL || mAudioTrack[inputbuffer->nDecodeID]->initCheck () != NO_ERROR) {
        int err = UNKNOWN_ERROR;
        int iChannels = AUDIO_CHANNEL_OUT_MONO;

        if(omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nChannels == 2)
            iChannels = AUDIO_CHANNEL_OUT_STEREO;

        if(mAudioTrack[inputbuffer->nDecodeID] == NULL) {
            mAudioTrack[inputbuffer->nDecodeID] = new AudioTrack(AUDIO_STREAM_MUSIC,
                                        omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nSamplingRate,
                                        AUDIO_FORMAT_PCM_16_BIT,
                                        iChannels,
                                        MAX_AUDIOTRACK_FRAMECOUNT,
                                        AUDIO_OUTPUT_FLAG_NONE,
                                        NULL,
                                        omx_alsasink_component_Private);
            err = mAudioTrack[inputbuffer->nDecodeID]->initCheck ();

            if (AudioSystem::getOutputLatency(&afLatency, AUDIO_STREAM_MUSIC) != NO_ERROR) {
                ALOGE("Could not get audioflinger");
            } else {
                ALOGI("get latency of AudioFlinger : %d", afLatency);

                (*(omx_alsasink_component_Private->callbacks->EventHandler))
                        (openmaxStandComp,
                        omx_alsasink_component_Private->callbackData,
                        OMX_EventVendorSetSTCDelay,
                        afLatency, 0, 0);
            }
        }
        if (err != NO_ERROR) {
            ALOGE ("AudioTrack initCheck error");
            return;
        }

        omx_alsasink_component_Private->mAudioTrack_MinFrameSize_ms = (int)((AUDIOTRACK_FRAMECOUNT_UNITSIZE*1000)/mAudioTrack[inputbuffer->nDecodeID]->getSampleRate())*AUDIOTRACK_MIN_BUFFER_RATIO;
        omx_alsasink_component_Private->mAudioTrack_byteSizePer1ms = (int)(mAudioTrack[inputbuffer->nDecodeID]->getSampleRate()*mAudioTrack[inputbuffer->nDecodeID]->frameSize())/1000;
    }
    else if (mAudioTrack[inputbuffer->nDecodeID]->stopped()) {
        mAudioTrack[inputbuffer->nDecodeID]->start();
    }

    /* Feed it to ALSA */
    frameSize =
        (omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nChannels *
        omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nBitPerSample) >> 3;
    DEBUG (DEB_LEV_FULL_SEQ, "Framesize is %u chl=%d bufSize=%d\n", (int) frameSize,
            (int) omx_alsasink_component_Private->sPCMModeParam[inputbuffer->nDecodeID].nChannels, (int) uiBufferLen);

    if (uiBufferLen< frameSize) {
        DEBUG (DEB_LEV_ERR, "Ouch!! In %s input buffer filled len(%d) less than frame size(%d)\n", __func__,
                (int) uiBufferLen, (int) frameSize);
        return;
    }

    allDataSent = OMX_FALSE;
#ifdef FILE_OUT_ALSA
    if (pOutA) {
        fwrite (inputbuffer->pBuffer, 1, uiBufferLen, pOutA);
        gAlsaFileSize += uiBufferLen;
        DEBUG (DEB_LEV_ERR, "ALSA Dump File size %d\n", gAlsaFileSize);
        if (gAlsaFileSize > 4 * 1024 * 1024) {
            fclose (pOutA);
            sync ();
            DEBUG (DEB_LEV_ERR, "ALSA Dump File Close success\n");
            pOutA = NULL;
            gAlsaFileSize = 0;
        }
    }
#endif

    if (balsasinkStopRequested & 0x2) {
        if(omx_alsasink_component_Private->eState == OMX_TIME_ClockStateWaitingForStartTime)
        {
            omx_alsasink_component_Private->eState = OMX_TIME_ClockStateRunning;
            (*(omx_alsasink_component_Private->callbacks->EventHandler))
                    (openmaxStandComp,
                    omx_alsasink_component_Private->callbackData,
                    OMX_EventVendorBufferingStart,
                    0, 0, 0);
        }

        written = mAudioTrack[inputbuffer->nDecodeID]->write(inputbuffer->pBuffer, uiBufferLen);
        if( written == uiBufferLen) {
            if (mAudioTrack[inputbuffer->nDecodeID]->stopped()) {
                mAudioTrack[inputbuffer->nDecodeID]->start();
            }
        } else {
            ALOGE("written %d :: bufferlen %d :: track stopped %d", written, uiBufferLen, mAudioTrack[inputbuffer->nDecodeID]->stopped());
            if(mAudioTrack[inputbuffer->nDecodeID]->stopped()) {
                if(written > 0) {
                    mAudioTrack[inputbuffer->nDecodeID]->start();
                    written = mAudioTrack[inputbuffer->nDecodeID]->write(inputbuffer->pBuffer+written, uiBufferLen-written);
                }
            } else {
                mAudioTrack[inputbuffer->nDecodeID]->stop();
                mAudioTrack[inputbuffer->nDecodeID]->reload();
            }
        }
    }
}

OMX_ERRORTYPE omx_alsasink_component_MessageHandler (OMX_COMPONENTTYPE * openmaxStandComp,
													 internalRequestMessageType * message)
{
	omx_alsasink_component_PrivateType *omx_alsasink_component_Private =
		(omx_alsasink_component_PrivateType *) openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE err;
	OMX_STATETYPE eCurrentState = omx_alsasink_component_Private->state;
	DEBUG (DEB_LEV_SIMPLE_SEQ, "In %s\n", __func__);

	if (message->messageType == OMX_CommandStateSet)
	{
		if ((message->messageParam == OMX_StateExecuting) && (omx_alsasink_component_Private->state == OMX_StateIdle))
		{
			;
		}
	}
	/** Execute the base message handling */
	err = omx_base_component_MessageHandler (openmaxStandComp, message);
	if (message->messageType == OMX_CommandStateSet)
	{
		if ((message->messageParam == OMX_StateIdle)
			&& (eCurrentState == OMX_StateExecuting || eCurrentState == OMX_StatePause))
		{
			;
		}
	}

	return err;
}

static OMX_ERRORTYPE omx_alsasink_component_DoStateSet(
	OMX_COMPONENTTYPE *h_component, 
	OMX_U32 ui_dest_state)
{
	OMX_ERRORTYPE e_error = OMX_ErrorNone;

	OMX_STATETYPE e_state;

	omx_alsasink_component_PrivateType *p_priv = (omx_alsasink_component_PrivateType *)h_component->pComponentPrivate;

//	DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);

	e_state = p_priv->state;

	e_error = omx_base_component_DoStateSet(h_component, ui_dest_state);
	if(e_error != OMX_ErrorNone)
	{
		DEBUG(DEB_LEV_ERR, "[ERROR] %s is failed\n", __func__);
		return e_error;
	}

	if((ui_dest_state == OMX_StateIdle) && (e_state == OMX_StateExecuting))
	{
		int i;
		for(i=0; i<TOTAL_AUDIO_TRACK; i++)
		{
			if(mAudioTrack[i] != NULL){
				if (!mAudioTrack[i]->stopped())
				{
					mAudioTrack[i]->stop();
				}
			}
		}
	}
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_alsasink_component_SetConfig (OMX_IN OMX_HANDLETYPE hComponent,
												OMX_IN OMX_INDEXTYPE nIndex, OMX_IN OMX_PTR pComponentConfigStructure)
{
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *) hComponent;

	omx_alsasink_component_PrivateType *omx_alsasink_component_Private = (omx_alsasink_component_PrivateType *)openmaxStandComp->pComponentPrivate;

	if (pComponentConfigStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}

	switch (nIndex)
	{	
	default:	// delegate to superclass
		return omx_base_component_SetConfig (hComponent, nIndex, pComponentConfigStructure);
	}
	return OMX_ErrorNone;

}

OMX_ERRORTYPE omx_alsasink_component_GetConfig (OMX_IN OMX_HANDLETYPE hComponent,
												OMX_IN OMX_INDEXTYPE nIndex,
												OMX_INOUT OMX_PTR pComponentConfigStructure)
{

	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *) hComponent;
	omx_alsasink_component_PrivateType *omx_alsasink_component_Private = (omx_alsasink_component_PrivateType *)openmaxStandComp->pComponentPrivate;

	if (pComponentConfigStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}

	switch (nIndex)
	{	
	default:	// delegate to superclass
		return omx_base_component_GetConfig (hComponent, nIndex, pComponentConfigStructure);
	}
	return OMX_ErrorNone;

}

OMX_ERRORTYPE omx_alsasink_component_SetParameter (OMX_IN OMX_HANDLETYPE hComponent,
												   OMX_IN OMX_INDEXTYPE nParamIndex,
												   OMX_IN OMX_PTR ComponentParameterStructure)
{
	OMX_S32   *piArg;
	int       omxErr = OMX_ErrorNone;
	OMX_AUDIO_PARAM_PORTFORMATTYPE *pAudioPortFormat;
	OMX_OTHER_PARAM_PORTFORMATTYPE *pOtherPortFormat;
	OMX_AUDIO_PARAM_MP3TYPE *pAudioMp3;
	OMX_U32   portIndex;


	/* Check which structure we are being fed and make control its header */
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *) hComponent;
	omx_alsasink_component_PrivateType *omx_alsasink_component_Private = (omx_alsasink_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType *pPort =
		(omx_base_audio_PortType *) omx_alsasink_component_Private->ports[OMX_BASE_SINK_INPUTPORT_INDEX];

	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}

	DEBUG (DEB_LEV_SIMPLE_SEQ, "   Setting parameter %i\n", nParamIndex);

  /** Each time we are (re)configuring the hw_params thing
  * we need to reinitialize it, otherwise previous changes will not take effect.
  * e.g.: changing a previously configured sampling rate does not have
  * any effect if we are not calling this each time.
  */

	switch (nParamIndex)
	{
	case OMX_IndexParamAudioPortFormat:
		pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE *) ComponentParameterStructure;
		portIndex = pAudioPortFormat->nPortIndex;
		/*Check Structure Header and verify component state */
		omxErr =
			omx_base_component_ParameterSanityCheck (hComponent, portIndex, pAudioPortFormat,
													 sizeof (OMX_AUDIO_PARAM_PORTFORMATTYPE));
		if (omxErr != OMX_ErrorNone)
		{
			DEBUG (DEB_LEV_ERR, "In %s Parameter Check Error=%x\n", __func__, omxErr);
			break;
		}
		if (portIndex < 1)
		{
			memcpy (&pPort->sAudioParam, pAudioPortFormat, sizeof (OMX_AUDIO_PARAM_PORTFORMATTYPE));
		}
		else
		{
			return OMX_ErrorBadPortIndex;
		}
		break;	

	case OMX_IndexParamAudioPcm:
		{
			unsigned int rate;
			int iChannels;
			OMX_AUDIO_PARAM_PCMMODETYPE *sPCMModeParam = (OMX_AUDIO_PARAM_PCMMODETYPE *) ComponentParameterStructure;

			portIndex = sPCMModeParam->nPortIndex;
			omx_alsasink_component_Private->AudioPCMConfigured = 1;
			if (sPCMModeParam->nPortIndex != omx_alsasink_component_Private->sPCMModeParam[0].nPortIndex)
			{
				DEBUG (DEB_LEV_ERR, "Error setting input pPort index\n");
				omxErr = OMX_ErrorBadParameter;
				break;
			}
			if(sPCMModeParam->nSamplingRate == 0)
			{
				ALOGE("Sampling Rate Change error to %d, set to 48000 forcibly", sPCMModeParam->nSamplingRate);
				omxErr = OMX_ErrorBadParameter;
				break;
			}

			if(sPCMModeParam->nChannels == 0)
			{
				ALOGE("Channel Number Change error to %d, set to 2 forcibly", sPCMModeParam->nChannels);
				omxErr = OMX_ErrorBadParameter;
				break;
			}
			memcpy (&omx_alsasink_component_Private->sPCMModeParam, ComponentParameterStructure, sizeof (OMX_AUDIO_PARAM_PCMMODETYPE));
			omx_alsasink_component_Private->alsa_configure_request = TRUE;
			ALOGD("Changing SamplingRate[%d]Channel[%d]\n", sPCMModeParam->nSamplingRate, sPCMModeParam->nChannels);
		}	
		break;
	case OMX_IndexParamAudioMp3:
		pAudioMp3 = (OMX_AUDIO_PARAM_MP3TYPE *) ComponentParameterStructure;
		/*Check Structure Header and verify component state */
		omxErr =
			omx_base_component_ParameterSanityCheck (hComponent, pAudioMp3->nPortIndex, pAudioMp3,
													 sizeof (OMX_AUDIO_PARAM_MP3TYPE));
		if (omxErr != OMX_ErrorNone)
		{
			DEBUG (DEB_LEV_ERR, "In %s Parameter Check Error=%x\n", __func__, omxErr);
			break;
		}
		break;
	case OMX_IndexVendorParamAlsaSinkSetVolume:
		{
			int *pVolumeValue = (int*)ComponentParameterStructure;
			int volumeID = pVolumeValue[0];
			float fVolume = pVolumeValue[1]/100.0;
			if(mAudioTrack[volumeID] != NULL){
				mAudioTrack[volumeID]->setVolume(fVolume, fVolume);
				ALOGD("Current Volume L:%f R:%f\n", fVolume, fVolume);
			/*View Other informations
			*/
				ALOGE("AudioTrack latency:%d\n", mAudioTrack[volumeID]->latency());
			}
			else
			{
				omx_alsasink_component_Private->setVolumeValue[volumeID] = pVolumeValue[1];
			}
		}
		break;
	case OMX_IndexVendorParamDxBGetSTCFunction:
		gfnDemuxGetSTCValue = (pfnDemuxGetSTC) ComponentParameterStructure;
		break;	
	case OMX_IndexVendorParamAlsaSinkMute:
		{
			int *piArg = (int*)ComponentParameterStructure;
			bool bMute;

			bMute = piArg[0];
			if(omx_alsasink_component_Private->iAudioMuteConfig == 0) {
				if(mAudioTrack[0] != NULL)
					AudioSystem::setStreamMute(AUDIO_STREAM_MUSIC, bMute);
			}
			else {
				//omx_alsasink_component_Private->iAudioMute_PCMDisable = bMute;
				omx_alsasink_component_Private->enable_output_audio_ch_index[0] = bMute? DISABLE_AUDIO_OUTPUT:ENABLE_AUDIO_OUTPUT;
				omx_alsasink_component_Private->enable_output_audio_ch_index[1] = bMute? DISABLE_AUDIO_OUTPUT:ENABLE_AUDIO_OUTPUT;
			}
			ALOGD("Current Mute State:%d\n", bMute);
		}
		break;

	case OMX_IndexVendorParamAlsaSinkMuteConfig:
		{
			int *piArg = (int*)ComponentParameterStructure;
			omx_alsasink_component_Private->iAudioMuteConfig = piArg[0];
			ALOGD("iAudioMuteConfig:%d\n", omx_alsasink_component_Private->iAudioMuteConfig);
		}
		break;

	case OMX_IndexVendorParamAlsaSinkAudioStartNotify:
		{
			if(omx_alsasink_component_Private->eState == OMX_TIME_ClockStateStopped) {
				omx_alsasink_component_Private->eState = OMX_TIME_ClockStateWaitingForStartTime;
			}
		}
		break;

	case OMX_IndexVendorParamSetSinkByPass:
		{
			OMX_U32 ulDevId;
			OMX_S32  *piArg;
			piArg = (OMX_S32 *) ComponentParameterStructure;
			ulDevId = (OMX_U32) piArg[0];
			balsasinkbypass = (OMX_BOOL) piArg[1];
		}
		break;

   	case OMX_IndexVendorParamStopRequest:
		{
			OMX_U32 ulDevId;
			OMX_S32  *piArg;
			piArg = (OMX_S32 *) ComponentParameterStructure;
			ulDevId = (OMX_U32) piArg[0];
			if((ulDevId == 0)||(ulDevId == 1))
			{
				omx_alsasink_component_Private->eState = OMX_TIME_ClockStateStopped;
					
				//we only support main device
				if((OMX_BOOL) piArg[1] == TRUE) //if stop
				{
					balsasinkStopRequested = 0x1; //Stop Requested, Clear Start flag
					if(mAudioTrack[ulDevId] != NULL) {
						if (!mAudioTrack[ulDevId]->stopped()) {
							mAudioTrack[ulDevId]->stop();
							mAudioTrack[ulDevId]->flush();
							ALOGD("%s:audioTrack Stopped",__func__);
						}
					}
				}
				else 
				{
					balsasinkStopRequested |= 0x2; //Start Requested, Stop first
					if(mAudioTrack[ulDevId] != NULL)
						AudioSystem::setStreamMute(AUDIO_STREAM_MUSIC, FALSE);
				}		
				ALOGD("balsasinkStopRequested[%d][%d]",balsasinkStopRequested,  (OMX_BOOL) piArg[1]);
			}
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

				if ((omx_alsasink_component_Private->nPVRFlags & PVR_FLAG_START) == 0)
				{
					omx_alsasink_component_Private->nPVRFlags = PVR_FLAG_START | PVR_FLAG_CHANGED;
				}
			}
			break;

		case OMX_IndexVendorParamPlayStopRequest:
			{
				OMX_S32 ulPvrId;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];

				if (omx_alsasink_component_Private->nPVRFlags & PVR_FLAG_START)
				{
					omx_alsasink_component_Private->nPVRFlags = PVR_FLAG_CHANGED;
				}
			}
			break;

		case OMX_IndexVendorParamSeekToRequest:
			{
				OMX_S32 ulPvrId, nSeekTime;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];
				nSeekTime = (OMX_S32) piArg[1];

				if (omx_alsasink_component_Private->nPVRFlags & PVR_FLAG_START)
				{
					if (nSeekTime < 0)
					{
						if (omx_alsasink_component_Private->nPVRFlags & PVR_FLAG_TRICK)
						{
							omx_alsasink_component_Private->nPVRFlags &= ~PVR_FLAG_TRICK;
							omx_alsasink_component_Private->nPVRFlags |= PVR_FLAG_CHANGED;
						}
					}
					else
					{
						if ((omx_alsasink_component_Private->nPVRFlags & PVR_FLAG_TRICK) == 0)
						{
							omx_alsasink_component_Private->nPVRFlags |= (PVR_FLAG_TRICK | PVR_FLAG_CHANGED);
						}
					}
				}
			}
			break;

		case OMX_IndexVendorParamPlaySpeed:
			{
				OMX_S32 ulPvrId, nPlaySpeed;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];
				nPlaySpeed = (OMX_S32) piArg[1];

				if (omx_alsasink_component_Private->nPVRFlags & PVR_FLAG_START)
				{
					if (nPlaySpeed == 1)
					{
						if (omx_alsasink_component_Private->nPVRFlags & PVR_FLAG_TRICK)
						{
							omx_alsasink_component_Private->nPVRFlags &= ~PVR_FLAG_TRICK;
							omx_alsasink_component_Private->nPVRFlags |= PVR_FLAG_CHANGED;
						}
					}
					else
					{
						if ((omx_alsasink_component_Private->nPVRFlags & PVR_FLAG_TRICK) == 0)
						{
							omx_alsasink_component_Private->nPVRFlags |= (PVR_FLAG_TRICK | PVR_FLAG_CHANGED);
						}
					}
				}
			}
			break;

		case OMX_IndexVendorParamPlayPause:
			{
				OMX_S32 ulPvrId, nPlayPause;

				piArg = (OMX_S32 *) ComponentParameterStructure;
				ulPvrId = (OMX_S32) piArg[0];
				nPlayPause = (OMX_S32) piArg[1];

				if (omx_alsasink_component_Private->nPVRFlags & PVR_FLAG_START)
				{
					if (nPlayPause == 0)
					{
						omx_alsasink_component_Private->nPVRFlags |= PVR_FLAG_PAUSE;
					}
					else
					{
						omx_alsasink_component_Private->nPVRFlags &= ~PVR_FLAG_PAUSE;
					}
				}
			}
			break;
#endif//SUPPORT_PVR

	case OMX_IndexVendorNotifyStartTimeSyncWithVideo:
		{
			if (omx_alsasink_component_Private->bAudioOutStartSyncWithVideo == OMX_TRUE)
			{
				if(mAudioTrack[0] != NULL)
					AudioSystem::setStreamMute(AUDIO_STREAM_MUSIC, FALSE);
			}
		}
		break;

	case OMX_IndexVendorParamSetEnableAudioStartSyncWithVideo:
		{
			OMX_S32  *piArg;
			piArg = (OMX_S32 *) ComponentParameterStructure;
			omx_alsasink_component_Private->bAudioOutStartSyncWithVideo = (OMX_BOOL) piArg[0];
			ALOGD("Set Enable Audio Start Sync With Video [%d]", omx_alsasink_component_Private->bAudioOutStartSyncWithVideo);
		}
		break;

	case OMX_IndexVendorParamSetPatternToCheckPTSnSTC:
		{
			OMX_S32  *piArg;
			piArg = (OMX_S32 *) ComponentParameterStructure;
			omx_alsasink_component_Private->iPatternToCheckPTSnSTC = (int)piArg[0];
			omx_alsasink_component_Private->iAudioSyncWaitTimeGap = (int)piArg[1];	//mS
			omx_alsasink_component_Private->iAudioSyncDropTimeGap = (int)piArg[2];	//mS
			omx_alsasink_component_Private->iAudioSyncJumpTimeGap = (int)piArg[3];	//mS

			/* Sycn Pattern #1 */
			if ( omx_alsasink_component_Private->iPatternToCheckPTSnSTC == 1 )
			{
				if ( omx_alsasink_component_Private->iAudioSyncWaitTimeGap < DEFAULT_SYNC_PATTERN1_PTSSTC_WAITTIME_GAP )
					omx_alsasink_component_Private->iAudioSyncWaitTimeGap = DEFAULT_SYNC_PATTERN1_PTSSTC_WAITTIME_GAP;
				if ( omx_alsasink_component_Private->iAudioSyncDropTimeGap > DEFAULT_SYNC_PATTERN1_PTSSTC_DROPTIME_GAP )
					omx_alsasink_component_Private->iAudioSyncDropTimeGap = DEFAULT_SYNC_PATTERN1_PTSSTC_DROPTIME_GAP;
				if ( omx_alsasink_component_Private->iAudioSyncJumpTimeGap < DEFAULT_SYNC_PATTERN1_PTSSTC_JUMPTIME_GAP )
					omx_alsasink_component_Private->iAudioSyncJumpTimeGap = DEFAULT_SYNC_PATTERN1_PTSSTC_JUMPTIME_GAP;
			}
			
			ALOGD("Set Pattern #[%d] to check PTS&STC - WaitTimeGap[%dmS] DropTimeGap[%dmS] JumpTimeGap[%dmS] ",
				omx_alsasink_component_Private->iPatternToCheckPTSnSTC,
				omx_alsasink_component_Private->iAudioSyncWaitTimeGap,
				omx_alsasink_component_Private->iAudioSyncDropTimeGap,
				omx_alsasink_component_Private->iAudioSyncJumpTimeGap);
		}
		break;

	case OMX_IndexVendorParamAudioOutputCh:
		{
			OMX_S32 devID, isEnableAudioOutput;
			OMX_S32  *piArg;
			piArg = (OMX_S32 *)ComponentParameterStructure;
			devID = (OMX_S32)piArg[0];
			isEnableAudioOutput = (OMX_S32)piArg[1];

			ALOGD("[ALSASINK] Change Audio Output to Ch[%d:%d] \n", devID, isEnableAudioOutput);
			omx_alsasink_component_Private->enable_output_audio_ch_index[devID] = isEnableAudioOutput? ENABLE_AUDIO_OUTPUT:DISABLE_AUDIO_OUTPUT;
			//TCCALSA_SoundReset(hComponent);
		}
		break;

	case OMX_IndexVendorSetSTCDelay:
		#if 1
 		{
			guiSTCDelay = (unsigned int)ComponentParameterStructure;
			ALOGD("STC Delay %d (us)", guiSTCDelay);
			if(guiSTCDelay > 1000000)
				guiSTCDelay = 300000;
		}
		#endif
            	break;		

	case OMX_IndexConfigResetRequestForALSASink:
		omx_alsasink_component_Private->alsa_configure_request = TRUE;
		break;

	default:	/*Call the base component function */
		return omx_base_component_SetParameter (hComponent, nParamIndex, ComponentParameterStructure);
	}
	return (OMX_ERRORTYPE) omxErr;
}

OMX_ERRORTYPE omx_alsasink_component_GetParameter (OMX_IN OMX_HANDLETYPE hComponent,
												   OMX_IN OMX_INDEXTYPE nParamIndex,
												   OMX_INOUT OMX_PTR ComponentParameterStructure)
{
	OMX_AUDIO_PARAM_PORTFORMATTYPE *pAudioPortFormat;
	OMX_OTHER_PARAM_PORTFORMATTYPE *pOtherPortFormat;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_AUDIO_CONFIG_INFOTYPE *info;

	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *) hComponent;
	omx_alsasink_component_PrivateType *omx_alsasink_component_Private = (omx_alsasink_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType *pPort = (omx_base_audio_PortType *) omx_alsasink_component_Private->ports[OMX_BASE_SINK_INPUTPORT_INDEX];
	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}
	DEBUG (DEB_LEV_SIMPLE_SEQ, "   Getting parameter %i\n", nParamIndex);
	/* Check which structure we are being fed and fill its header */
	switch (nParamIndex)
	{
	case OMX_IndexParamAudioInit:
		if ((err = checkHeader (ComponentParameterStructure, sizeof (OMX_PORT_PARAM_TYPE))) != OMX_ErrorNone)
		{
			break;
		}
		memcpy (ComponentParameterStructure, &omx_alsasink_component_Private->sPortTypesParam[OMX_PortDomainAudio],
				sizeof (OMX_PORT_PARAM_TYPE));
		break;	
	case OMX_IndexParamAudioPortFormat:
		pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE *) ComponentParameterStructure;
		if ((err = checkHeader (ComponentParameterStructure, sizeof (OMX_AUDIO_PARAM_PORTFORMATTYPE))) != OMX_ErrorNone)
		{
			break;
		}
		if (pAudioPortFormat->nPortIndex < 1)
		{
			memcpy (pAudioPortFormat, &pPort->sAudioParam, sizeof (OMX_AUDIO_PARAM_PORTFORMATTYPE));
		}
		else
		{
			return OMX_ErrorBadPortIndex;
		}
		break;
	case OMX_IndexParamAudioPcm:
		if (((OMX_AUDIO_PARAM_PCMMODETYPE *) ComponentParameterStructure)->nPortIndex !=
			omx_alsasink_component_Private->sPCMModeParam[0].nPortIndex)
		{
//			return OMX_ErrorBadParameter;
		}
		if ((err = checkHeader (ComponentParameterStructure, sizeof (OMX_AUDIO_PARAM_PCMMODETYPE))) != OMX_ErrorNone)
		{
			break;
		}
		memcpy (ComponentParameterStructure, &omx_alsasink_component_Private->sPCMModeParam[0],
				sizeof (OMX_AUDIO_PARAM_PCMMODETYPE));
		break;
	default:	/*Call the base component function */
		return omx_base_component_GetParameter (hComponent, nParamIndex, ComponentParameterStructure);
	}
	return err;
}
 
