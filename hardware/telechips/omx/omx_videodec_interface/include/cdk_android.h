/****************************************************************************
 *   FileName    : cdk_android.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/


#ifndef _CDK_ANDROID_H_
#define _CDK_ANDROID_H_

#define CDK_ANDROID_VIDEO_EXTRA_INFO_SIZE 8

#define CDK_ANDROID_VIDEO_EXTRA_DATA_IDENTIFIER 0x54434320 // 'TCC '

#define CDK_ANDROID_VIDEO_EXTRA_NORMAL_MODE				0x00000000
#define CDK_ANDROID_VIDEO_EXTRA_NORMAL_MODE_STEPUP1		0x00000001
#define CDK_ANDROID_VIDEO_EXTRA_NORMAL_MODE_STEPUP2		0x00000002
#define CDK_ANDROID_VIDEO_EXTRA_FRAME_SKIP_MODE			0x00000003

typedef struct
{
	unsigned long id;
	unsigned long mode; 
} CDK_ANDROID_VIDEO_EXTRA_INFO;

#endif // _CDK_ANDROID_H_

