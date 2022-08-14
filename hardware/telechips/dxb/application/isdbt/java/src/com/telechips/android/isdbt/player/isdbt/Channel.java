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

import com.telechips.android.isdbt.player.DxbChannelData;

public class Channel
{
	
	public int filePlay;	
	
	public int ID;
	
	public int channelNumber;
	public int countryCode;
	public int frequency;
	public int serviceType;
	
	public int audioPID;
	public int videoPID;
	public  int teletextPID;
	
	public int serviceID;
    public int channelFreq;	
	public String channelName;

	public int iBookmark;
	public int logicalChannel;
 
	public int pmtPID;
	public int remoconID;
	
	public int regionID;
	public int threeDigitNumber;
	public int tstreamID;
	public int berAVG;
	public int preset;
	public int networkID;
	
	public int totalAudioPID;
	public int totalVideoPID;
	public int totalSubtitleLang;
	public int CaptionMgmtNumLang;
	
	public int audioMode;
	public int videoMode;
	public int audioLang1;
	public int audioLang2;
	public int startMJD;
	public int startHH;
	public int startMM;
	public int startSS;
	public int durationHH;
	public int durationMM;
	public int durationSS;
	public String eventName;

	public int logoImageWidth;
	public int logoImageHeight;
	public int[] plogoImage;

	public Channel()
	{
	}

	public void clear()
	{
		this.ID = -1;
		
		//this.channelNumber		= -1;
		this.countryCode		= -1;
		this.frequency			= -1;
		this.serviceType		= -1;
		
		this.audioPID			= -1;
		this.videoPID			= -1;
		this.teletextPID		= -1;
		//this.serviceID			= -1;
		
		this.channelName		= "";
		
		this.iBookmark			= -1;
		this.logicalChannel		= -1;	

		this.pmtPID				= -1;
		this.remoconID			= -1;
		this.regionID			= -1;
		this.threeDigitNumber	= -1;
		this.tstreamID			= -1;
		this.berAVG				= -1;
		this.preset				= -1;
		this.networkID			= -1;
	}

	public void set(DxbChannelData data)
	{
		if(data == null)
		{
			clear();
		}
		else
		{
			this.ID					= data.iID;
			
			this.channelNumber		= data.iChannel_number;
			this.countryCode		= data.iCountry_code;
			this.frequency			= data.iFrequency;
			this.serviceType		= data.iService_type;
			
			this.audioPID			= data.iAudio_PID;
			this.videoPID			= data.iVideo_PID;
			this.teletextPID		= data.iTeletext_PID;
			this.serviceID			= data.iService_ID;
			
			this.channelName		= data.sChannel_name;
			
			this.iBookmark			= data.iBookmark;
			this.logicalChannel		= data.iLogical_channel;	
	
			this.pmtPID				= data.iPMT_PID;
			this.remoconID			= data.iRemocon_ID;
			this.regionID			= data.iRegion_ID;
			this.threeDigitNumber	= data.iThreeDigitNumber;
			this.tstreamID			= data.iTStream_ID;
			this.berAVG				= data.iBER_Avg;
			this.preset				= data.iPreset;
			this.networkID			= data.iNetwork_ID;
		}
	}
	
	public boolean isSameChannel(DxbChannelData data)
	{
		boolean ret = false;
		
		if(data == null)
		{
			if((this.ID == -1)
				&& (this.pmtPID == -1) && (this.remoconID == -1) && (this.regionID == -1) && (this.threeDigitNumber == -1)
				&& (this.tstreamID == -1) && (this.berAVG == -1) && (this.preset ==  -1) && (this.networkID == -1))
			{
				ret = true;
			}
		}
		else
		{
			if((this.ID == data.iID) 
				&& (this.channelNumber == data.iChannel_number) && (this.countryCode == data.iCountry_code) && (this.frequency == data.iFrequency)
				&& (this.serviceID == data.iService_ID) && (this.serviceType == data.iService_type)
				&& (this.audioPID == data.iAudio_PID) && (this.videoPID == data.iVideo_PID) && (this.pmtPID == data.iPMT_PID)
				&& (this.networkID == data.iNetwork_ID) && (this.tstreamID == data.iTStream_ID)
				&& (this.preset == data.iPreset))
			{
				ret = true;
			}
		}
		
		return ret;
	}
}
