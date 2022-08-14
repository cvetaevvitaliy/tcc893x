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

#ifndef _MPEG2_H_
#define _MPEG2_H_

//#include "mpeg_def.h"

/*****************************************************************************
*
*  Item : SDT_STRUCT
*
* Usage : Contains the Service Descriptor Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  service_description_section(){
*     table_id                            8       uimsbf
*     section_syntax_indicator  = 1       1       bslbf
*     DVB_reserved                        1       bslbf
*     ISO_reserved                        2       bslbf
*     section_length                      12      uimsbf
*     transport_stream_id                 16      uimsbf
*     ISO_reserved                        2       bslbf
*     version_number                      5       uimsbf
*     current_next_indicator              1       bslbf
*     section_number                      8       uimsbf
*     last_section_number                 8       uimsbf
*     original_network_id                 16      uimsbf
*     DVB_reserved                        8       uimsbf
*     for (i=0;i<N;i++){
*        service_id                       16      uimsbf
*        reserved_future_use              6       uimsbf
*        EIT_schedule_flag                1       bslbf
*        EIT_present_following_flag       1       bslbf
*        running_status                   3       uimsbf
*        free_CA_mode                     1       bslbf
*        descriptors_loop_length          12      uimsbf
*        for ( j = 0; j<N; j++){
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
typedef struct _sdt_service_data
{
   unsigned short			uiServiceID;
   unsigned char			fEIT_Schedule;
   unsigned char			fEIT_PresentFollowing;
   
   RUNNING_STATUS			enRunningStatus;
   unsigned char			fCA_Mode_free;
   unsigned short			uiSvcDescrCount;
   DESCRIPTOR_STRUCT		*pstSvcDescr;
   struct _sdt_service_data	*pstServiceData;
}  SDT_SERVICE_DATA;

typedef struct
{
   MPEG_TABLE_IDS	enTableType;		/* the order of the first 5 fields */
   unsigned short	uiTS_ID;			/* is important as it is used as   */
   unsigned char	ucVersionNumber;	/* a mask for all table structs    */   
   unsigned char	ucSection;
   unsigned char	ucLastSection;
   CURR_NEXT		enCurrNext;
   unsigned short	uiOrigNetID;
   
   SDT_SERVICE_DATA    *pstServiceData;
}  SDT_STRUCT;

/*****************************************************************************
*
*  Item : EIT_STRUCT
*
* Usage : Contains the Event Information Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  service_description_section(){
*     table_id                            8       uimsbf
*     section_syntax_indicator  = 1       1       bslbf
*     DVB_reserved                        1       bslbf
*     ISO_reserved                        2       bslbf
*     section_length                      12      uimsbf
*     service_id                          16      uimsbf
*     ISO_reserved                        2       bslbf
*     version_number                      5       uimsbf
*     current_next_indicator              1       bslbf
*     section_number                      8       uimsbf
*     last_section_number                 8       uimsbf
*     transport_stream_id                 16      uimsbf
*     original_network_id                 16      uimsbf
*     segment_last_section_number         8       uimsbf
*     last_table_id                       8       uimsbf
*     for (i=0;i<N;i++){
*        event_id                         16      uimsbf
*        start_time                       40      bslbf
*        duration                         24      uimsbf
*        running_status                   3       uimsbf
*        free_CA_mode                     1       bslbf
*        descriptors_loop_length          12      uimsbf
*        for ( j = 0; j<N; j++){
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
typedef struct _eit_event_data
{
	unsigned short			uiEventID;
	DATE_TIME_STRUCT		stStartTime;
	TIME_STRUCT			stDuration;
	RUNNING_STATUS			enRunningStatus;
	unsigned char			fCA_Mode_free;
	unsigned short			wEvtDescrCount;
	DESCRIPTOR_STRUCT		*pstEvtDescr;
	struct _eit_event_data	*pstEventData;
	
}  EIT_EVENT_DATA, *P_EIT_EVENT_DATA;

typedef struct
{
	MPEG_TABLE_IDS	enTableType;          /* the order of the first 5 fields */
	unsigned short	uiService_ID;         /* is important as it is used as   */
	unsigned char	ucVersionNumber;      /* a mask for all table structs    */
	unsigned char	ucSection;
	unsigned char	ucLastSection;
	CURR_NEXT	enCurrNext;
	unsigned short	uiTStreamID;
	unsigned short	uiOrgNetWorkID;
	unsigned char	ucSegmentLastSection; 
	MPEG_TABLE_IDS	enLastTableType;
	EIT_EVENT_DATA	*pstEventData;
	
}  EIT_STRUCT, *P_EIT_STRUCT;

/*****************************************************************************
*
*  Item : TDT_STRUCT
*
* Usage : Contains the Time and Date Information Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  time_date_section(){
*     table_id                            8       uimsbf
*     section_syntax_indicator  = 0       1       bslbf
*     reserved_future_use                 1       bslbf
*     reserved                            2       bslbf
*     section_length                      12      uimsbf
*     UTC_time                            40      bslbf
*  }
*
*
*****************************************************************************/
typedef struct
{
   MPEG_TABLE_IDS    enTableType;
   DATE_TIME_STRUCT  stDateTime;
}  TDT_STRUCT;

/*****************************************************************************
*
*  Item : TOT_STRUCT
*
* Usage : Contains the Time Offset Information Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  time_offset_section(){
*     table_id                            		8	uimsbf
*     section_syntax_indicator  = 0       1	bslbf
*     reserved_future_use                 	1	bslbf
*     reserved                            		2	bslbf
*     section_length                      	12	uimsbf
*     UTC_time                            		40	bslbf
*	reserved						4	bslbf
*	descriptors_loop_length			12	uimsbf
*	for(i=0;i<N;i++) {
*		descriptor()
*	}
*	CRC_32						32	rpchof
*  }
*
*
*****************************************************************************/
typedef struct
{
	MPEG_TABLE_IDS			enTableType;
	DATE_TIME_STRUCT		stDateTime;
	unsigned char				ucTotDescrCnt;
	DESCRIPTOR_STRUCT		*pstTotDescr;
}  TOT_STRUCT;

/*****************************************************************************
*
*  Item : RST_STRUCT
*
* Usage : Contains the Running Status Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  running_status_section(){
*     table_id                            8       uimsbf
*     section_syntax_indicator  = 0       1       bslbf
*     DVB_reserved                        1       bslbf
*     ISO_reserved                        2       bslbf
*     section_length                      12      uimsbf
*     for (i=0;i<N;i++){
*        transport_stream_id              16      uimsbf
*        original_network_id              16      uimsbf
*        service_id                       16      uimsbf
*        event_id                         16      uimsbf
*        DVB_reserved                     8       uimsbf
*        running_status                   3       uimsbf
*        }
*  }
*
*****************************************************************************/
typedef struct _rst_data
{
   unsigned short              uiTStreamID;
   unsigned short              uiOrgNetworkID;
   unsigned short              uiService_ID;
   unsigned short              uiEventID;
   RUNNING_STATUS    enRunningStatus;
   struct _rst_data *pstRSTMoreData;
}  RST_DATA;

typedef struct
{
   MPEG_TABLE_IDS    enTableType;
   RST_DATA         *pstRSTData;
}  RST_STRUCT;

typedef struct
{
	MPEG_TABLE_IDS    enTableType;
	unsigned short		uiDataLen;
	unsigned char 	*pstECMData;
}  ECM_STRUCT;

/*****************************************************************************
*
*  Item : BIT_BROADCASTER_DATA
*
* Usage : Contains the Broadcaster descriptor(s) for the second loop in BIT 
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  		No. of bits  Mnemonic 
*     for(i=0;i<N;i++){
*        broadcaster_id			 	8      uimsbf 
*        ISDB_reserved                     	4       bslbf
*        broadcaster_descriptors_length     12      uimsbf
*        for(j=0;j<N;j++){
*           descriptor()
*        }
*     }
*
*****************************************************************************/
typedef struct _bit_broadcaster_data
{
	unsigned short				uiBCDescrCount;
	DESCRIPTOR_STRUCT			*pstBCDescr;
	struct _bit_broadcaster_data	*pstBroadcasterData;
	
}  BIT_BROADCASTER_DATA, *P_BIT_BROADCASTER_DATA;

/*****************************************************************************
*
*  Item : BIT_STRUCT
*
* Usage : Contains the Broadcaster Information Table
*
*
*                             MPEG Definition
*                             ===============
*
*           Syntax                  No. of bits  Mnemonic
*  broadcaster_information_section(){
*     table_id						8       uimsbf
*     section_syntax_indicator		1       bslbf
*     ISDB_reserved				1       bslbf
*     ISO_reserved					2       bslbf
*     section_length					12      uimsbf
*     original_network_id				16      uimsbf
*     ISO_reserved                       	 2       bslbf
*     version_number                      	5       uimsbf
*     current_next_indicator              	1       bslbf
*     section_number                      	8       uimsbf
*     last_section_number                 	8       uimsbf
*     ISDB_reserved                        	3       uimsbf
*	braodcast_view_propriety		1	bslbf
*     first_descriptors_length          	12      uimsbf
*     for(i=0;i<N1;i++){
*        descriptor()
*     }
*     for(i=0;i<N2;i++){
*        broadcaster_id              		8      uimsbf
*        ISDB_reserved                     	4       bslbf
*        broadcaster_descriptors_length     12      uimsbf
*        for(j=0;j<N;j++){
*           descriptor()
*        }
*     }
*     CRC_32                              32      rpchof
*  }
*
*
*****************************************************************************/
typedef struct
{
   MPEG_TABLE_IDS			enTableType;      	/* the order of the first 5 fields */
   unsigned short				uiOrigNetID;       
   unsigned char				ucVersionNumber;   	/* a mask for all table structs    */
   CURR_NEXT 				enCurrNext;
   unsigned char				ucSection;
   unsigned char				ucLastSection;
   unsigned char				ucBroadcastViewPropriety;      
   unsigned short				uiFDescrCount;
   DESCRIPTOR_STRUCT		*pstFDescr;
   BIT_BROADCASTER_DATA		*pstBroadcasterData;
}  BIT_STRUCT, * P_BIT_STRUCT;

/*
  * CDT - ISDB-T Logo
  */
typedef struct {
	unsigned short			logo_type;
	unsigned short			logo_id;
	unsigned short			logo_version;
	unsigned short			data_size_length;
	unsigned char 			*data_byte;
} CDT_LOGO_DATA, *P_CDT_LOGO_DATA;
typedef struct {
	MPEG_TABLE_IDS			enTableType;
	unsigned short			download_data_id;
	unsigned char				ucVersionNumber;
	CURR_NEXT 				enCurrNext;
	unsigned char				ucSection;
	unsigned char				ucLastSection;
	unsigned short			original_network_id;
	unsigned char				data_type;
	CDT_LOGO_DATA			stLogoData;
} CDT_STRUCT, *P_CDT_STRUCT;
#endif        /* _MPEG2_H_ */
