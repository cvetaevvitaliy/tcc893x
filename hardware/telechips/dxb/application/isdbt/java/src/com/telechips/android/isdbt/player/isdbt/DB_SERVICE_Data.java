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

package com.telechips.android.isdbt.player.isdbt;

public class DB_SERVICE_Data {
	public DB_SERVICE_Data() {
	}
	
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
	public byte[] ucServiceName;// = new byte[32];
	//public String strServiceName;
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
	
	public int uiECMPID;
}

