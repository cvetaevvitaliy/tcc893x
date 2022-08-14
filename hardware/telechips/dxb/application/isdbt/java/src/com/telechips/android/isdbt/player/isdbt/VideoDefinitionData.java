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

public class VideoDefinitionData
{
	public VideoDefinitionData()
	{
	}
	public int DisplayID;

	public int main_DecoderID;
	public int width;
	public int height;
	public int aspect_ratio;
	public int frame_rate;
	public int progressive;

	public int sub_DecoderID;
	public int sub_width;
	public int sub_height;
	public int sub_aspect_ratio;
}

