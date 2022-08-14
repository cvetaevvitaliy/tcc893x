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

public class DB_AUDIO_Data {

	public int uiAudioPID;
	public int uiAudioIsScrambling;
	public int uiAudioStreamType;
	public int uiAudioType;
	public String stracLangCode;

	public DB_AUDIO_Data() {
	}

	public DB_AUDIO_Data(Parcel in) {

		uiAudioPID = in.readInt();
		uiAudioIsScrambling = in.readInt();
		uiAudioStreamType = in.readInt();
		uiAudioType = in.readInt();

		ByteBuffer byteBuffer = ByteBuffer.allocate(8);

		byteBuffer.order(ByteOrder.LITTLE_ENDIAN);

		IntBuffer intBuffer = byteBuffer.asIntBuffer();
		intBuffer.put(in.readInt());
		intBuffer.put(in.readInt());

		byte [] temp = byteBuffer.array();
		int i;
		for (i = 0; i < 8; i++)
		{
			if (temp[i] == 0x00)
				break;
		}
		stracLangCode = new String(temp, 0, i);
	}
}

