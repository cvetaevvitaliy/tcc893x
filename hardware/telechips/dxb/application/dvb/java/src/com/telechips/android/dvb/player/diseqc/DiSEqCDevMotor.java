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

import com.telechips.android.dvb.player.diseqc.Motor;
import android.util.Log;

import java.lang.Math;

public class DiSEqCDevMotor {

	private final DiSEqC mDiseqc;

	public DiSEqCDevMotor(DiSEqC diseqc) {
		mDiseqc = diseqc;
	}

	public boolean Execute(Tuner tune, Dish dish) {
		switch (dish.getMotorDiseqcMode()) {
			case Dish.LNB_DISEQC_1_2:
				return ExecuteDiSEqC(dish);
			case Dish.LNB_DISEQC_1_3:
				//return ExecuteUSALS(dish);
				return ExecuteDiSEqC(dish);
			case Dish.LNB_DISEQC_NONE:
				return true;
			default:
				break;
		}
		return false;
	}

	private boolean ExecuteDiSEqC(Dish dish) {
		int pos = dish.getId(); //dish.getMotorPosition();
		if (pos > 0) {
			return SendGotoPosCMD(pos, dish.getDiseqcRepeat());
		} else {
			DxbLog(E, "ExecuteDiSEqC() pos = " + pos);
		}
		return false;
	}

	private boolean ExecuteUSALS(Dish dish) {
		int angle = dish.getSatLongitude();
		double azimuth = CalculateAzimuth(angle);
		return SendGotoXCMD(azimuth, dish.getDiseqcRepeat());
	}

	private double CalculateAzimuth(int angle) {
/*
		double P  = Dish.mLatitude * (Math.PI / 180.0);
		double Ue = Dish.mLongitude * (Math.PI / 180.0);

		// Satellite Longitude in radians
		double Us = angle * (Math.PI / 180.0);

		return (180.0 / Math.PI) * Math.atan( Math.tan(Us - Ue) / Math.sin(P) );
*/
		double diff = angle/10.0 - Motor.getAngle_Longitude()/10.0;
		while (diff > 180.0) diff -= 180.0;
		while (diff <= -180.0) diff += 180.0;

		double P  = Motor.getAngle_Latitude()/10.0 * (Math.PI / 180.0);
		double U = diff * (Math.PI / 180.0);

		double A, B, C, D, E, F, G;
		double x1, y1, z1, x2, y2, z2, x3, y3, z3;
		double i = 6378, k = 35786 + 6378;

		// antenna
		x1 = 0;
		y1 = i*Math.cos(P);
		z1 = i*Math.sin(P);

		// satellite
		x2 = k*Math.sin(U);
		y2 = k*Math.cos(U);
		z2 = 0;

		x3 = 0;
		y3 = k;
		z3 = 0;

		E = Math.pow(x3-x1, 2) + Math.pow(y3-y1, 2) + Math.pow(z3-z1, 2);
		F = Math.pow(x2-x1, 2) + Math.pow(y2-y1, 2) + Math.pow(z2-z1, 2);
		G = Math.pow(x2-z3, 2) + Math.pow(y2-y3, 2) + Math.pow(z2-z3, 2);
		C = E + F - G;
		D = Math.pow(E * F, 0.5);
		B = 0.5 * C / D;
		A = (180.0 / Math.PI) * Math.acos(B);

		if (diff < 0 )
		{
			DxbLog(I, "CalculateAzimuth: W" + A + ", Lat: " + Motor.getAngle_Latitude() + ", Lon: " + Motor.getAngle_Longitude() + ", Ang: " + angle);
			return -A;
		}

		DxbLog(I, "CalculateAzimuth: E" + A + ", Lat: " + Motor.getAngle_Latitude() + ", Lon: " + Motor.getAngle_Longitude() + ", Ang: " + angle);
		return A;
	}

	public boolean ExecuteUSALS(int sat_long, int repeat) {
		double azimuth = CalculateAzimuth(sat_long);
		return SendGotoXCMD(azimuth, repeat);
	}

	public boolean SendGotoPosCMD(int index, int repeat) {
		DxbLog(D, "SendGotoPosCMD index = " + index);
		byte[] data = new byte[1];
		data[0] = (byte)(index);
		return mDiseqc.SendCommand(DiSEqC.DISEQC_ADR_POS_AZ, DiSEqC.DISEQC_CMD_GOTO_POS, repeat, 1, data);
	}

	public boolean SendStorePosCMD(int index, int repeat) {
		DxbLog(D, "SendStorePosCMD index = " + index);
		byte[] data = new byte[1];
		data[0] = (byte)(index);
		return mDiseqc.SendCommand(DiSEqC.DISEQC_ADR_POS_AZ, DiSEqC.DISEQC_CMD_STORE_POS, repeat, 1, data);
	}

	public boolean SendGotoXCMD(double azimuth, int repeat) {
		DxbLog(D, "SendGotoXCMD azimuth = " + azimuth);
		int az16 = (int) (Math.abs(azimuth) * 16.0);
		byte[] data = new byte[2];
		data[0] = (byte)(((azimuth > 0.0) ? 0xE0 : 0xD0) | ((az16 >> 8)  &0x0f));
		data[1] = (byte)(az16 & 0xff);
		return mDiseqc.SendCommand(DiSEqC.DISEQC_ADR_POS_AZ, DiSEqC.DISEQC_CMD_GOTO_X, repeat, 2, data);
	}

	public boolean SendDriveStopCMD(int repeat) {
		DxbLog(D, "SendDriveStopCMD");
		return mDiseqc.SendCommand(DiSEqC.DISEQC_ADR_POS_AZ, DiSEqC.DISEQC_CMD_HALT, repeat, 0, null);
	}

	public boolean SendDriveEastCMD(int steps, int repeat) {
		DxbLog(D, "SendDriveEastCMD steps = " + steps);
		byte[] data = new byte[1];
		data[0] = (byte)(steps);
		return mDiseqc.SendCommand(DiSEqC.DISEQC_ADR_POS_AZ, DiSEqC.DISEQC_CMD_DRIVE_E, repeat, 1, data);
	}

	public boolean SendDriveWestCMD(int steps, int repeat) {
		DxbLog(D, "SendDriveWestCMD steps = " + steps);
		byte[] data = new byte[1];
		data[0] = (byte)(steps);
		return mDiseqc.SendCommand(DiSEqC.DISEQC_ADR_POS_AZ, DiSEqC.DISEQC_CMD_DRIVE_W, repeat, 1, data);
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
			Log.d(TAG, mString);
			break;
		}
	}
}
