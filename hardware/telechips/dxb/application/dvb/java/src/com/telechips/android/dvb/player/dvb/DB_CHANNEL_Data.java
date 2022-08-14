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

public class DB_CHANNEL_Data implements Parcelable {

	public static final int MAX_AUDIO_TRACK = 5;
	public  static final int MAX_SUBTITLE_TRACK = 10;

	public DB_SERVICE_Data dbService;
	public DB_AUDIO_Data[] dbAudio = new DB_AUDIO_Data[MAX_AUDIO_TRACK];
	public DB_SUBTITLE_Data[] dbSubtitle = new DB_SUBTITLE_Data[MAX_SUBTITLE_TRACK];

	public DB_CHANNEL_Data() {

		dbService = new DB_SERVICE_Data();
		for (int i = 0; i < MAX_AUDIO_TRACK; i++)
			dbAudio[i] = new DB_AUDIO_Data();
		for (int i = 0; i < MAX_SUBTITLE_TRACK; i++)
			dbSubtitle[i] = new DB_SUBTITLE_Data();
	}

	public DB_CHANNEL_Data(Parcel in) {

		// Service DB
		dbService = new DB_SERVICE_Data(in);

		// Audio DB
		for (int i = 0; i < MAX_AUDIO_TRACK; i++) {
			int uiServiceID = in.readInt();
			int uiCurrentChannelNumber = in.readInt();
			int uiCurrentCountryCode = in.readInt();
			DB_AUDIO_Data db = new DB_AUDIO_Data(in);
			if (i < dbService.uiTotalAudioPID) {
				dbAudio[i] = db;
			}
		}

		// Subtitle DB
		for (int i = 0; i < MAX_SUBTITLE_TRACK; i++) {
			int uiServiceID = in.readInt();
			int uiCurrentChannelNumber = in.readInt();
			int uiCurrentCountryCode = in.readInt();
			DB_SUBTITLE_Data db = new DB_SUBTITLE_Data(in);
			if (i < dbService.uiTotalCntSubtLang) {
				dbSubtitle[i] = db;
			}
		}
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
	}

	private static final Parcelable.Creator<DB_CHANNEL_Data> CREATOR
			= new Parcelable.Creator<DB_CHANNEL_Data>() {
				@Override
				public DB_CHANNEL_Data createFromParcel(Parcel in) {
					return new DB_CHANNEL_Data(in);
				}

				@Override
				public DB_CHANNEL_Data[] newArray(int size) {
					return new DB_CHANNEL_Data[size];
				}
			};
}

