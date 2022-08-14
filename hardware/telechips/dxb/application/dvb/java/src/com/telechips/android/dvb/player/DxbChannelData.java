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

package com.telechips.android.dvb.player;

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
							int		frequency,
							int		serviceType,
							int		audioPID,
							int		videoPID,
							int		teletextPID,
							int		serviceID,
							String	channelName,
							int		freeCAmode,
							int		scrambling,
							int		bookmark,
							int		logicalChannel
	)
	{
		this.iID				= ID;
		
		this.iChannel_number	= channelNumber;
		this.iCountry_code		= countryCode;
		this.iFrequency			= frequency;
		this.iService_type		= serviceType;
		
		this.iAudio_PID			= audioPID;
		this.iVideo_PID			= videoPID;
		this.iTeletext_PID		= teletextPID;
		this.iService_ID		= serviceID;
		
		this.sChannel_name		= channelName;
		
		this.iFree_CA_mode		= freeCAmode;
		this.iScrambling		= scrambling;
		this.iBookmark			= bookmark;
		this.iLogical_channel	= logicalChannel;	
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