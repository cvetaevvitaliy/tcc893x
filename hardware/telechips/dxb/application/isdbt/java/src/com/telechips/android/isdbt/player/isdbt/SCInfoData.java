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

/*
	SCDataSize - total size of SCData of byte

			syntax						no. of byte		comment
    -------------------------------------------------------------------------------
	SCData {
		protocol_unit_number					1
		unit_len								1
		IC_card_instruction					2
		Return_code							2
		Number_of_card_IDs					1			(Number_of_card_IDs <= 8)
		dummy								1
		for (i=1; i <= Number_of_card_IDs; i++) {				i=1: individual card ID, i >=2: Group ID
			Card_Type {
				Manufacturer_Identifier			1
				version						1
			}
			Card_ID {
				dummy						1
				ID_Identifier					1
				ID[6]						1*6 
			}
			Check_Code						2
		}
		SW1_SW2							2
  */
public class SCInfoData {
	//B-CAS Smart Card TCC Error 
	public static final int SC_ERR_OK				= 0;
	public static final int SC_ERR_INVALID_NAD		= 1;
	public static final int SC_ERR_RECEIVE_EDC		= 2;
	public static final int SC_ERR_RECEIVE_OTHER	= 3;
	public static final int SC_ERR_PROTOCOL_TYPE	= 4;
	public static final int SC_ERR_CMD_FAIL			= 5;
	public static final int SC_ERR_OPEN				= 6;
	public static final int SC_ERR_CLOSE			= 7;
	public static final int SC_ERR_RESET			= 8;
	public static final int SC_ERR_GET_ATR			= 9;
	public static final int SC_ERR_CARD_DETECT		= 10;
	
	public SCInfoData() {
	}
	public int SCDataSize;
 	public byte[] SCData;
}


