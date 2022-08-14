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

public class Dish {

	public final static int LNB_TYPE_SINGLE      = 0;
	public final static int LNB_TYPE_UNIVERSAL   = 1;

	public final static int LNB_POWER_OFF        = 0;
	public final static int LNB_POWER_13V        = 1;
	public final static int LNB_POWER_18V        = 2;
	public final static int LNB_POWER_AUTO       = 3;

	public final static int LNB_TONE_OFF         = 0;
	public final static int LNB_TONE_22K         = 1;
	public final static int LNB_TONE_AUTO        = 2;

	public final static int LNB_TONEBURST_NONE   = 0;
	public final static int LNB_TONEBURST_A      = 1;
	public final static int LNB_TONEBURST_B      = 2;

	public final static int LNB_DISEQC_NONE      = 0;
	public final static int LNB_DISEQC_1_0       = 1;
	public final static int LNB_DISEQC_1_1       = 2;
	public final static int LNB_DISEQC_1_2       = 3;
	public final static int LNB_DISEQC_1_3       = 4;

	public final static int LNB_SEQUENCE_NONE    = 0;
	public final static int LNB_SEQUENCE_1       = 1;
	public final static int LNB_SEQUENCE_2       = 2;
	public final static int LNB_SEQUENCE_3       = 3;
	public final static int LNB_SEQUENCE_4       = 4;
	
	public final static int LNB_REPEAT_OFF       = 0;
	public final static int LNB_REPEAT_ON        = 1;

	private int    id;
//	private int    sat_id;
	private String sat_name;
	private int    sat_longitude;
	private int    lnb_no;
	private int    lnb_type;
	private int    lnb_lof_lo;
	private int    lnb_lof_hi;
	private int    lnb_lof_th;
	private int    lnb_power;
	private int    lnb_tone;
	private int    lnb_toneburst;
    private int    lnb_diseqc_mode;
	private int    lnb_diseqc_config10;
	private int    lnb_diseqc_config11;
	private int    lnb_cmd_sequence;
	private int    motor_diseqc_mode;
	private int    motor_id;
	private int    motor_position;
    private int    diseqc_repeat;
	private int    diseqc_fast;

	public Dish() {
		id = 0;
//		sat_id = 0;
		sat_name = null;
		sat_longitude = 0;
		lnb_no = 0;
		lnb_type = LNB_TYPE_SINGLE;
		lnb_lof_lo = 0;
		lnb_lof_hi = 0;
		lnb_lof_th = 0;
		lnb_power = LNB_POWER_OFF;
		lnb_tone = LNB_TONE_OFF;
		lnb_toneburst = LNB_TONEBURST_NONE;
		lnb_diseqc_mode = LNB_DISEQC_NONE;
		lnb_diseqc_config10 = 0;
		lnb_diseqc_config11 = 0;
		lnb_cmd_sequence = LNB_SEQUENCE_NONE;
		motor_diseqc_mode = LNB_DISEQC_NONE;
		motor_id = 0;
		motor_position = 0;
		diseqc_repeat = 1;
		diseqc_fast = 0;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId() {
		return this.id;
	}

//	public void setSatId(int id) {
//		this.sat_id = id;
//	}
//
//	public int getSatId() {
//		return this.sat_id;
//	}

	public void setSatName(String name) {
	    this.sat_name = name;
	}

	public String getSatName() {
	    return this.sat_name;
	}

	public void setSatLongitude(int pos) {
		this.sat_longitude = pos;
	}

	public int getSatLongitude() {
		return this.sat_longitude;
	}

	public void setLnbNo(int no) {
		this.lnb_no = no;
	}

	public int getLnbNo() {
		return this.lnb_no;
	}

	public void setLnbType(int type) {
		this.lnb_type = type;
	}

	public int getLnbType() {
		return this.lnb_type;
	}

	public void setLnbLoLof(int fre) {
		this.lnb_lof_lo = fre;
	}

	public int getLnbLoLof() {
		return this.lnb_lof_lo;
	}

	public void setLnbHiLof(int fre) {
		this.lnb_lof_hi = fre;
	}	

	public int getLnbHiLof(){
		return this.lnb_lof_hi;
	}	

	public void setLnbLofThreshold(int fre) {
		this.lnb_lof_th = fre;
	}	

	public int getLnbLofThreshold() {
		return this.lnb_lof_th;
	}

	public void setLnbPower(int mode) {
		this.lnb_power = mode;
	}

	public int getLnbPower(){
		return this.lnb_power;
	}

	public void setLnbTone(int mode) {
		this.lnb_tone = mode;
	}

	public int getLnbTone(){
		return this.lnb_tone;
	}

	public void setLnbToneburst(int type) {
		this.lnb_toneburst = type;
	}

	public int getLnbToneburst() {
		return this.lnb_toneburst;
	}

	public void setLnbDiseqcMode(int mode) {
		this.lnb_diseqc_mode = mode;
	}

	public int getLnbDiseqcMode() {
		return this.lnb_diseqc_mode;
	}

	public void setLnbDiseqcConfig10(int lnb) {
		this.lnb_diseqc_config10 = lnb;	
	}

	public int getLnbDiseqcConfig10() {
		return this.lnb_diseqc_config10;
	}

	public void setLnbDiseqcConfig11(int lnb) {
		this.lnb_diseqc_config11 = lnb;	
	}

	public int getLnbDiseqcConfig11() {
		return this.lnb_diseqc_config11;
	}

	public void setLnbSequence(int mode) {
		this.lnb_cmd_sequence = mode;	
	}

	public int getLnbSequence() {
		return this.lnb_cmd_sequence;
	}

	public void setMotorDiseqcMode(int mode) {
		this.motor_diseqc_mode = mode;	
	}

	public int getMotorDiseqcMode() {
		return this.motor_diseqc_mode;
	}

	public void setMotorId(int id) {
	    this.motor_id = id;
	}

	public int getMotorId() {
	    return this.motor_id;
	}

	public void setMotorPosition(int pos) {
		this.motor_position = pos;
	}

	public int getMotorPosition() {
		return this.motor_position;
	}

	public void setDiseqcRepeat(int mode) {
		this.diseqc_repeat = mode;	
	}

	public int getDiseqcRepeat(){
		return this.diseqc_repeat;
	}

	public void setFastDiseqc(int mode) {
		this.diseqc_fast = mode;	
	}

	public int getFastDiseqc() {
		return this.diseqc_fast;
	}

	public boolean IsHighBand(Tuner tune) {
		if (getLnbType() == LNB_TYPE_SINGLE)
			return false;
		if (tune == null)
			return false;
		return (tune.getFrequency() > getLnbLofThreshold());
	}

	public boolean IsHorizontal(Tuner tune) {
		if (tune == null) {
			return true;
		}
		int pol = tune.getPolarization();
		return (pol == Tuner.POL_H || pol == Tuner.POL_L) ^ IsPolarityInverted();
	}

	private boolean IsPolarityInverted() {
		return false;
	}
}
