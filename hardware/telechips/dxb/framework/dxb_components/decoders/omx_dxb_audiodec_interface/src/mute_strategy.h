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

#ifndef __MUTE_STRATEGY_H__
#define __MUTE_STRATEGY_H__

/* .H Part */
typedef signed short I16S;
typedef unsigned int I32U;
typedef signed int I32S;
typedef unsigned char I08U;

typedef void F_TC3X_MUTE_CTRL( I16S *left, I16S *right, I32S dec_error, I32U bitrate, I32U nSample);

typedef struct
{
	I32U gMuteMethod;
	I32U gFadeInStart;
	I32U gNew_Average_Factor;
	I32U gPrev_Average_Factor;
	I32U gFadeOut_BER_Threshold;
	I32U gFadeIn_BER_Threshold;
	I32U gFadeOut_Stepsize;
	I32U gFadeIn_Stepsize;
	I32U gMinimum_Volume;
	I32U gFader_Period;
}DAB_MUTE_CTRL_T;

void DAB_Mute_Control_byMP2(I16S *left, I16S *right, I32S dec_error, I32U bitrate, I32U nSample);
void DAB_Mute_Control_byMSCBER(I16S *left, I16S *right, I32S dec_error, I32U bitrate, I32U nSample);
void DAB_Mute_Ctrl_Init(void);
void DAB_Mute_SetParam_Method(I32U val);
void DAB_Mute_SetParam_FadeInStart(I32U val);
void DAB_Mute_SetParam_FadeInStep(I32U val);
void DAB_Mute_SetParam_FadeOutStep(I32U val);
void DAB_Mute_SetParam_AvgFactor(I32U val);
void DAB_Mute_SetParam_FadeOutLevel(I32U val);
void DAB_Mute_SetParam_FadeInLevel(I32U val);
void DAB_Mute_Var_Clear(void);
void DAB_Mute_Param_Set(I32U pl);

#endif /* __MUTE_STRATEGY_H__ */
