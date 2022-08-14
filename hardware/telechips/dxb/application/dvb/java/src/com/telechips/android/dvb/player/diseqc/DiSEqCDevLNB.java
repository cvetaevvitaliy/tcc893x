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

public class DiSEqCDevLNB {

	private final DiSEqC mDiseqc;

	public DiSEqCDevLNB(DiSEqC diseqc) {
		mDiseqc = diseqc;
	}

	public boolean Execute(Tuner tune, Dish dish) {
		if (SetPower(tune, dish) == false)
			return false;

		switch (dish.getLnbSequence()) {
			case Dish.LNB_SEQUENCE_1: // committed -> uncommitted -> toneburst
				if (SendCommittedCMD(tune, dish) == false)
					return false;
				if (SendUncommittedCMD(dish) == false)
					return false;
				if (SetToneburst(dish) == false)
					return false;
				break;
			case Dish.LNB_SEQUENCE_2: // toneburst -> committed -> uncommitted
				if (SetToneburst(dish) == false)
					return false;
				if (SendCommittedCMD(tune, dish) == false)
					return false;
				if (SendUncommittedCMD(dish) == false)
					return false;
				break;
			case Dish.LNB_SEQUENCE_3: // uncommitted -> committed -> toneburst
				if (SendUncommittedCMD(dish) == false)
					return false;
				if (SendCommittedCMD(tune, dish) == false)
					return false;
				if (SetToneburst(dish) == false)
					return false;
				break;
			case Dish.LNB_SEQUENCE_4: // toneburst -> uncommitted -> committed
				if (SetToneburst(dish) == false)
					return false;
				if (SendUncommittedCMD(dish) == false)
					return false;
				if (SendCommittedCMD(tune, dish) == false)
					return false;
				break;
			default:
				break;
		}

		if (SetTone(tune, dish) == false)
			return false;

		return true;
	}

	private boolean SetPower(Tuner tune, Dish dish) {
		switch (dish.getLnbPower()) {
			case Dish.LNB_POWER_OFF:
				DxbLog(D, "SetPower OFF");
				return mDiseqc.SetVoltage(DiSEqC.SEC_VOLTAGE_OFF);
			case Dish.LNB_POWER_13V:
				DxbLog(D, "SetPower 13V");
				return mDiseqc.SetVoltage(DiSEqC.SEC_VOLTAGE_13);
			case Dish.LNB_POWER_18V:
				DxbLog(D, "SetPower 18V");
				return mDiseqc.SetVoltage(DiSEqC.SEC_VOLTAGE_18);
			case Dish.LNB_POWER_AUTO:
				boolean isHorizontal = dish.IsHorizontal(tune);
				if (isHorizontal) {
					DxbLog(D, "SetPower AUTO (18V)");
					return mDiseqc.SetVoltage(DiSEqC.SEC_VOLTAGE_18);
				} else {
					DxbLog(D, "SetPower AUTO (13V)");
					return mDiseqc.SetVoltage(DiSEqC.SEC_VOLTAGE_13);
				}
			default:
				DxbLog(E, "SetPower Error");
				break;
		}
		return false;
	}

	private boolean SetTone(Tuner tune, Dish dish) {
		switch (dish.getLnbTone()) {
			case Dish.LNB_TONE_OFF:
				DxbLog(D, "SetTone OFF");
				return mDiseqc.SetTone(DiSEqC.SEC_TONE_OFF);
			case Dish.LNB_TONE_22K:
				DxbLog(D, "SetTone ON");
				return mDiseqc.SetTone(DiSEqC.SEC_TONE_ON);
			case Dish.LNB_TONE_AUTO:
				boolean isHigh = dish.IsHighBand(tune);
				if (isHigh) {
					DxbLog(D, "SetTone AUTO (ON)");
					return mDiseqc.SetTone(DiSEqC.SEC_TONE_ON);
				} else {
					DxbLog(D, "SetTone AUTO (OFF)");
					return mDiseqc.SetTone(DiSEqC.SEC_TONE_OFF);
				}
			default:
				DxbLog(E, "SetTone Error");
				break;
		}
		return false;
	}

	private boolean SetToneburst(Dish dish) {
		switch (dish.getLnbToneburst()) {
			case Dish.LNB_TONEBURST_A:
				DxbLog(D, "SetToneburst MINI-A");
				return mDiseqc.SetToneburst(DiSEqC.SEC_MINI_A);
			case Dish.LNB_TONEBURST_B:
				DxbLog(D, "SetToneburst MINI-B");
				return mDiseqc.SetToneburst(DiSEqC.SEC_MINI_B);
			case Dish.LNB_TONEBURST_NONE:
				DxbLog(D, "SetToneburst NONE");
				return true;
			default:
				DxbLog(E, "SetToneburst Error");
				break;
		}
		return false;
	}

	private boolean SendCommittedCMD(Tuner tune, Dish dish) {
		int pos = dish.getLnbDiseqcConfig10() - 1;
		DxbLog(D, "SendCommittedCMD pos = " + pos);
		if (0 <= pos && pos <= 4) {
			byte[] data = new byte[1];
			data[0] = (byte)((pos << 2) | (dish.IsHorizontal(tune) ? 0x02 : 0x00) | (dish.IsHighBand(tune)  ? 0x01 : 0x00));
			data[0] |= 0xf0;
			return mDiseqc.SendCommand(DiSEqC.DISEQC_ADR_SW_ALL, DiSEqC.DISEQC_CMD_WRITE_N0, dish.getDiseqcRepeat(), 1, data);
		}
		return true;
	}

	private boolean SendUncommittedCMD(Dish dish) {
		int pos = dish.getLnbDiseqcConfig11() - 1;
		DxbLog(D, "SendUncommittedCMD pos = " + pos);
		if (0 <= pos && pos <= 15) {
			byte[] data = new byte[1];
			data[0] = (byte)(pos);
			data[0] |= 0xf0;
			return mDiseqc.SendCommand(DiSEqC.DISEQC_ADR_SW_ALL, DiSEqC.DISEQC_CMD_WRITE_N1, dish.getDiseqcRepeat(), 1, data);
		}
		return true;
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
