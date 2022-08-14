/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PLATFORM_TCC_IOMAP_H_
#define _PLATFORM_TCC_IOMAP_H_

#include <platform/reg_physical.h>

#define TCC_GPIO_BASE        HwGPIO_BASE
#define TCC_UART0_BASE       HwUART0_BASE
#define TCC_UART1_BASE       HwUART1_BASE
#define TCC_UART2_BASE       HwUART2_BASE
#define TCC_UART3_BASE       HwUART3_BASE
#define TCC_UART4_BASE       HwUART4_BASE
#define TCC_UART5_BASE       HwUART5_BASE
#define TCC_UART6_BASE       HwUART6_BASE
#define TCC_UART7_BASE       HwUART7_BASE
#define TCC_IOBUSCFG_BASE    HwIOBUSCFG_BASE
#define TCC_VIC_BASE         HwPIC_BASE
#define TCC_TIMER_BASE       HwTMR_BASE
#define TCC_PMU_BASE         HwPMU_BASE
#define TCC_I2C_CH0_BASE     HwI2C_MASTER0_BASE
#define TCC_SMU_I2C_CH0_BASE HwSMUI2C_BASE
#define TCC_USB20OTG_BASE	 HwUSB20OTG_BASE
#define TCC_USBPHYCFG_BASE	 HwUSBPHYCFG_BASE
#define TCC_TSADC_BASE		 HwTSADC_BASE

#endif
