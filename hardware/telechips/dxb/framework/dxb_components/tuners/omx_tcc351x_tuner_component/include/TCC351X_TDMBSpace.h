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


/*****************************************************************************
* define
******************************************************************************/

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

#define		KOREA_BAND				0x01
#define		BAND_III				0x02
#define 	L_BAND					0x03
#define		CANADA_BAND				0x04
#define		CHINA_BAND_III			0x05
#define		KOREA_BAND_ONLYSEOUL	0x06


/*****************************************************************************
* structures
******************************************************************************/
typedef struct
{
	int frequency; // primary frequency in Khz
}OneChannel;


typedef struct
{
	char name[32]; // name of the tune space
	unsigned long countryCode; // band information
	unsigned long space; // 0=terrestrial, 1=cable, 2=satellite
	unsigned long minChannel; // min channel
	unsigned long maxChannel; // max channel
	unsigned long numbers; // number of channels
	OneChannel channels[128];
}TuneSpace;

typedef struct
{
	unsigned int	 CountryCode;
	TuneSpace CurrentSpace;
}TdmbTunerSpace;


/*****************************************************************************
* Variables
******************************************************************************/

/******************************************************************************
* declarations
******************************************************************************/
void TDMBSPACE_CTuneSpace(int countrycode);
void TDMBSPACE_SetCountryCode(unsigned long countryCode);
int TDMBSPACE_GetFrequency(unsigned long chIndex);
#endif		//TUNER_SPACE_H

