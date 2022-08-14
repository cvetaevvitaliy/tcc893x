/*
 * Copyright 2008 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *     http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */

#define LOG_TAG "TCC_OMXCore"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <utils/Log.h>

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <cutils/properties.h>

#define MAX_ROLES 20
#define MAX_TABLE_SIZE 30

typedef struct _ComponentTable {
    OMX_STRING name;
    OMX_U16 nRoles;
    OMX_STRING pRoleArray[MAX_ROLES];
}ComponentTable;

/**
 * size for the array of allocated components.  Sets the maximum 
 * number of components that can be allocated at once
 */
#define MAXCOMP (50)
#define MAXNAMESIZE (130)
#define EMPTY_STRING "\0"

/** Determine the number of elements in an array */
#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))

/** Array to hold the DLL pointers for each allocated component */
static void* pModules[MAXCOMP] = {0};

/** Array to hold the component handles for each allocated component */
static void* pComponents[COUNTOF(pModules)] = {0};

/** Count for call OMX_Init() */
static OMX_U32 count = 0;

/** Array to hold the Audio DLL pointers for each allocated audio component */
static void* pAudioDLLModules[MAXCOMP] = {0};

static pthread_mutex_t mutex;
static OMX_U32 tableCount = 0;
static ComponentTable componentTable[MAX_TABLE_SIZE];
static char * sRoleArray[60][20];
static char compName[60][200];

static char *tComponentName[MAXCOMP][2] = {

	/*video Decoder components */
    { "OMX.TCC.mpeg4dec",       "video_decoder.mpeg4"    },
	{ "OMX.TCC.avcdec",         "video_decoder.avc"      },
	{ "OMX.TCC.h263dec",        "video_decoder.h263"     },
#ifdef OPENMAX1_2
    { "OMX.TCC.wmvdec",         "video_decoder.vc1"      },
#else
    { "OMX.TCC.wmvdec",         "video_decoder.tcc_wmv"  },
#endif
    { "OMX.TCC.wmv12dec",       "video_decoder.tcc_wmv12"},
	{ "OMX.TCC.rvdec",          "video_decoder.rv"       },
	{ "OMX.TCC.divxdec",        "video_decoder.divx"     },
	{ "OMX.TCC.mpeg2dec",       "video_decoder.mpeg2"    },
	{ "OMX.TCC.mjpegdec",       "video_decoder.mjpeg"    },
	/*FLV1 decoder component*/
    { "OMX.TCC.flv1dec",        "video_decoder.sorenson_h263"},
	{ "OMX.TCC.avsdec",         "video_decoder.avs"      },
	{ "OMX.TCC.vp8dec",         "video_decoder.vp8"      },
	{ "OMX.TCC.vp9dec",         "video_decoder.vp9"      },
	{ "OMX.TCC.mvcdec",         "video_decoder.mvc"      },
	{ "OMX.TCC.google.vp8dec",  "google_vpx_decoder.vp8" },
	{ "OMX.TCC.google.vp9dec",  "google_vpx_decoder.vp9" },
	/*video Encoder components */
    { "OMX.TCC.ENC.mpeg4",      "video_encoder.mpeg4"    },
	{ "OMX.TCC.ENC.avc",        "video_encoder.avc"      },
	{ "OMX.TCC.ENC.h263",       "video_encoder.h263"     },

	/* Audio Decoder components */
#ifdef OPENMAX1_2
    { "OMX.TCC.mp3dec",     "audio_decoder.mp3" },
	{ "OMX.TCC.aacdec",     "audio_decoder.aac" },
	{ "OMX.TCC.wmadec",     "audio_decoder.wma" },
#else
    { "OMX.TCC.mp3dec",     "audio_decoder.tcc_mp3" },
	{ "OMX.TCC.aacdec",     "audio_decoder.tcc_aac" },
	{ "OMX.TCC.wmadec",     "audio_decoder.tcc_wma" },
#endif
    { "OMX.TCC.ac3dec",     "audio_decoder.ac3" },
    { "OMX.TCC.radec",      "audio_decoder.ra"  },
	{ "OMX.TCC.flacdec",    "audio_decoder.flac"},
	{ "OMX.TCC.apedec",     "audio_decoder.ape" },
	{ "OMX.TCC.mp2dec",     "audio_decoder.mp2" },
	{ "OMX.TCC.pcmdec",     "audio_decoder.pcm_dec"},
	{ "OMX.TCC.vorbisdec",  "audio_decoder.vorbis" },
	{ "OMX.TCC.dtsdec",     "audio_decoder.dts" },
	
	/* Audio Encoder components */
    { "OMX.TCC.aacenc", "audio_encoder.aac" },
	{ "OMX.TCC.mp3enc", "audio_encoder.tcc_mp3" },

#ifdef OPENMAX1_2
	/* OMX clock */
    { "OMX.TCC.clock", "clock.binary" },

	/* renderer */
    { "OMX.TCC.audio_renderer", "audio_renderer.pcm" },
	{ "OMX.TCC.video_renderer", "iv_renderer.yuv.overlay" },

	/* video processor */
    { "OMX.TCC.video_processor", "iv_processor.yuv" },

	/* video scheduler */
    { "OMX.TCC.video_scheduler", "video_scheduler.binary" },
#endif

	/* terminate the table */
	{NULL, NULL},
};

static OMX_ERRORTYPE BuildComponentTable();
static OMX_ERRORTYPE ComponentTable_EventHandler(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_PTR pAppData,
    OMX_IN OMX_EVENTTYPE eEvent,
    OMX_IN OMX_U32 nData1,
    OMX_IN OMX_U32 nData2,
    OMX_IN OMX_PTR pEventData);

static OMX_ERRORTYPE ComponentTable_EmptyBufferDone(
    OMX_OUT OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_PTR pAppData,
    OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);

static OMX_ERRORTYPE ComponentTable_FillBufferDone(
    OMX_OUT OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_PTR pAppData,
    OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);


OMX_ERRORTYPE OMX_Init()
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    ALOGV("%s", __func__);

    BuildComponentTable();
    return eError;
}

OMX_ERRORTYPE OMX_GetHandle(OMX_HANDLETYPE* pHandle,
                                OMX_STRING cComponentName,
                                OMX_PTR pAppData, OMX_CALLBACKTYPE* pCallBacks)
{
    static const char prefix[] = "lib";
    static const char postfix[] = ".so";
#if 1 //ZzaU : Video Encoder/Decoder union!!
	OMX_ERRORTYPE (*pComponentInit)(OMX_HANDLETYPE*, OMX_STRING, OMX_HANDLE_FUNC);
	int (*pAudioProc)(int, int*, void*, void*);
#else	
    OMX_ERRORTYPE (*pComponentInit)(OMX_HANDLETYPE*);
#endif
    OMX_ERRORTYPE err = OMX_ErrorNone;  /* This was ErrorUndefined */
    OMX_COMPONENTTYPE *componentType;
    OMX_U32 i, j;
	OMX_BOOL bNeedAudioDllLoading = OMX_FALSE;
    char buf[sizeof(prefix) + MAXNAMESIZE +sizeof(postfix)];
	char audio_dll_buf[sizeof(prefix) + MAXNAMESIZE +sizeof(postfix)];
    const char* pErr = dlerror();

    ALOGV("%s", __func__);

    if(pthread_mutex_lock(&mutex) != 0) {
        ALOGE("Error in Mutex lock");
    }

    if ((NULL == cComponentName) || (NULL == pHandle) || (NULL == pCallBacks)) {
        err = OMX_ErrorBadParameter;
		ALOGE("Error OMX_ErrorBadParameter");
        goto EXIT;
    }

    /* Verify that the name is not too long and could cause a crash.  Notice
     * that the comparison is a greater than or equals.  This is to make
     * sure that there is room for the terminating NULL at the end of the
     * name. */
    if( strlen(cComponentName) >= MAXNAMESIZE) {
        err = OMX_ErrorInvalidComponentName;
		ALOGE("Error OMX_ErrorInvalidComponentName");
        goto EXIT;
    }
    /* Locate the first empty slot for a component.  If no slots
     * are available, error out */
    for(i=0; i< COUNTOF(pModules); i++) {
        if(pModules[i] == NULL) break;
    }
    if(i == COUNTOF(pModules)) {
         err = OMX_ErrorInsufficientResources;
		ALOGE("Error OMX_ErrorInsufficientResources");
        goto EXIT;
    }

    /* load the component and check for an error.  If filename is not an 
     * absolute path (i.e., it does not  begin with a "/"), then the 
     * file is searched for in the following locations:
     *
     *     The LD_LIBRARY_PATH environment variable locations
     *     The library cache, /etc/ld.so.cache.
     *     /lib
     *     /usr/lib
     * 
     * If there is an error, we can't go on, so set the error code and exit */

	OMX_U32 passthrough_on = 0;
	char value[PROPERTY_VALUE_MAX];
	property_get("persist.sys.spdif_setting", value, "");
	if (!strcmp(value, "2") || !strcmp(value, "4"))
	{
		// AudioOutput update Planet 20121120 Start
		char hdmi_audio[PROPERTY_VALUE_MAX];
		memset(hdmi_audio, NULL, PROPERTY_VALUE_MAX);
		property_get("tcc.hdmi.audio_type", hdmi_audio, "0");
		
		if(hdmi_audio[0]!='3')	// AudioOutput update Planet 20121120 End
			passthrough_on = 1;
	}

#if 1 //ZzaU : Video Encoder/Decoder union!!
	{
		
	    for (j = 0; j < MAXCOMP; j++) {
	        if (tComponentName[j][0] == NULL) {
	            break;
	        }

			if (strcmp(cComponentName, tComponentName[j][0]) == 0)
			{
				//found!!
				if(strncmp(tComponentName[j][1], "video_decoder.", 14) == 0)
				{
					strcpy(buf, prefix);					  /* the lengths are defined herein or have been */
					strcat(buf, "OMX.TCC.VideoDec");          /* checked already, so strcpy and strcat are	*/
					strcat(buf, postfix);					  /* are safe to use in this context. */
					break;
				}
				else if(strncmp(tComponentName[j][1], "google_vpx_decoder.", 19) == 0)
				{
					strcpy(buf, prefix);					  /* the lengths are defined herein or have been */
					strcat(buf, "OMX.TCC.google.vpxdec");     /* checked already, so strcpy and strcat are	*/
					strcat(buf, postfix);					  /* are safe to use in this context. */
					break;
				}
				else if(strncmp(tComponentName[j][1], "video_encoder.", 14) == 0)
				{
					strcpy(buf, prefix);					  /* the lengths are defined herein or have been */
					strcat(buf, "OMX.TCC.VideoEnc");          /* checked already, so strcpy and strcat are	*/
					strcat(buf, postfix);					  /* are safe to use in this context. */
					break;
				}
				else if((passthrough_on == 1) &&
						(strncmp(tComponentName[j][1], "audio_decoder.ac3", 17) == 0 ||
						 strncmp(tComponentName[j][1], "audio_decoder.dts", 17) == 0))
				{
					strcpy(buf, prefix);					  /* the lengths are defined herein or have been */
					strcat(buf, "OMX.TCC.spdif");             /* checked already, so strcpy and strcat are	*/
					strcat(buf, postfix);					  /* are safe to use in this context. */

					if(strncmp(tComponentName[j][1], "audio_decoder.ac3", 17) == 0)
					{
						bNeedAudioDllLoading = OMX_TRUE;
						strcpy(audio_dll_buf, prefix);				/* the lengths are defined herein or have been */
						strcat(audio_dll_buf, &cComponentName[4]);  /* checked already, so strcpy and strcat are	*/
						strcat(audio_dll_buf, postfix);				/* are safe to use in this context. */						
					}
					
					break;
				}
				else
				{
					strcpy(buf, prefix);					  /* the lengths are defined herein or have been */
					strcat(buf, cComponentName);              /* checked already, so strcpy and strcat are	*/
					strcat(buf, postfix);					  /* are safe to use in this context. */

					if(strncmp(tComponentName[j][1], "audio_decoder.", 14) == 0)
					{
						bNeedAudioDllLoading = OMX_TRUE;
						strcpy(audio_dll_buf, prefix);				/* the lengths are defined herein or have been */
						strcat(audio_dll_buf, &cComponentName[4]);  /* checked already, so strcpy and strcat are	*/
						strcat(audio_dll_buf, postfix);				/* are safe to use in this context. */						
					}

					if(strncmp(tComponentName[j][1], "audio_encoder.", 14) == 0)
					{
						bNeedAudioDllLoading = OMX_TRUE;
						strcpy(audio_dll_buf, prefix);				/* the lengths are defined herein or have been */
						strcat(audio_dll_buf, &cComponentName[4]);  /* checked already, so strcpy and strcat are	*/
						strcat(audio_dll_buf, postfix);				/* are safe to use in this context. */						
					}
					break;
				}
			}
		}
	}
#else
    strcpy(buf, prefix);                      /* the lengths are defined herein or have been */
    strcat(buf, cComponentName);  /* checked already, so strcpy and strcat are  */
    strcat(buf, postfix);                      /* are safe to use in this context. */
#endif
	pAudioProc = NULL;
	if(bNeedAudioDllLoading)
	{
		pAudioDLLModules[i] = dlopen(audio_dll_buf, RTLD_LAZY | RTLD_GLOBAL);
	    if( pAudioDLLModules[i] == NULL ) {
	        ALOGE("Load library '%s' failed: %s buf move on to the next stage", audio_dll_buf, dlerror());
	        //err = OMX_ErrorComponentNotFound;
	        //ALOGE("Error AudioDecoderlibraryNotFound");
	        //goto EXIT;
	        pAudioProc = NULL;
	    } else {
			ALOGD("Library '%s' for '%s' loaded", audio_dll_buf, cComponentName);
			pAudioProc = dlsym(pAudioDLLModules[i], "Audio_Proc");
	    	if( pAudioProc == NULL ) {
	        	err = OMX_ErrorInvalidComponent;
				ALOGE("pAudioDec Error OMX_ErrorInvalidComponent");
	        	goto EXIT;
	    	}
	    }
	}

    pModules[i] = dlopen(buf, RTLD_LAZY | RTLD_GLOBAL);
    if( pModules[i] == NULL ) {
        ALOGE("Load library '%s' failed: %s", buf, dlerror());
        err = OMX_ErrorComponentNotFound;
        goto EXIT;
    } else {
		ALOGD("Library '%s' '%s' loaded", buf, cComponentName);
    }

    /* Get a function pointer to the "OMX_ComponentInit" function.  If 
     * there is an error, we can't go on, so set the error code and exit */
    pComponentInit = dlsym(pModules[i], "OMX_ComponentInit");
    if( /*(pErr != NULL) || */(pComponentInit == NULL) ) {
        err = OMX_ErrorInvalidComponent;
		if(pErr != NULL)
			ALOGE("pErr Error OMX_ErrorInvalidComponent");
		else
			ALOGE("pComponentInit Error OMX_ErrorInvalidComponent");
        goto EXIT;
    }


    /* We now can access the dll.  So, we need to call the "OMX_ComponentInit"
     * method to load up the "handle" (which is just a list of functions to 
     * call) and we should be all set.*/
    *pHandle = malloc(sizeof(OMX_COMPONENTTYPE));
    if(*pHandle == NULL) {
        err = OMX_ErrorInsufficientResources;
        ALOGE("malloc of pHandle* failed");
        goto EXIT;
    }
	memset(*pHandle, 0, sizeof(OMX_COMPONENTTYPE));
    pComponents[i] = *pHandle;
    componentType = (OMX_COMPONENTTYPE*) *pHandle;
    componentType->nSize = sizeof(OMX_COMPONENTTYPE);   
#if 1 //ZzaU : Video Encoder/Decoder union!!
    err = (*pComponentInit)(*pHandle, cComponentName, pAudioProc);
#else	
    err = (*pComponentInit)(*pHandle);
#endif
    if (OMX_ErrorNone == err) {
        err = (componentType->SetCallbacks)(*pHandle, pCallBacks, pAppData);
        if (err != OMX_ErrorNone) {
            ALOGE("Error Returned From Component SetCallBack");
            goto EXIT;
        }    
    } else {
        /* when the component fails to initialize, release the
           component handle structure */
        free(*pHandle);
        /* mark the component handle as NULL to prevent the caller from
           actually trying to access the component with it, should they
           ignore the return code */
        *pHandle = NULL;
        pComponents[i] = NULL;
        dlclose(pModules[i]);
		pModules[i] = NULL;
		
		if(pAudioDLLModules[i] != NULL)
		{
			dlclose(pAudioDLLModules[i]);
			pAudioDLLModules[i] = NULL;
		}
        /*  err = OMX_ErrorComponentNotFound; */
		ALOGE("Error[0x%x] init fail!!", err);
        goto EXIT;
    }
    err = OMX_ErrorNone;
EXIT:
    if(pthread_mutex_unlock(&mutex) != 0) {
        ALOGE("Error in Mutex unlock");
    } 
    return (err);
}
 

/**
 * Description:This method will unload the OMX component pointed by 
 * OMX_HANDLETYPE. It is the responsibility of the calling method to ensure that
 * the Deinit method of the component has been called prior to unloading component
 *
 * @param hComponent the component to unload
 *
 * @return OMX_NOERROR Successful
 */
OMX_ERRORTYPE OMX_FreeHandle (OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE retVal = OMX_ErrorUndefined;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)hComponent;
    OMX_U32 i;

    ALOGV("%s", __func__);

    if(pthread_mutex_lock(&mutex) != 0) {
        ALOGE("Error in Mutex lock");
    }

    /* Locate the component handle in the array of handles */
    for(i=0; i< COUNTOF(pModules); i++) {
        if(pComponents[i] == hComponent) break;
    }
    
    if(i == COUNTOF(pModules)) {
        retVal = OMX_ErrorBadParameter;
        goto EXIT;
    }

    retVal = pHandle->ComponentDeInit(hComponent);
    if (retVal != OMX_ErrorNone) {
        ALOGE("Error[0x%x] From ComponentDeInit", retVal);
        goto EXIT; 
    }

    /* release the component and the component handle */
    dlclose(pModules[i]);
    pModules[i] = NULL;

	if(pAudioDLLModules[i] != NULL)
	{
		dlclose(pAudioDLLModules[i]);
		pAudioDLLModules[i] = NULL;
	}

    free(pComponents[i]);
    pComponents[i] = NULL;
    retVal = OMX_ErrorNone;
   
EXIT:
    /* The unload is now complete, so set the error code to pass and exit */
    if(pthread_mutex_unlock(&mutex) != 0) {
        ALOGE("Error in Mutex unlock");
    } 

    return retVal;
}

/**
 * This method will release the resources of the OMX Core.  It is the 
 * responsibility of the application to call OMX_DeInit to ensure
 * the clean up of these resources.
 *
 * Returns:    OMX_NOERROR          Successful
 */
OMX_ERRORTYPE OMX_Deinit()
{
    count--;

    ALOGV("%s", __func__);

    if(pthread_mutex_lock(&mutex) != 0)
        ALOGE("Error in Mutex lock");
        
    if(count == 0) {
        if (pthread_mutex_unlock(&mutex) != 0)
            ALOGE("Error in Mutex unlock");
        if (pthread_mutex_destroy(&mutex) != 0) {
            ALOGE("Error in Mutex destroy");
        }
    } else {
        if (pthread_mutex_unlock(&mutex) != 0)
            ALOGE("Error in Mutex unlock");
    }            

    return OMX_ErrorNone;
}

/*************************************************************************
* OMX_SetupTunnel()
*
* Description: Setup the specified tunnel the two components
*
* Parameters:
* @param[in] hOutput     Handle of the component to be accessed
* @param[in] nPortOutput Source port used in the tunnel
* @param[in] hInput      Component to setup the tunnel with.
* @param[in] nPortInput  Destination port used in the tunnel
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
**************************************************************************/
/* OMX_SetupTunnel */
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_SetupTunnel(
    OMX_IN  OMX_HANDLETYPE hOutput,
    OMX_IN  OMX_U32 nPortOutput,
    OMX_IN  OMX_HANDLETYPE hInput,
    OMX_IN  OMX_U32 nPortInput)
{
    OMX_ERRORTYPE eError = OMX_ErrorNotImplemented;
    OMX_COMPONENTTYPE *pCompIn, *pCompOut;
    OMX_TUNNELSETUPTYPE oTunnelSetup;

    ALOGV("%s", __func__);

    if (hOutput == NULL && hInput == NULL)
        return OMX_ErrorBadParameter;

    oTunnelSetup.nTunnelFlags = 0;
    oTunnelSetup.eSupplier = OMX_BufferSupplyUnspecified;
    
    pCompOut = (OMX_COMPONENTTYPE*)hOutput;

    if (hOutput) {
        eError = pCompOut->ComponentTunnelRequest(hOutput, nPortOutput, hInput, nPortInput, &oTunnelSetup);
    }

    if (eError == OMX_ErrorNone && hInput) {  
        pCompIn = (OMX_COMPONENTTYPE*)hInput;
        eError = pCompIn->ComponentTunnelRequest(hInput, nPortInput, hOutput, nPortOutput, &oTunnelSetup);
        if (eError != OMX_ErrorNone && hOutput) {
            /* cancel tunnel request on output port since input port failed */
            pCompOut->ComponentTunnelRequest(hOutput, nPortOutput, NULL, 0, NULL);
        }
    }
  
    return eError;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_GetContentPipe(
    OMX_OUT OMX_HANDLETYPE *hPipe,
    OMX_IN OMX_STRING szURI)
{
    return OMX_ErrorNotImplemented;
}

/*************************************************************************
* OMX_ComponentNameEnum()
*
* Description: This method will provide the name of the component at the given nIndex
*
*Parameters:
* @param[out] cComponentName       The name of the component at nIndex
* @param[in] nNameLength                The length of the component name
* @param[in] nIndex                         The index number of the component 
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
**************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_ComponentNameEnum(
    OMX_OUT OMX_STRING cComponentName,
    OMX_IN  OMX_U32 nNameLength,
    OMX_IN  OMX_U32 nIndex)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    
    ALOGV("%s: nIndex=%d", __func__, (int) nIndex);

    if (nIndex >=  tableCount) {
        eError = OMX_ErrorNoMore;
    } else {
        strcpy(cComponentName, componentTable[nIndex].name);
    }
    
    return eError;
}


/*************************************************************************
* OMX_GetRolesOfComponent()
*
* Description: This method will query the component for its supported roles
*
*Parameters:
* @param[in] cComponentName     The name of the component to query
* @param[in] pNumRoles     The number of roles supported by the component
* @param[in] roles              The roles of the component
*
* Returns:    OMX_NOERROR          Successful
*                 OMX_ErrorBadParameter         Faliure due to a bad input parameter
*
* Note
*
**************************************************************************/
OMX_API OMX_ERRORTYPE OMX_GetRolesOfComponent (
    OMX_IN      OMX_STRING cComponentName,
    OMX_INOUT   OMX_U32 *pNumRoles,
    OMX_OUT     OMX_U8 **roles)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 i = 0;
    OMX_U32 j = 0;
    OMX_BOOL bFound = OMX_FALSE;
    
    ALOGV("%s: cComponentName=%s", __func__, cComponentName);

    if (cComponentName == NULL) {
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }

    while(!bFound && i < tableCount) {
        if (strcmp(cComponentName, componentTable[i].name) == 0) {
            bFound = OMX_TRUE;
        } else {
            i++;
        }
    }
    if (roles == NULL) { 
        *pNumRoles = componentTable[i].nRoles;
        goto EXIT;
    } else {
        if (bFound && (*pNumRoles == componentTable[i].nRoles)) {
            for (j = 0; j<componentTable[i].nRoles; j++) {
                strcpy((OMX_STRING)roles[j], componentTable[i].pRoleArray[j]);
            }
        }
    }
EXIT:
    return eError;
}

/*************************************************************************
* OMX_GetComponentsOfRole()
*
* Description: This method will query the component for its supported roles
*
*Parameters:
* @param[in] role     The role name to query for
* @param[in] pNumComps     The number of components supporting the given role
* @param[in] compNames      The names of the components supporting the given role
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
**************************************************************************/
OMX_API OMX_ERRORTYPE OMX_GetComponentsOfRole ( 
    OMX_IN      OMX_STRING role,
    OMX_INOUT   OMX_U32 *pNumComps,
    OMX_INOUT   OMX_U8  **compNames)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 i = 0;
    OMX_U32 j = 0;
    OMX_U32 k = 0;

    ALOGV("%s", __func__);

    if (role == NULL) {
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }
    
    /* This implies that the componentTable is not filled */
    if (componentTable[i].pRoleArray[j] == NULL)
    {
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }
    
    for (i = 0; i < tableCount; i++) {
        for (j = 0; j<componentTable[i].nRoles; j++) { 
            if (strcmp(componentTable[i].pRoleArray[j], role) == 0) {
                /* the first call to this function should only count the number
                   of roles so that for the second call compNames can be allocated
                   with the proper size for that number of roles */
                if (compNames != NULL) {
                    compNames[k] = (OMX_U8*)componentTable[i].name;
                }
                k++;
            }
        }
        *pNumComps = k;
    }
    
EXIT:
    return eError;
}


OMX_ERRORTYPE BuildComponentTable()
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_CALLBACKTYPE sCallbacks;
    int j = 0;
    int numFiles = 0;
    OMX_U32 i;

    ALOGV("%s", __func__);

    /* set up dummy call backs */
    sCallbacks.EventHandler    = ComponentTable_EventHandler;
    sCallbacks.EmptyBufferDone = ComponentTable_EmptyBufferDone;
    sCallbacks.FillBufferDone  = ComponentTable_FillBufferDone;

    for (i = 0, numFiles = 0; i < MAXCOMP; i ++) {
        if (tComponentName[i][0] == NULL) {
            break;
        }
        
        for (j = 0; j < numFiles; j ++) {
            if (!strcmp(componentTable[j].name, tComponentName[i][0])) {
                /* insert the role */
                if (tComponentName[i][1] != NULL) {
                    componentTable[j].pRoleArray[componentTable[j].nRoles] = tComponentName[i][1];
                    componentTable[j].nRoles ++;
                }
                break;
            }
        }
        
        if (j == numFiles) { /* new component */
            if (tComponentName[i][1] != NULL){
                componentTable[numFiles].pRoleArray[0] = tComponentName[i][1];
                componentTable[numFiles].nRoles = 1;
            }
            strcpy(compName[numFiles], tComponentName[i][0]);
            componentTable[numFiles].name = compName[numFiles];
            numFiles ++;
        }
    }
    tableCount = numFiles;

    if (eError != OMX_ErrorNone){
        printf("Error[0x%x]:  Could not build Component Table\n", eError);
    }
    return eError;
}

OMX_ERRORTYPE ComponentTable_EventHandler(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_PTR pAppData,
    OMX_IN OMX_EVENTTYPE eEvent,
    OMX_IN OMX_U32 nData1,
    OMX_IN OMX_U32 nData2,
    OMX_IN OMX_PTR pEventData)
{
    ALOGV("%s", __func__);
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE ComponentTable_EmptyBufferDone(
    OMX_OUT OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_PTR pAppData,
    OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
    ALOGV("%s", __func__);
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE ComponentTable_FillBufferDone(
    OMX_OUT OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_PTR pAppData,
    OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
    ALOGV("%s", __func__);
    return OMX_ErrorNotImplemented;
}
