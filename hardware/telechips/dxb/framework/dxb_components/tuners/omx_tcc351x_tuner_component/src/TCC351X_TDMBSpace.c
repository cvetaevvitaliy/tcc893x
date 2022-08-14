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
#include "TCC351X_TDMBSpace.h"
#define	 PRINTF

static TdmbTunerSpace gstTunerSpace;
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
};


void TDMBSPACE_CTuneSpace(int countrycode)
{
		
	TDMBSPACE_SetCountryCode(countrycode);
}

int	TDMBSPACE_GetMinChannel(void)
{
	return gstTunerSpace.CurrentSpace.minChannel;
}

int	TDMBSPACE_GetMaxChannel(void)
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
	PRINTF("[TUNER]Setting INFO : Band(%s)code(%d)\n",gstTunerSpace.CurrentSpace.name, gstTunerSpace.CurrentSpace.countryCode);
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

