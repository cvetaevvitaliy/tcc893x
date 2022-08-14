

#ifndef __TCSOCKET_UTIL_H__
#define __TCSOCKET_UTIL_H__

typedef void *TCSOCK_HANDLE;
TCSOCK_HANDLE TCSOCKUTIL_Open (char *pcIPstr, int iPort);
int TCSOCKUTIL_Read(TCSOCK_HANDLE handle, unsigned char *pucBuffer, int iBufferSize, int iTimeOutMS);
void TCSOCKUTIL_Close(TCSOCK_HANDLE handle);


#endif // __TCSOCKET_UTIL_H__

