/****************************************************************************
 *   FileName    : TCC_VPUs_CODEC.h
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

#include "TCC_VPU_C5_CODEC.h"

#ifndef _TCC_VPU_CODEC_H_
#define _TCC_VPU_CODEC_H_

#define RETCODE_MULTI_CODEC_EXIT_TIMEOUT	99

#ifndef SZ_1M
#define SZ_1M           (1024*1024)
#endif
#define ARRAY_MBYTE(x)            ((((x) + (SZ_1M-1))>> 20) << 20)

#define VPU_ENC_PUT_HEADER_SIZE (16*1024)
#define VPU_SW_ACCESS_REGION_SIZE ARRAY_MBYTE(WORK_CODE_PARA_BUF_SIZE + VPU_ENC_PUT_HEADER_SIZE)

typedef struct dec_user_info_t
{
	unsigned int  bitrate_mbps;				//!< video BitRate (Mbps)
	unsigned int  frame_rate;				//!< video FrameRate (fps)
	unsigned int  m_bJpegOnly;				//!< If this is set, content is jpeg only file (ex. **.jpg)
	unsigned int  jpg_length;	
	unsigned int  jpg_ScaleRatio; 			////!< 0 ( Original Size )
											//!< 1 ( 1/2 Scaling Down )
											//!< 2 ( 1/4 Scaling Down )
											//!< 3 ( 1/8 Scaling Down )	
}dec_user_info_t;

#endif
