/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

#ifndef _MPEG1_H_
#define _MPEG1_H_

/*****************************************************************************
*                                                                            *
*  Item : All #define(s) for mpeg_pid.h and user(s)                          *
*                                                                            *
*****************************************************************************/


/*********************************************************************
* BOUQUET_NAME_MAX defines maximum size of the bouquet name field    *
*********************************************************************/
#define  BOUQUET_NAME_MAX  (256 - 1)

/*********************************************************************
* CA_ID_MAX defines the number of CA_identifiers in descriptor       *
*********************************************************************/
#define CA_ID_MAX    (256 / 2)

/*********************************************************************
* CA_PRIVATE_DATA_MAX defines the number of U8s of data           *
*********************************************************************/
#define CA_PRIVATE_DATA_MAX   252

/*********************************************************************
* COMPONENT_TEXT_MAX defines the length of the text_decription in    *
*                        COMPONENT_DESCR                             *
*********************************************************************/
#define COMPONENT_TEXT_MAX   (256 - 1)

/*********************************************************************
* CONTENT_DATA_MAX defines the length of the content data in         *
*                        CONTENT_DESCR                               *
*********************************************************************/
#define CONTENT_DATA_MAX   16

/*********************************************************************
*                                                                    *
* COPYRIGHT_ADD_INFO_MAX defines the number of U8s of addition    *
*                        information U8s in COPYRIGHT_DESCR       *
*                                                                    *
*********************************************************************/
#define COPYRIGHT_ADD_INFO_MAX   252

/*********************************************************************
* COUNTRY_CODE_MAX defines the number of country_code in             *
*                        LOCAL_TIME_OFFSET                           *
*********************************************************************/
#define COUNTRY_CODE_MAX   3


/*********************************************************************
* DELIVERY_SYSTEM_TBD_MAX defines maximum size of TBD data           *
*********************************************************************/
#define  DELIVERY_SYSTEM_TBD_MAX 11

/*********************************************************************
* EVENT_LIST_MAX defines maximum size of the event name field        *
*********************************************************************/
#define  EVENT_LIST_MAX  ((256 - 1) / 6)

/*********************************************************************
* EXT_EVT_DESCR_MAX defines maximum size of the extended event       *
* descriptor field                                                   *
*********************************************************************/
#define  EXT_EVT_DESCR_MAX  (256 - 1)

/*********************************************************************
* EXT_EVT_ITEM_MAX defines maximum size of the extended event item   *
* field                                                              *
*********************************************************************/
#define  EXT_EVT_ITEM_MAX  (256 - 1)

/*********************************************************************
* EXT_EVT_TEXT_MAX defines maximum size of the extended event text   *
* field                                                              *
*********************************************************************/
#define  EXT_EVT_TEXT_MAX  (256 - 1)

/*********************************************************************
* ISO_LANG_CODE_LENGTH defines the length of the ISO language codes. *
*********************************************************************/
#define  ISO_LANG_CODE_LENGTH   3

/*********************************************************************
* ISO_COUNTRY_CODE_LENGTH defines the length of the ISO country codes*
*********************************************************************/
#define  ISO_COUNTRY_CODE_LENGTH   3

/*********************************************************************
* ISO_LANG_MAX defines maximum number of language codes/descriptor   *
*              (85 * 3 = 255)                                        *
*********************************************************************/
#define  ISO_LANG_MAX   85 

/*********************************************************************
* LINKAGE_DATA_MAX defines maximum size of the linkage private data  *
*********************************************************************/
#define  LINKAGE_DATA_MAX    (256 - 7)

/*********************************************************************
* MOSAIC_CELLS_MAX defines the number of elementary cells in the     *
* private descriptor MOSAIC_DESCR                                    *
*********************************************************************/
#define MOSAIC_CELLS_MAX    (256 - 1)

/*********************************************************************
* NETWORK_LIST_MAX defines the maximum number of networks in the     *
*                  network_list_descriptor                           *
*********************************************************************/
#define NETWORK_LIST_MAX   (256 / 2)

/*********************************************************************
* NETWORK_NAME_MAX defines maximum size of the network name field    *
*********************************************************************/
#define  NETWORK_NAME_MAX     (256 - 1)

/*********************************************************************
*                                                                    *
* NULL_PID defines a not valid PID for the software, although    *
*          this should normally be a 13 bit value, for the software   *
*          all PID fileds are 16 bits, hence 0xffff and not 0x1fff   *
* NULL_SERVICE_ID defines an invalid service ID (DVB defined)        *                                                                   *
*********************************************************************/
#define  NULL_PID_F        0xffff
#define  NULL_SERVICE_ID   0xffff
#define  REAL_NULL_PID     0x1fff  /*  13 bit null pid */

/*********************************************************************
* NVOD_DESCR_MAX defines maximum number of NVOD descriptors          *
*********************************************************************/
#define  NVOD_DESCR_MAX ((256 - 2) / 8)

/*********************************************************************
* PARENT_RATING_DATA_MAX defines the maximum size of the parental    *
* rating data                                                        *
*********************************************************************/
#define  PARENT_RATING_DATA_MAX   ((256 - 1) / 4)

/*********************************************************************
* PAT_ENTRY_MAX defines the number of services in a single PAT block *
*********************************************************************/
/* to difine the number of channel on a TP  */
#define PAT_ENTRY_MAX         253  /* org. : 60 */
#define PAT_ENTRY_MAX_NEW     253  /* ((1024 - 9) / 4) */

/*********************************************************************
*                                                                    *
* REGISTRATION_INFO_MAX defines the number of U8s of data         *
*                                                                    *
*********************************************************************/
#define REGSTRATION_INFO_MAX  252

/*********************************************************************
* S_EVENT_NAME_MAX defines maximum size of the short event name field*
*********************************************************************/
#define  S_EVENT_NAME_MAX  97 //97 = 96(isdb-t spec) + terminating null //64

/*********************************************************************
* S_EVENT_TEXT_MAX defines maximum size of the short event text field*
*********************************************************************/
#define  S_EVENT_TEXT_MAX  (256 - 1)

/*********************************************************************
* SERVICE_LIST_MAX defines the number of services in the             *
*                  service_list_descriptor                           *
*********************************************************************/
#define SERVICE_LIST_MAX   (255 / BYTE_SIZE_OF_SERVICE_LIST_STRUCT)

/*********************************************************************
* SVC_NAME_MAX & SVC_PROVIDER_MAX define the maximum length of the   *
*          service_name & service_provider in the service_descriptor *
*********************************************************************/
#define SVC_NAME_MAX       128
#define SVC_PROVIDER_MAX   (256 - 1 - SVC_NAME_MAX)

/*********************************************************************
* TELETEXT_DATA_MAX defines the max data for teletex_descriptor      *
*********************************************************************/
#define TELETEXT_DATA_MAX   ((256 - 1) / 8)

/*********************************************************************
* TERRESTRIAL_TBD_MAX defines maximum size of TBD data               *
*********************************************************************/
#define  TERRESTRIAL_TBD_MAX 11

/*********************************************************************
* PARENTAL_CONTROL_TEXT_MAX defines maximum size of PC message       *
*********************************************************************/
#define  PARENTAL_CONTROL_TEXT_MAX 256 - 7

/*********************************************************************
* AVAIL_AUDIO_LANG_MAX defines maximum number of available languages *
*********************************************************************/
#define  AVAIL_AUDIO_LANG_MAX 256 / 3

/*********************************************************************
* AFFILIATION_ID_MAX defines maximum number of affiliation IDs       *
*********************************************************************/
#define 	AFFILIATION_ID_MAX	255


/*********************************************************************
* COUNTRY_REGION_ID_MAX defines maximum number of country region IDs *
*********************************************************************/
#define	COUNTRY_REGION_ID_MAX		60

/*********************************************************************
 * The following is newly defined code
 *********************************************************************/
#define MAX_AUDIO_LANGUAGES               16
#define MAX_AUDIO_TYPES                    2   /* Censored & Uncensored */
#define AUDIO_TEXT_MAX_PER_ITEM           16

#define MAX_AUDIO_COMPONENT_COUNT         99

#define LANG_TEXT_MAX       32  /* length of language text in PMT for audio stream description */
#define CAPMTSIZE           1024
#define MAX_CAS_SYSTEM_ID   10

#define SATELLITE_NAME_LEN_MAX	12
#define NETWORK_NAME_LEN_MAX 	12
#define BOUQUET_NAME_LEN_MAX	12
#define SERVICE_NAME_LEN_MAX	12

#define SUBTITLE_OFFSET_MAX		20

#define LAST_NUMBER_OF_SATELLITE      40


#define MAX_FREQ_TBL_NUM		58	/* Japan:13~62 , Brazil:14~69 */

#define MAX_TS_NAME_LEN 21

/*****************************************************************************
 *    Item : All enums for mpeg_pid.h and user(s)                            *
 *****************************************************************************/
typedef enum
{                            /* service_type  description                 */
	SERVICE_RES_00,            /* 0x00          DVB_reserved                */
	SERVICE_DIG_TV,            /* 0x01          digital television service  */
	SERVICE_DIG_RADIO,          /* 0x02          digital radio sound service */
	SERVICE_DATA_1SEG	 = 0xc0	/* 0xC0		Data_service (for ISDB-T 1-SEG) */
} SERVICE_TYPE;

typedef enum
{
	AUDIO_STEREO,
	AUDIO_MONO_LEFT,
	AUDIO_MONO_RIGHT,
	AUDIO_MODE_LAST
} AUDIO_MODE;


/*********************************************************************
*  Item : AUDIO_STREAM_ALIGMENT                                      *
* Usage : Used in DATA_ST_ALIGN_DESCR                                *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*********************************************************************/
typedef enum
{
   AS_ALIGN_RES_0,   /* 0x00      reserved     */
   AS_AUDIO_FRAME,   /* 0x01      Audio frame  */
   AS_ALIGN_RES_2    /* 0x02-0xff reserved     */
   
}  AUDIO_STREAM_ALIGNMENT;

/**********************************************************************
 *  Item : CHROMA_FORMAT                                              *
 * Usage : Used in VIDEO_STREAM_STRUCT                                *
 *  NOTE : IDENTICAL TO MPEG VALUES                                   *
 **********************************************************************/
typedef enum
{
   CF_RESERVED,      /* 0b00  reserved  */
   CF_4_2_0,         /* 0b01  4:2:0     */
   CF_4_2_2,         /* 0b10  4:2:2     */
   CF_4_4_4          /* 0b11  4:4:4     */

}  CHROMA_FORMAT;

/*********************************************************************
*  Item : CURR_NEXT                                                  *
* Usage : Used in almost all PSI/DVBSI tables to destinguish between *
* the current and next sections.                                     *
*********************************************************************/
typedef enum
{
   NEXT,    /* 0x00 */
   CURRENT  /* 0x01 */
}  CURR_NEXT;

/*********************************************************************
*  Item : DESCRIPTOR_IDS                                             *
* Usage : Used to identify the various descriptors                   *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*********************************************************************/
typedef enum
{                                      /* value TS PS Descriptor                                     */
   RESERVED_0,                         /* 0x00  -  -  Reserved                                       */
   RESERVED_1,                         /* 0x01  -  -  Reserved                                       */
   VIDEO_STREAM_DESCR_ID,              /* 0x02  X  X  video_stream_descriptor                        */
   AUDIO_STREAM_DESCR_ID,              /* 0x03  X  X  audio_stream_descriptor                        */
   HIERARCY_DESCR_ID,                  /* 0x04  X  X  hierarchy_descriptor                           */
   REGISTRATION_DESCR_ID,              /* 0x05  X  X  registration_descriptor                        */
   DATA_S_ALIGN_DESCR_ID,              /* 0x06  X  X  data_stream_alignment_descriptor               */
   TARGET_BG_GRID_DESCR_ID,            /* 0x07  X  X  target_background_grid_descriptor              */
   VIDEO_WIND_DESCR_ID,                /* 0x08  X  X  video_window_descriptor                        */
   CA_DESCR_ID,                        /* 0x09  X  X  CA_descriptor                                  */
   LANG_DESCR_ID,                      /* 0x0A  X  X  ISO_639_language_descriptor                    */
   SYSTEM_CLOCK_DESCR_ID,              /* 0x0B  X  X  system_clock_descriptor                        */
   MUX_BUF_UTIL_DESCR_ID,              /* 0x0C  X  X  multiplex_buffer_utilization_descriptor        */
   COPYRIGHT_DESCR_ID,                 /* 0x0D  X  X  copyright_descriptor                           */
   MAX_BITRATE_DESCR_ID,               /* 0x0E  X  -  maximum bitrate descriptor                     */
   PRIVATE_DATA_INDICATOR_DESCR_ID,    /* 0x0F  X  X  private data indicator descriptor              */
   SMOOTHING_BUFFER_DESCR_ID,          /* 0X10  X     smoothing buffer descriptor      NO SUPPORT NOW*/
   STD_DESCR_ID,                       /* 0X11  X     STD_descriptor                   NO SUPPORT NOW*/
 
/* skott Add 2005.9.13  start */
   IOD_DESCR_ID 	= 0x1D,            /* 0X1D  X     IOD descriptor    for T-DMB             PMT	 */
   SL_DESCR_ID,						   /* 0X1E  X     SL descriptor     for T-DMB             PMT	 */
/* skott Add 2005.9.13  end */
                                       /* value  Descriptor                          NIT BAT SDT EIT */
   NETWORK_NAME_DESCR_ID     = 0x40,   /* 0x40 network_name_descriptor                *   -   -   -  */
   SERVICE_LIST_DESCR_ID,              /* 0x41 service_list_descriptor                *   *   -   -  */
   STUFFING_DESCR_ID,                  /* 0x42 stuffing_descriptor                    *   *   *   *  */
   SATELLITE_DS_DESCR_ID,              /* 0x43 satellite_delivery_system_descriptor   *   -   -   -  */
   CABLE_DS_DESCR_ID,                  /* 0x44 cable_delivery_system_descriptor       *   -   -   -  */
   RESERVED_1_ID,                      /* 0x45 */
   RESERVED_2_ID,                      /* 0x46 */
   BOUQUET_NAME_DESCR_ID,              /* 0x47 bouquet_name_descriptor                -   *   *   -  */
   SERVICE_DESCR_ID,                   /* 0x48 service_descriptor                     -   -   *   -  */
   COUNTRY_AVAIL_DESCR_ID,             /* 0x49 country_availability_descriptor        -   *   *   -  */
   LINKAGE_DESCR_ID,                   /* 0x4a linkage_descriptor                     *   *   *   *  */
   NVOD_REF_DESCR_ID,                  /* 0x4b NVOD_reference_descriptor              -   -   *   *  */
   TIME_S_SVC_DESCR_ID,                /* 0x4c time_shifted_service_descriptor        -   -   *   -  */
   SHRT_EVT_DESCR_ID,                  /* 0x4d short_event_descriptor                 -   -   -   *  */
   EXTN_EVT_DESCR_ID,                  /* 0x4e extended_event_descriptor              -   -   -   *  */
   TIME_S_EVT_DESCR_ID,                /* 0x4f time_shifted_event_descriptor          -   -   -   *  */
   COMPONENT_DESCR_ID,                 /* 0x50 audio_component_descriptor             -   -   -   *  */
   MOSAIC_DESCR_ID,                    /* 0x51 mosaic_descriptor                      -   -   -   -  */
   STREAM_IDENT_DESCR_ID,              /* 0x52 stream_identifier_descriptor           -   -   -   -  */
   CA_IDENTIFIER_DESCR_ID,             /* 0x53 CA_identifier_descriptor               -   *   *   *  */
   CONTENT_DESCR_ID,                   /* 0x54 content_descriptor                     -   -   -   *  */
   PARENT_DESCR_ID,                    /* 0x55 parental_rating_descriptor             -   -   -   *  */
   TELETEXT_DESCR_ID,                  /* 0x56 teletext_descriptor                    +   +   +   +  */
   TELE_DESCR_ID,                      /* 0x57 telephone_descriptor                   -   -   *   *  */
   LOCAL_TIME_OFFSET_DESCR_ID,         /* 0x58 local_time_offset_descriptor           -   -   -   -  */
   SUBTITLING_DESCR_ID       ,         /* 0x59 subtitling_desciptor                   -   -   -   -  */
   TERRESTRIAL_DS_DESCR_ID,            /* 0x5A terrestrial_delivery_system_descriptor *   -   -   -  */
   MULTI_NETWORK_NAME_DESCR_ID,        /* 0x5B multilingual_network_name_descriptor   *   -   -   -  */
   MULTI_BOUQUET_NAME_DESCR_ID,        /* 0x5C multilingual_bouquet_name_descriptor   -   *   -   -  */
   MULTI_SERVICE_NAME_DESCR_ID,        /* 0x5D multilingual_service_name_descriptor   -   -   *   -  */
   MULTI_COMPONENT_NAME_DESCR_ID,      /* 0x5E multilingual_network_name_descriptor   -   -   -   *  */

   PRIVATE_DATA_SPECIFIER_ID = 0x5F,   /* 0x5F private_data2_specifier_id - private   -   -   -   *  */
   FREQ_LIST_DESCR_ID = 0x62,          /* 0x62 frequency_list_descriptor              *   -   -   -  */
   CELL_FREQ_LINK_DESCR_ID = 0x6D,     /* 0x6D cell_frequency_link_descriptor         *   -   -   -  */
   AC3_DESCR_ID = 0x6A,	               /* 0x6A AC-3 descriptor (only in PMT)                         */
   EHN_AC3_DESCR_ID = 0x7A,            /* 0x7A Enhanced AC-3 descriptor (only in PMT)                */
   LOGICAL_CHANNEL_DECSRIPTOR_ID = 0x83,  /* 0x83 logical_channel_descriptor only in NIT at UK DTT   */
   /*
   * HD simulcast logical channel number is defined at CGV Specifications
   * DTCP_DESCR_ID is also 0x88, it is conflicted with DTCP_DESCR_ID. therefore DTCP_DESCR_ID changed to 0x888(Invalid value).
   */
   HD_LOGICAL_CHANNEL_DESCR_ID = 0x88,  /* 0x88 HD simulcast logical channel number */
   DTCP_DESCR_ID = 0x888,               /* 0x88 Digital Transmission Content Protection - PMT */
   

   DIGITALCOPY_CONTROL_ID = 0xc1,
   AUDIO_COMPONENT_DESCR_ID = 0xC4,		/* 0xC4 Audio Component Descriptor (ISDB-T. located in EIT) */
   USER_DEFINED_SVC_NAME_ID = 0xC5,
   CA_CONTRACT_INFO_DESCR_ID = 0xcb,    /* 0xCB CA contract information descriptor (used in ISDB-T)  */
   TS_INFO_DESCR_ID = 0xcd,             /* 0xCD TS information descriptor (used in ISDB-T)           */
   EXT_BROADCASTER_DESCR_ID = 0xce,
   LOGO_TRANSMISSION_DESCR_ID = 0xCF,
   EVT_GROUP_DESCR_ID = 0xd6,           /* 0xD6 event_group_discriptor (used in ISDB-T)              */
   CONTENT_AVAILABILITY_ID = 0xde,
   ISDBT_TERRESTRIAL_DS_DESCR_ID = 0xFA,
   PARTIAL_RECEPTION_DESCR_ID = 0xFB,
   EMERGENCY_INFORMATION_DESCR_ID = 0xFC,
   DATA_COMPONENT_DESCR_ID = 0xFD,
   SYSTEM_MANAGEMENT_DESCR_ID = 0xFE,

   RESERVED_FF                         /* 0xFF  -  -  Reserved                                     */
}  DESCRIPTOR_IDS;                     /* + = in PMT ONLY                                            */

/*********************************************************************
*  Item : COMP_STREAM_CONTENT                                        *
* Usage : Used in the component_descriptor                           *
*********************************************************************/
typedef enum
{
   COMP_CONTENTS_RSVD,  /* 0x00  stream_content reserved */
   COMP_VIDEO,          /* 0x01  stream_content video    */
   COMP_AUDIO,          /* 0x02  stream_content audio    */
   COMP_TELETEXT        /* 0x03  stream_content teletext */
}  COMP_STREAM_TYPE;

/*********************************************************************
*  Item : CONTENT_GENRE                                              *
* Usage : Used in the content_descriptor                             *
*********************************************************************/
typedef enum
{
   CONT_UNDEFINED_00,   /* 0x00  undefined content                   */
   CONT_MOVIE,          /* 0x01  movies                              */
   CONT_NEWS,           /* 0x02  news/current affairs                */
   CONT_SHOW,           /* 0x03  show/game show                      */
   CONT_SPORT,          /* 0x04  sports                              */
   CONT_CHILDREN,       /* 0x05  children's/youth programmes         */
   CONT_MUSIC,          /* 0x06  music/ballet/dance                  */
   CONT_ART,            /* 0x07  arts/culture (without music)        */
   CONT_SOCIAL,         /* 0x08  social/political issues/economics   */
   CONT_EDUCATION,      /* 0x09  education/science/factual topics    */
   CONT_LEISURE,        /* 0x0A  leisure hobbies                     */
   CONT_UNDEFINED_0B,   /* 0x0B  undefined content                   */
   CONT_UNDEFINED_0C,   /* 0x0C  undefined content                   */
   CONT_UNDEFINED_0D,   /* 0x0D  undefined content                   */
   CONT_UNDEFINED_0E,   /* 0x0E  undefined content                   */
   CONT_USER_DEFINED    /* 0x00  user defined                        */
}  CONTENT_GENRE;

/**********************************************************************
 *  Item : ES_TYPE                                                    *
 * Usage : Used in the elementary stream descriptors                  *
 *  NOTE : IDENTICAL TO MPEG VALUES                                   *
 **********************************************************************/
typedef enum
{
   ES_RESERVED,                  /* 0x00  ITU-T | ISO/IEC Reserved                                   */
   ES_MPEG1_VIDEO,               /* 0x01  ISO/IEC 11172 Video                                        */
   ES_MPEG2_VIDEO,               /* 0x02  ITU-T Rec. H.222.0 | ISO/IEC 13818 Video                   */
   ES_MPEG1_AUDIO,               /* 0x03  ISO/IEC 11172 Audio                                        */
   ES_MPEG2_AUDIO,               /* 0x04  ITU-T Rec. H.222.0 | ISO/IEC 13818 Audio                   */
   ES_PRIV_SECTIONS,             /* 0x05  ITU-T Rec. H.222.0 | ISO/IEC 13818 private_sections        */
   ES_PRIV_PES,                  /* 0x06  ITU-T Rec. H.222.0 | ISO/IEC 13818 PES packed private data */
   ES_MHEG,                      /* 0x07  ISO/IEC 13522 MHEG                                         */
   ES_DSM_CC,                    /* 0x08  ITU-T Rec. H.222.0 | ISO/IEC 13818 DSM CC                  */
   ES_AUX,                       /* 0x09  ITU-T Rec. H.222.0 | ISO/IEC 13818-1/11172-1 auxiliary     */
   ES_TC_DSM_CC = 0x0D,          /* 0x0D  ISO/IEC 13818-6 type D                                     */
   ES_SDMB_AAC = 0x0F,           /* 0x0F  ISO/IEC 13818-7 (MPEG-2 AAC)  | S-DMB                      */
   ES_SDTBD_AAC = 0x11,				 /*0x11 ISO/IEC 14496-3(MPEG-4 AAC)*/
   ES_TDMB_VIDEO = 0x12,         /* 0x12  ITU-T Rec. H.264.0 | ISO/IEC 14496-10 AVC | T-DMB          */
   ES_TDMB_BSAC = 0x13,          /* 0x13  ISO/IEC 14496-3 (BSAC) | T-DMB                             */
   ES_SDMB_VIDEO = 0x1B,         /* 0x1B  ITU-T Rec. H.264.0 | ISO/IEC 14496-10 AVC | S-DMB          */
   ES_AC3_AUDIO1 = 0x80,		 /* 0x80  ATSC AC3 */
   ES_AC3_AUDIO2 = 0x81			 /* 0x81  ATSC AC3 */
}  ES_TYPE;

/**********************************************************************
 *  Item : FEC_INNERBIT                                               *
 * Usage : Used in delievery descriptors                              *
 *  NOTE : IDENTICAL TO MPEG VALUES                                   *
 **********************************************************************/
typedef enum
{                       /* FEC_innerbit 3210  description     */
   FEC_IN_RES_0,        /*              0000  not defined     */
   FEC_IN_1_TO_2,       /*              0001  1/2  Conv. Code */
   FEC_IN_2_TO_3,       /*              0010  2/3  Conv. Code */
   FEC_IN_3_TO_4,       /*              0011  3/4  Conv. Code */
   FEC_IN_5_TO_6,       /*              0100  5/6  Conv. Code */
   FEC_IN_7_TO_8,       /*              0101  7/8  Conv. Code */
   FEC_IN_RES_6,        /*              0110  reserved        */
   FEC_IN_RES_7,        /*              0111  reserved        */
   FEC_IN_RES_8,        /*              1000  reserved        */
   FEC_IN_RES_9,        /*              1001  reserved        */
   FEC_IN_RES_A,        /*              1010  reserved        */
   FEC_IN_RES_B,        /*              1011  reserved        */
   FEC_IN_RES_C,        /*              1100  reserved        */
   FEC_IN_RES_D,        /*              1101  reserved        */
   FEC_IN_RES_E,        /*              1110  reserved        */
   FEC_IN_1_TO_1        /*              1111  1/1  Conv. Code */
}  FEC_INNERBIT;

/**********************************************************************
 *  Item : FEC_OUTERBIT                                               *
 * Usage : Used in delievery descriptors                              *
 *  NOTE : IDENTICAL TO MPEG VALUES                                   *
 **********************************************************************/
typedef enum
{                       /*  3210    description          */
   FEC_OUT_RES_0,       /*  0000    not defined          */
   FEC_OUT_NONE,        /*  0001    no outer FEC coding  */
   FEC_OUT_RS_204_188,  /*  0010    RS(204/188)          */
   FEC_OUT_RES_3        /*  0011 to 1111    reserved     */
}  FEC_OUTERBIT;


/**********************************************************************
 *  Item : FRAME_RATE_CODE                                            *
 * Usage : Used in VIDEO_STREAM_STRUCT                                *
 *  NOTE : IDENTICAL TO MPEG VALUES                                   *
 **********************************************************************/
typedef enum
{                    /* code    value                 */
   FRC_FORBIDDEN,    /* 0b0000  forbidden             */
   FRC_24_DIV,       /* 0b0001  24000 ?1001 (23,976) */
   FRC_24,           /* 0b0010  24                    */
   FRC_25,           /* 0b0011  25                    */
   FRC_30_DIV,       /* 0b0100  30000 ?1001 (29,97)  */
   FRC_30,           /* 0b0100  30                    */
   FRC_50,           /* 0b0110  50                    */
   FRC_60_DIV,       /* 0b0111  60000 ?1001 (59,94)  */
   FRC_60            /* 0b1000  60                    */
}  FRAME_RATE_CODES;

/*********************************************************************
*                                                                    *
*  Item : HIERARCY_TYPE                                              *
*                                                                    *
* Usage : Used in HIERARCY_DESCRIPTOR                                *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{
                  /* value description                                              */
   HIER_RES_0,    /* 0     reserved                                                 */
   HIER_SPATIAL,  /* 1     ITU-T Rec. H.262 | ISO/IEC 13818-2 Spatial Scalability   */
   HIER_SNR,      /* 2     ITU-T Rec. H.262 | ISO/IEC 13818-2 SNR Scalability       */
   HIER_TEMPORAL, /* 3     ITU-T Rec. H.262 | ISO/IEC 13818-2 Temporal Scalability  */
   HIER_DATA,     /* 4     ITU-T Rec. H.262 | ISO/IEC 13818-2 Data partitioning     */
   HIER_EXTEN,    /* 5     ISO/IEC 13818-3 Extension bitstream                      */
   HIER_RES_6,    /* 6     reserved                                                 */
   HIER_RES_7,    /* 7     reserved                                                 */
   HIER_RES_8,    /* 8     reserved                                                 */
   HIER_RES_9,    /* 9     reserved                                                 */
   HIER_RES_10,   /* 10    reserved                                                 */
   HIER_RES_11,   /* 11    reserved                                                 */
   HIER_RES_12,   /* 12    reserved                                                 */
   HIER_RES_13,   /* 13    reserved                                                 */
   HIER_RES_14,   /* 14    reserved                                                 */
   HIER_BASE      /* 15    Base layer                                               */
}  HIERARCY_TYPE;

/*********************************************************************
*                                                                    *
*  Item : LANG_AUDIO_TYPE                                            *
*                                                                    *
* Usage : Used in ISO_LANG_DESCR                                     *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{                          /* value      meaning                    */
   LANG_AUDIO_RES_00,      /* 0x00       reserved                   */
   LANG_CLEAN,             /* 0x01       clean effects              */
   LANG_H_IMPARED,         /* 0x02       hearing impaired           */
   LANG_V_IMPARED,         /* 0x03       visual impaired commentary */
   LANG_AUDIO_RES_04      /* 0x04-0xFF  reserved                   */
}  LANG_AUDIO_TYPE;

/*********************************************************************
*                                                                    *
*  Item : LEVEL_ID                                                   *
*                                                                    *
* Usage : Used in VIDEO_STREAM_STRUCT                                *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{
   LVL_RES_0,     /* 0b0000    (reserved)  */
   LVL_RES_1,     /* 0b0001    (reserved)  */
   LVL_RES_2,     /* 0b0010    (reserved)  */
   LVL_RES_3,     /* 0b0011    (reserved)  */
   LVL_HIGH,      /* 0b0100    High        */
   LVL_RES_5,     /* 0b0101    (reserved)  */
   LVL_HIGH_1440, /* 0b0110    High 1440   */
   LVL_RES_7,     /* 0b0111    (reserved)  */
   LVL_MAIN,      /* 0b1000    Main        */
   LVL_RES_9,     /* 0b1001    (reserved)  */
   LVL_LOW,       /* 0b1010    Low         */
   LVL_RES_B,     /* 0b1011    (reserved)  */
   LVL_RES_C,     /* 0b1100    (reserved)  */
   LVL_RES_D,     /* 0b1101    (reserved)  */
   LVL_RES_E,     /* 0b1110    (reserved)  */
   LVL_RES_F      /* 0b1111    (reserved)  */
}  LEVEL_ID;

/*********************************************************************
*                                                                    *
*  Item : LINKAGE_TYPE                                               *
*                                                                    *
* Usage : Used in linkage_descriptor                                 *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{                                      /* type    description                              */
   LINKAGE_RES_00,                     /* 0x00    DVB_reserved                             */
   LINKAGE_INFO_SVC,                   /* 0x01    information service                      */
   LINKAGE_EPG,                        /* 0x02    Electronic Programme Guide (EPG) service */
   LINKAGE_RES_03,                     /* 0x03 - 0x7F     DVB_reserved                     */
   LINKAGE_IRDETO_DL = 0x80,           /* Irdeto Download */
   LINKAGE_IRDETO_MUX_VERIF
                                       /* 0x80 - 0xFE     user defined                     */
                                       /* 0xFF    DVB_reserved                             */
}  LINKAGE_TYPE;

/*********************************************************************
*                                                                    *
*  Item : MODULATION_CAB                                             *
*                                                                    *
* Usage : Used in cable_delivery_descriptors                         *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{                       /*        description        */
   MOD_CAB_RES_00,      /* 0x00    not defined       */
   MOD_CAB_16_QAM,      /* 0x01    16 QAM            */
   MOD_CAB_32_QAM,      /* 0x02    32 QAM            */
   MOD_CAB_64_QAM,      /* 0x03    64 QAM            */
   MOD_CAB_128_QAM,     /* 0x04    128 QAM           */
   MOD_CAB_256_QAM,     /* 0x05    256 QAM           */
   MOD_CAB_RES_06       /* 0x06  -  0xFF   reserved  */
}  MODULATION_CAB;

/*********************************************************************
*                                                                    *
*  Item : MODULATION_SAT                                             *
*                                                                    *
* Usage : Used in satellite_delivery_descriptors                     *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{                     /* modulationbit   4 3210  description         */
   MOD_SAT_RES_00,    /*                 0 0000  not defined         */
   MOD_SAT_QPSK,      /*                 0 0001  QPSK                */
   MOD_SAT_RES_2      /*                 0 0010 - 1 1111   reserved  */
}  MODULATION_SAT;

/*****************************************************************************
*
*  Item : MPEG_TABLE_IDS
*
* Usage : Used to identify the various MPEG tables
*
*  NOTE : IDENTICAL TO MPEG VALUES
*
*****************************************************************************/
typedef enum
{
   PAT_ID,                    /* 0x00   program_association_section                             */
   CAT_ID,                    /* 0x01   conditional access_section                              */
   PMT_ID,                    /* 0x02   program_map_section                                     */
   
/* skott Add 2005.9.14  start */
   BIFS_ID			= 0x04,   /* 0x04   BIFS command section                                    */
   ODT_ID			= 0x05,   /* 0x05   Object description section                              */
/* skott Add 2005.9.14  end */

	DSMCC_R1_ID			= 0x3A, /* Multi-protocol capsule                                       */
	DSMCC_DII_MSG_ID	= 0x3B, /* U-N message includeing DII                                   */
	DSMCC_DDB_MSG_ID	= 0x3C, /* Same as in the left column                                   */
	DSMCC_SD_ID			= 0x3D, /* Same as in the left column                                   */
	DSMCC_PD_ID			= 0x3E, /* Same as in the left column                                   */
	DSMCC_R2_ID			= 0x3F, /* Same as in the left column                                   */

   NIT_A_ID          = 0x40,  /* 0x40  network_information_section - actual_network             */
   NIT_O_ID          = 0x41,  /* 0x41  network_information_section - other_network              */
   SDT_A_ID          = 0x42,  /* 0x42  service_description_section - actual_ts                  */
   SDT_O_ID          = 0x46,  /* 0x46  service_description_section - other_ts                   */
   BAT_ID            = 0x4A,  /* 0x4A  bouquet_association_section                              */
   EIT_PF_A_ID       = 0x4E,  /* 0x4E  event_information_section - actual_ts, present/following */
   EIT_PF_O_ID       = 0x4F,  /* 0x4F  event_information_section - other_ts, present/following  */
   EIT_SA_0_ID       = 0x50,  /* 0x50-0x5F   event_information_section - actual_ts, schedule    */
   EIT_SA_1_ID,
   EIT_SA_2_ID,
   EIT_SA_3_ID,
   EIT_SA_4_ID,
   EIT_SA_5_ID,
   EIT_SA_6_ID,
   EIT_SA_7_ID,
   EIT_SA_8_ID,
   EIT_SA_9_ID,
   EIT_SA_A_ID,
   EIT_SA_B_ID,
   EIT_SA_C_ID,
   EIT_SA_D_ID,
   EIT_SA_E_ID,
   EIT_SA_F_ID,
   EIT_SO_0_ID       = 0x60,  /* 0x60-0x6F   event_information_section - other_ts, schedule     */
   EIT_SO_1_ID,
   EIT_SO_2_ID,
   EIT_SO_3_ID,
   EIT_SO_4_ID,
   EIT_SO_5_ID,
   EIT_SO_6_ID,
   EIT_SO_7_ID,
   EIT_SO_8_ID,
   EIT_SO_9_ID,
   EIT_SO_A_ID,
   EIT_SO_B_ID,
   EIT_SO_C_ID,
   EIT_SO_D_ID,
   EIT_SO_E_ID,
   EIT_SO_F_ID,
   TDT_ID            = 0x70,  /* 0x70  time_date_section                */
   RST_ID            = 0x71,  /* 0x71  running_status_section           */
   ST_ID             = 0x72,  /* 0x72  stuffing_table                   */
   TOT_ID            = 0x73,  /* 0x73  time_offset_section              */

   ECM_0_ID					 = 0x82,
   ECM_1_ID					 = 0x83,
   EMM_0_ID					 = 0x84,
   EMM_1_ID					 = 0x85,

   SDTT_ID 		= 0xC3,
   BIT_ID		= 0xC4,
   CDT_ID		= 0xC8

}  MPEG_TABLE_IDS;


/*********************************************************************
*                                                                    *
*  Item : MOSAIC_CELL_LINKAGE                                        *
*                                                                    *
* Usage : Used in MOSAIC_DESCR                                       *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{
   MOS_UNDEFINED,
   MOS_BOUQUET,
   MOS_SERVICE,
   MOS_OTHER_MOSAIC,
   MOS_EVENT
} MOSAIC_CELL_LINKAGE;


/*********************************************************************
*                                                                    *
*  Item : MOSAIC_PRESENTATION                                        *
*                                                                    *
* Usage : Used in MOSAIC_DESCR                                       *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{
   MOS_INFO_UNDEFINED,
   MOS_INFO_VIDEO,
   MOS_INFO_STILL_PICTURE,
   MOS_INFO_GRAPHICS_TEXT
} MOSAIC_PRESENTATION;


/*********************************************************************
*                                                                    *
*  Item : PEL_ASPECT_RATIO                                           *
*                                                                    *
* Usage : Used in TARGET_BCK_GRID_DESCR                              *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{                    /*  code   heigt/width  comment        */
   PEL_AS_UNUSED,    /* 0b0000   undefined   forbidden      */
   PEL_AS_1_0,       /* 0b0001   1,0         square pels    */
   PEL_AS_0_6735,    /* 0b0010   0,6735                     */
   PEL_AS_0_7031,    /* 0b0011   0,7031      16:9  625 line */
   PEL_AS_0_7615,    /* 0b0100   0,7615                     */
   PEL_AS_0_8055,    /* 0b0101   0,8055                     */
   PEL_AS_0_8437,    /* 0b0110   0,8437      16:9  525 line */
   PEL_AS_0_8935,    /* 0b0111   0,8935                     */
   PEL_AS_0_9157,    /* 0b1000   0,9157      702x575 at 4:3 */
   PEL_AS_0_9815,    /* 0b1001   0,9815                     */
   PEL_AS_1_0255,    /* 0b1010   1,0255                     */
   PEL_AS_1_0695,    /* 0b1011   1,0695                     */
   PEL_AS_1_0950,    /* 0b1100   1,0950      711x487 at 4:3 */
   PEL_AS_1_1575,    /* 0b1101   1,1575                     */
   PEL_AS_1_2015,    /* 0b1110   1,2015                     */
   PEL_AS_RES_F      /* 0b1111   undefined   reserved       */
}  PEL_ASPECT_RATIO;

/*********************************************************************
*                                                                    *
*  Item : POLARIZATION                                               *
*                                                                    *
* Usage : Used in delivery descriptors                               *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{                        /* polarisation    description     */
   POL_LIN_HORIZ = 0,    /* 0b00        linear - horizontal */
   POL_LIN_VERT,         /* 0b01        linear - vertical   */
   POL_CIR_LEFT,         /* 0b10        circular - left     */
   POL_CIR_RIGHT         /* 0b11        circular - right    */
   
}  POLARIZATION;

/*********************************************************************
*                                                                    *
*  Item : PROFILE_ID                                                 *
*                                                                    *
* Usage : Used in VIDEO_STREAM_STRUCT                                *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{
   PROF_RES_0,       /* 0b000     (reserved)          */
   PROF_HIGH,        /* 0b001     High                */
   PROF_SPAT_SCAL,   /* 0b010     Spatially Scalable  */
   PROF_SNR_SCAL,    /* 0b011     SNR Scalable        */
   PROF_MAIN,        /* 0b100     Main                */
   PROF_SIMPLE,      /* 0b101     Simple              */
   PROF_RES_6,       /* 0b110     (reserved)          */
   PROF_RES_7        /* 0b111     (reserved)          */
}  PROFILE_ID;

/*********************************************************************
*                                                                    *
*  Item : RUNNING_STATUS                                             *
*                                                                    *
* Usage : Used in SDT, RST                                           *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{
   RS_RES_0,         /*  undefined                                            */
   RS_NOT_RUNNING,   /*  service not running                                  */
   RS_STARTS_SOON,   /*  service starts in a few seconds ( for VCR recording) */
   RS_PAUSING,       /*  service pausing                                      */
   RS_RUNNING,       /*  service running                                      */
   RS_RES_5,         /*  DB_reserved                                          */
   RS_RES_6,         /*  DB_reserved                                          */
   RS_RES_7          /*  DB_reserved                                          */
}  RUNNING_STATUS;


/*********************************************************************
*                                                                    *
*  Item : SERVICE_TYPE                                               *
*                                                                    *
* Usage : Used in SERVICE_LIST_DESCR                                 *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{                          /* possible NVOD types */
   NOT_NVOD,               /* normal service or event - no NVOD involved */
   VIRTUAL_NVOD,           /* virtual service or event for NVOD */
   REAL_NVOD               /* real service or event for NVOD */
}  NVOD_TYPE ;

/*********************************************************************
*                                                                    *
*  Item : TELETEXT_TYPE                                              *
*                                                                    *
* Usage : Used in TELETEXT_DESCR                                     *
*                                                                    *
*********************************************************************/
typedef enum
{                       /* teletext type description                 */
   TELETEXT_RES_00,     /* 0x00          reserved for future use     */
   TELETEXT_INIT_PAGE,  /* 0x01          initial teletext page       */
   TELETEXT_SUBTITLE,   /* 0x02          teletext subtitle page      */
   TELETEXT_ADD_INFO,   /* 0x03          additional information page */
   TELETEXT_PROG        /* 0x04          programme schedule page     */
                        /* 0x05 - 0xFF   reserved for future use     */
}  TELETEXT_TYPE;

/*********************************************************************
*                                                                    *
*  Item : COUNTRY_REGION                                             *
*                                                                    *
* Usage : Used in LOCAL_TIME_OFFSET_DESCR                            *
*                                                                    *
*********************************************************************/
typedef enum
{                       /* conuntry_region                           */
   LTO_RES_00,          /* 0x00          NO Time Zone Extension used */
   LTO_TIME_ZONE1,      /* 0x01          most easterly region         */
   LTO_TIME_ZONE2,    
   LTO_TIME_ZONE3,    
   LTO_TIME_ZONE4,    
   LTO_TIME_ZONE5,    
   LTO_TIME_ZONE6,    
   LTO_TIME_ZONE7,    
   LTO_TIME_ZONE8,    
   LTO_TIME_ZONE9,    
   LTO_TIME_ZONE10,   
   LTO_TIME_ZONE11,   
   LTO_TIME_ZONE12,   
   LTO_TIME_ZONE13,   
   LTO_TIME_ZONE14,   
   LTO_TIME_ZONE15,   
   LTO_TIME_ZONE16,   
   LTO_TIME_ZONE17,   
   LTO_TIME_ZONE18,   
   LTO_TIME_ZONE19,   
   LTO_TIME_ZONE20,   
   LTO_TIME_ZONE31,   
   LTO_TIME_ZONE32,   
   LTO_TIME_ZONE33,   
   LTO_TIME_ZONE34,   
   LTO_TIME_ZONE35,   
   LTO_TIME_ZONE36,   
   LTO_TIME_ZONE37,   
   LTO_TIME_ZONE38,   
   LTO_TIME_ZONE39,   
   LTO_TIME_ZONE40,   
   LTO_TIME_ZONE41,   
   LTO_TIME_ZONE42,   
   LTO_TIME_ZONE43,   
   LTO_TIME_ZONE44,   
   LTO_TIME_ZONE45,  
   LTO_TIME_ZONE46,   
   LTO_TIME_ZONE47,   
   LTO_TIME_ZONE48,   
   LTO_TIME_ZONE49,   
   LTO_TIME_ZONE50,   
   LTO_TIME_ZONE51,   
   LTO_TIME_ZONE52,   
   LTO_TIME_ZONE53,   
   LTO_TIME_ZONE54,   
   LTO_TIME_ZONE55,   
   LTO_TIME_ZONE56,   
   LTO_TIME_ZONE57,   
   LTO_TIME_ZONE58,   
   LTO_TIME_ZONE59,   
   LTO_TIME_ZONE60       /* 0x3C          most westerly region         */
                         /* 0x3D - 0x3F   reserved for future use     */
}  COUNTRY_REGION;

/*********************************************************************
*                                                                    *
*  Item : VIDEO_STREAM_ALIGMENT                                      *
*                                                                    *
* Usage : Used in DATA_ST_ALIGN_DESCR                                *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{
   VS_ALIGN_RES_0,         /* 0x00      reserved                     */
   VS_SLICE_PIC_GOP_SEQ,   /* 0x01      Slice, picture, GOP, or SEQ  */
   VS_PIC_GOP_SEQ,         /* 0x02      picture, GOP, or SEQ         */
   VS_GOP_SEQ,             /* 0x03      GOP, or SEQ                  */
   VS_SEQ,                 /* 0x04      SEQ                          */
   VS_ALIGN_RES_5          /* 0x05-FF   reserved                     */
}  VIDEO_STREAM_ALIGNMENT;

/*********************************************************************
*                                                                    *
*  Item : WEST_EAST                                                  *
*                                                                    *
* Usage : Used in delivery system                                    *
*                                                                    *
*  NOTE : IDENTICAL TO MPEG VALUES                                   *
*                                                                    *
*********************************************************************/
typedef enum
{
   WEST,    /* 0 */
   EAST     /* 1 */
}  WEST_EAST;

/*PAGE*/
/*SUBTITLE nested-"struct"s */
/*****************************************************************************
 *  Item : All nested struct(s)                                              *
 *****************************************************************************/

/*****************************************************************************
*
*  Item : PROFILE_LEVEL_STRUCT
*
* Usage : Used in VIDEO_STREAM_STRUCT
*
*         Bit assignment can be split as follows :
*
*                         Pseudo MPEG Definition
*                        ========================
*
*           Syntax                  No. of bits  Mnemonic
*  profile_level_data(){
*     escape_flag                         1       bslbf
*     profile_value                       3       bslbf
*     level_value                         4       bslbf
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned char    fEscape;       /* bit 7        */
   PROFILE_ID		enProfile;     /* bits 6,5,4   */
   LEVEL_ID  		enLevel;       /* bits 3,2,1,0 */
}  PROFILE_LEVEL_STRUCT;



/*****************************************************************************
*
*  Item : TIME_STRUCT
*         DATE_TIME_STRUCT
*
* Usage : Used in EIT_STRUCT and TDT_STRUCT
*
*         Bit assignment can be split as follows :
*
*                         Pseudo MPEG Definition
*                        ========================
*
*           Syntax                  No. of bits  Mnemonic
*  date_time_struct(){
*     mjd                                 16      uimsbf
*     year(){
*        hour_tens                        4       bslbf
*        hour_units                       4       bslbf
*     }
*     month(){
*        minute_tens                      4       bslbf
*        minute_units                     4       bslbf
*     }
*     day(){
*        second_tens                      4       bslbf
*        second_units                     4       bslbf
*     }
*  }
*
*
*        The packed values returned will be unpacked and stored in U8s.
*        Each U8 contains the equivalent year, month and day which has
*        been converted from packed ASCII to binary.
*
*****************************************************************************/
typedef struct
{
   U8        ucHour;
   U8        ucMinute;
   U8        ucSecond;
} TIME_STRUCT;

typedef struct
{
   U16         uiMJD;
   TIME_STRUCT stTime;
}  DATE_TIME_STRUCT;

/*****************************************************************************
 *                                                                           *
 *    Item : All MPEG descriptor structs                                     *
 *                                                                           *
 *****************************************************************************/

/*****************************************************************************
*  Item : AUDIO_STREAM_DESCR
* Usage : Contains the audio_stream_descriptor
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  audio_stream_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     free_format_flag                    1       bslbf
*     ID                                  1       bslbf
*     layer                               2       bslbf
*     reserved                            4       bslbf
*  }
*****************************************************************************/
typedef struct
{
   unsigned char   fFreeFormat;
   unsigned char   fID;
   unsigned char   bLayer;        /* still to be found */
}  AUDIO_STREAM_DESCR;

/*****************************************************************************
*  Item : CA_DESCR
* Usage : Contains the CA_descriptor
*
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  CA_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     CA_system_ID                        16      uimsbf
*     reserved                            3       bslbf
*     CA_PID                              13      uimsbf
*     for ( i=0; i<N; i++) {
*        private_data_byte                8       uimsbf
*     }
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned char    ucCA_Tag;	
   unsigned char    ucDescLen;	
   unsigned short   uiCASystemID;
   unsigned short   uiCA_PID;
   unsigned char    aucPrivateData[CA_PRIVATE_DATA_MAX];   /* 252 */
}CA_DESCR;

typedef struct
{
   unsigned char    CASystemID_Count;
   unsigned short   CASystemID[MAX_CAS_SYSTEM_ID];
}CA_ID_System_t;

/*SUBTITLE SI-Descriptors : COPYRIGHT_DESCR, DATA_S_ALIGN_DESCR */
/*****************************************************************************
*  Item : COPYRIGHT_DESCR
* Usage : Contains the copyright_descriptor
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  copyright_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     copyright_identifier                32      uimsbf
*     for (i = 0; i < N ; i++ ){
*        additional_copyright_info        8       bslbf
*     }
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned int     ulCopyrightID;
   unsigned char     ucInfoLen;
   unsigned char     *paucAddInfo;    /* 252 */
}  COPYRIGHT_DESCR;

/*****************************************************************************
 *
 *  Item : DATA_ST_ALIGN_DESCR
 *
 * Usage : Contains the data_stream_alignment_descriptor
 *
 *
 *                             MPEG Definition
 *                             ===============
 *
 *           Syntax                  No. of bits  Mnemonic
 *  data_stream_alignment_descriptor() {
 *     descriptor_tag                      8       uimsbf
 *     descriptor_length                   8       uimsbf
 *     alignment_type                      8       uimsbf
 *  }
 *
 *****************************************************************************/
typedef struct
{
   union
   {
      AUDIO_STREAM_ALIGNMENT   enAudioAlignment;
      VIDEO_STREAM_ALIGNMENT   enVideoAlignment;
   } unA;
}  DATA_S_ALIGN_DESCR;

/*PAGE*/
/*SUBTITLE SI-Descriptors : HIERARCHY_DESCR, ISO_LANG_DESCR  */

/*****************************************************************************
*
*  Item : HIERARCHY_DESCR
*
* Usage : Contains the hierarcy descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  hierarchy_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     reserved                            4       bslbf
*     hierarchy_type                      4       uimsbf
*     reserved                            2       bslbf
*     hierarchy_layer_index               6       uimsbf
*     reserved                            2       bslbf
*     hierarchy_embedded_layer_index      6       uimsbf
*     reserved                            2       bslbf
*     hierarchy_channel                   6       uimsbf
*  }
*
*****************************************************************************/
typedef struct
{
   HIERARCY_TYPE 		enHierarcyType;
   unsigned char        ucLayerIndex;            /* six bit ONLY */
   unsigned char        ucEmbeddedLayerIndex;    /* six bit ONLY */
   unsigned char        ucChannel;               /* six bit ONLY */
}  HIERARCY_DESCR;


/*****************************************************************************
*
*  Item : ISO_LANG_DESCR
*
* Usage : Contains the ISO_639_language_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  ISO_639_language_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i=0;i<N;i++) {
*        ISO_639_language_code            24      bslbf
*        audio type                       8       bslbf
*     }
*  }
*
*  NOTE : This mechanism ONLY allows for ISO_LANG_MAX codes per block.
*         If there are more than this number of codes, then they are
*         added in one (or more) chained blocks.
*         ONLY in the root block will the other, non code, information
*         be filled in.
*
*****************************************************************************/
typedef struct _iso_lang_descr
{
   unsigned char    ucLangCount;
   unsigned char    ucAudioType[ISO_LANG_MAX]  ;
   char             acLangCode[ISO_LANG_MAX][3];  /* [10][3] */
   char             szLangText[LANG_TEXT_MAX]  ;  /* delete after checking */    

}  ISO_LANG_DESCR;


/*****************************************************************************
*
*  Item : MUX_BUF_UTIL_DESCR
*
* Usage : Contains the multiplex_buffer_utilization_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  multiplex_buffer_utilization_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     bound_valid_flag                    1       bslbf
*     LTW_offset_lower_bound              15      uimsbf
*     reserved                            1       uimsbf
*     LTW_offset_upper_bound              5       bslbf
*  }
*
*
*****************************************************************************/
typedef struct
{
   unsigned char        fBoundValid;
   unsigned short       uiLTW_offset_lower_bound;
   unsigned short       uiLTW_offset_upper_bound;
}  MUX_BUF_UTIL_DESCR;


/*****************************************************************************
*
*  Item : MAX_BITRATE_DESCR
*
* Usage : Contains the maximum_bitrate_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  maximum_bitrate_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     reserved                            2       bslbf
*     maximum_bitrate                     22      uimsbf
*  }
*
*
*****************************************************************************/
typedef struct
{
   unsigned int ulMaxBitRate;
}  MAX_BITRATE_DESCR;

/*PAGE*/
/*SUBTITLE SI-Descriptors : PRIV_DATA_INDIC_DESCR, REGISTRATION_DESCR  */

/*****************************************************************************
*
*  Item : PRIV_DATA_INDIC_DESCR
*
* Usage : Contains the private_data_indicator_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  private_data_indicator_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     private_data_indicator              32      uimsbf
*  }
*
*
*****************************************************************************/
typedef struct
{
   unsigned int    ulPrivateDataIndicator;
}  PRIV_DATA_INDIC_DESCR;

/*****************************************************************************
*
*  Item : PRIV_DATA_SPECIFIER
*
* Usage : Same as above but as detailed in the DVBSI guidelines document
*
*
*           Syntax                  No. of bits  Mnemonic
*  private_data_indicator_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     private_data_specifier             32      uimsbf
*  }
*
*
*****************************************************************************/
typedef struct
{
   unsigned int    ulPrivateDataSpecifier;
}  PRIV_DATA_SPECIFIER_DESC;


/*****************************************************************************
*
*  Item : FREQUENCY_LIST_DESC
*
* Usage : Same as above but as detailed in the DVBSI guidelines document
*
*	frequency_list_descriptor(){
*		descriptor_tag 			8 uimsbf
*		descriptor_length 		8 uimsbf
*		reserved_future_use 	6 bslbf
*		coding_type 			2 bslbf
*		for (i=0;I<N;i++){
*		centre_frequency 		32 uimsbf
*		}
*	}
*
*
*****************************************************************************/
typedef struct
{
   unsigned char CodingType;
   unsigned char reserved1;
   unsigned char reserved2;
   unsigned char reserved3;
   unsigned int  uiFrequencyListNumber;
   unsigned int  *puiCenterFrequency;
}  FREQUENCY_LIST_DESC;


/*****************************************************************************
*
*  Item : CELL_FREQ_LINK_DESC
*
* Usage : Same as above but as detailed in the DVBSI guidelines document
*
*	cell_frequency_link_descriptor(){
*		descriptor_tag 8 uimsbf
*		descriptor_length 8 uimsbf
*		for (i=0;i<N;i++){
*			cell_id 16 uimsbf
*			frequency 32 uimsbf
*			subcell_info_loop_length 8 uimsbf
*			for (j=0;j<N;j++){
*				cell_id_extension 8 uimsbf
*				transposer_frequency 32 uimsbf
*			}
*		}
*	}
*
*****************************************************************************/
typedef struct
{
   unsigned char *pRawData;
   unsigned int  uiDataSize;
}  CELL_FREQ_LINK_DESC;

/*****************************************************************************

  Item : AC3_DESC

  Usage : AC3 Descriptor

AC-3_descriptor(){
	descriptor_tag 8 uimsbf
	descriptor_length 8 uimsbf
	component_type_flag 1 bslbf
	bsid_flag 1 bslbf
	mainid_flag 1 bslbf
	asvc_flag 1 bslbf
	reserved_flags 4 bslbf
	if (component_type_flag == 1){ 8 uimsbf
		component_type
	}
	if (bsid_flag == 1){ 8 uimsbf
		bsid
	}
	if (mainid_flag == 1){ 8 uimsbf
		mainid
	}
	if (asvc_flag == 1){ 8 uimsbf
		asvc
	}
	for(i=0;i<N;i++){ 8 uimsbf
		additional_info_byte
	}
}
*****************************************************************************/
typedef struct
{
   unsigned char *pRawData;
   unsigned int  uiDataSize;
}  AC3_DESC;

/*****************************************************************************
  Item : EHN_AC3_DESC

  Usage : Enhanced AC3 Descriptor

enhanced_ac-3_descriptor(){
	descriptor_tag 8 uimsbf
	descriptor_length 8 uimsbf
	component_type_flag 1 bslbf
	bsid_flag 1 bslbf
	mainid_flag 1 bslbf
	asvc_flag 1 bslbf
	mixinfoexists 1 bslbf
	substream1_flag 1 bslbf
	substream2_flag 1 bslbf
	substream3_flag 1 bslbf
	if (component_type_flag == 1){ 8 uimsbf
	component_type
	}
	if (bsid_flag == 1){ 8 uimsbf
		bsid
	}
	if (mainid_flag == 1){ 8 uimsbf
		mainid
	}
	if (asvc_flag == 1){ 8 bslbf
		asvc
	}
	if (substream1_flag == 1){ 8 uimsbf
		substream1
	}
	if (substream2_flag == 1){ 8 uimsbf
		substream2
	}
	if (substream3_flag == 1){ 8 uimsbf
		substream3
	}
	for (i=0;i<N;i++){ 8 bslbf
		additional_info_byte
	}
}
*****************************************************************************/
typedef struct
{
   unsigned char *pRawData;
   unsigned int  uiDataSize;
}  EHN_AC3_DESC;

/*****************************************************************************
*
*  Item : DTCP_DESC
*
*  Usage : Contains the DTCP_descriptor. (Digital Transmission Content Protection)
*
*
*                             MPEG Definition
*                             ===============
*
*  Syntax										No. of bits		Mnemonic
*  DTCP_descriptor()
*  {
*      descriptor_tag							          8		uimsbf
*      descriptor_length						          8		uimsbf
*      CA_System_ID								         16		uimsbf
*      for (i = 0; i < (descriptor_length - 2); i++)
*      {
*          private_data_byte					          8		bslbf
*          {
*              Reserved							          8		bslbf
*              Retention_Move_mode				          2		bslbf
*              Retention_State					          1		bslbf
*              EPN								          1		bslbf
*              DTCP_CCI							          2		bslbf
*              Reserved							          8		bslbf
*              Image_Constraint_Token			          1		bslbf
*              APS								          1		bslbf
*          }
*      }
*  }
*
*****************************************************************************/
typedef struct
{
	unsigned short	CA_System_ID;

	//unsigned char	Reserved;
	unsigned char	Retention_Move_mode;
	unsigned char	Retention_State;
	unsigned char	EPN;
	unsigned char	DTCP_CCI;
	//unsigned char	Reserved_;
	unsigned char	Image_Constraint_Token;
	unsigned char	APS;
} DTCP_DESC;


/*****************************************************************************
*
*  Item : DIGITALCOPY_CONTROL_DESC
*
*  Usage : Contains the digital_copy_control_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*  Syntax										No. of bits		Mnemonic
*  digital_copy_control_descriptor()
*  {
*      descriptor_tag							          8		uimsbf
*      descriptor_length						          8		uimsbf
*      digital_recording_control_data			          2		bslbf
*      maximum_bitrate_flag						          1		bslbf
*      component_control_flag					          1		bslbf
*      copy_control_type						          2		bslbf
*      if (copy_control_type != 0)
*      {
*          APS_control_data						          2		bslbf
*      }
*      else
*      {
*          reserved_future_use					          2		bslbf
*      }
*      if (maximum_bitrate_flag == 1)
*      {
*          maximum_bitrate						          8		uimsbf
*      }
*      if (component_control_flag == 1)
*      {
*          component_control_length				          8		uimsbf
*          for (j = 0; j < N; j++)
*          {
*              component_tag					          8		uimsbf
*              digital_recording_control_data	          2		bslbf
*              maximum_bitrate_flag				          1		bslbf
*              reserved_future_use				          1		bslbf
*              copy_control_type				          2		bslbf
*              if (copy_control_type != 0)
*              {
*                  APS_control_data				          2		bslbf
*              }
*              else
*              {
*                  reserved_future_use			          2		bslbf
*              }
*              if (maximum_bitrate_flag == 1)
*              {
*                  maximum_bitrate				          8		uimsbf
*              }
*          }
*      }
*  }
*
*****************************************************************************/
typedef struct
{
	unsigned char	digital_recording_control_data;
	unsigned char	maximum_bitrate_flag;
	unsigned char	component_control_flag;
	unsigned char	copy_control_type;

	unsigned char	APS_control_data;
	//unsigned char	reserved_future_use;

	unsigned char	maximum_bitrate;

#if (0)
	unsigned char	component_control_length;

	unsigned char	component_tag;
	unsigned char	digital_recording_control_data;
	unsigned char	maximum_bitrate_flag_;
	//unsigned char	reserved_future_use_;
	unsigned char	copy_control_type_;

	unsigned char	APS_control_data;
	//unsigned char	reserved_future_use__;

	unsigned char	maximum_bitrate_;
#endif

	unsigned char	ucDescIndex;
} DIGITALCOPY_CONTROL_DESC;


/*****************************************************************************
*
*  Item : CONTENT_AVAILABILITY_DESC
*
*  Usage : Contains the content_availability_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*  Syntax										No. of bits		Mnemonic
*  content_availability_descriptor()
*  {
*      descriptor_tag							          8		uimsbf
*      descriptor_length						          8		uimsbf
*      reserved_future_use						          2		bslbf
*      image_constraint_token					          1		bslbf
*      retention_mode							          1		bslbf
*      retention_state							          3		bslbf
*      encryption_mode							          1		bslbf
*      for (i = 0; i < N; i++)
*      {
*          reserved_future_use					          8		uimsbf
*      }
*  }
*
*****************************************************************************/
typedef struct
{
	//unsigned char	reserved_future_use;
	unsigned char	image_constraint_token;
	unsigned char	retention_mode;			// temporal_accumulation_control_bit;
	unsigned char	retention_state;		// allowable_time_of_temporal_accumulation;
	unsigned char	encryption_mode;		// output_protection_bit;
	//unsigned char	reserved_future_use_;
} CONTENT_AVAILABILITY_DESC;


/*****************************************************************************
*
*  Item : REGISTRATION_DESCR
*
* Usage : Contains the registration_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  registration_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     format_identifier                   32      uimsbf
*     for (i = 0; i < N ; i++ ){
*        additional_identification_info   8       bslbf
*     }
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned int     ulFormatID;
   unsigned char     ucRegLen;
   unsigned char     *paucAddInfo;   /* 252 */
}  REGISTRATION_DESCR;

/*PAGE*/
/*SUBTITLE SI-Descriptors : SYSTEM_CLOCK_DESCR, TARGET_BCK_GRID_DESCR  */

/*****************************************************************************
*
*  Item : SYSTEM_CLOCK_DESCR
*
* Usage : Contains the system_clock_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  system_clock_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     external_clock_reference_indicator  1       bslbf
*     reserved                            1       bslbf
*     clock_accuracy_integer              6       uimsbf
*     clock_accuracy_exponent             3       uimsbf
*     reserved                            5       bslbf
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned char   fExternalClockRef;
   unsigned char  ucClockAccurcyInt;
   unsigned char  ucClockAccurcyExp;
}  SYSTEM_CLOCK_DESCR;


/*****************************************************************************
*
*  Item : TARGET_BCK_GRID_DESCR
*
* Usage : Contains the target_background_grid_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  target_background_grid_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     horizontal_size                     14      uimsbf
*     vertical_size                       14      uimsbf
*     pel_aspect_ratio                    4       uimsbf
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned char     uiHorizontalSize;
   unsigned char     uiVerticalSize;
   PEL_ASPECT_RATIO  enPelAspectRatio;
}  TARGET_BG_GRID_DESCR;

/*PAGE*/
/*SUBTITLE SI-Descriptors : VIDEO_STREAM_DESCR, VIDEO_WINDOW_DESCR  */

/*****************************************************************************
*
*  Item : VIDEO_STREAM_DESCR
*
* Usage : Contains the video stream descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  video_stream_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     multiple_frame_rate_flag            1       bslbf
*     frame_rate_code                     4       uimsbf
*     MPEG_2_flag                         1       bslbf
*     constrained_parameter_flag          1       bslbf
*     still_picture_flag                  1       bslbf
*     if (MPEG_2_flag == 1){
*        profile_and_level_indication     8       uimsbf
*        chroma_format                    2       uimsbf
*        frame_rate_extension_flag        1       bslbf
*        reserved                         5       bslbf
*     }
*  }

*****************************************************************************/
typedef struct
{
   unsigned char        fMultiFrameRate;
   FRAME_RATE_CODES     enFrameRateCode;
   unsigned char        fMPEG_2;
   unsigned char        fConstrainedParam;
   unsigned char        fStillPicture;
   PROFILE_LEVEL_STRUCT stProfileLevel;
   CHROMA_FORMAT        enChromaFormat;
   unsigned char        fFrameRateExt;
}  VIDEO_STREAM_DESCR;

/*****************************************************************************
*
*  Item : VIDEO_WINDOW_DESCR
*
* Usage : Contains the video_window_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  video_window_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     horizontal_offset                   14      uimsbf
*     vertical_offset                     14      uimsbf
*     window_priority                     4       uimsbf
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned short   uiHorizOffset;
   unsigned short   uiVertOffset;
   unsigned char    ucWindowPriority;     /* use 4 bits ONLY */
}  VIDEO_WINDOW_DESCR;

/*****************************************************************************
*
*  Item : SMOOTHING_BUFFER_DESCR
*
* Usage : Contains the video_window_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  smoothing_buffer_descriptor() {
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     reserved                            2        bslbf
*     sb_leak_rate                        22      uimsbf
*     reserved                            2        bslbf
*     sb_size                             22      uimsbf
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned int   ulSbLeakRate;
   unsigned int   ulSbSize;
}  SMOOTHING_BUFFER_DESCR;

/*SUBTITLE DVB-Descriptors : */
/*****************************************************************************
 *  Item : All DVB descriptor structs                                        *
 *****************************************************************************/

/*****************************************************************************
*
*  Item : BOUQUET_NAME_DESCR
* usage : contains the bouquet_name_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  bouquet_name_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for ( i = 0; i<descriptor_length; i++){
*        char                             8       uimsbf
*     }
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned char    ucNameLen;
   char     		*pszBouqName;     /* 256 */
}  BOUQUET_NAME_DESCR;

/*****************************************************************************
*
*  Item : CA_IDENTIFIER_DESCR
*
* Usage : Contains the CA_identifier_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  CA_identifier_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for ( i = 0; i < N; i++){
*        CA_system_id                     16      uimsbf
*     }
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned char   ucCDIDCount;
   unsigned short  auiCASystemID[CA_ID_MAX];
}  CA_IDENTIFIER_DESCR;

/*****************************************************************************
*  Item : CABLE_DS_DESCR
* Usage : Contains the cable_delivery_system_descriptor
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  cable_delivery_system_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     frequency                           32      bslbf
*     DVB_reserved                        12      bslbf
*     FEC_outer                           4       bslbf
*     modulation                          8       bslbf
*     symbol_rate                         28      bslbf
*     FEC_inner                           4       bslbf
*  }
*
* NOTE  For the cable_delivery_system_descriptor :
*
*       The frequency is BCD coded in MHz, where the decimal occurs after
*       the fourth character (e.g. 0312.0000 MHz).
*
*       The modulation field occupies 8 bits.
*
*       The symbol_rate is a 28-bit field  giving the 4-bit BCD values
*       specifying 7 characters of the symbol_rate in Msymbol/s where the
*       decimal point occurs after the third character (e.g. 027.4500).
*
*****************************************************************************/
typedef struct
{
	unsigned char	aucFreq_BCD[4];       /* 8 BCD characters */
	FEC_OUTERBIT   	enFECOuter;
	MODULATION_CAB 	enModulation;
	unsigned char	aucSymRate_BCD[4];    /* 7 BCD characters */
	FEC_INNERBIT		enFECInner;
}  CABLE_DS_DESCR;

/*****************************************************************************
*
*  Item : COMPONENT_DESCR
*
* usage : contains the component_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  component_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     reserved_for_future_use             4       bslbf
*     stream_content                      4       uimsbf
*     component_type                      8       uimsbf
*     component_tag                       8       uimsbf
*     ISO_639_language_code               24      bslbf
*     for (i=0;i<N;i++){
*        text_char                        8       uimsbf
*     }
*  }
*
*  NOTE:  No enum is currently defined for:
*           component_tag
*        It is maintained as a simple unsigned char.
*
*****************************************************************************/
typedef struct
{
   COMP_STREAM_TYPE		enStreamContent;
   unsigned char		ucComponentType;
   unsigned char		ucComponentTag;
   char					acLangCode[3];
   unsigned char		ucTextLen;
   char					*pszComponentText;
}  COMPONENT_DESCR ;

/*****************************************************************************
*
*  Item : CONTENT_DESCR
*
* usage : contains the content_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  content_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i=0;i<N;i++){
*        content_nibble_level_1           4       uimsbf
*        content_nibble_level_2           4       uimsbf
*        user_nibble                      4       uimsbf
*        user_nibble                      4       uimsbf
*     }
*  }
*
*  NOTE:  No enum is currently defined for:
*           content_nibble_level_2
*        It is maintained as a simple unsigned char.
*
*****************************************************************************/
#define  NUM_UCHARS_IN_CONTENT_DATA  2

typedef struct
{
   CONTENT_GENRE	enNib_1;
   unsigned char	ucNib_2;
   unsigned char	ucUserNib_1;
   unsigned char	ucUserNib_2;
}  CONTENT_DATA;

typedef struct
{
  unsigned char		ucDataCount;
  CONTENT_DATA		*pastContentData;
} CONTENT_DESCR;

/*****************************************************************************
*
*  Item : COUNTRY_AVAIL_DESCR
*
* usage : contains the country_availability_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  country_availability_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     country_availability_flag           1       bslbf
*     DVB_reserved                        7       bslbf
*     for (i=0;i<N;i++){
*        country_code                     24      bslbf
*     }
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned char	fAllowCountry;
   unsigned char	ucCountries;
   char				acCountryCode[COUNTRY_CODE_MAX][3];
}  COUNTRY_AVAIL_DESCR;

/*****************************************************************************
*
*  Item : EXT_EVT_DESCR
*
* Usage : Contains the extended_event_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  extended_event_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     descriptor_number                   4       uimsbf
*     last_descriptor_number              4       uimsbf
*     ISO_639_language_code               24      bslbf
*     length_of_items                     8       uimsbf
*     for (i=0; i<N; i++){
*        item_description_length          8       uimsbf
*        for(i=0; i<N; i++){
*           item_description_char         8       uimsbf
*        }
*        item_length                      8       uimsbf
*        for(i=0; i<N; i++){
*           item_char                     8       uimsbf
*        }
*     }
*     text_length                         8       uimsbf
*     for (i=0;i<text_length;i++){
*        text_char                        8       uimsbf
*     }
*
*  }
*
*****************************************************************************/
typedef struct _ext_evt_struct
{
   unsigned char	ucExtEvtItemDescrLen;
   unsigned char	acExtEvtItemDescrName[EXT_EVT_DESCR_MAX];
   unsigned char	ucExtEvtItemLen;
   unsigned char	acExtEvtItemName[EXT_EVT_ITEM_MAX];
   struct _ext_evt_struct *pstExtEvtStruct;
} EXT_EVT_STRUCT;

typedef struct
{
   unsigned char	ucDescrNumber;
   unsigned char	ucLastDescrNumber;
   char				acLangCode[3];
   unsigned char	ucExtEvtCount;
   EXT_EVT_STRUCT	*pstExtEvtList;
   unsigned char	ucExtEvtTextLen;
   unsigned char	acExtEvtTextName[EXT_EVT_TEXT_MAX];
} EXT_EVT_DESCR;


/*****************************************************************************
*
*  Item : LINKAGE_DESCR
*
* Usage : Contains the linkage_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  linkage_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     transport_stream_id                 16      uimsbf
*     original_network_id                 16      uimsbf
*     service_id                          16      bslbf
*     linkage_type                        8       uimsbf
*     for (i=0;i<N;i++){
*        private_data_U8                8       bslbf
*     }
*  }
*
*****************************************************************************/
typedef struct
{
	unsigned short	uiTStreamID;
	unsigned short	wOrgNetID;
	unsigned short	uiServiceID;
	LINKAGE_TYPE	enLinkageType;
	unsigned char	ucPrivDataLen;
	unsigned char	*paucPrivData; /* 256 - 7 */
}  LINKAGE_DESCR;


/*****************************************************************************
*
*  Item : MOSAIC_DESCR
*
* Usage : Contains the mosaic_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  mosaic_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     mosaic_entry_point                  1       bslbf
*     number_of_horizontal_elementary_cells
*                                         3       uimsbf
*     reserved_for_future_use             1       bslbf
*     number_of_vertical_elementary_cells 3       uimsbf
*     for ( i = 0; i < N; i++){
*        logical_cell_id                  6       uimsbf
*        reserved_for_future_use          7       bslbf
*        logical_cell_presentation_info   3       uimsbf
*        elementary_cell_filed_length     8       uimsbf
*        for (i=0; i<elementary_cell_field_length; i++) {
*           reserved_for_future_use       2       bslbf
*           elementary_cell_id            6       uimsbf
*        }
*        cell_linkage_info                8       uimsbf
*        if (cell_linkage_info == 0x01) {
*           bouquet_id                    16      uimsbf
*        }
*        if (cell_linkage_info == 0x02) {
*           original_network_id           16      uimsbf
*           transport_stream_id           16      uimsbf
*           service_id                    16      uimsbf
*        }
*        if (cell_linkage_info == 0x03) {
*           original_network_id           16      uimsbf
*           transport_stream_id           16      uimsbf
*           service_id                    16      uimsbf
*        }
*        if (cell_linkage_info == 0x04) {
*           original_network_id           16      uimsbf
*           transport_stream_id           16      uimsbf
*           service_id                    16      uimsbf
*           event_id
*        }
*     }
*  }
*
*****************************************************************************/

typedef struct
{
   unsigned short uiBouquetID;
} MOSAIC_BOUQUET_LINKAGE;

typedef struct
{
   unsigned short uiOrigNetworkID;
   unsigned short uiTS_ID;
   unsigned short uiServiceID;
} MOSAIC_SERVICE_LINKAGE;

typedef struct
{
   unsigned short uiOrigNetworkID;
   unsigned short uiTS_ID;
   unsigned short uiServiceID;
} MOSAIC_OTHER_MOSAIC_LINKAGE;

typedef struct
{
   unsigned short uiOrigNetworkID;
   unsigned short uiTS_ID;
   unsigned short uiServiceID;
   unsigned short uiEventID;
} MOSAIC_EVENT_LINKAGE;

typedef union
{
   MOSAIC_BOUQUET_LINKAGE        stBouquet;
   MOSAIC_SERVICE_LINKAGE        stService;
   MOSAIC_OTHER_MOSAIC_LINKAGE   stMosaic;
   MOSAIC_EVENT_LINKAGE          stEvent;
} MOSAIC_LINKAGE_UNION;

typedef struct _mosaic_struct
{
   struct _mosaic_struct  *pstMosaicLink;
   MOSAIC_LINKAGE_UNION    unLink;
   MOSAIC_CELL_LINKAGE     enCellLinkType;
   MOSAIC_CELL_LINKAGE     enCellPresInfo;
   unsigned char                   ucLogicCellID;
   unsigned char                   ucElCellFieldLen;
   unsigned char                   aucElCellID[MOSAIC_CELLS_MAX];
} MOSAIC_STRUCT;

typedef struct
{
   MOSAIC_STRUCT  *pstMosaicCell;
   unsigned char            fEntryPoint;
   unsigned char           ucHorizCells;
   unsigned char           ucVertCells;
}  MOSAIC_DESCR;


/*****************************************************************************
*
*  Item : NETWORK_NAME_DESCR
*
* Usage : Contains the network_name_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  network_name_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for ( i = 0; i<descriptor_length; i++){
*        char                             8       uimsbf
*     }
*  }
*
*****************************************************************************/
typedef struct
{
  unsigned char    ucNameLen;
  char *pcNetName; /* 256 */
} NETWORK_NAME_DESCR;

/*****************************************************************************
*
*  Item : NVOD_REF_DESCR
*
* Usage : Contains the NVOD_reference_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  NVOD_reference_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i = 0; i<N; i++){
*        transport_stream_id             16       uimsbf
*        original_stream_id              16       uimsbf
*        service_id                      16       uimsbf
*     }
*  }
*
*****************************************************************************/

#define  NUM_UCHARS_IN_NVOD_REF_STRUCT  6

typedef struct
{
   unsigned short           uiTStreamID;
   unsigned short           uiOrgNetWorkID;
   unsigned short           uiServiceID;
}  NVOD_REF_STRUCT;

typedef struct
{
   unsigned char            ucNVODListCount;
   NVOD_REF_STRUCT *pastNVODList;
}  NVOD_REF_DESCR;


/*****************************************************************************
*
*  Item : PARENT_RATING_DESCR
*
* usage : contains the parental_rating_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  parental_rating_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i=0;i<N;i++){
*        country_code                     24      bslbf
*        rating                           8       uimsbf
*     }
*  }
*
*****************************************************************************/
#define  NUM_UCHARS_IN_PARENT_RATING_DATA  4

typedef struct
{
   char  acCountryCode[3];
   unsigned char  ucRating;
}  PARENT_RATING_DATA;

typedef struct
{
   unsigned char                 ucDataCount;
   PARENT_RATING_DATA   *pastParentData;
}  PARENT_RATING_DESCR;


/*****************************************************************************
*
*  Item : SATELLITE_DS_DESCR
*
* Usage : Contains the satellite_delivery_system_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  satellite_delivery_system_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     frequency                           32      bslbf
*     orbital_position                    16      bslbf
*     west_east_flag                      1       bslbf
*     polarisation                        2       bslbf
*     modulation                          5       bslbf
*     symbol_rate                         28      bslbf
*     FEC_inner                           4       bslbf
*  }
*
*       The frequency is 4-bit BCD coded in GHz, where the decimal point
*       occurs after the third character (e.g. 011.75725 GHz).
*
*       The orbital_position is a 16-bit field giving the 4-bit BCD values
*       specifying 4 characters of the orbital position in degrees where
*       the decimal point occurs after the third character (e.g. 019.2 deg.).
*
*       The modulation field occupies 5 bits.
*
*       The symbol_rate is a 28-bit field  giving the 4-bit BCD values
*       specifying 7 characters of the symbol_rate in Msymbol/s where the
*       decimal point occurs after the third character (e.g. 027.4500).
*
*****************************************************************************/
typedef struct
{
   unsigned char          aucFreq_BCD[4];       /* 8 BCD characters */
   unsigned char          aucOrbitPos_BCD[2];   /* 4 BCD characters */
   WEST_EAST      enWestEast;
   POLARIZATION   enPolarisation;
   MODULATION_SAT enModulation;
   unsigned char          aucSymRate_BCD[4];    /* 7 BCD characters */
   FEC_INNERBIT   enFECInner;
}  SATELLITE_DS_DESCR;


/**********************************************************
**                                                       **
**  The following is used to store the above structure   **
**  efficiently in EEPROM                                **
**                                                       **
**  Special Routines Support the changing to real DS     **
**                                                       **
**** allen rossouw ***************************************/

/**********************************************************
* The 12 bits of data are unpacked as follows:            *
*                                                         *
*   ucData[0]   = aucFreq_BCD[0];                         *
*   ucData[1]   = aucFreq_BCD[1];                         *
*   ucData[2]   = aucFreq_BCD[2];                         *
*   ucData[3]   = aucFreq_BCD[3];                         *
*   ucData[4]   = aucOrbitPos_BCD[0];                     *
*   ucData[5]   = aucOrbitPos_BCD[1];                     *
*   ucData[6]   = aucSymRate_BCD[0];                      *
*   ucData[7]   = aucSymRate_BCD[1];                      *
*   ucData[8]   = aucSymRate_BCD[2];                      *
*   ucData[9]   = aucSymRate_BCD[3];                      *
*   ucData[10]  = enFECInner & 0x0f;                      *
*   ucData[10] |= (enMuxVerif & 0x0f) << 4;               *
*   ucData[11]  = enModulation & 0x1F);                   *
*   ucData[11] |= (enPolarisation & 0x03) << 5;           *
*                                                         *
***********************************************************/

#define SATELLITE_EE_LENGTH 12

#define FREQ_OFFSET         0
#define ORBIT_OFFSET        4
#define SYM_RATE_OFFSET     6
#define MISC_1_OFFSET       10
#define MISC_2_OFFSET       11


typedef struct
{
   unsigned char  ucData[SATELLITE_EE_LENGTH];
}  SATELLITE_EE_DESCR;


/*****************************************************************************
*
*  Item : SERVICE_DESCR
*
* Usage : Contains the service_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  service_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     service_type                        8       uimsbf
*     service_provider_length             8       uimsbf
*     for (i=0;i<N;i++){
*        char                             8       uimsbf
*     }
*     service_name_length                 8       uimsbf
*     for (i=0;i<N;i++){
*        char                             8       uimsbf
*     }
*  }
*
* NOTE : Due to the fact that the lengths cannot be predicted, the two
*        name fields are limited in fixed length, should
*****************************************************************************/
typedef struct
{
   SERVICE_TYPE    enSvcType;
   unsigned char            fSvcProvCrop;  /* name was cropped */
   unsigned char              ucSvcProvLen;
   char           *pszSvcProvider;
   unsigned char            fSvcNameCrop;  /* name was cropped */
   unsigned char              ucSvcNameLen;
   char           *pszSvcName;
}  SERVICE_DESCR;


/*****************************************************************************
*
*  Item : SERVICE_LIST_DESCR
*
* Usage : Contains the service_list_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  service_list_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i = 0; i<N; i++){
*        service_id                       16      uimsbf
*        service_type                     8       uimsbf
*     }
*  }
*
*****************************************************************************/
#define BYTE_SIZE_OF_SERVICE_LIST_STRUCT		(3) // sizeof(unsigned short) + sizeof(unsigned char)

typedef struct
{
	unsigned short	uiServiceID;
	unsigned char	enSvcType; // Ref. SERVICE_TYPE enumeration.
	unsigned char	uc_PADDING_1; // Align Padding (unsigned char)
} SERVICE_LIST_STRUCT;

typedef struct
{
	unsigned char		ucSvcListCount;
	unsigned char		uc_PADDING_1[3]; // For align padding (unsigned short * 3)
	SERVICE_LIST_STRUCT	*pastSvcList;
} SERVICE_LIST_DESCR;


/*****************************************************************************
*
*  Item : SHRT_EVT_DESCR
*
* Usage : Contains the short_event_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  short_event_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     ISO_639_language_code               24      bslbf
*     event_name_length                   8       uimsbf
*     for (i=0;i<event_name_length;i++){
*        event_name_char                  8       uimsbf
*     }
*     text_length                         8       uimsbf
*     for (i=0;i<text_length;i++){
*        text_char                        8       uimsbf
*     }
*
*  }
*
*****************************************************************************/
typedef struct
{
   char  acLangCode[3];
   unsigned char  ucShortEventNameLen;
   unsigned char  *pszShortEventName;
   unsigned char  ucShortEventTextLen;
   unsigned char  *pszShortEventText;
}  SHRT_EVT_DESCR;


/*****************************************************************************
*
*  Item : STREAM_IDENT_DESCR
*
* Usage : Contains the stream_identifier_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  stream_identifier_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     component_tag                       8       uimsbf
*
*  }
*
*****************************************************************************/
#if 0
typedef struct
{
   unsigned char  ucComponentTag;
}  STREAM_IDENT_DESCR;
#else   //modify for ISDB-T, by ashley 070403
typedef struct
{
 	unsigned char descriptor_tag;
	unsigned char descriptor_length;
	unsigned char  ucComponentTag;
}  STREAM_IDENT_DESCR;
#endif

/*****************************************************************************
*
*  Item : TELETEXT_DESCR
*
* Usage : Contains the teletext_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  teletext_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i = 0; i<N; i++){
*        ISO_639_language_code            24      bslbf
*        teletext_type                    5       uimsbf
*        teletext_magazine_number         3       uimsbf
*        teletext_page_number             8       uimsbf
*     }
*  }
*
*****************************************************************************/
#define  NUM_UCHARS_IN_TELETEXT_DATA    5  /* in DVB definition, type & magazine num make up one UCHAR */

typedef struct
{
   char           acLangCode[ISO_LANG_CODE_LENGTH];
   TELETEXT_TYPE  enType;
   unsigned char           fActive;
   unsigned char          ucMagazineNum;
   unsigned char          ucPageNum;
}  TELETEXT_DATA;

typedef struct
{
   unsigned char                 ucCount;
   TELETEXT_DATA        *pastTeletext;
}  TELETEXT_DESCR;


/*****************************************************************************
*
*  Item : TERRESTRIAL_DS_DESCR
*
* Usage : Contains the terrestrial_delivery_system_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  terrestrial_delivery_system_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     TBD                                 88      bslbf
*  }
*
*****************************************************************************/
typedef struct
{
	//unsigned char	paucTemp[TERRESTRIAL_TBD_MAX];
	unsigned char *pRawData;
	unsigned int  uiDataSize; //MAX size is TERRESTRIAL_TBD_MAX
}  TERRESTRIAL_DS_DESCR;

/*****************************************************************************
*
*  Item : ISDBT_TERRESTRIAL_DS_DESCR
*
* Usage : Contains the terrestrial_delivery_system_descriptor for ISDB-T
*
*
*                             ISDB-T Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  terrestrial_delivery_system_descriptor(){
*     descriptor_tag				8	uimsbf
*     descriptor_length			8	uimsbf
*     area_code				12	bslbf
*	guard_interval				2	bslbf
*	transmission_mode			2	bslbf
*	for (i = 0; i<N; i++){
*		frequency			16	bslbf
*     }
*  }
*
*****************************************************************************/
typedef struct
{	
	unsigned short 	area_code;
	unsigned char		guard_interval;
	unsigned char		transmission_mode;
	unsigned char		freq_ch_num;
	unsigned char		freq_ch_table[MAX_FREQ_TBL_NUM];
}  ISDBT_TERRESTRIAL_DS_DESCR;


/*****************************************************************************
*
*  Item : TIME_S_EVT_DESCR
*
* Usage : Contains the time_shifted_event_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  time_shifted_event_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     reference_service_id                16      uimsbf
*     reference_event_id                  16      uimsbf
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned short  uiRefServiceID;
   unsigned short  wRefEventID;
}  TIME_S_EVT_DESCR;


/*****************************************************************************
*
*  Item : TIME_S_SVC_DESCR
*
* Usage : Contains the time_shifted_service_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  time_shifted_service_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     reference_service_id                16      uimsbf
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned short  uiRefServiceID;
}  TIME_S_SVC_DESCR;

/*****************************************************************************
*
*  Item : LOCAL_TIME_OFFSET_DESCR
*
* Usage : Contains the local_time_offset_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  local_time_offset_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i = 0; i<N; i++){
*        country_code                     24      bslbf
*        country_region_id                6       bslbf
*        reserved                         1       bslbf
*        local_time_offset_polarity       1       bslbf
*        local_time_offset                16      bslbf
*        time_of_change                   40      bslbf
*        next_time_offset                 16       bslbf
*     }
*  }
*
*****************************************************************************/
typedef struct
{
   unsigned char                   aucCountryCode[COUNTRY_CODE_MAX];
   COUNTRY_REGION                     aucCountryRegionID;
   unsigned char                          ucLocalTimeOffsetPolarity;
   unsigned char                           ucLocalTimeOffset_BCD[2];   /* 4 BCD characters */
   unsigned char                              ucTimeOfChange_BCD[5];   /* 4 BCD characters */
   unsigned char                            ucNextTimeOffset_BCD[2];   /* 4 BCD characters */
}  LOCAL_TIME_OFFSET_DATA;

typedef struct
{
   unsigned char                                             ucCount;
   LOCAL_TIME_OFFSET_DATA		astLocalTimeOffsetData[COUNTRY_REGION_ID_MAX];
}  LOCAL_TIME_OFFSET_DESCR;

/*****************************************************************************
*
*  Item : SUBTITLEING_DESCR
*
* Usage : Contains the subtitling_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  teletext_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i = 0; i<N; i++){
*        ISO_639_language_code            24      bslbf
*        subtitling_type                  8       bslbf
*        composition_page_id              16      bslbf
*        ancillary_page_id                16      bslbf
*     }
*  }
*
*****************************************************************************/
typedef struct
{
   char             acLangCode[ISO_LANG_CODE_LENGTH];
   unsigned char             ucSubtitlingType;
   unsigned short            uiCompositionPageId;
   unsigned short            uiAncillaryPageId;
}  SUBTITLE_DATA;

typedef struct
{
   unsigned char                 ucCount;
 //  SUBTITLE_DATA        *pastSUBTITLE;
   SUBTITLE_DATA        pastSUBTITLE[SUBTITLE_OFFSET_MAX];
}  SUBTITLING_DESCR;

/*****************************************************************************
*
*  Item : MULTI_BOUQUET_NAME_DESCR
*
* Usage : Contains the multilingual_bouquet_name_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  multilingual_bouquet_name_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i = 0; i<N; i++){
*        ISO_639_language_code            24      bslbf
*        bouquet_name_length              8       bslbf
*        for (i = 0; i<N; i++){
*           char                          8       bslbf
*        }
*     }
*  }
*
*****************************************************************************/
typedef struct
{
   char             acLangCode[ISO_LANG_CODE_LENGTH];
   unsigned char             ucBouquetNameLength;
   unsigned char             *pucChar;
}  MULTI_BOUQUET_NAME_DATA;

typedef struct
{
   unsigned char                          ucCount;
   MULTI_BOUQUET_NAME_DATA        *pastMultiBouquetName;
}  MULTI_BOUQUET_NAME_DESCR;

/*****************************************************************************
*
*  Item : MULTI_COMPONENT_DESCR
*
* Usage : Contains the multilingual_bouquet_name_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  multilingual_component_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i = 0; i<N; i++){
*        ISO_639_language_code            24      bslbf
*        text_description_length          8       bslbf
*        for (i = 0; i<N; i++){
*           text_char                     8       bslbf
*        }
*     }
*  }
*
*****************************************************************************/
typedef struct
{
   char             acLangCode[ISO_LANG_CODE_LENGTH];
   unsigned char             ucTextDescrLength;
   unsigned char             *pucTextChar;
}  MULTI_COMPONENT_DATA;

typedef struct
{
   unsigned char                          ucCount;
   MULTI_COMPONENT_DATA           *pastMultiComponent;
}  MULTI_COMPONENT_NAME_DESCR;

/*****************************************************************************
*  Item : MULTI_NETWORK_NAME_DESCR
* Usage : Contains the multilingual_network_name_descriptor
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  multilingual_network_name_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i = 0; i<N; i++){
*        ISO_639_language_code            24      bslbf
*        network_name_length              8       bslbf
*        for (i = 0; i<N; i++){
*           char                          8       bslbf
*        }
*     }
*  }
*****************************************************************************/
typedef struct
{
   char             acLangCode[ISO_LANG_CODE_LENGTH];
   unsigned char               ucNetworkNameLength;
   unsigned char              *pucChar;
}  MULTI_NETWORK_NAME_DATA;

typedef struct
{
   unsigned char                              ucCount;
   MULTI_NETWORK_NAME_DATA        *pastMultiNetworkName;
}  MULTI_NETWORK_NAME_DESCR;

/*****************************************************************************
*
*  Item : MULTI_SERVICE_NAME_DESCR
*
* Usage : Contains the multilingual_service_name_descriptor
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  multilingual_service_name_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i = 0; i<N; i++){
*        ISO_639_language_code            24      bslbf
*        service_provider_name_length     8       bslbf
*        for (i = 0; i<N; i++){
*           char                          8       bslbf
*        }
*        service_name_length              8       bslbf
*        for (i = 0; i<N; i++){
*           char                          8       bslbf
*        }
*     }
*  }
*
*****************************************************************************/
typedef struct
{
   char             acLangCode[ISO_LANG_CODE_LENGTH];
   unsigned char             ucProviderNameLength;
   unsigned char             *pucPrviderChar;
   unsigned char             ucServiceNameLength;
   unsigned char             *pucServiceChar;
}  MULTI_SERVICE_NAME_DATA;

typedef struct
{
   unsigned char                             ucCount;
   MULTI_SERVICE_NAME_DATA        *pastMultiServiceName;
}  MULTI_SERVICE_NAME_DESCR;


/*****************************************************************************
*  Item : SERVICE_MOVE_DESCR
* Usage : Contains the service_move_descriptor
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  service_move_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     new_original_network_id             16      uimsbf
*     new_transport_stream_id             16      uimsbf
*     new_service_id                      16      uimsbf
*  }
*****************************************************************************/
typedef struct
{
   unsigned char                          ucCount;
   unsigned short                         uiNewOriginalNetworkId;
   unsigned short                         uiNewTransportStreamId;
   unsigned short                         uiNewServiceId;
}  SERVICE_MOVE_DESCR;

/*****************************************************************************
*  Item : LOGICAL_CHANNEL_DECSR
* Usage : provides a default channel number label for services.
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  logical_channel_descriptor{ 
*      descriptor_tag                   8         uimsbf 
*      descriptor_length                8         uimsbf 
*      for (i=0; i<N; i++){ 
*          service_id                  16         uimsbf 
*          reserved                     6         bslbf 
*          logical_channel_number      10         uimsbf 
*      } 
*  }
*****************************************************************************/
#define NUM_UCHARS_IN_LOGICAL_CHANNEL_STRUCT 4
typedef struct
{
   U16   uiServiceID;
   U16   uiLogicalChNumber;
}  LOGICAL_CHANNEL_STRUCT;

typedef struct _logical_channel_descr
{
   U8     ucLogicalChNumberCount;
   LOGICAL_CHANNEL_STRUCT *pastLogicalChannelStruct;
}  LOGICAL_CHANNEL_DECSR;

/* skkim Add 2001.12.19  start */
/*****************************************************************************
*  Item : USER_DEFINED_DESCR
* Usage : Contains the service_move_descriptor
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  service_move_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     for (i = 0; i<N; i++){
*           char                          8       bslbf
*     }
*  }
*****************************************************************************/
typedef struct
{
   unsigned char			  ucNameLen;
   char             *pucUserDataChar;
}  USER_DEFINED_DESCR;
/* skkim Add 2001.12.19  end */

/* skott Add 2005.9.13  start */
/*****************************************************************************
*  Item : DECODER_SPECIFIC_INFO
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  DecoderSpecificInfo(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     specificInfo[descriptor_length]     8       uimsbf
*  }
*****************************************************************************/
typedef struct
{
	unsigned char			ucLength;
	unsigned char			pucSpecInfo[256];
}  DECODER_SPECIFIC_INFO;


/*****************************************************************************
*  Item : DECODER_CONFIG_DESCR
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  DecoderConfigDescriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     ObjectProfileIndication             8       uimsbf
*     StreamType 		          		  6       uimsbf
*     UpStream		                      1       uimsbf
*     reserved				         	  1       uimsbf
*     BufferSizeDB		            	  24       uimsbf
*     MaxBitrate		           		  32      uimsbf
*     AvgBitrate		            	  32       uimsbf
*
*     DecoderSpecificInfo()
*  }
*****************************************************************************/
typedef struct
{
	unsigned char			ucObjectProfileIndication;
	unsigned char			ucStreamType;
	unsigned int			uiBufferSizeDB;
	unsigned int			uiMaxBitrate;
	unsigned int			uiAvgBitrate;

	DECODER_SPECIFIC_INFO	stSpecInfo;
}  DECODER_CONFIG_DESCR;

/*****************************************************************************
*  Item : SL_CONFIG_DESCR
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  SL_Config_Descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     Predefined	                      8       uimsbf
*
*     UseAccessUnitStartFlag	          1       uimsbf
*     UseAccessUnitEndFlag		          1       uimsbf
*     UseRandomAccessPointFlag	          1       uimsbf
*     UseRandomAccessUnitOnlyFlag	      1       uimsbf
*     UsePaddingFlag				      1       uimsbf
*     UseTimeStampsFlag				      1       uimsbf
*     UseIdleFlag				          1       uimsbf
*     DurationFlag				          1       uimsbf
*
*     TimeStampResolution		          32      uimsbf
*     OCRResolution				          32      uimsbf
*
*     TimeStampLength			          8       uimsbf
*     OCRLength				        	  8       uimsbf
*     AU_Length						      8       uimsbf
*     InstantBitrate				      8       uimsbf
*     DegradationPriorityLength	          4       uimsbf
*     AU_SegNumLength			          5       uimsbf
*     Packet_SegNumLength		          5       uimsbf
*     reserved					          2       uimsbf
*  }
*****************************************************************************/
typedef struct
{
	unsigned char			Predefined;
	
	unsigned char			ucUseAccessUnitStartFlag;
	unsigned char			ucUseAccessUnitEndFlag;
	unsigned char			ucUseRandomAccessPointFlag;
	unsigned char			ucUseRandomAccessUnitOnlyFlag;
	unsigned char			ucUsePaddingFlag;
	unsigned char			ucUseTimeStampsFlag;
	unsigned char			ucUseIdleFlag;
	unsigned char			ucDurationFlag;
	
	unsigned int			uiTimeStampResolution;
	unsigned int			uiOCRResolution;
	
	unsigned char			ucTimeStampLength;
	unsigned char			ucOCRLength;
	unsigned char			ucAU_Length;
	unsigned char			ucInstantBitrate;
	
	unsigned char			ucDegradationPriorityLength;
	unsigned char			ucAU_SeqNumLength;
	unsigned char			ucPacket_SegNumLength;

}  SL_CONFIG_DESCR;


/*****************************************************************************
*  Item : ES_DESCR
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  ES_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     ES_ID			                      16      uimsbf
*     streamDependenceFlag 		          1       uimsbf
*     URL_Flag		                      1       uimsbf
*     OCRstreamFlag				          1       uimsbf
*     streamPriority		              5       uimsbf
*	  if( streamDependenceFlag )
*     	  dependsOn_ES_ID                 16      uimsbf
*	  if( URL_Flag )
*     {
*     	  URLlength                       8       uimsbf
*         URLstring[URLlength]            8*N     bslbf
*     }
*	  if( OCRstreamFlag )
*     	  OCR_ES_ID                       16      uimsbf
*
*     ExtensionDescriptor()
*     LanguageDescriptor()
*     DecoderConfigDescriptor()
*     SLConfigDescriptor()
*     IP_IdentificationDataSet()
*     QoS_Descriptor()
*  }
*****************************************************************************/
typedef struct _es_descriptor_
{
	unsigned short			usES_ID;
	unsigned short          usDependsOn_ES_ID;
	unsigned short			usOCR_ES_ID;
	DECODER_CONFIG_DESCR	stDecoderConfigDescr;
	SL_CONFIG_DESCR			stSLConfigDescr;
	struct _es_descriptor_	*pstLinkedESD;
}  ES_DESCR;

/*****************************************************************************
*  Item : OBJECT_DESCR
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  OBJECT_DESCR(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   16      uimsbf
*     ObjectDescriptorID                  10      uimsbf
*     URL_Flag		                      1       uimsbf
*     IncludeInLineProfilesFlag           1       uimsbf
*     reserved					          4       uimsbf
*	  if( !URL_Flag ){
*	  	OCI_descriptor()
*		ES_descriptor()
*	  }
*  }
*****************************************************************************/
typedef struct
{
	unsigned short		usObjectDescriptorID;
	unsigned short		usESCount;
	ES_DESCR			*stESD;
}  OBJECT_DESCR;

/*****************************************************************************
*  Item : INITIAL_OBJECT_DESCR
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  INITIAL_OBJECT_DESCR(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   16      uimsbf
*     ObjectDescriptorID                  10      uimsbf
*     URL_Flag		                      1       uimsbf
*     IncludeInLineProfilesFlag           1       uimsbf
*     reserved					          4       uimsbf
* 	  if( URL_Flag ){
*		URLstring[descriptor_length -2]
*	  }
*	  else {
*     	ODProfileLevelIndication            8       uimsbf  // not mpeg spec
*     	sceneProfileLevelIndication         8       uimsbf
*     	audioProfileLevelIndication         8       uimsbf
*     	visualProfileLevelIndication        8       uimsbf
*     	graphicsProfileLevelIndication      8       uimsbf
*	  }
*	  if( !URL_Flag ){
*	  	OCI_descriptor()
*		ES_descriptor()
*	  }
*  }
*****************************************************************************/
typedef struct
{
	unsigned short		usObjectDescriptorID;
	unsigned short		usESCount;
	ES_DESCR			*stESD;
}  INITIAL_OBJECT_DESCR;

/*****************************************************************************
*  Item : IOD_DESCR
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  service_move_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     scope_of_IOD_label                  8       uimsbf
*     IOD_label		                      8       uimsbf
*     for (i = 0; i<N; i++){
*           char                          8       bslbf
*     }
*  }
*****************************************************************************/
typedef struct
{
	unsigned char			ucScope_of_IOD_label;
	unsigned char			ucIOD_label;
	INITIAL_OBJECT_DESCR	stIOD;
}  IOD_DESCR;

/*****************************************************************************
*  Item : SL_DESCR
*                             MPEG Definition
*                             ===============
*           Syntax                  No. of bits  Mnemonic
*  service_move_descriptor(){
*     descriptor_tag                      8       uimsbf
*     descriptor_length                   8       uimsbf
*     ES_ID                  			  16       uimsbf
*  }
*****************************************************************************/
typedef struct
{
	unsigned short			usES_ID;
}  SL_DESCR;

/* skott Add 2005.9.13   end */

typedef struct
{
	unsigned short	data_component_id;
	unsigned char		DMF;
	unsigned char 	Timing;
} DATA_COMPONENT_DESCR;

typedef struct
{
	unsigned short			numServiceId;
	unsigned short			servicd_id[8];
} PARTIAL_RECEPTION_DESCR;

typedef struct
{
	unsigned short 	remote_control_key_id;
	unsigned char		length_of_ts_name;
	unsigned char		transmission_type_count;
	/* The maximum number of char is 10 two-byte characters plus '\0' */
	unsigned char 	ts_name_char[MAX_TS_NAME_LEN+1]; 
} TS_INFO_DESCR;

typedef struct
{
	unsigned char 	broadcaster_type;
	
	unsigned short	terrestrial_broadcaster_id;
	unsigned char	affiliation_id_num;
	unsigned char	affiliation_id_array[AFFILIATION_ID_MAX+1];
	
	unsigned char broadcaster_id_num;
	unsigned short original_network_id[AFFILIATION_ID_MAX+1];
	unsigned char broadcaster_id[AFFILIATION_ID_MAX+1];	
} EXT_BROADCASTER_DESCR;

/*****************************************************************************
 * EVT_GROUP_DESCR_ID
 *	event_group_descriptor(){ 
 *		descriptor_tag    					8         uimsbf 
 *		descriptor_length  					8         uimsbf 
 *		group_type		  					4         uimsbf 
 *		event_count		  					4         uimsbf 
 *		for (i=0;i<event_group;i++){             
 *			service_id  					16        uimsbf 
 *			event_id  						16        uimsbf 
 *		} 
 *****************************************************************************/
typedef struct {
	unsigned char			group_type;
	unsigned char 			event_count;
	unsigned short			service_id;
	unsigned short			event_id;
} EVT_GROUP_DESCR;

/*****************************************************************************
 * EMERGENCY_INFORMATION_DESCR_ID
 *	emergency_information_descriptor(){ 
 *		descriptor_tag    					8         uimsbf 
 *		descriptor_length  					8         uimsbf 
 *		for (i=0;i<N;i++){             
 *			service_id  					16        uimsbf 
 *			start_end_flag  				1         bslbf 
 *			signal_level  					1         bslbf 
 *			reserved_future_use  			6         bslbf 
 *			area_code_length  				8         uimsbf 
 *			for (j=0;j<N;j++){             
 *				area_code  				12        bslbf 
 *				reserved_future_use  		4         bslbf 
 *				}             
 *			}             
 *		} 
 *****************************************************************************/
typedef struct _emergency_area_code{
	struct _emergency_area_code *pNext;
	unsigned short area_code;
}EMERGENCY_AREA_CODE;

typedef struct _emergency_info_data{
	struct _emergency_info_data *pNext;
	unsigned short				service_id;
	unsigned char				start_end_flag;
	unsigned char				signal_level;
	unsigned char				reserved_future_use;
	EMERGENCY_AREA_CODE			*pAreaCode;
}EMERGENCY_INFO_DATA;

typedef struct {
	EMERGENCY_INFO_DATA	*pEID;
}EMERGENCY_INFO_DESCR;

#define	AUDIOCOMPONENTDESCR_TEXT_MAX	33
typedef struct _audio_component_descr {
	unsigned char	stream_content;
	unsigned char	component_type;
	unsigned char component_tag;
	unsigned char	stream_type;
	unsigned char	simulcast_group_tag;
	unsigned char	ES_multi_lingual_flag;
	unsigned char	main_component_flag;
	unsigned char	quality_indicator;
	unsigned char	sampling_rate;
	char             acLangCode[2][3];
	unsigned char	acAudioTypeName[AUDIOCOMPONENTDESCR_TEXT_MAX+1];
} AUDIO_COMPONENT_DESCR;

#define	LOGO_TRANSMISSION_SIMPLE_LENGTH	100
typedef struct _logo_transmission_descr {
	unsigned char		logo_transmission_type;
	unsigned short	logo_id;
	unsigned short	logo_version;
	unsigned short	download_data_id;
	unsigned char		simple_logo[LOGO_TRANSMISSION_SIMPLE_LENGTH+1];
	unsigned int		simple_logo_length;
} LOGO_TRANSMISSION_DESCR;

/*********************************************************************
*                                                                    *
*  Item : DESCRIPTOR_STRUCT                                          *
*                                                                    *
* Usage : Contains the union of all descriptor structs               *
*                                                                    *
*********************************************************************/
typedef union
{
   AUDIO_STREAM_DESCR         stASD;
   CA_DESCR                   stCAD;
   COPYRIGHT_DESCR            stCRD;
   DATA_S_ALIGN_DESCR         stDSAD;
   HIERARCY_DESCR             stHD;
   ISO_LANG_DESCR             stISO_LD;
   MAX_BITRATE_DESCR          stMBRD;
   MUX_BUF_UTIL_DESCR         stMBUD;
   PRIV_DATA_INDIC_DESCR      stPDID;
   REGISTRATION_DESCR         stRD;
   SYSTEM_CLOCK_DESCR         stSCD;
   TARGET_BG_GRID_DESCR       stTBGD;
   VIDEO_STREAM_DESCR         stVSD;
   VIDEO_WINDOW_DESCR         stVWD;
   SMOOTHING_BUFFER_DESCR     stSBD;
   
   BOUQUET_NAME_DESCR         stBND;
   CA_IDENTIFIER_DESCR        stCAID;
   CABLE_DS_DESCR             stCDSD;
   COMPONENT_DESCR            stCOMD;
   CONTENT_DESCR              stCOND;
   COUNTRY_AVAIL_DESCR        stCAVD;
   EXT_EVT_DESCR              stEED;
   LINKAGE_DESCR              stLD;
   MOSAIC_DESCR               stMD;
   NETWORK_NAME_DESCR         stNND;
   NVOD_REF_DESCR             stNVOD;
   PRIV_DATA_SPECIFIER_DESC   stPDSD;
   FREQUENCY_LIST_DESC		  stFLD;   
   CELL_FREQ_LINK_DESC		  stCFLD;
   AC3_DESC					  stAC3D;
   EHN_AC3_DESC				  stEHNAC3D;
   PARENT_RATING_DESCR        stPRD;
   SATELLITE_DS_DESCR         stSDSD;
   SERVICE_DESCR              stSD;
   SERVICE_LIST_DESCR         stSLD;
   SHRT_EVT_DESCR             stSED;
   STREAM_IDENT_DESCR         stSID;
   TELETEXT_DESCR             stTTXD;
   TERRESTRIAL_DS_DESCR       stTDSD;
   ISDBT_TERRESTRIAL_DS_DESCR	stITDSD;
   TIME_S_SVC_DESCR           stTSSD;
   TIME_S_EVT_DESCR           stTSED;
   LOCAL_TIME_OFFSET_DESCR    stLTOD;
   SUBTITLING_DESCR           stSUBD;
   MULTI_NETWORK_NAME_DESCR   stMNND;
   MULTI_BOUQUET_NAME_DESCR   stMBND;
   MULTI_SERVICE_NAME_DESCR   stMSND;
   MULTI_COMPONENT_NAME_DESCR stMCND;
   SERVICE_MOVE_DESCR         stSMD;
   IOD_DESCR                  stIOD_D;
   SL_DESCR                   stSL_D;
   OBJECT_DESCR               stOD_D;
   DATA_COMPONENT_DESCR       stDCOMP_D;
   PARTIAL_RECEPTION_DESCR    stPRECEP_D;
   USER_DEFINED_DESCR         stUDEFDE;
   DTCP_DESC                  stDTCPD;
   DIGITALCOPY_CONTROL_DESC   stDCCD;
   CONTENT_AVAILABILITY_DESC  stCONTAD;
   TS_INFO_DESCR              stTSID;
   EXT_BROADCASTER_DESCR      stEBD;
   EVT_GROUP_DESCR            stEGD;
   LOGICAL_CHANNEL_DECSR      stLCD;
   LOGICAL_CHANNEL_DECSR      stHDLCD;
   EMERGENCY_INFO_DESCR       stEID;
   AUDIO_COMPONENT_DESCR      stACD;
   LOGO_TRANSMISSION_DESCR	stLTD;
} DESCRIPTOR_UNION;

typedef struct _descriptor_struct
{
   struct _descriptor_struct  *pstLinkedDescriptor;
   DESCRIPTOR_IDS              enDescriptorID;
   DESCRIPTOR_UNION            unDesc;
}  DESCRIPTOR_STRUCT;

/*****************************************************************************
*
*  Item : TRANS_STREAM_DESCR
*
* Usage : Contains the transport stream descriptor(s) for the BAT/NIT
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*     DVB_reserved                        4       bslbf
*     transport_stream_loop_length        12      uimsbf
*     for(i=0;i<N;i++){
*        transport_stream_id              16      uimsbf
*        original_network_id              16      uimsbf
*        DVB_reserved                     4       bslbf
*        transport_descriptors_length     12      uimsbf
*        for(j=0;j<N;j++){
*           descriptor()
*        }
*     }
*
*****************************************************************************/
typedef struct _trans_stream_descr
{
   unsigned short                          uiTStreamID;
   unsigned short                          uiOrgTStreamID;
   unsigned short                          uiTSDescrCount;
   DESCRIPTOR_STRUCT            *pstTSDescriptor;
   struct _trans_stream_descr   *pstTStreamDescriptor;
}  TRANS_STREAM_DESCR;

/*PAGE*/
/*SUBTITLE PMT_STREAM_STRUCT */

/*****************************************************************************
*
*  Item : PMT_STREAM_DESCR
*
* Usage : Contains the transport stream descriptor for the PMT
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*     for (i=0;i<P;i++) {
*             stream_type                 8       uimsbf
*             reserved                    3       bslbf
*             elementary_PID              13      uimsnf
*             reserved                    4       bslbf
*             ES_info_length              12      uimsbf
*             for (i=0; i<Q; i++) {
*                     descriptor()
*             }
*     }
*
*****************************************************************************/
typedef struct _pmt_stream_descr
{
	ES_TYPE                    enStreamType;
	unsigned short                        uiStreamPID;
	unsigned char				ucTransport_Scrambling_Control;
	unsigned short                        uiLocalDescrCount;
	DESCRIPTOR_STRUCT         *pstLocalDescriptor;
	struct _pmt_stream_descr  *pstStreamDescriptor;
	unsigned char                       fActive;
}  PMT_STREAM_DESCR;

/*****************************************************************************
*
*  Item : PAT_ENTRY_STRUCT
*         PAT_STRUCT
*
* Usage : Contains the Program Association Table Data
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  program_association_section() {
*     table_id                            8       uimsbf
*     section_syntax_indicator            1       bslbf
*     '0'                                 1       bslbf
*     reserved                            2       bslbf
*     section_length                      12      uimsbf
*     transport_stream_id                 16      uimsbf
*     reserved                            2       bslbf
*     version_number                      5       uimsbf
*     current_next_indicator              1       bslbf
*     section_number                      8       uimsbf
*     last_section_number                 8       uimsbf
*     for (i=0; i<N;i++) {
*        program_number                   16      uimsbf
*        reserved                         3       bslbf
*        if(program_number == '0') {
*           network_PID                   13      uimsbf
*        }
*        else {
*           program_map_PID               13      uimsbf
*        }
*     }
*     CRC_32                              32      rpchof
*  }
*
*
*  NOTE : This mechanism ONLY allows for PAT_ENTRY_MAX entires per block.
*         If there are more than this number of services, then they are
*         added in one (or more) chained blocks.
*         ONLY in the root block will the other, non entry, information
*         be filled in.
*
*****************************************************************************/
typedef struct
{
   unsigned short  uiServiceID;
   unsigned short  uiPID;
}  PAT_ENTRY_STRUCT;

typedef struct
{
   MPEG_TABLE_IDS       enTableType;      /* the order of the first 5 fields */
   unsigned short                  uiTransportID;     /* is important as it is used as   */
   unsigned char                   ucVersionNumber;   /* a mask for all table structs    */
   unsigned char                   ucSection;
   unsigned char                   ucLastSection;
   CURR_NEXT            enCurrNext;       /* CurrentNext */
   unsigned short                  uiEntries;         /* number of program entires */
   PAT_ENTRY_STRUCT     astPatEntries[PAT_ENTRY_MAX];
}  PAT_STRUCT;

/*****************************************************************************
*
*  Item : STREAM_DESCR
*         PMT_STRUCT
*
* Usage : Contains the CA Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  TS_program_map_section() {
*     table_id                            8       uimsbf
*     section_syntax_indicator            1       bslbf
*     '0'                                 1       bslbf
*     reserved                            2       bslbf
*     section_length                      12      uimsbf
*     program_number                      16      uimsbf
*     reserved                            2       bslbf
*     version_number                      5       uimsbf
*     current_next_indicator              1       bslbf
*     section_number                      8       uimsbf
*     last_section_number                 8       uimsbf
*     reserved                            3       bslbf
*     PCR_PID                             13      uimsbf
*     reserved                            4       bslbf
*     program_info_length                 12      uimsbf
*     for (i=0; i<N; i++) {
*             descriptor()
*     }
*     for (i=0;i<P;i++) {
*             stream_type                 8       uimsbf
*             reserved                    3       bslbf
*             elementary_PID              13      uimsnf
*             reserved                    4       bslbf
*             ES_info_length              12      uimsbf
*             for (i=0; i<Q; i++) {
*                     descriptor()
*             }
*     }
*     CRC_32                              32      rpchof
*  }
*
*  NOTE : The descriptors are chained in the order that they are found
*         in the raw MPEG data.
*
*****************************************************************************/
typedef struct
{
   MPEG_TABLE_IDS       enTableType;      /* the order of the first 5 fields */
   unsigned short                  uiServiceID;       /* is important as it is used as   */
   unsigned char                   ucVersionNumber;   /* a mask for all table structs    */
   unsigned char                   ucSection;
   unsigned char                   ucLastSection;
   CURR_NEXT            enCurrNext;
   unsigned short                  uiPCR_PID;
   unsigned short                  wDataStreamPID;   /* valid or 0xffff if none */
   unsigned short                  uiGDescrCount;
   DESCRIPTOR_STRUCT   *pstGlobDescr;
   unsigned short                  uiEStreamCount;
   PMT_STREAM_DESCR    *pstEStreamDescr;
   /*                                                 */
   /* The following is for easier & faster processing */
   /* i.e. not directly MPEGeneese                    */
   /*                                                 */
   short                  iA_Lang_Count;          /* Audio language streams                    */
   unsigned char                 fVideoComponent;        /* does service have a video component       */
}  PMT_STRUCT;

/*****************************************************************************
*
*  Item : NIT_STRUCT
*
* Usage : Contains the Network Information Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  network_information_section(){
*     table_id                            8       uimsbf
*     section_syntax_indicator            1       bslbf
*     DVB_reserved                        1       bslbf
*     ISO_reserved                        2       bslbf
*     section_length                      12      uimsbf
*     network_id                          16      uimsbf
*     ISO_reserved                        2       bslbf
*     version_number                      5       uimsbf
*     current_next_indicator              1       bslbf
*     section_number                      8       uimsbf
*     last_section_number                 8       uimsbf
*     DVB reserved                        4       uimsbf
*     network_descriptors_length          12      uimsbf
*     for(i=0;i<N;i++){
*        descriptor()
*     }
*     DVB_reserved                        4       bslbf
*     transport_stream_loop_length        12      uimsbf
*     for(i=0;i<N;i++){
*        transport_stream_id              16      uimsbf
*        original_network_id              16      uimsbf
*        DVB_reserved                     4       bslbf
*        transport_descriptors_length     12      uimsbf
*        for(j=0;j<N;j++){
*           descriptor()
*        }
*     }
*     CRC_32                              32      rpchof
*  }
*
*  NOTE : The descriptors are chained in the order that they are found
*         in the raw MPEG data.
*
*****************************************************************************/
typedef struct
{
   MPEG_TABLE_IDS       enTableType;      /* the order of the first 5 fields */
   unsigned short                 uiNetworkID;       /* is important as it is used as   */
   unsigned char                 ucVersionNumber;   /* a mask for all table structs    */
   unsigned char                 ucSection;
   unsigned char                 ucLastSection;
   CURR_NEXT            enCurrNext;
   unsigned short                 uiNDescrCount;
   DESCRIPTOR_STRUCT   *pstNDescr;
   unsigned short                 uiTSDescrCount;
   TRANS_STREAM_DESCR  *pstTSDescriptor;
}  NIT_STRUCT;

/*****************************************************************************
*
*  Item : BAT_STRUCT
*
* Usage : Contains the Bouquet Association Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  bouquet_association_section(){
*     table_id                            8       uimsbf
*     section_syntax_indicator  = 1       1       bslbf
*     DVB_reserved                        1       bslbf
*     ISO_reserved                        2       bslbf
*     section_length                      12      uimsbf
*     bouquet_id                          16      uimsbf
*     ISO_reserved                        2       bslbf
*     version_number                      5       uimsbf
*     current_next_indicator              1       bslbf
*     section_number                      8       uimsbf
*     last_section_number                 8       uimsbf
*     DVB_reserved                        4       bslbf
*     descriptors_length                  12      uimsbf
*     for ( i=0; i<n; i++){
*        descriptor()
*     }
*     DVB_reserved                        4       bslbf
*     transport_stream_loop_length        12      uimsbf
*     for(i=0;i<N;i++){
*        transport_stream_id              16      uimsbf
*        original_network_id              16      uimsbf
*        DVB_reserved                     4       bslbf
*        transport_descriptors_length     12      uimsbf
*        for(j=0;j<N;j++){
*           descriptor()
*        }
*     }
*     CRC_32                              32      rpchof
*  }
*
*  NOTE : The descriptors are chained in the order that they are found
*         in the raw MPEG data.
*
*****************************************************************************/
typedef struct
{
   MPEG_TABLE_IDS       enTableType;       /* the order of the first 5 fields */
   unsigned short                 uiBouquetID;       /* is important as it is used as   */
   unsigned char                ucVersionNumber;   /* a mask for all table structs    */
   unsigned char                ucSection;
   unsigned char                ucLastSection;
   CURR_NEXT            enCurrNext;
   unsigned short                 uiBDescrCount;
   DESCRIPTOR_STRUCT   *pstBDescr;
   unsigned short                 uiTSDescrCount;
   TRANS_STREAM_DESCR  *pstTSDescriptor;
}  BAT_STRUCT;


/* skott Add 2005.9.14  start */
/*****************************************************************************
*
*  Item : STREAM_DESCR
*         CAT_STRUCT
*
* Usage : Contains the BIFS Command Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  BIFS_section() {
*     table_id                            8       uimsbf
*     section_syntax_indicator            1       bslbf
*     private_indicator                   1       bslbf
*     reserved                            2       bslbf
*     section_length                      12      uimsbf
*     table_id_extension                  16      uimsbf
*     reserved                            2       bslbf
*     version_number                      5       uimsbf
*     current_next_indicator              1       bslbf
*     section_number                      8       uimsbf
*     last_section_number                 8       uimsbf
*     for (i=0; i<N; i++) {
*             descriptor()
*     }
*     CRC_32                              32      rpchof
*  }
*
*  NOTE : The descriptors are chained in the order that they are found
*         in the raw MPEG data.
*
*****************************************************************************/
typedef struct
{
	MPEG_TABLE_IDS			enTableType;      /* the order of the first 5 fields */
	unsigned short			uiTableIdExt;
	unsigned char				ucVersionNumber;   /* a mask for all table structs    */
	unsigned char				ucSection;
	unsigned char				ucLastSection;
	CURR_NEXT				enCurrNext;
	unsigned short			uiObjDescrCount;
	DESCRIPTOR_STRUCT   	*pstObjectDescr;
}  CAT_STRUCT;

/*****************************************************************************
*
*  Item : STREAM_DESCR
*         BIFS_STRUCT
*
* Usage : Contains the BIFS Command Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  BIFS_section() {
*     table_id                            8       uimsbf
*     section_syntax_indicator            1       bslbf
*     private_indicator                   1       bslbf
*     reserved                            2       bslbf
*     section_length                      12      uimsbf
*     table_id_extension                  16      uimsbf
*     reserved                            2       bslbf
*     version_number                      5       uimsbf
*     current_next_indicator              1       bslbf
*     section_number                      8       uimsbf
*     last_section_number                 8       uimsbf
*     for (i=0; i<N; i++) {
*             descriptor()
*     }
*     CRC_32                              32      rpchof
*  }
*
*  NOTE : The descriptors are chained in the order that they are found
*         in the raw MPEG data.
*
*****************************************************************************/
typedef struct
{
	MPEG_TABLE_IDS			enTableType;      /* the order of the first 5 fields */
	unsigned short			uiTableIdExt;
	unsigned char			ucVersionNumber;   /* a mask for all table structs    */
	unsigned char			ucSection;
	unsigned char			ucLastSection;
	CURR_NEXT				enCurrNext;
	unsigned short			uiObjDescrCount;
	DESCRIPTOR_STRUCT   	*pstObjectDescr;
}  BIFS_STRUCT;

/*****************************************************************************
*
*  Item : STREAM_DESCR
*         ODT_STRUCT
*
* Usage : Contains the BIFS Command Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  Object_description_section() {
*     table_id                            8       uimsbf
*     section_syntax_indicator            1       bslbf
*     private_indicator                   1       bslbf
*     reserved                            2       bslbf
*     section_length                      12      uimsbf
*     table_id_extension                  16      uimsbf
*     reserved                            2       bslbf
*     version_number                      5       uimsbf
*     current_next_indicator              1       bslbf
*     section_number                      8       uimsbf
*     last_section_number                 8       uimsbf
*     for (i=0; i<N; i++) {
*             descriptor()
*     }
*     CRC_32                              32      rpchof
*  }
*
*  NOTE : The descriptors are chained in the order that they are found
*         in the raw MPEG data.
*
*****************************************************************************/
typedef struct
{
	MPEG_TABLE_IDS			enTableType;      /* the order of the first 5 fields */
	unsigned short			uiTableIdExt;
	unsigned char			ucVersionNumber;   /* a mask for all table structs    */
	unsigned char			ucSection;
	unsigned char			ucLastSection;
	CURR_NEXT				enCurrNext;
	unsigned short			uiObjDescrCount;
	DESCRIPTOR_STRUCT		*pstObjectDescr;
}  ODT_STRUCT;

/* skott Add 2005.9.14  end */



/**********************************************************
**                                                       **
** Types used by Delivery System Structure               **
**                                                       **
**********************************************************/
typedef enum
{
   ALL_SERVICES_ALLOWED,
   NO_SERVICES_ALLOWED,
   CLEAR_SERVICES_ALLOWED
}  MUX_ACCESS;

/* make a proper typedef */
typedef union
{
   SATELLITE_DS_DESCR   stSat;
   CABLE_DS_DESCR       stCab;
   TERRESTRIAL_DS_DESCR stTer;
}  DS_DESCR;

/*****************************************************************************
*
*  Item : DELIVERY_SYSTEM_STRUCT
*
* Usage : Contains the generic structure for a Delivery System Descriptor,
          identified by the DS Type ID
*
*****************************************************************************/

typedef struct
{
   DESCRIPTOR_IDS     enType;   /* at present ONLY SATELLITE_DS_DESCR_ID    &
                                                   CABLE_DS_DESCR_ID        &
                                                   TERRESTRIAL_DS_DESCR_ID  allowed */
   DS_DESCR           stDescr;  /* at present ONLY SATELLITE_DS_DESCR    &
                                                   CABLE_DS_DESCR_ID     &
                                                   TERRESTRIAL_DS_DESCR  allowed */
}  DELIVERY_SYSTEM_STRUCT;

/**********************************************************
**                                                       **
**  The following is used to store the above structure   **
**  efficiently in EEPROM                                **
**                                                       **
**  Special Routines Support the changing to real DS     **
**                                                       **
**********************************************************/
#define DS_EE_LENGTH 12

typedef struct
{
   unsigned char  ucData[DS_EE_LENGTH];
}  DS_EE_DESCR;

/**********************************************************
**                                                       **
** Type used for Delivery System ID in EEPROM structure  **
**                                                       **
**********************************************************/
typedef enum
{
   EE_SATELLITE_ID,
   EE_CABLE_ID,
   EE_TERRESTRIAL_ID
} DS_EE_TYPE;

/*****************************************************************************
*
*  Item : STREAM_DESCR
*         CAPMT_STRUCT
*
*
*                             CAS Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  ca_pmt() {
*     ca_pmt_tag                          24      uimsbf
*     length_field()                      
*     ca_pmt_list_management              8       uimsbf
*     program_number                      16      uimsbf
*     reserved                            2       bslbf
*     version_number                      5       uimsbf
*     current_next_indicator              1       bslbf
*     reserved                            4       bslbf
*     program_info_length                 12      uimsbf
*     if(program_info_length != 0) {
*        ca_pmt_cmd_id				      8       uimsbf 
*        for (i=0; i<N; i++) {	                        
*            descriptor()
*        }
*     }
*     for (i=0;i<P;i++) {
*        stream_type                      8       uimsbf
*        reserved                         3       bslbf
*        elementary_PID                   13      uimsnf
*        reserved                         4       bslbf
*        ES_info_length                   12      uimsbf
*        if(Es_info_length != 0) {
*            ca_pmt_cmd_id                8       uimsbf 
*            for (i=0; i<Q; i++) {
*                descriptor()
*            }
*        }
*     }
*  }
*
*  NOTE : The descriptors are chained in the order that they are found
*         in the raw MPEG data.
*
*****************************************************************************/
/* typedef enum 
{
	CAMORE,
	CAFIRST,
	CALAST,
	CAONLY,
	CAADD,
	CAUPDATE    		
}   CA_PMT_LIST;

typedef enum 
{
	NON_DESCRAM,
	OK_DESCRAM,
	OK_MMI,
	QUERY,
	NOT_SELECT,
	RFU
}   CA_PMT_CMD_ID;	
*/
typedef enum
{
	NOLEVEL,
	PROGLEVEL,
	ESLEVEL
}   CA_LEVEL;   	
   	
#endif /* _MPEG1_H_ */
