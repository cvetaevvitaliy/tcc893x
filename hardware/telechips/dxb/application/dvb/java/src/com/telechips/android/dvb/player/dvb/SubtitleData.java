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

public class SubtitleData implements Parcelable {

	public int region_id;
	public int x;
	public int y;
	public int width;
	public int height;
	public int[] data;

	public SubtitleData() {
	}

	public SubtitleData(Parcel in) {
		region_id = in.readInt();
		x = in.readInt();
		y = in.readInt();
		width = in.readInt();
		height = in.readInt();
		int size = width*height;
		data = new int[size];
		for (int i = 0; i < size; i++)
			data[i] = in.readInt();
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
	}

	private static final Parcelable.Creator<SubtitleData> CREATOR
			= new Parcelable.Creator<SubtitleData>() {
				@Override
				public SubtitleData createFromParcel(Parcel in) {
					return new SubtitleData(in);
				}

				@Override
				public SubtitleData[] newArray(int size) {
					return new SubtitleData[size];
				}
			};
}

