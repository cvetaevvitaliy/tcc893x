/****************************************************************************
 * arch/arm/mach-tcc892x/clocktable_list_ddr3.h
 * Copyright (C) 2013 Telechips Inc.
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

#ifndef __CLOCKTABLE_LIST_H__
#define __CLOCKTABLE_LIST_H__

struct core_voltage_t {
    struct clock_table_t    clks;
    unsigned int            voltage;    /* Unit.: mV */
};

const static struct core_voltage_t voltage_table[] = {
    /*     CPU     MEM     DDI     GPU     I/O    VBUS   VCORE    HSIO     SMU    Voltage    */
    {{  343750, 265050, 155150, 184000,  97470, 137750, 137750, 124320,  97470 }, 1000000},
    {{  401250, 322885, 189010, 224140, 118735, 167800, 167800, 151445, 118735 }, 1050000},
    {{  468750, 380710, 222860, 264290, 140000, 197860, 197860, 178570, 140000 }, 1100000},
    {{  546875, 456860, 267425, 317140, 168000, 237425, 237425, 214285, 168000 }, 1150000},
#if defined(CONFIG_TCC_GMAC) || defined(CONFIG_TCC_GMAC_MODULE)
    {{  625000, 533000, 312000, 370000, 196000, 277000, 277000, 230000, 196000 }, 1200000},
    {{  716500, 563450, 349440, 414400, 219520, 310240, 310240, 240000, 219520 }, 1250000},
#else
    {{  625000, 533000, 312000, 370000, 196000, 277000, 277000, 250000, 196000 }, 1200000},
    {{  716500, 563450, 349440, 414400, 219520, 310240, 310240, 280000, 219520 }, 1250000},
#endif
    {{  808000, 600000, 386880, 458800, 243040, 343480, 343480, 310000, 243040 }, 1300000},
    {{  833000, 600000, 396400, 470090, 249020, 351930, 351930, 317630, 249020 }, 1320000},
    {{  996800, 600000, 441450, 523510, 277320, 391930, 391930, 353720, 277320 }, 1425000},
};

const static struct clock_table_t normal_clktbls[] = {
    /*     CPU     MEM     DDI     GPU     I/O    VBUS   VCORE    HSIO     SMU */
    {   343750, 300000,      0,      0,  97470,      0,      0,      0,  97470 },
    {   401250, 322885,      0,      0, 118735,      0,      0,      0, 118735 },
    {   468750, 380710,      0,      0, 140000,      0,      0,      0, 118735 },
    {   546875, 456860,      0,      0, 168000,      0,      0,      0, 118735 },
    {   625000, 533000,      0,      0, 196000,      0,      0,      0, 118735 },
    {   716500, 563450,      0,      0, 219520,      0,      0,      0, 118735 },
    {   808000, 600000,      0,      0, 243040,      0,      0,      0, 118735 },
    {   833000, 600000,      0,      0, 249020,      0,      0,      0, 118735 },
    {   996800, 600000,      0,      0, 249020,      0,      0,      0, 118735 },
};

const static struct clock_table_t camera_clktbls[] = {
    {        0, 380710, 267425,      0,      0,      0,      0,      0,      0 },
    {        0, 456860, 267425,      0,      0,      0,      0,      0,      0 },
    {        0, 600000, 386880,      0,      0, 343480, 343480,      0,      0 },
    {        0, 600000, 441450,      0,      0, 391930, 391930,      0,      0 },
};

const static struct clock_table_t isp_camera_clktbls[] = {
    {        0, 380710, 222860,      0,      0,      0,      0,      0,      0 },
    {        0, 533000, 222860,      0,      0,      0,      0,      0,      0 },
    {        0, 533000, 386880,      0,      0,      0,      0,      0,      0 },
};

const static struct clock_table_t hwc_ddi_clktbls[] = {  /* same with vpu_clktbl's ddi clock */
#if defined(CONFIG_DRAM_16BIT_USED)
    {        0,      0, 189010,      0,      0,      0,      0,      0,      0 },
    {        0,      0, 222860,      0,      0,      0,      0,      0,      0 },
#else
    {        0,      0, 189010,      0,      0,      0,      0,      0,      0 },
    {        0,      0, 222860,      0,      0,      0,      0,      0,      0 },
#endif
#if defined(CONFIG_STB_BOARD_DONGLE) && !defined(CONFIG_CHIP_TCC8925S)
    {        0,      0, 312000,      0,      0,      0,      0,      0,      0 },
#else
    {        0,      0, 386880,      0,      0,      0,      0,      0,      0 },
#endif
};

const static struct clock_table_t vpu_clktbls[] = {
#if defined(CONFIG_DRAM_16BIT_USED)
    {   343750, 380710, 189010,      0, 118735, 167800, 167800,      0,      0 },
    {   401250, 456860, 222860,      0, 140000, 197860, 197860,      0,      0 },
#else
    {   343750, 322885, 189010,      0, 118735, 167800, 167800,      0,      0 },
    {   401250, 380710, 222860,      0, 140000, 197860, 197860,      0,      0 },
#endif
#if defined(CONFIG_STB_BOARD_DONGLE) && !defined(CONFIG_CHIP_TCC8925S)
    {   401250, 533000, 312000,      0, 196000, 277000, 277000,      0,      0 },
#else
    {   401250, 600000, 386880,      0, 243040, 343480, 343480,      0,      0 },
#endif
};

const static struct clock_table_t vpu_480p_clktbls[] = {
    {   343750,      0,      0,      0, 118735,      0,      0,      0,      0 },
    {   401250,      0,      0,      0, 140000,      0,      0,      0,      0 },
    {   468750,      0,      0,      0, 168000,      0,      0,      0,      0 },
    {   546875,      0,      0,      0, 168000,      0,      0,      0,      0 },
};

const static struct clock_table_t vpu_720p_clktbls[] = {
    {   401250,      0,      0,      0, 168000,      0,      0,      0,      0 },
    {   468750,      0,      0,      0, 168000,      0,      0,      0,      0 },
    {   625000,      0,      0,      0, 219520, 237425, 237425,      0,      0 },
    {   808000, 533000, 386880,      0, 243040, 343480, 343480,      0,      0 },
};

const static struct clock_table_t vpu_1080p_clktbls[] = {
    {   468750,      0,      0, 414400,      0,      0,      0,      0,      0 }, //    ~ 10 Mbps
    {   625000,      0,      0, 414400,      0,      0,      0,      0,      0 }, // 10 ~ 20 Mbps
    {   716500,      0,      0, 458800,      0,      0,      0,      0,      0 }, // 20 ~ 30 Mbps
    {   808000,      0,      0, 458800,      0,      0,      0,      0,      0 }, // 30 ~    Mbps
};

const static struct clock_table_t jpeg_clktbls[]= {
    {        0, 380710,      0,      0, 140000, 197860, 197860,      0,      0 },
    {        0, 533000,      0,      0, 196000, 277000, 277000,      0,      0 },
    {        0, 600000,      0,      0, 243040, 343480, 343480,      0,      0 },
};

const static struct clock_table_t hdmi_clktbls[] = {
#if defined(CONFIG_STB_BOARD_DONGLE) && !defined(CONFIG_CHIP_TCC8925S)
    {        0, 533000, 312000,      0, 196000,      0,      0,      0,      0 },
#elif defined(CONFIG_STB_BOARD_DONGLE) && defined(CONFIG_CHIP_TCC8925S)
    {        0, 600000, 386880,      0, 243040,      0,      0,      0,      0 },
#else
    {   625000, 600000, 386880,      0, 243040,      0,      0,      0,      0 },
#endif
};

const static struct clock_table_t mali_clktbls[] = {
#if defined(CONFIG_GPU_BUS_SCALING)
    {        0, 300000,      0, 184000,      0,      0,      0,      0,      0 },
    {        0, 322885,      0, 224140,      0,      0,      0,      0,      0 },
    {        0, 380710,      0, 264290,      0,      0,      0,      0,      0 },
    {        0, 456860,      0, 317140,      0,      0,      0,      0,      0 },
    {        0, 533000,      0, 370000,      0,      0,      0,      0,      0 },
    {        0, 563450,      0, 414400,      0,      0,      0,      0,      0 },
    {        0, 600000,      0, 458800,      0,      0,      0,      0,      0 },
#else
    {        0, 533000,      0, 370000,      0,      0,      0,      0,      0 },
#endif
};

const static struct clock_table_t fb_clktbls[] = {
    {        0, 300000, 155150,      0,  97470,      0,      0,      0,      0 },

};

const static struct clock_table_t overlay_clktbls[] = {
    {        0,      0,      0,      0, 196000,      0,      0,      0,      0 },
};

const static struct clock_table_t bt_clktbls[] = {
#if defined(CONFIG_STB_BOARD_DONGLE) && !defined(CONFIG_CHIP_TCC8925S)
    {        0, 533000,      0,      0, 196000,      0,      0,      0,      0 },
#else
    {        0, 600000,      0,      0, 196000,      0,      0,      0,      0 },
#endif
};

const static struct clock_table_t usb_clktbls[] = {
    {        0,      0,      0,      0,  97470,      0,      0,      0,      0 }, // Idle
    {        0, 533000,      0,      0, 196000,      0,      0,      0,      0 }, // Actived
};

const static struct clock_table_t remocon_clktbls[] = {
    {        0,      0,      0,      0, 243040,      0,      0,      0,      0 },

};

const static struct clock_table_t hsio_clktbls[] = {
    {        0,      0,      0,      0,      0,      0,      0, 124320,      0 },
    {        0, 533000,      0,      0,      0,      0,      0, 250000,      0 },
};

const static struct clock_table_t voip_clktbls[] = {
    {   625000, 533000,      0,      0, 196000,      0,      0,      0, 118735 },
#if defined(CONFIG_STB_BOARD_DONGLE) && !defined(CONFIG_CHIP_TCC8925S)
    {   808000, 533000,      0,      0, 196000,      0,      0,      0, 118735 },
#else
    {   808000, 600000,      0,      0, 243040,      0,      0,      0, 118735 },
#endif
};

const static struct clock_table_t wfd_clktbls[] = {
    {   808000, 600000,      0,      0, 243040,      0,      0,      0, 118735 },
};

const static struct clock_table_t tsif_clktbls[] = {
    {   468750, 600000,      0,      0, 140000,      0,      0,      0, 118735 },
};

const static struct clock_table_t multi_vpu_clktbls[] = {
#if defined(CONFIG_STB_BOARD_DONGLE) && !defined(CONFIG_CHIP_TCC8925S)
    {        0, 533000,      0,      0, 196000, 277000, 277000,      0,      0 },
#else
    {        0, 600000,      0,      0, 243040, 296000, 296000,      0,      0 },
#endif
};

const static struct clock_table_t tv_clktbls[] = {
    {        0, 533000, 312000,      0,      0,      0,      0,      0,      0 },
};

#endif /* __CLOCKTABLE_LIST_H__ */
