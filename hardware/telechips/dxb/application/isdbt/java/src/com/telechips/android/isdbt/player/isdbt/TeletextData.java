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

public class TeletextData
{	
	public int mErase; //0:don't erase, 1: erase page. this is for ttx subtitle.
	public int mPageNumber;
	public int mControlBits; // ...0/C14/C13/C12/C11/C10/C9/C8/C7/C6/C5/C4/0/0/0/0/
	public int mNationalOptionCharacterSubset;

	public byte[] mData;      //size (25+1)*40
	public byte[] mAttribute; //size (25+1)*40
	public byte[] mbUpdated;  //size (32)

	public int mActivePositionRow;
	public int mActivePositionColumn;
}

