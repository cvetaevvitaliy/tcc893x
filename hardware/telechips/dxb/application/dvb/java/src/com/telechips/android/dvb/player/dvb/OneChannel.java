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

public class OneChannel
{
	public String name;
	public int frequency1; // primary frequency in Khz
	public int frequency2; // alternative frequency
	public int finetune; // finetune offset in Khz, 0=no finetune
	public int bandwidth1; // 6/7/8 Mhz
	public int bandwidth2; // alternative bandwidth
	public int alt_selected; // alternative selected or not
	public int fec_inner;
};