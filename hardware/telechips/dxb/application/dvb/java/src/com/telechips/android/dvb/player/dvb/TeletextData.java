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

public class TeletextData implements Parcelable {

	public int mErase; //0:don't erase, 1: erase page. this is for ttx subtitle.
	public int mPageNumber;
	public int mControlBits; // ...0/C14/C13/C12/C11/C10/C9/C8/C7/C6/C5/C4/0/0/0/0/
	public int mNationalOptionCharacterSubset;

	public byte[] mData;      //size (25+1)*40 = 4 * 260
	public byte[] mAttribute; //size (25+1)*40 = 4 * 260
	public byte[] mbUpdated;  //size (32)      = 4 * 8

	public int mActivePositionRow;
	public int mActivePositionColumn;

	public TeletextData() {
	}

	public TeletextData(Parcel in) {
		mErase = in.readInt();
		mPageNumber = in.readInt();
		mControlBits = in.readInt();
		mNationalOptionCharacterSubset = in.readInt();
		mData = readByte(in, 260);
		mAttribute = readByte(in, 260);
		mbUpdated = readByte(in, 8);
		mActivePositionRow = in.readInt();
		mActivePositionColumn = in.readInt();
	}

	private byte[] readByte(Parcel in, int size) {
		ByteBuffer byteBuffer;
		IntBuffer intBuffer;

		byteBuffer = ByteBuffer.allocate(size*4);
		byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
		intBuffer = byteBuffer.asIntBuffer();
		for (int i = 0; i < size; i++) {
			intBuffer.put(in.readInt());
		}
		return byteBuffer.array();
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
	}

	private static final Parcelable.Creator<TeletextData> CREATOR
			= new Parcelable.Creator<TeletextData>() {
				@Override
				public TeletextData createFromParcel(Parcel in) {
					return new TeletextData(in);
				}

				@Override
				public TeletextData[] newArray(int size) {
					return new TeletextData[size];
				}
			};
}

