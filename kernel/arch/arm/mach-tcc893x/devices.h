/* linux/arch/arm/mach-tcc893x/devices.h
 *
 * Copyright (C) 2011 Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __ARCH_ARM_MACH_tcc893X_DEVICES_H
#define __ARCH_ARM_MACH_tcc893X_DEVICES_H

extern struct platform_device tcc8930_i2c_core0_device;
extern struct platform_device tcc8930_i2c_core1_device;
extern struct platform_device tcc8930_i2c_core2_device;
extern struct platform_device tcc8930_i2c_core3_device;
extern struct platform_device tcc8930_i2c_smu_device;

extern struct platform_device tcc8930_uart0_device;
extern struct platform_device tcc8930_uart1_device;
extern struct platform_device tcc8930_uart2_device;
extern struct platform_device tcc8930_uart3_device;
extern struct platform_device tcc8930_uart4_device;
extern struct platform_device tcc8930_uart5_device;

#if defined(CONFIG_RTC_DRV_TCC8930)
extern struct platform_device tcc8930_rtc_device;
#endif  /* CONFIG_RTC_DRV_TCC */
#if defined(CONFIG_INPUT_TCC_REMOTE)
extern struct platform_device tcc8930_remote_device;
#endif  /* CONFIG_INPUT_TCC_REMOTE */

extern struct platform_device tcc_sdhc0_device;
extern struct platform_device tcc_sdhc1_device;
extern struct platform_device tcc_sdhc2_device;
extern struct platform_device tcc_sdhc3_device;

extern int tcc_panel_id;
extern struct platform_device tcc_lcd_device;

extern struct platform_device tcc8930_dwc_otg_device;
extern struct platform_device ehci_hs_device;
extern struct platform_device ehci_fs_device;
extern struct platform_device xhci_hs_device;

extern struct platform_device tcc8930_dwc3_device;

extern struct platform_device tcc_ahci_device;

extern struct platform_device tcc8930_adc_device; // kch

#if defined(CONFIG_BATTERY_TCC)
extern struct platform_device tcc_battery_device; //kch
#endif

#if defined(CONFIG_TCC_GMAC) || defined(CONFIG_TCC_GMAC_MODULE)
extern struct platform_device tcc_gmac_device;
#endif

extern struct platform_device tcc8930_spi0_device;
extern struct platform_device tcc8930_spi1_device;
extern struct platform_device tcc_tsif_device;
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
extern struct platform_device tcc_tsif_ex_device;
#endif

#if defined(CONFIG_TCC_ECID_SUPPORT)
extern struct platform_device tcc_cpu_id_device;
#endif

#if defined(CONFIG_ION)
extern struct platform_device tcc_ion;
#endif

#endif
