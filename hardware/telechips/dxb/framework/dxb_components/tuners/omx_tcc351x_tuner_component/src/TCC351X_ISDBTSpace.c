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
#include "TCC351X_ISDBTSpace.h"

static IsdbtTunerSpace gstTunerSpace;
static TuneSpace s_spaces[]=
{
	//---------------- Japan Terrestrial ----------------
	//      name tel type bw min_ch max_ch         finetunes  numof_channels
	{"japan", 0,   0, 6000,     0,    39, {0, 0, 0, 0},            40, 
	{
		{ "13", 473143, 0, 0, 6000, 6000, 0 },
		{ "14", 479143, 0, 0, 6000, 6000, 0 },
		{ "15", 485143, 0, 0, 6000, 6000, 0 },
		{ "16", 491143, 0, 0, 6000, 6000, 0 },
		{ "17", 497143, 0, 0, 6000, 6000, 0 },
		{ "18", 503143, 0, 0, 6000, 6000, 0 },
		{ "19", 509143, 0, 0, 6000, 6000, 0 },
		{ "20", 515143, 0, 0, 6000, 6000, 0 },
		{ "21", 521143, 0, 0, 6000, 6000, 0 },
		{ "22", 527143, 0, 0, 6000, 6000, 0 },
		{ "23", 533143, 0, 0, 6000, 6000, 0 },
		{ "24", 539143, 0, 0, 6000, 6000, 0 },
		{ "25", 545143, 0, 0, 6000, 6000, 0 },
		{ "26", 551143, 0, 0, 6000, 6000, 0 },
		{ "27", 557143, 0, 0, 6000, 6000, 0 },
		{ "28", 563143, 0, 0, 6000, 6000, 0 },
		{ "29", 569143, 0, 0, 6000, 6000, 0 },
		{ "30", 575143, 0, 0, 6000, 6000, 0 },
		{ "31", 581143, 0, 0, 6000, 6000, 0 },
		{ "32", 587143, 0, 0, 6000, 6000, 0 },
		{ "33", 593143, 0, 0, 6000, 6000, 0 },
		{ "34", 599143, 0, 0, 6000, 6000, 0 },
		{ "35", 605143, 0, 0, 6000, 6000, 0 },
		{ "36", 611143, 0, 0, 6000, 6000, 0 },
		{ "37", 617143, 0, 0, 6000, 6000, 0 },
		{ "38", 623143, 0, 0, 6000, 6000, 0 },
		{ "39", 629143, 0, 0, 6000, 6000, 0 },
		{ "40", 635143, 0, 0, 6000, 6000, 0 },
		{ "41", 641143, 0, 0, 6000, 6000, 0 },
		{ "42", 647143, 0, 0, 6000, 6000, 0 },
		{ "43", 653143, 0, 0, 6000, 6000, 0 },
		{ "44", 659143, 0, 0, 6000, 6000, 0 },
		{ "45", 665143, 0, 0, 6000, 6000, 0 },
		{ "46", 671143, 0, 0, 6000, 6000, 0 },
		{ "47", 677143, 0, 0, 6000, 6000, 0 },
		{ "48", 683143, 0, 0, 6000, 6000, 0 },
		{ "49", 689143, 0, 0, 6000, 6000, 0 },
		{ "50", 695143, 0, 0, 6000, 6000, 0 },
		{ "51", 701143, 0, 0, 6000, 6000, 0 },
		{ "52", 707143, 0, 0, 6000, 6000, 0 },
	#if 0 //delete 52 ~ 62. because of frequency repacking by Jul 2012
		{ "53", 713143, 0, 0, 6000, 6000, 0 },
		{ "54", 719143, 0, 0, 6000, 6000, 0 },
		{ "55", 725143, 0, 0, 6000, 6000, 0 },
		{ "56", 731143, 0, 0, 6000, 6000, 0 },
		{ "57", 737143, 0, 0, 6000, 6000, 0 },
		{ "58", 743143, 0, 0, 6000, 6000, 0 },
		{ "59", 749143, 0, 0, 6000, 6000, 0 },
		{ "60", 755143, 0, 0, 6000, 6000, 0 },
		{ "61", 761143, 0, 0, 6000, 6000, 0 },
		{ "62", 767143, 0, 0, 6000, 6000, 0 },
	#endif
	}
	},
		
	//---------------- Brazil Terrestrial ----------------
	// name    tel, type, bw, min_ch, max_ch, finetunes,             numof_channels
	{ "Brazil", 1,  0,    6000,  0,      55,     { 0, 0, 0, 0 }, 56,
	{
		{ "14", 473143, 0, 0, 6000, 6000, 0 },
		{ "15", 479143, 0, 0, 6000, 6000, 0 },
		{ "16", 485143, 0, 0, 6000, 6000, 0 },
		{ "17", 491143, 0, 0, 6000, 6000, 0 },
		{ "18", 497143, 0, 0, 6000, 6000, 0 },
		{ "19", 503143, 0, 0, 6000, 6000, 0 },
		{ "20", 509143, 0, 0, 6000, 6000, 0 },
		{ "21", 515143, 0, 0, 6000, 6000, 0 },
		{ "22", 521143, 0, 0, 6000, 6000, 0 },
		{ "23", 527143, 0, 0, 6000, 6000, 0 },
		{ "24", 533143, 0, 0, 6000, 6000, 0 },
		{ "25", 539143, 0, 0, 6000, 6000, 0 },
		{ "26", 545143, 0, 0, 6000, 6000, 0 },
		{ "27", 551143, 0, 0, 6000, 6000, 0 },
		{ "28", 557143, 0, 0, 6000, 6000, 0 },
		{ "29", 563143, 0, 0, 6000, 6000, 0 },
		{ "30", 569143, 0, 0, 6000, 6000, 0 },
		{ "31", 575143, 0, 0, 6000, 6000, 0 },
		{ "32", 581143, 0, 0, 6000, 6000, 0 },
		{ "33", 587143, 0, 0, 6000, 6000, 0 },
		{ "34", 593143, 0, 0, 6000, 6000, 0 },
		{ "35", 599143, 0, 0, 6000, 6000, 0 },
		{ "36", 605143, 0, 0, 6000, 6000, 0 },
		{ "37", 611143, 0, 0, 6000, 6000, 0 },
		{ "38", 617143, 0, 0, 6000, 6000, 0 },
		{ "39", 623143, 0, 0, 6000, 6000, 0 },
		{ "40", 629143, 0, 0, 6000, 6000, 0 },
		{ "41", 635143, 0, 0, 6000, 6000, 0 },
		{ "42", 641143, 0, 0, 6000, 6000, 0 },
		{ "43", 647143, 0, 0, 6000, 6000, 0 },
		{ "44", 653143, 0, 0, 6000, 6000, 0 },
		{ "45", 659143, 0, 0, 6000, 6000, 0 },
		{ "46", 665143, 0, 0, 6000, 6000, 0 },
		{ "47", 671143, 0, 0, 6000, 6000, 0 },
		{ "48", 677143, 0, 0, 6000, 6000, 0 },
		{ "49", 683143, 0, 0, 6000, 6000, 0 },
		{ "50", 689143, 0, 0, 6000, 6000, 0 },
		{ "51", 695143, 0, 0, 6000, 6000, 0 },
		{ "52", 701143, 0, 0, 6000, 6000, 0 },
		{ "53", 707143, 0, 0, 6000, 6000, 0 },
		{ "54", 713143, 0, 0, 6000, 6000, 0 },
		{ "55", 719143, 0, 0, 6000, 6000, 0 },
		{ "56", 725143, 0, 0, 6000, 6000, 0 },
		{ "57", 731143, 0, 0, 6000, 6000, 0 },
		{ "58", 737143, 0, 0, 6000, 6000, 0 },
		{ "59", 743143, 0, 0, 6000, 6000, 0 },
		{ "60", 749143, 0, 0, 6000, 6000, 0 },
		{ "61", 755143, 0, 0, 6000, 6000, 0 },
		{ "62", 761143, 0, 0, 6000, 6000, 0 },
		{ "63", 767143, 0, 0, 6000, 6000, 0 },
		{ "64", 773143, 0, 0, 6000, 6000, 0 },
		{ "65", 779143, 0, 0, 6000, 6000, 0 },
		{ "66", 785143, 0, 0, 6000, 6000, 0 },
		{ "67", 791143, 0, 0, 6000, 6000, 0 },
		{ "68", 797143, 0, 0, 6000, 6000, 0 },
		{ "69", 803143, 0, 0, 6000, 6000, 0 },
	}
	}	
};

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
void ISDBTSPACE_SetSignalType(int signaltype)
{
	gstTunerSpace.SignalType = signaltype;
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
void ISDBTSPACE_SetBandwidth(int bandwidth)
{
	gstTunerSpace.DefaultBandWidth = bandwidth;
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
void ISDBTSPACE_CTuneSpace(int countrycode)
{
	ISDBTSPACE_SetSignalType(DEFAULT_SIGNAL_TYPE);
	//printf("countrycode = %d \n" , countrycode);
	if(countrycode==0)
		countrycode = DEFAULT_ISDBT_COUNTRY;
		
	ISDBTSPACE_SetCountryCode(/*DEFAULT_ISDBT_COUNTRY*/countrycode);
	ISDBTSPACE_SetBandwidth(DEFAULT_BANDWIDTH);
}


/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
int	ISDBTSPACE_GetMinChannel(void)
{
	return gstTunerSpace.CurrentSpace.minChannel;
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
int	ISDBTSPACE_GetMaxChannel(void)
{
	return gstTunerSpace.CurrentSpace.maxChannel;
}


/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
void ISDBTSPACE_SetCountryCode(unsigned long countryCode)
{
	int i;
	const int cCountries = sizeof(s_spaces) / sizeof(s_spaces[0]);
	gstTunerSpace.CountryCode = countryCode;
	for (i = 0; i < cCountries; i++) 
	{		
		if (gstTunerSpace.CountryCode == s_spaces[i].countryCode)
			break;
	}
	if ((gstTunerSpace.CountryCode == 82) || (gstTunerSpace.CountryCode == 1)) { // Korea and US

	}
	
	if (i >= cCountries) 
	{
		switch (gstTunerSpace.DefaultBandWidth) 
		{
			case 6:
				i = 0;
			break;
			case 7:
				i = 1;
			break;
			default:
				i = 2;
			break;
		}
	}

	memcpy(&gstTunerSpace.CurrentSpace, &s_spaces[i], sizeof(TuneSpace));
	//printf("[TUNER]Setting INFO : Contry(%s)Code(%d)BW(%d)\n",gstTunerSpace.CurrentSpace.name, gstTunerSpace.CurrentSpace.countryCode, gstTunerSpace.CurrentSpace.typicalBandwidth );
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  INPUT   :
*  OUTPUT:	
**************************************************************************/
int ISDBTSPACE_GetFrequency(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < gstTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &gstTunerSpace.CurrentSpace.channels[chIndex];
		return p_ch->alt_selected ? p_ch->frequency2 : p_ch->frequency1;
	}
	return 0;
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  INPUT   :
*  OUTPUT:	
**************************************************************************/
int ISDBTSPACE_GetFinetune(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < gstTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &gstTunerSpace.CurrentSpace.channels[chIndex];
		return p_ch->finetune;
	}
	return 0;
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  INPUT   :
*  OUTPUT:	
**************************************************************************/
int ISDBTSPACE_GetBandwidth(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < gstTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &gstTunerSpace.CurrentSpace.channels[chIndex];
		return p_ch->alt_selected ? p_ch->bandwidth2 : p_ch->bandwidth1;
	}
	return 0;
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  INPUT   :
*  OUTPUT:	
**************************************************************************/
OneChannel* ISDBTSPACE_GetOneChannel(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < gstTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &gstTunerSpace.CurrentSpace.channels[chIndex];
		return p_ch;
	}
	return NULL;
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  INPUT   :
*  OUTPUT:	
**************************************************************************/
void ISDBTSPACE_SetFinetune(unsigned long chIndex, long finetune)
{
	OneChannel* p_ch;

	if (chIndex < gstTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &gstTunerSpace.CurrentSpace.channels[chIndex];
		p_ch->finetune = finetune;
	}
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  INPUT   :chIndex : channel index ,  makeAlt : make alt or not
*  OUTPUT:	
**************************************************************************/
void ISDBTSPACE_SetAltSelected(unsigned long chIndex, int makeAlt)
{
	OneChannel* p_ch;

	if (chIndex < gstTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &gstTunerSpace.CurrentSpace.channels[chIndex];
		p_ch->alt_selected = makeAlt ? 1 : 0;
	}
}


/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  INPUT   :*pNumbers : number of pilot finetunes,   *pFinetunes : array of fintunes
*  OUTPUT:	true, if pilot finetunes exist / false, otherwise
**************************************************************************/
int ISDBTSPACE_GetPilotFinetunes(unsigned long* pNumbers, long* pFinetunes)
{
	unsigned long numbers = 0;
	unsigned int i;
	for (i = 0; i < 4; i++) 
	{
		if (gstTunerSpace.CurrentSpace.finetunes[i] != 0) 
		{
			numbers++;
			*pFinetunes++ = gstTunerSpace.CurrentSpace.finetunes[i];
		}
	}
	*pNumbers = numbers;
	return numbers > 0;
}


