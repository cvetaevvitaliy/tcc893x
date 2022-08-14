/****************************************************************************
*   FileName    : interface_client.h
*   Description : 
****************************************************************************
*
*   TCC Version 1.0
*   Copyright (c) Telechips, Inc.
*   ALL RIGHTS RESERVED
*
****************************************************************************/

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/

#ifndef _INTERFACE_CLIENT_
#define _INTERFACE_CLIENT_

#include <utils/threads.h> // Mutex
#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>



namespace android
{
    class InterfaceClient : public BBinder
    {
        protected:
		private:		
		    sp<IMemory> m_AudioBuffer;
		    sp<IMemory> m_TransBuffer;
            sp<IBinder> m_service;            
            virtual status_t onTransact(uint32_t, const Parcel&, Parcel*, uint32_t);
        public:		
            InterfaceClient();
            ~InterfaceClient();
			int WriteAudioInfo(unsigned int uiSampleRate, unsigned int uiChannelCounts);
			int WriteVideoFrameInfo(unsigned int uiDecodecdWidth, unsigned int uiDecodecdHeight, unsigned int uiFrame_rate);
            int WriteVideoFrame(unsigned char *pucData,unsigned int uiSize, unsigned int uiPTS, unsigned int uiPicType, void *pOption);
            int WriteAudioFrame(unsigned char *pucData,unsigned int uiSize, unsigned int uiPTS, void *pOption);
			int WriteSubtitleFrame(void *pData, unsigned int uiFormat, void *pOption);
			int WriteTrsStream(unsigned char *pucData, unsigned int uiSize, long long uiPTS, void *pOption);
			int TrsPrepare(char *pucData, void *pOption);
			int SetDrmContents(unsigned int setFlag);
    };

};
// namespace 

#endif

