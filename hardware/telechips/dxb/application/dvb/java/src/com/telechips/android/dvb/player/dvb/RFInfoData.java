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

import android.util.Log;
import android.os.Parcel;
import android.os.Parcelable;

public class RFInfoData implements Parcelable {

	public int iDVBSystem;
	public int iFrequency;
	public int iBandwidth;
	public int iRotation;
	public int iConstellation;
	public int iFecLength;
	public int iGuardInterval;
	public int iCodeRate;
	public int iFft;
	public int iPlp;
	public int iPilotPp;

	public RFInfoData() {
	}

	public RFInfoData(Parcel in) {
		iDVBSystem = in.readInt();
		iFrequency = in.readInt();
		iBandwidth = in.readInt();
		iRotation = in.readInt();
		iConstellation = in.readInt();
		iFecLength = in.readInt();
		iGuardInterval = in.readInt();
		iCodeRate = in.readInt();
		iFft = in.readInt();
		iPlp = in.readInt();
		iPilotPp = in.readInt();
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
	}

	private static final Parcelable.Creator<RFInfoData> CREATOR
			= new Parcelable.Creator<RFInfoData>() {
				@Override
				public RFInfoData createFromParcel(Parcel in) {
					return new RFInfoData(in);
				}

				@Override
				public RFInfoData[] newArray(int size) {
					return new RFInfoData[size];
				}
			};
}

