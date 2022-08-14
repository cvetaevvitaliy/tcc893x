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
package com.telechips.android.isdbt.player.isdbt;

public class DateTimeData
{	
	public int iMJD;
	public int iHour;
	public int iMinute;
	public int iSecond;

	public int iPolarity;
	public int iHourOffset;
	public int iMinOffset;

	public int iLocalMJD;
	public int iLocalHour;
	public int iLocalMin;
	public int iLocalSec;

	public DateTimeData()
	{
		iMJD = 0;
		iHour = 0;
		iMinute = 0;
		iSecond = 0;
	
		iPolarity = 0;
		iHourOffset = 0;
		iMinOffset = 0;

		iLocalMJD = 0;
		iLocalHour = 0;
		iLocalMin = 0;
		iLocalSec = 0;
	}
}

