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

public class VideoDefinitionData implements Parcelable {

	public int width;
	public int height;
	public int aspect_ratio;
	public int frame_rate;
	public int progressive;

	public VideoDefinitionData() {
	}

	public VideoDefinitionData(Parcel in) {
		width = in.readInt();
		height = in.readInt();
		aspect_ratio = in.readInt();
		frame_rate = in.readInt();
		progressive = in.readInt();
	}

	public String toString() {
		if (progressive == 1) {
			return String.valueOf(width) + "x" + String.valueOf(height) + " [P]";
		} else {
			return String.valueOf(width) + "x" + String.valueOf(height) + " [I]";
		}
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
	}

	private static final Parcelable.Creator<VideoDefinitionData> CREATOR
			= new Parcelable.Creator<VideoDefinitionData>() {
				@Override
				public VideoDefinitionData createFromParcel(Parcel in) {
					return new VideoDefinitionData(in);
				}

				@Override
				public VideoDefinitionData[] newArray(int size) {
					return new VideoDefinitionData[size];
				}
			};
}

