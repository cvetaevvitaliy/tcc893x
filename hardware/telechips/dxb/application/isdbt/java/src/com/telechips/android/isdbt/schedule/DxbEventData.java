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
package com.telechips.android.isdbt.schedule;

import android.os.Parcel;
import android.os.Parcelable;

public class DxbEventData implements Parcelable {

	public int		iAction;
	
	public int		iCountTV;
	public int		iCountRadio;
	
	public int		iIndexService;
	public int		iChannelType;

	public long	lUTC_baseTOT;
	public long	lUTC_Start;
	public long	lUTC_End;
	
	public int		iRepeatType;
	public int		iMode;

	public DxbEventData()
	{
		
	}
	
	public DxbEventData(Parcel in)
	{
		readFromParcel(in);
	}
	
	public DxbEventData(int	action,
			
						int	countTV,
						int	countRadio,
	
						int	indexService,
						int	channelType,
					
						long	UTC_baseTOT,
						long	UTC_Start,
						long	UTC_End,
						
						int	repeatType,
						int	mode
	)
	{
		this.iAction		= action;
		
		this.iCountTV		= countTV;
		this.iCountRadio	= countRadio;
		
		this.iIndexService	= indexService;
		this.iChannelType	= channelType;

		this.lUTC_baseTOT	= UTC_baseTOT;
		this.lUTC_Start		= UTC_Start;
		this.lUTC_End		= UTC_End;
		
		this.iRepeatType	= repeatType;
		this.iMode			= mode;
	}
	
	public int describeContents()
	{
		return 0;
	}

	public void writeToParcel(Parcel dest, int flags)
	{
		dest.writeInt(iAction);
		
		dest.writeInt(iCountTV);
		dest.writeInt(iCountRadio);
		
		dest.writeInt(iIndexService);
		dest.writeInt(iChannelType);
		
		dest.writeLong(lUTC_baseTOT);
		dest.writeLong(lUTC_Start);
		dest.writeLong(lUTC_End);
		
		dest.writeInt(iRepeatType);
		dest.writeInt(iMode);
	}
	
	private void readFromParcel(Parcel in)
	{
		iAction			= in.readInt();
		
		iCountTV		= in.readInt();
		iCountRadio		= in.readInt();
		
		iIndexService	= in.readInt();
		iChannelType	= in.readInt();

		lUTC_baseTOT	= in.readLong();
		lUTC_Start		= in.readLong();
		lUTC_End		= in.readLong();
		
		iRepeatType		= in.readInt();
		iMode			= in.readInt();
	}

	@SuppressWarnings("rawtypes")
	public static final Parcelable.Creator CREATOR = new Parcelable.Creator()
	{
		public DxbEventData createFromParcel(Parcel in)
		{
			return new DxbEventData(in);
		}
		
		public DxbEventData[] newArray(int size)
		{
			return new DxbEventData[size];
		}
	};
}
