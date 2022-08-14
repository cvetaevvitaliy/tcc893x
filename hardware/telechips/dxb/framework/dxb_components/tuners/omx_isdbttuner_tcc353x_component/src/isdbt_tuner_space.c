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
#define LOG_TAG	"TUNER_TCC353X"
#include <utils/Log.h>

//#include "globals.h"
#include "isdbt_tuner_space.h"

#define TABLE_INDEX_JAPAN_UHF	0
#define	TABLE_INDEX_JAPAN_VHF	1
#define	TABLE_INDEX_JAPAN_MID	2
#define	TABLE_INDEX_JAPAN_SHB	3

#define	COUNTRY_NAME_JAPAN	"japan"

static FrequencyRange s_spaces_range[] =
{
	/* japan */
	{0, 4,
		{
			{  0,  39, 40},	//UHF
 			{ 40,  51, 12},	//VHF
			{ 52,  61, 10}, //MID
			{ 62, 102, 41}, //SHB
		}
	},
	/* brazil */
	{1, 2,
		{
			{ 0,  6,  7},	//VHF
			{ 7, 62, 56},	//UHF
			{ 0,  0,  0},
			{ 0,  0,  0},
		}
	},
};


static TuneSpace s_spaces[]=
{
	//---------------- Japan Terrestrial ----------------
	//      name tel type bw min_ch max_ch         finetunes  numof_channels
	{COUNTRY_NAME_JAPAN, 0,   0, 6000,     0,    102, {0, 0, 0, 0},            103, 
	{
		{ "13", 473143, 0, 0, 6000, 6000, 0, 1},	//UHF ch13 ~ ch62
		{ "14", 479143, 0, 0, 6000, 6000, 0, 1},
		{ "15", 485143, 0, 0, 6000, 6000, 0, 1},
		{ "16", 491143, 0, 0, 6000, 6000, 0, 1},
		{ "17", 497143, 0, 0, 6000, 6000, 0, 1},
		{ "18", 503143, 0, 0, 6000, 6000, 0, 1},
		{ "19", 509143, 0, 0, 6000, 6000, 0, 1},
		{ "20", 515143, 0, 0, 6000, 6000, 0, 1},
		{ "21", 521143, 0, 0, 6000, 6000, 0, 1},
		{ "22", 527143, 0, 0, 6000, 6000, 0, 1},
		{ "23", 533143, 0, 0, 6000, 6000, 0, 1},
		{ "24", 539143, 0, 0, 6000, 6000, 0, 1},
		{ "25", 545143, 0, 0, 6000, 6000, 0, 1},
		{ "26", 551143, 0, 0, 6000, 6000, 0, 1},
		{ "27", 557143, 0, 0, 6000, 6000, 0, 1},
		{ "28", 563143, 0, 0, 6000, 6000, 0, 1},
		{ "29", 569143, 0, 0, 6000, 6000, 0, 1},
		{ "30", 575143, 0, 0, 6000, 6000, 0, 1},
		{ "31", 581143, 0, 0, 6000, 6000, 0, 1},
		{ "32", 587143, 0, 0, 6000, 6000, 0, 1},
		{ "33", 593143, 0, 0, 6000, 6000, 0, 1},
		{ "34", 599143, 0, 0, 6000, 6000, 0, 1},
		{ "35", 605143, 0, 0, 6000, 6000, 0, 1},
		{ "36", 611143, 0, 0, 6000, 6000, 0, 1},
		{ "37", 617143, 0, 0, 6000, 6000, 0, 1},
		{ "38", 623143, 0, 0, 6000, 6000, 0, 1},
		{ "39", 629143, 0, 0, 6000, 6000, 0, 1},
		{ "40", 635143, 0, 0, 6000, 6000, 0, 1},
		{ "41", 641143, 0, 0, 6000, 6000, 0, 1},
		{ "42", 647143, 0, 0, 6000, 6000, 0, 1},
		{ "43", 653143, 0, 0, 6000, 6000, 0, 1},
		{ "44", 659143, 0, 0, 6000, 6000, 0, 1},
		{ "45", 665143, 0, 0, 6000, 6000, 0, 1},
		{ "46", 671143, 0, 0, 6000, 6000, 0, 1},
		{ "47", 677143, 0, 0, 6000, 6000, 0, 1},
		{ "48", 683143, 0, 0, 6000, 6000, 0, 1},
		{ "49", 689143, 0, 0, 6000, 6000, 0, 1},
		{ "50", 695143, 0, 0, 6000, 6000, 0, 1},
		{ "51", 701143, 0, 0, 6000, 6000, 0, 1},
		{ "52", 707143, 0, 0, 6000, 6000, 0, 1},
	#if 0 //delete 52 ~ 62. because of frequency repacking by Jul 2012
		{ "53", 713143, 0, 0, 6000, 6000, 0, 1},
		{ "54", 719143, 0, 0, 6000, 6000, 0, 1},
		{ "55", 725143, 0, 0, 6000, 6000, 0, 1},
		{ "56", 731143, 0, 0, 6000, 6000, 0, 1},
		{ "57", 737143, 0, 0, 6000, 6000, 0, 1},
		{ "58", 743143, 0, 0, 6000, 6000, 0, 1},
		{ "59", 749143, 0, 0, 6000, 6000, 0, 1},
		{ "60", 755143, 0, 0, 6000, 6000, 0, 1},
		{ "61", 761143, 0, 0, 6000, 6000, 0, 1},
		{ "62", 767143, 0, 0, 6000, 6000, 0, 1},
	#endif
		{  "1",  93143, 0, 0, 6000, 6000, 0, 0}, // VHF ch1 ~ ch12
		{  "2",  99143, 0, 0, 6000, 6000, 0, 0},
		{  "3", 105143, 0, 0, 6000, 6000, 0, 0},
		{  "4", 173143, 0, 0, 6000, 6000, 0, 0},
		{  "5", 179143, 0, 0, 6000, 6000, 0, 0},
		{  "6", 185143, 0, 0, 6000, 6000, 0, 0},
		{  "7", 191143, 0, 0, 6000, 6000, 0, 0},
		{  "8", 195143, 0, 0, 6000, 6000, 0, 0},
		{  "9", 201143, 0, 0, 6000, 6000, 0, 0},
		{ "10", 207143, 0, 0, 6000, 6000, 0, 0},
		{ "11", 213143, 0, 0, 6000, 6000, 0, 0},
		{ "12", 219143, 0, 0, 6000, 6000, 0, 0},
		{"C13", 111143, 0, 0, 6000, 6000, 0, 0}, // MID C13 ~ C22
		{"C14", 117143, 0, 0, 6000, 6000, 0, 0},
		{"C15", 123143, 0, 0, 6000, 6000, 0, 0},
		{"C16", 129143, 0, 0, 6000, 6000, 0, 0},
		{"C17", 135143, 0, 0, 6000, 6000, 0, 0},
		{"C18", 141143, 0, 0, 6000, 6000, 0, 0},
		{"C19", 147143, 0, 0, 6000, 6000, 0, 0},
		{"C20", 153143, 0, 0, 6000, 6000, 0, 0},
		{"C21", 159143, 0, 0, 6000, 6000, 0, 0},
		{"C22", 167143, 0, 0, 6000, 6000, 0, 0},
		{"C23", 225143, 0, 0, 6000, 6000, 0, 0}, //SHD C23 ~ C63
		{"C24", 231143, 0, 0, 6000, 6000, 0, 0},
		{"C25", 237143, 0, 0, 6000, 6000, 0, 0},
		{"C26", 243143, 0, 0, 6000, 6000, 0, 0},
		{"C27", 249143, 0, 0, 6000, 6000, 0, 0},
		{"C28", 255143, 0, 0, 6000, 6000, 0, 0},
		{"C29", 261143, 0, 0, 6000, 6000, 0, 0},
		{"C30", 267143, 0, 0, 6000, 6000, 0, 0},
		{"C31", 273143, 0, 0, 6000, 6000, 0, 0},
		{"C32", 279143, 0, 0, 6000, 6000, 0, 0},
		{"C33", 285143, 0, 0, 6000, 6000, 0, 0},
		{"C34", 291143, 0, 0, 6000, 6000, 0, 0},
		{"C35", 297143, 0, 0, 6000, 6000, 0, 0},
		{"C36", 303143, 0, 0, 6000, 6000, 0, 0},
		{"C37", 309143, 0, 0, 6000, 6000, 0, 0},
		{"C38", 315143, 0, 0, 6000, 6000, 0, 0},
		{"C39", 321143, 0, 0, 6000, 6000, 0, 0},
		{"C40", 327143, 0, 0, 6000, 6000, 0, 0},
		{"C41", 333143, 0, 0, 6000, 6000, 0, 0},
		{"C42", 339143, 0, 0, 6000, 6000, 0, 0},
		{"C43", 345143, 0, 0, 6000, 6000, 0, 0},
		{"C44", 351143, 0, 0, 6000, 6000, 0, 0},
		{"C45", 357143, 0, 0, 6000, 6000, 0, 0},
		{"C46", 363143, 0, 0, 6000, 6000, 0, 0},
		{"C47", 369143, 0, 0, 6000, 6000, 0, 0},
		{"C48", 375143, 0, 0, 6000, 6000, 0, 0},
		{"C49", 381143, 0, 0, 6000, 6000, 0, 0},
		{"C50", 387143, 0, 0, 6000, 6000, 0, 0},
		{"C51", 393143, 0, 0, 6000, 6000, 0, 0},
		{"C52", 399143, 0, 0, 6000, 6000, 0, 0},
		{"C53", 405143, 0, 0, 6000, 6000, 0, 0},
		{"C54", 411143, 0, 0, 6000, 6000, 0, 0},
		{"C55", 417143, 0, 0, 6000, 6000, 0, 0},
		{"C56", 423143, 0, 0, 6000, 6000, 0, 0},
		{"C57", 429143, 0, 0, 6000, 6000, 0, 0},
		{"C58", 435143, 0, 0, 6000, 6000, 0, 0},
		{"C59", 441143, 0, 0, 6000, 6000, 0, 0},
		{"C60", 447143, 0, 0, 6000, 6000, 0, 0},
		{"C61", 453143, 0, 0, 6000, 6000, 0, 0},
		{"C62", 459143, 0, 0, 6000, 6000, 0, 0},
		{"C63", 465143, 0, 0, 6000, 6000, 0, 0},	
	}
	},
		
	//---------------- Brazil Terrestrial ----------------
	// name    tel, type, bw, min_ch, max_ch, finetunes,             numof_channels
	{ "Brazil", 1,  0,    6000,  0,      62,     { 0, 0, 0, 0 }, 63,
	{
		{ "7",  177143, 0, 0, 6000, 6000, 0, 1},	//VHF band
		{ "8",  183143, 0, 0, 6000, 6000, 0, 1},
		{ "9",  189143, 0, 0, 6000, 6000, 0, 1},
		{ "10", 195143, 0, 0, 6000, 6000, 0, 1},
		{ "11", 201143, 0, 0, 6000, 6000, 0, 1},
		{ "12", 207143, 0, 0, 6000, 6000, 0, 1},
		{ "13", 213143, 0, 0, 6000, 6000, 0, 1},
		{ "14", 473143, 0, 0, 6000, 6000, 0, 1},	//UHF band
		{ "15", 479143, 0, 0, 6000, 6000, 0, 1},
		{ "16", 485143, 0, 0, 6000, 6000, 0, 1},
		{ "17", 491143, 0, 0, 6000, 6000, 0, 1},
		{ "18", 497143, 0, 0, 6000, 6000, 0, 1},
		{ "19", 503143, 0, 0, 6000, 6000, 0, 1},
		{ "20", 509143, 0, 0, 6000, 6000, 0, 1},
		{ "21", 515143, 0, 0, 6000, 6000, 0, 1},
		{ "22", 521143, 0, 0, 6000, 6000, 0, 1},
		{ "23", 527143, 0, 0, 6000, 6000, 0, 1},
		{ "24", 533143, 0, 0, 6000, 6000, 0, 1},
		{ "25", 539143, 0, 0, 6000, 6000, 0, 1},
		{ "26", 545143, 0, 0, 6000, 6000, 0, 1},
		{ "27", 551143, 0, 0, 6000, 6000, 0, 1},
		{ "28", 557143, 0, 0, 6000, 6000, 0, 1},
		{ "29", 563143, 0, 0, 6000, 6000, 0, 1},
		{ "30", 569143, 0, 0, 6000, 6000, 0, 1},
		{ "31", 575143, 0, 0, 6000, 6000, 0, 1},
		{ "32", 581143, 0, 0, 6000, 6000, 0, 1},
		{ "33", 587143, 0, 0, 6000, 6000, 0, 1},
		{ "34", 593143, 0, 0, 6000, 6000, 0, 1},
		{ "35", 599143, 0, 0, 6000, 6000, 0, 1},
		{ "36", 605143, 0, 0, 6000, 6000, 0, 1},
		{ "37", 611143, 0, 0, 6000, 6000, 0, 1},
		{ "38", 617143, 0, 0, 6000, 6000, 0, 1},
		{ "39", 623143, 0, 0, 6000, 6000, 0, 1},
		{ "40", 629143, 0, 0, 6000, 6000, 0, 1},
		{ "41", 635143, 0, 0, 6000, 6000, 0, 1},
		{ "42", 641143, 0, 0, 6000, 6000, 0, 1},
		{ "43", 647143, 0, 0, 6000, 6000, 0, 1},
		{ "44", 653143, 0, 0, 6000, 6000, 0, 1},
		{ "45", 659143, 0, 0, 6000, 6000, 0, 1},
		{ "46", 665143, 0, 0, 6000, 6000, 0, 1},
		{ "47", 671143, 0, 0, 6000, 6000, 0, 1},
		{ "48", 677143, 0, 0, 6000, 6000, 0, 1},
		{ "49", 683143, 0, 0, 6000, 6000, 0, 1},
		{ "50", 689143, 0, 0, 6000, 6000, 0, 1},
		{ "51", 695143, 0, 0, 6000, 6000, 0, 1},
		{ "52", 701143, 0, 0, 6000, 6000, 0, 1},
		{ "53", 707143, 0, 0, 6000, 6000, 0, 1},
		{ "54", 713143, 0, 0, 6000, 6000, 0, 1},
		{ "55", 719143, 0, 0, 6000, 6000, 0, 1},
		{ "56", 725143, 0, 0, 6000, 6000, 0, 1},
		{ "57", 731143, 0, 0, 6000, 6000, 0, 1},
		{ "58", 737143, 0, 0, 6000, 6000, 0, 1},
		{ "59", 743143, 0, 0, 6000, 6000, 0, 1},
		{ "60", 749143, 0, 0, 6000, 6000, 0, 1},
		{ "61", 755143, 0, 0, 6000, 6000, 0, 1},
		{ "62", 761143, 0, 0, 6000, 6000, 0, 1},
		{ "63", 767143, 0, 0, 6000, 6000, 0, 1},
		{ "64", 773143, 0, 0, 6000, 6000, 0, 1},
		{ "65", 779143, 0, 0, 6000, 6000, 0, 1},
		{ "66", 785143, 0, 0, 6000, 6000, 0, 1},
		{ "67", 791143, 0, 0, 6000, 6000, 0, 1},
		{ "68", 797143, 0, 0, 6000, 6000, 0, 1},
		{ "69", 803143, 0, 0, 6000, 6000, 0, 1},
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
void SetSignalType(int signaltype)
{
	ISDBTTunerSpace.SignalType = signaltype;
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
void SetBandwidth(int bandwidth)
{
	ISDBTTunerSpace.DefaultBandWidth = bandwidth;
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
void CTuneSpace(int countrycode)
{
	SetSignalType(DEFAULT_SIGNAL_TYPE);
	ALOGD("countrycode = %d \n" , countrycode);
	if(countrycode==0)
		countrycode = DEFAULT_ISDBT_COUNTRY;
		
	SetCountryCode(/*DEFAULT_ISDBT_COUNTRY*/countrycode);
	SetBandwidth(DEFAULT_BANDWIDTH);
}


/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
int	GetMinChannel(void)
{
	return ISDBTTunerSpace.CurrentSpace.minChannel;
}

/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
int	GetMaxChannel(void)
{
	return ISDBTTunerSpace.CurrentSpace.maxChannel;
}


/**************************************************************************
*  FUNCTION NAME : 
*    int	ISDBT_Init_Tuner_Vritual(void)
*  
*  DESCRIPTION : 
*  OUTPUT:	
**************************************************************************/
void SetCountryCode(unsigned long countryCode)
{
	int i;
	const int cCountries = sizeof(s_spaces) / sizeof(s_spaces[0]);
	ISDBTTunerSpace.CountryCode = countryCode;
	for (i = 0; i < cCountries; i++) 
	{		
		if (ISDBTTunerSpace.CountryCode == s_spaces[i].countryCode)
			break;
	}
	
	if (i >= cCountries) 
	{
		switch (ISDBTTunerSpace.DefaultBandWidth) 
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

	memcpy(&ISDBTTunerSpace.CurrentSpace, &s_spaces[i], sizeof(TuneSpace));
//	ALOGD("[TUNER]Setting INFO : Contry(%s)Code(%d)BW(%d)\n",ISDBTTunerSpace.CurrentSpace.name, ISDBTTunerSpace.CurrentSpace.countryCode, ISDBTTunerSpace.CurrentSpace.typicalBandwidth );
}

/**************************************************************************
*  FUNCTION NAME : 
*    int GetFrequency(unsigned long chIndex)
*  
*  DESCRIPTION : 
*  INPUT   : chIndex - index for frequency table
*  OUTPUT:	return the selected frequency when it's valid
*			else return 0
**************************************************************************/
int GetFrequency(unsigned long chIndex)
{
	OneChannel* p_ch;
	int	frequency;

	if (chIndex < ISDBTTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &ISDBTTunerSpace.CurrentSpace.channels[chIndex];
		if (p_ch->alt_selected) frequency = p_ch->frequency2;
		else frequency = p_ch->frequency1;

		if (p_ch->flag == 0) frequency = 0;

		return frequency;
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
int GetFinetune(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < ISDBTTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &ISDBTTunerSpace.CurrentSpace.channels[chIndex];
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
int GetBandwidth(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < ISDBTTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &ISDBTTunerSpace.CurrentSpace.channels[chIndex];
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
OneChannel* GetOneChannel(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < ISDBTTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &ISDBTTunerSpace.CurrentSpace.channels[chIndex];
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
void SetFinetune(unsigned long chIndex, long finetune)
{
	OneChannel* p_ch;

	if (chIndex < ISDBTTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &ISDBTTunerSpace.CurrentSpace.channels[chIndex];
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
void SetAltSelected(unsigned long chIndex, int makeAlt)
{
	OneChannel* p_ch;

	if (chIndex < ISDBTTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &ISDBTTunerSpace.CurrentSpace.channels[chIndex];
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
int GetPilotFinetunes(unsigned long* pNumbers, long* pFinetunes)
{
	unsigned long numbers = 0;
	unsigned int i;
	for (i = 0; i < 4; i++) 
	{
		if (ISDBTTunerSpace.CurrentSpace.finetunes[i] != 0) 
		{
			numbers++;
			*pFinetunes++ = ISDBTTunerSpace.CurrentSpace.finetunes[i];
		}
	}
	*pNumbers = numbers;
	return numbers > 0;
}

int GetFrequencyValidity(unsigned long chIndex)
{
	OneChannel* p_ch;
	int rtn = 0;

	if (chIndex < ISDBTTunerSpace.CurrentSpace.numbers) 
	{
		p_ch = &ISDBTTunerSpace.CurrentSpace.channels[chIndex];
		if (p_ch->flag)
			rtn = 1;
	}
	return rtn;
}

int SetFreqBand_UpdateUsage (int on_off, int band)
{
	OneChannel	*p_ch;
	TableIndexInfo table_index;
	int	i;

	if (band < TABLE_INDEX_JAPAN_VHF || band > TABLE_INDEX_JAPAN_SHB)
		return 0;
	if (on_off)	on_off = 1;
	else	on_off = 0;

	table_index = s_spaces_range[0].freqtableinfo[band];
	table_index.maxChannel++;
	if (table_index.maxChannel > s_spaces[0].numbers)
		return 0;

	for (i=table_index.minChannel; i < table_index.maxChannel; i++) {
		p_ch = &s_spaces[0].channels[i];
		p_ch->flag = on_off;
	}

	return 1;
}

/*
  * int SetFreqBand (int freq_band)
  *	freq_band	bit0 - VHF
  *				bit1 - MID
  *				bit2 - SHB
  */
int SetFreqBand (int freq_band)
{
	unsigned long country_code;
	const int cCountries = sizeof(s_spaces) / sizeof(s_spaces[0]);

	SetFreqBand_UpdateUsage (freq_band & 1, TABLE_INDEX_JAPAN_VHF);
	SetFreqBand_UpdateUsage (freq_band & 2, TABLE_INDEX_JAPAN_MID);
	SetFreqBand_UpdateUsage (freq_band & 4, TABLE_INDEX_JAPAN_SHB);

	if (!strcmp(ISDBTTunerSpace.CurrentSpace.name, COUNTRY_NAME_JAPAN)) {
		country_code = ISDBTTunerSpace.CurrentSpace.countryCode;
		if (country_code < cCountries) {
			memcpy(&ISDBTTunerSpace.CurrentSpace, &s_spaces[country_code], sizeof(TuneSpace));
		}
	}

	return 1;
}
