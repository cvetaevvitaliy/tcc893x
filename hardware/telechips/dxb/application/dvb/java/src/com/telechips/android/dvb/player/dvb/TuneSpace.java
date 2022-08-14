/*
 * Copyright (C) 2009 Telechips, Inc.
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

package com.telechips.android.dvb.player.dvb;

public class TuneSpace
{
	public TuneSpace(int num)
	{
		finetunes = new int[4];
		numbers = num;
		channels = new OneChannel[num];
		for (int i=0; i < num; i++)
			channels[i] = new OneChannel();
	}

	public String name;            // name of the tune space
	public int countryCode;        // country code
	public int space;              // 0=terrestrial, 1=cable, 2=satellite
	public int typicalBandwidth;   // typical bandwidth
	public int minChannel;         // min channel
	public int maxChannel;         // max channel
	public int[] finetunes;        // pilot finetunes
	public int numbers;            // number of channels
	public OneChannel[] channels;
};

