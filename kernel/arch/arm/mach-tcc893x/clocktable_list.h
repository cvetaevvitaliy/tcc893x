/****************************************************************************
 * arch/arm/mach-tcc893x/clocktable_list.h
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
    /*     CPU     MEM     DDI     GPU     I/O    VBUS   VCORE    HSIO     SMU     G2D     CM3   Voltage    */
    {{  342960, 243750, 128077,  97155,  70848, 118081, 118081,  98401,  70848, 128077,  98401 },  900000},
    {{  424080, 300750, 160385, 134410,  88313, 147188, 147188, 122657,  88313, 160385, 122657 },  950000},
    {{  525480, 372000, 200769, 180979, 110143, 183572, 183572, 152976, 110143, 200769, 152976 }, 1000000},
    {{  626880, 443250, 241154, 227548, 131973, 219955, 219955, 183296, 131973, 241154, 183296 }, 1050000},
    {{  728320, 514500, 281538, 274117, 153803, 256339, 256339, 213616, 153803, 281538, 213616 }, 1100000},
    {{  819580, 578625, 317884, 316029, 173450, 289084, 289084, 240904, 173450, 317884, 240904 }, 1150000},
//  {{  850000, 600000, 330000, 330000, 180000, 300000, 300000, 250000, 180000, 330000, 250000 }, 1170000},  /* standard */
    {{  857000, 604000, 333542, 333542, 181914, 303191, 303191, 252659, 181914, 333542, 252659 }, 1175000},
//  {{  892000, 624000, 351255, 351255, 191489, 319149, 319149, 265957, 191489, 351255, 265957 }, 1200000},
//  {{  962000, 664000, 386680, 386680, 210638, 351064, 351064, 292554, 210638, 386680, 292554 }, 1250000},
//  {{ 1200000,      -,      -,      -,      -,      -,      -,      -,      -,      -,      - }, 1450000},  /* Not recommanded */
};

const static struct clock_table_t normal_clktbls[] = {
    /*     CPU     MEM     DDI     GPU     I/O    VBUS   VCORE    HSIO     SMU     G2D     CM3 */
    {   342000, 243750,      0,      0,  70848,      0,      0,      0, 110143,      0,      0 },   // -
    {   424000, 300750,      0,      0,  88313,      0,      0,      0, 110143,      0,      0 },   // -
    {   524000, 372000,      0,      0, 110143,      0,      0,      0, 110143,      0,      0 },   // 1.000V
    {   626000, 443250,      0,      0, 131973,      0,      0,      0, 110143,      0,      0 },   // 1.050V
    {   728000, 514500,      0,      0, 153803,      0,      0,      0, 110143,      0,      0 },   // 1.100V
//  {   819000, 578625,      0,      0, 173450,      0,      0,      0, 110143,      0,      0 },   // 1.150V
    {   850000, 600000,      0,      0, 180000,      0,      0,      0, 110143,      0,      0 },   // 1.175V
};

const static struct clock_table_t camera_clktbls[] = {
    {        0, 443250, 241154,      0,      0, 219955, 219955,      0,      0,      0,      0 },   // 1.050V
    {        0, 514500, 281538,      0,      0, 256339, 256339,      0,      0,      0,      0 },   // 1.100V
    {        0, 600000, 333542,      0,      0, 303191, 303191,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t isp_camera_clktbls[] = {
    {        0, 443250, 241154,      0,      0, 219955, 219955,      0,      0,      0,      0 },   // 1.050V
    {        0, 514500, 281538,      0,      0, 256339, 256339,      0,      0,      0,      0 },   // 1.100V
    {        0, 600000, 333542,      0,      0, 303191, 303191,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t hwc_ddi_clktbls[] = {  /* same with vpu_clktbl's ddi clock */
    {        0,      0, 241154,      0,      0,      0,      0,      0,      0,      0,      0 },   // 1.050V
    {        0,      0, 281538,      0,      0,      0,      0,      0,      0,      0,      0 },   // 1.100V
    {        0,      0, 333542,      0,      0,      0,      0,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t vpu_clktbls[] = {
    {        0, 372000, 200769,      0,      0, 183572, 183572,      0,      0,      0,      0 },   // 1.000V
    {        0, 514500, 281538,      0,      0, 256339, 256339,      0,      0,      0,      0 },   // 1.100V
    {        0, 600000, 333542,      0,      0, 303191, 303191,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t vpu_480p_clktbls[] = {
    {        0,      0,      0,      0, 110143,      0,      0,      0,      0,      0,      0 },   // * ~10Mbps, Core 1.000V
    {        0,      0,      0,      0, 131973,      0,      0,      0,      0,      0,      0 },   // 10~20Mbps, Core 1.050V
    {        0,      0,      0,      0, 153803,      0,      0,      0,      0,      0,      0 },   // 20~30Mbps, Core 1.100V
    {        0,      0,      0,      0, 173450,      0,      0,      0,      0,      0,      0 },   // 30Mbps ~ , Core 1.150V
};

const static struct clock_table_t vpu_720p_clktbls[] = {
    {        0,      0,      0,      0, 153803,      0,      0,      0,      0,      0,      0 },   // * ~10Mbps, Core 1.100V
    {        0,      0,      0,      0, 153803,      0,      0,      0,      0,      0,      0 },   // 10~20Mbps, Core 1.100V
    {        0,      0,      0,      0, 173450, 289084, 289084,      0,      0,      0,      0 },   // 20~30Mbps, Core 1.150V
    {        0, 600000, 333542,      0, 181914, 303191, 303191,      0,      0,      0,      0 },   // 30Mbps ~ , Core 1.175V
};

const static struct clock_table_t vpu_1080p_clktbls[] = {
    {   525480,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0 },   // * ~10Mbps, Core 1.000V
    {   626880,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0 },   // 10~20Mbps, Core 1.050V
    {   728320,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0 },   // 20~30Mbps, Core 1.100V
    {   850000,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0 },   // 30Mbps ~ , Core 1.175V
};

const static struct clock_table_t jpeg_clktbls[]= {
    {        0, 443250,      0,      0, 131973, 219955, 219955,      0,      0,      0,      0 },   // 1.050V
    {        0, 514500,      0,      0, 153803, 256339, 256339,      0,      0,      0,      0 },   // 1.100V
    {        0, 600000,      0,      0, 181914, 303191, 303191,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t hdmi_clktbls[] = {
    {        0, 600000, 333542,      0, 181914,      0,      0,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t mali_clktbls[] = {
#if defined(CONFIG_GPU_BUS_SCALING)
    {        0, 372000,      0, 180979,      0,      0,      0,      0,      0,      0,      0 },   // 1.000V
    {        0, 443250,      0, 227548,      0,      0,      0,      0,      0,      0,      0 },   // 1.050V
    {        0, 514500,      0, 274117,      0,      0,      0,      0,      0,      0,      0 },   // 1.100V
#endif
    {        0, 600000,      0, 333542,      0,      0,      0,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t fb_clktbls[] = {
    {        0,      0, 160385,      0, 110143,      0,      0,      0,      0,      0,      0 },   // 1.000V
};

const static struct clock_table_t overlay_clktbls[] = {
    {        0,      0,      0,      0,      0,      0,      0,      0,      0, 241154,      0 },   // 1.050V
};

const static struct clock_table_t bt_clktbls[] = {
    {        0, 600000,      0,      0, 181914,      0,      0,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t usb_clktbls[] = {
    {        0,      0,      0,      0, 110143,      0,      0,      0,      0,      0,      0 },   // 1.175V
    {        0, 600000,      0,      0, 181914,      0,      0,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t remocon_clktbls[] = {
    {        0,      0,      0,      0, 181914,      0,      0,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t hsio_clktbls[] = {
    {        0,      0,      0,      0,      0,      0,      0, 152976,      0,      0,      0 },   // 1.000V
    {        0, 600000,      0,      0,      0,      0,      0, 250000,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t voip_clktbls[] = {
    {        0, 514500,      0,      0, 153803,      0,      0,      0,      0,      0,      0 },   // 1.100V
    {        0, 600000,      0,      0, 181914,      0,      0,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t wfd_clktbls[] = {
    {   850000, 600000, 333542, 333542, 181914, 303191, 303191, 252659, 181914, 333542, 252659 },   // 1.175V
};

const static struct clock_table_t tsif_clktbls[] = {
    {        0, 600000,      0,      0, 181914,      0,      0,      0,      0,      0, 200000 },   // 1.175V
};

const static struct clock_table_t multi_vpu_clktbls[] = {
    {        0, 600000,      0,      0, 181914, 303191, 303191,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t smartcard_clktbls[] = {
    {        0, 600000,      0,      0, 181914,		0, 		0,      0,      0,      0,      0 },   // 1.175V
};

const static struct clock_table_t cipher_clktbls[] = {
    {        0, 600000,      0,      0, 	0,		0, 		0, 252659,      0,      0,      0 },   // 1.175V
};

#endif /* __CLOCKTABLE_LIST_H__ */
