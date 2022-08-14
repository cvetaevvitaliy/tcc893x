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

import com.telechips.android.dvb.player.dvb.DVBTPlayer;

import android.util.Log;

import android.os.SystemClock;

public class DiSEqC {

	// Framing byte
	public static final byte DISEQC_FRM                     = (byte)0xe0;
	public static final byte DISEQC_FRM_REPEAT              = (byte)(1 << 0);
	public static final byte DISEQC_FRM_REPLY_REQ           = (byte)(1 << 1);
	public static final byte DISEQC_FRM_REPLY_OK            = (byte)0x00;
	public static final byte DISEQC_FRM_REPLY_NOT_SUPPORT   = (byte)0x01;
	public static final byte DISEQC_FRM_REPLY_PARITY_ERR    = (byte)0x02;
	public static final byte DISEQC_FRM_REPLY_NOTRECOGNISED = (byte)0x03;

	// Address byte
	public static final byte DISEQC_ADR_ALL                 = (byte)0x00;
	public static final byte DISEQC_ADR_SW_ALL              = (byte)0x10;
	public static final byte DISEQC_ADR_LNB                 = (byte)0x11;
	public static final byte DISEQC_ADR_LNB_SW              = (byte)0x12;
	public static final byte DISEQC_ADR_SW_BLK              = (byte)0x14;
	public static final byte DISEQC_ADR_SW                  = (byte)0x15;
	public static final byte DISEQC_ADR_SMATV               = (byte)0x18;
	public static final byte DISEQC_ADR_POL_ALL             = (byte)0x20;
	public static final byte DISEQC_ADR_POL_LIN             = (byte)0x21;
	public static final byte DISEQC_ADR_POS_ALL             = (byte)0x30;
	public static final byte DISEQC_ADR_POS_AZ              = (byte)0x31;
	public static final byte DISEQC_ADR_POS_EL              = (byte)0x32;

	// Command byte
	public static final byte DISEQC_CMD_RESET               = (byte)0x00; // M v1.0,   3 bytes
	public static final byte DISEQC_CMD_CLR_RESET           = (byte)0x01; // R v2.0,   3 bytes
	public static final byte DISEQC_CMD_STANDBY             = (byte)0x02; // R v1.0,   3 bytes
	public static final byte DISEQC_CMD_POWER_ON            = (byte)0x03; // R v1.0,   3 bytes
	public static final byte DISEQC_CMD_SET_CONTEND         = (byte)0x04; // S v2.0,   3 bytes
	public static final byte DISEQC_CMD_CONTEND             = (byte)0x05; // S v2.0,   3 bytes,    address
	public static final byte DISEQC_CMD_CLR_CONTEND         = (byte)0x06; // R v2.0,   3 bytes
	public static final byte DISEQC_CMD_ADDRESS             = (byte)0x07; // S v2.0,   3 bytes,    address
	public static final byte DISEQC_CMD_MOVE_C              = (byte)0x08; // S v2.0,   4 bytes
	public static final byte DISEQC_CMD_MOVE                = (byte)0x09; // R v2.0,   4 bytes

	public static final byte DISEQC_CMD_STATUS              = (byte)0x10; // R v2.0,   3 bytes,     status
	public static final byte DISEQC_CMD_CONFIG              = (byte)0x11; // R v2.0,   3 bytes,     config
	
	public static final byte DISEQC_CMD_SWITCH_0            = (byte)0x14; // R v2.0,   3 bytes,   switches
	public static final byte DISEQC_CMD_SWITCH_1            = (byte)0x15; // R v2.0,   3 bytes,   switches
	public static final byte DISEQC_CMD_SWITCH_2            = (byte)0x16; //       ,   3 bytes
	public static final byte DISEQC_CMD_SWITCH_3            = (byte)0x17; //       ,   3 bytes

	public static final byte DISEQC_CMD_SET_LO              = (byte)0x20; // R v1.0,   3 bytes
	public static final byte DISEQC_CMD_SET_VR              = (byte)0x21; // R v1.0,   3 bytes
	public static final byte DISEQC_CMD_SET_POS_A           = (byte)0x22; // R v1.0,   3 bytes
	public static final byte DISEQC_CMD_SET_S0A             = (byte)0x23; // R v1.0,   3 bytes
	public static final byte DISEQC_CMD_SET_HI              = (byte)0x24; // R v1.0,   3 bytes
	public static final byte DISEQC_CMD_SET_HL              = (byte)0x25; // R v1.0,   3 bytes
	public static final byte DISEQC_CMD_SET_POS_B           = (byte)0x26; // R v1.0,   3 bytes
	public static final byte DISEQC_CMD_SET_S0B             = (byte)0x27; // R v1.0,   3 bytes
	public static final byte DISEQC_CMD_SET_S1A             = (byte)0x28; // R v1.1,   3 bytes
	public static final byte DISEQC_CMD_SET_S2A             = (byte)0x29; // R v1.1,   3 bytes
	public static final byte DISEQC_CMD_SET_S3A             = (byte)0x2a; // R v1.1,   3 bytes
	public static final byte DISEQC_CMD_SET_S4A             = (byte)0x2b; // R v1.1,   3 bytes
	public static final byte DISEQC_CMD_SET_S1B             = (byte)0x2c; // R v1.1,   3 bytes
	public static final byte DISEQC_CMD_SET_S2B             = (byte)0x2d; // R v1.1,   3 bytes
	public static final byte DISEQC_CMD_SET_S3B             = (byte)0x2e; // R v1.1,   3 bytes
	public static final byte DISEQC_CMD_SET_S4B             = (byte)0x2f; // R v1.1,   3 bytes
	public static final byte DISEQC_CMD_SLEEP               = (byte)0x30; //       ,   3 bytes
	public static final byte DISEQC_CMD_AWAKE               = (byte)0x31; //       ,   3 bytes

	public static final byte DISEQC_CMD_WRITE_N0            = (byte)0x38; // M v1.0,   4 bytes
	public static final byte DISEQC_CMD_WRITE_N1            = (byte)0x39; // M v1.1,   4 bytes
	public static final byte DISEQC_CMD_WRITE_N2            = (byte)0x3a; //       ,   4 bytes
	public static final byte DISEQC_CMD_WRITE_N3            = (byte)0x3b; //       ,   4 bytes

	public static final byte DISEQC_CMD_READ_A0             = (byte)0x40; // S v2.0,   3 bytes, byte value
	public static final byte DISEQC_CMD_READ_A1             = (byte)0x41; // S v2.0,   3 bytes, byte value

	public static final byte DISEQC_CMD_WRITE_A0            = (byte)0x48; // R v1.2,   4 bytes
	public static final byte DISEQC_CMD_WRITE_A1            = (byte)0x49; // S v1.2,   4 bytes

	public static final byte DISEQC_CMD_LO_STRING           = (byte)0x50; // S v2.0,   3 bytes,  BCD bytes
	public static final byte DISEQC_CMD_LO_NOW              = (byte)0x51; // R v2.0,   3 bytes,   F number
	public static final byte DISEQC_CMD_LO_LO               = (byte)0x52; // S v2.0,   3 bytes,   F number
	public static final byte DISEQC_CMD_LO_HI               = (byte)0x53; // S v2.0,   3 bytes,   F number

	public static final byte DISEQC_CMD_WRITE_FREQ          = (byte)0x58; // M v1.1, 5/6 bytes
	public static final byte DISEQC_CMD_CHANNEL_NUMBER      = (byte)0x59; //       ,   5 bytes
	public static final byte DISEQC_CMD_HALT                = (byte)0x60; // M v1.2,   3 bytes
	public static final byte DISEQC_CMD_LMT_OFF             = (byte)0x63; // M v1.2,   3 bytes
	public static final byte DISEQC_CMD_POS_STAT            = (byte)0x64; // R v2.2,   3 bytes, pos status
	public static final byte DISEQC_CMD_LMT_E               = (byte)0x66; // M v1.2,   3 bytes
	public static final byte DISEQC_CMD_LMT_W               = (byte)0x67; // M v1.2,   3 bytes
	public static final byte DISEQC_CMD_DRIVE_E             = (byte)0x68; // M v1.2,   4 bytes
	public static final byte DISEQC_CMD_DRIVE_W             = (byte)0x69; // M v1.2,   4 bytes
	public static final byte DISEQC_CMD_STORE_POS           = (byte)0x6a; // M v1.2,   4 bytes
	public static final byte DISEQC_CMD_GOTO_POS            = (byte)0x6b; // M v1.2,   4 bytes
	public static final byte DISEQC_CMD_GOTO_X              = (byte)0x6e; //       ,   4 bytes
	public static final byte DISEQC_CMD_SET_POSNS           = (byte)0x6f; // R v1.2, 4/6 bytes

	public static final int DISEQC_SHORT_WAIT     = 15;
	public static final int DISEQC_LONG_WAIT      = 100;
	public static final int DISEQC_POWER_OFF_WAIT = 1000;
	public static final int DISEQC_POWER_ON_WAIT  = 500;

	private static final int TIMEOUT_RETRIES       = 1;
	private static final int TIMEOUT_WAIT          = 250;

	public static final int SEC_VOLTAGE_13 = 0;
	public static final int SEC_VOLTAGE_18 = 1;
	public static final int SEC_VOLTAGE_OFF = 2;

	public static final int SEC_TONE_ON    = 0;
	public static final int SEC_TONE_OFF   = 1;

	public static final int SEC_MINI_A     = 0;
	public static final int SEC_MINI_B     = 1;

	private DVBTPlayer mPlayer;

	public DiSEqC() {
	}
	
	public void SetPlayer(DVBTPlayer player) {
		synchronized(this) {
			mPlayer = player;
		}
	}

	public boolean SetVoltage(int voltage) {
		boolean success = false;

		for (int retry = 0; !success && retry < TIMEOUT_RETRIES; retry++) {
			if (retry != 0) {
				SystemClock.sleep(TIMEOUT_WAIT);
			}
			synchronized(this) {
				if (mPlayer == null) {
					DxbLog(E, "Failed to set voltage : Player is not Ready!");
					return false;
				}
				if (mPlayer.SetLnbVoltage(voltage) == 0) {
					SystemClock.sleep(DISEQC_SHORT_WAIT);
					success = true;
				}
			}
		}

		if (!success)
			DxbLog(E, "Failed to set voltage!");

		return success;
	}

	public boolean SetTone(int on) {
		boolean success = false;

		for (int retry = 0; !success && retry < TIMEOUT_RETRIES; retry++) {
			if (retry != 0) {
				SystemClock.sleep(TIMEOUT_WAIT);
			}
			synchronized(this) {
				if (mPlayer == null) {
					DxbLog(E, "Setting Tone Switch failed.(mPlayer is null)!");
					return false;
				}
				if (mPlayer.SetTone(on) == 0) {
					SystemClock.sleep(DISEQC_SHORT_WAIT);
					success = true;
				}
			}
		}

		if (!success)
			DxbLog(E, "Setting Tone Switch failed.");

		return success;
	}

	public boolean SetToneburst(int pos) {
		boolean success = false;

		for (int retry = 0; !success && retry < TIMEOUT_RETRIES; retry++) {
			if (retry != 0) {
				SystemClock.sleep(TIMEOUT_WAIT);
			}
			synchronized(this) {
				if (mPlayer == null) {
					DxbLog(E, "Failed to set mini diseqc : Player is not Ready!");
					return false;
				}
				if (mPlayer.DiseqcSendBurst(pos) == 0) {
					SystemClock.sleep(DISEQC_SHORT_WAIT);
					success = true;
				}
			}
		}

		if (!success)
			DxbLog(E, "Failed to set mini diseqc!");

		return success;
	}

	public boolean SendCommand(byte addr, byte cmd, int repeats, int data_len, byte[] data) {
		// check payload validity
		if (data_len > 3 || (data_len > 0 && data.length == 0)) {
			DxbLog(E, "Bad DiSEqC command");
			return false;
		}

		// prepare command
		byte[] msg = new byte[data_len+3];
		msg[0] = DISEQC_FRM;
		msg[1] = addr;
		msg[2] = cmd;
		
		for (int i = 0; i < data_len; i++)
			msg[i+3] = data[i];

		// send the command
		for (int i = 0; i <= repeats; i++) {
			if (!SendDiseqcCMD(msg)) {
				DxbLog(E, "DiSEqC command failed");
				return false;
			}

			msg[0] |= DISEQC_FRM_REPEAT;
			SystemClock.sleep(DISEQC_SHORT_WAIT);
		}

		return true;
	}

	private boolean SendDiseqcCMD(byte[] msg) {
		boolean success = false;

		for (int retry = 0; !success && (retry < TIMEOUT_RETRIES); retry++) {
			if (retry != 0) {
				SystemClock.sleep(TIMEOUT_WAIT);
			}
			synchronized(this) {
				if (mPlayer == null) {
					DxbLog(E, "Failed to send diseqc cmd : Player is not Ready!");
					return false;
				}
				if (mPlayer.DiseqcSendCMD(msg) == 0) {
					SystemClock.sleep(DISEQC_SHORT_WAIT);
					success = true;
				}
			}
		}

		if (!success)
			DxbLog(E, "send_diseqc FE_DISEQC_SEND_MASTER_CMD failed");

		return success;
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
