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


#ifndef TUNER_SPACE_H
#define TUNER_SPACE_H


#ifndef TRUE
#define TRUE                1
#endif
	
#ifndef FALSE
#define FALSE               0
#endif

#ifndef NULL
#define NULL               0
#endif


// 61 = Australia  , 33= France 
//#define DEFAULT_DVB_COUNTRY	33 //France
#define DEFAULT_DVB_COUNTRY	9999 //UHF 8MHZ

#define DEFAULT_BANDWIDTH		8
#define DEFAULT_SIGNAL_TYPE   1     // air



/*****************************************************************************
* structures
******************************************************************************/
typedef struct
{
	char name[6];
	int frequency1; // primary frequency in Khz
	int frequency2; // alternative frequency
	int finetune; // finetune offset in Khz, 0=no finetune
	short bandwidth1; // 6/7/8 Mhz
	short bandwidth2; // alternative bandwidth
	short alt_selected; // alternative selected or not
}OneChannel;


typedef struct
{
	char name[32]; // name of the tune space
	unsigned long countryCode; // country code
	unsigned long space; // 0=terrestrial, 1=cable, 2=satellite
	unsigned long typicalBandwidth; // typical bandwidth
	unsigned long minChannel; // min channel
	unsigned long maxChannel; // max channel
	long finetunes[4]; // pilot finetunes
	unsigned long numbers; // number of channels
	OneChannel channels[128];
}TuneSpace;

typedef struct
{
	unsigned int	 SignalType;
	unsigned int	 DefaultBandWidth;
	unsigned int	 CountryCode;
	unsigned int     CurrentSpaceIndex;
}DvbtTunerSpace;


/*****************************************************************************
* Variables
******************************************************************************/

/******************************************************************************
* declarations
******************************************************************************/
void DVBTSPACE_SetTuneSpace(TuneSpace *tuneSpace);
void DVBTSPACE_SetSignalType(int signaltype);
void DVBTSPACE_SetBandwidth(int bandwidth);
void DVBTSPACE_CTuneSpace(int countrycode);
void DVBTSPACE_SetCountryCode(unsigned long countryCode);
int DVBTSPACE_GetFrequency(unsigned long chIndex);
int DVBTSPACE_GetIndexByFrequency(unsigned int uiFrequencyKhz);
int DVBTSPACE_GetFinetune(unsigned long chIndex);
int DVBTSPACE_GetBandwidth(unsigned long chIndex);
OneChannel* DVBTSPACE_GetOneChannel(unsigned long chIndex);
void DVBTSPACE_SetFinetune(unsigned long chIndex, long finetune);
void DVBTSPACE_SetAltSelected(unsigned long chIndex, int makeAlt);
int DVBTSPACE_GetPilotFinetunes(unsigned long* pNumbers, long* pFinetunes);
int DVBTSPACE_GetIndexByManualFrequency(int iFrequencyKhz, int iBandwidthMhz, int *piChannelIndex);


#endif		//TUNER_SPACE_H

