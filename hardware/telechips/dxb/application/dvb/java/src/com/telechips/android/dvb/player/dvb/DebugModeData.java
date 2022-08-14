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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;

import android.util.Log;
import android.os.Parcel;
import android.os.Parcelable;

public class DebugModeData implements Parcelable {

	public int    iIndex;
	public int    iLength;
	public byte[] msg;

	public DebugModeData() {
	}

	public DebugModeData(Parcel in) {
		iIndex = in.readInt();
		iLength = in.readInt();

		int intLength = (iLength+3)>>2;

		ByteBuffer byteBuffer;
		IntBuffer intBuffer;

		byteBuffer = ByteBuffer.allocate(intLength*4);
		byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
		intBuffer = byteBuffer.asIntBuffer();

		for (int i = 0; i < intLength; i++) {
			intBuffer.put(in.readInt());
		}
		msg = byteBuffer.array();
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
	}

	private static final Parcelable.Creator<DebugModeData> CREATOR
			= new Parcelable.Creator<DebugModeData>() {
				@Override
				public DebugModeData createFromParcel(Parcel in) {
					return new DebugModeData(in);
				}

				@Override
				public DebugModeData[] newArray(int size) {
					return new DebugModeData[size];
				}
			};
}
