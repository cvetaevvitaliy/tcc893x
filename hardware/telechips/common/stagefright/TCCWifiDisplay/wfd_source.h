/****************************************************************************
*   FileName    : mediastream.h
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
#ifndef TCC_WFD_SOURCE_H
#define TCC_WFD_SOURCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ortp.h>

#define MAX_RTP_SIZE	1500

enum WFDSourceStatus {
    WFD_READY,
    WFD_RUNNING,
 };

struct _WFDSoUMediaStreamInfo
{	
	unsigned int wfd_soruce_status;
	unsigned int vbps; // Video 
	unsigned int vfps; // Video
	unsigned int keyframe_interval; //Video
	unsigned int source_width; //Video
	unsigned int source_height; //Video
	unsigned int target_width; //Video
	unsigned int target_height; //Video
	unsigned int audio_codec; //Audio
	unsigned int audio_bitrate; //Audio
	unsigned int audio_samplerate;
	unsigned int audio_channel;
	RtpSession *session;
	OrtpEvQueue *evq;
};

typedef struct _WFDSoUMediaStreamInfo WFDSoUMediaStreamInfo;

extern int TCC_WFD_SOURCE_Init(WFDSoUMediaStreamInfo *stream);


extern int TCC_WFD_SOURCE_Start(void);


extern int TCC_WFD_SOURCE_Stop(WFDSoUMediaStreamInfo *stream);


extern int TCC_WFD_SOURCE_Pause(unsigned int puase);


extern int TCC_WFD_SOURCE_SendVideoInfo(void);


extern int TCC_WFD_SOURCE_SendAudioInfo(void);


extern int TCC_WFD_SOURCE_WriteAudioFrame(unsigned char *pcmdata, int len, unsigned int ts);


extern int TCC_WFD_SOURCE_SetBitrate(unsigned long Bitrate);


extern unsigned long TCC_WFD_SOURCE_GetBitrate(void);

extern int TCC_WFD_SOURCE_Set_HDCP(int hdcp_enable);

extern int TCC_WFD_SOURCE_Set_Contents_Protection(int drm_contents_protection);

extern int TCC_WFD_SOURCE_Set_HDCP2_Mode(int mode);

extern int TCC_WFD_SOURCE_Setup_RTP_Session(WFDSoUMediaStreamInfo *stream, int localport, const char *remote_ip, int remoteport);

extern void TCC_WFD_SOURCE_SetRTPSession(RtpSession *session);
extern int TCC_WFD_SOURCE_Video_Recovery(void);
#ifdef __cplusplus
}
#endif

#endif //TCC_TRSCODER_H
