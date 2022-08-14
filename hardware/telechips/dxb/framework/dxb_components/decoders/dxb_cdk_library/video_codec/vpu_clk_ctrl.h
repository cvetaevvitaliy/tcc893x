/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/
   																				
#ifndef _VPU_CLK_CTRL_H_
#define _VPU_CLK_CTRL_H_

#include "../cdk/cdk_pre_define.h"
#ifdef VPU_CLK_CONTROL

/************************************************************************/
/*						                                                */
/* Defines				                                                */
/*						                                                */
/************************************************************************/


#ifndef _TCC9300_
#define VPU_CLK_CTRL_VBUS_PWR_DOWN_REG			0xF0702000
#define VPU_CLK_CTRL_CLKCTRL_REG_BASE			0xF0400000	//CLK Register Base Address
#define VPU_CLK_CTRL_CLKCTRL5_REG				0xF0400014	//Bus Clock Control Register for VIdeo Codec
#define VPU_CLK_CTRL_CLKCTRL6_REG				0xF0400018	//Core Clock Control Register for VIdeo Codec
#else
#define VPU_CLK_CTRL_VBUS_PWR_DOWN_REG			0xB0920000
#define VPU_CLK_CTRL_CLKCTRL_REG_BASE			0xB0500000	//CLK Register Base Address
#define VPU_CLK_CTRL_CLKCTRL5_REG				0xB0500014	//Bus Clock Control Register for VIdeo Codec
#define VPU_CLK_CTRL_CLKCTRL6_REG				0xB0500018	//Core Clock Control Register for VIdeo Codec
#endif
#define VPU_CLK_CTRL_VBUS_PWR_DOWN_CODEC_MASK	(1<<2)
#define VPU_CLK_CTRL_CKLCTRL_SEL				( 0)
#define VPU_CLK_CTRL_CKLCTRL_SYNC				( 3)
#define VPU_CLK_CTRL_CKLCTRL_CONFIG				( 4)
#define VPU_CLK_CTRL_CKLCTRL_MD					(20)
#define VPU_CLK_CTRL_CKLCTRL_EN					(21)

/*!
 ***********************************************************************
 * \brief
 *		vpu clock init. function for LINUX
 ***********************************************************************
 */
int
vpu_clock_init(void);

/*!
 ***********************************************************************
 * \brief
 *		vpu clock deinit. function for LINUX
 ***********************************************************************
 */
int
vpu_clock_deinit(void);

#endif
#endif //_VPU_CLK_CTRL_H_
