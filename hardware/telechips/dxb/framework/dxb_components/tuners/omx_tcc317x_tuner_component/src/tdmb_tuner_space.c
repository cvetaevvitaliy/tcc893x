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

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/
#define LOG_TAG	"TUNER_TCC317X"
#include <utils/Log.h>
#include "tdmb_tuner_space.h"

TdmbTunerSpace gstTunerSpace;
static TuneSpace s_spaces[]=
{
	//---------------- Korea Band ----------------
	//name/countrycode(band)/space(0)/minChannel/maxChannel/numbers/
	{"Korea", KOREA_BAND,   0, 0, 20, 21, 
		{
			175280,177008,178736,
			181280,183008,184736,
			187280,189008,190736,
			193280,195008,196736,
			199280,201008,202736,
			205280,207008,208736,
			211280,213008,214736
		}
	},

	//---------------- Korea Only Seoul Band ----------------
	//name/countrycode(band)/space(0)/minChannel/maxChannel/numbers/
	{"Korea Seoul", KOREA_BAND_ONLYSEOUL,   0, 0, 6, 7, 
		{
			181280,183008,184736,
			205280,207008,208736,
			211280
		}
	},
	//---------------- Band III ----------------
	//name/countrycode(band)/space(0)/minChannel/maxChannel/numbers/
	{"BAND III", BAND_III,   0, 0, 40, 41, 
		{
			174928, 176640, 178352, 180064,     // 5A~ 5D
			181936, 183648, 185360, 187072,     // 6A~ 6D
			188928, 190640, 192352, 194064,     // 7A~ 7D
			195936, 197648, 199360, 201072,     // 8A~ 8D
			202928, 204640, 206352, 208064,     // 9A~ 9D
			209936, 211648, 213360, 215072, 210096,     //10A~10D, 10N
			216928, 218640, 220352, 222064, 217088,     //11A~11D, 11N
			223936, 225648, 227360, 229072, 224096,     //12A~12D, 12N
			230784, 232496, 234208, 235776, 237488, 239200,     //13A~13F
		}
	}
};


void TDMBSPACE_CTuneSpace(int countrycode)
{		
	TDMBSPACE_SetCountryCode(countrycode);
}

int TDMBSPACE_GetMinChannel(void)
{
	return gstTunerSpace.CurrentSpace.minChannel;
}

int TDMBSPACE_GetMaxChannel(void)
{
	return gstTunerSpace.CurrentSpace.maxChannel;
}

void TDMBSPACE_SetCountryCode(unsigned long countryCode)
{
	int i;
	const int cCountries = sizeof(s_spaces) / sizeof(s_spaces[0]);	
	for (i = 0; i < cCountries; i++) 
	{		
		if (countryCode == s_spaces[i].countryCode)
			break;
	}

	if(i==cCountries)
	{
		countryCode = KOREA_BAND;
	}
	gstTunerSpace.CountryCode = countryCode;
	memcpy(&gstTunerSpace.CurrentSpace, &s_spaces[i], sizeof(TuneSpace));	
}

int TDMBSPACE_GetFrequency(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < gstTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &gstTunerSpace.CurrentSpace.channels[chIndex];
		return p_ch->frequency;
	}
	return 0;
}

