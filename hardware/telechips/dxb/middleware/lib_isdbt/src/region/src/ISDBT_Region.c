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


#include "ISDBT_Region.h"

#define	NULL_STR	((unsigned char *)"")
#ifndef	NULL
#define	NULL	0
#endif

int TCCDEMUX_GetISDBTFeature(unsigned int *feature);

/* ======================================================================
**  LOCAL VARIABLE
** ====================================================================== */

static unsigned char StringLanguageType = E_STR_LANG_JAPANESE;


/* ----------------------------------------------------------------------
**  String DB
** ---------------------------------------------------------------------- */
const unsigned char ISDBT_JPN_STR_UNDEFINED[] = { 0x4C, 0x24, 0x44, 0x6A, 0x35, 0x41, 0x00 };
const unsigned char ISDBT_JPN_STR_KANTO_WIDE_AREA[] = { 0x34, 0x58, 0x45, 0x6C, 0x39, 0x2D, 0x30, 0x68, 0x00 };
const unsigned char ISDBT_JPN_STR_KINKI_WIDE_AREA[] = { 0x36, 0x61, 0x35, 0x26, 0x39, 0x2D, 0x30, 0x68, 0x00 };
const unsigned char ISDBT_JPN_STR_CHUKYO_WIDE_AREA[] = { 0x43, 0x66, 0x35, 0x7E, 0x39, 0x2D, 0x30, 0x68, 0x00 };
const unsigned char ISDBT_JPN_STR_HOKKAIDO_AREA[] = { 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x3E, 0x6B, 0x00 };
const unsigned char ISDBT_JPN_STR_OKAYAMA_N_KAGAWA[] = { 0x32, 0x2C, 0x3B, 0x33, 0x39, 0x61, 0x40, 0x6E, 0x00 };
const unsigned char ISDBT_JPN_STR_SHIMANE_N_TOTTORI[] = { 0x45, 0x67, 0x3A, 0x2C, 0x44, 0x3B, 0x3C, 0x68, 0x00 };
const unsigned char ISDBT_JPN_STR_HOKKAIDO_SAPPORO[] = { 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x21, 0x4A, 0x3B, 0x25, 0x4B, 0x5A, 0x21, 0x4B, 0x00 };
const unsigned char ISDBT_JPN_STR_HOKKAIDO_HAKODATE[] = { 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x21, 0x4A, 0x48, 0x21, 0x34, 0x5B, 0x21, 0x4B, 0x00 };
const unsigned char ISDBT_JPN_STR_HOKKAIDO_ASAHIKAWA[] = { 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x21, 0x4A, 0x30, 0x30, 0x40, 0x6E, 0x21, 0x4B, 0x00 };
const unsigned char ISDBT_JPN_STR_HOKKAIDO_OBIHIRO[] = { 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x21, 0x4A, 0x42, 0x53, 0x39, 0x2D, 0x21, 0x4B, 0x00 };
const unsigned char ISDBT_JPN_STR_HOKKAIDO_KUSHIRO[] = { 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x21, 0x4A, 0x36, 0x7C, 0x4F, 0x29, 0x21, 0x4B, 0x00 };
const unsigned char ISDBT_JPN_STR_HOKKAIDO_KITAMI[] = { 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x21, 0x4A, 0x4B, 0x4C, 0x38, 0x2B, 0x21, 0x4B, 0x00 };
const unsigned char ISDBT_JPN_STR_HOKKAIDO_MURORAN[] = { 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x21, 0x4A, 0x3C, 0x3C, 0x4D, 0x76, 0x21, 0x4B, 0x00 };
const unsigned char ISDBT_JPN_STR_MIYAGI[] = { 0x35, 0x5C, 0x3E, 0x6B, 0x00 };
const unsigned char ISDBT_JPN_STR_AKITA[] = { 0x3D, 0x29, 0x45, 0x44, 0x00 };
const unsigned char ISDBT_JPN_STR_YAMAGATA[] = { 0x3B, 0x33, 0x37, 0x41, 0x00 };
const unsigned char ISDBT_JPN_STR_IWATE[] = { 0x34, 0x64, 0x3C, 0x6A, 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUSHIMA[] = { 0x4A, 0x21, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_AOMORI[] = { 0x40, 0x44, 0x3F, 0x39, 0x00 };
const unsigned char ISDBT_JPN_STR_TOKYO[] = { 0x45, 0x6C, 0x35, 0x7E, 0x00 };
const unsigned char ISDBT_JPN_STR_KANAGAWA[] = { 0x3F, 0x40, 0x46, 0x60, 0x40, 0x6E, 0x00 };
const unsigned char ISDBT_JPN_STR_GUNMA[] = { 0x37, 0x32, 0x47, 0x4F, 0x00 };
const unsigned char ISDBT_JPN_STR_IBARAKI[] = { 0x30, 0x71, 0x3E, 0x6B, 0x00 };
const unsigned char ISDBT_JPN_STR_CHIBA[] = { 0x40, 0x69, 0x4D, 0x55, 0x00 };
const unsigned char ISDBT_JPN_STR_TOCHIGI[] = { 0x46, 0x4A, 0x4C, 0x5A, 0x00 };
const unsigned char ISDBT_JPN_STR_SAITAMA[] = { 0x3A, 0x6B, 0x36, 0x4C, 0x00 };
const unsigned char ISDBT_JPN_STR_NAGANO[] = { 0x44, 0x39, 0x4C, 0x6E, 0x00 };
const unsigned char ISDBT_JPN_STR_NIIGATA[] = { 0x3F, 0x37, 0x33, 0x63, 0x00 };
const unsigned char ISDBT_JPN_STR_YAMANASHI[] = { 0x3B, 0x33, 0x4D, 0x7C, 0x00 };
const unsigned char ISDBT_JPN_STR_AICHI[] = { 0x30, 0x26, 0x43, 0x4E, 0x00 };
const unsigned char ISDBT_JPN_STR_ISHIKAWA[] = { 0x40, 0x50, 0x40, 0x6E, 0x00 };
const unsigned char ISDBT_JPN_STR_SHIZUOKA[] = { 0x40, 0x45, 0x32, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUI[] = { 0x4A, 0x21, 0x30, 0x66, 0x00 };
const unsigned char ISDBT_JPN_STR_TOYAMA[] = { 0x49, 0x59, 0x3B, 0x33, 0x00 };
const unsigned char ISDBT_JPN_STR_MIE[] = { 0x3B, 0x30, 0x3D, 0x45, 0x00 };
const unsigned char ISDBT_JPN_STR_GIFU[] = { 0x34, 0x74, 0x49, 0x6C, 0x00 };
const unsigned char ISDBT_JPN_STR_OSAKA[] = { 0x42, 0x67, 0x3A, 0x65, 0x00 };
const unsigned char ISDBT_JPN_STR_KYOTO[] = { 0x35, 0x7E, 0x45, 0x54, 0x00 };
const unsigned char ISDBT_JPN_STR_HYOGO[] = { 0x4A, 0x3C, 0x38, 0x4B, 0x00 };
const unsigned char ISDBT_JPN_STR_WAKAYAMA[] = { 0x4F, 0x42, 0x32, 0x4E, 0x3B, 0x33, 0x00 };
const unsigned char ISDBT_JPN_STR_NARA[] = { 0x46, 0x60, 0x4E, 0x49, 0x00 };
const unsigned char ISDBT_JPN_STR_SHIGA[] = { 0x3C, 0x22, 0x32, 0x6C, 0x00 };
const unsigned char ISDBT_JPN_STR_HIROSHIMA[] = { 0x39, 0x2D, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_OKAYAMA[] = { 0x32, 0x2C, 0x3B, 0x33, 0x00 };
const unsigned char ISDBT_JPN_STR_SHIMANE[] = { 0x45, 0x67, 0x3A, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_TOTTORI[] = { 0x44, 0x3B, 0x3C, 0x68, 0x00 };
const unsigned char ISDBT_JPN_STR_YAMAGUCHI[] = { 0x3B, 0x33, 0x38, 0x7D, 0x00 };
const unsigned char ISDBT_JPN_STR_EHIME[] = { 0x30, 0x26, 0x49, 0x32, 0x00 };
const unsigned char ISDBT_JPN_STR_KAGAWA[] = { 0x39, 0x61, 0x40, 0x6E, 0x00 };
const unsigned char ISDBT_JPN_STR_TOKUSHIMA[] = { 0x46, 0x41, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_KOUCHI[] = { 0x39, 0x62, 0x43, 0x4E, 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUOKA[] = { 0x4A, 0x21, 0x32, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_KUMAMOTO[] = { 0x37, 0x27, 0x4B, 0x5C, 0x00 };
const unsigned char ISDBT_JPN_STR_NAGASAKI[] = { 0x44, 0x39, 0x3A, 0x6A, 0x00 };
const unsigned char ISDBT_JPN_STR_KAGOSHIMA[] = { 0x3C, 0x2F, 0x3B, 0x79, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_MIYAZAKI[] = { 0x35, 0x5C, 0x3A, 0x6A, 0x00 };
const unsigned char ISDBT_JPN_STR_OITA[] = { 0x42, 0x67, 0x4A, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_SAGA[] = { 0x3A, 0x34, 0x32, 0x6C, 0x00 };
const unsigned char ISDBT_JPN_STR_OKINAWA[] = { 0x32, 0x2D, 0x46, 0x6C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NIPPON_TV_BROADCASTING_NETWORK[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_FUJI_TV_NETWORK_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TV_ASAHI_CORP[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_EDUCATIONAL[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_MAINICHI_BROADCASTING_SYSTEM[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_KANSAI_TELECASTING_CORP[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_YOMIURI_TELECASTING_CORP[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TOKAI_TV_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_CHUBU_NIPPON_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NAGOYA_BROADCASTING_NETWORK[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_CHUKYU_TV_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_HOKKAIDO_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_THE_SAPPORO_TELEVISION_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_HOKKAIDO_TELEVISION_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_HOKKAIDO_CULTURAL_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_HOKKAIDO_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NISHINIPPON_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SETONAIKAI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SANYO_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TV_SETOUCHI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_OKAYAMA_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SAIN_IN_CHUO_TELEVISION_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_BROADCASTING_SYSTEM_OF_SAN_IN_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TOUHOKU_BROADCASTING_COMPANY[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SENDAI_TELEVISION_INCORPORATED[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_MIYAGI_TELEVISION_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_HIGASHINIPPON_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_AKITA_BROADCASTING_SYSTEM[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_AKITA_TELEVISION_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_AKITA_ASAHI_BROADCASTING[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_YAMAGATA_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_YAMAGATA_TELEVISION_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TV_U_YAMAGATA_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SAKURANBO_TELEVISION_BROADCASTING_CORPORATION[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_IWATE_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_IWATE_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_IWATE_ASAHI_TV_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUSHIMA_TELEVISION_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUSHIMA_CENTRAL_TELEVISION_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUSHIMA_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TV_U_FUKUSHIMA_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_AOMORI_BROADCASTING_CORPORATION[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_AOMORI_TELEVISION_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TOKYO_METROPOLITAN_TELEVISION_BROADCASTING_CORP[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_KANAGAWA_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_GUNMA_TELEVISION_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_CHIBA_TELEVISION_BROADCASTING_CORP[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TOCHIGI_TELEVISION_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_SAITAMA_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TV_SHINSHU_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_ASAHI_BROADCASTINGNAGANO_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SHIN_ETSU_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NAGANO_BROADCASTING_SYSTEM_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_BROADCASTING_SYSTEM_OF_NIIGATA_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NIIGATA_SOGO_TELEVISION_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISON_NIIGATA_NETWORK_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_THE_NIIGATA_TELEVISION_NETWORK_21_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_YAMANASHI_BROADCASTING_SYSTEM[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_UHF_TELEVISION_YAMANASHI_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_AICHI_TELEVISION_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TV_KANAZAWA[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_HOKURIKU_ASAHI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_HOKURIKU_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_ISHIKAWA_TV_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SHIZUOKA_BROADCASTING_SYSTEM[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SHIZUOKA_TELECASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SHIZUOKA_DAIICHI_TELEVISION_CORPORATION[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUI_BROADCASTING_CORPORATION[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUI_TELEVISION_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_KITANIHON_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TOYAMA_TELEVISION_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TULIP_TELEVISION_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_MIE_TELEVISION_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_GIFU_BROADCASTING_SYSTEM_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISIO_OSAKA_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_KYOTO_BROADCASTING_SYSTEM_COMPANY_LIMITED[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_WAKAYAMA_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NARA_TELEVISION_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_BIWAKO_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_RCC_BROADCASTING_COMPANY[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_HIROSHIMA_TELECASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_HIROSHIMA_HOME_TELEVISION_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SHINHIROSHIMA_TELECASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_YAMAGUCHI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TV_YAMAGUCHI_BROADCASTING_SYSTEMS[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_YAMAGUCHI_ASAHI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NANKAI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_EHIME_ASAHI_TELEVISION_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_I_TELEVISION_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_EHIME_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SHIKOKU_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_KOCHI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_KOCHI_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_KOCHI_SUN_SUN_BROADCASTING_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_KYUSHU_ASAHI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_RKB_MAINICHI_BROADCASTING_CORPORATION[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUOKA_BROADCASTING_SYSTEM[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TVQ_KYUSHU_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_NISHINIPPON_CORPORATION[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TV_KUMAMOTO_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_KUMAMOTO_KENMIN_TV[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_KUMAMOTO_ASAHI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NAGASAKI_BROADCASTING_COMPANY[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_K_K_TELEVISION_NAGASAKI[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NAGASAKI_CULTURE_TELECASTING_CORPORATION[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NAGASAKI_INTERNATIONAL_TELEVISION_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_MINAMINIHON_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_KAGOSHIMA_TELEVISION_STATION_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_KAGOSHIMA_BROADCASTING_CORPORATION[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_KAGOSHIMA_YOMIURI_TELEVISION[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_MIYAZAKI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_MIYAZAKI_TELECASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_OITA_BROADCASTING_SYSTEM_INC[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_OITA_SYSTEM[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_OITA_ASAHI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_SAGA_TELEVISION_STATION_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_RYUKYU_BROADCASTING_CORPORATION[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_RYUKYU_ASAHI_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_OKINAWA_TELEVISION_BROADCASTING_CO_LTD[] = { 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_TOKYO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x45, 0x6C, 0x35, 0x7E, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_TOKYO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x45, 0x6C, 0x35, 0x7E, 0x00 };
const unsigned char ISDBT_JPN_STR_NIPPON_TV[] = { 0x46, 0x7C, 0x4B, 0x5C, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_TBS[] = { 0x23, 0x54, 0x23, 0x42, 0x23, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_FUJI_TV[] = { 0x25, 0x55, 0x25, 0x38, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x25, 0x38, 0x25, 0x67, 0x25, 0x73, 0x00 };
const unsigned char ISDBT_JPN_STR_TV_ASAHI[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x44, 0x2B, 0x46, 0x7C, 0x00 };
const unsigned char ISDBT_JPN_STR_TV_TOKYO[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x45, 0x6C, 0x35, 0x7E, 0x00 };
const unsigned char ISDBT_JPN_STR_THE_UNIVERSITY_OF_THE_AIR[] = { 0x4A, 0x7C, 0x41, 0x77, 0x42, 0x67, 0x33, 0x58, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_OSAKA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x42, 0x67, 0x3A, 0x65, 0x00 };
const unsigned char ISDBT_JPN_STR_MBS_MAINICHI_BROADCASTING[] = { 0x23, 0x4D, 0x23, 0x42, 0x23, 0x53, 0x4B, 0x68, 0x46, 0x7C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_ABC_TELEVISION[] = { 0x23, 0x41, 0x23, 0x42, 0x23, 0x43, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_KANSAI_TELECASTING[] = { 0x34, 0x58, 0x40, 0x3E, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_YOMIURI_TELECASTING[] = { 0x46, 0x49, 0x47, 0x64, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_NAGOYA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x4C, 0x3E, 0x38, 0x45, 0x32, 0x30, 0x00 };
const unsigned char ISDBT_JPN_STR_TOKAI_TV[] = { 0x45, 0x6C, 0x33, 0x24, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_CBC[] = { 0x23, 0x43, 0x23, 0x42, 0x23, 0x43, 0x00 };
const unsigned char ISDBT_JPN_STR_MEE_TELE[] = { 0x25, 0x61, 0x21, 0x41 /*0x7F, 0x5E*/, 0x25, 0x46, 0x25, 0x6C, 0x00 };
const unsigned char ISDBT_JPN_STR_CHUKYO_TV[] = { 0x43, 0x66, 0x35, 0x7E, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_HBC_HOKKAIDO_BROADCASTING[] = { 0x23, 0x48, 0x23, 0x42, 0x23, 0x43, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_STV_SAPPORO_TELEVISION[] = { 0x23, 0x53, 0x23, 0x54, 0x23, 0x56, 0x3B, 0x25, 0x4B, 0x5A, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_HTB_HOKKAIDO_TELEVISION[] = { 0x23, 0x48, 0x23, 0x54, 0x23, 0x42, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_UHB[] = { 0x23, 0x55, 0x23, 0x48, 0x23, 0x42, 0x00 };
const unsigned char ISDBT_JPN_STR_TVH[] = { 0x23, 0x54, 0x23, 0x56, 0x23, 0x48, 0x00 };
const unsigned char ISDBT_JPN_STR_RNC_NISHINIPPON_TELEVISION[] = { 0x23, 0x52, 0x23, 0x4E, 0x23, 0x43, 0x40, 0x3E, 0x46, 0x7C, 0x4B, 0x5C, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_KSB_SETONAIKAI_BROADCASTING[] = { 0x23, 0x4B, 0x23, 0x53, 0x23, 0x42, 0x40, 0x25, 0x38, 0x4D, 0x46, 0x62, 0x33, 0x24, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_RSK_TELEVISION[] = { 0x23, 0x52, 0x23, 0x53, 0x23, 0x4B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_SETOUCHI[] = { 0x23, 0x54, 0x23, 0x53, 0x23, 0x43, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x1B, 0x7D, 0xBB, 0xC8, 0xA6, 0xC1, 0x00 };
const unsigned char ISDBT_JPN_STR_OHK_TV[] = { 0x23, 0x4F, 0x23, 0x48, 0x23, 0x4B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_SAN_IN_CHUO_TELEVISION[] = { 0x3B, 0x33, 0x31, 0x22, 0x43, 0x66, 0x31, 0x7B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_BSS_TELEVISION[] = { 0x23, 0x42, 0x23, 0x53, 0x23, 0x53, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NIHONKAI_TELEVISION[] = { 0x46, 0x7C, 0x4B, 0x5C, 0x33, 0x24, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_SAPPORO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3B, 0x25, 0x4B, 0x5A, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_SAPPORO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x3B, 0x25, 0x4B, 0x5A, 0x00 };
const unsigned char ISDBT_JPN_STR_HBC_SAPPORO[] = { 0x23, 0x48, 0x23, 0x42, 0x23, 0x43, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_STV_SAPPORO[] = { 0x23, 0x53, 0x23, 0x54, 0x23, 0x56, 0x3B, 0x25, 0x4B, 0x5A, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_HTB_SAPPORO[] = { 0x23, 0x48, 0x23, 0x54, 0x23, 0x42, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_UHB_SAPPORO[] = { 0x23, 0x55, 0x23, 0x48, 0x23, 0x42, 0x00 };
const unsigned char ISDBT_JPN_STR_TVH_SAPPORO[] = { 0x23, 0x54, 0x23, 0x56, 0x23, 0x48, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_HAKODATE[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x48, 0x21, 0x34, 0x5B, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_HAKODATE[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x48, 0x21, 0x34, 0x5B, 0x00 };
const unsigned char ISDBT_JPN_STR_HBC_HAKODATE[] = { 0x23, 0x48, 0x23, 0x42, 0x23, 0x43, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_STV_HAKODATE[] = { 0x23, 0x53, 0x23, 0x54, 0x23, 0x56, 0x3B, 0x25, 0x4B, 0x5A, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_HTB_HAKODATE[] = { 0x23, 0x48, 0x23, 0x54, 0x23, 0x42, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_UHB_HAKODATE[] = { 0x23, 0x55, 0x23, 0x48, 0x23, 0x42, 0x00 };
const unsigned char ISDBT_JPN_STR_TVH_HAKODATE[] = { 0x23, 0x54, 0x23, 0x56, 0x23, 0x48, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_ASAHIKAWA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x30, 0x30, 0x40, 0x6E, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_ASAHIKAWA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x30, 0x30, 0x40, 0x6E, 0x00 };
const unsigned char ISDBT_JPN_STR_HBC_ASAHIKAWA[] = { 0x23, 0x48, 0x23, 0x42, 0x23, 0x43, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_STV_ASAHIKAWA[] = { 0x23, 0x53, 0x23, 0x54, 0x23, 0x56, 0x3B, 0x25, 0x4B, 0x5A, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_HTB_ASAHIKAWA[] = { 0x23, 0x48, 0x23, 0x54, 0x23, 0x42, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_UHB_ASAHIKAWA[] = { 0x23, 0x55, 0x23, 0x48, 0x23, 0x42, 0x00 };
const unsigned char ISDBT_JPN_STR_TVH_ASAHIKAWA[] = { 0x23, 0x54, 0x23, 0x56, 0x23, 0x48, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_OBIHIRO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x42, 0x53, 0x39, 0x2D, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_OBIHIRO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x42, 0x53, 0x39, 0x2D, 0x00 };
const unsigned char ISDBT_JPN_STR_HBC_OBIHIRO[] = { 0x23, 0x48, 0x23, 0x42, 0x23, 0x43, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_STV_OBIHIRO[] = { 0x23, 0x53, 0x23, 0x54, 0x23, 0x56, 0x3B, 0x25, 0x4B, 0x5A, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_HTB_OBIHIRO[] = { 0x23, 0x48, 0x23, 0x54, 0x23, 0x42, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_UHB_OBIHIRO[] = { 0x23, 0x55, 0x23, 0x48, 0x23, 0x42, 0x00 };
const unsigned char ISDBT_JPN_STR_TVH_OBIHIRO[] = { 0x23, 0x54, 0x23, 0x56, 0x23, 0x48, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_KUSHIRO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x36, 0x7C, 0x4F, 0x29, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_KUSHIRO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x36, 0x7C, 0x4F, 0x29, 0x00 };
const unsigned char ISDBT_JPN_STR_HBC_KUSHIRO[] = { 0x23, 0x48, 0x23, 0x42, 0x23, 0x43, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_STV_KUSHIRO[] = { 0x23, 0x53, 0x23, 0x54, 0x23, 0x56, 0x3B, 0x25, 0x4B, 0x5A, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_HTB_KUSHIRO[] = { 0x23, 0x48, 0x23, 0x54, 0x23, 0x42, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_UHB_KUSHIRO[] = { 0x23, 0x55, 0x23, 0x48, 0x23, 0x42, 0x00 };
const unsigned char ISDBT_JPN_STR_TVH_KUSHIRO[] = { 0x23, 0x54, 0x23, 0x56, 0x23, 0x48, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_KITAMI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x4B, 0x4C, 0x38, 0x2B, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_KITAMI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x4B, 0x4C, 0x38, 0x2B, 0x00 };
const unsigned char ISDBT_JPN_STR_HBC_KITAMI[] = { 0x23, 0x48, 0x23, 0x42, 0x23, 0x43, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_STV_KITAMI[] = { 0x23, 0x53, 0x23, 0x54, 0x23, 0x56, 0x3B, 0x25, 0x4B, 0x5A, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_HTB_KITAMI[] = { 0x23, 0x48, 0x23, 0x54, 0x23, 0x42, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_UHB_KITAMI[] = { 0x23, 0x55, 0x23, 0x48, 0x23, 0x42, 0x00 };
const unsigned char ISDBT_JPN_STR_TVH_KITAMI[] = { 0x23, 0x54, 0x23, 0x56, 0x23, 0x48, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_MURORAN[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3C, 0x3C, 0x4D, 0x76, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_MURORAN[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x3C, 0x3C, 0x4D, 0x76, 0x00 };
const unsigned char ISDBT_JPN_STR_HBC_MURORAN[] = { 0x23, 0x48, 0x23, 0x42, 0x23, 0x43, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_STV_MURORAN[] = { 0x23, 0x53, 0x23, 0x54, 0x23, 0x56, 0x3B, 0x25, 0x4B, 0x5A, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_HTB_MURORAN[] = { 0x23, 0x48, 0x23, 0x54, 0x23, 0x42, 0x4B, 0x4C, 0x33, 0x24, 0x46, 0x3B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_UHB_MURORAN[] = { 0x23, 0x55, 0x23, 0x48, 0x23, 0x42, 0x00 };
const unsigned char ISDBT_JPN_STR_TVH_MURORAN[] = { 0x23, 0x54, 0x23, 0x56, 0x23, 0x48, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_SENDAI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x40, 0x67, 0x42, 0x66, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_SENDAI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x40, 0x67, 0x42, 0x66, 0x00 };
const unsigned char ISDBT_JPN_STR_TBC_TELEVISION[] = { 0x23, 0x54, 0x23, 0x42, 0x23, 0x43, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_SENDAI_TELEVISION[] = { 0x40, 0x67, 0x42, 0x66, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_MIYAGI_TELEVISION[] = { 0x25, 0x5F, 0x25, 0x64, 0x25, 0x2E, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_KHB_HIGASHINIPPON_BROADCASTING[] = { 0x23, 0x4B, 0x23, 0x48, 0x23, 0x42, 0x45, 0x6C, 0x46, 0x7C, 0x4B, 0x5C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_AKITA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3D, 0x29, 0x45, 0x44, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_AKITA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x3D, 0x29, 0x45, 0x44, 0x00 };
const unsigned char ISDBT_JPN_STR_ABS_AKITA_BROADCASTING[] = { 0x23, 0x41, 0x23, 0x42, 0x23, 0x53, 0x3D, 0x29, 0x45, 0x44, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_AKT_AKITA_TELEVISION[] = { 0x23, 0x41, 0x23, 0x4B, 0x23, 0x54, 0x3D, 0x29, 0x45, 0x44, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_AAB_AKITA_ASAHI_BROADCASTING[] = { 0x23, 0x41, 0x23, 0x41, 0x23, 0x42, 0x3D, 0x29, 0x45, 0x44, 0x44, 0x2B, 0x46, 0x7C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_YAMAGATA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3B, 0x33, 0x37, 0x41, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_YAMAGATA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x3B, 0x33, 0x37, 0x41, 0x00 };
const unsigned char ISDBT_JPN_STR_YBC_YAMAGATA_BROADCASTING[] = { 0x23, 0x59, 0x23, 0x42, 0x23, 0x43, 0x3B, 0x33, 0x37, 0x41, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_YTS_YAMAGATA_TELEVISION[] = { 0x23, 0x59, 0x23, 0x54, 0x23, 0x53, 0x3B, 0x33, 0x37, 0x41, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_TV_U_YAMAGATA[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x25, 0x66, 0x1B, 0x7D, 0xF9, 0x3B, 0x33, 0x37, 0x41, 0x00 };
const unsigned char ISDBT_JPN_STR_SAKURANBO_TELEVISION[] = { 0x1B, 0x7D, 0xB5, 0xAF, 0xE9, 0xF3, 0xDC, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_MORIOKA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x40, 0x39, 0x32, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_MORIOKA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x40, 0x39, 0x32, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_IBC_TELEVISION[] = { 0x23, 0x49, 0x23, 0x42, 0x23, 0x43, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_IWATE[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x34, 0x64, 0x3C, 0x6A, 0x00 };
const unsigned char ISDBT_JPN_STR_MENKOI_TELEVISION[] = { 0x1B, 0x7D, 0xE1, 0xF3, 0xB3, 0xA4, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_IWATE_ASAHI_TELEVISION[] = { 0x34, 0x64, 0x3C, 0x6A, 0x44, 0x2B, 0x46, 0x7C, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_FUKUSHIMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x4A, 0x21, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_FUKUSHIMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x4A, 0x21, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUSHIMA_TELEVISION[] = { 0x4A, 0x21, 0x45, 0x67, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUSHIMA_CENTRAL_TELEVISIO[] = { 0x4A, 0x21, 0x45, 0x67, 0x43, 0x66, 0x31, 0x7B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_KFB_FUKUSHIMA_BROADCASTING[] = { 0x23, 0x4B, 0x23, 0x46, 0x23, 0x42, 0x4A, 0x21, 0x45, 0x67, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_TV_U_FUKUSHIMA[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x25, 0x66, 0x1B, 0x7D, 0xF9, 0x4A, 0x21, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_AOMORI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x40, 0x44, 0x3F, 0x39, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_AOMORI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x40, 0x44, 0x3F, 0x39, 0x00 };
const unsigned char ISDBT_JPN_STR_RAB_AOMORI_BROADCASTING[] = { 0x23, 0x52, 0x23, 0x41, 0x23, 0x42, 0x40, 0x44, 0x3F, 0x39, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_ATV_AOMORI_TELEVISION[] = { 0x23, 0x41, 0x23, 0x54, 0x23, 0x56, 0x40, 0x44, 0x3F, 0x39, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_ASAHI_BROADCASTING_AOMORI[] = { 0x40, 0x44, 0x3F, 0x39, 0x44, 0x2B, 0x46, 0x7C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_TOKYO_MX_TELEVISION[] = { 0x23, 0x54, 0x23, 0x4F, 0x23, 0x4B, 0x23, 0x59, 0x23, 0x4F, 0x21, 0x21, 0x23, 0x4D, 0x23, 0x58, 00 };
const unsigned char ISDBT_JPN_STR_TVK[] = { 0x23, 0x74, 0x23, 0x76, 0x23, 0x6B, 0x00 };
const unsigned char ISDBT_JPN_STR_GUNMA_TELEVISION[] = { 0x37, 0x32, 0x47, 0x4F, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_MITO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3F, 0x65, 0x38, 0x4D, 0x00 };
const unsigned char ISDBT_JPN_STR_CHIBA_TELEVISION[] = { 0x25, 0x41, 0x25, 0x50, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_TOCHIGI_TELEVISION[] = { 0x1B, 0x7D, 0xC8, 0xC1, 0xAE, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_TELETAMA[] = { 0x25, 0x46, 0x25, 0x6C, 0x36, 0x4C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_NAGANO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x44, 0x39, 0x4C, 0x6E, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_NAGANO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x44, 0x39, 0x4C, 0x6E, 0x00 };
const unsigned char ISDBT_JPN_STR_TV_SHINSHU[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x3F, 0x2E, 0x3D, 0x23, 0x00 };
const unsigned char ISDBT_JPN_STR_ABN_ASAHI_BROADCASTING_NAGANO[] = { 0x23, 0x61, 0x23, 0x62, 0x23, 0x6E, 0x44, 0x39, 0x4C, 0x6E, 0x44, 0x2B, 0x46, 0x7C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_SBC_SHIN_ETSU_BROADCASTING[] = { 0x23, 0x53, 0x23, 0x42, 0x23, 0x43, 0x3F, 0x2E, 0x31, 0x5B, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_NBS_NAGANO_BROADCASTING[] = { 0x23, 0x4E, 0x23, 0x42, 0x23, 0x53, 0x44, 0x39, 0x4C, 0x6E, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_NIIGATA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3F, 0x37, 0x33, 0x63, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_NIIGATA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x3F, 0x37, 0x33, 0x63, 0x00 };
const unsigned char ISDBT_JPN_STR_BSN[] = { 0x23, 0x42, 0x23, 0x53, 0x23, 0x4E, 0x00 };
const unsigned char ISDBT_JPN_STR_NST[] = { 0x23, 0x4E, 0x23, 0x53, 0x23, 0x54, 0x00 };
const unsigned char ISDBT_JPN_STR_TENY_TELEVISION_NIIGATA[] = { 0x23, 0x54, 0x23, 0x65, 0x23, 0x4E, 0x23, 0x59, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x3F, 0x37, 0x33, 0x63, 0x00 };
const unsigned char ISDBT_JPN_STR_NIIGATA_TELEVISION_21[] = { 0x3F, 0x37, 0x33, 0x63, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x23, 0x32, 0x23, 0x31, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_KOFU[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x39, 0x43, 0x49, 0x5C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_KOFU[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x39, 0x43, 0x49, 0x5C, 0x00 };
const unsigned char ISDBT_JPN_STR_YBS_YAMANASHI_BROADCASTING[] = { 0x23, 0x59, 0x23, 0x42, 0x23, 0x53, 0x3B, 0x33, 0x4D, 0x7C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_UTY[] = { 0x23, 0x55, 0x23, 0x54, 0x23, 0x59, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_NAGOYA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x4C, 0x3E, 0x38, 0x45, 0x32, 0x30, 0x00 };
const unsigned char ISDBT_JPN_STR_AICHI_TELEVISION[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x30, 0x26, 0x43, 0x4E, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_KANAZAWA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x36, 0x62, 0x42, 0x74, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_KANAZAWA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x36, 0x62, 0x42, 0x74, 0x00 };
const unsigned char ISDBT_JPN_STR_TVKANAZAWA[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x36, 0x62, 0x42, 0x74, 0x00 };
const unsigned char ISDBT_JPN_STR_HOKURIKU_ASAHI_BROADCASTING[] = { 0x4B, 0x4C, 0x4E, 0x26, 0x44, 0x2B, 0x46, 0x7C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_MRO[] = { 0x23, 0x4D, 0x23, 0x52, 0x23, 0x4F, 0x00 };
const unsigned char ISDBT_JPN_STR_ISHIKAWA_TV[] = { 0x40, 0x50, 0x40, 0x6E, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_SHIZUOKA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x40, 0x45, 0x32, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_SHIZUOKA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x40, 0x45, 0x32, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_SBS[] = { 0x23, 0x53, 0x23, 0x42, 0x23, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_SHIZUOKA_TELECASTING[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x40, 0x45, 0x32, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_SHIZUOKA_DAIICHI_TELEVISIN[] = { 0x24, 0x40, 0x24, 0x24, 0x24,0x24, 0x24, 0x41, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_SHIZUOKA_ASAHI_TV[] = { 0x40, 0x45, 0x32, 0x2C, 0x44, 0x2B, 0x46, 0x7C, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_FUKUI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x4A, 0x21, 0x30, 0x66, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_FUKUI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x4A, 0x21, 0x30, 0x66, 0x00 };
const unsigned char ISDBT_JPN_STR_FBC_TELEVISION[] = { 0x23, 0x46, 0x23, 0x42, 0x23, 0x43, 0x00 };
const unsigned char ISDBT_JPN_STR_FUKUI_TELEVISION[] = { 0x4A, 0x21, 0x30, 0x66, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_TOYAMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x49, 0x59, 0x3B, 0x33, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_TOYAMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x49, 0x59, 0x3B, 0x33, 0x00 };
const unsigned char ISDBT_JPN_STR_KNB_KITANIHON_BROADCASTING[] = { 0x23, 0x4B, 0x23, 0x4E, 0x23, 0x42, 0x4B, 0x4C, 0x46, 0x7C, 0x4B, 0x5C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_BBT_TOYAMA_TELEVISION[] = { 0x23, 0x42, 0x23, 0x42, 0x23, 0x54, 0x49, 0x59, 0x3B, 0x33, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_TULIP_TELEVISION[] = { 0x25, 0x41, 0x25, 0x65, 0x1B, 0x7D, 0xF9, 0x25, 0x6A, 0x25, 0x43, 0x25, 0x57, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_TSU[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x44, 0x45, 0x00 };
const unsigned char ISDBT_JPN_STR_MIE_TELEVISION[] = { 0x3B, 0x30, 0x3D, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_GIFU[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x34, 0x74, 0x49, 0x6C, 0x00 };
const unsigned char ISDBT_JPN_STR_GIFU_TELEVISION[] = { 0x24, 0x2E, 0x24, 0x55, 0x25, 0x41, 0x25, 0x63, 0x25,0x73, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_OSAKA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x42, 0x67, 0x3A, 0x65, 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_OSAKA[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x42, 0x67, 0x3A, 0x65, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_KYOTO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x35, 0x7E, 0x45, 0x54, 0x00 };
const unsigned char ISDBT_JPN_STR_KBS_KYOTO[] = { 0x23, 0x4B, 0x23, 0x42, 0x23, 0x53, 0x35, 0x7E, 0x45, 0x54, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_KOBE[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3F, 0x40, 0x38, 0x4D, 0x00 };
const unsigned char ISDBT_JPN_STR_SUN_TELEVISION[] = { 0x25, 0x35, 0x25, 0x73, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_WAKAYAMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x4F, 0x42, 0x32, 0x4E, 0x3B, 0x33, 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_WAKAYAMA[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x4F, 0x42, 0x32, 0x4E, 0x3B, 0x33, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_NARA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x46, 0x60, 0x4E, 0x49, 0x00 };
const unsigned char ISDBT_JPN_STR_NARA_TELEVISION[] = { 0x46, 0x60, 0x4E, 0x49, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_OTSU[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x42, 0x67, 0x44, 0x45, 0x00 };
const unsigned char ISDBT_JPN_STR_BBC_BIWAKO_BROADCASTING[] = { 0x23, 0x42, 0x23, 0x42, 0x23, 0x43, 0x1B, 0x7D, 0xD3, 0xEF, 0x38, 0x50, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_HIROSHIMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x39, 0x2D, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_HIROSHIMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x39, 0x2D, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_RCC_TELEVISION[] = { 0x23, 0x52, 0x23, 0x43, 0x23, 0x43, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_HIROSHIMA_TELEVISION[] = { 0x39, 0x2D, 0x45, 0x67, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_HIROSHIMA_HOME_TELEVISION[] = { 0x39, 0x2D, 0x45, 0x67, 0x25, 0x5B, 0x1B, 0x7D, 0xF9, 0x25, 0x60, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_TSS[] = { 0x23, 0x54, 0x23, 0x53, 0x23, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_OKAYAMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x32, 0x2C, 0x3B, 0x33, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_OKAYAMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x32, 0x2C, 0x3B, 0x33, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_MATSUE[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3E, 0x3E, 0x39, 0x3E, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_MATSUE[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x3E, 0x3E, 0x39, 0x3E, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_TOTTORI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x44, 0x3B, 0x3C, 0x68, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_TOTTORI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x44, 0x3B, 0x3C, 0x68, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_YAMAGUCHI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3B, 0x33, 0x38, 0x7D, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_YAMAGUCHI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x3B, 0x33, 0x38, 0x7D, 0x00 };
const unsigned char ISDBT_JPN_STR_KRY_YAMAGUCHI_BROADCASTING[] = { 0x23, 0x4B, 0x23, 0x52, 0x23, 0x59, 0x3B, 0x33, 0x38, 0x7D, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_TYS_TV_YAMAGUCHI[] = { 0x23, 0x74, 0x23, 0x79, 0x23, 0x73, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x3B, 0x33, 0x38, 0x7D, 0x00 };
const unsigned char ISDBT_JPN_STR_YAB_YAMAGUCHI_ASAHI[] = { 0x23, 0x79, 0x23, 0x61, 0x23, 0x62, 0x3B, 0x33, 0x38, 0x7D, 0x44, 0x2B, 0x46, 0x7C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_MATSUYAMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3E, 0x3E, 0x3B, 0x33, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_MATSUYAMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x3E, 0x3E, 0x3B, 0x33, 0x00 };
const unsigned char ISDBT_JPN_STR_NANKAI_BROADCASTING[] = { 0x46, 0x6E, 0x33, 0x24, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_EHIME_ASAHI_TELEVISION[] = { 0x30, 0x26, 0x49, 0x32, 0x44, 0x2B, 0x46, 0x7C, 0x00 };
const unsigned char ISDBT_JPN_STR_I_TELEVISION[] = { 0x1B, 0x7D, 0xA2, 0xA4, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_EHIME[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x30, 0x26, 0x49, 0x32, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_TAKAMATSU[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x39, 0x62, 0x3E, 0x3E, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_TAKAMATSU[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x39, 0x62, 0x3E, 0x3E, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_TOKUSHIMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x46, 0x41, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_TOKUSHIMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x46, 0x41, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_SHIKOKU_BROADCATING[] = { 0x3B, 0x4D, 0x39, 0x71, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_KOUCHI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x39, 0x62, 0x43, 0x4E, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_KOUCHI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x39, 0x62, 0x43, 0x4E, 0x00 };
const unsigned char ISDBT_JPN_STR_KOCHI_BROADCASTING[] = { 0x39, 0x62, 0x43, 0x4E, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_TELEVISION_KOCHI[] = { 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x39, 0x62, 0x43, 0x4E, 0x00 };
const unsigned char ISDBT_JPN_STR_SUN_SUN_TELEVISION[] = { 0x1B, 0x7D, 0xB5, 0xF3, 0xB5, 0xF3, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_FUKUOKA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x4A, 0x21, 0x32, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_KITAKYUSHU[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x4B, 0x4C, 0x36, 0x65, 0x3D, 0x23, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_FUKUOKA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x4A, 0x21, 0x32, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_KITAKYUSHU[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x4B, 0x4C, 0x36, 0x65, 0x3D, 0x23, 0x00 };
const unsigned char ISDBT_JPN_STR_KBC_KYUSHU_ASAHI_BROADCASTING[] = { 0x23, 0x4B, 0x23, 0x42, 0x23, 0x43, 0x36, 0x65, 0x3D, 0x23, 0x44, 0x2B, 0x46, 0x7C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_RKB_MAINICHI_BROADCASTING[] = { 0x23, 0x52, 0x23, 0x4B, 0x23, 0x42, 0x4B, 0x68, 0x46, 0x7C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_FBS_FUKUOKA_BROADCASTING[] = { 0x23, 0x46, 0x23, 0x42, 0x23, 0x53, 0x4A, 0x21, 0x32, 0x2C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_TVQ_KYUSHU_BROADCASTING[] = { 0x23, 0x54, 0x23, 0x56, 0x23, 0x51, 0x36, 0x65, 0x3D, 0x23, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_TNC_TELEVISION_NISHINIPPON[] = { 0x23, 0x54, 0x23, 0x4E, 0x23, 0x43, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x40, 0x3E, 0x46, 0x7C, 0x4B, 0x5C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_KUMAMOTO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x37, 0x27, 0x4B, 0x5C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_KUMAMOTO[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x37, 0x27, 0x4B, 0x5C, 0x00 };
const unsigned char ISDBT_JPN_STR_RKK_KUMAMOTO_BROADCASTING[] = { 0x23, 0x52, 0x23, 0x4B, 0x23, 0x4B, 0x37, 0x27, 0x4B, 0x5C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_TKU_TV_KUMAMOTO[] = { 0x23, 0x54, 0x23, 0x4B, 0x23, 0x55, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x37, 0x27, 0x4B, 0x5C, 0x00 };
const unsigned char ISDBT_JPN_STR_KKT_KUMAMATO_KENMIN[] = { 0x23, 0x4B, 0x23, 0x4B, 0x23, 0x54, 0x1B, 0x7D, 0xAF, 0xDE, 0xE2, 0xC8, 0x38, 0x29, 0x4C, 0x31, 0x00 };
const unsigned char ISDBT_JPN_STR_KAB_KUMAMOTO_ASAHI_BROADCASTING[] = { 0x23, 0x4B, 0x23, 0x41, 0x23, 0x42, 0x37, 0x27, 0x4B, 0x5C, 0x44, 0x2B, 0x46, 0x7C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_NAGASAKI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x44, 0x39, 0x3A, 0x6A, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_NAGASAKI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x44, 0x39, 0x3A, 0x6A, 0x00 };
const unsigned char ISDBT_JPN_STR_NBC_NAGASAKI_BROADCASTING[] = { 0x23, 0x4E, 0x23, 0x42, 0x23, 0x43, 0x44, 0x39, 0x3A, 0x6A, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_KTN_TELEVISION_NAGASAKI[] = { 0x23, 0x4B, 0x23, 0x54, 0x23, 0x4E, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x44, 0x39, 0x3A, 0x6A, 0x00 };
const unsigned char ISDBT_JPN_STR_NCC_NAGASAKI_CULTURE_TELECASTING[] = { 0x23, 0x4E, 0x23, 0x43, 0x23, 0x43, 0x44, 0x39, 0x3A, 0x6A, 0x4A, 0x38, 0x32, 0x3D, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_NIB_NAGASAKI_INTERNATIONAL_TELEVISION[] = { 0x23, 0x4E, 0x23, 0x49, 0x23, 0x42, 0x44, 0x39, 0x3A, 0x6A, 0x39, 0x71, 0x3A, 0x5D, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_KAGOSHIMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3C, 0x2F, 0x3B, 0x79, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_KAGOSHIMA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x3C, 0x2F, 0x3B, 0x79, 0x45, 0x67, 0x00 };
const unsigned char ISDBT_JPN_STR_MBC_MINAMINIHON_BROADCASTING[] = { 0x23, 0x4D, 0x23, 0x42, 0x23, 0x43, 0x46, 0x6E, 0x46, 0x7C, 0x4B, 0x5C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_KTS_KAGOSHIMA_TELEVISION[] = { 0x23, 0x4B, 0x23, 0x54, 0x23, 0x53, 0x3C, 0x2F, 0x3B, 0x79, 0x45, 0x67, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_KKB_KAGOSHIMA_BROADCASTING[] = { 0x23, 0x4B, 0x23, 0x4B, 0x23, 0x42, 0x3C, 0x2F, 0x3B, 0x79, 0x45, 0x67, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_KYT_KAGOSHIMA_YOMIURI_TELEVISION[] = { 0x23, 0x4B, 0x23, 0x59, 0x23, 0x54, 0x3C, 0x2F, 0x3B, 0x79, 0x45, 0x67, 0x46, 0x49, 0x47, 0x64, 0x23, 0x54, 0x23, 0x56, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_MIYAZAKI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x35, 0x5C, 0x3A, 0x6A, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_MIYAZAKI[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x35, 0x5C, 0x3A, 0x6A, 0x00 };
const unsigned char ISDBT_JPN_STR_MRT_MIYAZAKI_BROADCASTING[] = { 0x23, 0x4D, 0x23, 0x52, 0x23, 0x54, 0x35, 0x5C, 0x3A, 0x6A, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_UMK_MIYAZAKI_TELECASTING[] = { 0x23, 0x55, 0x23, 0x4D, 0x23, 0x4B, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x35, 0x5C, 0x3A, 0x6A, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_OITA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x42, 0x67, 0x4A, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_OITA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x42, 0x67, 0x4A, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_OBS_OITA_BROADCASTING[] = { 0x23, 0x4F, 0x23, 0x42, 0x23, 0x53, 0x42, 0x67, 0x4A, 0x2C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_TOS_TELEVISION_OITA[] = { 0x23, 0x54, 0x23, 0x4F, 0x23, 0x53, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x42, 0x67, 0x4A, 0x2C, 0x00 };
const unsigned char ISDBT_JPN_STR_OAB_OITA_ASAHI_BROADCASTING[] = { 0x23, 0x4F, 0x23, 0x41, 0x23, 0x42, 0x42, 0x67, 0x4A, 0x2C, 0x44, 0x2B, 0x46, 0x7C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_SAGA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x3A, 0x34, 0x32, 0x6C, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_SAGA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x3A, 0x34, 0x32, 0x6C, 0x00 };
const unsigned char ISDBT_JPN_STR_STS_SAGA_TELEVISION[] = { 0x23, 0x53, 0x23, 0x54, 0x23, 0x53, 0x25, 0x35, 0x25, 0x2C, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_G_NAHA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x41, 0x6D, 0x39, 0x67, 0x21, 0x26, 0x46, 0x61, 0x47, 0x46, 0x00 };
const unsigned char ISDBT_JPN_STR_NHK_E_NAHA[] = { 0x23, 0x4E, 0x23, 0x48, 0x23, 0x4B, 0x23, 0x45, 0x25, 0x46, 0x25, 0x6C, 0x46, 0x61, 0x47, 0x46, 0x00 };
const unsigned char ISDBT_JPN_STR_RBC_RYUKYU_BROADCASTING[] = { 0x23, 0x52, 0x23, 0x42, 0x23, 0x43, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x00 };
const unsigned char ISDBT_JPN_STR_QAB_RYUKYU_ASAHI_BROADCASTING[] = { 0x23, 0x51, 0x23, 0x41, 0x23, 0x42, 0x4E, 0x30, 0x35, 0x65, 0x44, 0x2B, 0x46, 0x7C, 0x4A, 0x7C, 0x41, 0x77, 0x00 };
const unsigned char ISDBT_JPN_STR_OKINAWA_TELEVISION_OTV[] = { 0x32, 0x2D, 0x46, 0x6C, 0x25, 0x46, 0x25, 0x6C, 0x25, 0x53, 0x21, 0x4A, 0x23, 0x4F, 0x23, 0x54, 0x23, 0x56, 0x21, 0x4B, 0x00 };


const T_STRING_INFO	stISDBT_StringDB[E_ISDBT_STR_ID_MAX] =
{
	{ E_ISDBT_STR_ID_NONE, { NULL_STR, NULL_STR } },

	/* ----------------------------------------------------------------------
	**  Region Name
	*/
	{ E_ISDBT_STR_ID_UNDEFINED, { (unsigned char *)"undefined", (unsigned char *)ISDBT_JPN_STR_UNDEFINED } },
	{ E_ISDBT_STR_ID_KANTO_WIDE_AREA, { (unsigned char *)"Kanto wide-area", (unsigned char *)ISDBT_JPN_STR_KANTO_WIDE_AREA } },
	{ E_ISDBT_STR_ID_KINKI_WIDE_AREA, { (unsigned char *)"Kinki wide-area", (unsigned char *)ISDBT_JPN_STR_KINKI_WIDE_AREA } },
	{ E_ISDBT_STR_ID_CHUKYO_WIDE_AREA, { (unsigned char *)"Chukyo wide-area", (unsigned char *)ISDBT_JPN_STR_CHUKYO_WIDE_AREA } },
	{ E_ISDBT_STR_ID_HOKKAIDO_AREA, { (unsigned char *)"Hokkaido area", (unsigned char *)ISDBT_JPN_STR_HOKKAIDO_AREA } },
	{ E_ISDBT_STR_ID_OKAYAMA_N_KAGAWA, { (unsigned char *)"Okayama, Kagawa", (unsigned char *)ISDBT_JPN_STR_OKAYAMA_N_KAGAWA } },
	{ E_ISDBT_STR_ID_SHIMANE_N_TOTTORI, { (unsigned char *)"Shimane, Tottori", (unsigned char *)ISDBT_JPN_STR_SHIMANE_N_TOTTORI } },
	{ E_ISDBT_STR_ID_HOKKAIDO_SAPPORO, { (unsigned char *)"Hokkaido (Sapporo)", (unsigned char *)ISDBT_JPN_STR_HOKKAIDO_SAPPORO } },
	{ E_ISDBT_STR_ID_HOKKAIDO_HAKODATE, { (unsigned char *)"Hokkaido (Hakodate)", (unsigned char *)ISDBT_JPN_STR_HOKKAIDO_HAKODATE } },
	{ E_ISDBT_STR_ID_HOKKAIDO_ASAHIKAWA, { (unsigned char *)"Hokkaido (Asahikawa)", (unsigned char *)ISDBT_JPN_STR_HOKKAIDO_ASAHIKAWA } },
	{ E_ISDBT_STR_ID_HOKKAIDO_OBIHIRO, { (unsigned char *)"Hokkaido (Obihiro)", (unsigned char *)ISDBT_JPN_STR_HOKKAIDO_OBIHIRO } },
	{ E_ISDBT_STR_ID_HOKKAIDO_KUSHIRO, { (unsigned char *)"Hokkaido (Kushiro)", (unsigned char *)ISDBT_JPN_STR_HOKKAIDO_KUSHIRO } },
	{ E_ISDBT_STR_ID_HOKKAIDO_KITAMI, { (unsigned char *)"Hokkaido (Kitami)", (unsigned char *)ISDBT_JPN_STR_HOKKAIDO_KITAMI } },
	{ E_ISDBT_STR_ID_HOKKAIDO_MURORAN, { (unsigned char *)"Hokkaido (Muroran)", (unsigned char *)ISDBT_JPN_STR_HOKKAIDO_MURORAN } },
	{ E_ISDBT_STR_ID_MIYAGI, { (unsigned char *)"Miyagi", (unsigned char *)ISDBT_JPN_STR_MIYAGI } },
	{ E_ISDBT_STR_ID_AKITA, { (unsigned char *)"Akita", (unsigned char *)ISDBT_JPN_STR_AKITA } },
	{ E_ISDBT_STR_ID_YAMAGATA, { (unsigned char *)"Yamagata", (unsigned char *)ISDBT_JPN_STR_YAMAGATA } },
	{ E_ISDBT_STR_ID_IWATE, { (unsigned char *)"Iwate", (unsigned char *)ISDBT_JPN_STR_IWATE } },
	{ E_ISDBT_STR_ID_FUKUSHIMA, { (unsigned char *)"Fukushima", (unsigned char *)ISDBT_JPN_STR_FUKUSHIMA } },
	{ E_ISDBT_STR_ID_AOMORI, { (unsigned char *)"Aomori", (unsigned char *)ISDBT_JPN_STR_AOMORI } },
	{ E_ISDBT_STR_ID_TOKYO, { (unsigned char *)"Tokyo", (unsigned char *)ISDBT_JPN_STR_TOKYO } },
	{ E_ISDBT_STR_ID_KANAGAWA, {(unsigned char *)"Kanagawa", (unsigned char *)ISDBT_JPN_STR_KANAGAWA } },
	{ E_ISDBT_STR_ID_GUNMA, { (unsigned char *)"Gunma", (unsigned char *)ISDBT_JPN_STR_GUNMA } },
	{ E_ISDBT_STR_ID_IBARAKI, { (unsigned char *)"Ibaraki", (unsigned char *)ISDBT_JPN_STR_IBARAKI } },
	{ E_ISDBT_STR_ID_CHIBA, { (unsigned char *)"Chiba", (unsigned char *)ISDBT_JPN_STR_CHIBA } },
	{ E_ISDBT_STR_ID_TOCHIGI, { (unsigned char *)"Tochigi", (unsigned char *)ISDBT_JPN_STR_TOCHIGI } },
	{ E_ISDBT_STR_ID_SAITAMA, { (unsigned char *)"Saitama", (unsigned char *)ISDBT_JPN_STR_SAITAMA } },
	{ E_ISDBT_STR_ID_NAGANO, { (unsigned char *)"Nagano", (unsigned char *)ISDBT_JPN_STR_NAGANO } },
	{ E_ISDBT_STR_ID_NIIGATA, { (unsigned char *)"Niigata", (unsigned char *)ISDBT_JPN_STR_NIIGATA } },
	{ E_ISDBT_STR_ID_YAMANASHI, { (unsigned char *)"Yamanashi", (unsigned char *)ISDBT_JPN_STR_YAMANASHI } },
	{ E_ISDBT_STR_ID_AICHI, { (unsigned char *)"Aichi", (unsigned char *)ISDBT_JPN_STR_AICHI } },
	{ E_ISDBT_STR_ID_ISHIKAWA, { (unsigned char *)"Ishikawa", (unsigned char *)ISDBT_JPN_STR_ISHIKAWA } },
	{ E_ISDBT_STR_ID_SHIZUOKA, { (unsigned char *)"Shizuoka", (unsigned char *)ISDBT_JPN_STR_SHIZUOKA } },
	{ E_ISDBT_STR_ID_FUKUI, { (unsigned char *)"Fukui", (unsigned char *)ISDBT_JPN_STR_FUKUI } },
	{ E_ISDBT_STR_ID_TOYAMA, { (unsigned char *)"Toyama", (unsigned char *)ISDBT_JPN_STR_TOYAMA } },
	{ E_ISDBT_STR_ID_MIE, { (unsigned char *)"Mie", (unsigned char *)ISDBT_JPN_STR_MIE } },
	{ E_ISDBT_STR_ID_GIFU, { (unsigned char *)"Gifu", (unsigned char *)ISDBT_JPN_STR_GIFU } },
	{ E_ISDBT_STR_ID_OSAKA, { (unsigned char *)"Osaka", (unsigned char *)ISDBT_JPN_STR_OSAKA } },
	{ E_ISDBT_STR_ID_KYOTO, { (unsigned char *)"Kyoto", (unsigned char *)ISDBT_JPN_STR_KYOTO } },
	{ E_ISDBT_STR_ID_HYOGO, { (unsigned char *)"Hyogo", (unsigned char *)ISDBT_JPN_STR_HYOGO } },
	{ E_ISDBT_STR_ID_WAKAYAMA, {(unsigned char *)"Wakayama", (unsigned char *)ISDBT_JPN_STR_WAKAYAMA } },
	{ E_ISDBT_STR_ID_NARA, { (unsigned char *)"Nara", (unsigned char *)ISDBT_JPN_STR_NARA } },
	{ E_ISDBT_STR_ID_SHIGA, { (unsigned char *)"Shiga", (unsigned char *)ISDBT_JPN_STR_SHIGA } },
	{ E_ISDBT_STR_ID_HIROSHIMA, { (unsigned char *)"Hiroshima", (unsigned char *)ISDBT_JPN_STR_HIROSHIMA } },
	{ E_ISDBT_STR_ID_OKAYAMA, { (unsigned char *)"Okayama", (unsigned char *)ISDBT_JPN_STR_OKAYAMA } },
	{ E_ISDBT_STR_ID_SHIMANE, { (unsigned char *)"Shimane", (unsigned char *)ISDBT_JPN_STR_SHIMANE } },
	{ E_ISDBT_STR_ID_TOTTORI, { (unsigned char *)"Tottori", (unsigned char *)ISDBT_JPN_STR_TOTTORI } },
	{ E_ISDBT_STR_ID_YAMAGUCHI, { (unsigned char *)"Yamaguchi", (unsigned char *)ISDBT_JPN_STR_YAMAGUCHI } },
	{ E_ISDBT_STR_ID_EHIME, { (unsigned char *)"Ehime", (unsigned char *)ISDBT_JPN_STR_EHIME } },
	{ E_ISDBT_STR_ID_KAGAWA, { (unsigned char *)"Kagawa", (unsigned char *)ISDBT_JPN_STR_KAGAWA } },
	{ E_ISDBT_STR_ID_TOKUSHIMA, { (unsigned char *)"Tokushima", (unsigned char *)ISDBT_JPN_STR_TOKUSHIMA } },
	{ E_ISDBT_STR_ID_KOUCHI, { (unsigned char *)"Kouchi", (unsigned char *)ISDBT_JPN_STR_KOUCHI } },
	{ E_ISDBT_STR_ID_FUKUOKA, { (unsigned char *)"Fukuoka", (unsigned char *)ISDBT_JPN_STR_FUKUOKA } },
	{ E_ISDBT_STR_ID_KUMAMOTO, { (unsigned char *)"Kumamoto", (unsigned char *)ISDBT_JPN_STR_KUMAMOTO } },
	{ E_ISDBT_STR_ID_NAGASAKI, { (unsigned char *)"Nagasaki", (unsigned char *)ISDBT_JPN_STR_NAGASAKI } },
	{ E_ISDBT_STR_ID_KAGOSHIMA, { (unsigned char *)"Kagoshima", (unsigned char *)ISDBT_JPN_STR_KAGOSHIMA } },
	{ E_ISDBT_STR_ID_MIYAZAKI, { (unsigned char *)"Miyazaki", (unsigned char *)ISDBT_JPN_STR_MIYAZAKI } },
	{ E_ISDBT_STR_ID_OITA, { (unsigned char *)"Oita", (unsigned char *)ISDBT_JPN_STR_OITA } },
	{ E_ISDBT_STR_ID_SAGA, {(unsigned char *)"Saga", (unsigned char *)ISDBT_JPN_STR_SAGA } },
	{ E_ISDBT_STR_ID_OKINAWA, { (unsigned char *)"Okinawa", (unsigned char *)ISDBT_JPN_STR_OKINAWA } },

	/* ----------------------------------------------------------------------
	**  Broadcaster Name
	*/
	{ E_ISDBT_STR_ID_NHK_G, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G } },
	{ E_ISDBT_STR_ID_NHK_E, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E } },
	{ E_ISDBT_STR_ID_NIPPON_TV_BROADCASTING_NETWORK, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NIPPON_TV_BROADCASTING_NETWORK } },
	//{ E_ISDBT_STR_ID_TBS, { NULL_STR, NULL_STR } },
	{ E_ISDBT_STR_ID_FUJI_TV_NETWORK_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FUJI_TV_NETWORK_INC } },
	{ E_ISDBT_STR_ID_TV_ASAHI_CORP, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_ASAHI_CORP } },
	//{ E_ISDBT_STR_ID_TV_TOKYO, { NULL_STR, NULL_STR } },
	//{ E_ISDBT_STR_ID_THE_UNIVERSITY_OF_THE_AIR, { NULL_STR, NULL_STR } },
	{ E_ISDBT_STR_ID_NHK_EDUCATIONAL, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_EDUCATIONAL } },
	{ E_ISDBT_STR_ID_MAINICHI_BROADCASTING_SYSTEM, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MAINICHI_BROADCASTING_SYSTEM } },
	{ E_ISDBT_STR_ID_KANSAI_TELECASTING_CORP, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KANSAI_TELECASTING_CORP } },
	{ E_ISDBT_STR_ID_YOMIURI_TELECASTING_CORP, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_YOMIURI_TELECASTING_CORP } },
	{ E_ISDBT_STR_ID_TOKAI_TV_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TOKAI_TV_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_CHUBU_NIPPON_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_CHUBU_NIPPON_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_NAGOYA_BROADCASTING_NETWORK, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NAGOYA_BROADCASTING_NETWORK } },
	{ E_ISDBT_STR_ID_CHUKYU_TV_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_CHUKYU_TV_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_HOKKAIDO_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HOKKAIDO_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_THE_SAPPORO_TELEVISION_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_THE_SAPPORO_TELEVISION_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_HOKKAIDO_TELEVISION_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HOKKAIDO_TELEVISION_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_HOKKAIDO_CULTURAL_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HOKKAIDO_CULTURAL_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_TELEVISION_HOKKAIDO_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_HOKKAIDO_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_NISHINIPPON_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NISHINIPPON_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_SETONAIKAI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SETONAIKAI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_SANYO_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SANYO_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_TV_SETOUCHI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_SETOUCHI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_OKAYAMA_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_OKAYAMA_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_SAIN_IN_CHUO_TELEVISION_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SAIN_IN_CHUO_TELEVISION_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_BROADCASTING_SYSTEM_OF_SAN_IN_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_BROADCASTING_SYSTEM_OF_SAN_IN_INC } },
	{ E_ISDBT_STR_ID_TOUHOKU_BROADCASTING_COMPANY, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TOUHOKU_BROADCASTING_COMPANY } },
	{ E_ISDBT_STR_ID_SENDAI_TELEVISION_INCORPORATED, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SENDAI_TELEVISION_INCORPORATED } },
	{ E_ISDBT_STR_ID_MIYAGI_TELEVISION_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MIYAGI_TELEVISION_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_HIGASHINIPPON_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HIGASHINIPPON_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_AKITA_BROADCASTING_SYSTEM, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_AKITA_BROADCASTING_SYSTEM } },
	{ E_ISDBT_STR_ID_AKITA_TELEVISION_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_AKITA_TELEVISION_CO_LTD } },
	{ E_ISDBT_STR_ID_AKITA_ASAHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_AKITA_ASAHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_YAMAGATA_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_YAMAGATA_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_YAMAGATA_TELEVISION_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_YAMAGATA_TELEVISION_CO_LTD } },
	{ E_ISDBT_STR_ID_TV_U_YAMAGATA_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_U_YAMAGATA_INC } },
	{ E_ISDBT_STR_ID_SAKURANBO_TELEVISION_BROADCASTING_CORPORATION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SAKURANBO_TELEVISION_BROADCASTING_CORPORATION } },
	{ E_ISDBT_STR_ID_IWATE_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_IWATE_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_TELEVISION_IWATE_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_IWATE_CO_LTD } },
	{ E_ISDBT_STR_ID_IWATE_ASAHI_TV_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_IWATE_ASAHI_TV_CO_LTD } },
	{ E_ISDBT_STR_ID_FUKUSHIMA_TELEVISION_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FUKUSHIMA_TELEVISION_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_FUKUSHIMA_CENTRAL_TELEVISION_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FUKUSHIMA_CENTRAL_TELEVISION_CO_LTD } },
	{ E_ISDBT_STR_ID_FUKUSHIMA_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FUKUSHIMA_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_TV_U_FUKUSHIMA_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_U_FUKUSHIMA_INC } },
	{ E_ISDBT_STR_ID_AOMORI_BROADCASTING_CORPORATION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_AOMORI_BROADCASTING_CORPORATION } },
	{ E_ISDBT_STR_ID_AOMORI_TELEVISION_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_AOMORI_TELEVISION_BROADCASTING_CO_LTD } },
	//{ E_ISDBT_STR_ID_ASAHI_BROADCASTING_AOMORI, { NULL_STR, NULL_STR } },
	{ E_ISDBT_STR_ID_TOKYO_METROPOLITAN_TELEVISION_BROADCASTING_CORP, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TOKYO_METROPOLITAN_TELEVISION_BROADCASTING_CORP } },
	{ E_ISDBT_STR_ID_TELEVISION_KANAGAWA_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_KANAGAWA_INC } },
	{ E_ISDBT_STR_ID_GUNMA_TELEVISION_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_GUNMA_TELEVISION_CO_LTD } },
	{ E_ISDBT_STR_ID_CHIBA_TELEVISION_BROADCASTING_CORP, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_CHIBA_TELEVISION_BROADCASTING_CORP } },
	{ E_ISDBT_STR_ID_TOCHIGI_TELEVISION_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TOCHIGI_TELEVISION_CO_LTD } },
	{ E_ISDBT_STR_ID_TELEVISION_SAITAMA_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_SAITAMA_CO_LTD } },
	{ E_ISDBT_STR_ID_TV_SHINSHU_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_SHINSHU_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_ASAHI_BROADCASTINGNAGANO_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_ASAHI_BROADCASTINGNAGANO_CO_LTD } },
	{ E_ISDBT_STR_ID_SHIN_ETSU_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SHIN_ETSU_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_NAGANO_BROADCASTING_SYSTEM_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NAGANO_BROADCASTING_SYSTEM_INC } },
	{ E_ISDBT_STR_ID_BROADCASTING_SYSTEM_OF_NIIGATA_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_BROADCASTING_SYSTEM_OF_NIIGATA_INC } },
	{ E_ISDBT_STR_ID_NIIGATA_SOGO_TELEVISION_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NIIGATA_SOGO_TELEVISION_INC } },
	{ E_ISDBT_STR_ID_TELEVISON_NIIGATA_NETWORK_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISON_NIIGATA_NETWORK_CO_LTD } },
	{ E_ISDBT_STR_ID_THE_NIIGATA_TELEVISION_NETWORK_21_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_THE_NIIGATA_TELEVISION_NETWORK_21_INC } },
	{ E_ISDBT_STR_ID_YAMANASHI_BROADCASTING_SYSTEM, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_YAMANASHI_BROADCASTING_SYSTEM } },
	{ E_ISDBT_STR_ID_UHF_TELEVISION_YAMANASHI_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_UHF_TELEVISION_YAMANASHI_INC } },
	{ E_ISDBT_STR_ID_AICHI_TELEVISION_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_AICHI_TELEVISION_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_TV_KANAZAWA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_KANAZAWA } },
	{ E_ISDBT_STR_ID_HOKURIKU_ASAHI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HOKURIKU_ASAHI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_HOKURIKU_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HOKURIKU_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_ISHIKAWA_TV_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_ISHIKAWA_TV_CO_LTD } },
	{ E_ISDBT_STR_ID_SHIZUOKA_BROADCASTING_SYSTEM, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SHIZUOKA_BROADCASTING_SYSTEM } },
	{ E_ISDBT_STR_ID_SHIZUOKA_TELECASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SHIZUOKA_TELECASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_SHIZUOKA_DAIICHI_TELEVISION_CORPORATION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SHIZUOKA_DAIICHI_TELEVISION_CORPORATION } },
	//{ E_ISDBT_STR_ID_SHIZUOKA_ASAHI_TV, { NULL_STR, NULL_STR } },
	{ E_ISDBT_STR_ID_FUKUI_BROADCASTING_CORPORATION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FUKUI_BROADCASTING_CORPORATION } },
	{ E_ISDBT_STR_ID_FUKUI_TELEVISION_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FUKUI_TELEVISION_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_KITANIHON_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KITANIHON_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_TOYAMA_TELEVISION_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TOYAMA_TELEVISION_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_TULIP_TELEVISION_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TULIP_TELEVISION_CO_LTD } },
	{ E_ISDBT_STR_ID_MIE_TELEVISION_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MIE_TELEVISION_CO_LTD } },
	{ E_ISDBT_STR_ID_GIFU_BROADCASTING_SYSTEM_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_GIFU_BROADCASTING_SYSTEM_INC } },
	{ E_ISDBT_STR_ID_TELEVISIO_OSAKA_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISIO_OSAKA_INC } },
	{ E_ISDBT_STR_ID_KYOTO_BROADCASTING_SYSTEM_COMPANY_LIMITED, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KYOTO_BROADCASTING_SYSTEM_COMPANY_LIMITED } },
	{ E_ISDBT_STR_ID_TELEVISION_WAKAYAMA_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_WAKAYAMA_CO_LTD } },
	{ E_ISDBT_STR_ID_NARA_TELEVISION_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NARA_TELEVISION_CO_LTD } },
	{ E_ISDBT_STR_ID_BIWAKO_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_BIWAKO_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_RCC_BROADCASTING_COMPANY, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_RCC_BROADCASTING_COMPANY } },
	{ E_ISDBT_STR_ID_HIROSHIMA_TELECASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HIROSHIMA_TELECASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_HIROSHIMA_HOME_TELEVISION_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HIROSHIMA_HOME_TELEVISION_CO_LTD } },
	{ E_ISDBT_STR_ID_SHINHIROSHIMA_TELECASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SHINHIROSHIMA_TELECASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_YAMAGUCHI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_YAMAGUCHI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_TV_YAMAGUCHI_BROADCASTING_SYSTEMS, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_YAMAGUCHI_BROADCASTING_SYSTEMS } },
	{ E_ISDBT_STR_ID_YAMAGUCHI_ASAHI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_YAMAGUCHI_ASAHI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_NANKAI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NANKAI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_EHIME_ASAHI_TELEVISION_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_EHIME_ASAHI_TELEVISION_INC } },
	{ E_ISDBT_STR_ID_I_TELEVISION_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_I_TELEVISION_INC } },
	{ E_ISDBT_STR_ID_EHIME_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_EHIME_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_SHIKOKU_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SHIKOKU_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_KOCHI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KOCHI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_TELEVISION_KOCHI_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_KOCHI_CO_LTD } },
	{ E_ISDBT_STR_ID_KOCHI_SUN_SUN_BROADCASTING_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KOCHI_SUN_SUN_BROADCASTING_INC } },
	{ E_ISDBT_STR_ID_KYUSHU_ASAHI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KYUSHU_ASAHI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_RKB_MAINICHI_BROADCASTING_CORPORATION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_RKB_MAINICHI_BROADCASTING_CORPORATION } },
	{ E_ISDBT_STR_ID_FUKUOKA_BROADCASTING_SYSTEM, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FUKUOKA_BROADCASTING_SYSTEM } },
	{ E_ISDBT_STR_ID_TVQ_KYUSHU_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVQ_KYUSHU_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_TELEVISION_NISHINIPPON_CORPORATION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_NISHINIPPON_CORPORATION } },
	//{ E_ISDBT_STR_ID_RKK_KUMAMOTO_BROADCASTING, { NULL_STR, NULL_STR } },
	{ E_ISDBT_STR_ID_TV_KUMAMOTO_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_KUMAMOTO_CO_LTD } },
	{ E_ISDBT_STR_ID_KUMAMOTO_KENMIN_TV, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KUMAMOTO_KENMIN_TV } },
	{ E_ISDBT_STR_ID_KUMAMOTO_ASAHI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KUMAMOTO_ASAHI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_NAGASAKI_BROADCASTING_COMPANY, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NAGASAKI_BROADCASTING_COMPANY } },
	{ E_ISDBT_STR_ID_K_K_TELEVISION_NAGASAKI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_K_K_TELEVISION_NAGASAKI } },
	{ E_ISDBT_STR_ID_NAGASAKI_CULTURE_TELECASTING_CORPORATION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NAGASAKI_CULTURE_TELECASTING_CORPORATION } },
	{ E_ISDBT_STR_ID_NAGASAKI_INTERNATIONAL_TELEVISION_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NAGASAKI_INTERNATIONAL_TELEVISION_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_MINAMINIHON_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MINAMINIHON_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_KAGOSHIMA_TELEVISION_STATION_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KAGOSHIMA_TELEVISION_STATION_CO_LTD } },
	{ E_ISDBT_STR_ID_KAGOSHIMA_BROADCASTING_CORPORATION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KAGOSHIMA_BROADCASTING_CORPORATION } },
	{ E_ISDBT_STR_ID_KAGOSHIMA_YOMIURI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KAGOSHIMA_YOMIURI_TELEVISION } },
	{ E_ISDBT_STR_ID_MIYAZAKI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MIYAZAKI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_MIYAZAKI_TELECASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MIYAZAKI_TELECASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_OITA_BROADCASTING_SYSTEM_INC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_OITA_BROADCASTING_SYSTEM_INC } },
	{ E_ISDBT_STR_ID_TELEVISION_OITA_SYSTEM, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_OITA_SYSTEM } },
	{ E_ISDBT_STR_ID_OITA_ASAHI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_OITA_ASAHI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_SAGA_TELEVISION_STATION_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SAGA_TELEVISION_STATION_CO_LTD } },
	{ E_ISDBT_STR_ID_RYUKYU_BROADCASTING_CORPORATION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_RYUKYU_BROADCASTING_CORPORATION } },
	{ E_ISDBT_STR_ID_RYUKYU_ASAHI_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_RYUKYU_ASAHI_BROADCASTING_CO_LTD } },
	{ E_ISDBT_STR_ID_OKINAWA_TELEVISION_BROADCASTING_CO_LTD, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_OKINAWA_TELEVISION_BROADCASTING_CO_LTD } },

	/* ----------------------------------------------------------------------
	**  TS Name (Full-width within 10 characters)
	*/
	{ E_ISDBT_STR_ID_NHK_G_TOKYO, { (unsigned char *)"NHK G, Tokyo", (unsigned char *)ISDBT_JPN_STR_NHK_G_TOKYO } },
	{ E_ISDBT_STR_ID_NHK_E_TOKYO, { (unsigned char *)"NHK E, Tokyo", (unsigned char *)ISDBT_JPN_STR_NHK_E_TOKYO } },
	{ E_ISDBT_STR_ID_NIPPON_TV, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NIPPON_TV } },
	{ E_ISDBT_STR_ID_TBS, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TBS } },
	{ E_ISDBT_STR_ID_FUJI_TV, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FUJI_TV } },
	{ E_ISDBT_STR_ID_TV_ASAHI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_ASAHI } },
	{ E_ISDBT_STR_ID_TV_TOKYO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_TOKYO } },
	{ E_ISDBT_STR_ID_THE_UNIVERSITY_OF_THE_AIR, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_THE_UNIVERSITY_OF_THE_AIR } },
	{ E_ISDBT_STR_ID_NHK_E_OSAKA, { (unsigned char *)"NHK E, Osaka", (unsigned char *)ISDBT_JPN_STR_NHK_E_OSAKA } },
	{ E_ISDBT_STR_ID_MBS_MAINICHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MBS_MAINICHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_ABC_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_ABC_TELEVISION } },
	{ E_ISDBT_STR_ID_KANSAI_TELECASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KANSAI_TELECASTING } },
	{ E_ISDBT_STR_ID_YOMIURI_TELECASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_YOMIURI_TELECASTING } },
	{ E_ISDBT_STR_ID_NHK_E_NAGOYA, { (unsigned char *)"NHK E, Nagoya", (unsigned char *)ISDBT_JPN_STR_NHK_E_NAGOYA } },
	{ E_ISDBT_STR_ID_TOKAI_TV, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TOKAI_TV } },
	{ E_ISDBT_STR_ID_CBC, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_CBC } },
	{ E_ISDBT_STR_ID_MEE_TELE, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MEE_TELE } },
	{ E_ISDBT_STR_ID_CHUKYO_TV, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_CHUKYO_TV } },
	{ E_ISDBT_STR_ID_HBC_HOKKAIDO_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HBC_HOKKAIDO_BROADCASTING } },
	{ E_ISDBT_STR_ID_STV_SAPPORO_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_STV_SAPPORO_TELEVISION } },
	{ E_ISDBT_STR_ID_HTB_HOKKAIDO_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HTB_HOKKAIDO_TELEVISION } },
	{ E_ISDBT_STR_ID_UHB, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_UHB } },
	{ E_ISDBT_STR_ID_TVH, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVH } },
	{ E_ISDBT_STR_ID_RNC_NISHINIPPON_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_RNC_NISHINIPPON_TELEVISION } },
	{ E_ISDBT_STR_ID_KSB_SETONAIKAI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KSB_SETONAIKAI_BROADCASTING } },
	{ E_ISDBT_STR_ID_RSK_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_RSK_TELEVISION } },
	{ E_ISDBT_STR_ID_TELEVISION_SETOUCHI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_SETOUCHI } },
	{ E_ISDBT_STR_ID_OHK_TV, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_OHK_TV } },
	{ E_ISDBT_STR_ID_SAN_IN_CHUO_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SAN_IN_CHUO_TELEVISION } },
	{ E_ISDBT_STR_ID_BSS_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_BSS_TELEVISION } },
	{ E_ISDBT_STR_ID_NIHONKAI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NIHONKAI_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_SAPPORO, { (unsigned char *)"NHK G, Sapporo", (unsigned char *)ISDBT_JPN_STR_NHK_G_SAPPORO } },
	{ E_ISDBT_STR_ID_NHK_E_SAPPORO, { (unsigned char *)"NHK E, Sapporo", (unsigned char *)ISDBT_JPN_STR_NHK_E_SAPPORO } },
	{ E_ISDBT_STR_ID_HBC_SAPPORO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HBC_SAPPORO } },
	{ E_ISDBT_STR_ID_STV_SAPPORO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_STV_SAPPORO } },
	{ E_ISDBT_STR_ID_HTB_SAPPORO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HTB_SAPPORO } },
	{ E_ISDBT_STR_ID_UHB_SAPPORO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_UHB_SAPPORO } },
	{ E_ISDBT_STR_ID_TVH_SAPPORO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVH_SAPPORO } },
	{ E_ISDBT_STR_ID_NHK_G_HAKODATE, { (unsigned char *)"NHK G, Hakodate", (unsigned char *)ISDBT_JPN_STR_NHK_G_HAKODATE } },
	{ E_ISDBT_STR_ID_NHK_E_HAKODATE, { (unsigned char *)"NHK E, Hakodate", (unsigned char *)ISDBT_JPN_STR_NHK_E_HAKODATE } },
	{ E_ISDBT_STR_ID_HBC_HAKODATE, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HBC_HAKODATE } },
	{ E_ISDBT_STR_ID_STV_HAKODATE, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_STV_HAKODATE } },
	{ E_ISDBT_STR_ID_HTB_HAKODATE, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HTB_HAKODATE } },
	{ E_ISDBT_STR_ID_UHB_HAKODATE, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_UHB_HAKODATE } },
	{ E_ISDBT_STR_ID_TVH_HAKODATE, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVH_HAKODATE } },
	{ E_ISDBT_STR_ID_NHK_G_ASAHIKAWA, {(unsigned char *) "NHK G, Asahikawa", (unsigned char *)ISDBT_JPN_STR_NHK_G_ASAHIKAWA } },
	{ E_ISDBT_STR_ID_NHK_E_ASAHIKAWA, { (unsigned char *)"NHK E, Asahikawa", (unsigned char *)ISDBT_JPN_STR_NHK_E_ASAHIKAWA } },
	{ E_ISDBT_STR_ID_HBC_ASAHIKAWA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HBC_ASAHIKAWA } },
	{ E_ISDBT_STR_ID_STV_ASAHIKAWA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_STV_ASAHIKAWA } },
	{ E_ISDBT_STR_ID_HTB_ASAHIKAWA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HTB_ASAHIKAWA } },
	{ E_ISDBT_STR_ID_UHB_ASAHIKAWA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_UHB_ASAHIKAWA } },
	{ E_ISDBT_STR_ID_TVH_ASAHIKAWA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVH_ASAHIKAWA } },
	{ E_ISDBT_STR_ID_NHK_G_OBIHIRO, { (unsigned char *)"NHK G, Obihiro", (unsigned char *)ISDBT_JPN_STR_NHK_G_OBIHIRO } },
	{ E_ISDBT_STR_ID_NHK_E_OBIHIRO, { (unsigned char *)"NHK E, Obihiro", (unsigned char *)ISDBT_JPN_STR_NHK_E_OBIHIRO } },
	{ E_ISDBT_STR_ID_HBC_OBIHIRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HBC_OBIHIRO } },
	{ E_ISDBT_STR_ID_STV_OBIHIRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_STV_OBIHIRO } },
	{ E_ISDBT_STR_ID_HTB_OBIHIRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HTB_OBIHIRO } },
	{ E_ISDBT_STR_ID_UHB_OBIHIRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_UHB_OBIHIRO } },
	{ E_ISDBT_STR_ID_TVH_OBIHIRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVH_OBIHIRO } },
	{ E_ISDBT_STR_ID_NHK_G_KUSHIRO, {(unsigned char *) "NHK G, Kushiro", (unsigned char *)ISDBT_JPN_STR_NHK_G_KUSHIRO } },
	{ E_ISDBT_STR_ID_NHK_E_KUSHIRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_KUSHIRO } },
	{ E_ISDBT_STR_ID_HBC_KUSHIRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HBC_KUSHIRO } },
	{ E_ISDBT_STR_ID_STV_KUSHIRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_STV_KUSHIRO } },
	{ E_ISDBT_STR_ID_HTB_KUSHIRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HTB_KUSHIRO } },
	{ E_ISDBT_STR_ID_UHB_KUSHIRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_UHB_KUSHIRO } },
	{ E_ISDBT_STR_ID_TVH_KUSHIRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVH_KUSHIRO } },
	{ E_ISDBT_STR_ID_NHK_G_KITAMI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_KITAMI } },
	{ E_ISDBT_STR_ID_NHK_E_KITAMI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_KITAMI } },
	{ E_ISDBT_STR_ID_HBC_KITAMI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HBC_KITAMI } },
	{ E_ISDBT_STR_ID_STV_KITAMI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_STV_KITAMI } },
	{ E_ISDBT_STR_ID_HTB_KITAMI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HTB_KITAMI } },
	{ E_ISDBT_STR_ID_UHB_KITAMI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_UHB_KITAMI } },
	{ E_ISDBT_STR_ID_TVH_KITAMI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVH_KITAMI } },
	{ E_ISDBT_STR_ID_NHK_G_MURORAN, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_MURORAN } },
	{ E_ISDBT_STR_ID_NHK_E_MURORAN, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_MURORAN } },
	{ E_ISDBT_STR_ID_HBC_MURORAN, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HBC_MURORAN } },
	{ E_ISDBT_STR_ID_STV_MURORAN, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_STV_MURORAN } },
	{ E_ISDBT_STR_ID_HTB_MURORAN, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HTB_MURORAN } },
	{ E_ISDBT_STR_ID_UHB_MURORAN, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_UHB_MURORAN } },
	{ E_ISDBT_STR_ID_TVH_MURORAN, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVH_MURORAN } },
	{ E_ISDBT_STR_ID_NHK_G_SENDAI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_SENDAI } },
	{ E_ISDBT_STR_ID_NHK_E_SENDAI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_SENDAI } },
	{ E_ISDBT_STR_ID_TBC_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TBC_TELEVISION } },
	{ E_ISDBT_STR_ID_SENDAI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SENDAI_TELEVISION } },
	{ E_ISDBT_STR_ID_MIYAGI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MIYAGI_TELEVISION } },
	{ E_ISDBT_STR_ID_KHB_HIGASHINIPPON_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KHB_HIGASHINIPPON_BROADCASTING } },
	{ E_ISDBT_STR_ID_NHK_G_AKITA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_AKITA } },
	{ E_ISDBT_STR_ID_NHK_E_AKITA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_AKITA } },
	{ E_ISDBT_STR_ID_ABS_AKITA_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_ABS_AKITA_BROADCASTING } },
	{ E_ISDBT_STR_ID_AKT_AKITA_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_AKT_AKITA_TELEVISION } },
	{ E_ISDBT_STR_ID_AAB_AKITA_ASAHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_AAB_AKITA_ASAHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_NHK_G_YAMAGATA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_YAMAGATA } },
	{ E_ISDBT_STR_ID_NHK_E_YAMAGATA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_YAMAGATA } },
	{ E_ISDBT_STR_ID_YBC_YAMAGATA_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_YBC_YAMAGATA_BROADCASTING } },
	{ E_ISDBT_STR_ID_YTS_YAMAGATA_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_YTS_YAMAGATA_TELEVISION } },
	{ E_ISDBT_STR_ID_TV_U_YAMAGATA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_U_YAMAGATA } },
	{ E_ISDBT_STR_ID_SAKURANBO_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SAKURANBO_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_MORIOKA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_MORIOKA } },
	{ E_ISDBT_STR_ID_NHK_E_MORIOKA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_MORIOKA } },
	{ E_ISDBT_STR_ID_IBC_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_IBC_TELEVISION } },
	{ E_ISDBT_STR_ID_TELEVISION_IWATE, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_IWATE } },
	{ E_ISDBT_STR_ID_MENKOI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MENKOI_TELEVISION } },
	{ E_ISDBT_STR_ID_IWATE_ASAHI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_IWATE_ASAHI_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_FUKUSHIMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_FUKUSHIMA } },
	{ E_ISDBT_STR_ID_NHK_E_FUKUSHIMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_FUKUSHIMA } },
	{ E_ISDBT_STR_ID_FUKUSHIMA_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FUKUSHIMA_TELEVISION } },
	{ E_ISDBT_STR_ID_FUKUSHIMA_CENTRAL_TELEVISIO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FUKUSHIMA_CENTRAL_TELEVISIO } },
	{ E_ISDBT_STR_ID_KFB_FUKUSHIMA_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KFB_FUKUSHIMA_BROADCASTING } },
	{ E_ISDBT_STR_ID_TV_U_FUKUSHIMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_U_FUKUSHIMA } },
	{ E_ISDBT_STR_ID_NHK_G_AOMORI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_AOMORI } },
	{ E_ISDBT_STR_ID_NHK_E_AOMORI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_AOMORI } },
	{ E_ISDBT_STR_ID_RAB_AOMORI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_RAB_AOMORI_BROADCASTING } },
	{ E_ISDBT_STR_ID_ATV_AOMORI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_ATV_AOMORI_TELEVISION } },
	{ E_ISDBT_STR_ID_ASAHI_BROADCASTING_AOMORI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_ASAHI_BROADCASTING_AOMORI } },
	{ E_ISDBT_STR_ID_TOKYO_MX_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TOKYO_MX_TELEVISION } },
	{ E_ISDBT_STR_ID_TVK, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVK } },
	{ E_ISDBT_STR_ID_GUNMA_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_GUNMA_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_MITO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_MITO } },
	{ E_ISDBT_STR_ID_CHIBA_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_CHIBA_TELEVISION } },
	{ E_ISDBT_STR_ID_TOCHIGI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TOCHIGI_TELEVISION } },
	{ E_ISDBT_STR_ID_TELETAMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELETAMA } },
	{ E_ISDBT_STR_ID_NHK_G_NAGANO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_NAGANO } },
	{ E_ISDBT_STR_ID_NHK_E_NAGANO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_NAGANO } },
	{ E_ISDBT_STR_ID_TV_SHINSHU, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TV_SHINSHU } },
	{ E_ISDBT_STR_ID_ABN_ASAHI_BROADCASTING_NAGANO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_ABN_ASAHI_BROADCASTING_NAGANO } },
	{ E_ISDBT_STR_ID_SBC_SHIN_ETSU_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SBC_SHIN_ETSU_BROADCASTING } },
	{ E_ISDBT_STR_ID_NBS_NAGANO_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NBS_NAGANO_BROADCASTING } },
	{ E_ISDBT_STR_ID_NHK_G_NIIGATA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_NIIGATA } },
	{ E_ISDBT_STR_ID_NHK_E_NIIGATA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_NIIGATA } },
	{ E_ISDBT_STR_ID_BSN, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_BSN } },
	{ E_ISDBT_STR_ID_NST, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NST } },
	{ E_ISDBT_STR_ID_TENY_TELEVISION_NIIGATA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TENY_TELEVISION_NIIGATA } },
	{ E_ISDBT_STR_ID_NIIGATA_TELEVISION_21, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NIIGATA_TELEVISION_21 } },
	{ E_ISDBT_STR_ID_NHK_G_KOFU, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_KOFU } },
	{ E_ISDBT_STR_ID_NHK_E_KOFU, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_KOFU } },
	{ E_ISDBT_STR_ID_YBS_YAMANASHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_YBS_YAMANASHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_UTY, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_UTY } },
	{ E_ISDBT_STR_ID_NHK_G_NAGOYA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_NAGOYA } },
	{ E_ISDBT_STR_ID_AICHI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_AICHI_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_KANAZAWA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_KANAZAWA } },
	{ E_ISDBT_STR_ID_NHK_E_KANAZAWA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_KANAZAWA } },
	{ E_ISDBT_STR_ID_TVKANAZAWA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVKANAZAWA } },
	{ E_ISDBT_STR_ID_HOKURIKU_ASAHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HOKURIKU_ASAHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_MRO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MRO } },
	{ E_ISDBT_STR_ID_ISHIKAWA_TV, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_ISHIKAWA_TV } },
	{ E_ISDBT_STR_ID_NHK_G_SHIZUOKA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_SHIZUOKA } },
	{ E_ISDBT_STR_ID_NHK_E_SHIZUOKA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_SHIZUOKA } },
	{ E_ISDBT_STR_ID_SBS, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SBS } },
	{ E_ISDBT_STR_ID_SHIZUOKA_TELECASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SHIZUOKA_TELECASTING } },
	{ E_ISDBT_STR_ID_SHIZUOKA_DAIICHI_TELEVISIN, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SHIZUOKA_DAIICHI_TELEVISIN } },
	{ E_ISDBT_STR_ID_SHIZUOKA_ASAHI_TV, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SHIZUOKA_ASAHI_TV } },
	{ E_ISDBT_STR_ID_NHK_G_FUKUI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_FUKUI } },
	{ E_ISDBT_STR_ID_NHK_E_FUKUI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_FUKUI } },
	{ E_ISDBT_STR_ID_FBC_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FBC_TELEVISION } },
	{ E_ISDBT_STR_ID_FUKUI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FUKUI_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_TOYAMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_TOYAMA } },
	{ E_ISDBT_STR_ID_NHK_E_TOYAMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_TOYAMA } },
	{ E_ISDBT_STR_ID_KNB_KITANIHON_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KNB_KITANIHON_BROADCASTING } },
	{ E_ISDBT_STR_ID_BBT_TOYAMA_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_BBT_TOYAMA_TELEVISION } },
	{ E_ISDBT_STR_ID_TULIP_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TULIP_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_TSU, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_TSU } },
	{ E_ISDBT_STR_ID_MIE_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MIE_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_GIFU, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_GIFU } },
	{ E_ISDBT_STR_ID_GIFU_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_GIFU_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_OSAKA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_OSAKA } },
	{ E_ISDBT_STR_ID_TELEVISION_OSAKA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_OSAKA } },
	{ E_ISDBT_STR_ID_NHK_G_KYOTO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_KYOTO } },
	{ E_ISDBT_STR_ID_KBS_KYOTO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KBS_KYOTO } },
	{ E_ISDBT_STR_ID_NHK_G_KOBE, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_KOBE } },
	{ E_ISDBT_STR_ID_SUN_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SUN_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_WAKAYAMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_WAKAYAMA } },
	{ E_ISDBT_STR_ID_TELEVISION_WAKAYAMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_WAKAYAMA } },
	{ E_ISDBT_STR_ID_NHK_G_NARA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_NARA } },
	{ E_ISDBT_STR_ID_NARA_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NARA_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_OTSU, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_OTSU } },
	{ E_ISDBT_STR_ID_BBC_BIWAKO_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_BBC_BIWAKO_BROADCASTING } },
	{ E_ISDBT_STR_ID_NHK_G_HIROSHIMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_HIROSHIMA } },
	{ E_ISDBT_STR_ID_NHK_E_HIROSHIMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_HIROSHIMA } },
	{ E_ISDBT_STR_ID_RCC_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_RCC_TELEVISION } },
	{ E_ISDBT_STR_ID_HIROSHIMA_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HIROSHIMA_TELEVISION } },
	{ E_ISDBT_STR_ID_HIROSHIMA_HOME_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_HIROSHIMA_HOME_TELEVISION } },
	{ E_ISDBT_STR_ID_TSS, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TSS } },
	{ E_ISDBT_STR_ID_NHK_G_OKAYAMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_OKAYAMA } },
	{ E_ISDBT_STR_ID_NHK_E_OKAYAMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_OKAYAMA } },
	{ E_ISDBT_STR_ID_NHK_G_MATSUE, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_MATSUE } },
	{ E_ISDBT_STR_ID_NHK_E_MATSUE, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_MATSUE } },
	{ E_ISDBT_STR_ID_NHK_G_TOTTORI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_TOTTORI } },
	{ E_ISDBT_STR_ID_NHK_E_TOTTORI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_TOTTORI } },
	{ E_ISDBT_STR_ID_NHK_G_YAMAGUCHI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_YAMAGUCHI } },
	{ E_ISDBT_STR_ID_NHK_E_YAMAGUCHI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_YAMAGUCHI } },
	{ E_ISDBT_STR_ID_KRY_YAMAGUCHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KRY_YAMAGUCHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_TYS_TV_YAMAGUCHI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TYS_TV_YAMAGUCHI } },
	{ E_ISDBT_STR_ID_YAB_YAMAGUCHI_ASAHI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_YAB_YAMAGUCHI_ASAHI } },
	{ E_ISDBT_STR_ID_NHK_G_MATSUYAMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_MATSUYAMA } },
	{ E_ISDBT_STR_ID_NHK_E_MATSUYAMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_MATSUYAMA } },
	{ E_ISDBT_STR_ID_NANKAI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NANKAI_BROADCASTING } },
	{ E_ISDBT_STR_ID_EHIME_ASAHI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_EHIME_ASAHI_TELEVISION } },
	{ E_ISDBT_STR_ID_I_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_I_TELEVISION } },
	{ E_ISDBT_STR_ID_TELEVISION_EHIME, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_EHIME } },
	{ E_ISDBT_STR_ID_NHK_G_TAKAMATSU, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_TAKAMATSU } },
	{ E_ISDBT_STR_ID_NHK_E_TAKAMATSU, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_TAKAMATSU } },
	{ E_ISDBT_STR_ID_NHK_G_TOKUSHIMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_TOKUSHIMA } },
	{ E_ISDBT_STR_ID_NHK_E_TOKUSHIMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_TOKUSHIMA } },
	{ E_ISDBT_STR_ID_SHIKOKU_BROADCATING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SHIKOKU_BROADCATING } },
	{ E_ISDBT_STR_ID_NHK_G_KOUCHI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_KOUCHI } },
	{ E_ISDBT_STR_ID_NHK_E_KOUCHI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_KOUCHI } },
	{ E_ISDBT_STR_ID_KOCHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KOCHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_TELEVISION_KOCHI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TELEVISION_KOCHI } },
	{ E_ISDBT_STR_ID_SUN_SUN_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_SUN_SUN_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_FUKUOKA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_FUKUOKA } },
	{ E_ISDBT_STR_ID_NHK_G_KITAKYUSHU, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_KITAKYUSHU } },
	{ E_ISDBT_STR_ID_NHK_E_FUKUOKA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_FUKUOKA } },
	{ E_ISDBT_STR_ID_NHK_E_KITAKYUSHU, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_KITAKYUSHU } },
	{ E_ISDBT_STR_ID_KBC_KYUSHU_ASAHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KBC_KYUSHU_ASAHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_RKB_MAINICHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_RKB_MAINICHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_FBS_FUKUOKA_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_FBS_FUKUOKA_BROADCASTING } },
	{ E_ISDBT_STR_ID_TVQ_KYUSHU_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TVQ_KYUSHU_BROADCASTING } },
	{ E_ISDBT_STR_ID_TNC_TELEVISION_NISHINIPPON, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TNC_TELEVISION_NISHINIPPON } },
	{ E_ISDBT_STR_ID_NHK_G_KUMAMOTO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_KUMAMOTO } },
	{ E_ISDBT_STR_ID_NHK_E_KUMAMOTO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_KUMAMOTO } },
	{ E_ISDBT_STR_ID_RKK_KUMAMOTO_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_RKK_KUMAMOTO_BROADCASTING } },
	{ E_ISDBT_STR_ID_TKU_TV_KUMAMOTO, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TKU_TV_KUMAMOTO } },
	{ E_ISDBT_STR_ID_KKT_KUMAMATO_KENMIN, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KKT_KUMAMATO_KENMIN } },
	{ E_ISDBT_STR_ID_KAB_KUMAMOTO_ASAHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KAB_KUMAMOTO_ASAHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_NHK_G_NAGASAKI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_NAGASAKI } },
	{ E_ISDBT_STR_ID_NHK_E_NAGASAKI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_NAGASAKI } },
	{ E_ISDBT_STR_ID_NBC_NAGASAKI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NBC_NAGASAKI_BROADCASTING } },
	{ E_ISDBT_STR_ID_KTN_TELEVISION_NAGASAKI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KTN_TELEVISION_NAGASAKI } },
	{ E_ISDBT_STR_ID_NCC_NAGASAKI_CULTURE_TELECASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NCC_NAGASAKI_CULTURE_TELECASTING } },
	{ E_ISDBT_STR_ID_NIB_NAGASAKI_INTERNATIONAL_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NIB_NAGASAKI_INTERNATIONAL_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_KAGOSHIMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_KAGOSHIMA } },
	{ E_ISDBT_STR_ID_NHK_E_KAGOSHIMA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_KAGOSHIMA } },
	{ E_ISDBT_STR_ID_MBC_MINAMINIHON_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MBC_MINAMINIHON_BROADCASTING } },
	{ E_ISDBT_STR_ID_KTS_KAGOSHIMA_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KTS_KAGOSHIMA_TELEVISION } },
	{ E_ISDBT_STR_ID_KKB_KAGOSHIMA_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KKB_KAGOSHIMA_BROADCASTING } },
	{ E_ISDBT_STR_ID_KYT_KAGOSHIMA_YOMIURI_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_KYT_KAGOSHIMA_YOMIURI_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_MIYAZAKI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_MIYAZAKI } },
	{ E_ISDBT_STR_ID_NHK_E_MIYAZAKI, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_MIYAZAKI } },
	{ E_ISDBT_STR_ID_MRT_MIYAZAKI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_MRT_MIYAZAKI_BROADCASTING } },
	{ E_ISDBT_STR_ID_UMK_MIYAZAKI_TELECASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_UMK_MIYAZAKI_TELECASTING } },
	{ E_ISDBT_STR_ID_NHK_G_OITA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_OITA } },
	{ E_ISDBT_STR_ID_NHK_E_OITA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_OITA } },
	{ E_ISDBT_STR_ID_OBS_OITA_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_OBS_OITA_BROADCASTING } },
	{ E_ISDBT_STR_ID_TOS_TELEVISION_OITA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_TOS_TELEVISION_OITA } },
	{ E_ISDBT_STR_ID_OAB_OITA_ASAHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_OAB_OITA_ASAHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_NHK_G_SAGA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_SAGA } },
	{ E_ISDBT_STR_ID_NHK_E_SAGA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_SAGA } },
	{ E_ISDBT_STR_ID_STS_SAGA_TELEVISION, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_STS_SAGA_TELEVISION } },
	{ E_ISDBT_STR_ID_NHK_G_NAHA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_G_NAHA } },
	{ E_ISDBT_STR_ID_NHK_E_NAHA, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_NHK_E_NAHA } },
	{ E_ISDBT_STR_ID_RBC_RYUKYU_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_RBC_RYUKYU_BROADCASTING } },
	{ E_ISDBT_STR_ID_QAB_RYUKYU_ASAHI_BROADCASTING, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_QAB_RYUKYU_ASAHI_BROADCASTING } },
	{ E_ISDBT_STR_ID_OKINAWA_TELEVISION_OTV, { NULL_STR, (unsigned char *)ISDBT_JPN_STR_OKINAWA_TELEVISION_OTV } },

	{ E_ISDBT_STR_ID_LAST, { NULL_STR, NULL_STR } }
};


/* ----------------------------------------------------------------------
**  Broadcaster List Information according to Region ID
** ---------------------------------------------------------------------- */

const T_BROADCASTER_INFO stBroadInfo_UNDEFINED_0[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0xFFFF,	E_ISDBT_STR_ID_UNDEFINED,	0,	0xFFFF,	0xFFFF	}
};

const T_BROADCASTER_INFO stBroadInfo_KANTO_WIDE_AREA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7FE0,	E_ISDBT_STR_ID_NHK_G_TOKYO,					1,	0x0580,	0x0587	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7FE1,	E_ISDBT_STR_ID_NHK_E_TOKYO,					2,	0x0588,	0x058F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7FE2,	E_ISDBT_STR_ID_NIPPON_TV,					4,	0x0590,	0x0597	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7FE3,	E_ISDBT_STR_ID_TBS,							6,	0x0598,	0x059F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7FE4,	E_ISDBT_STR_ID_FUJI_TV,						8,	0x05A0,	0x05A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7FE5,	E_ISDBT_STR_ID_TV_ASAHI,					5,	0x05A8,	0x05AF	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7FE6,	E_ISDBT_STR_ID_TV_TOKYO,					7,	0x05B0,	0x05B7	},
	{	8,	E_ISDBT_STR_ID_NONE,	0x7FE8,	E_ISDBT_STR_ID_THE_UNIVERSITY_OF_THE_AIR,	12,	0x05C0,	0x05C7	}
};

const T_BROADCASTER_INFO stBroadInfo_KINKI_WIDE_AREA[] =
{
	{	1,	E_ISDBT_STR_ID_NONE,	0x7FD1,	E_ISDBT_STR_ID_NHK_E_OSAKA,					2,	0x0988,	0x098F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7FD2,	E_ISDBT_STR_ID_MBS_MAINICHI_BROADCASTING,	4,	0x0990,	0x0997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7FD3,	E_ISDBT_STR_ID_ABC_TELEVISION,				6,	0x0998,	0x099F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7FD4,	E_ISDBT_STR_ID_KANSAI_TELECASTING,			8,	0x09A0,	0x09A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7FD5,	E_ISDBT_STR_ID_YOMIURI_TELECASTING,			10,	0x09A8,	0x09AF	}
};

const T_BROADCASTER_INFO stBroadInfo_CHUKYO_WIDE_AREA[] =
{
	{	1,	E_ISDBT_STR_ID_NONE,	0x7FC1,	E_ISDBT_STR_ID_NHK_E_NAGOYA,	2,	0x0D88,	0x0D8F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7FC2,	E_ISDBT_STR_ID_TOKAI_TV,		1,	0x0D90,	0x0D97	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7FC3,	E_ISDBT_STR_ID_CBC,				5,	0x0D98,	0x0D9F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7FC4,	E_ISDBT_STR_ID_MEE_TELE,		6,	0x0DA0,	0x0DA7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7FC5,	E_ISDBT_STR_ID_CHUKYO_TV,		4,	0x0DA8,	0x0DAF	}
};

const T_BROADCASTER_INFO stBroadInfo_HOKKAIDO_AREA[] =
{
	{	2,	E_ISDBT_STR_ID_NONE,	0x7FB2,	E_ISDBT_STR_ID_HBC_HOKKAIDO_BROADCASTING,	1,	0x1190,	0x1197	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7FB3,	E_ISDBT_STR_ID_STV_SAPPORO_TELEVISION,		5,	0x1198,	0x119F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7FB4,	E_ISDBT_STR_ID_HTB_HOKKAIDO_TELEVISION,		6,	0x11A0,	0x11A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7FB5,	E_ISDBT_STR_ID_UHB,							8,	0x11A8,	0x11AF	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7FB6,	E_ISDBT_STR_ID_TVH,							7,	0x11B0,	0x11B7	}
};

const T_BROADCASTER_INFO stBroadInfo_OKAYAMA_N_KAGAWA[] =
{
	{	2,	E_ISDBT_STR_ID_NONE,	0x7FA2,	E_ISDBT_STR_ID_RNC_NISHINIPPON_TELEVISION,	4,	0x1590,	0x1597	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7FA3,	E_ISDBT_STR_ID_KSB_SETONAIKAI_BROADCASTING,	5,	0x1598,	0x159F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7FA4,	E_ISDBT_STR_ID_RSK_TELEVISION,				6,	0x15A0,	0x15A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7FA5,	E_ISDBT_STR_ID_TELEVISION_SETOUCHI,			7,	0x15A8,	0x15AF	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7FA6,	E_ISDBT_STR_ID_OHK_TV,						8,	0x15B0,	0x15B7	}
};

const T_BROADCASTER_INFO stBroadInfo_SHIMANE_N_TOTTORI[] =
{
	{	2,	E_ISDBT_STR_ID_NONE,	0x7F92,	E_ISDBT_STR_ID_SAN_IN_CHUO_TELEVISION,	8,	0x1990,	0x1997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7F93,	E_ISDBT_STR_ID_BSS_TELEVISION,			6,	0x1998,	0x199F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7F94,	E_ISDBT_STR_ID_NIHONKAI_TELEVISION,		1,	0x19A0,	0x19A7	}
};

const T_BROADCASTER_INFO stBroadInfo_UNDEFINED_7[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0xFFFF,	E_ISDBT_STR_ID_UNDEFINED,	0,	0xFFFF,	0xFFFF	}
};

const T_BROADCASTER_INFO stBroadInfo_UNDEFINED_8[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0xFFFF,	E_ISDBT_STR_ID_UNDEFINED,	0,	0xFFFF,	0xFFFF	}
};

const T_BROADCASTER_INFO stBroadInfo_UNDEFINED_9[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0xFFFF,	E_ISDBT_STR_ID_UNDEFINED,	0,	0xFFFF,	0xFFFF	}
};

const T_BROADCASTER_INFO stBroadInfo_HOKKAIDO_SAPPORO[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7F50,	E_ISDBT_STR_ID_NHK_G_SAPPORO,	3,	0x2980,	0x2987	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7F51,	E_ISDBT_STR_ID_NHK_E_SAPPORO,	2,	0x2988,	0x298F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7F52,	E_ISDBT_STR_ID_HBC_SAPPORO,		1,	0x2990,	0x2997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7F53,	E_ISDBT_STR_ID_STV_SAPPORO,		5,	0x2998,	0x299F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7F54,	E_ISDBT_STR_ID_HTB_SAPPORO,		6,	0x29A0,	0x29A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7F55,	E_ISDBT_STR_ID_UHB_SAPPORO,		8,	0x29A8,	0x29AF	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7F56,	E_ISDBT_STR_ID_TVH_SAPPORO,		7,	0x29B0,	0x29B7	}
};

const T_BROADCASTER_INFO stBroadInfo_HOKKAIDO_HAKODATE[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7F40,	E_ISDBT_STR_ID_NHK_G_HAKODATE,	3,	0x2D80,	0x2D87	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7F41,	E_ISDBT_STR_ID_NHK_E_HAKODATE,	2,	0x2D88,	0x2D8F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7F42,	E_ISDBT_STR_ID_HBC_HAKODATE,	1,	0x2D90,	0x2D97	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7F43,	E_ISDBT_STR_ID_STV_HAKODATE,	5,	0x2D98,	0x2D9F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7F44,	E_ISDBT_STR_ID_HTB_HAKODATE,	6,	0x2DA0,	0x2DA7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7F45,	E_ISDBT_STR_ID_UHB_HAKODATE,	8,	0x2DA8,	0x2DAF	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7F46,	E_ISDBT_STR_ID_TVH_HAKODATE,	7,	0x2DB0,	0x2DB7	}
};

const T_BROADCASTER_INFO stBroadInfo_HOKKAIDO_ASAHIKAWA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7F30,	E_ISDBT_STR_ID_NHK_G_ASAHIKAWA,	3,	0x3180,	0x3187	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7F31,	E_ISDBT_STR_ID_NHK_E_ASAHIKAWA,	2,	0x3188,	0x318F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7F32,	E_ISDBT_STR_ID_HBC_ASAHIKAWA,	1,	0x3190,	0x3197	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7F33,	E_ISDBT_STR_ID_STV_ASAHIKAWA,	5,	0x3198,	0x319F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7F34,	E_ISDBT_STR_ID_HTB_ASAHIKAWA,	6,	0x31A0,	0x31A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7F35,	E_ISDBT_STR_ID_UHB_ASAHIKAWA,	8,	0x31A8,	0x31AF	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7F36,	E_ISDBT_STR_ID_TVH_ASAHIKAWA,	7,	0x31B0,	0x31B7	}
};

const T_BROADCASTER_INFO stBroadInfo_HOKKAIDO_OBIHIRO[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7F20,	E_ISDBT_STR_ID_NHK_G_OBIHIRO,	3,	0x3580,	0x3587	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7F21,	E_ISDBT_STR_ID_NHK_E_OBIHIRO,	2,	0x3588,	0x358F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7F22,	E_ISDBT_STR_ID_HBC_OBIHIRO,		1,	0x3590,	0x3597	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7F23,	E_ISDBT_STR_ID_STV_OBIHIRO,		5,	0x3598,	0x359F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7F24,	E_ISDBT_STR_ID_HTB_OBIHIRO,		6,	0x35A0,	0x35A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7F25,	E_ISDBT_STR_ID_UHB_OBIHIRO,		8,	0x35A8,	0x35AF	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7F26,	E_ISDBT_STR_ID_TVH_OBIHIRO,		7,	0x35B0,	0x35B7	}
};

const T_BROADCASTER_INFO stBroadInfo_HOKKAIDO_KUSHIRO[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7F10,	E_ISDBT_STR_ID_NHK_G_KUSHIRO,	3,	0x3980,	0x3987	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7F11,	E_ISDBT_STR_ID_NHK_E_KUSHIRO,	2,	0x3988,	0x398F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7F12,	E_ISDBT_STR_ID_HBC_KUSHIRO,		1,	0x3990,	0x3997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7F13,	E_ISDBT_STR_ID_STV_KUSHIRO,		5,	0x3998,	0x399F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7F14,	E_ISDBT_STR_ID_HTB_KUSHIRO,		6,	0x39A0,	0x39A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7F15,	E_ISDBT_STR_ID_UHB_KUSHIRO,		8,	0x39A8,	0x39AF	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7F16,	E_ISDBT_STR_ID_TVH_KUSHIRO,		7,	0x39B0,	0x39B7	}
};

const T_BROADCASTER_INFO stBroadInfo_HOKKAIDO_KITAMI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7F00,	E_ISDBT_STR_ID_NHK_G_KITAMI,	3,	0x3D80,	0x3D87	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7F01,	E_ISDBT_STR_ID_NHK_E_KITAMI,	2,	0x3D88,	0x3D8F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7F02,	E_ISDBT_STR_ID_HBC_KITAMI,		1,	0x3D90,	0x3D97	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7F03,	E_ISDBT_STR_ID_STV_KITAMI,		5,	0x3D98,	0x3D9F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7F04,	E_ISDBT_STR_ID_HTB_KITAMI,		6,	0x3DA0,	0x3DA7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7F05,	E_ISDBT_STR_ID_UHB_KITAMI,		8,	0x3DA8,	0x3DAF	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7F06,	E_ISDBT_STR_ID_TVH_KITAMI,		7,	0x3DB0,	0x3DB7	}
};

const T_BROADCASTER_INFO stBroadInfo_HOKKAIDO_MURORAN[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7EF0,	E_ISDBT_STR_ID_NHK_G_MURORAN,	3,	0x4180,	0x4187	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7EF1,	E_ISDBT_STR_ID_NHK_E_MURORAN,	2,	0x4188,	0x418F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7EF2,	E_ISDBT_STR_ID_HBC_MURORAN,		1,	0x4190,	0x4197	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7EF3,	E_ISDBT_STR_ID_STV_MURORAN,		5,	0x4198,	0x419F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7EF4,	E_ISDBT_STR_ID_HTB_MURORAN,		6,	0x41A0,	0x41A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7EF5,	E_ISDBT_STR_ID_UHB_MURORAN,		8,	0x41A8,	0x41AF	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7EF6,	E_ISDBT_STR_ID_TVH_MURORAN,		7,	0x41B0,	0x41B7	}
};

const T_BROADCASTER_INFO stBroadInfo_MIYAGI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7EE0,	E_ISDBT_STR_ID_NHK_G_SENDAI,					3,	0x4580,	0x4587	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7EE1,	E_ISDBT_STR_ID_NHK_E_SENDAI,					2,	0x4588,	0x458F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7EE2,	E_ISDBT_STR_ID_TBC_TELEVISION,					1,	0x4590,	0x4597	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7EE3,	E_ISDBT_STR_ID_SENDAI_TELEVISION,				8,	0x4598,	0x459F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7EE4,	E_ISDBT_STR_ID_MIYAGI_TELEVISION,				4,	0x45A0,	0x45A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7EE5,	E_ISDBT_STR_ID_KHB_HIGASHINIPPON_BROADCASTING,	5,	0x45A8,	0x45AF	}
};

const T_BROADCASTER_INFO stBroadInfo_AKITA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7ED0,	E_ISDBT_STR_ID_NHK_G_AKITA,						1,	0x4980,	0x4987	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7ED1,	E_ISDBT_STR_ID_NHK_E_AKITA,						2,	0x4988,	0x498F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7ED2,	E_ISDBT_STR_ID_ABS_AKITA_BROADCASTING,			4,	0x4990,	0x4997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7ED3,	E_ISDBT_STR_ID_AKT_AKITA_TELEVISION,			8,	0x4998,	0x499F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7ED4,	E_ISDBT_STR_ID_AAB_AKITA_ASAHI_BROADCASTING,	5,	0x49A0,	0x49A7	}
};

const T_BROADCASTER_INFO stBroadInfo_YAMAGATA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7EC0,	E_ISDBT_STR_ID_NHK_G_YAMAGATA,				1,	0x4D80,	0x4D87	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7EC1,	E_ISDBT_STR_ID_NHK_E_YAMAGATA,				2,	0x4D88,	0x4D8F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7EC2,	E_ISDBT_STR_ID_YBC_YAMAGATA_BROADCASTING,	4,	0x4D90,	0x4D97	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7EC3,	E_ISDBT_STR_ID_YTS_YAMAGATA_TELEVISION,		5,	0x4D98,	0x4D9F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7EC4,	E_ISDBT_STR_ID_TV_U_YAMAGATA,				6,	0x4DA0,	0x4DA7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7EC5,	E_ISDBT_STR_ID_SAKURANBO_TELEVISION,		8,	0x4DA8,	0x4DAF	}
};

const T_BROADCASTER_INFO stBroadInfo_IWATE[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7EB0,	E_ISDBT_STR_ID_NHK_G_MORIOKA,			1,	0x5180,	0x5187	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7EB1,	E_ISDBT_STR_ID_NHK_E_MORIOKA,			2,	0x5188,	0x518F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7EB2,	E_ISDBT_STR_ID_IBC_TELEVISION,			6,	0x5190,	0x5197	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7EB3,	E_ISDBT_STR_ID_TELEVISION_IWATE,		4,	0x5198,	0x519F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7EB4,	E_ISDBT_STR_ID_MENKOI_TELEVISION,		8,	0x51A0,	0x51A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7EB5,	E_ISDBT_STR_ID_IWATE_ASAHI_TELEVISION,	5,	0x51A8,	0x51AF	}
};

const T_BROADCASTER_INFO stBroadInfo_FUKUSHIMA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7EA0,	E_ISDBT_STR_ID_NHK_G_FUKUSHIMA,				1,	0x5580,	0x5587	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7EA1,	E_ISDBT_STR_ID_NHK_E_FUKUSHIMA,				2,	0x5588,	0x558F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7EA2,	E_ISDBT_STR_ID_FUKUSHIMA_TELEVISION,		8,	0x5590,	0x5597	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7EA3,	E_ISDBT_STR_ID_FUKUSHIMA_CENTRAL_TELEVISIO,	4,	0x5598,	0x559F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7EA4,	E_ISDBT_STR_ID_KFB_FUKUSHIMA_BROADCASTING,	5,	0x55A0,	0x55A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7EA5,	E_ISDBT_STR_ID_TV_U_FUKUSHIMA,				6,	0x55A8,	0x55AF	}
};

const T_BROADCASTER_INFO stBroadInfo_AOMORI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7E90,	E_ISDBT_STR_ID_NHK_G_AOMORI,				3,	0x5980,	0x5987	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7E91,	E_ISDBT_STR_ID_NHK_E_AOMORI,				2,	0x5988,	0x598F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7E92,	E_ISDBT_STR_ID_RAB_AOMORI_BROADCASTING,		1,	0x5990,	0x5997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7E93,	E_ISDBT_STR_ID_ATV_AOMORI_TELEVISION,		6,	0x5998,	0x599F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7E94,	E_ISDBT_STR_ID_ASAHI_BROADCASTING_AOMORI,	5,	0x59A0,	0x59A7	}
};

const T_BROADCASTER_INFO stBroadInfo_TOKYO[] =
{
	{	7,	E_ISDBT_STR_ID_NONE,	0x7E87,	E_ISDBT_STR_ID_TOKYO_MX_TELEVISION,		9,	0x5DB8,	0x5DBF	}
};

const T_BROADCASTER_INFO stBroadInfo_KANAGAWA[] =
{
	{	7,	E_ISDBT_STR_ID_NONE,	0x7E77,	E_ISDBT_STR_ID_TVK,		3,	0x61B8,	0x61BF	}
};

const T_BROADCASTER_INFO stBroadInfo_GUNMA[] =
{
	{	7,	E_ISDBT_STR_ID_NONE,	0x7E67,	E_ISDBT_STR_ID_GUNMA_TELEVISION,	3,	0x65B8,	0x65BF	}
};

const T_BROADCASTER_INFO stBroadInfo_IBARAKI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7E50,	E_ISDBT_STR_ID_NHK_G_MITO,	1,	0x6980,	0x6987	}
};

const T_BROADCASTER_INFO stBroadInfo_CHIBA[] =
{
	{	7,	E_ISDBT_STR_ID_NONE,	0x7E47,	E_ISDBT_STR_ID_CHIBA_TELEVISION,	3,	0x6DB8,	0x6DBF	}
};

const T_BROADCASTER_INFO stBroadInfo_TOCHIGI[] =
{
	{	7,	E_ISDBT_STR_ID_NONE,	0x7E37,	E_ISDBT_STR_ID_TOCHIGI_TELEVISION,	3,	0x71B8,	0x71BF	}
};

const T_BROADCASTER_INFO stBroadInfo_SAITAMA[] =
{
	{	7,	E_ISDBT_STR_ID_NONE,	0x7E27,	E_ISDBT_STR_ID_TELETAMA,	3,	0x75B8,	0x75BF	}
};

const T_BROADCASTER_INFO stBroadInfo_NAGANO[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7E10,	E_ISDBT_STR_ID_NHK_G_NAGANO,					1,	0x7980,	0x7987	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7E11,	E_ISDBT_STR_ID_NHK_E_NAGANO,					2,	0x7988,	0x798F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7E12,	E_ISDBT_STR_ID_TV_SHINSHU,						4,	0x7990,	0x7997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7E13,	E_ISDBT_STR_ID_ABN_ASAHI_BROADCASTING_NAGANO,	5,	0x7998,	0x799F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7E14,	E_ISDBT_STR_ID_SBC_SHIN_ETSU_BROADCASTING,		6,	0x79A0,	0x79A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7E15,	E_ISDBT_STR_ID_NBS_NAGANO_BROADCASTING,			8,	0x79A8,	0x79AF	}
};

const T_BROADCASTER_INFO stBroadInfo_NIIGATA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7E00,	E_ISDBT_STR_ID_NHK_G_NIIGATA,			1,	0x7D80,	0x7D87	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7E01,	E_ISDBT_STR_ID_NHK_E_NIIGATA,			2,	0x7D88,	0x7D8F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7E02,	E_ISDBT_STR_ID_BSN,						6,	0x7D90,	0x7D97	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7E03,	E_ISDBT_STR_ID_NST,						8,	0x7D98,	0x7D9F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7E04,	E_ISDBT_STR_ID_TENY_TELEVISION_NIIGATA,	4,	0x7DA0,	0x7DA7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7E05,	E_ISDBT_STR_ID_NIIGATA_TELEVISION_21,	5,	0x7DA8,	0x7DAF	}
};

const T_BROADCASTER_INFO stBroadInfo_YAMANASHI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7DF0,	E_ISDBT_STR_ID_NHK_G_KOFU,					1,	0x8180,	0x8187	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7DF1,	E_ISDBT_STR_ID_NHK_E_KOFU,					2,	0x8188,	0x818F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7DF2,	E_ISDBT_STR_ID_YBS_YAMANASHI_BROADCASTING,	4,	0x8190,	0x8197	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7DF3,	E_ISDBT_STR_ID_UTY,							6,	0x8198,	0x819F	}
};

const T_BROADCASTER_INFO stBroadInfo_AICHI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7DE0,	E_ISDBT_STR_ID_NHK_G_NAGOYA,		3,	0x8580,	0x8587	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7DE6,	E_ISDBT_STR_ID_AICHI_TELEVISION,	10,	0x85B0,	0x85B7	},
};

const T_BROADCASTER_INFO stBroadInfo_ISHIKAWA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7DD0,	E_ISDBT_STR_ID_NHK_G_KANAZAWA,				1,	0x8980,	0x8987	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7DD1,	E_ISDBT_STR_ID_NHK_E_KANAZAWA,				2,	0x8988,	0x898F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7DD2,	E_ISDBT_STR_ID_TVKANAZAWA,					4,	0x8990,	0x8997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7DD3,	E_ISDBT_STR_ID_HOKURIKU_ASAHI_BROADCASTING,	5,	0x8998,	0x899F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7DD4,	E_ISDBT_STR_ID_MRO,							6,	0x89A0,	0x89A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7DD5,	E_ISDBT_STR_ID_ISHIKAWA_TV,					8,	0x89A8,	0x89AF	}
};

const T_BROADCASTER_INFO stBroadInfo_SHIZUOKA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7DC0,	E_ISDBT_STR_ID_NHK_G_SHIZUOKA,				1,	0x8D80,	0x8D87	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7DC1,	E_ISDBT_STR_ID_NHK_E_SHIZUOKA,				2,	0x8D88,	0x8D8F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7DC2,	E_ISDBT_STR_ID_SBS,							6,	0x8D90,	0x8D97	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7DC3,	E_ISDBT_STR_ID_SHIZUOKA_TELECASTING,		8,	0x8D98,	0x8D9F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7DC4,	E_ISDBT_STR_ID_SHIZUOKA_DAIICHI_TELEVISIN,	4,	0x8DA0,	0x8DA7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7DC5,	E_ISDBT_STR_ID_SHIZUOKA_ASAHI_TV,			5,	0x8DA8,	0x8DAF	}
};

const T_BROADCASTER_INFO stBroadInfo_FUKUI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7DB0,	E_ISDBT_STR_ID_NHK_G_FUKUI,			1,	0x9180,	0x9187	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7DB1,	E_ISDBT_STR_ID_NHK_E_FUKUI,			2,	0x9188,	0x918F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7DB2,	E_ISDBT_STR_ID_FBC_TELEVISION,		7,	0x9190,	0x9197	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7DB3,	E_ISDBT_STR_ID_FUKUI_TELEVISION,	8,	0x9198,	0x919F	}
};

const T_BROADCASTER_INFO stBroadInfo_TOYAMA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7DA0,	E_ISDBT_STR_ID_NHK_G_TOYAMA,				3,	0x9580,	0x9587	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7DA1,	E_ISDBT_STR_ID_NHK_E_TOYAMA,				2,	0x9588,	0x958F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7DA2,	E_ISDBT_STR_ID_KNB_KITANIHON_BROADCASTING,	1,	0x9590,	0x9597	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7DA3,	E_ISDBT_STR_ID_BBT_TOYAMA_TELEVISION,		8,	0x9598,	0x959F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7DA4,	E_ISDBT_STR_ID_TULIP_TELEVISION,			6,	0x95A0,	0x95A7	}
};

const T_BROADCASTER_INFO stBroadInfo_MIE[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7D90,	E_ISDBT_STR_ID_NHK_G_TSU,		3,	0x9980,	0x9987	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7D96,	E_ISDBT_STR_ID_MIE_TELEVISION,	7,	0x99B0,	0x99B7	}
};

const T_BROADCASTER_INFO stBroadInfo_GIFU[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7D80,	E_ISDBT_STR_ID_NHK_G_GIFU,		3,	0x9D80,	0x9D87	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7D86,	E_ISDBT_STR_ID_GIFU_TELEVISION,	8,	0x9DB0,	0x9DB7	}
};

const T_BROADCASTER_INFO stBroadInfo_OSAKA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7D70,	E_ISDBT_STR_ID_NHK_G_OSAKA,			1,	0xA180,	0xA187	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7D76,	E_ISDBT_STR_ID_TELEVISION_OSAKA,	7,	0xA1B0,	0xA1B7	}
};

const T_BROADCASTER_INFO stBroadInfo_KYOTO[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7D60,	E_ISDBT_STR_ID_NHK_G_KYOTO,	1,	0xA580,	0xA587	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7D66,	E_ISDBT_STR_ID_KBS_KYOTO,	5,	0xA5B0,	0xA5B7	}
};

const T_BROADCASTER_INFO stBroadInfo_HYOGO[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7D50,	E_ISDBT_STR_ID_NHK_G_KOBE,		1,	0xA980,	0xA987	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7D56,	E_ISDBT_STR_ID_SUN_TELEVISION,	3,	0xA9B0,	0xA9B7	}
};

const T_BROADCASTER_INFO stBroadInfo_WAKAYAMA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7D40,	E_ISDBT_STR_ID_NHK_G_WAKAYAMA,		1,	0xAD80,	0xAD87	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7D46,	E_ISDBT_STR_ID_TELEVISION_WAKAYAMA,	5,	0xADB0,	0xADB7	}
};

const T_BROADCASTER_INFO stBroadInfo_NARA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7D30,	E_ISDBT_STR_ID_NHK_G_NARA,		1,	0xB180,	0xB187	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7D36,	E_ISDBT_STR_ID_NARA_TELEVISION,	9,	0xB1B0,	0xB1B7	}
};

const T_BROADCASTER_INFO stBroadInfo_SHIGA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7D20,	E_ISDBT_STR_ID_NHK_G_OTSU,				1,	0xB580,	0xB587	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7D26,	E_ISDBT_STR_ID_BBC_BIWAKO_BROADCASTING,	3,	0xB5B0,	0xB5B7	}
};

const T_BROADCASTER_INFO stBroadInfo_HIROSHIMA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7D10,	E_ISDBT_STR_ID_NHK_G_HIROSHIMA,				1,	0xB980,	0xB987	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7D11,	E_ISDBT_STR_ID_NHK_E_HIROSHIMA,				2,	0xB988,	0xB98F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7D12,	E_ISDBT_STR_ID_RCC_TELEVISION,				3,	0xB990,	0xB997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7D13,	E_ISDBT_STR_ID_HIROSHIMA_TELEVISION,		4,	0xB998,	0xB99F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7D14,	E_ISDBT_STR_ID_HIROSHIMA_HOME_TELEVISION,	5,	0xB9A0,	0xB9A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7D15,	E_ISDBT_STR_ID_TSS,							8,	0xB9A8,	0xB9AF	}
};

const T_BROADCASTER_INFO stBroadInfo_OKAYAMA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7D00,	E_ISDBT_STR_ID_NHK_G_OKAYAMA,	1,	0xBD80,	0xBD87	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7D01,	E_ISDBT_STR_ID_NHK_E_OKAYAMA,	2,	0xBD88,	0xBD8F	}
};

const T_BROADCASTER_INFO stBroadInfo_SHIMANE[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7CF0,	E_ISDBT_STR_ID_NHK_G_MATSUE,	3,	0xC180,	0xC187	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7CF1,	E_ISDBT_STR_ID_NHK_E_MATSUE,	2,	0xC188,	0xC18F	}
};

const T_BROADCASTER_INFO stBroadInfo_TOTTORI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7CE0,	E_ISDBT_STR_ID_NHK_G_TOTTORI,	3,	0xC580,	0xC587	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7CE1,	E_ISDBT_STR_ID_NHK_E_TOTTORI,	2,	0xC588,	0xC58F	}
};

const T_BROADCASTER_INFO stBroadInfo_YAMAGUCHI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7CD0,	E_ISDBT_STR_ID_NHK_G_YAMAGUCHI,				1,	0xC980,	0xC987	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7CD1,	E_ISDBT_STR_ID_NHK_E_YAMAGUCHI,				2,	0xC988,	0xC98F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7CD2,	E_ISDBT_STR_ID_KRY_YAMAGUCHI_BROADCASTING,	4,	0xC990,	0xC997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7CD3,	E_ISDBT_STR_ID_TYS_TV_YAMAGUCHI,			3,	0xC998,	0xC99F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7CD4,	E_ISDBT_STR_ID_YAB_YAMAGUCHI_ASAHI,			5,	0xC9A0,	0xC9A7	}
};

const T_BROADCASTER_INFO stBroadInfo_EHIME[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7CC0,	E_ISDBT_STR_ID_NHK_G_MATSUYAMA,			1,	0xCD80,	0xCD87	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7CC1,	E_ISDBT_STR_ID_NHK_E_MATSUYAMA,			2,	0xCD88,	0xCD8F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7CC2,	E_ISDBT_STR_ID_NANKAI_BROADCASTING,		4,	0xCD90,	0xCD97	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7CC3,	E_ISDBT_STR_ID_EHIME_ASAHI_TELEVISION,	5,	0xCD98,	0xCD9F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7CC4,	E_ISDBT_STR_ID_I_TELEVISION,			6,	0xCDA0,	0xCDA7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7CC5,	E_ISDBT_STR_ID_TELEVISION_EHIME,		8,	0xCDA8,	0xCDAF	}
};

const T_BROADCASTER_INFO stBroadInfo_KAGAWA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7CB0,	E_ISDBT_STR_ID_NHK_G_TAKAMATSU,	1,	0xD180,	0xD187	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7CB1,	E_ISDBT_STR_ID_NHK_E_TAKAMATSU,	2,	0xD188,	0xD18F	},
};

const T_BROADCASTER_INFO stBroadInfo_TOKUSHIMA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7CA0,	E_ISDBT_STR_ID_NHK_G_TOKUSHIMA,		3,	0xD580,	0xD587	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7CA1,	E_ISDBT_STR_ID_NHK_E_TOKUSHIMA,		2,	0xD588,	0xD58F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7CA2,	E_ISDBT_STR_ID_SHIKOKU_BROADCATING,	1,	0xD590,	0xD597	}
};

const T_BROADCASTER_INFO stBroadInfo_KOUCHI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7C90,	E_ISDBT_STR_ID_NHK_G_KOUCHI,		1,	0xD980,	0xD987	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7C91,	E_ISDBT_STR_ID_NHK_E_KOUCHI,		2,	0xD988,	0xD98F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7C92,	E_ISDBT_STR_ID_KOCHI_BROADCASTING,	4,	0xD990,	0xD997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7C93,	E_ISDBT_STR_ID_TELEVISION_KOCHI,	6,	0xD998,	0xD99F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7C94,	E_ISDBT_STR_ID_SUN_SUN_TELEVISION,	8,	0xD9A0,	0xD9A7	}
};

const T_BROADCASTER_INFO stBroadInfo_FUKUOKA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7C80,	E_ISDBT_STR_ID_NHK_G_FUKUOKA,					3,	0xDD80,	0xDD87	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7C81, E_ISDBT_STR_ID_NHK_E_FUKUOKA,					2,	0xDD88, 0xDD8F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7C82,	E_ISDBT_STR_ID_KBC_KYUSHU_ASAHI_BROADCASTING,	1,	0xDD90,	0xDD97	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7C83,	E_ISDBT_STR_ID_RKB_MAINICHI_BROADCASTING,		4,	0xDD98,	0xDD9F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7C84,	E_ISDBT_STR_ID_FBS_FUKUOKA_BROADCASTING,		5,	0xDDA0,	0xDDA7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7C85,	E_ISDBT_STR_ID_TVQ_KYUSHU_BROADCASTING,			7,	0xDDA8,	0xDDAF	},
	{	6,	E_ISDBT_STR_ID_NONE,	0x7C86,	E_ISDBT_STR_ID_TNC_TELEVISION_NISHINIPPON,		8,	0xDDB0,	0xDDB7	}
};

const T_BROADCASTER_INFO stBroadInfo_FUKUOKA2[] = {
	{	1,	E_ISDBT_STR_ID_NONE,	0x7881, E_ISDBT_STR_ID_NHK_G_KITAKYUSHU,				3,	0xDF80, 0xDF87	},
	{	0,	E_ISDBT_STR_ID_NONE,	0x7880, E_ISDBT_STR_ID_NHK_E_KITAKYUSHU,				2,	0xDF88, 0xDF8F	},
};

const T_BROADCASTER_INFO stBroadInfo_KUMAMOTO[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7C70,	E_ISDBT_STR_ID_NHK_G_KUMAMOTO,					1,	0xE180,	0xE187	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7C71,	E_ISDBT_STR_ID_NHK_E_KUMAMOTO,					2,	0xE188,	0xE18F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7C72,	E_ISDBT_STR_ID_RKK_KUMAMOTO_BROADCASTING,		3,	0xE190,	0xE197	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7C73,	E_ISDBT_STR_ID_TKU_TV_KUMAMOTO,					8,	0xE198,	0xE19F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7C74,	E_ISDBT_STR_ID_KKT_KUMAMATO_KENMIN,				4,	0xE1A0,	0xE1A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7C75,	E_ISDBT_STR_ID_KAB_KUMAMOTO_ASAHI_BROADCASTING,	5,	0xE1A8,	0xE1AF	}
};

const T_BROADCASTER_INFO stBroadInfo_NAGASAKI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7C60,	E_ISDBT_STR_ID_NHK_G_NAGASAKI,							1,	0xE580,	0xE587	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7C61,	E_ISDBT_STR_ID_NHK_E_NAGASAKI,							2,	0xE588,	0xE58F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7C62,	E_ISDBT_STR_ID_NBC_NAGASAKI_BROADCASTING,				3,	0xE590,	0xE597	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7C63,	E_ISDBT_STR_ID_KTN_TELEVISION_NAGASAKI,					8,	0xE598,	0xE59F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7C64,	E_ISDBT_STR_ID_NCC_NAGASAKI_CULTURE_TELECASTING,		5,	0xE5A0,	0xE5A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7C65,	E_ISDBT_STR_ID_NIB_NAGASAKI_INTERNATIONAL_TELEVISION,	4,	0xE5A8,	0xE5AF	}
};

const T_BROADCASTER_INFO stBroadInfo_KAGOSHIMA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7C50,	E_ISDBT_STR_ID_NHK_G_KAGOSHIMA,						3,	0xE980,	0xE987	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7C51,	E_ISDBT_STR_ID_NHK_E_KAGOSHIMA,						2,	0xE988,	0xE98F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7C52,	E_ISDBT_STR_ID_MBC_MINAMINIHON_BROADCASTING,		1,	0xE990,	0xE997	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7C53,	E_ISDBT_STR_ID_KTS_KAGOSHIMA_TELEVISION,			8,	0xE998,	0xE99F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7C54,	E_ISDBT_STR_ID_KKB_KAGOSHIMA_BROADCASTING,			5,	0xE9A0,	0xE9A7	},
	{	5,	E_ISDBT_STR_ID_NONE,	0x7C55,	E_ISDBT_STR_ID_KYT_KAGOSHIMA_YOMIURI_TELEVISION,	4,	0xE9A8,	0xE9AF	}
};

const T_BROADCASTER_INFO stBroadInfo_MIYAZAKI[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7C40,	E_ISDBT_STR_ID_NHK_G_MIYAZAKI,				1,	0xED80,	0xED87	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7C41,	E_ISDBT_STR_ID_NHK_E_MIYAZAKI,				2,	0xED88,	0xED8F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7C42,	E_ISDBT_STR_ID_MRT_MIYAZAKI_BROADCASTING,	6,	0xED90,	0xED97	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7C43,	E_ISDBT_STR_ID_UMK_MIYAZAKI_TELECASTING,	3,	0xED98,	0xED9F	}
};

const T_BROADCASTER_INFO stBroadInfo_OITA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7C30,	E_ISDBT_STR_ID_NHK_G_OITA,					1,	0xF180,	0xF187	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7C31,	E_ISDBT_STR_ID_NHK_E_OITA,					2,	0xF188,	0xF18F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7C32,	E_ISDBT_STR_ID_OBS_OITA_BROADCASTING,		3,	0xF190,	0xF197	},
	{	3,	E_ISDBT_STR_ID_NONE,	0x7C33,	E_ISDBT_STR_ID_TOS_TELEVISION_OITA,			4,	0xF198,	0xF19F	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7C34,	E_ISDBT_STR_ID_OAB_OITA_ASAHI_BROADCASTING,	5,	0xF1A0,	0xF1A7	}
};

const T_BROADCASTER_INFO stBroadInfo_SAGA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7C20,	E_ISDBT_STR_ID_NHK_G_SAGA,			1,	0xF580,	0xF587	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7C21,	E_ISDBT_STR_ID_NHK_E_SAGA,			2,	0xF588,	0xF58F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7C22,	E_ISDBT_STR_ID_STS_SAGA_TELEVISION,	3,	0xF590,	0xF597	}
};

const T_BROADCASTER_INFO stBroadInfo_OKINAWA[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0x7C10,	E_ISDBT_STR_ID_NHK_G_NAHA,						1,	0xF980,	0xF987	},
	{	1,	E_ISDBT_STR_ID_NONE,	0x7C11,	E_ISDBT_STR_ID_NHK_E_NAHA,						2,	0xF988,	0xF98F	},
	{	2,	E_ISDBT_STR_ID_NONE,	0x7C12,	E_ISDBT_STR_ID_RBC_RYUKYU_BROADCASTING,			3,	0xF990,	0xF997	},
	{	4,	E_ISDBT_STR_ID_NONE,	0x7C14,	E_ISDBT_STR_ID_QAB_RYUKYU_ASAHI_BROADCASTING,	5,	0xF9A0,	0xF9A7	},
	{	7,	E_ISDBT_STR_ID_NONE,	0x7C17,	E_ISDBT_STR_ID_OKINAWA_TELEVISION_OTV,			8,	0xF9B8,	0xF9BF	}
};

const T_BROADCASTER_INFO stBroadInfo_UNDEFINED_63[] =
{
	{	0,	E_ISDBT_STR_ID_NONE,	0xFFFF,	E_ISDBT_STR_ID_UNDEFINED,	0,	0xFFFF,	0xFFFF	}
};


/* ----------------------------------------------------------------------
**  Region ID Information
** ---------------------------------------------------------------------- */

#define BORADINFO_LIST_N_NUMS(a)		(T_BROADCASTER_INFO *)(a), (sizeof((a))/sizeof(T_BROADCASTER_INFO))

const T_REGION_ID_INFO stISDBT_RegionID_Info[E_REGION_ID_MAX] =
{
	{	E_ISDBT_STR_ID_UNDEFINED,			E_REGION_ID_UNDEFINED_0,		NULL,	0	},	// BORADINFO_LIST_N_NUMS(stBroadInfo_UNDEFINED_0)
	{	E_ISDBT_STR_ID_KANTO_WIDE_AREA,		E_REGION_ID_KANTO_WIDE_AREA,	BORADINFO_LIST_N_NUMS(stBroadInfo_KANTO_WIDE_AREA)		},
	{	E_ISDBT_STR_ID_KINKI_WIDE_AREA,		E_REGION_ID_KINKI_WIDE_AREA,	BORADINFO_LIST_N_NUMS(stBroadInfo_KINKI_WIDE_AREA)		},
	{	E_ISDBT_STR_ID_CHUKYO_WIDE_AREA,	E_REGION_ID_CHUKYO_WIDE_AREA,	BORADINFO_LIST_N_NUMS(stBroadInfo_CHUKYO_WIDE_AREA)		},
	{	E_ISDBT_STR_ID_HOKKAIDO_AREA,		E_REGION_ID_HOKKAIDO_AREA,		BORADINFO_LIST_N_NUMS(stBroadInfo_HOKKAIDO_AREA)		},
	{	E_ISDBT_STR_ID_OKAYAMA_N_KAGAWA,	E_REGION_ID_OKAYAMA_N_KAGAWA,	BORADINFO_LIST_N_NUMS(stBroadInfo_OKAYAMA_N_KAGAWA)		},
	{	E_ISDBT_STR_ID_SHIMANE_N_TOTTORI,	E_REGION_ID_SHIMANE_N_TOTTORI,	BORADINFO_LIST_N_NUMS(stBroadInfo_SHIMANE_N_TOTTORI)	},
	{	E_ISDBT_STR_ID_UNDEFINED,			E_REGION_ID_UNDEFINED_7,		NULL,	0	},	// BORADINFO_LIST_N_NUMS(stBroadInfo_UNDEFINED_7)
	{	E_ISDBT_STR_ID_UNDEFINED,			E_REGION_ID_UNDEFINED_8,		NULL,	0	},	// BORADINFO_LIST_N_NUMS(stBroadInfo_UNDEFINED_8)
#if 0
	{	E_ISDBT_STR_ID_UNDEFINED,			E_REGION_ID_UNDEFINED_9,		NULL,	0	},	// BORADINFO_LIST_N_NUMS(stBroadInfo_UNDEFINED_9)
#else
	{	E_ISDBT_STR_ID_FUKUOKA,				E_REGION_ID_UNDEFINED_9,		BORADINFO_LIST_N_NUMS(stBroadInfo_FUKUOKA2)				},
#endif
	/* ------------------------------ */

	{	E_ISDBT_STR_ID_HOKKAIDO_SAPPORO,	E_REGION_ID_HOKKAIDO_SAPPORO,	BORADINFO_LIST_N_NUMS(stBroadInfo_HOKKAIDO_SAPPORO)		},
	{	E_ISDBT_STR_ID_HOKKAIDO_HAKODATE,	E_REGION_ID_HOKKAIDO_HAKODATE,	BORADINFO_LIST_N_NUMS(stBroadInfo_HOKKAIDO_HAKODATE)	},
	{	E_ISDBT_STR_ID_HOKKAIDO_ASAHIKAWA,	E_REGION_ID_HOKKAIDO_ASAHIKAWA,	BORADINFO_LIST_N_NUMS(stBroadInfo_HOKKAIDO_ASAHIKAWA)	},
	{	E_ISDBT_STR_ID_HOKKAIDO_OBIHIRO,	E_REGION_ID_HOKKAIDO_OBIHIRO,	BORADINFO_LIST_N_NUMS(stBroadInfo_HOKKAIDO_OBIHIRO)		},
	{	E_ISDBT_STR_ID_HOKKAIDO_KUSHIRO,	E_REGION_ID_HOKKAIDO_KUSHIRO,	BORADINFO_LIST_N_NUMS(stBroadInfo_HOKKAIDO_KUSHIRO)		},
	{	E_ISDBT_STR_ID_HOKKAIDO_KITAMI,		E_REGION_ID_HOKKAIDO_KITAMI,	BORADINFO_LIST_N_NUMS(stBroadInfo_HOKKAIDO_KITAMI)		},
	{	E_ISDBT_STR_ID_HOKKAIDO_MURORAN,	E_REGION_ID_HOKKAIDO_MURORAN,	BORADINFO_LIST_N_NUMS(stBroadInfo_HOKKAIDO_MURORAN)		},
	{	E_ISDBT_STR_ID_MIYAGI,				E_REGION_ID_MIYAGI,				BORADINFO_LIST_N_NUMS(stBroadInfo_MIYAGI),				},
	{	E_ISDBT_STR_ID_AKITA,				E_REGION_ID_AKITA,				BORADINFO_LIST_N_NUMS(stBroadInfo_AKITA),				},
	{	E_ISDBT_STR_ID_YAMAGATA,			E_REGION_ID_YAMAGATA,			BORADINFO_LIST_N_NUMS(stBroadInfo_YAMAGATA),			},
	{	E_ISDBT_STR_ID_IWATE,				E_REGION_ID_IWATE,				BORADINFO_LIST_N_NUMS(stBroadInfo_IWATE),				},
	{	E_ISDBT_STR_ID_FUKUSHIMA,			E_REGION_ID_FUKUSHIMA,			BORADINFO_LIST_N_NUMS(stBroadInfo_FUKUSHIMA),			},
	{	E_ISDBT_STR_ID_AOMORI,				E_REGION_ID_AOMORI,				BORADINFO_LIST_N_NUMS(stBroadInfo_AOMORI),				},
	{	E_ISDBT_STR_ID_TOKYO,				E_REGION_ID_TOKYO,				BORADINFO_LIST_N_NUMS(stBroadInfo_TOKYO),				},
	{	E_ISDBT_STR_ID_KANAGAWA,			E_REGION_ID_KANAGAWA,			BORADINFO_LIST_N_NUMS(stBroadInfo_KANAGAWA),			},
	{	E_ISDBT_STR_ID_GUNMA,				E_REGION_ID_GUNMA,				BORADINFO_LIST_N_NUMS(stBroadInfo_GUNMA),				},
	{	E_ISDBT_STR_ID_IBARAKI,				E_REGION_ID_IBARAKI,			BORADINFO_LIST_N_NUMS(stBroadInfo_IBARAKI),				},
	{	E_ISDBT_STR_ID_CHIBA,				E_REGION_ID_CHIBA,				BORADINFO_LIST_N_NUMS(stBroadInfo_CHIBA),				},
	{	E_ISDBT_STR_ID_TOCHIGI,				E_REGION_ID_TOCHIGI,			BORADINFO_LIST_N_NUMS(stBroadInfo_TOCHIGI),				},
	{	E_ISDBT_STR_ID_SAITAMA,				E_REGION_ID_SAITAMA,			BORADINFO_LIST_N_NUMS(stBroadInfo_SAITAMA),				},
	{	E_ISDBT_STR_ID_NAGANO,				E_REGION_ID_NAGANO,				BORADINFO_LIST_N_NUMS(stBroadInfo_NAGANO),				},
	{	E_ISDBT_STR_ID_NIIGATA,				E_REGION_ID_NIIGATA,			BORADINFO_LIST_N_NUMS(stBroadInfo_NIIGATA),				},
	{	E_ISDBT_STR_ID_YAMANASHI,			E_REGION_ID_YAMANASHI,			BORADINFO_LIST_N_NUMS(stBroadInfo_YAMANASHI),			},
	{	E_ISDBT_STR_ID_AICHI,				E_REGION_ID_AICHI,				BORADINFO_LIST_N_NUMS(stBroadInfo_AICHI),				},
	{	E_ISDBT_STR_ID_ISHIKAWA,			E_REGION_ID_ISHIKAWA,			BORADINFO_LIST_N_NUMS(stBroadInfo_ISHIKAWA),			},
	{	E_ISDBT_STR_ID_SHIZUOKA,			E_REGION_ID_SHIZUOKA,			BORADINFO_LIST_N_NUMS(stBroadInfo_SHIZUOKA),			},
	{	E_ISDBT_STR_ID_FUKUI,				E_REGION_ID_FUKUI,				BORADINFO_LIST_N_NUMS(stBroadInfo_FUKUI),				},
	{	E_ISDBT_STR_ID_TOYAMA,				E_REGION_ID_TOYAMA,				BORADINFO_LIST_N_NUMS(stBroadInfo_TOYAMA),				},
	{	E_ISDBT_STR_ID_MIE,					E_REGION_ID_MIE,				BORADINFO_LIST_N_NUMS(stBroadInfo_MIE),					},
	{	E_ISDBT_STR_ID_GIFU,				E_REGION_ID_GIFU,				BORADINFO_LIST_N_NUMS(stBroadInfo_GIFU),				},
	{	E_ISDBT_STR_ID_OSAKA,				E_REGION_ID_OSAKA,				BORADINFO_LIST_N_NUMS(stBroadInfo_OSAKA),				},
	{	E_ISDBT_STR_ID_KYOTO,				E_REGION_ID_KYOTO,				BORADINFO_LIST_N_NUMS(stBroadInfo_KYOTO),				},
	{	E_ISDBT_STR_ID_HYOGO,				E_REGION_ID_HYOGO,				BORADINFO_LIST_N_NUMS(stBroadInfo_HYOGO),				},
	{	E_ISDBT_STR_ID_WAKAYAMA,			E_REGION_ID_WAKAYAMA,			BORADINFO_LIST_N_NUMS(stBroadInfo_WAKAYAMA),			},
	{	E_ISDBT_STR_ID_NARA,				E_REGION_ID_NARA,				BORADINFO_LIST_N_NUMS(stBroadInfo_NARA),				},
	{	E_ISDBT_STR_ID_SHIGA,				E_REGION_ID_SHIGA,				BORADINFO_LIST_N_NUMS(stBroadInfo_SHIGA),				},
	{	E_ISDBT_STR_ID_HIROSHIMA,			E_REGION_ID_HIROSHIMA,			BORADINFO_LIST_N_NUMS(stBroadInfo_HIROSHIMA),			},
	{	E_ISDBT_STR_ID_OKAYAMA,				E_REGION_ID_OKAYAMA,			BORADINFO_LIST_N_NUMS(stBroadInfo_OKAYAMA),				},
	{	E_ISDBT_STR_ID_SHIMANE,				E_REGION_ID_SHIMANE,			BORADINFO_LIST_N_NUMS(stBroadInfo_SHIMANE),				},
	{	E_ISDBT_STR_ID_TOTTORI,				E_REGION_ID_TOTTORI,			BORADINFO_LIST_N_NUMS(stBroadInfo_TOTTORI),				},
	{	E_ISDBT_STR_ID_YAMAGUCHI,			E_REGION_ID_YAMAGUCHI,			BORADINFO_LIST_N_NUMS(stBroadInfo_YAMAGUCHI),			},
	{	E_ISDBT_STR_ID_EHIME,				E_REGION_ID_EHIME,				BORADINFO_LIST_N_NUMS(stBroadInfo_EHIME),				},
	{	E_ISDBT_STR_ID_KAGAWA,				E_REGION_ID_KAGAWA,				BORADINFO_LIST_N_NUMS(stBroadInfo_KAGAWA),				},
	{	E_ISDBT_STR_ID_TOKUSHIMA,			E_REGION_ID_TOKUSHIMA,			BORADINFO_LIST_N_NUMS(stBroadInfo_TOKUSHIMA),			},
	{	E_ISDBT_STR_ID_KOUCHI,				E_REGION_ID_KOUCHI,				BORADINFO_LIST_N_NUMS(stBroadInfo_KOUCHI),				},
	{	E_ISDBT_STR_ID_FUKUOKA,				E_REGION_ID_FUKUOKA,			BORADINFO_LIST_N_NUMS(stBroadInfo_FUKUOKA),				},
	{	E_ISDBT_STR_ID_KUMAMOTO,			E_REGION_ID_KUMAMOTO,			BORADINFO_LIST_N_NUMS(stBroadInfo_KUMAMOTO),			},
	{	E_ISDBT_STR_ID_NAGASAKI,			E_REGION_ID_NAGASAKI,			BORADINFO_LIST_N_NUMS(stBroadInfo_NAGASAKI),			},
	{	E_ISDBT_STR_ID_KAGOSHIMA,			E_REGION_ID_KAGOSHIMA,			BORADINFO_LIST_N_NUMS(stBroadInfo_KAGOSHIMA),			},
	{	E_ISDBT_STR_ID_MIYAZAKI,			E_REGION_ID_MIYAZAKI,			BORADINFO_LIST_N_NUMS(stBroadInfo_MIYAZAKI),			},
	{	E_ISDBT_STR_ID_OITA,				E_REGION_ID_OITA,				BORADINFO_LIST_N_NUMS(stBroadInfo_OITA),				},
	{	E_ISDBT_STR_ID_SAGA,				E_REGION_ID_SAGA,				BORADINFO_LIST_N_NUMS(stBroadInfo_SAGA),				},
	{	E_ISDBT_STR_ID_OKINAWA,				E_REGION_ID_OKINAWA,			BORADINFO_LIST_N_NUMS(stBroadInfo_OKINAWA),				},
	{	E_ISDBT_STR_ID_UNDEFINED,			E_REGION_ID_UNDEFINED_63,		NULL,	0	}	// BORADINFO_LIST_N_NUMS(stBroadInfo_UNDEFINED_63)
};


/* ----------------------------------------------------------------------
**  Channel Information according to Region
** ---------------------------------------------------------------------- */

const T_CHANNEL_INFO stChInfo_HOKKAIDO_SAPPORO[] =
{
	{ E_REGION_ID_HOKKAIDO_SAPPORO,	1,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_HOKKAIDO_SAPPORO,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_HOKKAIDO_SAPPORO,	3,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_HOKKAIDO_SAPPORO,	5,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_HOKKAIDO_SAPPORO,	6,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_HOKKAIDO_SAPPORO,	7,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_HOKKAIDO_SAPPORO,	8,	E_ISDBT_FREQ_25 }
};

const T_CHANNEL_INFO stChInfo_HOKKAIDO_HAKODATE[] =
{
	{ E_REGION_ID_HOKKAIDO_HAKODATE,	1,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_HOKKAIDO_HAKODATE,	2,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_HOKKAIDO_HAKODATE,	3,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_HOKKAIDO_HAKODATE,	5,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_HOKKAIDO_HAKODATE,	6,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_HOKKAIDO_HAKODATE,	7,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_HOKKAIDO_HAKODATE,	8,	E_ISDBT_FREQ_25 }
};

const T_CHANNEL_INFO stChInfo_HOKKAIDO_ASAHIKAWA[] =
{
	{ E_REGION_ID_HOKKAIDO_ASAHIKAWA,	1,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_HOKKAIDO_ASAHIKAWA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_HOKKAIDO_ASAHIKAWA,	3,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_HOKKAIDO_ASAHIKAWA,	5,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_HOKKAIDO_ASAHIKAWA,	6,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_HOKKAIDO_ASAHIKAWA,	7,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_HOKKAIDO_ASAHIKAWA,	8,	E_ISDBT_FREQ_25 }
};

const T_CHANNEL_INFO stChInfo_HOKKAIDO_OBIHIRO[] =
{
	{ E_REGION_ID_HOKKAIDO_OBIHIRO,	1,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_HOKKAIDO_OBIHIRO,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_HOKKAIDO_OBIHIRO,	3,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_HOKKAIDO_OBIHIRO,	5,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_HOKKAIDO_OBIHIRO,	6,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_HOKKAIDO_OBIHIRO,	7,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_HOKKAIDO_OBIHIRO,	8,	E_ISDBT_FREQ_25 }
};

const T_CHANNEL_INFO stChInfo_HOKKAIDO_KUSHIRO[] =
{
	{ E_REGION_ID_HOKKAIDO_KUSHIRO,	1,	E_ISDBT_FREQ_45 },
	{ E_REGION_ID_HOKKAIDO_KUSHIRO,	2,	E_ISDBT_FREQ_29 },
	{ E_REGION_ID_HOKKAIDO_KUSHIRO,	3,	E_ISDBT_FREQ_33 },
	{ E_REGION_ID_HOKKAIDO_KUSHIRO,	5,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_HOKKAIDO_KUSHIRO,	6,	E_ISDBT_FREQ_36 },
	{ E_REGION_ID_HOKKAIDO_KUSHIRO,	7,	E_ISDBT_FREQ_41 },
	{ E_REGION_ID_HOKKAIDO_KUSHIRO,	8,	E_ISDBT_FREQ_43 }
};

const T_CHANNEL_INFO stChInfo_HOKKAIDO_KITAMI[] =
{
	{ E_REGION_ID_HOKKAIDO_KITAMI,	1,	E_ISDBT_FREQ_32 },
	{ E_REGION_ID_HOKKAIDO_KITAMI,	2,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_HOKKAIDO_KITAMI,	3,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_HOKKAIDO_KITAMI,	5,	E_ISDBT_FREQ_29 },
	{ E_REGION_ID_HOKKAIDO_KITAMI,	6,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_HOKKAIDO_KITAMI,	7,	E_ISDBT_FREQ_25 },
	{ E_REGION_ID_HOKKAIDO_KITAMI,	8,	E_ISDBT_FREQ_33 }
};

const T_CHANNEL_INFO stChInfo_HOKKAIDO_MURORAN[] =
{
	{ E_REGION_ID_HOKKAIDO_MURORAN,	1,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_HOKKAIDO_MURORAN,	2,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_HOKKAIDO_MURORAN,	3,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_HOKKAIDO_MURORAN,	5,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_HOKKAIDO_MURORAN,	6,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_HOKKAIDO_MURORAN,	7,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_HOKKAIDO_MURORAN,	8,	E_ISDBT_FREQ_33 }
};

const T_CHANNEL_INFO stChInfo_MIYAGI[] =
{
	{ E_REGION_ID_MIYAGI,	1,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_MIYAGI,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_MIYAGI,	3,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_MIYAGI,	4,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_MIYAGI,	5,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_MIYAGI,	8,	E_ISDBT_FREQ_21 }
};

const T_CHANNEL_INFO stChInfo_AKITA[] =
{
	{ E_REGION_ID_AKITA,	1,	E_ISDBT_FREQ_48 },
	{ E_REGION_ID_AKITA,	2,	E_ISDBT_FREQ_50 },
	{ E_REGION_ID_AKITA,	4,	E_ISDBT_FREQ_35 },
	{ E_REGION_ID_AKITA,	5,	E_ISDBT_FREQ_29 },
	{ E_REGION_ID_AKITA,	8,	E_ISDBT_FREQ_21 }
};

const T_CHANNEL_INFO stChInfo_YAMAGATA[] =
{
	{ E_REGION_ID_YAMAGATA,	1,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_YAMAGATA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_YAMAGATA,	4,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_YAMAGATA,	5,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_YAMAGATA,	6,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_YAMAGATA,	8,	E_ISDBT_FREQ_22 }
};

const T_CHANNEL_INFO stChInfo_IWATE[] =
{
	{ E_REGION_ID_IWATE,	1,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_IWATE,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_IWATE,	4,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_IWATE,	5,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_IWATE,	6,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_IWATE,	8,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_MIYAGI,	1,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_MIYAGI,	4,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_MIYAGI,	5,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_MIYAGI,	8,	E_ISDBT_FREQ_21 }
};

const T_CHANNEL_INFO stChInfo_FUKUSHIMA[] =
{
	{ E_REGION_ID_FUKUSHIMA,	1,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_FUKUSHIMA,	2,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_FUKUSHIMA,	4,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_FUKUSHIMA,	5,	E_ISDBT_FREQ_29 },
	{ E_REGION_ID_FUKUSHIMA,	6,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_FUKUSHIMA,	8,	E_ISDBT_FREQ_25 },
	{ E_REGION_ID_MIYAGI,	1,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_MIYAGI,	4,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_MIYAGI,	5,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_MIYAGI,	8,	E_ISDBT_FREQ_21 }
};

const T_CHANNEL_INFO stChInfo_AOMORI[] =
{
	{ E_REGION_ID_AOMORI,	1,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_AOMORI,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_AOMORI,	3,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_AOMORI,	5,	E_ISDBT_FREQ_32 },
	{ E_REGION_ID_AOMORI,	6,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_HOKKAIDO_HAKODATE,	6,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_HOKKAIDO_HAKODATE,	8,	E_ISDBT_FREQ_25 }
};

const T_CHANNEL_INFO stChInfo_TOKYO[] =
{
	{ E_REGION_ID_KANTO_WIDE_AREA,	1,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	2,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	4,	E_ISDBT_FREQ_25 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	5,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	6,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	7,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	8,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_TOKYO,		9,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	12,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_KANAGAWA, 	3,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_CHIBA,			3,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_SAITAMA,			3,	E_ISDBT_FREQ_32 }
};

const T_CHANNEL_INFO stChInfo_KANAGAWA[] =
{
	{ E_REGION_ID_KANTO_WIDE_AREA,	1,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	2,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_KANAGAWA,		3,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	4,	E_ISDBT_FREQ_25 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	5,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	6,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	7,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	8,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	12,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_TOKYO,		9,	E_ISDBT_FREQ_16 }
};

const T_CHANNEL_INFO stChInfo_GUNMA[] =
{
	{ E_REGION_ID_KANTO_WIDE_AREA,	1,	E_ISDBT_FREQ_37 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	2,	E_ISDBT_FREQ_39 },
	{ E_REGION_ID_GUNMA,			3,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	4,	E_ISDBT_FREQ_33 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	5,	E_ISDBT_FREQ_43 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	6,	E_ISDBT_FREQ_36 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	7,	E_ISDBT_FREQ_45 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	8,	E_ISDBT_FREQ_42 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	12,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_SAITAMA,			3,	E_ISDBT_FREQ_32 }
};

const T_CHANNEL_INFO stChInfo_IBARAKI[] =
{
	{ E_REGION_ID_IBARAKI,			1,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	4,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	5,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	7,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	8,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_TOKYO,			9,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	12,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_CHIBA,			3,	E_ISDBT_FREQ_30 }
};

const T_CHANNEL_INFO stChInfo_CHIBA[] =
{
	{ E_REGION_ID_KANTO_WIDE_AREA,	1,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	2,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_CHIBA,			3,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	4,	E_ISDBT_FREQ_25 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	5,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	6,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	7,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	8,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	12,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_KANAGAWA, 		3,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_SAITAMA,			3,	E_ISDBT_FREQ_32 },
	{ E_REGION_ID_TOKYO,		9,	E_ISDBT_FREQ_16 }
};

const T_CHANNEL_INFO stChInfo_TOCHIGI[] =
{
	{ E_REGION_ID_KANTO_WIDE_AREA,	1,	E_ISDBT_FREQ_47 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	2,	E_ISDBT_FREQ_39 },
	{ E_REGION_ID_TOCHIGI,			3,	E_ISDBT_FREQ_29 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	4,	E_ISDBT_FREQ_34 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	5,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	7,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	8,	E_ISDBT_FREQ_35 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	12,	E_ISDBT_FREQ_28 }
};

const T_CHANNEL_INFO stChInfo_SAITAMA[] =
{
	{ E_REGION_ID_KANTO_WIDE_AREA,	1,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	2,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_SAITAMA,			3,	E_ISDBT_FREQ_32 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	4,	E_ISDBT_FREQ_25 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	5,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	6,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	7,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	8,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	12,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_GUNMA,			3,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_CHIBA,			3,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_TOKYO,		9,	E_ISDBT_FREQ_16 }
};

const T_CHANNEL_INFO stChInfo_NAGANO[] =
{
	{ E_REGION_ID_NAGANO,	1,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_NAGANO,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_NAGANO,	4,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_NAGANO,	5,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_NAGANO,	6,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_NAGANO,	8,	E_ISDBT_FREQ_15 }
};

const T_CHANNEL_INFO stChInfo_NIIGATA[] =
{
	{ E_REGION_ID_NIIGATA,	1,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_NIIGATA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_NIIGATA,	4,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_NIIGATA,	5,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_NIIGATA,	6,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_NIIGATA,	8,	E_ISDBT_FREQ_19 }
};

const T_CHANNEL_INFO stChInfo_YAMANASHI[] =
{
	{ E_REGION_ID_YAMANASHI,	1,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_YAMANASHI,	2,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_YAMANASHI,	4,	E_ISDBT_FREQ_25 },
	{ E_REGION_ID_YAMANASHI,	6,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	4,	E_ISDBT_FREQ_29 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	5,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	6,	E_ISDBT_FREQ_45 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	7,	E_ISDBT_FREQ_44 },
	{ E_REGION_ID_KANTO_WIDE_AREA,	8,	E_ISDBT_FREQ_38 }
};

const T_CHANNEL_INFO stChInfo_AICHI[] =
{
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	1,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_AICHI,			3,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	4,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	5,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	6,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_AICHI,			10,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_MIE,				7,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_GIFU,				8,	E_ISDBT_FREQ_30	}
};

const T_CHANNEL_INFO stChInfo_ISHIKAWA[] =
{
	{ E_REGION_ID_ISHIKAWA,	1,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_ISHIKAWA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_ISHIKAWA,	4,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_ISHIKAWA,	5,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_ISHIKAWA,	6,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_ISHIKAWA,	8,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_TOYAMA,	1,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_TOYAMA,	8,	E_ISDBT_FREQ_18	}
};

const T_CHANNEL_INFO stChInfo_SHIZUOKA[] =
{
	{ E_REGION_ID_SHIZUOKA,	1,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_SHIZUOKA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_SHIZUOKA,	4,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_SHIZUOKA,	5,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_SHIZUOKA,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_SHIZUOKA,	8,	E_ISDBT_FREQ_17 }
};

const T_CHANNEL_INFO stChInfo_FUKUI[] =
{
	{ E_REGION_ID_FUKUI,	1,	E_ISDBT_FREQ_19	},
	{ E_REGION_ID_FUKUI,	2,	E_ISDBT_FREQ_21	},
	{ E_REGION_ID_FUKUI,	7,	E_ISDBT_FREQ_20	},
	{ E_REGION_ID_FUKUI,	8,	E_ISDBT_FREQ_22	},
	{ E_REGION_ID_ISHIKAWA, 6,	E_ISDBT_FREQ_14 },
};

const T_CHANNEL_INFO stChInfo_TOYAMA[] =
{
	{ E_REGION_ID_TOYAMA,	1,	E_ISDBT_FREQ_28	},
	{ E_REGION_ID_TOYAMA,	2,	E_ISDBT_FREQ_24	},
	{ E_REGION_ID_TOYAMA,	3,	E_ISDBT_FREQ_27	},
	{ E_REGION_ID_TOYAMA,	6,	E_ISDBT_FREQ_22	},
	{ E_REGION_ID_TOYAMA,	8,	E_ISDBT_FREQ_18	},
	{ E_REGION_ID_ISHIKAWA, 6,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_ISHIKAWA,	8,	E_ISDBT_FREQ_16 }
};

const T_CHANNEL_INFO stChInfo_MIE[] =
{
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	1,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	2,	E_ISDBT_FREQ_44 },
	{ E_REGION_ID_MIE,				3,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	4,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	5,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	6,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_MIE,				7,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	4,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	8,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	10, E_ISDBT_FREQ_14 },
	{ E_REGION_ID_AICHI,			10, E_ISDBT_FREQ_23 }
};

const T_CHANNEL_INFO stChInfo_GIFU[] =
{
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	1,	E_ISDBT_FREQ_21	},
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	2,	E_ISDBT_FREQ_13	},
	{ E_REGION_ID_GIFU,				3,	E_ISDBT_FREQ_29	},
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	4,	E_ISDBT_FREQ_19	},
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	5,	E_ISDBT_FREQ_18	},
	{ E_REGION_ID_CHUKYO_WIDE_AREA,	6,	E_ISDBT_FREQ_22	},
	{ E_REGION_ID_GIFU,				8,	E_ISDBT_FREQ_30	},
	{ E_REGION_ID_MIE,				7,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_AICHI,			10,	E_ISDBT_FREQ_23 }
};

const T_CHANNEL_INFO stChInfo_OSAKA[] =
{
	{ E_REGION_ID_OSAKA,			1,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	4,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_OSAKA,			7,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	8,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	10,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_HYOGO,			3,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_KYOTO,			5,	E_ISDBT_FREQ_23 }
};

const T_CHANNEL_INFO stChInfo_KYOTO[] =
{
	{ E_REGION_ID_KYOTO,			1,	E_ISDBT_FREQ_25 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	2,	E_ISDBT_FREQ_40 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	4,	E_ISDBT_FREQ_33 },
	{ E_REGION_ID_KYOTO,			5,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	6,	E_ISDBT_FREQ_38 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	8,	E_ISDBT_FREQ_42 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	10,	E_ISDBT_FREQ_35 },
	{ E_REGION_ID_HYOGO,			3,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_OSAKA,			7,	E_ISDBT_FREQ_18 }
};

const T_CHANNEL_INFO stChInfo_HYOGO[] =
{
	{ E_REGION_ID_HYOGO,			1,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_HYOGO,			3,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	4,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	8,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	10,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_OSAKA,			7,	E_ISDBT_FREQ_18 }
};

const T_CHANNEL_INFO stChInfo_WAKAYAMA[] =
{
	{ E_REGION_ID_WAKAYAMA,			1,	E_ISDBT_FREQ_23 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	4,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_WAKAYAMA,			5,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	8,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	10,	E_ISDBT_FREQ_14 }
};

const T_CHANNEL_INFO stChInfo_NARA[] =
{
	{ E_REGION_ID_NARA,				1,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	4,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	8,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_NARA,				9,	E_ISDBT_FREQ_29 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	10,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_HYOGO,			3,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_OSAKA,			7,	E_ISDBT_FREQ_18 }
};

const T_CHANNEL_INFO stChInfo_SHIGA[] =
{
	{ E_REGION_ID_SHIGA,			1,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_SHIGA,			3,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	4,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	8,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	10,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_KYOTO,			5,	E_ISDBT_FREQ_23 },
};

const T_CHANNEL_INFO stChInfo_HIROSHIMA[] =
{
	{ E_REGION_ID_HIROSHIMA,	1,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_HIROSHIMA,	2,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_HIROSHIMA,	3,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_HIROSHIMA,	4,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_HIROSHIMA,	5,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_HIROSHIMA,	8,	E_ISDBT_FREQ_23 }
};

const T_CHANNEL_INFO stChInfo_OKAYAMA[] =
{
	{ E_REGION_ID_OKAYAMA,			1,	E_ISDBT_FREQ_32 },
	{ E_REGION_ID_OKAYAMA,			2,	E_ISDBT_FREQ_45 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	4,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	5,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	6,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	7,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	8,	E_ISDBT_FREQ_27 }
};

const T_CHANNEL_INFO stChInfo_SHIMANE[] =
{
	{ E_REGION_ID_SHIMANE_N_TOTTORI,	1,	E_ISDBT_FREQ_41 },
	{ E_REGION_ID_SHIMANE,				2,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_SHIMANE,				3,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_SHIMANE_N_TOTTORI,	6,	E_ISDBT_FREQ_45 },
	{ E_REGION_ID_SHIMANE_N_TOTTORI,	8,	E_ISDBT_FREQ_43 }
};

const T_CHANNEL_INFO stChInfo_TOTTORI[] =
{
	{ E_REGION_ID_SHIMANE_N_TOTTORI,	1,	E_ISDBT_FREQ_38 },
	{ E_REGION_ID_TOTTORI,				2,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_TOTTORI,				3,	E_ISDBT_FREQ_29 },
	{ E_REGION_ID_SHIMANE_N_TOTTORI,	6,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_SHIMANE_N_TOTTORI,	8,	E_ISDBT_FREQ_36 }
};

const T_CHANNEL_INFO stChInfo_YAMAGUCHI[] =
{
	{ E_REGION_ID_YAMAGUCHI,	1,	E_ISDBT_FREQ_37 },
	{ E_REGION_ID_YAMAGUCHI,	2,	E_ISDBT_FREQ_40 },
	{ E_REGION_ID_YAMAGUCHI,	3,	E_ISDBT_FREQ_34 },
	{ E_REGION_ID_YAMAGUCHI,	4,	E_ISDBT_FREQ_36 },
	{ E_REGION_ID_YAMAGUCHI,	5,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_FUKUOKA,	1,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_OITA,	3,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_FUKUOKA,	4,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_FUKUOKA,	5,	E_ISDBT_FREQ_32 },
	{ E_REGION_ID_FUKUOKA,	7,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_FUKUOKA,	8,	E_ISDBT_FREQ_29 },
};

const T_CHANNEL_INFO stChInfo_EHIME[] =
{
	{ E_REGION_ID_EHIME,	1,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_EHIME,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_EHIME,	4,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_EHIME,	5,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_EHIME,	6,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_EHIME,	8,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	4,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_HIROSHIMA,	4,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_HIROSHIMA,	5,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	6,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	7,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_HIROSHIMA,	8,	E_ISDBT_FREQ_23 }
};

const T_CHANNEL_INFO stChInfo_KAGAWA[] =
{
	{ E_REGION_ID_KAGAWA,			1,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_KAGAWA,			2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	4,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	5,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	6,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	7,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_OKAYAMA_N_KAGAWA,	8,	E_ISDBT_FREQ_27 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	4,	E_ISDBT_FREQ_48 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	6,	E_ISDBT_FREQ_44 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	8,	E_ISDBT_FREQ_39 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	10, E_ISDBT_FREQ_33 }
};

const T_CHANNEL_INFO stChInfo_TOKUSHIMA[] =
{
	{ E_REGION_ID_TOKUSHIMA,	1,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_TOKUSHIMA,	2,	E_ISDBT_FREQ_40 },
	{ E_REGION_ID_TOKUSHIMA,	3,	E_ISDBT_FREQ_34 },
	{ E_REGION_ID_HYOGO,			3,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	4,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_WAKAYAMA,			5,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_OSAKA,			7,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	8,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_KINKI_WIDE_AREA,	10, E_ISDBT_FREQ_14 }
};

const T_CHANNEL_INFO stChInfo_KOUCHI[] =
{
	{ E_REGION_ID_KOUCHI,	1,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_KOUCHI,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_KOUCHI,	4,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_KOUCHI,	6,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_KOUCHI,	8,	E_ISDBT_FREQ_21 }
};

const T_CHANNEL_INFO stChInfo_FUKUOKA[] =
{
	{ E_REGION_ID_FUKUOKA,	1,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_FUKUOKA,	2,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_FUKUOKA,	3,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_FUKUOKA,	4,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_FUKUOKA,	5,	E_ISDBT_FREQ_32 },
	{ E_REGION_ID_FUKUOKA,	7,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_FUKUOKA,	8,	E_ISDBT_FREQ_34 },
	{ E_REGION_ID_UNDEFINED_9,	2,	E_ISDBT_FREQ_42 },
	{ E_REGION_ID_UNDEFINED_9,	3,	E_ISDBT_FREQ_40 },
	{ E_REGION_ID_KUMAMOTO,	3,	E_ISDBT_FREQ_41 },
	{ E_REGION_ID_SAGA,	3,	E_ISDBT_FREQ_44 }
};

const T_CHANNEL_INFO stChInfo_KUMAMOTO[] =
{
	{ E_REGION_ID_KUMAMOTO,	1,	E_ISDBT_FREQ_28 },
	{ E_REGION_ID_KUMAMOTO,	2,	E_ISDBT_FREQ_24 },
	{ E_REGION_ID_KUMAMOTO,	3,	E_ISDBT_FREQ_41 },
	{ E_REGION_ID_KUMAMOTO,	4,	E_ISDBT_FREQ_47 },
	{ E_REGION_ID_KUMAMOTO,	5,	E_ISDBT_FREQ_49 },
	{ E_REGION_ID_KUMAMOTO,	8,	E_ISDBT_FREQ_42 },
	{ E_REGION_ID_FUKUOKA,	1,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_SAGA,	3,	E_ISDBT_FREQ_44 },
	{ E_REGION_ID_FUKUOKA,	4,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_FUKUOKA,	7,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_NAGASAKI,	8,	E_ISDBT_FREQ_20 }
};

const T_CHANNEL_INFO stChInfo_NAGASAKI[] =
{
	{ E_REGION_ID_NAGASAKI,	1,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_NAGASAKI,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_NAGASAKI,	3,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_NAGASAKI,	4,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_NAGASAKI,	5,	E_ISDBT_FREQ_19 },
	{ E_REGION_ID_NAGASAKI,	8,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_FUKUOKA,	1,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_KUMAMOTO,	3,	E_ISDBT_FREQ_41 },
	{ E_REGION_ID_FUKUOKA,	4,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_KUMAMOTO,	4,	E_ISDBT_FREQ_47 },
	{ E_REGION_ID_FUKUOKA,	8,	E_ISDBT_FREQ_34 },
	{ E_REGION_ID_KUMAMOTO,	8,	E_ISDBT_FREQ_42 }
};

const T_CHANNEL_INFO stChInfo_KAGOSHIMA[] =
{
	{ E_REGION_ID_KAGOSHIMA,	1,	E_ISDBT_FREQ_40 },
	{ E_REGION_ID_KAGOSHIMA,	2,	E_ISDBT_FREQ_18 },
	{ E_REGION_ID_KAGOSHIMA,	3,	E_ISDBT_FREQ_34 },
	{ E_REGION_ID_KAGOSHIMA,	4,	E_ISDBT_FREQ_29 },
	{ E_REGION_ID_KAGOSHIMA,	5,	E_ISDBT_FREQ_36 },
	{ E_REGION_ID_KAGOSHIMA,	8,	E_ISDBT_FREQ_42 },
	{ E_REGION_ID_MIYAZAKI,	3,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_KUMAMOTO,	4,	E_ISDBT_FREQ_47 },
	{ E_REGION_ID_KUMAMOTO,	5,	E_ISDBT_FREQ_49 },
	{ E_REGION_ID_MIYAZAKI,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_NAGASAKI,	8,	E_ISDBT_FREQ_20 }
};

const T_CHANNEL_INFO stChInfo_MIYAZAKI[] =
{
	{ E_REGION_ID_MIYAZAKI,	1,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_MIYAZAKI,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_MIYAZAKI,	3,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_MIYAZAKI,	6,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_KAGOSHIMA,	1,	E_ISDBT_FREQ_40 },
	{ E_REGION_ID_KAGOSHIMA,	5,	E_ISDBT_FREQ_36 },
	{ E_REGION_ID_KAGOSHIMA,	8,	E_ISDBT_FREQ_42 }
};

const T_CHANNEL_INFO stChInfo_OITA[] =
{
	{ E_REGION_ID_OITA,	1,	E_ISDBT_FREQ_15 },
	{ E_REGION_ID_OITA,	2,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_OITA,	3,	E_ISDBT_FREQ_22 },
	{ E_REGION_ID_OITA,	4,	E_ISDBT_FREQ_34 },
	{ E_REGION_ID_OITA,	5,	E_ISDBT_FREQ_32 },
	{ E_REGION_ID_FUKUOKA,	1,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_EHIME,	4,	E_ISDBT_FREQ_20 },
	{ E_REGION_ID_FUKUOKA,	4,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_FUKUOKA,	5,	E_ISDBT_FREQ_21 },
	{ E_REGION_ID_FUKUOKA,	7,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_FUKUOKA,	8,	E_ISDBT_FREQ_29 }
};

const T_CHANNEL_INFO stChInfo_SAGA[] =
{
	{ E_REGION_ID_SAGA,	1,	E_ISDBT_FREQ_33 },
	{ E_REGION_ID_SAGA,	2,	E_ISDBT_FREQ_25 },
	{ E_REGION_ID_SAGA,	3,	E_ISDBT_FREQ_44 },
	{ E_REGION_ID_FUKUOKA,	1,	E_ISDBT_FREQ_31 },
	{ E_REGION_ID_KUMAMOTO,	3,	E_ISDBT_FREQ_41 },
	{ E_REGION_ID_NAGASAKI,	3,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_FUKUOKA,	4,	E_ISDBT_FREQ_30 },
	{ E_REGION_ID_FUKUOKA,	5,	E_ISDBT_FREQ_32 },
	{ E_REGION_ID_FUKUOKA,	7,	E_ISDBT_FREQ_26 },
	{ E_REGION_ID_FUKUOKA,	8,	E_ISDBT_FREQ_34 },
	{ E_REGION_ID_KUMAMOTO, 8,	E_ISDBT_FREQ_42 },
	{ E_REGION_ID_NAGASAKI, 8,	E_ISDBT_FREQ_20 }
};

const T_CHANNEL_INFO stChInfo_OKINAWA[] =
{
	{ E_REGION_ID_OKINAWA,	1,	E_ISDBT_FREQ_17 },
	{ E_REGION_ID_OKINAWA,	2,	E_ISDBT_FREQ_13 },
	{ E_REGION_ID_OKINAWA,	3,	E_ISDBT_FREQ_14 },
	{ E_REGION_ID_OKINAWA,	5,	E_ISDBT_FREQ_16 },
	{ E_REGION_ID_OKINAWA,	8,	E_ISDBT_FREQ_15 }
};


/* ----------------------------------------------------------------------
**  Region Information
** ---------------------------------------------------------------------- */

#define CHINFO_LIST_N_NUMS(a)			(T_CHANNEL_INFO *)(a), (sizeof((a))/sizeof(T_CHANNEL_INFO))

const T_REGION_INFO stISDBT_RegionInfo[E_REGION_MAX] =
{
	{	E_ISDBT_STR_ID_HOKKAIDO_SAPPORO,	CHINFO_LIST_N_NUMS(stChInfo_HOKKAIDO_SAPPORO)	},
	{	E_ISDBT_STR_ID_HOKKAIDO_HAKODATE,	CHINFO_LIST_N_NUMS(stChInfo_HOKKAIDO_HAKODATE)	},
	{	E_ISDBT_STR_ID_HOKKAIDO_ASAHIKAWA,	CHINFO_LIST_N_NUMS(stChInfo_HOKKAIDO_ASAHIKAWA)	},
	{	E_ISDBT_STR_ID_HOKKAIDO_OBIHIRO,	CHINFO_LIST_N_NUMS(stChInfo_HOKKAIDO_OBIHIRO)	},
	{	E_ISDBT_STR_ID_HOKKAIDO_KUSHIRO,	CHINFO_LIST_N_NUMS(stChInfo_HOKKAIDO_KUSHIRO)	},
	{	E_ISDBT_STR_ID_HOKKAIDO_KITAMI,		CHINFO_LIST_N_NUMS(stChInfo_HOKKAIDO_KITAMI)	},
	{	E_ISDBT_STR_ID_HOKKAIDO_MURORAN,	CHINFO_LIST_N_NUMS(stChInfo_HOKKAIDO_MURORAN)	},
	{	E_ISDBT_STR_ID_MIYAGI,				CHINFO_LIST_N_NUMS(stChInfo_MIYAGI)				},
	{	E_ISDBT_STR_ID_AKITA,				CHINFO_LIST_N_NUMS(stChInfo_AKITA)				},
	{	E_ISDBT_STR_ID_YAMAGATA,			CHINFO_LIST_N_NUMS(stChInfo_YAMAGATA)			},
	{	E_ISDBT_STR_ID_IWATE,				CHINFO_LIST_N_NUMS(stChInfo_IWATE)				},
	{	E_ISDBT_STR_ID_FUKUSHIMA,			CHINFO_LIST_N_NUMS(stChInfo_FUKUSHIMA)			},
	{	E_ISDBT_STR_ID_AOMORI,				CHINFO_LIST_N_NUMS(stChInfo_AOMORI)				},
	{	E_ISDBT_STR_ID_TOKYO,				CHINFO_LIST_N_NUMS(stChInfo_TOKYO)				},
	{	E_ISDBT_STR_ID_KANAGAWA,			CHINFO_LIST_N_NUMS(stChInfo_KANAGAWA)			},
	{	E_ISDBT_STR_ID_GUNMA,				CHINFO_LIST_N_NUMS(stChInfo_GUNMA)				},
	{	E_ISDBT_STR_ID_IBARAKI,				CHINFO_LIST_N_NUMS(stChInfo_IBARAKI)			},
	{	E_ISDBT_STR_ID_CHIBA,				CHINFO_LIST_N_NUMS(stChInfo_CHIBA)				},
	{	E_ISDBT_STR_ID_TOCHIGI,				CHINFO_LIST_N_NUMS(stChInfo_TOCHIGI)			},
	{	E_ISDBT_STR_ID_SAITAMA,				CHINFO_LIST_N_NUMS(stChInfo_SAITAMA)			},
	{	E_ISDBT_STR_ID_NAGANO,				CHINFO_LIST_N_NUMS(stChInfo_NAGANO)				},
	{	E_ISDBT_STR_ID_NIIGATA,				CHINFO_LIST_N_NUMS(stChInfo_NIIGATA)			},
	{	E_ISDBT_STR_ID_YAMANASHI,			CHINFO_LIST_N_NUMS(stChInfo_YAMANASHI)			},
	{	E_ISDBT_STR_ID_AICHI,				CHINFO_LIST_N_NUMS(stChInfo_AICHI)				},
	{	E_ISDBT_STR_ID_ISHIKAWA,			CHINFO_LIST_N_NUMS(stChInfo_ISHIKAWA)			},
	{	E_ISDBT_STR_ID_SHIZUOKA,			CHINFO_LIST_N_NUMS(stChInfo_SHIZUOKA)			},
	{	E_ISDBT_STR_ID_FUKUI,				CHINFO_LIST_N_NUMS(stChInfo_FUKUI)				},
	{	E_ISDBT_STR_ID_TOYAMA,				CHINFO_LIST_N_NUMS(stChInfo_TOYAMA)				},
	{	E_ISDBT_STR_ID_MIE,					CHINFO_LIST_N_NUMS(stChInfo_MIE)				},
	{	E_ISDBT_STR_ID_GIFU,				CHINFO_LIST_N_NUMS(stChInfo_GIFU)				},
	{	E_ISDBT_STR_ID_OSAKA,				CHINFO_LIST_N_NUMS(stChInfo_OSAKA)				},
	{	E_ISDBT_STR_ID_KYOTO,				CHINFO_LIST_N_NUMS(stChInfo_KYOTO)				},
	{	E_ISDBT_STR_ID_HYOGO,				CHINFO_LIST_N_NUMS(stChInfo_HYOGO)				},
	{	E_ISDBT_STR_ID_WAKAYAMA,			CHINFO_LIST_N_NUMS(stChInfo_WAKAYAMA)			},
	{	E_ISDBT_STR_ID_NARA,				CHINFO_LIST_N_NUMS(stChInfo_NARA)				},
	{	E_ISDBT_STR_ID_SHIGA,				CHINFO_LIST_N_NUMS(stChInfo_SHIGA)				},
	{	E_ISDBT_STR_ID_HIROSHIMA,			CHINFO_LIST_N_NUMS(stChInfo_HIROSHIMA)			},
	{	E_ISDBT_STR_ID_OKAYAMA,				CHINFO_LIST_N_NUMS(stChInfo_OKAYAMA)			},
	{	E_ISDBT_STR_ID_SHIMANE,				CHINFO_LIST_N_NUMS(stChInfo_SHIMANE)			},
	{	E_ISDBT_STR_ID_TOTTORI,				CHINFO_LIST_N_NUMS(stChInfo_TOTTORI)			},
	{	E_ISDBT_STR_ID_YAMAGUCHI,			CHINFO_LIST_N_NUMS(stChInfo_YAMAGUCHI)			},
	{	E_ISDBT_STR_ID_EHIME,				CHINFO_LIST_N_NUMS(stChInfo_EHIME)				},
	{	E_ISDBT_STR_ID_KAGAWA,				CHINFO_LIST_N_NUMS(stChInfo_KAGAWA)				},
	{	E_ISDBT_STR_ID_TOKUSHIMA,			CHINFO_LIST_N_NUMS(stChInfo_TOKUSHIMA)			},
	{	E_ISDBT_STR_ID_KOUCHI,				CHINFO_LIST_N_NUMS(stChInfo_KOUCHI)				},
	{	E_ISDBT_STR_ID_FUKUOKA,				CHINFO_LIST_N_NUMS(stChInfo_FUKUOKA)			},
	{	E_ISDBT_STR_ID_KUMAMOTO,			CHINFO_LIST_N_NUMS(stChInfo_KUMAMOTO)			},
	{	E_ISDBT_STR_ID_NAGASAKI,			CHINFO_LIST_N_NUMS(stChInfo_NAGASAKI)			},
	{	E_ISDBT_STR_ID_KAGOSHIMA,			CHINFO_LIST_N_NUMS(stChInfo_KAGOSHIMA)			},
	{	E_ISDBT_STR_ID_MIYAZAKI,			CHINFO_LIST_N_NUMS(stChInfo_MIYAZAKI)			},
	{	E_ISDBT_STR_ID_OITA,				CHINFO_LIST_N_NUMS(stChInfo_OITA)				},
	{	E_ISDBT_STR_ID_SAGA,				CHINFO_LIST_N_NUMS(stChInfo_SAGA)				},
	{	E_ISDBT_STR_ID_OKINAWA,				CHINFO_LIST_N_NUMS(stChInfo_OKINAWA)			}
};


/* Preset Information */
T_PRESET_INFO stISDBT_PresetInfo = { E_PRESET_LIST_1ST, };

static int	giISDBT_Preset_CountryCode = 0;	//0 - Japan, 1 - Brazil

/* ======================================================================
**  GLOBAL FUNCTION
** ====================================================================== */

/**************************************************************************
*  
*  FUNCTION NAME
*      int ISDBT_Set_StringLanguage(unsigned char uiStrLengType);
*  
*  DESCRIPTION 
*      This function sets the string lenguage type.
*  
*  INPUT
*      uiStrLengType = String lenguage type.
*  
*  OUTPUT
*      int = Return type.
*  
*  REMARK
*      None
**************************************************************************/
int ISDBT_Set_StringLanguage(unsigned char uiStrLengType)
{
	/* Check the input string language type. */
	if (uiStrLengType >= E_STR_LANG_MAX)
	{
		return (0);
	}

	/* Set new string language type. */
	StringLanguageType = uiStrLengType;

	return (1);
}


/**************************************************************************
*  
*  FUNCTION NAME
*      unsigned char ISDBT_Get_StringLanguage(void);
*  
*  DESCRIPTION 
*      This function returns the current string lenguage type.
*  
*  INPUT
*      None
*  
*  OUTPUT
*      unsigned char = Current string lenguage type.
*  
*  REMARK
*      None
**************************************************************************/
unsigned char ISDBT_Get_StringLanguage(void)
{
	return StringLanguageType;
}


/**************************************************************************
*  
*  FUNCTION NAME
*      unsigned char *ISDBT_Get_String(U32_STRING_ID str_id);
*  
*  DESCRIPTION 
*      This function returns the pointer of string according to input string ID.
*  
*  INPUT
*      str_id = String ID
*  
*  OUTPUT
*      unsigned char * - Pointer of string searched.
*  
*  REMARK
*      None
**************************************************************************/
unsigned char *ISDBT_Get_String(U32_STRING_ID str_id)
{
	unsigned char	*pucString = NULL;
	U32_STRING_ID	uiLBID;	// Left Boundary ID
	U32_STRING_ID	uiRBID;	// Right Boundary ID
	U32_STRING_ID	uiMID;		// Middle ID

	/* Check the valid string ID. */
	if (str_id == E_ISDBT_STR_ID_NONE)
	{
		return NULL;
	}
	else if (str_id >= E_ISDBT_STR_ID_LAST)
	{
		return NULL;
	}

	/* Set the initial ID of left/right boundary. */
	uiLBID = E_ISDBT_STR_ID_NONE;
	uiRBID = E_ISDBT_STR_ID_LAST;

	/* Search the string ID which is equal to input string ID. */
	while (1)
	{
		/* Calculate the middle ID between LBID and RBID. */
		uiMID = (uiLBID + uiRBID) / 2;
		/* Compare the input ID with searched ID. */
		if (stISDBT_StringDB[uiMID].string_id == str_id)
		{
			/* Store the pointer of string which is searched. */
			pucString = (unsigned char *)(stISDBT_StringDB[uiMID].p_string_data[StringLanguageType]);

			/* Go out of while statement. */
			break;
		}

		/* Set the next ID of left/right boundary. */
		if (stISDBT_StringDB[uiMID].string_id > str_id)
		{
			uiRBID = uiMID;
		}
		else // if (stISDBT_StringDB[uiMID].string_id < str_id)
		{
			uiLBID = uiMID;
		}

		/* Failed in searching the string ID. */
		if (uiLBID >= uiRBID)
		{
			break;
		}
	}

	/* Return the pointer of string. */
	return pucString;
}


int ISDBT_Init_PresetChannelList(unsigned char PresetIndex)
{
	T_PRESET_LIST_INFO *pstPresetListInfo = NULL;

	int i;

	/* Check the preset index. */
	if (PresetIndex >= E_PRESET_LIST_MAX)
	{
		return (0);
	}

	/* Get the preset info of preset index. */
	pstPresetListInfo = &stISDBT_PresetInfo.preset_list_info[PresetIndex];

	/* Initialize the preset channel list information. */
	for (i = 0; i < E_PRESET_CHANNEL_ITEM_MAX; i++)
	{
		pstPresetListInfo->channel_info[i].region_id  = E_REGION_ID_MAX;
		pstPresetListInfo->channel_info[i].transport_stream_id  = 0xFFFF;
		memset((void *)pstPresetListInfo->channel_info[i].chName, 0, MAX_LENGTH_OF_CHANNEL_NAME);
		pstPresetListInfo->channel_info[i].remote_control_key_id  = E_PRESET_CHANNEL_ITEM_MAX;
		pstPresetListInfo->channel_info[i].chFreqIdx = E_ISDBT_FREQ_MAX;
	}

	/* Initialize the region IDs. */
	pstPresetListInfo->wide_region_id = E_REGION_ID_MAX;
	pstPresetListInfo->prefecture_region_id = E_REGION_ID_MAX;

	/* Init the channel index of current preset. */
	if (stISDBT_PresetInfo.cur_preset_idx == PresetIndex)
	{
		stISDBT_PresetInfo.cur_channel_index = E_PRESET_CHANNEL_ITEM_1ST;
	}

	return (1);
}


int ISDBT_Init_CurPresetChannelList(void)
{
//	T_PRESET_LIST_INFO *pstPresetListInfo = NULL;

	int iRet;

	iRet = ISDBT_Init_PresetChannelList(stISDBT_PresetInfo.cur_preset_idx);

	return iRet;
}


int ISDBT_Init_PresetInfo(unsigned char PresetIndex)
{
	T_PRESET_LIST_INFO *pstPresetListInfo = NULL;

	int iRet = 0;

	/* Check the preset index. */
	if (PresetIndex < E_PRESET_LIST_MAX)
	{
		/* Get the preset info of preset index. */
		pstPresetListInfo = &stISDBT_PresetInfo.preset_list_info[PresetIndex];

		/* Reset the preset information. */
		iRet = ISDBT_Init_PresetChannelList(PresetIndex);
		if (iRet == 1)
		{
			memset((void *)pstPresetListInfo->preset_name, 0, MAX_LENGTH_OF_CHANNEL_NAME);
			memcpy((void *)pstPresetListInfo->preset_name, "PRESET ", 7);
			pstPresetListInfo->preset_name[7] = (PresetIndex + 1) + '0';
		}
	}

	return iRet;
}


int ISDBT_Init_CurPresetInfo(void)
{
	int iRet = 0;

	iRet = ISDBT_Init_PresetInfo(stISDBT_PresetInfo.cur_preset_idx);

	return iRet;
}


void ISDBT_Init_AllPresetInfo(int country_code)
{
	int i;
//	int j;

	giISDBT_Preset_CountryCode = country_code;

	stISDBT_PresetInfo.cur_preset_idx    = E_PRESET_LIST_1ST;

	for (i = 0; i < E_PRESET_LIST_MAX; i++)
	{
		(void)ISDBT_Init_PresetInfo(i);
	}
}


unsigned char ISDBT_Get_CurPresetIndex(void)
{
	/* Return the current preset index. */
	return stISDBT_PresetInfo.cur_preset_idx;
}


int ISDBT_Set_CurPresetIndex(unsigned char PresetIndex)
{
	/* Check the preset index. */
	if (PresetIndex >= E_PRESET_LIST_MAX)
	{
		return (0);
	}

	/* Set the current preset index. */
	stISDBT_PresetInfo.cur_preset_idx = PresetIndex;

	return (1);
}


unsigned char ISDBT_Get_PresetWideRegionID(unsigned char PresetIndex)
{
	unsigned char CurPresetRegionID;

	CurPresetRegionID = stISDBT_PresetInfo.preset_list_info[PresetIndex].wide_region_id;

	return CurPresetRegionID;
}


unsigned char ISDBT_Get_CurPresetWideRegionID(void)
{
	unsigned char CurPresetRegionID;

	CurPresetRegionID = ISDBT_Get_PresetWideRegionID(stISDBT_PresetInfo.cur_preset_idx);

	return CurPresetRegionID;
}


unsigned char ISDBT_Get_PresetPrefectureRegionID(unsigned char PresetIndex)
{
	unsigned char CurPresetRegionID;

	CurPresetRegionID = stISDBT_PresetInfo.preset_list_info[PresetIndex].prefecture_region_id;

	return CurPresetRegionID;
}


unsigned char ISDBT_Get_CurPresetPrefectureRegionID(void)
{
	unsigned char CurPresetRegionID;

	CurPresetRegionID = ISDBT_Get_PresetPrefectureRegionID(stISDBT_PresetInfo.cur_preset_idx);

	return CurPresetRegionID;
}

int ISDBT_Get_TotalChNum_from_RegionInfo(int region)
{
	if ((region < 0) || (region >= (int)E_REGION_MAX)){
		return -1;
	}
	
	return stISDBT_RegionInfo[region].channel_list_nums;
}

int ISDBT_Get_FreqIndex_from_RegionInfo(int index, int region)
{
	int i=0, total_ch_num=0;
	T_CHANNEL_INFO *pstChList = NULL;

	if ((region < 0) || (region >= (int)E_REGION_MAX)){
		return -1;
	}

	pstChList = stISDBT_RegionInfo[region].pst_channel_list;

	return (int)pstChList[index].channel_frequency_index;	
}

int ISDBT_Set_PresetChannelListByRegion(unsigned char PresetIndex, unsigned char Region)
{
	T_CHANNEL_INFO *pstChList = NULL;
	T_PRESET_LIST_INFO *pstPresetListInfo = NULL;
	T_BROADCASTER_INFO *pstBroadcasterList = NULL;

	int i;
	int j;
	int iChListNums;
//	unsigned int uiCurPresetIndex;
	unsigned int uiChPos;
	int iBroadcasterListNums;
	unsigned char *pucStr = NULL;
	unsigned char wide_region_id = E_REGION_ID_MAX;
	unsigned char prefecture_region_id = E_REGION_ID_MAX;
	int iStringLength;

	/* Check the preset index and region ID. */
	if ((PresetIndex >= E_PRESET_LIST_MAX) || (Region >= E_REGION_MAX))
	{
		return (0);
	}

	/* Get the value in order to set the channel list info of region. */
	iChListNums       = stISDBT_RegionInfo[Region].channel_list_nums;
	pstChList         = stISDBT_RegionInfo[Region].pst_channel_list;
	pstPresetListInfo = &stISDBT_PresetInfo.preset_list_info[PresetIndex];

	uiChPos = 0;

	/* Set the channel list info. */
	for (i = 0; i < iChListNums; i++)
	{
		iBroadcasterListNums = stISDBT_RegionID_Info[pstChList[i].region_id].broadcaster_list_nums;
		pstBroadcasterList   = stISDBT_RegionID_Info[pstChList[i].region_id].pst_broadcaster_list;
		for (j = 0; j < iBroadcasterListNums; j++)
		{
			if (pstBroadcasterList[j].remote_control_key_id == pstChList[i].remote_control_key_id)
			{
				break;
			}
		}

		/* Check the valid broadcaster. */
		if (j < iBroadcasterListNums)
		{
			//uiChPos = pstChList[i].remote_control_key_id - 1;		//find position according to remote_control_key_id

			pstPresetListInfo->channel_info[uiChPos].transport_stream_id   = pstBroadcasterList[j].transport_stream_id;
			memset((void *)pstPresetListInfo->channel_info[uiChPos].chName, 0, MAX_LENGTH_OF_CHANNEL_NAME);
			pucStr = ISDBT_Get_String(pstBroadcasterList[j].ts_name_str_id);
			if (pucStr != NULL)
			{
				iStringLength = strlen((char *)pucStr);
				if (iStringLength > 0)
				{
					if (iStringLength > (MAX_LENGTH_OF_CHANNEL_NAME - 1))
					{
						iStringLength = (MAX_LENGTH_OF_CHANNEL_NAME - 1);
					}

					memcpy((void *)pstPresetListInfo->channel_info[uiChPos].chName, pucStr, iStringLength);
				}
			}

			pstPresetListInfo->channel_info[uiChPos].region_id             = pstChList[i].region_id;
			pstPresetListInfo->channel_info[uiChPos].remote_control_key_id = pstChList[i].remote_control_key_id;
			pstPresetListInfo->channel_info[uiChPos].chFreqIdx             = pstChList[i].channel_frequency_index;

			/* Check the wide region ID. */
			if ((wide_region_id == E_REGION_ID_MAX) && M_CHECK_WIDE_REGION_ID(pstChList[i].region_id))
			{
				wide_region_id = pstChList[i].region_id;
			}

			/* Check the wide region ID. */
			if ((prefecture_region_id == E_REGION_ID_MAX) && M_CHECK_WIDE_PREFECTURE_ID(pstChList[i].region_id))
			{
				prefecture_region_id = pstChList[i].region_id;
			}

			uiChPos++;		//find position increasingly
		}
	}

	/* Set the wide/prefecture region ID. */
	pstPresetListInfo->wide_region_id       = wide_region_id;
	pstPresetListInfo->prefecture_region_id = prefecture_region_id;

	return (1);
}


int ISDBT_Set_CurPresetChannelListByRegion(int area_code)
{
	int iRet = 0;
	unsigned char Region;

	if (area_code >= E_REGION_ID_HOKKAIDO_SAPPORO)
	{
		Region = area_code -E_REGION_ID_HOKKAIDO_SAPPORO;
		iRet = ISDBT_Set_PresetChannelListByRegion(stISDBT_PresetInfo.cur_preset_idx, Region);
	}
	else
		iRet = 0;

	return iRet;
}


unsigned char ISDBT_Get_ChannelFrequencyIndex(unsigned char PresetIndex, unsigned char channel_index)
{
	unsigned int	specific_info;
	unsigned char ChannelFrequencyIndex;

	/* Check the preset index and region ID. */
	if ((PresetIndex >= E_PRESET_LIST_MAX) || (channel_index >= E_PRESET_CHANNEL_ITEM_MAX))
	{
		return (E_ISDBT_FREQ_MAX);
	}

	/* Get the value in order to set the channel list info of region. */
	ChannelFrequencyIndex = stISDBT_PresetInfo.preset_list_info[PresetIndex].channel_info[channel_index].chFreqIdx;

	if (giISDBT_Preset_CountryCode == 0)
	{
		if (ChannelFrequencyIndex >= ISDBT_FREQ_JPN_MIN)
			ChannelFrequencyIndex -= ISDBT_FREQ_JPN_MIN;
	}
	else 
	{
		if (ChannelFrequencyIndex >= SBTVD_FREQ_BRA_MIN)
			ChannelFrequencyIndex -= SBTVD_FREQ_BRA_MIN;
	}
	return ChannelFrequencyIndex;
}

unsigned char ISDBT_Get_CurChannelFrequencyIndex(unsigned char channel_index)
{
	unsigned char ChannelFrequencyIndex;

	ChannelFrequencyIndex = ISDBT_Get_ChannelFrequencyIndex(stISDBT_PresetInfo.cur_preset_idx, channel_index);

	return ChannelFrequencyIndex;
}

unsigned char *ISDBT_Get_ChannelNameIndex (unsigned char PresetIndex, unsigned char channel_index)
{
	unsigned char *pChName = NULL;

	if ((PresetIndex >= E_PRESET_LIST_MAX) || (channel_index >= E_PRESET_CHANNEL_ITEM_MAX))
	{
		return (NULL);
	}

	pChName = stISDBT_PresetInfo.preset_list_info[PresetIndex].channel_info[channel_index].chName;
	return pChName;
}
unsigned char *ISDBT_Get_CurChannelNameIndex (unsigned char channel_index)
{
	unsigned char *pChName = NULL;

	pChName = ISDBT_Get_ChannelNameIndex (stISDBT_PresetInfo.cur_preset_idx, channel_index);
	return	pChName;
}

unsigned char ISDBT_Get_ChannelRemoconIDIndex (unsigned char PresetIndex, unsigned char channel_index)
{
	unsigned char	remote_control_key_id = 0;

	if ((PresetIndex >= E_PRESET_LIST_MAX) || (channel_index >= E_PRESET_CHANNEL_ITEM_MAX))
	{
		//empty
	}
	else
	{
		remote_control_key_id = stISDBT_PresetInfo.preset_list_info[PresetIndex].channel_info[channel_index].remote_control_key_id;
	}
	return	remote_control_key_id;
}
unsigned char ISDBT_Get_CurChannelRemoconIDIndex (unsigned char channel_index)
{
	unsigned char remote_control_key_id = 0;
	
	remote_control_key_id = ISDBT_Get_ChannelRemoconIDIndex (stISDBT_PresetInfo.cur_preset_idx, channel_index);
	return remote_control_key_id;
}


unsigned char ISDBT_Get_RegionIDIndex (unsigned char PresetIndex, unsigned char channel_index)
{
	unsigned char region_id = 0;
	if ((PresetIndex >= E_PRESET_LIST_MAX) || (channel_index >= E_PRESET_CHANNEL_ITEM_MAX))
	{
		//empty
	}
	else
	{
		region_id = stISDBT_PresetInfo.preset_list_info[PresetIndex].channel_info[channel_index].region_id;
	}
	return	region_id;
}
unsigned char ISDBT_Get_CurRegionIDIndex (unsigned channel_index)
{
	unsigned char	region_id;

	region_id = ISDBT_Get_RegionIDIndex (stISDBT_PresetInfo.cur_preset_idx, channel_index);
	return region_id;
}

unsigned short ISDBT_Get_TStreamIDIndex (unsigned char PresetIndex, unsigned char channel_index)
{
	unsigned short transport_stream_id = 0;

	if ((PresetIndex >= E_PRESET_LIST_MAX) || (channel_index >= E_PRESET_CHANNEL_ITEM_MAX))
	{
		//empty
	}
	else
	{
		transport_stream_id = stISDBT_PresetInfo.preset_list_info[PresetIndex].channel_info[channel_index].transport_stream_id;
	}
	return transport_stream_id;
}
unsigned short ISDBT_Get_CurTStreamIDIndex (unsigned char channel_index)
{
	unsigned short transport_stream_id = 0;

	transport_stream_id = ISDBT_Get_TStreamIDIndex (stISDBT_PresetInfo.cur_preset_idx, channel_index);
	return transport_stream_id;
}

//#ifdef DxB_SBTVD_INCLUDE
unsigned short ISDBT_Get_ChannelAreaCode(unsigned char PresetIndex, unsigned char channel_index)
{
	unsigned short ChannelAreaCode;

	/* Check the preset index and region ID. */
	if ((PresetIndex >= E_PRESET_LIST_MAX) || (channel_index >= E_PRESET_CHANNEL_ITEM_MAX))
	{
		return (E_ISDBT_FREQ_MAX);
	}

	/* Get the value in order to set the channel list info of region. */
	ChannelAreaCode = stISDBT_PresetInfo.preset_list_info[PresetIndex].channel_info[channel_index].areaCode;

	return ChannelAreaCode;
}

unsigned short ISDBT_Get_CurChannelAreaCode(unsigned char channel_index)
{
	unsigned short ChannelAreaCode;

	ChannelAreaCode = ISDBT_Get_ChannelAreaCode(stISDBT_PresetInfo.cur_preset_idx, channel_index);

	return ChannelAreaCode;
}
//#endif	// DxB_SBTVD_INCLUDE

T_PRESET_CHANNEL_INFO *ISDBT_Get_ChannelListInfo(unsigned char PresetIndex, unsigned char channel_index)
{
	/* Check the preset index and region ID. */
	if ((PresetIndex >= E_PRESET_LIST_MAX) || (channel_index >= E_PRESET_CHANNEL_ITEM_MAX))
	{
		return (NULL);
	}

	return &stISDBT_PresetInfo.preset_list_info[PresetIndex].channel_info[channel_index];
}


T_PRESET_CHANNEL_INFO *ISDBT_Get_CurChannelListInfo(unsigned char channel_index)
{
	T_PRESET_CHANNEL_INFO *pChannelInfo;

	pChannelInfo = ISDBT_Get_ChannelListInfo (stISDBT_PresetInfo.cur_preset_idx, channel_index);
	return	pChannelInfo;
}


int ISDBT_Set_ChannelInfo(unsigned char channel_index, T_PRESET_CHANNEL_INFO *pstChInfo)
{
	T_PRESET_CHANNEL_INFO *pstCurChInfo = NULL;
	unsigned char CurPresetIndex;

	if ((channel_index >= E_PRESET_CHANNEL_ITEM_MAX) || (pstChInfo == NULL))
	{
		return (0);
	}

	CurPresetIndex = stISDBT_PresetInfo.cur_preset_idx;
	pstCurChInfo = (T_PRESET_CHANNEL_INFO *)&stISDBT_PresetInfo.preset_list_info[CurPresetIndex].channel_info[channel_index];

	/* Copy new channel info to old one. */
	pstCurChInfo->service_id            = pstChInfo->service_id;
	pstCurChInfo->region_id             = pstChInfo->region_id;
	pstCurChInfo->transport_stream_id   = pstChInfo->transport_stream_id;
	pstCurChInfo->remote_control_key_id = pstChInfo->remote_control_key_id;	
	pstCurChInfo->chFreqIdx             = pstChInfo->chFreqIdx;
//#ifdef DxB_SBTVD_INCLUDE
	pstCurChInfo->areaCode		= pstChInfo->areaCode;
//#endif
	memcpy((void *)pstCurChInfo->chName, (void *)pstChInfo->chName, MAX_LENGTH_OF_CHANNEL_NAME);

	/* Check the wide region ID and register it. */
	if ( (stISDBT_PresetInfo.preset_list_info[CurPresetIndex].wide_region_id == E_REGION_ID_MAX) &&
	     M_CHECK_WIDE_REGION_ID(pstChInfo->region_id) )
	{
		stISDBT_PresetInfo.preset_list_info[CurPresetIndex].wide_region_id = pstChInfo->region_id;
	}

	/* Check the wide region ID and register it. */
	if ( (stISDBT_PresetInfo.preset_list_info[CurPresetIndex].prefecture_region_id == E_REGION_ID_MAX) &&
	     M_CHECK_WIDE_PREFECTURE_ID(pstChInfo->region_id) )
	{
		stISDBT_PresetInfo.preset_list_info[CurPresetIndex].prefecture_region_id = pstChInfo->region_id;
	}

	return (1);
}


unsigned char ISDBT_Get_CurrentChannelIndex(void)
{
	return stISDBT_PresetInfo.cur_channel_index;
}


int ISDBT_Set_CurrentChannelIndex(unsigned char CurChIdx)
{
	/* Check the valid channel index. */
	if (CurChIdx >= E_PRESET_CHANNEL_ITEM_MAX)
	{
		return (0);
	}

	stISDBT_PresetInfo.cur_channel_index = CurChIdx;

	return (1);
}


unsigned char ISDBT_Get_FirstPlayChannelIndex(void)
{
	T_PRESET_LIST_INFO *pstPresetListInfo = NULL;
	int i;

	pstPresetListInfo = (T_PRESET_LIST_INFO *)&stISDBT_PresetInfo.preset_list_info[stISDBT_PresetInfo.cur_preset_idx];
	for (i = 0; i < E_PRESET_CHANNEL_ITEM_MAX; i++)
	{
		if (pstPresetListInfo->channel_info[i].chFreqIdx != E_ISDBT_FREQ_MAX)
		{
		#if 0
			ISDBT_SetCurrentServiceId(pstPresetListInfo->channel_info[i].service_id);
		#ifdef ISDBT_DETECT_EXT_SERVICE_ID
			ISDBT_SetCurrentExtServiceId(pstPresetListInfo->channel_info[i].ext_service_id);
		#endif
		#endif
		
			return i;
		}
	}

	return E_PRESET_CHANNEL_ITEM_MAX;
}


unsigned char ISDBT_Get_NextPlayChannelIndex(void)
{
	T_PRESET_LIST_INFO *pstPresetListInfo = NULL;
	int i = 0;
	int ChannelIndex = 0;

	pstPresetListInfo = (T_PRESET_LIST_INFO *)&stISDBT_PresetInfo.preset_list_info[stISDBT_PresetInfo.cur_preset_idx];
	ChannelIndex = stISDBT_PresetInfo.cur_channel_index;
	for (i = 0; i < E_PRESET_CHANNEL_ITEM_MAX; i++)
	{
		ChannelIndex = (ChannelIndex + 1) % E_PRESET_CHANNEL_ITEM_MAX;
		if (pstPresetListInfo->channel_info[ChannelIndex].chFreqIdx != E_ISDBT_FREQ_MAX)
		{
		#if 0
			ISDBT_SetCurrentServiceId(pstPresetListInfo->channel_info[ChannelIndex].service_id);
		#ifdef ISDBT_DETECT_EXT_SERVICE_ID
			ISDBT_SetCurrentExtServiceId(pstPresetListInfo->channel_info[ChannelIndex].ext_service_id);
		#endif
		#endif

			return ChannelIndex;
		}
	}

	return E_PRESET_CHANNEL_ITEM_MAX;
}


unsigned char ISDBT_Get_PrevPlayChannelIndex(void)
{
	T_PRESET_LIST_INFO *pstPresetListInfo = NULL;
	int i = 0;
	int ChannelIndex = 0;

	pstPresetListInfo = (T_PRESET_LIST_INFO *)&stISDBT_PresetInfo.preset_list_info[stISDBT_PresetInfo.cur_preset_idx];
	ChannelIndex = stISDBT_PresetInfo.cur_channel_index;
	for (i = 0; i < E_PRESET_CHANNEL_ITEM_MAX; i++)
	{
		ChannelIndex = (ChannelIndex + (E_PRESET_CHANNEL_ITEM_MAX - 1)) % E_PRESET_CHANNEL_ITEM_MAX;
		if (pstPresetListInfo->channel_info[ChannelIndex].chFreqIdx != E_ISDBT_FREQ_MAX)
		{
		#if 0
			ISDBT_SetCurrentServiceId(pstPresetListInfo->channel_info[ChannelIndex].service_id);
		#ifdef ISDBT_DETECT_EXT_SERVICE_ID
			ISDBT_SetCurrentExtServiceId(pstPresetListInfo->channel_info[ChannelIndex].ext_service_id);
		#endif
		#endif

			return ChannelIndex;
		}
	}

	return E_PRESET_CHANNEL_ITEM_MAX;
}


int ISDBT_Get_ValidChannelNums(void)
{
	T_PRESET_LIST_INFO *pstPresetListInfo = NULL;
	int iCount = 0;
	int iValidChannelNums = 0;

	pstPresetListInfo = (T_PRESET_LIST_INFO *)&stISDBT_PresetInfo.preset_list_info[stISDBT_PresetInfo.cur_preset_idx];
	for (iCount = 0; iCount < E_PRESET_CHANNEL_ITEM_MAX; iCount++)
	{
		if (pstPresetListInfo->channel_info[iCount].chFreqIdx != E_ISDBT_FREQ_MAX)
		{
			iValidChannelNums++;
		}
	}

	return iValidChannelNums;
}


/*********** BRAZIL **************/

/*   <Area_code: State Specification>
	[Remark 1] If you want to know each name matched by state ID, 
  		Refer to the table 1 in Annex F of SBTVD Standard N03.
  	[Remark 2] All of the string codes below are completed by following the ISO/IEC 8859-15. */

const unsigned char sbtvd_state_01_str[] = {0x52, 0x6f, 0x6e, 0x64, 0xf4, 0x6e, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state_02_str[] = {0x41, 0x63, 0x72, 0x65, 0x00};
const unsigned char sbtvd_state_03_str[] = {0x41, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state_04_str[] = {0x52, 0x6f, 0x72, 0x61, 0x69, 0x6d, 0x61, 0x00};
const unsigned char sbtvd_state_05_str[] = {0x50, 0x61, 0x72, 0xe1, 0x00};
const unsigned char sbtvd_state_06_str[] = {0x41, 0x6d, 0x61, 0x70, 0xe1, 0x00};
const unsigned char sbtvd_state_07_str[] = {0x54, 0x6f, 0x63, 0x61, 0x6e, 0x74, 0x69, 0x6e, 0x73, 0x00};
const unsigned char sbtvd_state_08_str[] = {0x4d, 0x61, 0x72, 0x61, 0x6e, 0x68, 0xe3, 0x6f, 0x00};
const unsigned char sbtvd_state_09_str[] = {0x50, 0x69, 0x61, 0x75, 0xed, 0x00};
const unsigned char sbtvd_state_10_str[] = {0x43, 0x65, 0x61, 0x72, 0xe1, 0x00};
const unsigned char sbtvd_state_11_str[] = {0x52, 0x69, 0x6f, 0x20, 0x47, 0x72, 0x61, 0x6e, 0x64, 0x65, 0x20, 0x64, 0x6f, 0x20, 0x4e, 0x6f, 0x72, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state_12_str[] = {0x50, 0x61, 0x72, 0x61, 0xed, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state_13_str[] = {0x50, 0x65, 0x72, 0x6e, 0x61, 0x6d, 0x62, 0x75, 0x63, 0x6f, 0x00};
const unsigned char sbtvd_state_14_str[] = {0x53, 0x65, 0x72, 0x67, 0x69, 0x70, 0x65, 0x00};
const unsigned char sbtvd_state_15_str[] = {0x41, 0x6c, 0x61, 0x67, 0x6f, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state_16_str[] = {0x42, 0x61, 0x68, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state_17_str[] = {0x4d, 0x69, 0x6e, 0x61, 0x73, 0x20, 0x47, 0x65, 0x72, 0x61, 0x69, 0x73, 0x00};
const unsigned char sbtvd_state_18_str[] = {0x45, 0x73, 0x70, 0xed, 0x72, 0x69, 0x74, 0x6f, 0x20, 0x53, 0x61, 0x6e, 0x74, 0x6f, 0x00};
const unsigned char sbtvd_state_19_str[] = {0x52, 0x69, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x4a, 0x61, 0x6e, 0x65, 0x69, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state_20_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x50, 0x61, 0x75, 0x6c, 0x6f, 0x00};
const unsigned char sbtvd_state_21_str[] = {0x50, 0x61, 0x72, 0x61, 0x6e, 0xe1, 0x00};
const unsigned char sbtvd_state_22_str[] = {0x53, 0x61, 0x6e, 0x74, 0x61, 0x20, 0x43, 0x61, 0x74, 0x61, 0x72, 0x69, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state_23_str[] = {0x52, 0x69, 0x6f, 0x20, 0x47, 0x72, 0x61, 0x6e, 0x64, 0x65, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x75, 0x6c, 0x00};
const unsigned char sbtvd_state_24_str[] = {0x4d, 0x61, 0x74, 0x6f, 0x20, 0x47, 0x72, 0x6f, 0x73, 0x73, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x75, 0x6c, 0x00};
const unsigned char sbtvd_state_25_str[] = {0x4d, 0x61, 0x74, 0x6f, 0x20, 0x47, 0x72, 0x6f, 0x73, 0x73, 0x6f, 0x00};
const unsigned char sbtvd_state_26_str[] = {0x47, 0x6f, 0x69, 0xe1, 0x73, 0x00};
const unsigned char sbtvd_state_27_str[] = {0x44, 0x69, 0x73, 0x74, 0x72, 0x69, 0x74, 0x6f, 0x20, 0x46, 0x65, 0x64, 0x65, 0x72, 0x61, 0x6c, 0x00};

/*   <Area_code: Micro-region Specification>
	[Remark 1] If you want to know each name matched by micro-region ID (per each state ID), 
  		Refer to the document downloadable from :
  		ftp://geoftp.ibge.gov.br/Organizacao/Divisao_Territorial/2007/
  	[Remark 2] All of the string codes below are completed by following the ISO/IEC 8859-15. */

const unsigned char sbtvd_state01_region_01_str[] = {0x50, 0x6f, 0x72, 0x74, 0x6f, 0x20, 0x56, 0x65, 0x6c, 0x68, 0x6f, 0x00};
const unsigned char sbtvd_state01_region_02_str[] = {0x47, 0x75, 0x61, 0x6a, 0x61, 0x72, 0xe1, 0x2d, 0x4d, 0x69, 0x72, 0x69, 0x6d, 0x00};
const unsigned char sbtvd_state01_region_03_str[] = {0x41, 0x72, 0x69, 0x71, 0x75, 0x65, 0x6d, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state01_region_04_str[] = {0x4a, 0x69, 0x2d, 0x50, 0x61, 0x72, 0x61, 0x6e, 0xe1, 0x00};
const unsigned char sbtvd_state01_region_05_str[] = {0x41, 0x6c, 0x76, 0x6f, 0x72, 0x61, 0x64, 0x20, 0x44, 0x27, 0x4f, 0x65, 0x73, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state01_region_06_str[] = {0x43, 0x61, 0x63, 0x6f, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state01_region_07_str[] = {0x56, 0x69, 0x6c, 0x68, 0x65, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state01_region_08_str[] = {0x43, 0x6f, 0x6c, 0x6f, 0x72, 0x61, 0x64, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x4f, 0x65, 0x73, 0x74, 0x65, 0x00};

const unsigned char sbtvd_state02_region_01_str[] = {0x43, 0x72, 0x75, 0x7a, 0x65, 0x69, 0x72, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x75, 0x6c, 0x00};
const unsigned char sbtvd_state02_region_02_str[] = {0x54, 0x61, 0x72, 0x61, 0x75, 0x61, 0x63, 0xe1, 0x00};
const unsigned char sbtvd_state02_region_03_str[] = {0x53, 0x65, 0x6e, 0x61, 0x20, 0x4d, 0x61, 0x64, 0x75, 0x72, 0x65, 0x69, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state02_region_04_str[] = {0x52, 0x69, 0x6f, 0x20, 0x42, 0x72, 0x61, 0x6e, 0x63, 0x6f, 0x00};
const unsigned char sbtvd_state02_region_05_str[] = {0x42, 0x72, 0x61, 0x73, 0x69, 0x6c, 0xe9, 0x69, 0x61, 0x00};

const unsigned char sbtvd_state03_region_01_str[] = {0x52, 0x69, 0x6f, 0x20, 0x4e, 0x65, 0x67, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state03_region_02_str[] = {0x4a, 0x61, 0x70, 0x75, 0x72, 0xe1, 0x00};
const unsigned char sbtvd_state03_region_03_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x53, 0x6f, 0x6c, 0x69, 0x6d, 0xf5, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state03_region_04_str[] = {0x4a, 0x75, 0x72, 0x75, 0xe1, 0x00};
const unsigned char sbtvd_state03_region_05_str[] = {0x54, 0x65, 0x66, 0xe9, 0x00};
const unsigned char sbtvd_state03_region_06_str[] = {0x43, 0x6f, 0x61, 0x72, 0x69, 0x00};
const unsigned char sbtvd_state03_region_07_str[] = {0x4d, 0x61, 0x6e, 0x61, 0x75, 0x73, 0x00};
const unsigned char sbtvd_state03_region_08_str[] = {0x52, 0x69, 0x6f, 0x20, 0x50, 0x72, 0x65, 0x74, 0x6f, 0x20, 0x64, 0x61, 0x20, 0x45, 0x76, 0x61, 0x00};
const unsigned char sbtvd_state03_region_09_str[] = {0x49, 0x74, 0x61, 0x63, 0x6f, 0x61, 0x74, 0x69, 0x61, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state03_region_10_str[] = {0x50, 0x61, 0x72, 0x69, 0x6e, 0x74, 0x69, 0x6e, 0x73, 0x00};
const unsigned char sbtvd_state03_region_11_str[] = {0x42, 0x6f, 0x63, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x41, 0x63, 0x72, 0x65, 0x00};
const unsigned char sbtvd_state03_region_12_str[] = {0x50, 0x75, 0x72, 0x75, 0x73, 0x00};
const unsigned char sbtvd_state03_region_13_str[] = {0x4d, 0x61, 0x64, 0x65, 0x69, 0x72, 0x61, 0x00};

const unsigned char sbtvd_state04_region_01_str[] = {0x42, 0x6f, 0x61, 0x20, 0x56, 0x69, 0x73, 0x74, 0x61, 0x00};
const unsigned char sbtvd_state04_region_02_str[] = {0x4e, 0x6f, 0x72, 0x64, 0x65, 0x73, 0x74, 0x65, 0x20, 0x64, 0x65, 0x20, 0x52, 0x6f, 0x72, 0x61, 0x69, 0x6d, 0x61, 0x00};
const unsigned char sbtvd_state04_region_03_str[] = {0x43, 0x61, 0x72, 0x61, 0x63, 0x61, 0x72, 0x61, 0xed, 0x00};
const unsigned char sbtvd_state04_region_04_str[] = {0x53, 0x75, 0x64, 0x65, 0x73, 0x74, 0x65, 0x20, 0x64, 0x65, 0x20, 0x52, 0x6f, 0x72, 0x61, 0x69, 0x6d, 0x61, 0x00};

const unsigned char sbtvd_state05_region_01_str[] = {0xd3, 0x62, 0x69, 0x64, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state05_region_02_str[] = {0x53, 0x61, 0x6e, 0x74, 0x61, 0x72, 0xe9, 0x6d, 0x00};
const unsigned char sbtvd_state05_region_03_str[] = {0x41, 0x6c, 0x6d, 0x65, 0x69, 0x72, 0x69, 0x6d, 0x00};
const unsigned char sbtvd_state05_region_04_str[] = {0x50, 0x6f, 0x72, 0x74, 0x65, 0x6c, 0x00};
const unsigned char sbtvd_state05_region_05_str[] = {0x46, 0x75, 0x72, 0x6f, 0x73, 0x20, 0x64, 0x65, 0x20, 0x42, 0x72, 0x65, 0x76, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state05_region_06_str[] = {0x41, 0x72, 0x61, 0x72, 0x69, 0x00};
const unsigned char sbtvd_state05_region_07_str[] = {0x42, 0x65, 0x6c, 0xe9, 0x6d, 0x00};
const unsigned char sbtvd_state05_region_08_str[] = {0x43, 0x61, 0x73, 0x74, 0x61, 0x6e, 0x68, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state05_region_09_str[] = {0x53, 0x61, 0x6c, 0x67, 0x61, 0x64, 0x6f, 0x00};
const unsigned char sbtvd_state05_region_10_str[] = {0x42, 0x72, 0x61, 0x67, 0x61, 0x6e, 0x74, 0x69, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state05_region_11_str[] = {0x43, 0x61, 0x6d, 0x65, 0x74, 0xe1, 0x00};
const unsigned char sbtvd_state05_region_12_str[] = {0x54, 0x6f, 0x6d, 0xe9, 0x2d, 0x41, 0xe7, 0x75, 0x00};
const unsigned char sbtvd_state05_region_13_str[] = {0x47, 0x75, 0x61, 0x6d, 0xe1, 0x00};
const unsigned char sbtvd_state05_region_14_str[] = {0x49, 0x74, 0x61, 0x69, 0x74, 0x75, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state05_region_15_str[] = {0x41, 0x6c, 0x74, 0x61, 0x6d, 0x69, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state05_region_16_str[] = {0x54, 0x75, 0x63, 0x75, 0x75, 0xed, 0x00};
const unsigned char sbtvd_state05_region_17_str[] = {0x50, 0x61, 0x72, 0x61, 0x67, 0x6f, 0x6d, 0x69, 0x6e, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state05_region_18_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x46, 0xe9, 0x6c, 0x69, 0x78, 0x20, 0x64, 0x6f, 0x20, 0x58, 0x69, 0x6e, 0x67, 0x75, 0x00};
const unsigned char sbtvd_state05_region_19_str[] = {0x50, 0x61, 0x72, 0x61, 0x75, 0x61, 0x70, 0x65, 0x62, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state05_region_20_str[] = {0x4d, 0x61, 0x72, 0x61, 0x62, 0xe1, 0x00};
const unsigned char sbtvd_state05_region_21_str[] = {0x52, 0x65, 0x64, 0x65, 0x6e, 0xe7, 0xe3, 0x6f, 0x00};
const unsigned char sbtvd_state05_region_22_str[] = {0x43, 0x6f, 0x6e, 0x63, 0x65, 0x69, 0xe7, 0xe3, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x41, 0x72, 0x61, 0x67, 0x75, 0x61, 0x69, 0x61, 0x00};

const unsigned char sbtvd_state06_region_01_str[] = {0x4f, 0x69, 0x61, 0x70, 0x69, 0x71, 0x75, 0x65, 0x00};
const unsigned char sbtvd_state06_region_02_str[] = {0x41, 0x6d, 0x61, 0x70, 0xe1, 0x00};
const unsigned char sbtvd_state06_region_03_str[] = {0x4d, 0x61, 0x63, 0x61, 0x70, 0xe1, 0x00};
const unsigned char sbtvd_state06_region_04_str[] = {0x4d, 0x61, 0x7a, 0x61, 0x67, 0xe3, 0x6f, 0x00};

const unsigned char sbtvd_state07_region_01_str[] = {0x42, 0x69, 0x43, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x50, 0x61, 0x70, 0x61, 0x67, 0x61, 0x69, 0x6f, 0x00};
const unsigned char sbtvd_state07_region_02_str[] = {0x41, 0x72, 0x61, 0x67, 0x75, 0x61, 0xed, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state07_region_03_str[] = {0x4d, 0x69, 0x72, 0x61, 0x63, 0x65, 0x6d, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x54, 0x6f, 0x63, 0x61, 0x6e, 0x69, 0x6e, 0x73, 0x00};
const unsigned char sbtvd_state07_region_04_str[] = {0x52, 0x69, 0x6f, 0x20, 0x46, 0x6f, 0x72, 0x6d, 0x6f, 0x73, 0x6f, 0x00};
const unsigned char sbtvd_state07_region_05_str[] = {0x47, 0x75, 0x72, 0x75, 0x70, 0x69, 0x00};
const unsigned char sbtvd_state07_region_06_str[] = {0x50, 0x6f, 0x72, 0x74, 0x6f, 0x20, 0x4e, 0x61, 0x63, 0x69, 0x6f, 0x6e, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state07_region_07_str[] = {0x4a, 0x61, 0x6c, 0x61, 0x70, 0xe3, 0x6f, 0x00};
const unsigned char sbtvd_state07_region_08_str[] = {0x44, 0x69, 0x61, 0x6e, 0xf3, 0x70, 0x6f, 0x6c, 0x69, 0x73, 0x00};

const unsigned char sbtvd_state08_region_01_str[] = {0x4c, 0x69, 0x74, 0x6f, 0x72, 0x61, 0x6c, 0x20, 0x4f, 0x63, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x61, 0x6c, 0x20, 0x4c, 0x61, 0x72, 0x61, 0x6e, 0x68, 0x65, 0x6e, 0x73, 0x65, 0x00};
const unsigned char sbtvd_state08_region_02_str[] = {0x41, 0x67, 0x6c, 0x6f, 0x6d, 0x65, 0x72, 0xe7, 0xe3, 0x6f, 0x20, 0x55, 0x72, 0x62, 0x61, 0x6e, 0x61, 0x20, 0x64, 0x65, 0x20, 0x53, 0xe3, 0x6f, 0x20, 0x4c, 0x75, 0xed, 0x73, 0x00};
const unsigned char sbtvd_state08_region_03_str[] = {0x52, 0x6f, 0x73, 0xe1, 0x72, 0x69, 0x6f, 0x00};
const unsigned char sbtvd_state08_region_04_str[] = {0x4c, 0x65, 0x6e, 0xe7, 0x6f, 0x69, 0x73, 0x20, 0x4d, 0x61, 0x72, 0x61, 0x6e, 0x68, 0x65, 0x6e, 0x73, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state08_region_05_str[] = {0x42, 0x61, 0x69, 0x78, 0x61, 0x64, 0x61, 0x20, 0x4d, 0x61, 0x72, 0x61, 0x6e, 0x68, 0x65, 0x6e, 0x73, 0x65, 0x00};
const unsigned char sbtvd_state08_region_06_str[] = {0x49, 0x74, 0x61, 0x70, 0x65, 0x63, 0x75, 0x72, 0x75, 0x20, 0x4d, 0x69, 0x72, 0x69, 0x6d, 0x00};
const unsigned char sbtvd_state08_region_07_str[] = {0x47, 0x75, 0x72, 0x75, 0x70, 0x69, 0x00};
const unsigned char sbtvd_state08_region_08_str[] = {0x50, 0x69, 0x6e, 0x64, 0x61, 0x72, 0xe9, 0x00};
const unsigned char sbtvd_state08_region_09_str[] = {0x49, 0x6d, 0x70, 0x65, 0x72, 0x61, 0x74, 0x72, 0x69, 0x7a, 0x00};
const unsigned char sbtvd_state08_region_10_str[] = {0x4d, 0xe9, 0x64, 0x69, 0x6f, 0x20, 0x4d, 0x65, 0x61, 0x72, 0x69, 0x6d, 0x00};
const unsigned char sbtvd_state08_region_11_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x4d, 0x65, 0x61, 0x72, 0x69, 0x6d, 0x20, 0x65, 0x20, 0x47, 0x72, 0x61, 0x6a, 0x61, 0xf9, 0x00};
const unsigned char sbtvd_state08_region_12_str[] = {0x50, 0x72, 0x65, 0x73, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x65, 0x20, 0x44, 0x75, 0x74, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state08_region_13_str[] = {0x42, 0x61, 0x69, 0x78, 0x6f, 0x20, 0x50, 0x61, 0x72, 0x6e, 0xed, 0x62, 0x61, 0x20, 0x4d, 0x61, 0x72, 0x61, 0x6e, 0x68, 0x65, 0x73, 0x65, 0x00};
const unsigned char sbtvd_state08_region_14_str[] = {0x43, 0x68, 0x61, 0x70, 0x61, 0x64, 0x69, 0x6e, 0x68, 0x61, 0x00};
const unsigned char sbtvd_state08_region_15_str[] = {0x43, 0x6f, 0x64, 0xf3, 0x00};
const unsigned char sbtvd_state08_region_16_str[] = {0x43, 0x6f, 0x65, 0x6c, 0x68, 0x6f, 0x20, 0x4e, 0x65, 0x74, 0x6f, 0x00};
const unsigned char sbtvd_state08_region_17_str[] = {0x43, 0x61, 0x78, 0x69, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state08_region_18_str[] = {0x43, 0x68, 0x61, 0x70, 0x61, 0x64, 0x61, 0x73, 0x20, 0x64, 0x6f, 0x20, 0x41, 0x6c, 0x74, 0x6f, 0x20, 0x49, 0x74, 0x61, 0x70, 0x65, 0x63, 0x75, 0x72, 0x75, 0x00};
const unsigned char sbtvd_state08_region_19_str[] = {0x50, 0x6f, 0x72, 0x74, 0x6f, 0x20, 0x46, 0x72, 0x61, 0x6e, 0x63, 0x6f, 0x00};
const unsigned char sbtvd_state08_region_20_str[] = {0x47, 0x65, 0x72, 0x61, 0x69, 0x73, 0x20, 0x64, 0x65, 0x20, 0x42, 0x61, 0x6c, 0x73, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state08_region_21_str[] = {0x43, 0x48, 0x61, 0x70, 0x61, 0x64, 0x61, 0x73, 0x20, 0x64, 0x61, 0x73, 0x20, 0x4d, 0x61, 0x6e, 0x67, 0x61, 0x62, 0x65, 0x69, 0x72, 0x61, 0x73, 0x00};

const unsigned char sbtvd_state09_region_01_str[] = {0x42, 0x61, 0x69, 0x78, 0x6f, 0x20, 0x50, 0x61, 0x72, 0x6e, 0x61, 0xed, 0x62, 0x61, 0x20, 0x50, 0x69, 0x61, 0x75, 0x69, 0x65, 0x6e, 0x73, 0x65, 0x00};
const unsigned char sbtvd_state09_region_02_str[] = {0x5c, 0x69, 0x74, 0x6f, 0x72, 0x61, 0x6c, 0x20, 0x50, 0x69, 0x61, 0x75, 0x69, 0x65, 0x6e, 0x73, 0x65, 0x00};
const unsigned char sbtvd_state09_region_03_str[] = {0x54, 0x65, 0x72, 0x65, 0x73, 0x69, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state09_region_04_str[] = {0x43, 0x61, 0x6d, 0x70, 0x6f, 0x20, 0x4d, 0x61, 0x69, 0x6f, 0x72, 0x00};
const unsigned char sbtvd_state09_region_05_str[] = {0x4d, 0xe9, 0x64, 0x69, 0x6f, 0x20, 0x50, 0x61, 0x72, 0x6e, 0x61, 0xed, 0x62, 0x61, 0x20, 0x50, 0x69, 0x61, 0x75, 0x69, 0x65, 0x6e, 0x73, 0x65, 0x00};
const unsigned char sbtvd_state09_region_06_str[] = {0x56, 0x61, 0x6c, 0x65, 0x6e, 0xe7, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x50, 0x69, 0x61, 0x75, 0xed, 0x00};
const unsigned char sbtvd_state09_region_07_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x50, 0x61, 0x72, 0x6e, 0x61, 0xed, 0x62, 0x61, 0x20, 0x50, 0x69, 0x61, 0x75, 0x69, 0x65, 0x6e, 0x73, 0x65, 0x00};
const unsigned char sbtvd_state09_region_08_str[] = {0x42, 0x65, 0x72, 0x74, 0x6f, 0x6c, 0xed, 0x6e, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state09_region_09_str[] = {0x46, 0x6c, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x6f, 0x00};
const unsigned char sbtvd_state09_region_10_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x4d, 0xe9, 0x64, 0x69, 0x6f, 0x20, 0x47, 0x75, 0x72, 0x67, 0x75, 0xe9, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state09_region_11_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x52, 0x61, 0x69, 0x6d, 0x75, 0x6e, 0x64, 0x6f, 0x20, 0x4e, 0x6f, 0x6e, 0x61, 0x74, 0x6f, 0x00};
const unsigned char sbtvd_state09_region_12_str[] = {0x43, 0x48, 0x61, 0x70, 0x61, 0x64, 0x61, 0x73, 0x20, 0x64, 0x6f, 0x20, 0x45, 0x78, 0x74, 0x72, 0x65, 0x6d, 0x6f, 0x20, 0x53, 0x75, 0x6c, 0x20, 0x50, 0x69, 0x61, 0x75, 0x69, 0x65, 0x6e, 0x73, 0x65, 0x00};
const unsigned char sbtvd_state09_region_13_str[] = {0x50, 0x69, 0x63, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state09_region_14_str[] = {0x50, 0x69, 0x6f, 0x20}; /* [Caution] One character to be written is 'Roman 4' (missing 1 character) */
const unsigned char sbtvd_state09_region_15_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x4d, 0xe9, 0x64, 0x69, 0x6f, 0x20, 0x43, 0x61, 0x6e, 0x69, 0x6e, 0x64, 0xe9, 0x00};

const unsigned char sbtvd_state10_region_01_str[] = {0x4c, 0x69, 0x74, 0x6f, 0x72, 0x61, 0x6c, 0x20, 0x64, 0x65, 0x20, 0x43, 0x61, 0x6d, 0x6f, 0x63, 0x69, 0x6d, 0x20, 0x65, 0x20, 0x41, 0x63, 0x61, 0x72, 0x61, 0xfa, 0x00};
const unsigned char sbtvd_state10_region_02_str[] = {0x49, 0x62, 0x69, 0x61, 0x70, 0x61, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state10_region_03_str[] = {0x43, 0x6f, 0x72, 0x65, 0x61, 0xfa, 0x00};
const unsigned char sbtvd_state10_region_04_str[] = {0x4d, 0x65, 0x72, 0x75, 0x6f, 0x63, 0x61, 0x00};
const unsigned char sbtvd_state10_region_05_str[] = {0x53, 0x6f, 0x62, 0x72, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state10_region_06_str[] = {0x49, 0x70, 0x75, 0x00};
const unsigned char sbtvd_state10_region_07_str[] = {0x53, 0x61, 0x6e, 0x74, 0x61, 0x20, 0x51, 0x75, 0x69, 0x74, 0xe9, 0x72, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state10_region_08_str[] = {0x49, 0x74, 0x61, 0x70, 0x69, 0x70, 0x6f, 0x63, 0x61, 0x00};
const unsigned char sbtvd_state10_region_09_str[] = {0x42, 0x61, 0x69, 0x78, 0x6f, 0x20, 0x43, 0x75, 0x72, 0x75, 0x00};
const unsigned char sbtvd_state10_region_10_str[] = {0x55, 0x72, 0x75, 0x62, 0x75, 0x72, 0x65, 0x74, 0x61, 0x6d, 0x61, 0x00};
const unsigned char sbtvd_state10_region_11_str[] = {0x4d, 0xe9, 0x64, 0x69, 0x6f, 0x20, 0x43, 0x75, 0x72, 0x75, 0x00};
const unsigned char sbtvd_state10_region_12_str[] = {0x43, 0x61, 0x6e, 0x69, 0x6e, 0x64, 0xe9, 0x00};
const unsigned char sbtvd_state10_region_13_str[] = {0x42, 0x61, 0x74, 0x75, 0x72, 0x69, 0x74, 0xe9, 0x00};
const unsigned char sbtvd_state10_region_14_str[] = {0x43, 0x68, 0x6f, 0x72, 0x6f, 0x7a, 0x69, 0x6e, 0x68, 0x6f, 0x00};
const unsigned char sbtvd_state10_region_15_str[] = {0x43, 0x61, 0x73, 0x63, 0x61, 0x76, 0x65, 0x6c, 0x00};
const unsigned char sbtvd_state10_region_16_str[] = {0x46, 0x6f, 0x72, 0x74, 0x61, 0x6c, 0x65, 0x7a, 0x61, 0x00};
const unsigned char sbtvd_state10_region_17_str[] = {0x50, 0x61, 0x63, 0x61, 0x6a, 0x75, 0x73, 0x00};
const unsigned char sbtvd_state10_region_18_str[] = {0x53, 0x65, 0x72, 0x74, 0xe3, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x43, 0x72, 0x61, 0x74, 0xe9, 0x75, 0x73, 0x00};
const unsigned char sbtvd_state10_region_19_str[] = {0x53, 0x65, 0x72, 0x74, 0xe3, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x51, 0x75, 0x69, 0x78, 0x65, 0x72, 0x61, 0x6d, 0x6f, 0x62, 0x69, 0x6d, 0x00};
const unsigned char sbtvd_state10_region_20_str[] = {0x53, 0x65, 0x72, 0x74, 0xe3, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x49, 0x6e, 0x68, 0x61, 0x6d, 0x75, 0x6e, 0x73, 0x00};
const unsigned char sbtvd_state10_region_21_str[] = {0x53, 0x65, 0x72, 0x74, 0xe3, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x53, 0x65, 0x6e, 0x61, 0x64, 0x6f, 0x72, 0x20, 0x50, 0x6f, 0x6d, 0x70, 0x65, 0x75, 0x00};
const unsigned char sbtvd_state10_region_22_str[] = {0x4c, 0x69, 0x74, 0x6f, 0x72, 0x61, 0x6c, 0x20, 0x64, 0x65, 0x20, 0x41, 0x72, 0x61, 0x63, 0x61, 0x74, 0x69, 0x00};
const unsigned char sbtvd_state10_region_23_str[] = {0x42, 0x61, 0x69, 0x78, 0x6f, 0x20, 0x4a, 0x61, 0x67, 0x75, 0x61, 0x72, 0x69, 0x62, 0x65, 0x00};
const unsigned char sbtvd_state10_region_24_str[] = {0x4d, 0xe9, 0x64, 0x69, 0x6f, 0x20, 0x4a, 0x61, 0x67, 0x75, 0x61, 0x72, 0x69, 0x62, 0x65, 0x00};
const unsigned char sbtvd_state10_region_25_str[] = {0x53, 0x65, 0x72, 0x72, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x50, 0x65, 0x72, 0x65, 0x69, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state10_region_26_str[] = {0x49, 0x67, 0x75, 0x61, 0x74, 0x75, 0x00};
const unsigned char sbtvd_state10_region_27_str[] = {0x56, 0xe1, 0x72, 0x7a, 0x65, 0x61, 0x20, 0x41, 0x6c, 0x65, 0x67, 0x72, 0x65, 0x00};
const unsigned char sbtvd_state10_region_28_str[] = {0x4c, 0x61, 0x56, 0x72, 0x61, 0x73, 0x20, 0x64, 0x61, 0x20, 0x4d, 0x61, 0x6e, 0x67, 0x61, 0x62, 0x65, 0x69, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state10_region_29_str[] = {0x43, 0x48, 0x61, 0x70, 0x61, 0x64, 0x16, 0x20, 0x64, 0x6f, 0x20, 0x41, 0x72, 0x61, 0x72, 0x69, 0x70, 0x65, 0x00};
const unsigned char sbtvd_state10_region_30_str[] = {0x43, 0x61, 0x72, 0x69, 0x72, 0x69, 0x61, 0xe7, 0x75, 0x00};
const unsigned char sbtvd_state10_region_31_str[] = {0x42, 0x61, 0x72, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state10_region_32_str[] = {0x43, 0x61, 0x72, 0x69, 0x72, 0x69, 0x00};
const unsigned char sbtvd_state10_region_33_str[] = {0x42, 0x72, 0x65, 0x6a, 0x6f, 0x20, 0x53, 0x61, 0x6e, 0x74, 0x6f, 0x00};

const unsigned char sbtvd_state11_region_01_str[] = {0x4d, 0x6f, 0x73, 0x73, 0x6f, 0x72, 0xf3, 0x00};
const unsigned char sbtvd_state11_region_02_str[] = {0x43, 0x68, 0x61, 0x70, 0x61, 0x64, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x41, 0x70, 0x6f, 0x64, 0x69, 0x00};
const unsigned char sbtvd_state11_region_03_str[] = {0x4d, 0xe9, 0x64, 0x69, 0x6f, 0x20, 0x4f, 0x65, 0x73, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state11_region_04_str[] = {0x56, 0x61, 0x6c, 0x65, 0x20, 0x64, 0x6f, 0x20, 0x41, 0xe7, 0x75, 0x00};
const unsigned char sbtvd_state11_region_05_str[] = {0x53, 0x65, 0x72, 0x72, 0x61, 0x20, 0x64, 0x65, 0x20, 0x53, 0xe3, 0x6f, 0x20, 0x4d, 0x69, 0x67, 0x75, 0x65, 0x6c, 0x00};
const unsigned char sbtvd_state11_region_06_str[] = {0x50, 0x61, 0x75, 0x20, 0x64, 0x6f, 0x73, 0x20, 0x46, 0x65, 0x72, 0x72, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state11_region_07_str[] = {0x55, 0x6c, 0x61, 0x72, 0x69, 0x7a, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state11_region_08_str[] = {0x4d, 0x61, 0x63, 0x61, 0x75, 0x00};
const unsigned char sbtvd_state11_region_09_str[] = {0x41, 0x6e, 0x67, 0x69, 0x63, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state11_region_10_str[] = {0x53, 0x65, 0x72, 0x72, 0x61, 0x20, 0x64, 0x65, 0x20, 0x53, 0x61, 0x6e, 0x74, 0x61, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state11_region_11_str[] = {0x53, 0x65, 0x72, 0x69, 0x64, 0xf3, 0x20, 0x4f, 0x63, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state11_region_12_str[] = {0x53, 0x65, 0x72, 0x69, 0x64, 0xf3, 0x20, 0x4f, 0x72, 0x69, 0x65, 0x6e, 0x74, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state11_region_13_str[] = {0x42, 0x61, 0x69, 0x78, 0x61, 0x20, 0x56, 0x65, 0x72, 0x64, 0x65, 0x00};
const unsigned char sbtvd_state11_region_14_str[] = {0x42, 0x6f, 0x72, 0x62, 0x6f, 0x72, 0x65, 0x6d, 0x61, 0x20, 0x50, 0x6f, 0x74, 0x69, 0x67, 0x75, 0x61, 0x72, 0x00};
const unsigned char sbtvd_state11_region_15_str[] = {0x41, 0x67, 0x72, 0x65, 0x73, 0x74, 0x65, 0x20, 0x50, 0x6f, 0x74, 0x69, 0x67, 0x75, 0x61, 0x72, 0x00};
const unsigned char sbtvd_state11_region_16_str[] = {0x4c, 0x69, 0x74, 0x6f, 0x72, 0x61, 0x6c, 0x20, 0x4e, 0x6f, 0x72, 0x64, 0x65, 0x73, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state11_region_17_str[] = {0x4d, 0x61, 0x63, 0x61, 0xed, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state11_region_18_str[] = {0x4e, 0x61, 0x74, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state11_region_19_str[] = {0x4c, 0x69, 0x74, 0x6f, 0x72, 0x61, 0x6c, 0x20, 0x53, 0x75, 0x6c, 0x00};

const unsigned char sbtvd_state12_region_01_str[] = {0x43, 0x61, 0x74, 0x6f, 0x6c, 0xe6, 0x20, 0x64, 0x6f, 0x20, 0x52, 0x6f, 0x63, 0x68, 0x61, 0x00};
const unsigned char sbtvd_state12_region_02_str[] = {0x43, 0x51, 0x6a, 0x61, 0x7a, 0x65, 0x69, 0x72, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state12_region_03_str[] = {0x53, 0x6f, 0x75, 0x73, 0x61, 0x00};
const unsigned char sbtvd_state12_region_04_str[] = {0x50, 0x61, 0x74, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state12_region_05_str[] = {0x50, 0x69, 0x61, 0x6e, 0x63, 0xf3, 0x00};
const unsigned char sbtvd_state12_region_06_str[] = {0x49, 0x74, 0x61, 0x70, 0x6f, 0x73, 0x61, 0x6e, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state12_region_07_str[] = {0x53, 0x65, 0x72, 0x72, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x54, 0x65, 0x69, 0x78, 0x65, 0x69, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state12_region_08_str[] = {0x53, 0x65, 0x72, 0x69, 0x64, 0xfe, 0x20, 0x4f, 0x63, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x61, 0x6c, 0x20, 0x50, 0x61, 0x72, 0x61, 0x69, 0x62, 0x61, 0x6e, 0x6f, 0x00};
const unsigned char sbtvd_state12_region_09_str[] = {0x53, 0x65, 0x72, 0x69, 0x64, 0xf3, 0x20, 0x4f, 0x72, 0x69, 0x65, 0x6e, 0x74, 0x61, 0x6c, 0x20, 0x50, 0x61, 0x72, 0x61, 0x69, 0x62, 0x61, 0x6e, 0x6f, 0x00};
const unsigned char sbtvd_state12_region_10_str[] = {0x43, 0x61, 0x72, 0x69, 0x72, 0x69, 0x20, 0x4f, 0x63, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state12_region_11_str[] = {0x43, 0x61, 0x72, 0x69, 0x72, 0x69, 0x20, 0x4f, 0x72, 0x69, 0x65, 0x6e, 0x74, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state12_region_12_str[] = {0x43, 0x75, 0x72, 0x69, 0x6d, 0x61, 0x74, 0x61, 0xfa, 0x20, 0x4f, 0x63, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state12_region_13_str[] = {0x43, 0x75, 0x72, 0x69, 0x6d, 0x61, 0x74, 0x61, 0xfa, 0x20, 0x4f, 0x72, 0x69, 0x65, 0x6e, 0x74, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state12_region_14_str[] = {0x45, 0x73, 0x70, 0x65, 0x72, 0x61, 0x6e, 0xe7, 0x61, 0x00};
const unsigned char sbtvd_state12_region_15_str[] = {0x42, 0x72, 0x65, 0x6a, 0x6f, 0x20, 0x50, 0x61, 0x72, 0x61, 0x69, 0x62, 0x61, 0x6e, 0x6f, 0x00};
const unsigned char sbtvd_state12_region_16_str[] = {0x47, 0x75, 0x61, 0x72, 0x61, 0x62, 0x69, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state12_region_17_str[] = {0x43, 0x61, 0x6d, 0x70, 0x69, 0x6e, 0x61, 0x20, 0x47, 0x72, 0x61, 0x6e, 0x64, 0x65, 0x00};
const unsigned char sbtvd_state12_region_18_str[] = {0x49, 0x74, 0x61, 0x62, 0x61, 0x69, 0x61, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state12_region_19_str[] = {0x55, 0x6d, 0x62, 0x75, 0x7a, 0x65, 0x69, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state12_region_20_str[] = {0x4c, 0x69, 0x74, 0x6f, 0x72, 0x61, 0x6c, 0x20, 0x4e, 0x6f, 0x72, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state12_region_21_str[] = {0x53, 0x61, 0x70, 0xe9, 0x00};
const unsigned char sbtvd_state12_region_22_str[] = {0x4a, 0x6f, 0x3e, 0x6f, 0x20, 0x50, 0x65, 0x73, 0x73, 0x6f, 0x61, 0x00};
const unsigned char sbtvd_state12_region_23_str[] = {0x4c, 0x69, 0x74, 0x6f, 0x72, 0x61, 0x6c, 0x20, 0x53, 0x75, 0x6c, 0x00};

const unsigned char sbtvd_state13_region_01_str[] = {0x41, 0x72, 0x61, 0x72, 0x69, 0x70, 0x69, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state13_region_02_str[] = {0x53, 0x61, 0x6c, 0x67, 0x75, 0x65, 0x69, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state13_region_03_str[] = {0x50, 0x61, 0x6a, 0x65, 0xfa, 0x00};
const unsigned char sbtvd_state13_region_04_str[] = {0x53, 0x65, 0x72, 0x74, 0xe3, 0x6f, 0x20, 0x63, 0x6f, 0x20, 0x4d, 0x6f, 0x78, 0x6f, 0x74, 0xf3, 0x00};
const unsigned char sbtvd_state13_region_05_str[] = {0x50, 0x65, 0x74, 0x72, 0x6f, 0x6c, 0x69, 0x6e, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state13_region_06_str[] = {0x49, 0x74, 0x61, 0x70, 0x61, 0x72, 0x69, 0x63, 0x61, 0x00};
const unsigned char sbtvd_state13_region_07_str[] = {0x56, 0x61, 0x6c, 0x65, 0x20, 0x64, 0x6f, 0x20, 0x49, 0x70, 0x61, 0x6e, 0x65, 0x6d, 0x61, 0x00};
const unsigned char sbtvd_state13_region_08_str[] = {0x56, 0x61, 0x6c, 0x65, 0x20, 0x64, 0x6f, 0x20, 0x49, 0x70, 0x6f, 0x6a, 0x75, 0x63, 0x61, 0x00};
const unsigned char sbtvd_state13_region_09_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x43, 0x61, 0x70, 0x69, 0x62, 0x61, 0x72, 0x69, 0x62, 0x65, 0x00};
const unsigned char sbtvd_state13_region_10_str[] = {0x4d, 0xe9, 0x64, 0x69, 0x6f, 0x20, 0x43, 0x61, 0x70, 0x69, 0x62, 0x61, 0x72, 0x69, 0x62, 0x65, 0x00};
const unsigned char sbtvd_state13_region_11_str[] = {0x47, 0x61, 0x72, 0x61, 0x6e, 0x68, 0x75, 0x6e, 0x73, 0x00};
const unsigned char sbtvd_state13_region_12_str[] = {0x42, 0x72, 0x65, 0x6a, 0x6f, 0x20, 0x50, 0x65, 0x72, 0x6e, 0x61, 0x6d, 0x62, 0x75, 0x63, 0x61, 0x6e, 0x6f, 0x00};
const unsigned char sbtvd_state13_region_13_str[] = {0x4d, 0x61, 0x74, 0x61, 0x20, 0x53, 0x65, 0x74, 0x65, 0x6e, 0x74, 0x72, 0x69, 0x6f, 0x6e, 0x61, 0x6c, 0x20, 0x50, 0x65, 0x72, 0x6e, 0x61, 0x6d, 0x62, 0x75, 0x63, 0x61, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state13_region_14_str[] = {0x56, 0x69, 0x74, 0xf3, 0x72, 0x69, 0x61, 0x20, 0x64, 0x65, 0x20, 0x53, 0x61, 0x6e, 0x74, 0x6f, 0x20, 0x41, 0x6e, 0x74, 0xe3, 0x6f, 0x00};
const unsigned char sbtvd_state13_region_15_str[] = {0x4d, 0x61, 0x74, 0x61, 0x20, 0x4d, 0x65, 0x72, 0x69, 0x64, 0x69, 0x6f, 0x6e, 0x61, 0x6c, 0x20, 0x50, 0x65, 0x72, 0x6e, 0x61, 0x6d, 0x62, 0x75, 0x63, 0x61, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state13_region_16_str[] = {0x49, 0x74, 0x61, 0x6d, 0x61, 0x72, 0x61, 0x63, 0xe1, 0x00};
const unsigned char sbtvd_state13_region_17_str[] = {0x52, 0x65, 0x63, 0x69, 0x66, 0x65, 0x00};
const unsigned char sbtvd_state13_region_18_str[] = {0x53, 0x75, 0x61, 0x70, 0x65, 0x00};
const unsigned char sbtvd_state13_region_19_str[] = {0x46, 0x65, 0x72, 0x6e, 0x61, 0x6e, 0x64, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x4e, 0x6f, 0x72, 0x6f, 0x6e, 0x68, 0x61, 0x00};

const unsigned char sbtvd_state14_region_01_str[] = {0x53, 0x65, 0x72, 0x67, 0x69, 0x70, 0x61, 0x6e, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x65, 0x72, 0x74, 0xe3, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x53, 0xe3, 0x6f, 0x20, 0x46, 0x72, 0x61, 0x6e, 0x63, 0x69, 0x73, 0x63, 0x6f, 0x00};
const unsigned char sbtvd_state14_region_02_str[] = {0x43, 0x61, 0x72, 0x69, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state14_region_03_str[] = {0x4e, 0x6f, 0x73, 0x73, 0x61, 0x20, 0x53, 0x65, 0x6e, 0x68, 0x6f, 0x72, 0x61, 0x20, 0x64, 0x61, 0x73, 0x20, 0x44, 0x6f, 0x72, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state14_region_04_str[] = {0x41, 0x67, 0x72, 0x65, 0x73, 0x74, 0x65, 0x20, 0x64, 0x65, 0x20, 0x49, 0x74, 0x61, 0x62, 0x61, 0x69, 0x61, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state14_region_05_str[] = {0x54, 0x6f, 0x62, 0x69, 0x61, 0x73, 0x20, 0x42, 0x61, 0x72, 0x72, 0x65, 0x74, 0x6f, 0x00};
const unsigned char sbtvd_state14_region_06_str[] = {0x41, 0x67, 0x72, 0x65, 0x73, 0x74, 0x65, 0x20, 0x64, 0x65, 0x20, 0x4c, 0x61, 0x67, 0x61, 0x72, 0x74, 0x6f, 0x00};
const unsigned char sbtvd_state14_region_07_str[] = {0x50, 0x72, 0x6f, 0x70, 0x72, 0x69, 0xe1, 0x00};
const unsigned char sbtvd_state14_region_08_str[] = {0x43, 0x6f, 0x74, 0x69, 0x6e, 0x67, 0x75, 0x69, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state14_region_09_str[] = {0x4a, 0x61, 0x70, 0x61, 0x72, 0x61, 0x74, 0x75, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state14_region_10_str[] = {0x42, 0x61, 0x69, 0x78, 0x6f, 0x20, 0x43, 0x6f, 0x74, 0x69, 0x6e, 0x67, 0x75, 0x69, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state14_region_11_str[] = {0x41, 0x72, 0x61, 0x63, 0x61, 0x6a, 0x75, 0x00};
const unsigned char sbtvd_state14_region_12_str[] = {0x42, 0x6f, 0x71, 0x75, 0x69, 0x6d, 0x00};
const unsigned char sbtvd_state14_region_13_str[] = {0x45, 0x73, 0x74, 0xe2, 0x6e, 0x63, 0x69, 0x61, 0x00};

const unsigned char sbtvd_state15_region_01_str[] = {0x53, 0x65, 0x72, 0x72, 0x61, 0x6e, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x65, 0x72, 0x74, 0xe3, 0x6f, 0x20, 0x41, 0x6c, 0x61, 0x67, 0x6f, 0x61, 0x6e, 0x6f, 0x00};
const unsigned char sbtvd_state15_region_02_str[] = {0x41, 0x6c, 0x61, 0x67, 0x6f, 0x61, 0x6e, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x65, 0x72, 0x74, 0xe3, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x53, 0xe3, 0x6f, 0x20, 0x46, 0x72, 0x61, 0x6e, 0x63, 0x69, 0x73, 0x63, 0x6f, 0x00};
const unsigned char sbtvd_state15_region_03_str[] = {0x53, 0x61, 0x6e, 0x74, 0x61, 0x6e, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x49, 0x70, 0x61, 0x6e, 0x65, 0x6d, 0x61, 0x00};
const unsigned char sbtvd_state15_region_04_str[] = {0x42, 0x61, 0x74, 0x61, 0x6c, 0x68, 0x61, 0x00};
const unsigned char sbtvd_state15_region_05_str[] = {0x50, 0x61, 0x6c, 0x6d, 0x65, 0x69, 0x72, 0x61, 0x20, 0x64, 0x6f, 0x73, 0x20, 0xcc, 0x6e, 0x64, 0x69, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state15_region_06_str[] = {0x41, 0x72, 0x61, 0x70, 0x69, 0x72, 0x61, 0x63, 0x61, 0x00};
const unsigned char sbtvd_state15_region_07_str[] = {0x54, 0x72, 0x61, 0x69, 0x70, 0x75, 0x00};
const unsigned char sbtvd_state15_region_08_str[] = {0x53, 0x65, 0x72, 0x72, 0x61, 0x6e, 0x61, 0x20, 0x64, 0x6f, 0x73, 0x20, 0x51, 0x75, 0x69, 0x6c, 0x6f, 0x6d, 0x62, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state15_region_09_str[] = {0x4d, 0x61, 0x74, 0x61, 0x20, 0x41, 0x6c, 0x61, 0x67, 0x6f, 0x61, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state15_region_10_str[] = {0x4c, 0x69, 0x74, 0x6f, 0x72, 0x61, 0x6c, 0x20, 0x4e, 0x6f, 0x72, 0x74, 0x65, 0x20, 0x41, 0x6c, 0x61, 0x67, 0x6f, 0x61, 0x6e, 0x6f, 0x00};
const unsigned char sbtvd_state15_region_11_str[] = {0x4d, 0x61, 0x63, 0x65, 0x69, 0xf3, 0x00};
const unsigned char sbtvd_state15_region_12_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x4d, 0x69, 0x67, 0x75, 0x65, 0x6c, 0x20, 0x64, 0x6f, 0x73, 0x20, 0x43, 0x61, 0x6d, 0x70, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state15_region_13_str[] = {0x50, 0x65, 0x6e, 0x65, 0x64, 0x6f, 0x00};

const unsigned char sbtvd_state16_region_01_str[] = {0x42, 0x61, 0x72, 0x72, 0x65, 0x69, 0x72, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state16_region_02_str[] = {0x43, 0x6f, 0x73, 0x65, 0x67, 0x69, 0x70, 0x65, 0x00};
const unsigned char sbtvd_state16_region_03_str[] = {0x53, 0x61, 0x6e, 0x74, 0x61, 0x20, 0x4d, 0x61, 0x72, 0x69, 0x61, 0x20, 0x42, 0x61, 0x20, 0x56, 0x69, 0x74, 0xf3, 0x72, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state16_region_04_str[] = {0x4a, 0x75, 0x61, 0x7a, 0x65, 0x69, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state16_region_05_str[] = {0x50, 0x61, 0x75, 0x6c, 0x6f, 0x20, 0x41, 0x66, 0x6f, 0x6e, 0x73, 0x6f, 0x00};
const unsigned char sbtvd_state16_region_06_str[] = {0x42, 0x61, 0x72, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state16_region_07_str[] = {0x42, 0x6f, 0x6d, 0x20, 0x4a, 0x65, 0x73, 0x75, 0x73, 0x20, 0x64, 0x61, 0x20, 0x4c, 0x61, 0x70, 0x61, 0x00};
const unsigned char sbtvd_state16_region_08_str[] = {0x53, 0x65, 0x6e, 0x68, 0x6f, 0x72, 0x20, 0x64, 0x6f, 0x20, 0x42, 0x6f, 0x6e, 0x66, 0x69, 0x6d, 0x00};
const unsigned char sbtvd_state16_region_09_str[] = {0x49, 0x72, 0x65, 0x63, 0xea, 0x00};
const unsigned char sbtvd_state16_region_10_str[] = {0x4a, 0x61, 0x63, 0x6f, 0x62, 0x69, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state16_region_11_str[] = {0x49, 0x74, 0x61, 0x62, 0x65, 0x72, 0x61, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state16_region_12_str[] = {0x46, 0x65, 0x69, 0x72, 0x61, 0x20, 0x64, 0x65, 0x20, 0x53, 0x61, 0x6e, 0x74, 0x61, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state16_region_13_str[] = {0x4a, 0x65, 0x72, 0x65, 0x6d, 0x6f, 0x61, 0x62, 0x6f, 0x00};
const unsigned char sbtvd_state16_region_14_str[] = {0x45, 0x75, 0x63, 0x6c, 0x69, 0x64, 0x65, 0x73, 0x20, 0x64, 0x61, 0x20, 0x43, 0x75, 0x6e, 0x68, 0x61, 0x00};
const unsigned char sbtvd_state16_region_15_str[] = {0x52, 0x69, 0x62, 0x65, 0x69, 0x72, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x50, 0x6f, 0x6d, 0x62, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state16_region_16_str[] = {0x53, 0x65, 0x72, 0x72, 0x69, 0x6e, 0x68, 0x61, 0x00};
const unsigned char sbtvd_state16_region_17_str[] = {0x41, 0x6c, 0x61, 0x67, 0x6f, 0x69, 0x6e, 0x68, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state16_region_18_str[] = {0x45, 0x6e, 0x74, 0x72, 0x65, 0x20, 0x52, 0x69, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state16_region_19_str[] = {0x43, 0x61, 0x74, 0x75, 0x00};
const unsigned char sbtvd_state16_region_20_str[] = {0x53, 0x61, 0x6e, 0x74, 0x6f, 0x20, 0x41, 0x6e, 0x74, 0xf4, 0x6e, 0x69, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x4a, 0x65, 0x73, 0x75, 0x73, 0x00};
const unsigned char sbtvd_state16_region_21_str[] = {0x53, 0x61, 0x6c, 0x76, 0x61, 0x64, 0x6f, 0x72, 0x00};
const unsigned char sbtvd_state16_region_22_str[] = {0x42, 0x6f, 0x71, 0x75, 0x69, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state16_region_23_str[] = {0x53, 0x65, 0x61, 0x62, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state16_region_24_str[] = {0x4a, 0x65, 0x71, 0x75, 0x69, 0xe9, 0x00};
const unsigned char sbtvd_state16_region_25_str[] = {0x4c, 0x69, 0x76, 0x72, 0x61, 0x6d, 0x65, 0x6e, 0x74, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x42, 0x72, 0x75, 0x6d, 0x61, 0x64, 0x6f, 0x00};
const unsigned char sbtvd_state16_region_26_str[] = {0x47, 0x75, 0x61, 0x6e, 0x61, 0x6d, 0x62, 0x69, 0x00};
const unsigned char sbtvd_state16_region_27_str[] = {0x42, 0x72, 0x75, 0x6d, 0x61, 0x64, 0x6f, 0x00};
const unsigned char sbtvd_state16_region_28_str[] = {0x56, 0x69, 0x74, 0xf3, 0x72, 0x69, 0x61, 0x20, 0x64, 0x61, 0x20, 0x43, 0x6f, 0x6e, 0x71, 0x75, 0x69, 0x73, 0x74, 0x61, 0x00};
const unsigned char sbtvd_state16_region_29_str[] = {0x49, 0x74, 0x61, 0x70, 0x65, 0x74, 0x69, 0x6e, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state16_region_30_str[] = {0x56, 0x61, 0x6c, 0x65, 0x6e, 0xe7, 0x61, 0x00};
const unsigned char sbtvd_state16_region_31_str[] = {0x49, 0x6c, 0x68, 0xe9, 0x75, 0x73, 0x2d, 0x49, 0x74, 0x61, 0x62, 0x75, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state16_region_32_str[] = {0x50, 0x6f, 0x72, 0x74, 0x6f, 0x20, 0x53, 0x65, 0x67, 0x75, 0x72, 0x6f, 0x00};

const unsigned char sbtvd_state17_region_01_str[] = {0x55, 0x6e, 0x61, 0xed, 0x00};
const unsigned char sbtvd_state17_region_02_str[] = {0x50, 0x61, 0x72, 0x61, 0x63, 0x61, 0x74, 0x75, 0x00};
const unsigned char sbtvd_state17_region_03_str[] = {0x4a, 0x61, 0x6e, 0x75, 0xe1, 0x72, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state17_region_04_str[] = {0x4a, 0x61, 0x6e, 0x61, 0xfa, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state17_region_05_str[] = {0x53, 0x61, 0x6c, 0x69, 0x6e, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state17_region_06_str[] = {0x50, 0x69, 0x72, 0x61, 0x70, 0x6f, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state17_region_07_str[] = {0x4d, 0x6f, 0x6e, 0x74, 0x65, 0x73, 0x20, 0x43, 0x6c, 0x61, 0x72, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state17_region_08_str[] = {0x47, 0x72, 0xe3, 0x6f, 0x20, 0x4d, 0x6f, 0x67, 0x6f, 0x6c, 0x00};
const unsigned char sbtvd_state17_region_09_str[] = {0x42, 0x6f, 0x63, 0x61, 0x69, 0xfa, 0x76, 0x61, 0x00};
const unsigned char sbtvd_state17_region_10_str[] = {0x44, 0x69, 0x61, 0x6c, 0x61, 0x6e, 0x74, 0x69, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state17_region_11_str[] = {0x43, 0x61, 0x70, 0x65, 0x6c, 0x69, 0x6e, 0x68, 0x61, 0x00};
const unsigned char sbtvd_state17_region_12_str[] = {0x41, 0x72, 0x61, 0xe7, 0x75, 0x61, 0xed, 0x00};
const unsigned char sbtvd_state17_region_13_str[] = {0x50, 0x65, 0x64, 0x72, 0x61, 0x20, 0x41, 0x7a, 0x75, 0x6c, 0x00};
const unsigned char sbtvd_state17_region_14_str[] = {0x41, 0x6c, 0x6d, 0x65, 0x6e, 0x61, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state17_region_15_str[] = {0x54, 0x65, 0xf3, 0x66, 0x69, 0x6c, 0x6f, 0x20, 0x4f, 0x74, 0x6f, 0x6e, 0x69, 0x00};
const unsigned char sbtvd_state17_region_16_str[] = {0x4e, 0x61, 0x6e, 0x75, 0x71, 0x75, 0x65, 0x00};
const unsigned char sbtvd_state17_region_17_str[] = {0x49, 0x74, 0x75, 0x69, 0x75, 0x74, 0x61, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state17_region_18_str[] = {0x55, 0x62, 0x65, 0x72, 0x6c, 0xe2, 0x6e, 0x64, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state17_region_19_str[] = {0x50, 0x61, 0x74, 0x72, 0x6f, 0x63, 0xed, 0x6e, 0x69, 0x6f, 0x00};
const unsigned char sbtvd_state17_region_20_str[] = {0x50, 0x61, 0x74, 0x6f, 0x73, 0x20, 0x64, 0x65, 0x20, 0x4d, 0x69, 0x6e, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state17_region_21_str[] = {0x46, 0x72, 0x75, 0x74, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state17_region_22_str[] = {0x55, 0x62, 0x65, 0x72, 0x61, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state17_region_23_str[] = {0x41, 0x72, 0x61, 0x78, 0xe1, 0x00};
const unsigned char sbtvd_state17_region_24_str[] = {0x54, 0x72, 0xea, 0x73, 0x20, 0x4d, 0x61, 0x72, 0x69, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state17_region_25_str[] = {0x43, 0x75, 0x72, 0x76, 0x65, 0x6c, 0x6f, 0x00};
const unsigned char sbtvd_state17_region_26_str[] = {0x42, 0x6f, 0x6d, 0x20, 0x44, 0x65, 0x73, 0x70, 0x61, 0x63, 0x68, 0x6f, 0x00};
const unsigned char sbtvd_state17_region_27_str[] = {0x53, 0x65, 0x74, 0x65, 0x20, 0x4c, 0x61, 0x67, 0x6f, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state17_region_28_str[] = {0x43, 0x6f, 0x6e, 0x63, 0x65, 0x69, 0xe7, 0xe3, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x4d, 0x61, 0x74, 0x6f, 0x20, 0x44, 0x65, 0x6e, 0x74, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state17_region_29_str[] = {0x50, 0x61, 0x72, 0xe1, 0x20, 0x64, 0x65, 0x20, 0x5d, 0x69, 0x6e, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state17_region_30_str[] = {0x42, 0x65, 0x6c, 0x6f, 0x20, 0x48, 0x6f, 0x72, 0x69, 0x7a, 0x6f, 0x6e, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state17_region_31_str[] = {0x49, 0x74, 0x61, 0x62, 0x69, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state17_region_32_str[] = {0x49, 0x74, 0x61, 0x67, 0x75, 0x61, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state17_region_33_str[] = {0x4f, 0x75, 0x72, 0x6f, 0x20, 0x50, 0x72, 0x65, 0x74, 0x6f, 0x00};
const unsigned char sbtvd_state17_region_34_str[] = {0x43, 0x6f, 0x6e, 0x73, 0x65, 0x6c, 0x68, 0x65, 0x69, 0x72, 0x6f, 0x20, 0x4c, 0x61, 0x66, 0x61, 0x69, 0x65, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state17_region_35_str[] = {0x47, 0x75, 0x61, 0x6e, 0x68, 0xe3, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state17_region_36_str[] = {0x50, 0x65, 0xe7, 0x61, 0x6e, 0x68, 0x61, 0x00};
const unsigned char sbtvd_state17_region_37_str[] = {0x47, 0x6f, 0x76, 0x65, 0x72, 0x6e, 0x61, 0x64, 0x6f, 0x72, 0x20, 0x56, 0x61, 0x6c, 0x61, 0x64, 0x61, 0x72, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state17_region_38_str[] = {0x4d, 0x61, 0x6e, 0x74, 0x65, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state17_region_39_str[] = {0x49, 0x70, 0x61, 0x74, 0x69, 0x6e, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state17_region_40_str[] = {0x43, 0x61, 0x72, 0x61, 0x74, 0x69, 0x6e, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state17_region_41_str[] = {0x41, 0x69, 0x6d, 0x6f, 0x72, 0xe9, 0x73, 0x00};
const unsigned char sbtvd_state17_region_42_str[] = {0x50, 0x69, 0x75, 0xed, 0x00};
const unsigned char sbtvd_state17_region_43_str[] = {0x44, 0x69, 0x76, 0x69, 0x6e, 0xf3, 0x70, 0x6f, 0x6c, 0x69, 0x73, 0x00};
const unsigned char sbtvd_state17_region_44_str[] = {0x46, 0x6f, 0x72, 0x6d, 0x69, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state17_region_45_str[] = {0x43, 0x61, 0x6d, 0x70, 0x6f, 0x20, 0x42, 0x65, 0x6c, 0x6f, 0x00};
const unsigned char sbtvd_state17_region_46_str[] = {0x4f, 0x6c, 0x69, 0x76, 0x65, 0x69, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state17_region_47_str[] = {0x50, 0x61, 0x73, 0x73, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state17_region_48_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x53, 0x65, 0x62, 0x61, 0x73, 0x74, 0x69, 0xe3, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x50, 0x61, 0x72, 0x61, 0xed, 0x73, 0x6f, 0x00};
const unsigned char sbtvd_state17_region_49_str[] = {0x41, 0x6c, 0x66, 0x65, 0x6e, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state17_region_50_str[] = {0x56, 0x61, 0x72, 0x67, 0x69, 0x6e, 0x68, 0x61, 0x00};
const unsigned char sbtvd_state17_region_51_str[] = {0x50, 0x6f, 0xe7, 0x6f, 0x73, 0x20, 0x64, 0x65, 0x20, 0x43, 0x61, 0x6c, 0x64, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state17_region_52_str[] = {0x50, 0x6f, 0x75, 0x73, 0x6f, 0x20, 0x41, 0x6c, 0x65, 0x67, 0x72, 0x65, 0x00};
const unsigned char sbtvd_state17_region_53_str[] = {0x53, 0x61, 0x6e, 0x74, 0x61, 0x20, 0x52, 0x69, 0x74, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x61, 0x70, 0x75, 0x63, 0x61, 0xed, 0x00};
const unsigned char sbtvd_state17_region_54_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x4c, 0x6f, 0x75, 0x72, 0x65, 0x6e, 0xe7, 0x6f, 0x00};
const unsigned char sbtvd_state17_region_55_str[] = {0x41, 0x6e, 0x64, 0x72, 0x65, 0x6c, 0xe2, 0x6e, 0x64, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state17_region_56_str[] = {0x49, 0x74, 0x61, 0x6a, 0x75, 0x62, 0xe1, 0x00};
const unsigned char sbtvd_state17_region_57_str[] = {0x4c, 0x61, 0x76, 0x72, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state17_region_58_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x4a, 0x6f, 0xe3, 0x6f, 0x20, 0x44, 0x65, 0x6c, 0x20, 0x52, 0x65, 0x69, 0x00};
const unsigned char sbtvd_state17_region_59_str[] = {0x42, 0x61, 0x72, 0x62, 0x61, 0x63, 0x65, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state17_region_60_str[] = {0x50, 0x6f, 0x6e, 0x74, 0x65, 0x20, 0x4e, 0x6f, 0x76, 0x61, 0x00};
const unsigned char sbtvd_state17_region_61_str[] = {0x4d, 0x61, 0x6e, 0x68, 0x75, 0x61, 0xe7, 0x75, 0x00};
const unsigned char sbtvd_state17_region_62_str[] = {0x56, 0x69, 0xe7, 0x6f, 0x73, 0x61, 0x00};
const unsigned char sbtvd_state17_region_63_str[] = {0x4d, 0x75, 0x72, 0x69, 0x61, 0xe9, 0x00};
const unsigned char sbtvd_state17_region_64_str[] = {0x55, 0x62, 0xe1, 0x00};
const unsigned char sbtvd_state17_region_65_str[] = {0x4a, 0x75, 0x69, 0x7a, 0x20, 0x64, 0x65, 0x20, 0x46, 0x6f, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state17_region_66_str[] = {0x43, 0x61, 0x74, 0x61, 0x67, 0x75, 0x61, 0x73, 0x65, 0x73, 0x00};

const unsigned char sbtvd_state18_region_01_str[] = {0x42, 0x61, 0x72, 0x72, 0x61, 0x20, 0x64, 0x65, 0x20, 0x53, 0xe3, 0x6f, 0x20, 0x46, 0x72, 0x61, 0x6e, 0x63, 0x69, 0x73, 0x63, 0x6f, 0x00};
const unsigned char sbtvd_state18_region_02_str[] = {0x4e, 0x6f, 0x76, 0x61, 0x20, 0x56, 0x65, 0x6e, 0xe9, 0x63, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state18_region_03_str[] = {0x43, 0x6f, 0x6c, 0x61, 0x74, 0x69, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state18_region_04_str[] = {0x4d, 0x6f, 0x6e, 0x74, 0x61, 0x6e, 0x68, 0x61, 0x00};
const unsigned char sbtvd_state18_region_05_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x4d, 0x61, 0x74, 0x65, 0x75, 0x73, 0x00};
const unsigned char sbtvd_state18_region_06_str[] = {0x4c, 0x69, 0x6e, 0x68, 0x61, 0x72, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state18_region_07_str[] = {0x41, 0x66, 0x6f, 0x6e, 0x73, 0x6f, 0x20, 0x43, 0x6c, 0xe1, 0x75, 0x64, 0x69, 0x6f, 0x00};
const unsigned char sbtvd_state18_region_08_str[] = {0x53, 0x61, 0x6e, 0x74, 0x61, 0x20, 0x54, 0x65, 0x72, 0x65, 0x73, 0x61, 0x00};
const unsigned char sbtvd_state18_region_09_str[] = {0x56, 0x69, 0x74, 0xf3, 0x72, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state18_region_10_str[] = {0x47, 0x75, 0x61, 0x72, 0x61, 0x70, 0x61, 0x72, 0x69, 0x00};
const unsigned char sbtvd_state18_region_11_str[] = {0x41, 0x6c, 0x65, 0x67, 0x72, 0x65, 0x00};
const unsigned char sbtvd_state18_region_12_str[] = {0x43, 0x61, 0x63, 0x68, 0x6f, 0x65, 0x69, 0x72, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x49, 0x74, 0x61, 0x70, 0x65, 0x6d, 0x69, 0x72, 0x69, 0x6d, 0x00};
const unsigned char sbtvd_state18_region_13_str[] = {0x49, 0x74, 0x61, 0x70, 0x65, 0x6d, 0x69, 0x72, 0x69, 0x6d, 0x00};

const unsigned char sbtvd_state19_region_01_str[] = {0x49, 0x74, 0x61, 0x70, 0x65, 0x72, 0x75, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state19_region_02_str[] = {0x53, 0x61, 0x6e, 0x74, 0x6f, 0x20, 0x41, 0x6e, 0x74, 0xf4, 0x6e, 0x69, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x50, 0xe1, 0x64, 0x75, 0x61, 0x00};
const unsigned char sbtvd_state19_region_03_str[] = {0x43, 0x61, 0x6d, 0x70, 0x6f, 0x73, 0x20, 0x64, 0x6f, 0x73, 0x20, 0x47, 0x6f, 79, 0x74, 0x61, 0x63, 0x61, 0x7a, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state19_region_04_str[] = {0x4d, 0x61, 0x63, 0x61, 0xe9, 0x00};
const unsigned char sbtvd_state19_region_05_str[] = {0x54, 0x72, 0xea, 0x73, 0x20, 0x52, 0x69, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state19_region_06_str[] = {0x43, 0x61, 0x6e, 0x74, 0x61, 0x67, 0x61, 0x6c, 0x6f, 0x2d, 0x43, 0x6f, 0x72, 0x64, 0x65, 0x69, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state19_region_07_str[] = {0x4e, 0x6f, 0x56, 0x61, 0x20, 0x46, 0x72, 0x69, 0x62, 0x75, 0x72, 0x67, 0x6f, 0x00};
const unsigned char sbtvd_state19_region_08_str[] = {0x53, 0x61, 0x6e, 0x74, 0x61, 0x20, 0x4d, 0x61, 0x72, 0x69, 0x61, 0x20, 0x4d, 0x61, 0x64, 0x61, 0x6c, 0x65, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state19_region_09_str[] = {0x62, 0x61, 0x63, 0x69, 0x61, 0x20, 0x64, 0x65, 0x20, 0x53, 0xe3, 0x6f, 0x20, 0x4a, 0x6f, 0xe3, 0x6f, 0x00};
const unsigned char sbtvd_state19_region_10_str[] = {0x4c, 0x61, 0x67, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state19_region_11_str[] = {0x56, 0x61, 0x6c, 0x65, 0x20, 0x64, 0x6f, 0x20, 0x50, 0x61, 0x72, 0x61, 0xed, 0x62, 0x61, 0x20, 0x46, 0x6c, 0x75, 0x6d, 0x69, 0x6e, 0x65, 0x6e, 0x73, 0x65, 0x00};
const unsigned char sbtvd_state19_region_12_str[] = {0x42, 0x61, 0x72, 0x72, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x50, 0x69, 0x72, 0x61, 0xed, 0x00};
const unsigned char sbtvd_state19_region_13_str[] = {0x42, 0x61, 0xed, 0x61, 0x20, 0x64, 0x61, 0x20, 0x49, 0x6c, 0x68, 0x61, 0x20, 0x47, 0x72, 0x61, 0x6e, 0x64, 0x65, 0x00};
const unsigned char sbtvd_state19_region_14_str[] = {0x56, 0x61, 0x73, 0x73, 0x6f, 0x75, 0x72, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state19_region_15_str[] = {0x53, 0x65, 0x72, 0x72, 0x61, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state19_region_16_str[] = {0x4d, 0x61, 0x63, 0x61, 0x63, 0x75, 0x2d, 0x43, 0x61, 0x63, 0x65, 0x72, 0x69, 0x62, 0x75, 0x00};
const unsigned char sbtvd_state19_region_17_str[] = {0x49, 0x74, 0x61, 0x67, 0x75, 0x61, 0xed, 0x00};
const unsigned char sbtvd_state19_region_18_str[] = {0x52, 0x69, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x4a, 0x61, 0x6e, 0x65, 0x69, 0x72, 0x6f, 0x00};

const unsigned char sbtvd_state20_region_01_str[] = {0x4a, 0x61, 0x6c, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state20_region_02_str[] = {0x46, 0x65, 0x72, 0x6e, 0x61, 0x6e, 0x64, 0xf3, 0x70, 0x6f, 0x6c, 0x69, 0x73, 0x00};
const unsigned char sbtvd_state20_region_03_str[] = {0x56, 0x6f, 0x74, 0x75, 0x70, 0x6f, 0x72, 0x61, 0x6e, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state20_region_04_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x4a, 0x6f, 0x73, 0xe9, 0x20, 0x64, 0x6f, 0x20, 0x52, 0x69, 0x6f, 0x20, 0x50, 0x72, 0x65, 0x74, 0x6f, 0x00};
const unsigned char sbtvd_state20_region_05_str[] = {0x43, 0x61, 0x74, 0x61, 0x6e, 0x64, 0x75, 0x76, 0x61, 0x00};
const unsigned char sbtvd_state20_region_06_str[] = {0x41, 0x75, 0x72, 0x69, 0x66, 0x6c, 0x61, 0x6d, 0x61, 0x00};
const unsigned char sbtvd_state20_region_07_str[] = {0x4e, 0x68, 0x61, 0x6e, 0x64, 0x65, 0x61, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state20_region_08_str[] = {0x4e, 0x6f, 0x76, 0x6f, 0x20, 0x48, 0x6f, 0x72, 0x69, 0x7a, 0x6f, 0x6e, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state20_region_09_str[] = {0x42, 0x61, 0x72, 0x72, 0x65, 0x74, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state20_region_10_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x4a, 0x6f, 0x61, 0x71, 0x75, 0x69, 0x6d, 0x20, 0x64, 0x61, 0x20, 0x42, 0x61, 0x72, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state20_region_11_str[] = {0x49, 0x74, 0x75, 0x76, 0x65, 0x72, 0x61, 0x76, 0x61, 0x00};
const unsigned char sbtvd_state20_region_12_str[] = {0x46, 0x72, 0x61, 0x6e, 0x63, 0x61, 0x00};
const unsigned char sbtvd_state20_region_13_str[] = {0x4a, 0x61, 0x62, 0x6f, 0x74, 0x69, 0x63, 0x61, 0x62, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state20_region_14_str[] = {0x52, 0x69, 0x62, 0x65, 0x69, 0x72, 0xe3, 0x6f, 0x20, 0x50, 0x72, 0x65, 0x74, 0x6f, 0x00};
const unsigned char sbtvd_state20_region_15_str[] = {0x42, 0x61, 0x74, 0x61, 0x74, 0x61, 0x69, 0x73, 0x00};
const unsigned char sbtvd_state20_region_16_str[] = {0x41, 0x6e, 0x64, 0x72, 0x61, 0x64, 0x69, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state20_region_17_str[] = {0x41, 0x72, 0x61, 0xe7, 0x61, 0x74, 0x75, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state20_region_18_str[] = {0x42, 0x69, 0x72, 0x69, 0x67, 0x75, 0x69, 0x00};
const unsigned char sbtvd_state20_region_19_str[] = {0x4c, 0x69, 0x6e, 0x73, 0x00};
const unsigned char sbtvd_state20_region_20_str[] = {0x42, 0x61, 0x75, 0x72, 0x75, 0x00};
const unsigned char sbtvd_state20_region_21_str[] = {0x4a, 0x61, 0xfa, 0x00};
const unsigned char sbtvd_state20_region_22_str[] = {0x51, 0x76, 0x61, 0x72, 0xe9, 0x00};
const unsigned char sbtvd_state20_region_23_str[] = {0x42, 0x6f, 0x74, 0x75, 0x63, 0x61, 0x74, 0x75, 0x00};
const unsigned char sbtvd_state20_region_24_str[] = {0x41, 0x72, 0x61, 0x72, 0x61, 0x71, 0x61, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state20_region_25_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x43, 0x61, 0x72, 0x6c, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state20_region_26_str[] = {0x52, 0x69, 0x6f, 0x20, 0x43, 0x6c, 0x61, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state20_region_27_str[] = {0x4c, 0x69, 0x6d, 0x65, 0x69, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state20_region_28_str[] = {0x50, 0x69, 0x72, 0x61, 0x63, 0x69, 0x63, 0x61, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state20_region_29_str[] = {0x50, 0x69, 0x72, 0x61, 0x73, 0x73, 0x75, 0x6e, 0x75, 0x6e, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state20_region_30_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x4a, 0x6f, 0xe3, 0x6f, 0x20, 0x64, 0x61, 0x20, 0x42, 0x6f, 0x61, 0x20, 0x56, 0x69, 0x73, 0x74, 0x61, 0x00};
const unsigned char sbtvd_state20_region_31_str[] = {0x4d, 0x6f, 0x6a, 0x69, 0x20, 0x4d, 0x69, 0x72, 0x69, 0x6d, 0x00};
const unsigned char sbtvd_state20_region_32_str[] = {0x43, 0x61, 0x6d, 0x70, 0x69, 0x6e, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state20_region_33_str[] = {0x41, 0x6d, 0x70, 0x61, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state20_region_34_str[] = {0x44, 0x72, 0x61, 0x63, 0x65, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state20_region_35_str[] = {0x41, 0x64, 0x61, 0x6d, 0x61, 0x6e, 0x74, 0x69, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state20_region_36_str[] = {0x50, 0x72, 0x65, 0x73, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x65, 0x20, 0x50, 0x72, 0x75, 0x64, 0x65, 0x6e, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state20_region_37_str[] = {0x54, 0x75, 0x70, 0xe3, 0x00};
const unsigned char sbtvd_state20_region_38_str[] = {0x4d, 0x61, 0x72, 0xed, 0x6c, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state20_region_39_str[] = {0x41, 0x73, 0x73, 0x69, 0x73, 0x00};
const unsigned char sbtvd_state20_region_40_str[] = {0x4f, 0x75, 0x72, 0x69, 0x6e, 0x68, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state20_region_41_str[] = {0x49, 0x74, 0x61, 0x70, 0x65, 0x76, 0x61, 0x00};
const unsigned char sbtvd_state20_region_42_str[] = {0x49, 0x74, 0x61, 0x70, 0x65, 0x74, 0x69, 0x6e, 0x69, 0x6e, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state20_region_43_str[] = {0x54, 0x61, 0x74, 0x75, 0xed, 0x00};
const unsigned char sbtvd_state20_region_44_str[] = {0x43, 0x61, 0xe3, 0x6f, 0x20, 0x42, 0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x00};
const unsigned char sbtvd_state20_region_45_str[] = {0x50, 0x69, 0x65, 0x64, 0x61, 0x64, 0x65, 0x00};
const unsigned char sbtvd_state20_region_46_str[] = {0x73, 0x6f, 0x72, 0x6f, 0x63, 0x61, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state20_region_47_str[] = {0x4a, 0x75, 0x6e, 0x64, 0x69, 0x61, 0xed, 0x00};
const unsigned char sbtvd_state20_region_48_str[] = {0x42, 0x72, 0x61, 0x67, 0x61, 0x6e, 0xe7, 0x61, 0x20, 0x50, 0x61, 0x75, 0x6c, 0x69, 0x73, 0x74, 0x61, 0x00};
const unsigned char sbtvd_state20_region_49_str[] = {0x43, 0x61, 0x6d, 0x70, 0x6f, 0x73, 0x20, 0x64, 0x6f, 0x20, 0x4a, 0x6f, 0x72, 0x64, 0xe3, 0x6f, 0x00};
const unsigned char sbtvd_state20_region_50_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x4a, 0x6f, 0x73, 0xe9, 0x20, 0x64, 0x6f, 0x73, 0x20, 0x43, 0x61, 0x6d, 0x70, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state20_region_51_str[] = {0x47, 0x75, 0x61, 0x72, 0x61, 0x74, 0x69, 0x6e, 0x67, 0x75, 0x65, 0x74, 0xe1, 0x00};
const unsigned char sbtvd_state20_region_52_str[] = {0x42, 0x61, 0x6e, 0x61, 0x6e, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state20_region_53_str[] = {0x50, 0x61, 0x72, 0x61, 0x69, 0x62, 0x75, 0x6e, 0x61, 0x2f, 0x50, 0x61, 0x72, 0x61, 0x69, 0x74, 0x69, 0x6e, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state20_region_54_str[] = {0x43, 0x61, 0x72, 0x61, 0x67, 0x75, 0x61, 0x74, 0x61, 0x74, 0x75, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state20_region_55_str[] = {0x52, 0x65, 0x67, 0x69, 0x73, 0x74, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state20_region_56_str[] = {0x49, 0x74, 0x61, 0x6e, 0x68, 0x61, 0xe9, 0x6d, 0x00};
const unsigned char sbtvd_state20_region_57_str[] = {0x4f, 0x73, 0x61, 0x73, 0x63, 0x6f, 0x00};
const unsigned char sbtvd_state20_region_58_str[] = {0x46, 0x72, 0x61, 0x6e, 0x63, 0x6f, 0x20, 0x64, 0x61, 0x20, 0x52, 0x6f, 0x63, 0x68, 0x61, 0x00};
const unsigned char sbtvd_state20_region_59_str[] = {0x47, 0x75, 0x61, 0x72, 0x75, 0x6c, 0x68, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state20_region_60_str[] = {0x49, 0x74, 0x61, 0x70, 0x65, 0x63, 0x65, 0x72, 0x69, 0x63, 0x61, 0x20, 0x64, 0x61, 0x20, 0x53, 0x65, 0x72, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state20_region_61_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x50, 0x61, 0x75, 0x6c, 0x6f, 0x00};
const unsigned char sbtvd_state20_region_62_str[] = {0x4d, 0x6f, 0x67, 0x69, 0x20, 0x64, 0x61, 0x73, 0x20, 0x43, 0x72, 0x75, 0x7a, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state20_region_63_str[] = {0x53, 0x61, 0x6e, 0x74, 0x6f, 0x73, 0x00};

const unsigned char sbtvd_state21_region_01_str[] = {0x50, 0x61, 0x72, 0x61, 0x6e, 0x61, 0x76, 0x61, 0xed, 0x00};
const unsigned char sbtvd_state21_region_02_str[] = {0x55, 0x6d, 0x75, 0x61, 0x72, 0x61, 0x6d, 0x61, 0x00};
const unsigned char sbtvd_state21_region_03_str[] = {0x43, 0x69, 0x61, 0x6e, 0x6f, 0x72, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state21_region_04_str[] = {0x47, 0x6f, 0x69, 0x6f, 0x65, 0x72, 0xea, 0x00};
const unsigned char sbtvd_state21_region_05_str[] = {0x43, 0x61, 0x6d, 0x70, 0x6f, 0x20, 0x4d, 0x6f, 0x75, 0x72, 0xe3, 0x6f, 0x00};
const unsigned char sbtvd_state21_region_06_str[] = {0x41, 0x73, 0x74, 0x6f, 0x72, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state21_region_07_str[] = {0x50, 0x6f, 0x72, 0x65, 0x63, 0x61, 0x74, 0x75, 0x00};
const unsigned char sbtvd_state21_region_08_str[] = {0x46, 0x6c, 0x6f, 0x72, 0x61, 0xed, 0x00};
const unsigned char sbtvd_state21_region_09_str[] = {0x4d, 0x61, 0x72, 0x69, 0x6e, 0x67, 0xe1, 0x00};
const unsigned char sbtvd_state21_region_10_str[] = {0x41, 0x70, 0x75, 0x63, 0x61, 0x72, 0x61, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state21_region_11_str[] = {0x4c, 0x6f, 0x6e, 0x44, 0x72, 0x69, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state21_region_12_str[] = {0x46, 0x61, 0x78, 0x69, 0x6e, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state21_region_13_str[] = {0x49, 0x76, 0x61, 0x69, 0x70, 0x6f, 0x72, 0xe3, 0x00};
const unsigned char sbtvd_state21_region_14_str[] = {0x41, 0x73, 0x73, 0x61, 0xed, 0x00};
const unsigned char sbtvd_state21_region_15_str[] = {0x43, 0x6f, 0x72, 0x6e, 0xe9, 0x6c, 0x69, 0x6f, 0x20, 0x50, 0x72, 0x6f, 0x63, 0xf3, 0x70, 0x69, 0x6f, 0x00};
const unsigned char sbtvd_state21_region_16_str[] = {0x4a, 0x61, 0x63, 0x61, 0x72, 0x65, 0x7a, 0x69, 0x6e, 0x68, 0x6f, 0x00};
const unsigned char sbtvd_state21_region_17_str[] = {0x49, 0x62, 0x61, 0x69, 0x74, 0x69, 0x00};
const unsigned char sbtvd_state21_region_18_str[] = {0x57, 0x65, 0x6e, 0x63, 0x65, 0x73, 0x6c, 0x61, 0x75, 0x20, 0x42, 0x72, 0x61, 0x7a, 0x00};
const unsigned char sbtvd_state21_region_19_str[] = {0x54, 0x65, 0x6c, 0xea, 0x6d, 0x61, 0x63, 0x6f, 0x20, 0x42, 0x6f, 0x72, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state21_region_20_str[] = {0x4a, 0x61, 0x67, 0x75, 0x61, 0x72, 0x69, 0x61, 0xed, 0x76, 0x61, 0x00};
const unsigned char sbtvd_state21_region_21_str[] = {0x50, 0x6f, 0x6e, 0x74, 0x61, 0x20, 0x47, 0x72, 0x6f, 0x73, 0x73, 0x61, 0x00};
const unsigned char sbtvd_state21_region_22_str[] = {0x54, 0x6f, 0x6c, 0x65, 0x64, 0x6f, 0x00};
const unsigned char sbtvd_state21_region_23_str[] = {0x43, 0x61, 0x73, 0x63, 0x61, 0x76, 0x65, 0x6c, 0x00};
const unsigned char sbtvd_state21_region_24_str[] = {0x46, 0x6f, 0x7a, 0x20, 0x64, 0x6f, 0x20, 0x49, 0x67, 0x75, 0x61, 0xe7, 0x75, 0x00};
const unsigned char sbtvd_state21_region_25_str[] = {0x43, 0x61, 0x70, 0x61, 0x6e, 0x65, 0x6d, 0x61, 0x00};
const unsigned char sbtvd_state21_region_26_str[] = {0x46, 0x72, 0x61, 0x6e, 0x63, 0x69, 0x73, 0x63, 0x6f, 0x20, 0x42, 0x65, 0x6c, 0x74, 0x72, 0xe3, 0x6f, 0x00};
const unsigned char sbtvd_state21_region_27_str[] = {0x50, 0x61, 0x74, 0x6f, 0x20, 0x42, 0x72, 0x61, 0x6e, 0x63, 0x6f, 0x00};
const unsigned char sbtvd_state21_region_28_str[] = {0x50, 0x69, 0x74, 0x61, 0x6e, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state21_region_29_str[] = {0x47, 0x75, 0x61, 0x72, 0x61, 0x70, 0x75, 0x61, 0x76, 0x61, 0x00};
const unsigned char sbtvd_state21_region_30_str[] = {0x50, 0x61, 0x6c, 0x6d, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state21_region_31_str[] = {0x50, 0x72, 0x75, 0x64, 0x65, 0x6e, 0x74, 0xf3, 0x70, 0x6f, 0x6c, 0x69, 0x73, 0x00};
const unsigned char sbtvd_state21_region_32_str[] = {0x49, 0x72, 0x61, 0x74, 0x69, 0x00};
const unsigned char sbtvd_state21_region_33_str[] = {0x55, 0x6e, 0x69, 0xe3, 0x6f, 0x20, 0x64, 0x61, 0x20, 0x56, 0x69, 0x74, 0xf3, 0x72, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state21_region_34_str[] = {0x73, 0xe3, 0x6f, 0x20, 0x4d, 0x61, 0x74, 0x65, 0x75, 0x73, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x75, 0x6c, 0x00};
const unsigned char sbtvd_state21_region_35_str[] = {0x43, 0x65, 0x72, 0x72, 0x6f, 0x20, 0x41, 0x7a, 0x75, 0x6c, 0x00};
const unsigned char sbtvd_state21_region_36_str[] = {0x4c, 0x61, 0x70, 0x61, 0x00};
const unsigned char sbtvd_state21_region_37_str[] = {0x43, 0x75, 0x72, 0x69, 0x74, 0x69, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state21_region_38_str[] = {0x50, 0x61, 0x72, 0x61, 0x6e, 0x61, 0x67, 0x75, 0xe1, 0x00};
const unsigned char sbtvd_state21_region_39_str[] = {0x52, 0x69, 0x6f, 0x20, 0x4e, 0x65, 0x67, 0x72, 0x6f, 0x00};

const unsigned char sbtvd_state22_region_01_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x4d, 0x69, 0x67, 0x75, 0x65, 0x6c, 0x20, 0x64, 0x6f, 0x20, 0x4f, 0x65, 0x73, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state22_region_02_str[] = {0x43, 0x68, 0x61, 0x70, 0x65, 0x63, 0xf3, 0x00};
const unsigned char sbtvd_state22_region_03_str[] = {0x58, 0x61, 0x6e, 0x78, 0x65, 0x72, 0xea, 0x00};
const unsigned char sbtvd_state22_region_04_str[] = {0x4a, 0x6f, 0x61, 0xe7, 0x61, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state22_region_05_str[] = {0x43, 0x6f, 0x6e, 0x63, 0xf3, 0x72, 0x64, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state22_region_06_str[] = {0x43, 0x61, 0x6e, 0x6f, 0x69, 0x6e, 0x68, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state22_region_07_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x42, 0x65, 0x6e, 0x74, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x75, 0x6c, 0x00};
const unsigned char sbtvd_state22_region_08_str[] = {0x4a, 0x6f, 0x69, 0x6e, 0x76, 0x69, 0x6c, 0x6c, 0x65, 0x00};
const unsigned char sbtvd_state22_region_09_str[] = {0x43, 0x75, 0x72, 0x69, 0x74, 0x69, 0x62, 0x61, 0x6e, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state22_region_10_str[] = {0x43, 0x61, 0x6d, 0x70, 0x6f, 0x73, 0x20, 0x64, 0x65, 0x20, 0x4c, 0x61, 0x67, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state22_region_11_str[] = {0x52, 0x69, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x75, 0x6c, 0x00};
const unsigned char sbtvd_state22_region_12_str[] = {0x42, 0x6c, 0x75, 0x6d, 0x65, 0x6e, 0x61, 0x75, 0x00};
const unsigned char sbtvd_state22_region_13_str[] = {0x49, 0x74, 0x61, 0x6a, 0x61, 0xed, 0x00};
const unsigned char sbtvd_state22_region_14_str[] = {0x49, 0x74, 0x75, 0x70, 0x6f, 0x72, 0x61, 0x6e, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state22_region_15_str[] = {0x54, 0x69, 0x6a, 0x75, 0x63, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state22_region_16_str[] = {0x46, 0x6c, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0xf3, 0x70, 0x6f, 0x6c, 0x69, 0x73, 0x00};
const unsigned char sbtvd_state22_region_17_str[] = {0x54, 0x61, 0x62, 0x75, 0x6c, 0x65, 0x69, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state22_region_18_str[] = {0x54, 0x75, 0x62, 0x61, 0x72, 0xe3, 0x6f, 0x00};
const unsigned char sbtvd_state22_region_19_str[] = {0x43, 0x72, 0x69, 0x63, 0x69, 0xfa, 0x6d, 0x61, 0x00};
const unsigned char sbtvd_state22_region_20_str[] = {0x41, 0x72, 0x61, 0x72, 0x61, 0x6e, 0x67, 0x75, 0xe1, 0x00};

const unsigned char sbtvd_state23_region_01_str[] = {0x53, 0x61, 0x6e, 0x74, 0x61, 0x20, 0x52, 0x6f, 0x73, 0x61, 0x00};
const unsigned char sbtvd_state23_region_02_str[] = {0x54, 0x72, 0xea, 0x73, 0x20, 0x50, 0x61, 0x73, 0x73, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state23_region_03_str[] = {0x46, 0x72, 0x65, 0x64, 0x72, 0x69, 0x63, 0x6f, 0x20, 0x57, 0x65, 0x73, 0x74, 0x70, 0x68, 0x61, 0x6c, 0x65, 0x6e, 0x00};
const unsigned char sbtvd_state23_region_04_str[] = {0x45, 0x72, 0x65, 0x63, 0x68, 0x69, 0x6d, 0x00};
const unsigned char sbtvd_state23_region_05_str[] = {0x53, 0x61, 0x6e, 0x61, 0x6e, 0x64, 0x75, 0x76, 0x61, 0x00};
const unsigned char sbtvd_state23_region_06_str[] = {0x43, 0x65, 0x72, 0x72, 0x6f, 0x20, 0x4c, 0x61, 0x72, 0x67, 0x6f, 0x00};
const unsigned char sbtvd_state23_region_07_str[] = {0x53, 0x61, 0x6e, 0x74, 0x6f, 0x20, 0xc2, 0x6e, 0x67, 0x65, 0x6c, 0x6f, 0x00};
const unsigned char sbtvd_state23_region_08_str[] = {0x49, 0x6a, 0x75, 0xed, 0x00};
const unsigned char sbtvd_state23_region_09_str[] = {0x43, 0x61, 0x72, 0x61, 0x7a, 0x69, 0x6e, 0x68, 0x6f, 0x00};
const unsigned char sbtvd_state23_region_10_str[] = {0x50, 0x61, 0x73, 0x73, 0x6f, 0x20, 0x46, 0x75, 0x6e, 0x64, 0x6f, 0x00};
const unsigned char sbtvd_state23_region_11_str[] = {0x43, 0x72, 0x75, 0x7a, 0x20, 0x41, 0x6c, 0x74, 0x61, 0x00};
const unsigned char sbtvd_state23_region_12_str[] = {0x4e, 0xe3, 0x6f, 0x2d, 0x4d, 0x65, 0x2d, 0x54, 0x6f, 0x71, 0x75, 0x65, 0x00};
const unsigned char sbtvd_state23_region_13_str[] = {0x53, 0x6f, 0x6c, 0x65, 0x64, 0x61, 0x64, 0x65, 0x00};
const unsigned char sbtvd_state23_region_14_str[] = {0x47, 0x75, 0x61, 0x70, 0x6f, 0x72, 0xe9, 0x00};
const unsigned char sbtvd_state23_region_15_str[] = {0x56, 0x61, 0x63, 0x61, 0x72, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state23_region_16_str[] = {0x43, 0x61, 0x78, 0x69, 0x61, 0x73, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x75, 0x6c, 0x00};
const unsigned char sbtvd_state23_region_17_str[] = {0x53, 0x61, 0x6e, 0x74, 0x69, 0x61, 0x67, 0x6f, 0x00};
const unsigned char sbtvd_state23_region_18_str[] = {0x53, 0x61, 0x6e, 0x74, 0x61, 0x20, 0x4d, 0x61, 0x72, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state23_region_19_str[] = {0x52, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67, 0x61, 0x20, 0x53, 0x65, 0x63, 0x61, 0x00};
const unsigned char sbtvd_state23_region_20_str[] = {0x53, 0x61, 0x6e, 0x74, 0x61, 0x20, 0x43, 0x72, 0x75, 0x7a, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x75, 0x6c, 0x00};
const unsigned char sbtvd_state23_region_21_str[] = {0x4c, 0x61, 0x6a, 0x65, 0x61, 0x64, 0x6f, 0x2d, 0x45, 0x73, 0x74, 0x72, 0x65, 0x6c, 0x61, 0x00};
const unsigned char sbtvd_state23_region_22_str[] = {0x43, 0x61, 0x63, 0x68, 0x6f, 0x65, 0x69, 0x72, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x53, 0x75, 0x6c, 0x00};
const unsigned char sbtvd_state23_region_23_str[] = {0x4d, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x65, 0x67, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state23_region_24_str[] = {0x47, 0x72, 0x61, 0x6d, 0x61, 0x64, 0x6f, 0x2d, 0x43, 0x61, 0x6e, 0x65, 0x6c, 0x61, 0x00};
const unsigned char sbtvd_state23_region_25_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x4a, 0x65, 0xf4, 0x6e, 0x69, 0x6d, 0x6f, 0x00};
const unsigned char sbtvd_state23_region_26_str[] = {0x50, 0x6f, 0x72, 0x74, 0x6f, 0x20, 0x41, 0x6c, 0x65, 0x67, 0x72, 0x65, 0x00};
const unsigned char sbtvd_state23_region_27_str[] = {0x4f, 0x73, 0xf3, 0x72, 0x69, 0x6f, 0x00};
const unsigned char sbtvd_state23_region_28_str[] = {0x43, 0x61, 0x6d, 0x61, 0x71, 0x75, 0xe3, 0x00};
const unsigned char sbtvd_state23_region_29_str[] = {0x43, 0x61, 0x6d, 0x70, 0x61, 0x6e, 0x68, 0x61, 0x20, 0x4f, 0x63, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state23_region_30_str[] = {0x43, 0x61, 0x6d, 0x70, 0x61, 0x6e, 0x68, 0x61, 0x20, 0x43, 0x65, 0x6e, 0x74, 0x72, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state23_region_31_str[] = {0x43, 0x61, 0x6d, 0x70, 0x61, 0x6e, 0x68, 0x61, 0x20, 0x6d, 0x65, 0x72, 0x69, 0x64, 0x69, 0x6f, 0x6d, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state23_region_32_str[] = {0x53, 0x65, 0x72, 0x72, 0x61, 0x73, 0x20, 0x64, 0x65, 0x20, 0x53, 0x75, 0x64, 0x65, 0x73, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state23_region_33_str[] = {0x50, 0x65, 0x6c, 0x6f, 0x74, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state23_region_34_str[] = {0x4a, 0x61, 0x67, 0x75, 0x61, 0x72, 0xe3, 0x6f, 0x00};
const unsigned char sbtvd_state23_region_35_str[] = {0x4c, 0x69, 0x74, 0x6f, 0x72, 0x61, 0x6c, 0x20, 0x4c, 0x61, 0x67, 0x75, 0x6e, 0x61, 0x72, 0x00};

const unsigned char sbtvd_state24_region_01_str[] = {0x42, 0x61, 0x69, 0x78, 0x6f, 0x20, 0x50, 0x61, 0x6e, 0x74, 0x61, 0x6e, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state24_region_02_str[] = {0x41, 0x71, 0x75, 0x69, 0x64, 0x61, 0x75, 0x61, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state24_region_03_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x54, 0x61, 0x71, 0x75, 0x61, 0x72, 0x69, 0x00};
const unsigned char sbtvd_state24_region_04_str[] = {0x43, 0x61, 0x6c, 0x70, 0x6f, 0x20, 0x47, 0x72, 0x61, 0x6e, 0x64, 0x65, 0x00};
const unsigned char sbtvd_state24_region_05_str[] = {0x43, 0x61, 0x73, 0x73, 0x69, 0x6c, 0xe3, 0x6e, 0x64, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state24_region_06_str[] = {0x50, 0x61, 0x72, 0x61, 0x6e, 0x61, 0xed, 0x62, 0x61, 0x00};
const unsigned char sbtvd_state24_region_07_str[] = {0x54, 0x72, 0xea, 0x73, 0x20, 0x4c, 0x61, 0x67, 0x6f, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state24_region_08_str[] = {0x4e, 0x6f, 0x76, 0x61, 0x20, 0x41, 0x6e, 0x64, 0x72, 0x61, 0x64, 0x69, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state24_region_09_str[] = {0x42, 0x6f, 0x64, 0x6f, 0x71, 0x75, 0x65, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state24_region_10_str[] = {0x44, 0x6f, 0x75, 0x72, 0x61, 0x64, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state24_region_11_str[] = {0x49, 0x67, 0x75, 0x61, 0x74, 0x65, 0x6d, 0x69, 0x00};

const unsigned char sbtvd_state25_region_01_str[] = {0x41, 0x72, 0x69, 0x70, 0x75, 0x61, 0x6e, 0xe3, 0x00};
const unsigned char sbtvd_state25_region_02_str[] = {0x41, 0x6c, 0x74, 0x61, 0x20, 0x46, 0x6c, 0x6f, 0x72, 0x65, 0x73, 0x74, 0x61, 0x00};
const unsigned char sbtvd_state25_region_03_str[] = {0x43, 0x6f, 0x6c, 0xed, 0x64, 0x65, 0x72, 0x00};
const unsigned char sbtvd_state25_region_04_str[] = {0x50, 0x61, 0x72, 0x65, 0x63, 0x69, 0x73, 0x00};
const unsigned char sbtvd_state25_region_05_str[] = {0x41, 0x72, 0x69, 0x6e, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state25_region_06_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x54, 0x65, 0x6c, 0x65, 0x73, 0x20, 0x50, 0x69, 0x72, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state25_region_07_str[] = {0x53, 0x69, 0x6e, 0x6f, 0x70, 0x00};
const unsigned char sbtvd_state25_region_08_str[] = {0x50, 0x61, 0x72, 0x61, 0x6e, 0x61, 0x74, 0x69, 0x6e, 0x67, 0x61, 0x00};
const unsigned char sbtvd_state25_region_09_str[] = {0x4e, 0x6f, 0x72, 0x74, 0x65, 0x20, 0x41, 0x72, 0x61, 0x67, 0x75, 0x61, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state25_region_10_str[] = {0x43, 0x61, 0x6e, 0x61, 0x72, 0x61, 0x6e, 0x61, 0x00};
const unsigned char sbtvd_state25_region_11_str[] = {0x4d, 0xe9, 0x64, 0x69, 0x6f, 0x20, 0x41, 0x72, 0x61, 0x67, 0x75, 0x61, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state25_region_12_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x47, 0x75, 0x61, 0x70, 0x6f, 0x72, 0xe9, 0x00};
const unsigned char sbtvd_state25_region_13_str[] = {0x54, 0x61, 0x6e, 0x67, 0x61, 0x72, 0xe1, 0x20, 0x64, 0x61, 0x20, 0x53, 0x65, 0x72, 0x72, 0x61, 0x00};
const unsigned char sbtvd_state25_region_14_str[] = {0x4a, 0x61, 0x75, 0x72, 0x75, 0x00};
const unsigned char sbtvd_state25_region_15_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x50, 0x61, 0x72, 0x61, 0x67, 0x75, 0x61, 0x69, 0x00};
const unsigned char sbtvd_state25_region_16_str[] = {0x52, 0x6f, 0x73, 0xe1, 0x72, 0x69, 0x6f, 0x20, 0x4f, 0x65, 0x73, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state25_region_17_str[] = {0x43, 0x75, 0x69, 0x61, 0x62, 0xe1, 0x00};
const unsigned char sbtvd_state25_region_18_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x50, 0x61, 0x6e, 0x74, 0x61, 0x6e, 0x61, 0x6c, 0x00};
const unsigned char sbtvd_state25_region_19_str[] = {0x50, 0x72, 0x69, 0x6d, 0x61, 0x76, 0x65, 0x72, 0x61, 0x20, 0x64, 0x6f, 0x20, 0x4c, 0x65, 0x73, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state25_region_20_str[] = {0x54, 0x65, 0x73, 0x6f, 0x75, 0x72, 0x6f, 0x00};
const unsigned char sbtvd_state25_region_21_str[] = {0x52, 0x6f, 0x6e, 0x64, 0x6f, 0x6e, 0xf3, 0x70, 0x6f, 0x6c, 0x69, 0x73, 0x00};
const unsigned char sbtvd_state25_region_22_str[] = {0x41, 0x6c, 0x74, 0x6f, 0x20, 0x41, 0x72, 0x61, 0x67, 0x75, 0x61, 0x69, 0x61, 0x00};

const unsigned char sbtvd_state26_region_01_str[] = {0x53, 0xe3, 0x6f, 0x20, 0x4d, 0x69, 0x67, 0x75, 0x65, 0x6c, 0x20, 0x64, 0x6f, 0x20, 0x41, 0x72, 0x61, 0x67, 0x75, 0x61, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state26_region_02_str[] = {0x52, 0x69, 0x6f, 0x20, 0x56, 0x65, 0x72, 0x6d, 0x65, 0x6c, 0x68, 0x6f, 0x00};
const unsigned char sbtvd_state26_region_03_str[] = {0x41, 0x72, 0x61, 0x67, 0x61, 0x72, 0xe7, 0x61, 0x73, 0x00};
const unsigned char sbtvd_state26_region_04_str[] = {0x50, 0x6f, 0x72, 0x61, 0x6e, 0x67, 0x61, 0x74, 0x75, 0x00};
const unsigned char sbtvd_state26_region_05_str[] = {0x43, 0x68, 0x61, 0x70, 0x61, 0x64, 0x61, 0x20, 0x64, 0x6f, 0x73, 0x20, 0x56, 0x65, 0x61, 0x64, 0x65, 0x69, 0x72, 0x6f, 0x73, 0x00};
const unsigned char sbtvd_state26_region_06_str[] = {0x43, 0x65, 0x72, 0x65, 0x73, 0x00};
const unsigned char sbtvd_state26_region_07_str[] = {0x41, 0x6e, 0xe1, 0x70, 0x6f, 0x6c, 0x69, 0x73, 0x00};
const unsigned char sbtvd_state26_region_08_str[] = {0x49, 0x70, 0x6f, 0x72, 0xe1, 0x00};
const unsigned char sbtvd_state26_region_09_str[] = {0x41, 0x6e, 0x69, 0x63, 0x75, 0x6e, 0x73, 0x00};
const unsigned char sbtvd_state26_region_10_str[] = {0x47, 0x6f, 0x69, 0xe2, 0x6e, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state26_region_11_str[] = {0x56, 0xe3, 0x6f, 0x20, 0x64, 0x6f, 0x20, 0x50, 0x61, 0x72, 0x61, 0x6e, 0xe3, 0x00};
const unsigned char sbtvd_state26_region_12_str[] = {0x45, 0x6e, 0x74, 0x6f, 0x72, 0x6e, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x42, 0x72, 0x61, 0x73, 0xed, 0x6c, 0x69, 0x61, 0x00};
const unsigned char sbtvd_state26_region_13_str[] = {0x53, 0x75, 0x64, 0x6f, 0x65, 0x73, 0x74, 0x65, 0x20, 0x64, 0x65, 0x20, 0x47, 0x6f, 0x69, 0xe1, 0x73, 0x00};
const unsigned char sbtvd_state26_region_14_str[] = {0x56, 0x61, 0x6c, 0x65, 0x20, 0x64, 0x6f, 0x20, 0x52, 0x69, 0x6f, 0x20, 0x64, 0x69, 0x73, 0x20, 0x42, 0x6f, 0x69, 0x73, 0x00};
const unsigned char sbtvd_state26_region_15_str[] = {0x4d, 0x65, 0x69, 0x61, 0x20, 0x50, 0x6f, 0x6e, 0x74, 0x65, 0x00};
const unsigned char sbtvd_state26_region_16_str[] = {0x50, 0x69, 0x72, 0x65, 0x73, 0x20, 0x64, 0x6f, 0x20, 0x52, 0x69, 0x6f, 0x00};
const unsigned char sbtvd_state26_region_17_str[] = {0x43, 0x61, 0x74, 0x61, 0x6c, 0xe3, 0x6f, 0x00};
const unsigned char sbtvd_state26_region_18_str[] = {0x51, 0x75, 0x69, 0x72, 0x69, 0x6e, 0xf3, 0x70, 0x6f, 0x6c, 0x69, 0x73, 0x00};

const unsigned char sbtvd_state27_region_01_str[] = {0x42, 0x72, 0x61, 0x73, 0xed, 0x6c, 0x69, 0x61, 0x00};


const unsigned char * sbtvd_state01_region_strpool[SBTVD_STATE01_REGION_NUM_MAX] = 
{
	sbtvd_state01_region_01_str, sbtvd_state01_region_02_str, sbtvd_state01_region_03_str,
	sbtvd_state01_region_04_str, sbtvd_state01_region_05_str, sbtvd_state01_region_06_str,
	sbtvd_state01_region_07_str, sbtvd_state01_region_08_str
};

const unsigned char * sbtvd_state02_region_strpool[SBTVD_STATE02_REGION_NUM_MAX] = 
{
	sbtvd_state02_region_01_str, sbtvd_state02_region_02_str, sbtvd_state02_region_03_str,
	sbtvd_state02_region_04_str, sbtvd_state02_region_05_str
};

const unsigned char * sbtvd_state03_region_strpool[SBTVD_STATE03_REGION_NUM_MAX] = 
{
	sbtvd_state03_region_01_str, sbtvd_state03_region_02_str, sbtvd_state03_region_03_str,
	sbtvd_state03_region_04_str, sbtvd_state03_region_05_str
};

const unsigned char * sbtvd_state04_region_strpool[SBTVD_STATE04_REGION_NUM_MAX] = 
{
	sbtvd_state04_region_01_str, sbtvd_state04_region_02_str, sbtvd_state04_region_03_str,
	sbtvd_state04_region_04_str
};

const unsigned char * sbtvd_state05_region_strpool[SBTVD_STATE05_REGION_NUM_MAX] = 
{
	sbtvd_state05_region_01_str, sbtvd_state05_region_02_str, sbtvd_state05_region_03_str,
	sbtvd_state05_region_04_str, sbtvd_state05_region_05_str, sbtvd_state05_region_06_str, 
	sbtvd_state05_region_07_str, sbtvd_state05_region_08_str, sbtvd_state05_region_09_str, 
	sbtvd_state05_region_10_str, sbtvd_state05_region_11_str, sbtvd_state05_region_12_str, 
	sbtvd_state05_region_13_str, sbtvd_state05_region_14_str, sbtvd_state05_region_15_str, 
	sbtvd_state05_region_16_str, sbtvd_state05_region_17_str, sbtvd_state05_region_18_str, 
	sbtvd_state05_region_19_str, sbtvd_state05_region_20_str, sbtvd_state05_region_21_str, 
	sbtvd_state05_region_22_str
};

const unsigned char * sbtvd_state06_region_strpool[SBTVD_STATE06_REGION_NUM_MAX] = 
{
	sbtvd_state06_region_01_str, sbtvd_state06_region_02_str, sbtvd_state06_region_03_str,
	sbtvd_state06_region_04_str
};

const unsigned char * sbtvd_state07_region_strpool[SBTVD_STATE07_REGION_NUM_MAX] = 
{
	sbtvd_state07_region_01_str, sbtvd_state07_region_02_str, sbtvd_state07_region_03_str,
	sbtvd_state07_region_04_str, sbtvd_state07_region_05_str, sbtvd_state07_region_06_str, 
	sbtvd_state07_region_07_str, sbtvd_state07_region_08_str
};

const unsigned char * sbtvd_state08_region_strpool[SBTVD_STATE08_REGION_NUM_MAX] = 
{
	sbtvd_state08_region_01_str, sbtvd_state08_region_02_str, sbtvd_state08_region_03_str,
	sbtvd_state08_region_04_str, sbtvd_state08_region_05_str, sbtvd_state08_region_06_str, 
	sbtvd_state08_region_07_str, sbtvd_state08_region_08_str, sbtvd_state08_region_09_str, 
	sbtvd_state08_region_10_str, sbtvd_state08_region_11_str, sbtvd_state08_region_12_str, 
	sbtvd_state08_region_13_str, sbtvd_state08_region_14_str, sbtvd_state08_region_15_str, 
	sbtvd_state08_region_16_str, sbtvd_state08_region_17_str, sbtvd_state08_region_18_str, 
	sbtvd_state08_region_19_str, sbtvd_state08_region_20_str, sbtvd_state08_region_21_str
};

const unsigned char * sbtvd_state09_region_strpool[SBTVD_STATE09_REGION_NUM_MAX] = 
{
	sbtvd_state09_region_01_str, sbtvd_state09_region_02_str, sbtvd_state09_region_03_str,
	sbtvd_state09_region_04_str, sbtvd_state09_region_05_str, sbtvd_state09_region_06_str, 
	sbtvd_state09_region_07_str, sbtvd_state09_region_08_str, sbtvd_state09_region_09_str, 
	sbtvd_state09_region_10_str, sbtvd_state09_region_11_str, sbtvd_state09_region_12_str, 
	sbtvd_state09_region_13_str, sbtvd_state09_region_14_str, sbtvd_state09_region_15_str
};

const unsigned char * sbtvd_state10_region_strpool[SBTVD_STATE10_REGION_NUM_MAX] = 
{
	sbtvd_state10_region_01_str, sbtvd_state10_region_02_str, sbtvd_state10_region_03_str,
	sbtvd_state10_region_04_str, sbtvd_state10_region_05_str, sbtvd_state10_region_06_str, 
	sbtvd_state10_region_07_str, sbtvd_state10_region_08_str, sbtvd_state10_region_09_str, 
	sbtvd_state10_region_10_str, sbtvd_state10_region_11_str, sbtvd_state10_region_12_str, 
	sbtvd_state10_region_13_str, sbtvd_state10_region_14_str, sbtvd_state10_region_15_str, 
	sbtvd_state10_region_16_str, sbtvd_state10_region_17_str, sbtvd_state10_region_18_str, 
	sbtvd_state10_region_19_str, sbtvd_state10_region_20_str, sbtvd_state10_region_21_str, 
	sbtvd_state10_region_22_str, sbtvd_state10_region_23_str, sbtvd_state10_region_24_str,
	sbtvd_state10_region_25_str, sbtvd_state10_region_26_str, sbtvd_state10_region_27_str,
	sbtvd_state10_region_28_str, sbtvd_state10_region_29_str, sbtvd_state10_region_30_str,
	sbtvd_state10_region_31_str, sbtvd_state10_region_32_str, sbtvd_state10_region_33_str
};

const unsigned char * sbtvd_state11_region_strpool[SBTVD_STATE11_REGION_NUM_MAX] = 
{
	sbtvd_state11_region_01_str, sbtvd_state11_region_02_str, sbtvd_state11_region_03_str,
	sbtvd_state11_region_04_str, sbtvd_state11_region_05_str, sbtvd_state11_region_06_str, 
	sbtvd_state11_region_07_str, sbtvd_state11_region_08_str, sbtvd_state11_region_09_str, 
	sbtvd_state11_region_10_str, sbtvd_state11_region_11_str, sbtvd_state11_region_12_str, 
	sbtvd_state11_region_13_str, sbtvd_state11_region_14_str, sbtvd_state11_region_15_str, 
	sbtvd_state11_region_16_str, sbtvd_state11_region_17_str, sbtvd_state11_region_18_str, 
	sbtvd_state11_region_19_str
};

const unsigned char * sbtvd_state12_region_strpool[SBTVD_STATE12_REGION_NUM_MAX] = 
{
	sbtvd_state12_region_01_str, sbtvd_state12_region_02_str, sbtvd_state12_region_03_str,
	sbtvd_state12_region_04_str, sbtvd_state12_region_05_str, sbtvd_state12_region_06_str, 
	sbtvd_state12_region_07_str, sbtvd_state12_region_08_str, sbtvd_state12_region_09_str, 
	sbtvd_state12_region_10_str, sbtvd_state12_region_11_str, sbtvd_state12_region_12_str, 
	sbtvd_state12_region_13_str, sbtvd_state12_region_14_str, sbtvd_state12_region_15_str, 
	sbtvd_state12_region_16_str, sbtvd_state12_region_17_str, sbtvd_state12_region_18_str, 
	sbtvd_state12_region_19_str, sbtvd_state12_region_20_str, sbtvd_state12_region_21_str, 
	sbtvd_state12_region_22_str, sbtvd_state12_region_23_str
};

const unsigned char * sbtvd_state13_region_strpool[SBTVD_STATE13_REGION_NUM_MAX] = 
{
	sbtvd_state13_region_01_str, sbtvd_state13_region_02_str, sbtvd_state13_region_03_str,
	sbtvd_state13_region_04_str, sbtvd_state13_region_05_str, sbtvd_state13_region_06_str, 
	sbtvd_state13_region_07_str, sbtvd_state13_region_08_str, sbtvd_state13_region_09_str, 
	sbtvd_state13_region_10_str, sbtvd_state13_region_11_str, sbtvd_state13_region_12_str, 
	sbtvd_state13_region_13_str, sbtvd_state13_region_14_str, sbtvd_state13_region_15_str, 
	sbtvd_state13_region_16_str, sbtvd_state13_region_17_str, sbtvd_state13_region_18_str, 
	sbtvd_state13_region_19_str
};

const unsigned char * sbtvd_state14_region_strpool[SBTVD_STATE14_REGION_NUM_MAX] = 
{
	sbtvd_state14_region_01_str, sbtvd_state14_region_02_str, sbtvd_state14_region_03_str,
	sbtvd_state14_region_04_str, sbtvd_state14_region_05_str, sbtvd_state14_region_06_str, 
	sbtvd_state14_region_07_str, sbtvd_state14_region_08_str, sbtvd_state14_region_09_str, 
	sbtvd_state14_region_10_str, sbtvd_state14_region_11_str, sbtvd_state14_region_12_str, 
	sbtvd_state14_region_13_str
};

const unsigned char * sbtvd_state15_region_strpool[SBTVD_STATE15_REGION_NUM_MAX] = 
{
	sbtvd_state15_region_01_str, sbtvd_state15_region_02_str, sbtvd_state15_region_03_str,
	sbtvd_state15_region_04_str, sbtvd_state15_region_05_str, sbtvd_state15_region_06_str, 
	sbtvd_state15_region_07_str, sbtvd_state15_region_08_str, sbtvd_state15_region_09_str, 
	sbtvd_state15_region_10_str, sbtvd_state15_region_11_str, sbtvd_state15_region_12_str, 
	sbtvd_state15_region_13_str
};

const unsigned char * sbtvd_state16_region_strpool[SBTVD_STATE16_REGION_NUM_MAX] = 
{
	sbtvd_state16_region_01_str, sbtvd_state16_region_02_str, sbtvd_state16_region_03_str,
	sbtvd_state16_region_04_str, sbtvd_state16_region_05_str, sbtvd_state16_region_06_str, 
	sbtvd_state16_region_07_str, sbtvd_state16_region_08_str, sbtvd_state16_region_09_str, 
	sbtvd_state16_region_10_str, sbtvd_state16_region_11_str, sbtvd_state16_region_12_str, 
	sbtvd_state16_region_13_str, sbtvd_state16_region_14_str, sbtvd_state16_region_15_str, 
	sbtvd_state16_region_16_str, sbtvd_state16_region_17_str, sbtvd_state16_region_18_str, 
	sbtvd_state16_region_19_str, sbtvd_state16_region_20_str, sbtvd_state16_region_21_str, 
	sbtvd_state16_region_22_str, sbtvd_state16_region_23_str, sbtvd_state16_region_24_str,
	sbtvd_state16_region_25_str, sbtvd_state16_region_26_str, sbtvd_state16_region_27_str,
	sbtvd_state16_region_28_str, sbtvd_state16_region_29_str, sbtvd_state16_region_30_str,
	sbtvd_state16_region_31_str, sbtvd_state16_region_32_str
};

const unsigned char * sbtvd_state17_region_strpool[SBTVD_STATE17_REGION_NUM_MAX] = 
{
	sbtvd_state17_region_01_str, sbtvd_state17_region_02_str, sbtvd_state17_region_03_str, 
	sbtvd_state17_region_04_str, sbtvd_state17_region_05_str, sbtvd_state17_region_06_str,
	sbtvd_state17_region_07_str, sbtvd_state17_region_08_str, sbtvd_state17_region_09_str, 
	sbtvd_state17_region_10_str, sbtvd_state17_region_11_str, sbtvd_state17_region_12_str,
	sbtvd_state17_region_13_str, sbtvd_state17_region_14_str, sbtvd_state17_region_15_str,
	sbtvd_state17_region_16_str, sbtvd_state17_region_17_str, sbtvd_state17_region_18_str,
	sbtvd_state17_region_19_str, sbtvd_state17_region_20_str, sbtvd_state17_region_21_str,
	sbtvd_state17_region_22_str, sbtvd_state17_region_23_str, sbtvd_state17_region_24_str,
	sbtvd_state17_region_25_str, sbtvd_state17_region_26_str, sbtvd_state17_region_27_str,
	sbtvd_state17_region_28_str, sbtvd_state17_region_29_str, sbtvd_state17_region_30_str,
	sbtvd_state17_region_31_str, sbtvd_state17_region_32_str, sbtvd_state17_region_33_str,
	sbtvd_state17_region_34_str, sbtvd_state17_region_35_str, sbtvd_state17_region_36_str,
	sbtvd_state17_region_37_str, sbtvd_state17_region_38_str, sbtvd_state17_region_39_str,
	sbtvd_state17_region_40_str, sbtvd_state17_region_41_str, sbtvd_state17_region_42_str,
	sbtvd_state17_region_43_str, sbtvd_state17_region_44_str, sbtvd_state17_region_45_str,
	sbtvd_state17_region_46_str, sbtvd_state17_region_47_str, sbtvd_state17_region_48_str,
	sbtvd_state17_region_49_str, sbtvd_state17_region_50_str, sbtvd_state17_region_51_str,
	sbtvd_state17_region_52_str, sbtvd_state17_region_53_str, sbtvd_state17_region_54_str,
	sbtvd_state17_region_55_str, sbtvd_state17_region_56_str, sbtvd_state17_region_57_str,
	sbtvd_state17_region_58_str, sbtvd_state17_region_59_str, sbtvd_state17_region_60_str,
	sbtvd_state17_region_61_str, sbtvd_state17_region_62_str, sbtvd_state17_region_63_str,
	sbtvd_state17_region_64_str, sbtvd_state17_region_65_str, sbtvd_state17_region_66_str
};

const unsigned char * sbtvd_state18_region_strpool[SBTVD_STATE18_REGION_NUM_MAX] = 
{
	sbtvd_state18_region_01_str, sbtvd_state18_region_02_str, sbtvd_state18_region_03_str, 
	sbtvd_state18_region_04_str, sbtvd_state18_region_05_str, sbtvd_state18_region_06_str,
	sbtvd_state18_region_07_str, sbtvd_state18_region_08_str, sbtvd_state18_region_09_str, 
	sbtvd_state18_region_10_str, sbtvd_state18_region_11_str, sbtvd_state18_region_12_str,
	sbtvd_state18_region_13_str
};

const unsigned char * sbtvd_state19_region_strpool[SBTVD_STATE19_REGION_NUM_MAX] = 
{
	sbtvd_state19_region_01_str, sbtvd_state19_region_02_str, sbtvd_state19_region_03_str, 
	sbtvd_state19_region_04_str, sbtvd_state19_region_05_str, sbtvd_state19_region_06_str,
	sbtvd_state19_region_07_str, sbtvd_state19_region_08_str, sbtvd_state19_region_09_str, 
	sbtvd_state19_region_10_str, sbtvd_state19_region_11_str, sbtvd_state19_region_12_str,
	sbtvd_state19_region_13_str, sbtvd_state19_region_14_str, sbtvd_state19_region_15_str,
	sbtvd_state19_region_16_str, sbtvd_state19_region_17_str, sbtvd_state19_region_18_str
};

const unsigned char * sbtvd_state20_region_strpool[SBTVD_STATE20_REGION_NUM_MAX] = 
{
	sbtvd_state20_region_01_str, sbtvd_state20_region_02_str, sbtvd_state20_region_03_str, 
	sbtvd_state20_region_04_str, sbtvd_state20_region_05_str, sbtvd_state20_region_06_str,
	sbtvd_state20_region_07_str, sbtvd_state20_region_08_str, sbtvd_state20_region_09_str, 
	sbtvd_state20_region_10_str, sbtvd_state20_region_11_str, sbtvd_state20_region_12_str,
	sbtvd_state20_region_13_str, sbtvd_state20_region_14_str, sbtvd_state20_region_15_str,
	sbtvd_state20_region_16_str, sbtvd_state20_region_17_str, sbtvd_state20_region_18_str,
	sbtvd_state20_region_19_str, sbtvd_state20_region_20_str, sbtvd_state20_region_21_str,
	sbtvd_state20_region_22_str, sbtvd_state20_region_23_str, sbtvd_state20_region_24_str,
	sbtvd_state20_region_25_str, sbtvd_state20_region_26_str, sbtvd_state20_region_27_str,
	sbtvd_state20_region_28_str, sbtvd_state20_region_29_str, sbtvd_state20_region_30_str,
	sbtvd_state20_region_31_str, sbtvd_state20_region_32_str, sbtvd_state20_region_33_str,
	sbtvd_state20_region_34_str, sbtvd_state20_region_35_str, sbtvd_state20_region_36_str,
	sbtvd_state20_region_37_str, sbtvd_state20_region_38_str, sbtvd_state20_region_39_str,
	sbtvd_state20_region_40_str, sbtvd_state20_region_41_str, sbtvd_state20_region_42_str,
	sbtvd_state20_region_43_str, sbtvd_state20_region_44_str, sbtvd_state20_region_45_str,
	sbtvd_state20_region_46_str, sbtvd_state20_region_47_str, sbtvd_state20_region_48_str,
	sbtvd_state20_region_49_str, sbtvd_state20_region_50_str, sbtvd_state20_region_51_str,
	sbtvd_state20_region_52_str, sbtvd_state20_region_53_str, sbtvd_state20_region_54_str,
	sbtvd_state20_region_55_str, sbtvd_state20_region_56_str, sbtvd_state20_region_57_str,
	sbtvd_state20_region_58_str, sbtvd_state20_region_59_str, sbtvd_state20_region_60_str,
	sbtvd_state20_region_61_str, sbtvd_state20_region_62_str, sbtvd_state20_region_63_str
};

const unsigned char * sbtvd_state21_region_strpool[SBTVD_STATE21_REGION_NUM_MAX] = 
{
	sbtvd_state21_region_01_str, sbtvd_state21_region_02_str, sbtvd_state21_region_03_str, 
	sbtvd_state21_region_04_str, sbtvd_state21_region_05_str, sbtvd_state21_region_06_str,
	sbtvd_state21_region_07_str, sbtvd_state21_region_08_str, sbtvd_state21_region_09_str, 
	sbtvd_state21_region_10_str, sbtvd_state21_region_11_str, sbtvd_state21_region_12_str,
	sbtvd_state21_region_13_str, sbtvd_state21_region_14_str, sbtvd_state21_region_15_str,
	sbtvd_state21_region_16_str, sbtvd_state21_region_17_str, sbtvd_state21_region_18_str,
	sbtvd_state21_region_19_str, sbtvd_state21_region_20_str, sbtvd_state21_region_21_str,
	sbtvd_state21_region_22_str, sbtvd_state21_region_23_str, sbtvd_state21_region_24_str,
	sbtvd_state21_region_25_str, sbtvd_state21_region_26_str, sbtvd_state21_region_27_str,
	sbtvd_state21_region_28_str, sbtvd_state21_region_29_str, sbtvd_state21_region_30_str,
	sbtvd_state21_region_31_str, sbtvd_state21_region_32_str, sbtvd_state21_region_33_str,
	sbtvd_state21_region_34_str, sbtvd_state21_region_35_str, sbtvd_state21_region_36_str,
	sbtvd_state21_region_37_str, sbtvd_state21_region_38_str, sbtvd_state21_region_39_str
};

const unsigned char * sbtvd_state22_region_strpool[SBTVD_STATE22_REGION_NUM_MAX] = 
{
	sbtvd_state22_region_01_str, sbtvd_state22_region_02_str, sbtvd_state22_region_03_str, 
	sbtvd_state22_region_04_str, sbtvd_state22_region_05_str, sbtvd_state22_region_06_str,
	sbtvd_state22_region_07_str, sbtvd_state22_region_08_str, sbtvd_state22_region_09_str, 
	sbtvd_state22_region_10_str, sbtvd_state22_region_11_str, sbtvd_state22_region_12_str,
	sbtvd_state22_region_13_str, sbtvd_state22_region_14_str, sbtvd_state22_region_15_str,
	sbtvd_state22_region_16_str, sbtvd_state22_region_17_str, sbtvd_state22_region_18_str,
	sbtvd_state22_region_19_str, sbtvd_state22_region_20_str
};

const unsigned char * sbtvd_state23_region_strpool[SBTVD_STATE23_REGION_NUM_MAX] = 
{
	sbtvd_state23_region_01_str, sbtvd_state23_region_02_str, sbtvd_state23_region_03_str, 
	sbtvd_state23_region_04_str, sbtvd_state23_region_05_str, sbtvd_state23_region_06_str,
	sbtvd_state23_region_07_str, sbtvd_state23_region_08_str, sbtvd_state23_region_09_str, 
	sbtvd_state23_region_10_str, sbtvd_state23_region_11_str, sbtvd_state23_region_12_str,
	sbtvd_state23_region_13_str, sbtvd_state23_region_14_str, sbtvd_state23_region_15_str,
	sbtvd_state23_region_16_str, sbtvd_state23_region_17_str, sbtvd_state23_region_18_str,
	sbtvd_state23_region_19_str, sbtvd_state23_region_20_str, sbtvd_state23_region_21_str,
	sbtvd_state23_region_22_str, sbtvd_state23_region_23_str, sbtvd_state23_region_24_str,
	sbtvd_state23_region_25_str, sbtvd_state23_region_26_str, sbtvd_state23_region_27_str,
	sbtvd_state23_region_28_str, sbtvd_state23_region_29_str, sbtvd_state23_region_30_str,
	sbtvd_state23_region_31_str, sbtvd_state23_region_32_str, sbtvd_state23_region_33_str,
	sbtvd_state23_region_34_str, sbtvd_state23_region_35_str
};

const unsigned char * sbtvd_state24_region_strpool[SBTVD_STATE24_REGION_NUM_MAX] = 
{
	sbtvd_state24_region_01_str, sbtvd_state24_region_02_str, sbtvd_state24_region_03_str, 
	sbtvd_state24_region_04_str, sbtvd_state24_region_05_str, sbtvd_state24_region_06_str,
	sbtvd_state24_region_07_str, sbtvd_state24_region_08_str, sbtvd_state24_region_09_str, 
	sbtvd_state24_region_10_str, sbtvd_state24_region_11_str
};

const unsigned char * sbtvd_state25_region_strpool[SBTVD_STATE25_REGION_NUM_MAX] = 
{
	sbtvd_state25_region_01_str, sbtvd_state25_region_02_str, sbtvd_state25_region_03_str, 
	sbtvd_state25_region_04_str, sbtvd_state25_region_05_str, sbtvd_state25_region_06_str, 
	sbtvd_state25_region_07_str, sbtvd_state25_region_08_str, sbtvd_state25_region_09_str, 
	sbtvd_state25_region_10_str, sbtvd_state25_region_11_str, sbtvd_state25_region_12_str, 
	sbtvd_state25_region_13_str, sbtvd_state25_region_14_str, sbtvd_state25_region_15_str, 
	sbtvd_state25_region_16_str, sbtvd_state25_region_17_str, sbtvd_state25_region_18_str, 
	sbtvd_state25_region_19_str, sbtvd_state25_region_20_str, sbtvd_state25_region_21_str, 
	sbtvd_state25_region_22_str
};

const unsigned char * sbtvd_state26_region_strpool[SBTVD_STATE26_REGION_NUM_MAX] = 
{
	sbtvd_state26_region_01_str, sbtvd_state26_region_02_str, sbtvd_state26_region_03_str, 
	sbtvd_state26_region_04_str, sbtvd_state26_region_05_str, sbtvd_state26_region_06_str,
	sbtvd_state26_region_07_str, sbtvd_state26_region_08_str, sbtvd_state26_region_09_str, 
	sbtvd_state26_region_10_str, sbtvd_state26_region_11_str, sbtvd_state26_region_12_str,
	sbtvd_state26_region_13_str, sbtvd_state26_region_14_str, sbtvd_state26_region_15_str,
	sbtvd_state26_region_16_str, sbtvd_state26_region_17_str, sbtvd_state26_region_18_str
};

const unsigned char * sbtvd_state27_region_strpool[SBTVD_STATE27_REGION_NUM_MAX] = 
{
	sbtvd_state27_region_01_str
};


const unsigned char sbtvd_region_num_values[SBTVD_STATE_NUM_MAX] = 
{
	SBTVD_STATE01_REGION_NUM_MAX, 	SBTVD_STATE02_REGION_NUM_MAX, 	SBTVD_STATE03_REGION_NUM_MAX,
	SBTVD_STATE04_REGION_NUM_MAX, 	SBTVD_STATE05_REGION_NUM_MAX, 	SBTVD_STATE06_REGION_NUM_MAX,
	SBTVD_STATE07_REGION_NUM_MAX, 	SBTVD_STATE08_REGION_NUM_MAX, 	SBTVD_STATE09_REGION_NUM_MAX,
	SBTVD_STATE10_REGION_NUM_MAX, 	SBTVD_STATE11_REGION_NUM_MAX, 	SBTVD_STATE12_REGION_NUM_MAX,
	SBTVD_STATE13_REGION_NUM_MAX, 	SBTVD_STATE14_REGION_NUM_MAX, 	SBTVD_STATE15_REGION_NUM_MAX,
	SBTVD_STATE16_REGION_NUM_MAX, 	SBTVD_STATE17_REGION_NUM_MAX, 	SBTVD_STATE18_REGION_NUM_MAX,
	SBTVD_STATE19_REGION_NUM_MAX, 	SBTVD_STATE20_REGION_NUM_MAX, 	SBTVD_STATE21_REGION_NUM_MAX,
	SBTVD_STATE22_REGION_NUM_MAX, 	SBTVD_STATE23_REGION_NUM_MAX, 	SBTVD_STATE24_REGION_NUM_MAX,
	SBTVD_STATE25_REGION_NUM_MAX, 	SBTVD_STATE26_REGION_NUM_MAX, 	SBTVD_STATE27_REGION_NUM_MAX
};

const SBTVD_AREACODE_STRPOOL sbtvd_area_strpool[SBTVD_STATE_NUM_MAX] = 
{
	{ sbtvd_state_01_str, sbtvd_state01_region_strpool,  (unsigned char)SBTVD_STATE01_REGION_NUM_MAX},
	{ sbtvd_state_02_str, sbtvd_state02_region_strpool , (unsigned char)SBTVD_STATE02_REGION_NUM_MAX},
	{ sbtvd_state_03_str, sbtvd_state03_region_strpool , (unsigned char)SBTVD_STATE03_REGION_NUM_MAX},
	{ sbtvd_state_04_str, sbtvd_state04_region_strpool , (unsigned char)SBTVD_STATE04_REGION_NUM_MAX},
	{ sbtvd_state_05_str, sbtvd_state05_region_strpool , (unsigned char)SBTVD_STATE05_REGION_NUM_MAX},
	{ sbtvd_state_06_str, sbtvd_state06_region_strpool , (unsigned char)SBTVD_STATE06_REGION_NUM_MAX},
	{ sbtvd_state_07_str, sbtvd_state07_region_strpool , (unsigned char)SBTVD_STATE07_REGION_NUM_MAX},
	{ sbtvd_state_08_str, sbtvd_state08_region_strpool , (unsigned char)SBTVD_STATE08_REGION_NUM_MAX},
	{ sbtvd_state_09_str, sbtvd_state09_region_strpool , (unsigned char)SBTVD_STATE09_REGION_NUM_MAX},
	{ sbtvd_state_10_str, sbtvd_state10_region_strpool , (unsigned char)SBTVD_STATE10_REGION_NUM_MAX},
	{ sbtvd_state_11_str, sbtvd_state11_region_strpool , (unsigned char)SBTVD_STATE11_REGION_NUM_MAX},
	{ sbtvd_state_12_str, sbtvd_state12_region_strpool , (unsigned char)SBTVD_STATE12_REGION_NUM_MAX},
	{ sbtvd_state_13_str, sbtvd_state13_region_strpool , (unsigned char)SBTVD_STATE13_REGION_NUM_MAX},
	{ sbtvd_state_14_str, sbtvd_state14_region_strpool , (unsigned char)SBTVD_STATE14_REGION_NUM_MAX},
	{ sbtvd_state_15_str, sbtvd_state15_region_strpool , (unsigned char)SBTVD_STATE15_REGION_NUM_MAX},
	{ sbtvd_state_16_str, sbtvd_state16_region_strpool , (unsigned char)SBTVD_STATE16_REGION_NUM_MAX},
	{ sbtvd_state_17_str, sbtvd_state17_region_strpool , (unsigned char)SBTVD_STATE17_REGION_NUM_MAX},
	{ sbtvd_state_18_str, sbtvd_state18_region_strpool , (unsigned char)SBTVD_STATE18_REGION_NUM_MAX},
	{ sbtvd_state_19_str, sbtvd_state19_region_strpool , (unsigned char)SBTVD_STATE19_REGION_NUM_MAX},
	{ sbtvd_state_20_str, sbtvd_state20_region_strpool , (unsigned char)SBTVD_STATE20_REGION_NUM_MAX},
	{ sbtvd_state_21_str, sbtvd_state21_region_strpool , (unsigned char)SBTVD_STATE21_REGION_NUM_MAX},
	{ sbtvd_state_22_str, sbtvd_state22_region_strpool , (unsigned char)SBTVD_STATE22_REGION_NUM_MAX},
	{ sbtvd_state_23_str, sbtvd_state23_region_strpool , (unsigned char)SBTVD_STATE23_REGION_NUM_MAX},
	{ sbtvd_state_24_str, sbtvd_state24_region_strpool , (unsigned char)SBTVD_STATE24_REGION_NUM_MAX},
	{ sbtvd_state_25_str, sbtvd_state25_region_strpool , (unsigned char)SBTVD_STATE25_REGION_NUM_MAX},
	{ sbtvd_state_26_str, sbtvd_state26_region_strpool , (unsigned char)SBTVD_STATE26_REGION_NUM_MAX},
	{ sbtvd_state_27_str, sbtvd_state27_region_strpool , (unsigned char)SBTVD_STATE27_REGION_NUM_MAX}
};


const unsigned short SBTVD_TimeZone_RegionID_Pool[SBTVD_STATE_NUM_MAX] = 
{
	4, 6, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 5, 5, 3, 3
};

/* State ID #13: Pernambuco */
const unsigned short SBTVD_TimeZone_State13_RegionID_Pool[SBTVD_STATE13_REGION_NUM_MAX] = 
{
//	         4           8       11       14       17       		// Region ID : 2
	1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1
};

unsigned char SBTVD_GetRegionTableID(unsigned char StateID, unsigned short MicroRegionID)
{	
	unsigned char ucRegionID = 0;

	/* Sanity check of State ID */
	if(StateID > 0 && StateID <= SBTVD_STATE_NUM_MAX) 
	{
		/* In advance to check Micro-Region ID, Find Region ID by State ID. */
		ucRegionID = SBTVD_TimeZone_RegionID_Pool[StateID - 1];	

		/* Check if current state has multiple time zone.(Pernambuco) */	
		if(StateID == 13)	
		{
			/* Sanity check of Micro-Region ID */
			if(MicroRegionID && MicroRegionID <= SBTVD_STATE13_REGION_NUM_MAX)
			{
				ucRegionID = SBTVD_TimeZone_State13_RegionID_Pool[MicroRegionID - 1];
			}
			else
			{	
				ucRegionID = 1;
			}
		}
	}

	return ucRegionID;
	
}

unsigned char SBTVD_GetChannelMaxRegionNum(unsigned char StateID)
{	
	return sbtvd_region_num_values[StateID];
}

SBTVD_AREACODE_STRPOOL * SBTVD_GetAreaStrPoolPtr(void)
{
	return &sbtvd_area_strpool;
}

unsigned char SBTVD_GetChannelStateID(unsigned short Areacode)
{	
	return (Areacode & 0xf80) >> 7;
}

unsigned short SBTVD_GetChannelMicroRegionID(unsigned short Areacode)
{
	return (Areacode & 0x7f);
}

/* End of "ISDBT_Region.c" */

