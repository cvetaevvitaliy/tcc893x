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

#include "mute_strategy.h"

/* .c Part */
I16S dab_demute_table[40] = {100,89,79,70,63,56,50,44,39,35,31,28,25,22,19,17,15,14,12,11,10,8,7,7,6,5,5,4,3,3,3,2,2,2,1,1,1,1,1,1};
I32U fade_inout_thr[5][2] = {{1950,1750},{1750,1550},{1350,1150},{1150,850},{750,550}};

DAB_MUTE_CTRL_T	gDabMuteCtrl;
F_TC3X_MUTE_CTRL *func_TC3X_MUTE_CTRL;

I32U gDemuteStep = 0;
I32U gDabMuteState = 0;
I32U dab_scalefactor_error_mute = 0;
I32U gMuteTimeCnt = 0;
I16S dab_demute_scaler[40];
I32U gDemuteStepMax;
I32U gMuteWaitTimeCnt;
I32U gDemuteWaitTimeCnt;
I32U gMSCBER_Mute_Cnt;
I32U gAvgMSCBER;
I32U gFadeInStartFlag;
I32U gDabPl;

void DAB_Mute_SetParam_Method(I32U val)
{
	gDabMuteCtrl.gMuteMethod = val;

	DAB_Mute_Var_Clear();

	if(gDabMuteCtrl.gMuteMethod == 0)
		func_TC3X_MUTE_CTRL = DAB_Mute_Control_byMP2;
	else
		func_TC3X_MUTE_CTRL = DAB_Mute_Control_byMSCBER;

}

void DAB_Mute_SetParam_FadeInStart(I32U val)
{
	gDabMuteCtrl.gFadeInStart = val;
	DAB_Mute_Var_Clear();
}

void DAB_Mute_SetParam_FadeInStep(I32U val)
{
	gDabMuteCtrl.gFadeIn_Stepsize = val ;//3%;
	if(gDabMuteCtrl.gFadeIn_Stepsize < 3 )
		gDabMuteCtrl.gFadeIn_Stepsize= 3;	

	DAB_Mute_Var_Clear();
}

void DAB_Mute_SetParam_FadeOutStep(I32U val)
{
	gDabMuteCtrl.gFadeOut_Stepsize = val ;//8%

	if(gDabMuteCtrl.gFadeOut_Stepsize < 3 )
		gDabMuteCtrl.gFadeOut_Stepsize= 3;

	DAB_Mute_Var_Clear();
}

void DAB_Mute_SetParam_AvgFactor(I32U val)
{
	gDabMuteCtrl.gNew_Average_Factor = val;//1/4
	gDabMuteCtrl.gPrev_Average_Factor = val;// 3/4

	DAB_Mute_Var_Clear();
}

void DAB_Mute_SetParam_FadeOutLevel(I32U val)
{
	gDabMuteCtrl.gFadeOut_BER_Threshold = val;

	DAB_Mute_Var_Clear();
}

void DAB_Mute_SetParam_FadeInLevel(I32U val)
{
	gDabMuteCtrl.gFadeIn_BER_Threshold = val;

	DAB_Mute_Var_Clear();
}

void DAB_Mute_SetParam(I32U nMod, I08U method, I08U fadein_start, I32U fadein_step, I32U fadeout_step, I32U avg_factor, I32U fadein_level, I32U fadeout_level)
{
	DAB_Mute_SetParam_Method(method);
	DAB_Mute_SetParam_FadeInStart(fadein_start);
	DAB_Mute_SetParam_FadeInStep(fadein_step);
	DAB_Mute_SetParam_FadeOutStep(fadeout_step);
	DAB_Mute_SetParam_AvgFactor(avg_factor);
	DAB_Mute_SetParam_FadeInLevel(fadein_level);
	DAB_Mute_SetParam_FadeOutLevel(fadeout_level);
}

void DAB_Mute_Param_Set(I32U pl)
{
	if(pl == 0 || pl >5)
	{
		gDabPl = 2;		
	}
	else
	{
		gDabPl = pl - 1;
	}

	gDabMuteCtrl.gFadeOut_BER_Threshold = fade_inout_thr[gDabPl][0];
	gDabMuteCtrl.gFadeIn_BER_Threshold = fade_inout_thr[gDabPl][1];

	DAB_Mute_Var_Clear();
}

void DAB_Mute_Var_Clear(void)
{
	gDemuteStep = 0;
	gDabMuteState = 0;
	gMuteTimeCnt = 0;
	gDemuteStepMax = 0;
	gMuteWaitTimeCnt = 0;
	gDemuteWaitTimeCnt = 0;
	gAvgMSCBER = 0;
	gMSCBER_Mute_Cnt = 0;
	gFadeInStartFlag = 1;

	if(gDabMuteCtrl.gFadeInStart)
	{
		gDemuteStep = 100;
		gFadeInStartFlag = 0;
	}
}

void DAB_Mute_Ctrl_Init(void)
{
	gDemuteStep = 0;
	gDabMuteState = 0;
	gMuteTimeCnt = 0;
	gDemuteStepMax = 0;
	gMuteWaitTimeCnt = 0;
	gDemuteWaitTimeCnt = 0;
	gFadeInStartFlag = 1;
	gDabPl = 2;//protection level 3
	
	gAvgMSCBER = 0;
	gDabMuteCtrl.gMuteMethod = 0;
	gDabMuteCtrl.gFadeInStart = 0;
	gDabMuteCtrl.gNew_Average_Factor = 2;//1/4
	gDabMuteCtrl.gPrev_Average_Factor = 2;// 3/4
	gDabMuteCtrl.gFadeOut_BER_Threshold = fade_inout_thr[gDabPl][0];
	gDabMuteCtrl.gFadeIn_BER_Threshold = fade_inout_thr[gDabPl][1];
	gDabMuteCtrl.gFadeOut_Stepsize = 8 ;//8%
	gDabMuteCtrl.gFadeIn_Stepsize = 3 ;//3%;
	gDabMuteCtrl.gMinimum_Volume = 100;//25%
	gDabMuteCtrl.gFader_Period = 2; //48ms (2 * 24)
	gMSCBER_Mute_Cnt = 0;

	if(gDabMuteCtrl.gFadeOut_Stepsize < 3 )
		gDabMuteCtrl.gFadeOut_Stepsize= 3;
	if(gDabMuteCtrl.gFadeIn_Stepsize < 3 )
		gDabMuteCtrl.gFadeIn_Stepsize= 3;	

	func_TC3X_MUTE_CTRL = DAB_Mute_Control_byMP2;
}

I32U DAB_AudioOut_Smoothing(short *left, short *right, unsigned int size)
{
	unsigned int i;
	short scale;
	
	if(gDabMuteCtrl.gFadeInStart && gFadeInStartFlag == 0)
	{
		if(gDemuteStep > gDabMuteCtrl.gFadeIn_Stepsize)
		{
			gDemuteStep -= gDabMuteCtrl.gFadeIn_Stepsize;
		}
		else
		{
			gDemuteStep = 0;
		}
		
		if(gDemuteStep == 0)
		{	
			gFadeInStartFlag = 1;
		}
		else
		{
			scale = (short)gDemuteStep;
			for(i = 0; i < size; i++)
			{
				left[i] = left[i] / scale;
				right[i] = right[i] / scale;
			}
		}
	}

	return gFadeInStartFlag;	
}
/*
	Rule for demuting time

	delta_td = (delta_tp/5) 
			  with 96ms <= delta_td <= 4080ms

	where, delta_td is demuting time(mute->demute).
	          delta_tp is muting time
*/

/* dec_error
 * 0 : no error,
 * 1: error but codec can decode it.
 */
void DAB_Mute_Control_byMP2(I16S *left, I16S *right, I32S dec_error, I32U bitrate, I32U nSample)
{
	I16S scale = 0;
	I32U k;
	I32U step, index;

	if (dec_error == 0 || (dec_error == 1 && dab_scalefactor_error_mute == 1))
	{
		if( gDabMuteState == 1 )
		{
			if(gDemuteStep == 1)			scale = 6;
			else if(gDemuteStep == 2)		scale = 14;
			else	 if(gDemuteStep == 3)	scale = 30;
			else							scale = 100;
			
			for( k=0; k<nSample; k++) /* 48kHz  : 24mS, 24kHz : 48mS */
			{
				left[k] = left[k]/ scale;
				right[k] =right[k]/ scale;
			}
			
			gDemuteStep++;
			if(gDemuteStep >= 8)
			{
				gDabMuteState = 2;
			}

		}
		else if(gDabMuteState == 2) //mute -->demute
		{
			gDemuteWaitTimeCnt++;
			if (gDemuteWaitTimeCnt > 4)
				gDemuteWaitTimeCnt = 4;

			if( gDemuteWaitTimeCnt == 4 )
			{
				if(gMuteTimeCnt < 20)//mute 구간이 480ms(96 * 5) 이하라면 demute time은 96ms
				{
					gDemuteStepMax = 4;
				}
				else if(gMuteTimeCnt > 850)//mute 구간이 20400(4080ms * 5) 이상이면 demute time을 4080ms
				{
					gDemuteStepMax = 40;
				}
				else
				{
					gDemuteStepMax =(gMuteTimeCnt * 24) / 555 + 4;
	//				p->gDemuteStepMax = ((p->gMuteTimeCnt * 24) / 111)/5 + 4;
				}

				step = 40 / gDemuteStepMax;
				index = 0;
				
				for(k = 0; k < gDemuteStepMax; k++)
				{
					dab_demute_scaler[k] = dab_demute_table[index];
					index += step;
				}

				scale = dab_demute_scaler[0];
				
				for( k=0; k<nSample; k++)
				{
					left[k] = left[k]/ scale;
					right[k] =right[k]/ scale;
				}

				gDabMuteState = 3;
				gMuteTimeCnt = 0;
				gDemuteStep = 1;
			}
			else
			{
				for( k=0; k<nSample; k++)
				{
					left[k] = 0;
					right[k] =0;
				}
			}
		}
		else if (gDabMuteState == 3)
		{
			
			scale = dab_demute_scaler[gDemuteStep];
			gDemuteStep++;
			if(gDemuteStep >= gDemuteStepMax )
			{
				gDabMuteState = 0;
				gDemuteStep = 0;
			}
			
			for( k=0; k<nSample; k++)
			{
				left[k] = left[k]/ scale;
				right[k] =right[k]/ scale;
			}				
		}
		else if( gDabMuteState == 0)
		{					
			gMuteTimeCnt = 0;
			gDemuteStep = 0;
		}

		gMuteWaitTimeCnt = 0;
		
	}
	else
	{
		if( dec_error == 1)
		{
			gMuteWaitTimeCnt++;
			if(gMuteWaitTimeCnt > 4 )
				gMuteWaitTimeCnt = 4;

			if(gDabMuteState == 2 && gDemuteWaitTimeCnt < 4)
			{
				gMuteWaitTimeCnt = 4;
			}
		}
		else /* broekn frame error */
		{
			gMuteWaitTimeCnt = 4;
		}

		if( gMuteWaitTimeCnt == 4)
		{
		
			if( gDabMuteState == 0)
			{
				scale = 2;
				
				for( k=0; k<nSample; k++)
				{
					left[k] = left[k]/ scale;
					right[k] =right[k]/ scale;
				}

				gDabMuteState = 1;
				gDemuteStep = 1;
			}
			else if( gDabMuteState == 1)
			{
				if(gDemuteStep == 1)			scale = 6;
				else if(gDemuteStep == 2)		scale = 14;
				else							scale = 30;

				for( k=0; k<nSample; k++)
				{
					left[k] = left[k]/ scale;
					right[k] =right[k]/ scale;
				}
				gDemuteStep++;
				if(gDemuteStep >= 4)
				{
					gDabMuteState = 2;
				}
			}
			else if (gDabMuteState == 2)
			{
				gDemuteStep = 0;
				for( k=0; k<nSample; k++)
				{
					left[k] = 0;
					right[k] =0;
				}
			}
			else if( gDabMuteState == 3)
			{
				gDabMuteState = 2;
				gDemuteStep = 0;
				scale = 100;
				for( k=0; k<nSample; k++)
				{
					left[k] = left[k]/ scale;
					right[k] =right[k]/ scale;
				}
			}
			gMuteTimeCnt++;
		}
		gDemuteWaitTimeCnt = 0;
	}
}


void DAB_Mute_Control_byMSCBER(I16S *left, I16S *right, I32S dec_error, I32U bitrate, I32U nSample)
{
	I32U current_mscber;
	I32U k, totalbit;

	I16S scaler;
	DAB_MUTE_CTRL_T *p = &gDabMuteCtrl;
	/*
	gDabMuteState : 2-state variable
	*/

	/* 
	//tuning
	totalbit = bitrate * 24;
	current_mscber = TCC3100_read_pcber(0);
	current_mscber = (current_mscber * 10000) / totalbit;

	gAvgMSCBER = gAvgMSCBER / gPrev_Average_Factor + current_mscber / gNew_Average_Factor;
	TC3X_MSG("msc : %d-%d   /%1d.%05d\n", gAvgMSCBER, dec_error, gavgpcber/ 100000, gavgpcber % 100000);
	return;
	*/
	
	if ( gMSCBER_Mute_Cnt == 0)//50ms 마다 control이 들어감
	{
		totalbit = bitrate * 24;

#ifdef TCC3100_INCLUDE
		current_mscber = TCC3100_read_pcber(0);
#endif

		//total bit이 10000bit 이라고 생각하고 ber 환산
		current_mscber = (current_mscber * 10000) / totalbit;

		gAvgMSCBER = gAvgMSCBER / p->gPrev_Average_Factor + current_mscber / p->gNew_Average_Factor;
//		gAvgMSCBER = (gAvgMSCBER * 3)/ gPrev_Average_Factor + current_mscber / gNew_Average_Factor;

		if(gAvgMSCBER > p->gFadeOut_BER_Threshold || (dec_error != 0 && dec_error != 1))
		{		
			if( (gDemuteStep + p->gFadeOut_Stepsize) < p->gMinimum_Volume )
			{
				gDemuteStep += p->gFadeOut_Stepsize;
			}
			else
			{
				gDemuteStep = p->gMinimum_Volume;
			}
			gDabMuteState = 1;
		}
		else if (gAvgMSCBER < p->gFadeIn_BER_Threshold)
		{
			if(gDemuteStep > p->gFadeIn_Stepsize)
			{
				gDemuteStep -= p->gFadeIn_Stepsize;
			}
			else
			{
				gDemuteStep = 0;
			}
			gDabMuteState = 0;
		}
		else
		{
			if(gDabMuteState == 0)
			{
				if(gDemuteStep > p->gFadeIn_Stepsize)
				{
					gDemuteStep -= p->gFadeIn_Stepsize;
				}
				else
				{
					gDemuteStep = 0;
				}
			}
			else
			{
				if( (gDemuteStep + p->gFadeOut_Stepsize) < p->gMinimum_Volume )
				{
					gDemuteStep += p->gFadeOut_Stepsize;
				}
				else
				{
					gDemuteStep = p->gMinimum_Volume;
				}
			}
		}
	}

	scaler = gDemuteStep;
	if( scaler == 0 )
	{
	}
	else if ( scaler == (I16S)p->gMinimum_Volume)
	{
		for( k=0; k<nSample; k++)
		{
			left[k] = 0;
			right[k] = 0;
		}
	}
	else
	{
		for( k=0; k<nSample; k++)
		{
			left[k] /= scaler;
			right[k] /= scaler;
		}
	}

	gMSCBER_Mute_Cnt++;
	gMSCBER_Mute_Cnt %=p->gFader_Period;// 2-module
}


