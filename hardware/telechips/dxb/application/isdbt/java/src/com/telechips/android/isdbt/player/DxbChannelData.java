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
package com.telechips.android.isdbt.player;

import android.os.Parcel;
import android.os.Parcelable;

public class DxbChannelData implements Parcelable
{
	public int	iID;
	
	public int	iChannel_number;
	public int	iCountry_code;
	public int	iFrequency;
	public int	iService_type;
	
	public int	iAudio_PID;
	public int	iVideo_PID;
	public int	iTeletext_PID;
	
	public int	iService_ID;

	public String	sChannel_name;
	
	public int	iFree_CA_mode;
	public int	iScrambling;
	public int	iBookmark;
	public int	iLogical_channel;

    public int	iPMT_PID;
    public int	iRemocon_ID;
    public int	iRegion_ID;
    public int	iThreeDigitNumber;
    public int  iTStream_ID;
    public int	iBER_Avg;
    public int	iPreset;
    public int	iNetwork_ID;
    
	public DxbChannelData()
	{
		
	}
	
	public DxbChannelData(Parcel in)
	{
		readFromParcel(in);
	}
	
	public DxbChannelData(	int		ID,
							int		channelNumber,
							int		countryCode,
							//int		frequency,
							int		serviceType,
							int		audioPID,
							int		videoPID,
							//int		teletextPID,
							int		serviceID,
							String	channelName,
							//int		freeCAmode,
							//int		scrambling,
							//int		bookmark,
							//int		logicalChannel,
						    int		pmtPID,
						    int		remoconID,
						    int		regionID,
						    int		threeDigitNumber,
						    int		tstreamID,
						    int		berAvg,
						    int		preset,
						    int		networkID							
	)
	{
		this.iID				= ID;
		
		this.iChannel_number	= channelNumber;
		this.iCountry_code		= countryCode;
		this.iFrequency			= 0; //frequency;
		this.iService_type		= serviceType;
		
		this.iAudio_PID			= audioPID;
		this.iVideo_PID			= videoPID;
		this.iTeletext_PID		= 0; //teletextPID;
		this.iService_ID		= serviceID;
		
		this.sChannel_name		= channelName;
		
		this.iFree_CA_mode		= 0; //freeCAmode;
		this.iScrambling		= 0; //scrambling;
		this.iBookmark			= 0; //bookmark;
		this.iLogical_channel	= 0; //logicalChannel;	

		this.iPMT_PID			= pmtPID;
		this.iRemocon_ID		= remoconID;
		this.iRegion_ID			= regionID;
		this.iThreeDigitNumber	= threeDigitNumber;
		this.iTStream_ID		= tstreamID;
		this.iBER_Avg			= berAvg;
		this.iPreset			= preset;
		this.iNetwork_ID		= networkID;    
	}
	
	public int describeContents()
	{
		return 0;
	}

	public void writeToParcel(Parcel dest, int flags)
	{
		dest.writeInt(iID);
		
		dest.writeInt(iChannel_number);
		dest.writeInt(iCountry_code);
		dest.writeInt(iFrequency);
		dest.writeInt(iService_type);

		dest.writeInt(iAudio_PID);
		dest.writeInt(iVideo_PID);
		dest.writeInt(iTeletext_PID);
		dest.writeInt(iService_ID);

		dest.writeString(sChannel_name);

		dest.writeInt(iFree_CA_mode);
		dest.writeInt(iScrambling);
		dest.writeInt(iBookmark);
		dest.writeInt(iLogical_channel);
		
		dest.writeInt(iPMT_PID);
		dest.writeInt(iRemocon_ID);
		dest.writeInt(iRegion_ID);
		dest.writeInt(iThreeDigitNumber);
		dest.writeInt(iTStream_ID);
		dest.writeInt(iBER_Avg);
		dest.writeInt(iPreset);
		dest.writeInt(iNetwork_ID);
	}
	
	private void readFromParcel(Parcel in)
	{
		iID					= in.readInt();
		
		iChannel_number		= in.readInt();
		iCountry_code		= in.readInt();
		iFrequency			= in.readInt();
		iService_type		= in.readInt();

		iAudio_PID			= in.readInt();
		iVideo_PID			= in.readInt();
		iTeletext_PID		= in.readInt();
		iService_ID			= in.readInt();
		
		sChannel_name		= in.readString();
		
		iFree_CA_mode		= in.readInt();
		iScrambling			= in.readInt();
		iBookmark			= in.readInt();
		iLogical_channel	= in.readInt();

		iPMT_PID			= in.readInt();
		iRemocon_ID			= in.readInt();
		iRegion_ID			= in.readInt();
		iThreeDigitNumber	= in.readInt();
		iTStream_ID			= in.readInt();
		iBER_Avg			= in.readInt();
		iPreset				= in.readInt();
		iNetwork_ID			= in.readInt();
	}

	@SuppressWarnings("rawtypes")
	public static final Parcelable.Creator CREATOR = new Parcelable.Creator()
	{
		public DxbChannelData createFromParcel(Parcel in)
		{
			return new DxbChannelData(in);
		}
		
		public DxbChannelData[] newArray(int size)
		{
			return new DxbChannelData[size];
		}
	};
}
