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

#include "DVBTSpace.h"
#define	 PRINTF

static DvbtTunerSpace gstTunerSpace;
static TuneSpace s_spaces[]=
{
	//---------------- NULL ----------------
	// name            tel,   type,     bw, min_ch, max_ch,       finetunes, numof_channels,
	{ "",                0,      0,      0,      0,      0,  { 0, 0, 0, 0 },              0, 
	{
	}
	},
	//---------------- Other (6Mhz) ----------------
	// name            tel,   type,     bw, min_ch, max_ch,       finetunes, numof_channels,
	{ "Other(6Mhz)",  9996,      0,      6,      1,     83,  { 0, 0, 0, 0 },             84,
	{
		{ "C0", 0,		 0, 0, 6, 6, 0 },
		{ "C1", 0,		 0, 0, 6, 6, 0 },
		{ "C2", 0,		 0, 0, 6, 6, 0 },
		{ "C3", 0,		 0, 0, 6, 6, 0 },
		{ "C4", 0,		 0, 0, 6, 6, 0 },
		{ "C5", 0,		 0, 0, 6, 6, 0 },
		{ "C6", 0,		 0, 0, 6, 6, 0 },
		{ "C7", 177000, 0, 0, 6, 6, 0 },
		{ "C8", 183000, 0, 0, 6, 6, 0 },
		{ "C9", 189000, 0, 0, 6, 6, 0 },
		{ "C10", 195000, 0, 0, 6, 6, 0 },
		{ "C11", 201000, 0, 0, 6, 6, 0 },
		{ "C12", 207000, 0, 0, 6, 6, 0 },
		{ "C13", 213000, 0, 0, 6, 6, 0 },
		{ "C14", 473000, 0, 0, 6, 6, 0 },
		{ "C15", 479000, 0, 0, 6, 6, 0 },
		{ "C16", 485000, 0, 0, 6, 6, 0 },
		{ "C17", 491000, 0, 0, 6, 6, 0 },
		{ "C18", 497000, 0, 0, 6, 6, 0 },
		{ "C19", 503000, 0, 0, 6, 6, 0 },
		{ "C20", 509000, 0, 0, 6, 6, 0 },
		{ "C21", 515000, 0, 0, 6, 6, 0 },
		{ "C22", 521000, 0, 0, 6, 6, 0 },
		{ "C23", 527000, 0, 0, 6, 6, 0 },
		{ "C24", 533000, 0, 0, 6, 6, 0 },
		{ "C25", 539000, 0, 0, 6, 6, 0 },
		{ "C26", 545000, 0, 0, 6, 6, 0 },
		{ "C27", 551000, 0, 0, 6, 6, 0 },
		{ "C28", 557000, 0, 0, 6, 6, 0 },
		{ "C29", 563000, 0, 0, 6, 6, 0 },
		{ "C30", 569000, 0, 0, 6, 6, 0 },
		{ "C31", 575000, 0, 0, 6, 6, 0 },
		{ "C32", 581000, 0, 0, 6, 6, 0 },
		{ "C33", 587000, 0, 0, 6, 6, 0 },
		{ "C34", 593000, 0, 0, 6, 6, 0 },
		{ "C35", 599000, 0, 0, 6, 6, 0 },
		{ "C36", 605000, 0, 0, 6, 6, 0 },
		{ "C37", 611000, 0, 0, 6, 6, 0 },
		{ "C38", 617000, 0, 0, 6, 6, 0 },
		{ "C39", 623000, 0, 0, 6, 6, 0 },
		{ "C40", 629000, 0, 0, 6, 6, 0 },
		{ "C41", 635000, 0, 0, 6, 6, 0 },
		{ "C42", 641000, 0, 0, 6, 6, 0 },
		{ "C43", 647000, 0, 0, 6, 6, 0 },
		{ "C44", 653000, 0, 0, 6, 6, 0 },
		{ "C45", 659000, 0, 0, 6, 6, 0 },
		{ "C46", 665000, 0, 0, 6, 6, 0 },
		{ "C47", 671000, 0, 0, 6, 6, 0 },
		{ "C48", 677000, 0, 0, 6, 6, 0 },
		{ "C49", 683000, 0, 0, 6, 6, 0 },
		{ "C50", 689000, 0, 0, 6, 6, 0 },
		{ "C51", 695000, 0, 0, 6, 6, 0 },
		{ "C52", 701000, 0, 0, 6, 6, 0 },
		{ "C53", 707000, 0, 0, 6, 6, 0 },
		{ "C54", 713000, 0, 0, 6, 6, 0 },
		{ "C55", 719000, 0, 0, 6, 6, 0 },
		{ "C56", 725000, 0, 0, 6, 6, 0 },
		{ "C57", 731000, 0, 0, 6, 6, 0 },
		{ "C58", 737000, 0, 0, 6, 6, 0 },
		{ "C59", 743000, 0, 0, 6, 6, 0 },
		{ "C60", 749000, 0, 0, 6, 6, 0 },
		{ "C61", 755000, 0, 0, 6, 6, 0 },
		{ "C62", 761000, 0, 0, 6, 6, 0 },
		{ "C63", 767000, 0, 0, 6, 6, 0 },
		{ "C64", 773000, 0, 0, 6, 6, 0 },
		{ "C65", 779000, 0, 0, 6, 6, 0 },
		{ "C66", 785000, 0, 0, 6, 6, 0 },
		{ "C67", 791000, 0, 0, 6, 6, 0 },
		{ "C68", 797000, 0, 0, 6, 6, 0 },
		{ "C69", 803000, 0, 0, 6, 6, 0 },
		{ "C70", 809000, 0, 0, 6, 6, 0 },
		{ "C71", 815000, 0, 0, 6, 6, 0 },
		{ "C72", 821000, 0, 0, 6, 6, 0 },
		{ "C73", 827000, 0, 0, 6, 6, 0 },
		{ "C74", 833000, 0, 0, 6, 6, 0 },
		{ "C75", 839000, 0, 0, 6, 6, 0 },
		{ "C76", 845000, 0, 0, 6, 6, 0 },
		{ "C77", 851000, 0, 0, 6, 6, 0 },
		{ "C78", 857000, 0, 0, 6, 6, 0 },
		{ "C79", 863000, 0, 0, 6, 6, 0 },
		{ "C80", 869000, 0, 0, 6, 6, 0 },
		{ "C81", 875000, 0, 0, 6, 6, 0 },
		{ "C82", 881000, 0, 0, 6, 6, 0 },
		{ "C83", 887000, 0, 0, 6, 6, 0 },
	}
	},
		
	//---------------- Other (7Mhz) ----------------
	// name            tel,   type,     bw, min_ch, max_ch,       finetunes, numof_channels,
	{ "Other(7Mhz)",  9997,      0,      7,      1,     69,    { 0, 0, 0, 0},            70,
	{
		{ "C0", 0,		 0, 0, 7, 7, 0 },
		{ "C1", 0,		 0, 0, 7, 7, 0 },
		{ "C2", 0,		 0, 0, 7, 7, 0 },
		{ "C3", 0,		 0, 0, 7, 7, 0 },
		{ "C4", 0,		 0, 0, 7, 7, 0 },
		{ "C5", 177500, 0, 0, 7, 7, 0 },
		{ "C6", 184500, 0, 0, 7, 7, 0 },
		{ "C7", 191500, 0, 0, 7, 7, 0 },
		{ "C8", 198500, 0, 0, 7, 7, 0 },
		{ "C9", 205500, 0, 0, 7, 7, 0 },
		{ "C10", 212500, 0, 0, 7, 7, 0 },
		{ "C11", 219500, 0, 0, 7, 7, 0 },
		{ "C12", 226500, 0, 0, 7, 7, 0 },
		{ "C13", 0,		 0, 0, 7, 7, 0 },
		{ "C14", 0,		 0, 0, 7, 7, 0 },
		{ "C15", 0,		 0, 0, 7, 7, 0 },
		{ "C16", 0,		 0, 0, 7, 7, 0 },
		{ "C17", 0,		 0, 0, 7, 7, 0 },
		{ "C18", 0,		 0, 0, 7, 7, 0 },
		{ "C19", 0,		 0, 0, 7, 7, 0 },
		{ "C20", 0,		 0, 0, 7, 7, 0 },
		{ "C21", 0,		 0, 0, 7, 7, 0 },
		{ "C22", 0,		 0, 0, 7, 7, 0 },
		{ "C23", 0,		 0, 0, 7, 7, 0 },
		{ "C24", 0,		 0, 0, 7, 7, 0 },
		{ "C25", 0,		 0, 0, 7, 7, 0 },
		{ "C26", 0,		 0, 0, 7, 7, 0 },
		{ "C27", 0,		 0, 0, 7, 7, 0 },
		{ "C28", 529500, 0, 0, 7, 7, 0 },
		{ "C29", 536500, 0, 0, 7, 7, 0 },
		{ "C30", 543500, 0, 0, 7, 7, 0 },
		{ "C31", 550500, 0, 0, 7, 7, 0 },
		{ "C32", 557500, 0, 0, 7, 7, 0 },
		{ "C33", 564500, 0, 0, 7, 7, 0 },
		{ "C34", 571500, 0, 0, 7, 7, 0 },
		{ "C35", 578500, 0, 0, 7, 7, 0 },
		{ "C36", 585500, 0, 0, 7, 7, 0 },
		{ "C37", 592500, 0, 0, 7, 7, 0 },
		{ "C38", 599500, 0, 0, 7, 7, 0 },
		{ "C39", 606500, 0, 0, 7, 7, 0 },
		{ "C40", 613500, 0, 0, 7, 7, 0 },
		{ "C41", 620500, 0, 0, 7, 7, 0 },
		{ "C42", 627500, 0, 0, 7, 7, 0 },
		{ "C43", 634500, 0, 0, 7, 7, 0 },
		{ "C44", 641500, 0, 0, 7, 7, 0 },
		{ "C45", 648500, 0, 0, 7, 7, 0 },
		{ "C46", 655500, 0, 0, 7, 7, 0 },
		{ "C47", 662500, 0, 0, 7, 7, 0 },
		{ "C48", 669500, 0, 0, 7, 7, 0 },
		{ "C49", 676500, 0, 0, 7, 7, 0 },
		{ "C50", 683500, 0, 0, 7, 7, 0 },
		{ "C51", 690500, 0, 0, 7, 7, 0 },
		{ "C52", 697500, 0, 0, 7, 7, 0 },
		{ "C53", 704500, 0, 0, 7, 7, 0 },
		{ "C54", 711500, 0, 0, 7, 7, 0 },
		{ "C55", 718500, 0, 0, 7, 7, 0 },
		{ "C56", 725500, 0, 0, 7, 7, 0 },
		{ "C57", 732500, 0, 0, 7, 7, 0 },
		{ "C58", 739500, 0, 0, 7, 7, 0 },
		{ "C59", 746500, 0, 0, 7, 7, 0 },
		{ "C60", 753500, 0, 0, 7, 7, 0 },
		{ "C61", 760500, 0, 0, 7, 7, 0 },
		{ "C62", 767500, 0, 0, 7, 7, 0 },
		{ "C63", 774500, 0, 0, 7, 7, 0 },
		{ "C64", 781500, 0, 0, 7, 7, 0 },
		{ "C65", 788500, 0, 0, 7, 7, 0 },
		{ "C66", 795500, 0, 0, 7, 7, 0 },
		{ "C67", 802500, 0, 0, 7, 7, 0 },
		{ "C68", 809500, 0, 0, 7, 7, 0 },
		{ "C69", 816500, 0, 0, 7, 7, 0 },
	}
	},
		
	//---------------- Other (8Mhz) ----------------
	// name            tel,   type,     bw, min_ch, max_ch,       finetunes, numof_channels,
	{ "Other(8Mhz)",  9998,      0,      8,      1,     69,  { 0, 0, 0, 0 },             70,
	{
		{ "C0",  0,		 0, 0, 8, 8, 0 },
		{ "C1",  0,		 0, 0, 8, 8, 0 },
		{ "C2",  0,		 0, 0, 8, 8, 0 },
		{ "C3",  0,		 0, 0, 8, 8, 0 },
		{ "C4",  0,		 0, 0, 8, 8, 0 },
		{ "C5",  178750, 0, 0, 8, 8, 0 },
		{ "C6",  186750, 0, 0, 8, 8, 0 },
		{ "C7",  194750, 0, 0, 8, 8, 0 },
		{ "C8",  202750, 0, 0, 8, 8, 0 },
		{ "C9",  210750, 0, 0, 8, 8, 0 },
		{ "C10", 218750, 0, 0, 8, 8, 0 },
		{ "C11", 0,		 0, 0, 8, 8, 0 },
		{ "C12", 0,		 0, 0, 8, 8, 0 },
		{ "C13", 0,		 0, 0, 8, 8, 0 },
		{ "C14", 0,		 0, 0, 8, 8, 0 },
		{ "C15", 0,		 0, 0, 8, 8, 0 },
		{ "C16", 0,		 0, 0, 8, 8, 0 },
		{ "C17", 0,		 0, 0, 8, 8, 0 },
		{ "C18", 0,		 0, 0, 8, 8, 0 },
		{ "C19", 0,		 0, 0, 8, 8, 0 },
		{ "C20", 0,		 0, 0, 8, 8, 0 },
		{ "C21", 474000, 0, 0, 8, 8, 0 },
		{ "C22", 482000, 0, 0, 8, 8, 0 },
		{ "C23", 490000, 0, 0, 8, 8, 0 },
		{ "C24", 498000, 0, 0, 8, 8, 0 },
		{ "C25", 506000, 0, 0, 8, 8, 0 },
		{ "C26", 514000, 0, 0, 8, 8, 0 },
		{ "C27", 522000, 0, 0, 8, 8, 0 },
		{ "C28", 530000, 0, 0, 8, 8, 0 },
		{ "C29", 538000, 0, 0, 8, 8, 0 },
		{ "C30", 546000, 0, 0, 8, 8, 0 },
		{ "C31", 554000, 0, 0, 8, 8, 0 },
		{ "C32", 562000, 0, 0, 8, 8, 0 },
		{ "C33", 570000, 0, 0, 8, 8, 0 },
		{ "C34", 578000, 0, 0, 8, 8, 0 },
		{ "C35", 586000, 0, 0, 8, 8, 0 },
		{ "C36", 594000, 0, 0, 8, 8, 0 },
		{ "C37", 602000, 0, 0, 8, 8, 0 },
		{ "C38", 610000, 0, 0, 8, 8, 0 },
		{ "C39", 618000, 0, 0, 8, 8, 0 },
		{ "C40", 626000, 0, 0, 8, 8, 0 },
		{ "C41", 634000, 0, 0, 8, 8, 0 },
		{ "C42", 642000, 0, 0, 8, 8, 0 },
		{ "C43", 650000, 0, 0, 8, 8, 0 },
		{ "C44", 658000, 0, 0, 8, 8, 0 },
		{ "C45", 666000, 0, 0, 8, 8, 0 },
		{ "C46", 674000, 0, 0, 8, 8, 0 },
		{ "C47", 682000, 0, 0, 8, 8, 0 },
		{ "C48", 690000, 0, 0, 8, 8, 0 },
		{ "C49", 698000, 0, 0, 8, 8, 0 },
		{ "C50", 706000, 0, 0, 8, 8, 0 },
		{ "C51", 714000, 0, 0, 8, 8, 0 },
		{ "C52", 722000, 0, 0, 8, 8, 0 },
		{ "C53", 730000, 0, 0, 8, 8, 0 },
		{ "C54", 738000, 0, 0, 8, 8, 0 },
		{ "C55", 746000, 0, 0, 8, 8, 0 },
		{ "C56", 754000, 0, 0, 8, 8, 0 },
		{ "C57", 762000, 0, 0, 8, 8, 0 },
		{ "C58", 770000, 0, 0, 8, 8, 0 },
		{ "C59", 778000, 0, 0, 8, 8, 0 },
		{ "C60", 786000, 0, 0, 8, 8, 0 },
		{ "C61", 794000, 0, 0, 8, 8, 0 },
		{ "C62", 802000, 0, 0, 8, 8, 0 },
		{ "C63", 810000, 0, 0, 8, 8, 0 },
		{ "C64", 818000, 0, 0, 8, 8, 0 },
		{ "C65", 826000, 0, 0, 8, 8, 0 },
		{ "C66", 834000, 0, 0, 8, 8, 0 },
		{ "C67", 842000, 0, 0, 8, 8, 0 },
		{ "C68", 850000, 0, 0, 8, 8, 0 },
		{ "C69", 858000, 0, 0, 8, 8, 0 },
	}
	}
};

void DVBTSPACE_SetTuneSpace(TuneSpace *tuneSpace)
{
	memcpy(&s_spaces[0], tuneSpace, sizeof(TuneSpace));
}

void DVBTSPACE_SetSignalType(int signaltype)
{
	gstTunerSpace.SignalType = signaltype;
}

void DVBTSPACE_SetBandwidth(int bandwidth)
{
	gstTunerSpace.DefaultBandWidth = bandwidth;
}

void DVBTSPACE_CTuneSpace(int countrycode)
{
	DVBTSPACE_SetSignalType(DEFAULT_SIGNAL_TYPE);
//	printf("countrycode = %d \n" , countrycode);
	if(countrycode==0)
		countrycode = DEFAULT_DVB_COUNTRY;
		
	DVBTSPACE_SetCountryCode(/*DEFAULT_DVB_COUNTRY*/countrycode);
	DVBTSPACE_SetBandwidth(DEFAULT_BANDWIDTH);
}

int	DVBTSPACE_GetMinChannel(void)
{
	//return s_spaces[gstTunerSpace.CurrentSpaceIndex].minChannel;
	return 0;
}

int	DVBTSPACE_GetMaxChannel(void)
{
	//return s_spaces[gstTunerSpace.CurrentSpaceIndex].maxChannel;
	return (s_spaces[gstTunerSpace.CurrentSpaceIndex].numbers-1); //End channel index
}

void DVBTSPACE_SetCountryCode(unsigned long countryCode)
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
				i = 1;
			break;
			case 7:
				i = 2;
			break;
			default:
				i = 3;
			break;
		}
	}
	gstTunerSpace.CurrentSpaceIndex = i;
	PRINTF("[TUNER]Setting INFO : Country(%s)Code(%d)BW(%d)\n",s_spaces[gstTunerSpace.CurrentSpaceIndex].name, s_spaces[gstTunerSpace.CurrentSpaceIndex].countryCode, s_spaces[gstTunerSpace.CurrentSpaceIndex].typicalBandwidth );
}

int DVBTSPACE_GetFrequency(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < s_spaces[gstTunerSpace.CurrentSpaceIndex].numbers) 
	{
		p_ch = &s_spaces[gstTunerSpace.CurrentSpaceIndex].channels[chIndex];
		return p_ch->alt_selected ? p_ch->frequency2 : p_ch->frequency1;
	}
	return 0;
}

int DVBTSPACE_GetIndexByFrequency(unsigned int uiFrequencyKhz)
{
    unsigned int i, freq;
	OneChannel* p_ch;
    for(i=0;i<s_spaces[gstTunerSpace.CurrentSpaceIndex].numbers;i++)
    {
        p_ch = &s_spaces[gstTunerSpace.CurrentSpaceIndex].channels[i];
        freq = p_ch->alt_selected ? p_ch->frequency2 : p_ch->frequency1;
        if(freq == uiFrequencyKhz)
            return i;

    }
	return -1;
}

int DVBTSPACE_GetFinetune(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < s_spaces[gstTunerSpace.CurrentSpaceIndex].numbers) 
	{
		p_ch = &s_spaces[gstTunerSpace.CurrentSpaceIndex].channels[chIndex];
		return p_ch->finetune;
	}
	return 0;
}

int DVBTSPACE_GetBandwidth(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < s_spaces[gstTunerSpace.CurrentSpaceIndex].numbers) 
	{
		p_ch = &s_spaces[gstTunerSpace.CurrentSpaceIndex].channels[chIndex];
		return p_ch->alt_selected ? p_ch->bandwidth2 : p_ch->bandwidth1;
	}
	return 0;
}

OneChannel* DVBTSPACE_GetOneChannel(unsigned long chIndex)
{
	OneChannel* p_ch;

	if (chIndex < s_spaces[gstTunerSpace.CurrentSpaceIndex].numbers) 
	{
		p_ch = &s_spaces[gstTunerSpace.CurrentSpaceIndex].channels[chIndex];
		return p_ch;
	}
	return NULL;
}


void DVBTSPACE_SetFinetune(unsigned long chIndex, long finetune)
{
	OneChannel* p_ch;

	if (chIndex < s_spaces[gstTunerSpace.CurrentSpaceIndex].numbers) 
	{
		p_ch = &s_spaces[gstTunerSpace.CurrentSpaceIndex].channels[chIndex];
		p_ch->finetune = finetune;
	}
}

void DVBTSPACE_SetAltSelected(unsigned long chIndex, int makeAlt)
{
	OneChannel* p_ch;

	if (chIndex < s_spaces[gstTunerSpace.CurrentSpaceIndex].numbers) 
	{
		p_ch = &s_spaces[gstTunerSpace.CurrentSpaceIndex].channels[chIndex];
		p_ch->alt_selected = makeAlt ? 1 : 0;
	}
}


int DVBTSPACE_GetPilotFinetunes(unsigned long* pNumbers, long* pFinetunes)
{
	unsigned long numbers = 0;
	unsigned int i;
	for (i = 0; i < 4; i++) 
	{
		if (s_spaces[gstTunerSpace.CurrentSpaceIndex].finetunes[i] != 0) 
		{
			numbers++;
			*pFinetunes++ = s_spaces[gstTunerSpace.CurrentSpaceIndex].finetunes[i];
		}
	}
	*pNumbers = numbers;
	return numbers > 0;
}

int DVBTSPACE_GetIndexByManualFrequency(int iFrequencyKhz, int iBandwidthMhz, int *piChannelIndex)
{
	const int cCountries = sizeof(s_spaces) / sizeof(s_spaces[0]);
    unsigned int i, index, freq;
	OneChannel* p_ch;
	*piChannelIndex = -1;

	for(index = 0; index < cCountries; index++)
    {
        for(i=0;i<s_spaces[index].numbers;i++)
        {
            p_ch = &s_spaces[index].channels[i];
            freq = p_ch->alt_selected ? p_ch->frequency2 : p_ch->frequency1;
            if(freq == iFrequencyKhz && iBandwidthMhz == p_ch->bandwidth1)
            {
                *piChannelIndex = i;
	            return s_spaces[index].countryCode;
            }

        }
    }
	return 0;

}


