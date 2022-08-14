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

public class DB_CHANNEL_Data {
	public DB_CHANNEL_Data() {
	}

	public int channelNumber;
	public int countryCode;
	public int uiVersionNum;
	public int audioPID;
	public int videoPID;
	public int stPID;
	public int siPID;
	public int PMT_PID;
	public int remoconID;
	public int serviceType;
	public int serviceID;
	public int regionID;
	public int threedigitNumber;
	public int TStreamID;
	public int berAVG;			
	public int preset;
	public int networkID;
	public int areaCode;
	public char [] channelName;
}
