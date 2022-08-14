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

import android.util.Log;

import com.telechips.android.dvb.player.dvb.DVBTPlayer;

public class DiSEqCDev {

	private final DiSEqC         mDiseqc;
	private final DiSEqCDevLNB   mLnb;
	private final DiSEqCDevMotor mMotor;

	public DiSEqCDev() {
		mDiseqc = new DiSEqC();
		mLnb = new DiSEqCDevLNB(mDiseqc);
		mMotor = new DiSEqCDevMotor(mDiseqc);
	}

	public void setPlayer(DVBTPlayer player) {
		DxbLog(D, "setPlayer()");
		mDiseqc.SetPlayer(player);
	}
	
	public boolean Execute(Tuner tune, Dish dish) {
		DxbLog(D, "Execute()");
		boolean success = (dish != null);
		if (success) {
			success = mLnb.Execute(tune, dish);
		}
		if (success) {
			success = mMotor.Execute(tune, dish);
		}
		return success;
	}

	public boolean ExecuteMotorUSALS(int sat_long) {
		DxbLog(D, "ExecuteMotorUSALS()");
		return mMotor.ExecuteUSALS(sat_long, 1);
	}

	public boolean ExecuteMotorDriveEast(int steps) {
		DxbLog(D, "ExecuteMotorDriveEast(" + steps + ")");
		return mMotor.SendDriveEastCMD(steps, 1);
	}

	public boolean ExecuteMotorDriveWest(int steps) {
		DxbLog(D, "ExecuteMotorDriveWest(" + steps + ")");
		return mMotor.SendDriveWestCMD(steps, 1);
	}

	public boolean ExecuteMotorDriveStop() {
		DxbLog(D, "ExecuteMotorDriveStop()");
		return mMotor.SendDriveStopCMD(1);
	}

	public boolean ExecuteMotorGotoX(double azimuth) {
		DxbLog(D, "ExecuteMotorGotoX(" + azimuth + ")");
		return mMotor.SendGotoXCMD(azimuth, 1);
	}

	public boolean ExecuteMotorGotoNN(int index) {
		DxbLog(D, "ExecuteMotorGotoNN(" + index + ")");
		return mMotor.SendGotoPosCMD(index, 1);
	}

	public boolean ExecuteMotorStoreNN(int index) {
		DxbLog(D, "ExecuteMotorStoreNN(" + index + ")");
		return mMotor.SendStorePosCMD(index, 1);
	}

	static public int getIntermediateFrequency(Tuner tune, Dish dish) {
		int abs_freq = tune.getFrequency();
		int lof = (dish.IsHighBand(tune)) ? dish.getLnbHiLof() : dish.getLnbLoLof();
		return (lof > abs_freq) ? (lof - abs_freq) : (abs_freq - lof);
	}

/*****************************************************************************************************************************************************************************
 *	Debug - log_message
 *****************************************************************************************************************************************************************************/
	private static final int D = 0;
	private static final int E = 1;
	private static final int I = 2;
	private static final int V = 3;
	private static final int W = 4;

	private final String TAG = "[[[" + getClass().getSimpleName() + "]]]";

	private void DxbLog(int level, String mString) {
		switch(level) {
		case E:
			Log.e(TAG, mString);
			break;
		case I:
			Log.i(TAG, mString);
			break;
		case V:
			Log.v(TAG, mString);
			break;
		case W:
			Log.w(TAG, mString);
			break;
		case D:
		default:
			//Log.d(TAG, mString);
			break;
		}
	}
}
