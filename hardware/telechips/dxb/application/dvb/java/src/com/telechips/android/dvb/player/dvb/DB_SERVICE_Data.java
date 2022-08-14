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

public class DB_SERVICE_Data {

	public int uiCurrentChannelNumber;
	public int uiCurrentCountryCode;
	public int uiVersionNum;
	public int uiTStreamID;
	public int uiOrigNetID;
	public int uiServiceID;
	public int uiPMTPID;
	public int uiEnType;
	public int uifEITSchedule;
	public int uifEITPresentFollowing;
	public int uifCAModeFree;
	public int uifAllowCountry;
	public int uiNameLength;
	public String strServiceName;
	public int uiAudioPID;
	public int uiVideoPID;
	public int uiVideoIsScrambling;
	public int uiAudioStreamType;
	public int uiVideoStreamType;
	public int uiPCRPID;
	public int uiTotalAudioPID;
	public int uiLastTableID;
	public int uiLogicalChannel;
	public int uiTotalCntSubtLang;
	public int uiTeletextPID;
	public int uiCountTeletextInfo;
	public int uiPMTVersionNum;
	public int uiSDTVersionNum;
	public int uiNITVersionNum;

	public DB_SERVICE_Data() {
	}

	public DB_SERVICE_Data(Parcel in) {

		uiCurrentChannelNumber = in.readInt();
		uiCurrentCountryCode = in.readInt();
		uiVersionNum = in.readInt();
		uiTStreamID = in.readInt();
		uiOrigNetID = in.readInt();
		uiServiceID = in.readInt();
		uiPMTPID = in.readInt();
		uiEnType = in.readInt();
		uifEITSchedule = in.readInt();
		uifEITPresentFollowing = in.readInt();
		uifCAModeFree = in.readInt();
		uifAllowCountry = in.readInt();
		uiNameLength = in.readInt();

		ByteBuffer byteBuffer = ByteBuffer.allocate(32);

		byteBuffer.order(ByteOrder.LITTLE_ENDIAN);

		IntBuffer intBuffer = byteBuffer.asIntBuffer();
		intBuffer.put(in.readInt());
		intBuffer.put(in.readInt());
		intBuffer.put(in.readInt());
		intBuffer.put(in.readInt());
		intBuffer.put(in.readInt());
		intBuffer.put(in.readInt());
		intBuffer.put(in.readInt());
		intBuffer.put(in.readInt());

		byte [] temp = byteBuffer.array();
		int i;
		for (i = 0; i < uiNameLength; i++)
		{
			if (temp[i] == 0x00)
				break;
		}
		strServiceName = new String(temp, 0, i);

		uiAudioPID = in.readInt();
		uiVideoPID = in.readInt();
		uiVideoIsScrambling = in.readInt();
		uiAudioStreamType = in.readInt();
		uiVideoStreamType = in.readInt();
		uiPCRPID = in.readInt();
		uiTotalAudioPID = in.readInt();
		uiLastTableID = in.readInt();
		uiLogicalChannel = in.readInt();
		uiTotalCntSubtLang = in.readInt();
		uiTeletextPID = in.readInt();
		uiCountTeletextInfo = in.readInt();
		uiPMTVersionNum = in.readInt();
		uiSDTVersionNum = in.readInt();
		uiNITVersionNum = in.readInt();
	}
}

