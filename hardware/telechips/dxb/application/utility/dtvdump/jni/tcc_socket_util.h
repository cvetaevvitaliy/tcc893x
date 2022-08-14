/*
 * Copyright (C) 2013 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __TCSOCKET_UTIL_H__
#define __TCSOCKET_UTIL_H__

typedef void *TCSOCK_HANDLE;
TCSOCK_HANDLE TCSOCKUTIL_Open (char *pcIPstr, int iPort);
int TCSOCKUTIL_Read(TCSOCK_HANDLE handle, unsigned char *pucBuffer, int iBufferSize, int iTimeOutMS);
void TCSOCKUTIL_Close(TCSOCK_HANDLE handle);


#endif // __TCSOCKET_UTIL_H__

