/****************************************************************************
One line to give the program's name and a brief idea of what it does.
Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/


#ifndef __RDA5888FREQTBL_H__
#define __RDA5888FREQTBL_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "rda5888app.h"

// ---------------------------------------------------------------------------
// Default Channel Maps for USA, Canada, Korea, Taiwan, Mexico, 
// Chile, Venezuela, the Philippines
// Default Channel map for NTSC
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_ntsc_chn_map[];

// ---------------------------------------------------------------------------
// Default Channel Map for China
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_pald_chn_map[];

// ---------------------------------------------------------------------------
// Default Channel Map for ShenZhen
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_pald_sz_map[];

// ---------------------------------------------------------------------------
// Default Channel Map for Vietnam
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_vietnam_chn_map[];

// ---------------------------------------------------------------------------
// Default Channel Map for Western Europe, 
// Turkey, UAE, Afghanistan, Singapore, Thailand, 
// Cambodia, Indonesia, Malaysia, India,Pakistan,
// Portugal,Spain, and Laos
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_w_europe_chn_map[];

// ---------------------------------------------------------------------------
// Default Channel Map for Burma
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_burma_chn_map[];

// ---------------------------------------------------------------------------
// Default Channel Map for Japan
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_ntsc_japan_chn_map[];

// ---------------------------------------------------------------------------
// Default Channel Maps for UK, Hong Kong,
// and South Africa
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_a_hongkong_chn_map[];

// ---------------------------------------------------------------------------
// Default Channel Map for Brazil
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_brazil_chn_map[];

// ---------------------------------------------------------------------------
// Default Channel Map for Argentina
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_argentina_chn_map[];

// ---------------------------------------------------------------------------
// Default Channel Map for Egypt, Germany, Iraq, Iran 
//	Libya, Lebanon, SandiArabia, Tunisia.
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_secam_b_g_chn_map[]; 

// ---------------------------------------------------------------------------
// Default Channel Map for Russia, Bulgaria, Congo, 
//  Hungry, Poland, Romania.
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_secam_d_k_chn_map[];

// ---------------------------------------------------------------------------
// Default channel map for factory test.
// ---------------------------------------------------------------------------
extern rdamtv_channel_t rdamtv_factory_chn_map[];

extern rdamtv_channel_t rdamtv_test_chn_map[];
extern rdamtv_channel_t rdamtv_test1_chn_map[];
extern rdamtv_channel_t rdamtv_test2_chn_map[];
extern rdamtv_channel_t rdamtv_test_Hanzhou_chn_map[];
extern rdamtv_channel_t rdamtv_search_chn_map[];

#ifdef __cplusplus
}
#endif

#endif
