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

public class DB_SUBTITLE_Data {
	public DB_SUBTITLE_Data() {
	}

	public int uiServiceID;
	public int uiCurrentChannelNumber;
	public int uiCurrentCountryCode;
	public int uiSubtitlePID;
	public byte[] ucacSubtLangCode;//[8] = new byte[8];
	//public String stracSubtLangCode;
	public int uiSubtType;
	public int uiCompositionPageID;
	public int uiAncillaryPageID;
}

