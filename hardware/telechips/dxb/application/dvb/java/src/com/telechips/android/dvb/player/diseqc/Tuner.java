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

package com.telechips.android.dvb.player.diseqc;

public class Tuner
{
	public static final int POL_H = 0;
	public static final int POL_V = 1;
	public static final int POL_L = 2;

	private int id;
	private int ts_id;
	private int sat_id;
	private int frequency;
	private int symbol;
	private int polarization;
	private int fec_inner;

	public Tuner() {
		id = 0;
		ts_id = 0;
		sat_id = 0;
		frequency = 0;
		symbol = 0;
		polarization = POL_H;
		fec_inner = 0;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId() {
		return this.id;
	}

	public void setTsId(int id) {
		this.ts_id = id;
	}
	
	public int getTsId() {
		return this.ts_id;
	}
	
	public void setSatId(int id) {
		this.sat_id = id;
	}
	
	public int getSatId() {
		return this.sat_id;
	}
	
	public void setFrequency(int fre) {
		this.frequency = fre;
	}
	
	public int getFrequency() {
		return this.frequency;
	}
	
	public void setSymbolRate(int sym) {
		this.symbol = sym;
	}
	
	public int getSymbolRate() {
		return this.symbol;
	}
	
	public void setPolarization(int pol) {
		this.polarization = pol;
	}
	
	public int getPolarization() {
		return this.polarization;
	}

	public void setFecInner(int fec) {
		this.fec_inner = fec;
	}
	
	public int getFecInner() {
		return this.fec_inner;
	}
}
