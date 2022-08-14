#ifndef _ISDBT_TUNER_TCC353X_H_
#define _ISDBT_TUNER_TCC353X_H_

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

/******************************************************************************
* include 
******************************************************************************/
#include "tcpal_types.h"
#include "tcc353x_monitoring.h"

/*****************************************************************************
* define
******************************************************************************/
#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

/*****************************************************************************
* structures
******************************************************************************/
typedef enum
{
	SEARCH_START = 0,
	SEARCH_CONTINUE,
	SEARCH_STOP
} E_SEARCH_CMD;

typedef struct
{
	I32U uiMinChannel;
	I32U uiMaxChannel;
	I32U uiCountryCode;
}TUNER_SEARCH_INFO;

typedef struct
{
	I32U Channel;
	I32U Freq;
	I32U Search_Stop_Flag;
}ISDBT_CHANNEL_INFO;

/*****************************************************************************
* Variables
******************************************************************************/
I32U    ISDBT_Tunertype;
ISDBT_CHANNEL_INFO *ISDBT_ChannelInfo;

/******************************************************************************
* declarations
******************************************************************************/
I32S tcc353x_init(I32S commandInterface, I32S streamInterface);
I32S tcc353x_deinit(I32S commandInterface, I32S streamInterface);
I32S tcc353x_search_channel(I32S searchmode, I32S channel ,I32S countrycode);
I32S tcc353x_channel_set(I32S channel);
I32S tcc353x_channel_tune(I32S channel);
I32S tcc353x_frequency_set(I32S iFreqKhz, I32S iBWKhz);
I32S tcc353x_get_signal_strength(Tcc353xStatus_t *_monitorValues);
I32S tcc353x_get_ews_flag (void);
I32S tcc353x_get_tmcc_information(tmccInfo_t *pTmccinfo);


#endif

