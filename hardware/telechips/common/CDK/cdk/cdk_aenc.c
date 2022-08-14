/****************************************************************************
 *   FileName    : cdk_aenc.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¡°AS IS¡± and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information¡¯s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#ifdef HAVE_ANDROID_OS
#define LOG_TAG	"CDK_AENC"
#include <utils/Log.h>

static int DEBUG_ON	= 0;
#define DPRINTF(msg...)	 ALOGE( ": " msg);
#define DSTATUS(msg...)	 if (DEBUG_ON) { ALOGD( ": " msg);}
#endif

#include "cdk_core.h"
#include "cdk.h"
#include "aenc_vars.h"
#include "cdk_audio.h"

//static cdk_audio_func_t* gspfAEnc;
static cdk_audio_func_t* gspfAEncList[20] = 
{
	TCC_AAC_ENC, 	TCC_MP3_ENC,	0, 				0, 
	0,				0, 				0, 				0,
	0,				0,	TCC_EVRC_ENC,	0,
	0,              0,              0,              0,
	TCC_WAV_ENC,    0,              0,              0
};

#define gspfAEnc			gsAEnc->gspfAEnc
#define gsAEncInput			gsAEnc->gsAEncInput
#define gsAEncOutput		gsAEnc->gsAEncOutput
#define gsAEncInit			gsAEnc->gsAEncInit
#define gpucStreamBuffer	gsAEnc->gpucStreamBuffer
#define gpucPcmBuffer		gsAEnc->gpucPcmBuffer
#define guiRecTime			gsAEnc->guiRecTime
#define guiTimePos			gsAEnc->guiTimePos				
#define gCDSync				gsAEnc->gCDSync

FILE *fd_t;
static gl_cnt = 0;
static unsigned int	counted_filesize=0;

cdk_result_t
cdk_aenc_init( cdk_core_t* pCdk, cmux_info_t *pCmuxInfo, int iAencType, int iCtype, cdk_func_t* callback, AENC_VARS* gsAEnc)
{
	int ret = CDK_ERR_NONE;

#ifdef HAVE_ANDROID_OS
	gspfAEnc= callback;
#else
	gspfAEnc = gspfAEncList[iAencType];
#endif

	if( gspfAEnc )
	{
		pCdk->m_psCallback->m_pfMemset( &gsAEncInput, 0, sizeof(aenc_input_t) );
		pCdk->m_psCallback->m_pfMemset( &gsAEncOutput, 0, sizeof(aenc_output_t) );
		pCdk->m_psCallback->m_pfMemset( &gsAEncInit, 0, sizeof(aenc_init_t) );

		ALOGI("---------------------------------------------------");
		ALOGI("=> pCdk->m_iAudioSamplePerSec (%d) ", pCdk->m_iAudioSamplePerSec);
		ALOGI("=> pCdk->m_iAudioChannels (%d)", pCdk->m_iAudioChannels);
		ALOGI("=> pCdk->m_iAudioBitsPerSample (%d)", pCdk->m_iAudioBitsPerSample);
		ALOGI("=> pCdk->m_iAudioBitRates (%d)", pCdk->m_iAudioBitRates);
		ALOGI("---------------------------------------------------");

		/* audio encoder common */
		gsAEncInput.m_eSampleRate = pCdk->m_iAudioSamplePerSec; // 44100
		gsAEncInput.m_uiNumberOfChannel = pCdk->m_iAudioChannels;	  // 2
		gsAEncInput.m_uiBitsPerSample = pCdk->m_iAudioBitsPerSample;// 16
		if( gsAEncInput.m_uiBitsPerSample == 0 )
		{
			gsAEncInput.m_uiBitsPerSample = 16; //default
		}

		gsAEncInput.m_ePcmInterLeaved = 1;	// 0 or 1
		gsAEncInput.m_iNchannelOrder[CH_LEFT_FRONT] = 1;	//first channel
		gsAEncInput.m_iNchannelOrder[CH_RIGHT_FRONT] = 2;	//second channel
		if( gsAEncInput.m_ePcmInterLeaved == 1 )
		{
			gsAEncInput.m_iNchannelOffsets[0] = 0;
			gsAEncInput.m_iNchannelOffsets[1] = 2;
		}
		
		/* audio encoder output setting */
		gsAEncOutput.m_uiBitRates = pCdk->m_iAudioBitRates;

		/* muxer common extra */
		if( pCmuxInfo != NULL )
		{
			gsAEncInit.m_sCommonInfo.m_pucExtraData = pCmuxInfo->m_sAudioInfo.m_pExtraData;
			gsAEncInit.m_sCommonInfo.m_iExtraDataLen = pCmuxInfo->m_sAudioInfo.m_iExtraDataLen;
		}

		/* audio callback */
		gsAEncInit.m_sCommonInfo.m_psAudioCallback = pCdk->m_psCallback->m_pfMalloc(sizeof(aenc_callback_func_t));
		gsAEncInit.m_sCommonInfo.m_psAudioCallback->m_pfMalloc = pCdk->m_psCallback->m_pfMalloc;
		gsAEncInit.m_sCommonInfo.m_psAudioCallback->m_pfRealloc = pCdk->m_psCallback->m_pfRealloc;
		gsAEncInit.m_sCommonInfo.m_psAudioCallback->m_pfFree = pCdk->m_psCallback->m_pfFree;
		gsAEncInit.m_sCommonInfo.m_psAudioCallback->m_pfMemcpy = pCdk->m_psCallback->m_pfMemcpy;
		gsAEncInit.m_sCommonInfo.m_psAudioCallback->m_pfMemmove = pCdk->m_psCallback->m_pfMemmove;
		gsAEncInit.m_sCommonInfo.m_psAudioCallback->m_pfMemset = pCdk->m_psCallback->m_pfMemset;
		
		gsAEncInit.m_sCommonInfo.m_psAudioEncInput = &gsAEncInput;
		gsAEncInit.m_sCommonInfo.m_psAudioEncOutput = &gsAEncOutput;

		if(iAencType == AUDIO_ID_AAC)
		{
			if( iCtype == 2 ) 
			{
				gsAEncInit.m_unAudioEncodeParams.m_unAAC.m_iAACHeaderType = 2;	//ADTS
			}
			else
			{
				gsAEncInit.m_unAudioEncodeParams.m_unAAC.m_iAACHeaderType = 0; //RAW
			}
			
			gsAEncInit.m_unAudioEncodeParams.m_unAAC.m_iAACTurnOnPns = TCAS_DISABLE;
			gsAEncInit.m_unAudioEncodeParams.m_unAAC.m_iAACTurnOnTns = TCAS_DISABLE;
			gsAEncInit.m_unAudioEncodeParams.m_unAAC.m_iVersionInfo = 1;	//1 = MPEG2, 0 = MPEG4
			
			gsAEncOutput.m_uiNumberOfChannel = gsAEncInput.m_uiNumberOfChannel;
			gsAEncOutput.m_uiSamplesPerChannel = 1024;
			gsAEncInit.m_sCommonInfo.m_iMaxStreamLength = 2048;

		}
		else if(iAencType == AUDIO_ID_MP3)
		{
			//-------------------------------//
			gCDSync.CDsyncEnable = 0; // 1
			gCDSync.CDsyncSilenceCnt = 0;
			gCDSync.CDsyncTime=2000; 	//unit ms
			gCDSync.CDsyncLevel=2000; 	
			gCDSync.CDsyncDetect=0; 	
			//-------------------------------//
			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_pvCdsync = &gCDSync;

			gsAEncInit.m_sCommonInfo.m_iMaxStreamLength = 2048;

			gsAEncOutput.m_uiNumberOfChannel = 2;
			gsAEncOutput.m_uiSamplesPerChannel = 1152;			

			// If you wnat to use USER_SETTING_MODE, refer to the below sample codes.	
//#define	USER_SETTING_MODE
		#ifdef	USER_SETTING_MODE 
			gsAEncOutput.m_uiBitRates = 0;
			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_uiVersion = 1;			// '1' MPEG1, '0'-MPEG2
			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_uiSamplingFreqIndex = 0;	// '0' - 44.1 or 22.05 ,  '1' - 48 or 24 ,  '2' - 32 or 16
			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_uiStereoMode = 1;			// '0' - stereo '1' - Joint Stereo
			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_iChannels = 2;			// '1' - mono '2' - stereo
			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_uiBitrateIndex =13;		// refer to MP3 Spec.
			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_iBitrate = 256000;		// refer to MP3 Spec. , Unit : bps/1000.
			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_iSamplingFreq = 44100;	// sampling rate		


			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_uiClipEnable = 0;			// '1' - enable '0' - disable
			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_iCutoffFreq = 40;			// adjust this value from test. ( Do not exceed 80 )
		#else

			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_uiClipEnable = 0;			// '1' - enable '0' - disable
			gsAEncInit.m_unAudioEncodeParams.m_unMP3ENC.m_iCutoffFreq = 40;			// adjust this value from test. ( Do not exceed 80 )
		#endif

		}
		else if(iAencType == AUDIO_ID_EVRC)
		{

			gsAEncInit.m_unAudioEncodeParams.m_unEVRC.m_iUseNoiseSuppression = 0; //noise suppression off
			gsAEncOutput.m_eSampleRate = 8000;
			gsAEncOutput.m_uiNumberOfChannel = 1;
			gsAEncOutput.m_uiSamplesPerChannel = 160;
			gsAEncInit.m_sCommonInfo.m_iMaxStreamLength = 24;

		}
#if 0  //2013-03-04 :  Jason - Disable wma encoder code
		else if(iAencType == AUDIO_ID_WMA)
		{
			//----------------------------------//
			gCDSync.CDsyncEnable = 0;			// 1
			gCDSync.CDsyncSilenceCnt = 0;
			gCDSync.CDsyncTime=2000; 			//unit ms
			gCDSync.CDsyncLevel=2000; 	
			gCDSync.CDsyncDetect=0; 	
			//----------------------------------//


			gsAEncInit.m_sCommonInfo.m_iMaxStreamLength = 12000;
			gsAEncInit.m_unAudioEncodeParams.m_unWMA.m_pvCdsync = &gCDSync;
			gsAEncInit.m_unAudioEncodeParams.m_unWMA.m_uiHeaderPerPacket = 0;

			gsAEncOutput.m_uiNumberOfChannel = 2;
			gsAEncOutput.m_uiSamplesPerChannel = 2048;

#if 0	
			pCdk->m_pfEncodedOutFile = cdk_fopen(pCdk->m_pszEncodedOutFileName, "wb");


			//
			// Initialize ASF region 
			//
			{
				unsigned char  ch = 0;
				cdk_fwrite(&ch,1,512,pCdk->m_pfEncodedOutFile);
			}

			pCdk->m_pszEncodedOutFileName = NULL ;
#endif
		}
#endif	//#if 0  //2013-03-04	
		else if( iAencType == AUDIO_ID_WAV )
		{
			gsAEncInit.m_unAudioEncodeParams.m_unWAV.m_iWaveFormat = pCdk->m_iAudioSubCodec;
			gsAEncOutput.m_uiNumberOfChannel = gsAEncInput.m_uiNumberOfChannel;
			gsAEncOutput.m_uiSamplesPerChannel = 4096;
			gsAEncInit.m_sCommonInfo.m_iMaxStreamLength = ( 4096 * 2 * sizeof(short) );
		}
		
		gpucStreamBuffer = pCdk->m_psCallback->m_pfMalloc(gsAEncInit.m_sCommonInfo.m_iMaxStreamLength);
		if(gpucStreamBuffer == NULL)
		{
			DPRINTF( "[CDK_CORE] cdk_aenc_init gpucStreamBuffer allocation fail\n");
			return -1;
		}
		
		gsAEncOutput.m_pcStream = (signed char *)gpucStreamBuffer;
		
		gsAEncInit.m_sCommonInfo.m_iPcmLength = gsAEncOutput.m_uiNumberOfChannel * gsAEncOutput.m_uiSamplesPerChannel * sizeof(short);
		gpucPcmBuffer = (unsigned char*)pCdk->m_psCallback->m_pfMalloc(gsAEncInit.m_sCommonInfo.m_iPcmLength);
		if(gpucPcmBuffer == NULL)
		{
			DPRINTF( "[CDK_CORE] cdk_aenc_init gpucPcmBuffer allocation fail\n");
			return -1;
		}
		
		gsAEncInput.m_pvChannel[0] = (void *)gpucPcmBuffer;
		
		ALOGI("@@@@@ init Start");
		ret = gspfAEnc( AUDIO_INIT, &pCdk->m_iAudioHandle, &gsAEncInit, NULL);
		if( ret < 0 )
		{
			DPRINTF( "[CDK_CORE:Err%d] gspfAEnc AUDIO_INIT failed \n", ret );
			gspfAEnc( AUDIO_CLOSE, &pCdk->m_iAudioHandle, NULL, NULL );
			return ret;
		}
		ALOGI("@@@@@ init End ");

		if( (gsAEncOutput.m_uiNumberOfChannel==0) || (gsAEncOutput.m_eSampleRate==0) || (gsAEncOutput.m_uiSamplesPerChannel==0) )
		{
			DPRINTF( "[CDK_CORE] Error invalid paramters [ch:%d, samplerate: %d, bitPersample%d, samplePerchannel%d]\n", 
				gsAEncOutput.m_uiNumberOfChannel, gsAEncOutput.m_eSampleRate, gsAEncInput.m_uiBitsPerSample, gsAEncOutput.m_uiSamplesPerChannel);					

			return -1;
		}
		else
		{
			DPRINTF( "[CDK_CORE] [channel:%4d, samplerate :%4d] \n", gsAEncOutput.m_uiNumberOfChannel, gsAEncOutput.m_eSampleRate);
		}
		
		gsAEncInput.m_uiSamplesPerChannel = gsAEncOutput.m_uiSamplesPerChannel;
		gsAEncInput.m_uiNumberOfChannel	  = gsAEncOutput.m_uiNumberOfChannel;
		gsAEncInput.m_eSampleRate		  = gsAEncOutput.m_eSampleRate;		
	}
	else
	{
		DPRINTF( "[CDK_CORE] audio decoder not found (iAencType: %d) \n", iAencType );
		return -1;
	}

#ifdef __FILE_INPUT__
	fd_t = fopen("/sdcard/input.pcm", "rb");
#endif
	ALOGI("### Enc Init Done, ret(%d)", ret);

	return ret;
}

cdk_result_t
cdk_aenc_encode( cdk_core_t* pCdk, cmux_info_t *pCmuxInfo, cmux_input_t *pCmuxInput, int iAencType, AENC_VARS* gsAEnc )
{
	unsigned int n_read, read_num;
	int ret = CDK_ERR_NONE;
	
#ifdef __FILE_INPUT__
	ALOGI("@@@@@ gl_cnt (%d) ", gl_cnt);
	if (gl_cnt < 5000)
	{
		gl_cnt++;
		read_num = gsAEncInput.m_uiNumberOfChannel * gsAEncInput.m_uiSamplesPerChannel * sizeof(short);

		ALOGI(" QQQ read_num(%d) = NumberOfChannel(%d) * SamplesPerChannel(%d) * short(%d)",
				read_num, gsAEncInput.m_uiNumberOfChannel,gsAEncInput.m_uiSamplesPerChannel, sizeof(short));

		n_read = fread(gpucPcmBuffer, 1, read_num, fd_t);  // input
	}
	else
	{
		cdk_aenc_close( pCdk );
		return -1;
	}

	if(n_read != read_num)
	{
		if( n_read == 0 )
			return -1;
		if( n_read < read_num)
		{
			pCdk->m_psCallback->m_pfMemset(gpucPcmBuffer + n_read, 0, read_num - n_read);
		}
	}

	//	Audio Encoding mode
	gsAEncInput.m_pvChannel[0] = gpucPcmBuffer;		// file input
#else
	gsAEncInput.m_pvChannel[0] = pCmuxInput->m_pData;  // pcm input
#endif
	//ALOGI("@@@@@ encode stsrt");
	ret = gspfAEnc( AUDIO_ENCODE, &pCdk->m_iAudioHandle, &gsAEncInput, &gsAEncOutput);

	if( ret < 0 )
	{
		DPRINTF( "[CDK_CORE] cdk_audio_dec_run error\n" );
		return ret;
	}

	//ALOGI("++++ ret(%d) gsAEncOutput.m_iStreamLength(%d)", ret, gsAEncOutput.m_iStreamLength);
	// encoder component do not refer this variables
	//pCmuxInput->m_pData = (unsigned char*)gsAEncOutput.m_pcStream;
	//pCmuxInput->m_iDataSize = gsAEncOutput.m_iStreamLength;
	counted_filesize += gsAEncOutput.m_iStreamLength; // ssize

	return 0;
}

cdk_result_t
cdk_aenc_close( cdk_core_t* pCdk, AENC_VARS* gsAEnc )
{
	cdk_result_t ret = CDK_ERR_NONE;
#ifdef __FILE_INPUT__
	fclose(fd_t);
#endif
	
	if(gspfAEnc)
	{
#if 0		//2013-03-04 :  Jason - Disable wma encoder code
		if(pCdk->m_iAudioCodec == AUDIO_ID_WMA)
		{
			/* In  case : finish WMA Encoding */
			// gsAEncInput.m_pvExtraInfo = pCdk->m_psCallback->m_pfFtell(pCdk->m_pfEncodedOutFile);
			gsAEncInput.m_pvExtraInfo = counted_filesize;

			/**************************************** 
			* Get the ASF Heder information			*
			****************************************/ 
			#define	WMAENC_GET_HEADER			0x30
			gspfAEnc(WMAENC_GET_HEADER, &pCdk->m_iAudioHandle, &gsAEncInput,&gsAEncOutput); 

			// updates the ASF data field for closing WMA ENCODING 	
			//-----------------------------------------------------------//					
		#if 0
			// Find the ASF region .
			pCdk->m_psCallback->m_pfFseek(pCdk->m_pfEncodedOutFile, 0, SEEK_SET);
			// Updates the ASF data field 
			pCdk->m_psCallback->m_pfFwrite(gsAEncOutput.m_pvExtraInfo, 1,512, pCdk->m_pfEncodedOutFile);
			pCdk->m_psCallback->m_pfFseek(pCdk->m_pfEncodedOutFile, 0, SEEK_END);
			//-----------------------------------------------------------//					
		#else
			// Updates the ASF data field

			pCdk->m_pOutWav = gsAEncOutput.m_pvExtraInfo;    // buffer
			//pCdk->m_pSize = 512; // ssize


		#endif
			/**************************************** 
			* Close the TC_WMA Encoder				*
			****************************************/ 
			ret = gspfAEnc( AUDIO_CLOSE, &pCdk->m_iAudioHandle, NULL, NULL );
			ALOGI("@@@@@ aenc close");

		}
		else
#endif	//#if 0		//2013-03-04 		
			ret = gspfAEnc( AUDIO_CLOSE, &pCdk->m_iAudioHandle, NULL, NULL );
	}

	if( gpucPcmBuffer )
	{
		pCdk->m_psCallback->m_pfFree(gpucPcmBuffer);
		gpucPcmBuffer = NULL;
	}

	if( gpucStreamBuffer )
	{
		pCdk->m_psCallback->m_pfFree(gpucStreamBuffer);
		gpucStreamBuffer = NULL;
	}

	if( gsAEncInit.m_sCommonInfo.m_psAudioCallback )
	{
		pCdk->m_psCallback->m_pfFree(gsAEncInit.m_sCommonInfo.m_psAudioCallback);
	}

	return ret;
}
