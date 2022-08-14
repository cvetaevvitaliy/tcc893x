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

import android.os.Parcel;
import android.os.Parcelable;

public class BlindScanInfo implements Parcelable {

	public int mPercent;
	public int mIndex;
	public int mFreqMHz;
	public int mSymbolKHz;

	public BlindScanInfo() {
	}

	public BlindScanInfo(Parcel in) {
		mPercent = in.readInt();
		mIndex = in.readInt();
		mFreqMHz = in.readInt();
		mSymbolKHz = in.readInt();
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
	}

	private static final Parcelable.Creator<BlindScanInfo> CREATOR
			= new Parcelable.Creator<BlindScanInfo>() {
				@Override
				public BlindScanInfo createFromParcel(Parcel in) {
					return new BlindScanInfo(in);
				}

				@Override
				public BlindScanInfo[] newArray(int size) {
					return new BlindScanInfo[size];
				}
			};
}
