#ifndef _TCC_VIDEO_CONFIG_DATA_H
#define _TCC_VIDEO_CONFIG_DATA_H

#define TCC_VIDEO_CONFIG_ID "TCCCONFIG"

typedef struct _TCCVideoConfigData
{
	long iWidth;
	long iHeight;
	char id[10]; 	
	char iContainerType;
	char iBitRate;
	long iFrameRate; 
} TCCVideoConfigData;

#endif // _TCC_VIDEO_CONFIG_DATA_H
