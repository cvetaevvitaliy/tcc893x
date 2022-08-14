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

package com.telechips.android.tdmb;

public class EWSParser {

    public EWSParser() {
		ews_type = new String[][]
		{
			// 소방방재청
			{"HRA", "\uD638\uC6B0 \uC8FC\uC758\uBCF4"}, 										// Heavy Rain Watch 						"호우 주의보"
			{"HRW", "\uD638\uC6B0 \uACBD\uBCF4"},												// Heavy Rain Warning						"호우 경보"
			{"HSW", "\uB300\uC124 \uC8FC\uC758\uBCF4"}, 										// Heavy Snow Watch						"대설 주의보"
			{"HAS", "\uB300\uC124 \uACBD\uBCF4"},												// Heavy Snow Warning						"대설 경보"
			{"SSA", "\uD3ED\uD48D\uD574\uC77C \uC8FC\uC758\uBCF4"}, 							// Storm Surge Watch						"폭풍해일 주의보"
			{"SSW", "\uD3ED\uD48D\uD574\uC77C \uACBD\uBCF4"},									// Storm Surge Warning						"폭풍해일 경보"
			{"YSW", "\uD669\uC0AC \uACBD\uBCF4"},												// Yellow Sand Warning						"황사 경보"
			{"CWA", "\uD55C\uD30C \uC8FC\uC758\uBCF4"}, 										// Cold Wave Watch							"한파 주의보"
			{"CWW", "\uD55C\uD30C \uACBD\uBCF4"},												// Cold Wave Warning						"한파 경보"
			{"WWW", "\uD48D\uB791 \uACBD\uBCF4"},												// Wind and Waves Warning					"풍랑 경보"
			{"HAW", "\uAC74\uC870 \uACBD\uBCF4"},												// Heavy Arid Warning						"건조 경보"
			{"MFW", "\uC0B0\uBD88 \uACBD\uBCF4"},												// Mountain Fire Warning						"산불 경보"
			{"RTW", "\uAD50\uD1B5 \uD1B5\uC81C"},												// Regulate Traffic Warning					"교통 통제"
		
			// 전국
			{"EAN", "\uAD6D\uAC00 \uBE44\uC0C1 \uC0C1\uD669 \uBC1C\uC0DD"}, 					// Emergency Action Notification (National only)		"국가 비상 상황 발생"
			{"EAT", "\uAD6D\uAC00 \uBE44\uC0C1 \uC0C1\uD669 \uC885\uB8CC"}, 					// Emergency Action Termination (National only)		"국가 비상 상황 종료"
			{"NIC", "\uC911\uC559 \uC7AC\uB09C \uC548\uC804 \uB300\uCC45 \uBCF8\uBD80"},		// National Information Center					"중앙 재난 안전 대책 본부"
			{"NPT", "\uC804\uAD6D\uC801 \uC8FC\uAE30 \uD14C\uC2A4\uD2B8"},						// National Periodic Test						"전국적 주기 테스트"
			{"RMT", "\uC804\uAD6D\uC801 \uC6D4\uBCC4 \uC758\uBB34 \uD14C\uC2A4\uD2B8"},			 // Required Monthly Test						"전국적 월별 의무 테스트"
			{"RWT", "\uC804\uAD6D\uC801 \uC8FC\uAC04\uBCC4 \uC758\uBB34 \uD14C\uC2A4"},			 // Required Weekly Test						"전국적 주간별 의무 테스트"
		
			// 테스트
			{"STT", "\uD2B9\uC218 \uC218\uC2E0\uAE30 \uD14C\uC2A4\uD2B8"},						// Special Terminal Test						"특수 수신기 테스트"
		
			// 지역
			{"ADR", "\uD589\uC815 \uBA54\uC2DC\uC9C0"}, 										// Administrative Message						"행정 메시지"
			{"AVW", "\uC0B0\uC0AC\uD0DC \uACBD\uBCF4"}, 										// Avalanche Warning						"산사태 경보"
			{"AVA", "\uC0B0\uC0AC\uD0DC \uC8FC\uC758\uBCF4"},									// Avalanche Watch							"산사태 주의보"
			{"BZW", "\uD3ED\uD48D\uC124\uACBD\uBCF4"},											// Blizzard Warning							"폭풍설경보"
			{"CAE", "\uC5B4\uB9B0\uC774 \uC720\uAD34 \uAE34\uAE09 \uC0C1\uD669"},				// Child Abduction Emergency					"어린이 유괴 긴급 상황"
			{"CDW", "\uC2DC\uBBFC \uC704\uD5D8 \uC0C1\uD669 \uACBD\uBCF4"}, 					// Civil Danger Warning						"시민 위험 상황 경보"
			{"CEM", "\uC2DC\uBBFC \uC751\uAE09 \uC0C1\uD669 \uBA54\uC2DC\uC9C0"},				// Civil Emergency Message					"시민 응급 상황 메시지"
			{"CFW", "\uD574\uC548 \uCE68\uC218 \uACBD\uBCF4"},									// Coastal Flood Warning						"해안 침수 경보"
			{"CFA", "\uD574\uC548 \uCE68\uC218 \uC8FC\uC758\uBCF4"},							// Coastal Flood Watch						"해안 침수 주의보"
			{"DSW", "\uBAA8\uB798 \uD3ED\uD48D \uACBD\uBCF4"},									// Dust Storm Warning						"모래 폭풍 경보"
			{"EQW", "\uC9C0\uC9C4 \uACBD\uBCF4"},												// Earthquake Warning						"지진 경보"
			{"EVI", "\uC989\uC2DC \uB300\uD53C"},												// Evacuation Immediate						"즉시 대피"
			{"FRW", "\uD654\uC7AC \uACBD\uBCF4"},												// Fire Warning								"화재 경보"
			{"FFW", "\uAE34\uAE09 \uD64D\uC218 \uACBD\uBCF4"},									// Flash Flood Warning						"긴급 홍수 경보"
			{"FFA", "\uAE34\uAE09 \uD64D\uC218 \uC8FC\uC758\uBCF4"},							// Flash Flood Watch							"긴급 홍수 주의보"
			{"FFS", "\uAE34\uAE09 \uD64D\uC218 \uC0C1\uD669"},									// Flash Flood Statement						"긴급 홍수 상황"
			{"FLW", "\uD64D\uC218 \uACBD\uBCF4"},												// Flood Warning							"홍수 경보"
			{"FLA", "\uD64D\uC218 \uC8FC\uC758\uBCF4"}, 										// Flood Watch								"홍수 주의보"
			{"FLS", "\uD64D\uC218 \uC0C1\uD669"},												// Flood Statement							"홍수 상황"
			{"HMW", "\uC704\uD5D9 \uBB3C\uC9C8 \uACBD\uBCF4"},									// Hazardous Materials Warning					"위험 물질 경보"
			{"HWW", "\uAC15\uD48D \uACBD\uBCF4"},												// High Wind Warning						"강풍 경보"
			{"HWA", "\uAC15\uD48D \uC8FC\uC758\uBCF4"}, 										// High Wind Watch							"강풍 주의보"
			{"HUW", "\uD0DC\uD48D \uACBD\uBCF4"},												// Hurricane Warning							"태풍 경보"
			{"HUA", "\uD0DC\uD48D \uC8FC\uC758\uBCF4"}, 										// Hurricane Watch							"태풍 주의보"
			{"HLS", "\uD0DC\uD48D\uC815\uBCF4"},												// Hurricane Statement						"태풍정보"
			{"LEW", "\uBC95\uC9D1\uD589 \uACBD\uACE0"}, 										// Law Enforcement Warning					"법집행 경고"
			{"LAE", "\uC9C0\uC5ED \uAE34\uAE09 \uC0C1\uD669"},									// Local Area Emergency						"지역 긴급 상황"
			{"NMN", "\uD1B5\uC2E0 \uBA54\uC2DC\uC9C0 \uC54C\uB9BC"},							// Network Message Notification					"통신 메지시 알림"
			{"TOE", "\u0031\u0031\u0039\uC804\uD654 \uBD88\uD1B5 \uC751\uAE09 \uC0C1\uD669"},	// 119 Telephone Outage Emergency				"119전화 불통 응급 상황"
			{"NUW", "\uD575\uBC1C\uC804\uC18C \uAD00\uB828 \uACBD\uBCF4"},						// Nuclear Power Plant Warning					"핵발전소 관련 경보"
			{"DMO", "\uC5F0\uC2B5\u002F\uC2DC\uC5F0 \uACBD\uBCF4"},								// Practice/Demo Warning						"연습/시연 경보"
			{"RHW", "\uBC29\uC0AC\uB2A5 \uC704\uD5D8 \uACBD\uBCF4"},							// Radiological Hazard Warning					"방사능 위험 경보"
			{"SVR", "\uB1CC\uC6B0 \uACBD\uBCF4"},												// Severe Thunderstorm Warning				"뇌우 경보"
			{"SVA", "\uB1CC\uC6B0 \uC8FC\uC758\uBCF4"}, 										// Severe Thunderstorm Watch					"뇌우 주의보"
			{"SVS", "\uC545\uAE30\uC0C1\uC815\uBCF4"},											// Severe Weather Statement					"악기상정보"
			{"SPW", "\uC548\uC804\uD55C \uC7A5\uC18C\uB85C \uD53C\uB09C \uACBD\uBCF4"}, 		// Shelter in Place Warning					"안전한 장소로 피난 경보"
			{"SMW", "\uD2B9\uC218 \uD574\uC591 \uACBD\uBCF4"},									// Special Marine Warning						"특수 해양 경보"
			{"SPS", "\uD2B9\uC774 \uAE30\uC0C1 C815\uBCF4"},									// Special Weather Statement					"특이 기상 정보"
			{"TOR", "\uD1A0\uB124\uC774\uB3C4 \uACBD\uBCF4"},									// Tornado Warning							"토네이도 경보"
			{"TOA", "\uD1A0\uB124\uC774\uB3C4 \uC8FC\uC758\uBCF4"}, 							// Tornado Watch							"토네이도 주의보"
			{"TRW", "\uC5F4\uB300 \uD3ED\uD48D(\uD0DC\uD48D) \uACBD\uBCF4"},					// Tropical Storm Warning						"열대 폭풍(태풍) 경보"
			{"TRA", "\uC5F4\uB300 \uD3ED\uD48D(\uD0DC\uD48D) \uC8FC\uC758\uBCF4"},				// Tropical Storm Watch						"열대 폭풍(태풍) 주의보"
			{"TSW", "\uC9C0\uC9C4\uD574\uC77C \uACBD\uBCF4"},									// Tsunami Warning							"지진해일 경보"
			{"TSA", "\uC9C0\uC9C4\uD574\uC77C \uC8FC\uC758\uBCF4"}, 							// Tsunami Watch							"지진해일 주의보"
			{"VOW", "\uD654\uC0B0 \uACBD\uBCF4"},												// Volcano Warning							"화산 경보"
			{"WSW", "\uB208\uD3ED\uD48D \uACBD\uBCF4"}, 										// Winter Storm Warning						"눈폭풍 경보"
			{"WSA", "\uB208\uD3ED\uD48D \uC8FC\uC758\uBCF4"},									// Winter Storm Watch						"눈폭풍 주의보"
		};

		ews_priority = new String[]
		{
			"Unknown",							// 
			"\uBCF4\uD1B5",						// 보통
			"\uAE34\uAE09",						// 긴급
			"\uB9E4\uC6B0 \uAE34\uAE09",		// 매우 긴급
		};

		ews_Seoul_code_index = new int[]
		{
			0x14000000,
			0x17000000,
			0x20000000,
			0x21500000,
			0x23000000,
			0x26000000,
			0x29000000,
			0x30500000,
			0x32000000,
			0x35000000,
			0x38000000,
			0x41000000,
			0x44000000,
			0x47000000,
			0x50000000,
			0x53000000,
			0x54500000,
			0x56000000,
			0x59000000,
			0x62000000,
			0x65000000,
			0x68000000,
			0x71000000,
			0x74000000,
		};
		ews_Seoul_code = new String[]
		{
			"\uC911\uAD6C",						// "중구"
			"\uC6A9\uC0B0\uAD6C",				// "용산구"
			"\uC131\uB3D9\uAD6C",				// "성동구"
			"\uAD11\uC9C4\uAD6C",				// "광진구"
			"\uB3D9\uB300\uBB38\uAD6C",			// "동대문구"
			"\uC911\uB791\uAD6C",				// "중랑구"
			"\uC131\uBD81\uAD6C",				// "성북구"
			"\uAC15\uBD81\uAD6C",				// "강북구"
			"\uB3C4\uBD09\uAD6C",				// "도봉구"
			"\uB178\uC6D0\uAD6C",				// "노원구"
			"\uC740\uD3C9\uAD6C",				// "은평구"
			"\uC11C\uB300\uBB38\uAD6C",			// "서대문구"
			"\uB9C8\uD3EC\uAD6C",				// "마포구"
			"\uC591\uCC9C\uAD6C",				// "양천구"
			"\uAC15\uC11C\uAD6C",				// "강서구"
			"\uAD6C\uB85C\uAD6C",				// "구로구"
			"\uAE08\uCC9C\uAD6C",				// "금천구"
			"\uC601\uB4F1\uD3EC\uAD6C",			// "영등포구"
			"\uB3D9\uC791\uAD6C",				// "동작구"
			"\uAD00\uC545\uAD6C",				// "관악구"
			"\uC11C\uCD08\uAD6C",				// "서초구"
			"\uAC15\uB0A8\uAD6C",				// "강남구"
			"\uC1A1\uD30C\uAD6C",				// "송파구"
			"\uAC15\uB3D9\uAD6C",				// "강동구"
		};

		ews_Busan_code_index = new int[]
		{
			0x11000000,
			0x14000000,
			0x17000000,
			0x20000000,
			0x23000000,
			0x26000000,
			0x29000000,
			0x32000000,
			0x35000000,
			0x38000000,
			0x41000000,
			0x44000000,
			0x47000000,
			0x50000000,
			0x53000000,
			0x71000000,
		};
		ews_Busan_code = new String[]
		{
			"\uC911\uAD6C",						// "중구"
			"\uC11C\uAD6C",						// "서구"
			"\uB3D9\uAD6C",						// "동구"
			"\uC601\uB3C4\uAD6C",				// "영도구"
			"\uBD80\uC0B0\uC9C4\uAD6C",			// "부산진구"
			"\uB3D9\uB798\uAD6C",				// "동래구"
			"\uB0A8\uAD6C",						// "남구"
			"\uBD81\uAD6C",						// "북구"
			"\uD574\uC6B4\uB300\uAD6C",			// "해운대구"
			"\uC0AC\uD558\uAD6C",				// "사하구"
			"\uAE08\uC815\uAD6C",				// "금정구"
			"\uAC15\uC11C\uAD6C",				// "강서구"
			"\uC5F0\uC81C\uAD6C",				// "연제구"
			"\uC218\uC601\uAD6C",				// "수영구"
			"\uC0AC\uC0C1\uAD6C",				// "사상구"
			"\uAE30\uC7A5\uAD70",				// "기장군"
		};

		ews_Daegu_code_index = new int[]
		{
			0x11000000,
			0x14000000,
			0x17000000,
			0x20000000,
			0x23000000,
			0x26000000,
			0x29000000,
			0x71000000,
		};
		ews_Daegu_code = new String[]
		{
			"\uC911\uAD6C",						// "중구"
			"\uB3D9\uAD6C",						// "동구"
			"\uC11C\uAD6C",						// "서구"
			"\uB0A8\uAD6C",						// "남구"
			"\uBD81\uAD6C",						// "북구"
			"\uC218\uC131\uAD6C",				// "수성구"
			"\uB2EC\uC11C\uAD6C",				// "달서구"
			"\uB2EC\uC131\uAD70",				// "달성군"
		};

		ews_Incheon_code_index = new int[]
		{
			0x11000000,
			0x11500000,
			0x11600000,
			0x14000000,
			0x17000000,
			0x18500000,
			0x20000000,
			0x23700000,
			0x24500000,
			0x26000000,
			0x26500000,
			0x71000000,
			0x72000000,
		};
		ews_Incheon_code = new String[]
		{
			"\uC911\uAD6C",							// "중구"
			"\uC911\uAD6C\uC601\uC885\uCD9C\uC7A5",	// "중구영종출장"
			"\uC911\uAD6C\uC6A9\uC720\uCD9C\uC7A5",	// "중구용유출장"
			"\uB3D9\uAD6C",							// "동구"
			"\uB0A8\uAD6C",							// "남구"
			"\uC5F0\uC218\uAD6C",					// "연수구"
			"\uB0A8\uB3D9\uAD6C",					// "남동구"
			"\uBD80\uD3C9\uAD6C",					// "부평구"
			"\uACC4\uC591\uAD6C",					// "계양구"
			"\uC11C\uAD6C",							// "서구"
			"\uC11C\uAD6C\uAC80\uB2E8\uCD9C\uC7A5",	// "서구검단출장"
			"\uAC15\uD654\uAD70",					// "강화군"
			"\uC639\uC9C4\uAD70",					// "옹진군"
		};

		ews_Gwangju_code_index = new int[]
		{
			0x11000000,
			0x14000000,
			0x15500000,
			0x17000000,
			0x20000000,
		};
		ews_Gwangju_code = new String[]
		{
			"\uB3D9\uAD6C",							// "동구"
			"\uC11C\uAD6C",							// "서구"
			"\uB0A8\uAD6C",							// "남구"
			"\uBD81\uAD6C",							// "북구"
			"\uAD11\uC0B0\uAD6C",					// "광산구"
		};

		ews_Daejeon_code_index = new int[]
		{
			0x11000000,
			0x14000000,
			0x17000000,
			0x20000000,
			0x23000000,
		};
		ews_Daejeon_code = new String[]
		{
			"\uB3D9\uAD6C",							// "동구"
			"\uC911\uAD6C",							// "중구"
			"\uC11C\uAD6C",							// "서구"
			"\uC720\uC131\uAD6C",					// "유성구"
			"\uB300\uB355\uAD6C",					// "대덕구"
		};

		ews_Ulsan_code_index = new int[]
		{
			0x11000000,
			0x14000000,
			0x17000000,
			0x20000000,
			0x71000000,
		};
		ews_Ulsan_code = new String[]
		{
			"\uC911\uAD6C",							// "중구"
			"\uB0A8\uAD6C",							// "남구"
			"\uB3D9\uAD6C",							// "동구"
			"\uBD81\uAD6C",							// "북구"
			"\uC6B8\uC8FC\uAD70",					// "울주군"
		};

		ews_Gyeonggi_code_index = new int[]
		{
			0x10500000,
			0x11000000,
			0x11100000,
			0x11300000,
			0x11500000,
			0x11700000,
			0x13000000,
			0x13100000,
			0x13300000,
			0x13500000,
			0x15000000,
			0x17000000,
			0x17100000,
			0x17300000,
			0x19000000, 
			0x19500000, 
			0x19700000, 
			0x19900000, 
			0x21000000, 
			0x22000000, 
			0x25000000, 
			0x27000000, 
			0x27100000, 
			0x27300000, 
			0x28000000, 
			0x28100000, 
			0x28500000, 
			0x28700000, 
			0x29000000, 
			0x31000000, 
			0x36000000, 
			0x37000000, 
			0x39000000, 
			0x41000000, 
			0x43000000, 
			0x45000000, 
			0x46000000, 
			0x46100000, 
			0x46300000, 
			0x46500000, 
			0x48000000, 
			0x50000000, 
			0x55000000, 
			0x57000000, 
			0x59000000, 
			0x61000000, 
			0x63000000, 
			0x65000000, 
			0x73000000, 
			0x80000000, 
			0x82000000, 
			0x83000000, 
		};
		ews_Gyeonggi_code = new String[]
		{
 			 "\uBD81\uBD80\uCD9C\uC7A5\uC18C",			// "북부출장소"
			 "\uC218\uC6D0\uC2DC",						// "수원시"
			 "\uC218\uC6D0\uC2DC\uC7A5\uC548\uAD6C",	// "수원시장안구"
			 "\uC218\uC6D0\uC2DC\uAD8C\uC120\uAD6C",	// "수원시권선구"
			 "\uC218\uC6D0\uC2DC\uD314\uB2EC\uAD6C",	// "수원시팔달구"
			 "\uC218\uC6D0\uC2DC\uC601\uD1B5\uAD6C",	// "수원시영통구"
			 "\uC131\uB0A8\uC2DC",						// "성남시"
			 "\uC131\uB0A8\uC2DC\uC218\uC815\uAD6C",	// "성남시수정구"
			 "\uC131\uB0A8\uC2DC\uC911\uC6D0\uAD6C",	// "성남시중원구"
			 "\uC131\uB0A8\uC2DC\uBD84\uB2F9\uAD6C",	// "성남시분당구"
			 "\uC758\uC815\uBD80\uC2DC",				// "의정부시"
			 "\uC548\uC591\uC2DC",						// "안양시"
			 "\uC548\uC591\uC2DC\uB9CC\uC548\uAD6C",	// "안양시만안구"
			 "\uC548\uC591\uC2DC\uB3D9\uC548\uAD6C",	// "안양시동안구"
			 "\uBD80\uCC9C\uC2DC",						// "부천시"
			 "\uBD80\uCC9C\uC2DC\uC6D0\uBBF8\uAD6C", 	// "부천시원미구"
			 "\uBD80\uCC9C\uC2DC\uC18C\uC0AC\uAD6C",	// "부천시소사구"
			 "\uBD80\uCC9C\uC2DC\uC624\uC815\uAD6C",	// "부천시오정구"
			 "\uAD11\uBA85\uC2DC",						// "광명시"
			 "\uD3C9\uD0DD\uC2DC",						// "평택시"
			 "\uB3D9\uB450\uCC9C\uC2DC",				// "동두천시"
			 "\uC548\uC0B0\uC2DC",						// "안산시"
			 "\uC548\uC0B0\uC2DC\uC0C1\uB85D\uAD6C",	// "안산시상록구"
			 "\uC548\uC0B0\uC2DC\uB2E8\uC6D0\uAD6C",	// "안산시단원구"
			 "\uACE0\uC591\uC2DC", 						//  "고양시"
			 "\uACE0\uC591\uC2DC\uB355\uC591\uAD6C",	// "고양시덕양구"
			 "\uACE0\uC591\uC2DC\uC77C\uC0B0\uB3D9",	// "고양시일산동"
			 "\uACE0\uC591\uC2DC\uC77C\uC0B0\uC11C",	// "고양시일산서"
			 "\uACFC\uCC9C\uC2DC", 						//  "과천시"
			 "\uAD6C\uB9AC\uC2DC",						// "구리시"
			 "\uB0A8\uC591\uC8FC\uC2DC",				// "남양주시"
			 "\uC624\uC0B0\uC2DC",						// "오산시"
			 "\uC2DC\uD765\uC2DC",						// "시흥시"
			 "\uAD70\uD3EC\uC2DC",						// "군포시"
			 "\uC758\uC655\uC2DC",						// "의왕시"
			 "\uD558\uB0A8\uC2DC",						// "하남시"
			 "\uC6A9\uC778\uC2DC",						// "용인시"
			 "\uC6A9\uC778\uC2DC\uCC98\uC778\uAD6C",	// "용인시처인구"
			 "\uC6A9\uC778\uC2DC\uAE30\uD765\uAD6C",	// "용인시기흥구"
			 "\uC6A9\uC778\uC2DC\uC218\uC9C0\uAD6C",	// "용인시수지구"
			 "\uD30C\uC8FC\uC2DC",						// "파주시"
			 "\uC774\uCC9C\uC2DC",						// "이천시"
			 "\uC548\uC131\uC2DC",						// "안성시"
			 "\uAE40\uD3EC\uC2DC",						// "김포시"
			 "\uD654\uC131\uC2DC",						// "화성시"
			 "\uAD11\uC8FC\uC2DC",						// "광주시"
			 "\uC591\uC8FC\uC2DC",						// "양주시"
			 "\uD3EC\uCC9C\uC2DC",						// "포천시"
			 "\uC5EC\uC8FC\uAD70",						// "여주군"
			 "\uC5F0\uCC9C\uAD70",						// "연천군"
			 "\uAC00\uD3C9\uAD70",						// "가평군"
			 "\uC591\uD3C9\uAD70",						// "양평군"
		};

		ews_Gangwon_code_index = new int[]
		{
 			0x10500000, 
			0x11000000, 
			0x13000000, 
			0x15000000, 
			0x17000000, 
			0x19000000, 
			0x21000000, 
			0x23000000, 
			0x72000000, 
			0x73000000, 
			0x75000000, 
			0x76000000, 
			0x77000000, 
			0x78000000, 
			0x79000000, 
			0x80000000, 
			0x81000000, 
			0x82000000, 
			0x83000000, 
		};
		 ews_Gangwon_code = new String[]
		{
 			 "\uB3D9\uD574\uCD9C\uC7A5\uC18C",			// "동해출장소"
			 "\uCD98\uCC9C\uC2DC",						// "춘천시"
			 "\uC6D0\uC8FC\uC2DC",						// "원주시"
			 "\uAC15\uB989\uC2DC",						// "강릉시"
			 "\uB3D9\uD574\uC2DC",						// "동해시"
			 "\uD0DC\uBC31\uC2DC",						// "태백시"
			 "\uC18D\uCD08\uC2DC",						// "속초시"
			 "\uC0BC\uCC99\uC2DC",						// "삼척시"
			 "\uD64D\uCC9C\uAD70",						// "홍천군"
			 "\uD6A1\uC131\uAD70",						// "횡성군"
			 "\uB3D9\uC6D4\uAD70",						// "영월군"
			 "\uD3C9\uCC3D\uAD70",						// "평창군"
			 "\uC815\uC120\uAD70",						// "정선군"
			 "\uCCA0\uC6D0\uAD70",						// "철원군"
			 "\uD654\uCC9C\uAD70",						// "화천군"
			 "\uC591\uAD6C\uAD70",						// "양구군"
			 "\uC778\uC81C\uAD70",						// "인제군"
			 "\uACE0\uC131\uAD70",						// "고성군"
			 "\uC591\uC591\uAD70",						// "양양군"
		};

		ews_Chungbuk_code_index = new int[]
		{
			0x11000000,
			0x11100000,
			0x11300000,
			0x13000000,
			0x15000000,
			0x71000000,
			0x72000000,
			0x73000000,
			0x74000000,
			0x74500000,
			0x75000000,
			0x76000000,
			0x77000000,
			0x80000000,
		};
		ews_Chungbuk_code = new String[]
		{
			"\uCCAD\uC8FC\uC2DC",						// "청주시"
			"\uCCAD\uC8FC\uC2DC\uC0C1\uB2F9\uAD6C",		// "청주시상당구"
			"\uCCAD\uC8FC\uC2DC\uD765\uB355\uAD6C",		// "청주시흥덕구"
			"\uCDA9\uC8FC\uC2DC",						// "충주시"
			"\uC81C\uCC9C\uC2DC",						// "제천시"
			"\uCCAD\uC6D0\uAD70",						// "청원군"
			"\uBCF4\uC740\uAD70",						// "보은군"
			"\uC625\uCC9C\uAD70",						// "옥천군"
			"\uC601\uB3D9\uAD70",						// "영동군"
			"\uC99D\uD3C9\uAD70",						// "증평군"
			"\uC9C4\uCC9C\uAD70",						// "진천군"
			"\uAD34\uC0B0\uAD70",						// "괴산군"
			"\uC74C\uC131\uAD70",						// "음성군"
			"\uB2E8\uC591\uAD70",						// "단양군"
		};

		ews_Chungnam_code_index = new int[]
		{
			0x13000000,
			0x13100000,
			0x13300000,
			0x15000000,
			0x18000000,
			0x20000000,
			0x21000000,
			0x23000000,
			0x25000000,
			0x71000000,
			0x73000000,
			0x76000000,
			0x77000000,
			0x79000000,
			0x80000000,
			0x81000000,
			0x82500000,
			0x83000000,
		};
		ews_Chungnam_code = new String[]
		{
			"\uCC9C\uC548\uC2DC",						// "천안시"
			"\uCC9C\uC548\uC2DC\uB3D9\uB0A8\uAD6C",		// "천안시동남구"
			"\uCC9C\uC548\uC2DC\uC11C\uBD81\uAD6C",		// "천안시서북구"
			"\uACF5\uC8FC\uC2DC",						// "공주시"
			"\uBCF4\uB839\uC2DC",						// "보령시"
			"\uC544\uC0B0\uC2DC",						// "아산시"
			"\uC11C\uC0B0\uC2DC",						// "서산시"
			"\uB17C\uC0B0\uC2DC",						// "논산시"
			"\uACC4\uB8E1\uC2DC",						// "계룡시"
			"\uAE08\uC0B0\uAD70",						// "금산군"
			"\uC5F0\uAE30\uAD70",						// "연기군"
			"\uBD80\uC5EC\uAD70",						// "부여군"
			"\uC11C\uCC9C\uAD70",						// "서천군"
			"\uCCAD\uC591\uAD70",						// "청양군"
			"\uD64D\uC131\uAD70",						// "홍성군"
			"\uC608\uC0B0\uAD70",						// "예산군"
			"\uD0DC\uC548\uAD70",						// "태안군"
			"\uB2F9\uC9C4\uAD70",						// "당진군"
		};

		ews_Jeonbuk_code_index = new int[]
		{
			0x11000000,
			0x11100000,
			0x11300000,
			0x11800000,
			0x13000000,
			0x14000000,
			0x14500000,
			0x18000000,
			0x19000000,
			0x21000000,
			0x71000000,
			0x72000000,
			0x73000000,
			0x74000000,
			0x75000000,
			0x77000000,
			0x79000000,
			0x80000000, 
		};
		 ews_Jeonbuk_code = new String[]
		{
 			 "\uC804\uC8FC\uC2DC",						// "전주시"
			 "\uC804\uC8FC\uC2DC\uC644\uC0B0\uAD6C",	// "전주시완산구"
			 "\uC804\uC8FC\uC2DC\uB355\uC9C4\uAD6C",	// "전주시덕진구"
			 "\uC804\uC8FC\uC2DC\uD6A8\uC790\uCD9C",	// "전주시효자출"
			 "\uAD70\uC0B0\uC2DC",						// "군산시"
			 "\uC775\uC0B0\uC2DC",						// "익산시"
			 "\uC775\uC0B0\uC2DC\uD568\uC5F4\uCD9C",	// "익산시함열출"
			 "\uC815\uC74D\uC2DC",						// "정읍시"
			 "\uB0A8\uC6D0\uC2DC",						// "남원시"
			 "\uAE40\uC81C\uC2DC",						// "김제시"
			 "\uC644\uC8FC\uAD70",						// "완주군"
			 "\uC9C4\uC548\uAD70",						// "진안군"
			 "\uBB34\uC8FC\uAD70",						// "무주군"
			 "\uC7A5\uC218\uAD70",						// "장수군"
			 "\uC784\uC2E4\uAD70",						// "임실군"
			 "\uC21C\uCC3D\uAD70",						// "순창군"
			 "\uACE0\uCC3D\uAD70",						// "고창군"
			 "\uBD80\uC548\uAD70",						// "부안군"
		};

		ews_Jeonnam_code_index = new int[]
		{
			0x11000000,
			0x13000000,
			0x15000000,
			0x17000000,
			0x23000000,
			0x71000000,
			0x72000000,
			0x73000000,
			0x77000000,
			0x78000000,
			0x79000000,
			0x80000000,
			0x81000000,
			0x82000000,
			0x83000000,
			0x84000000,
			0x86000000,
			0x87000000,
			0x88000000,
			0x89000000,
			0x90000000,
			0x91000000,
		};
		ews_Jeonnam_code = new String[]
		{
			"\uBAA9\uD3EC\uC2DC",						// "목포시"
			"\uC5EC\uC218\uC2DC",						// "여수시"
			"\uC21C\uCC9C\uC2DC",						// "순천시"
			"\uB098\uC8FC\uC2DC",						// "나주시"
			"\uAD11\uC591\uC2DC",						// "광양시"
			"\uB2F4\uC591\uAD70",						// "담양군"
			"\uACE1\uC131\uAD70",						// "곡성군"
			"\uAD6C\uB840\uAD70",						// "구례군"
			"\uACE0\uD765\uAD70",						// "고흥군"
			"\uBCF4\uC131\uAD70",						// "보성군"
			"\uD654\uC21C\uAD70",						// "화순군"
			"\uC7A5\uD765\uAD70",						// "장흥군"
			"\uAC15\uC9C4\uAD70",						// "강진군"
			"\uD574\uB0A8\uAD70",						// "해남군"
			"\uC601\uC554\uAD70",						// "영암군"
			"\uBB34\uC548\uAD70",						// "무안군"
			"\uD568\uD3C9\uAD70",						// "함평군"
			"\uC601\uAD11\uAD70",						// "영광군"
			"\uC7A5\uC131\uAD70",						// "장성군"
			"\uC644\uB3C4\uAD70",						// "완도군"
			"\uC9C4\uB3C4\uAD70",						// "진도군"
			"\uC2E0\uC548\uAD70",						// "신안군"
		};

		ews_Gyeongbuk_code_index = new int[]
		{
			0x11000000,
			0x11100000,
			0x11300000,
			0x13000000,
			0x15000000,
			0x17000000,
			0x19000000,
			0x21000000,
			0x23000000,
			0x25000000,
			0x28000000,
			0x29000000,
			0x72000000,
			0x73000000,
			0x75000000,
			0x76000000,
			0x77000000,
			0x82000000,
			0x83000000,
			0x84000000,
			0x85000000,
			0x90000000,
			0x92000000,
			0x93000000,
			0x94000000,
		};
		ews_Gyeongbuk_code = new String[]
		{
			"\uD3EC\uD56D\uC2DC",						// "포항시"
			"\uD3EC\uD56D\uC2DC\uB0A8\uAD6C",			// "포항시남구"
			"\uD3EC\uD56D\uC2DC\uBD81\uAD6C",			// "포항시북구"
			"\uACBD\uC8FC\uC2DC",						// "경주시"
			"\uAE40\uCC9C\uC2DC",						// "김천시"
			"\uC548\uB3D9\uC2DC",						// "안동시"
			"\uAD6C\uBBF8\uC2DC",						// "구미시"
			"\uC601\uC8FC\uC2DC",						// "영주시"
			"\uC601\uCC9C\uC2DC",						// "영천시"
			"\uC0C1\uC8FC\uC2DC",						// "상주시"
			"\uBB38\uACBD\uC2DC",						// "문경시"
			"\uACBD\uC0B0\uC2DC",						// "경산시"
			"\uAD70\uC704\uAD70",						// "군위군"
			"\uC758\uC131\uAD70",						// "의성군"
			"\uCCAD\uC1A1\uAD70",						// "청송군"
			"\uC601\uC591\uAD70",						// "영양군"
			"\uC601\uB355\uAD70",						// "영덕군"
			"\uCCAD\uB3C4\uAD70",						// "청도군"
			"\uACE0\uB839\uAD70",						// "고령군"
			"\uC131\uC8FC\uAD70",						// "성주군"
			"\uCE60\uACE1\uAD70",						// "칠곡군"
			"\uC608\uCC9C\uAD70",						// "예천군"
			"\uBD09\uD654\uAD70",						// "봉화군"
			"\uC6B8\uC9C4\uAD70",						// "울진군"
			"\uC6B8\uB989\uAD70",						// "울릉군"
		};

		ews_Gyeongnam_code_index = new int[]
		{
			0x11000000,
			0x16000000,
			0x17000000,
			0x19000000,
			0x22000000,
			0x24000000,
			0x24500000,
			0x25000000,
			0x27000000,
			0x31000000,
			0x33000000,
			0x72000000,
			0x73000000,
			0x74000000,
			0x82000000,
			0x84000000,
			0x85000000,
			0x86000000,
			0x87000000,
			0x88000000,
			0x89000000,
		};
		ews_Gyeongnam_code = new String[]
		{
			"\uCC3D\uC6D0\uC2DC",						// "창원시"
			"\uB9C8\uC0B0\uC2DC",						// "마산시"
			"\uC9C4\uC8FC\uC2DC",						// "진주시"
			"\uC9C4\uD574\uC2DC",						// "진해시"
			"\uD1B5\uC601\uC2DC",						// "통영시"
			"\uC0AC\uCC9C\uC2DC",						// "사천시"
			"\uC0AC\uCC9C\uB0A8\uC591\uCD9C\uC7A5",		// "사천남양출장"
			"\uAE40\uD574\uC2DC",						// "김해시"
			"\uBC00\uC591\uC2DC",						// "밀양시"
			"\uAC70\uC81C\uC2DC",						// "거제시"
			"\uC591\uC0B0\uC2DC",						// "양산시"
			"\uC758\uB839\uAD70",						// "의령군"
			"\uD568\uC548\uAD70",						// "함안군"
			"\uCC3D\uB155\uAD70",						// "창녕군"
			"\uACE0\uC131\uAD70",						// "고성군"
			"\uB0A8\uD574\uAD70",						// "남해군"
			"\uD558\uB3D9\uAD70",						// "하동군"
			"\uC0B0\uCCAD\uAD70",						// "산청군"
			"\uD568\uC591\uAD70",						// "함양군"
			"\uAC70\uCC3D\uAD70",						// "거창군"
			"\uD569\uCC9C\uAD70",						// "합천군"
		};

		ews_Jeju_code_index = new int[]
		{
			0x11000000,
			0x13000000,
		};
		ews_Jeju_code = new String[]
		{
			"\uC81C\uC8FC\uC2DC",						// "제주시"
			"\uC11C\uADC0\uD3EC\uC2DC",					// "서귀포시"
		};

		ews_Local_code_index = new int[]
		{
			0x00,
			0x11,
			0x26,
			0x27,
			0x28,
			0x29,
			0x30,
			0x31,
			0x41,
			0x42,
			0x43,
			0x44,
			0x45,
			0x46,
			0x47,
			0x48,
			0x50,
		};
		ews_Local_code = new String[]
		{
			"\uC804\uAD6D",									// "전국"
			"\uC11C\uC6B8\uD2B9\uBCC4\uC2DC",				// "서울특별시"
			"\uBD80\uC0B0\uAD11\uC5ED\uC2DC",				// "부산광역시"
			"\uB300\uAD6C\uAD11\uC5ED\uC2DC",				// "대구광역시"
			"\uC778\uCC9C\uAD11\uC5ED\uC2DC",				// "인천광역시"
			"\uAD11\uC8FC\uAD11\uC5ED\uC2DC",				// "광주광역시"
			"\uB300\uC804\uAD11\uC5ED\uC2DC",				// "대전광역시"
			"\uC6B8\uC0B0\uAD11\uC5ED\uC2DC",				// "울산광역시"
			"\uACBD\uAE30\uB3C4",							// "경기도"
			"\uAC15\uC6D0\uB3C4",							// "강원도"
			"\uCDA9\uCCAD\uBD81\uB3C4",						// "충청북도"
			"\uCDA9\uCCAD\uB0A8\uB3C4",						// "충청남도"
			"\uC804\uB77C\uBD81\uB3C4",						// "전라북도"
			"\uC804\uB77C\uB0A8\uB3C4",						// "전라남도"
			"\uACBD\uC0C1\uBD81\uB3C4",						// "경상북도"
			"\uACBD\uC0C1\uB0A8\uB3C4",						// "경상남도"
			"\uC81C\uC8FC\uD2B9\uBCC4\uC790\uCE58\uB3C4",	//"제주특별자치도"
		};

		type = new byte[3];
		local_code_main = new int[16];
		local_code_sub = new int[16];
    }

    public String[][] ews_type;
    public String[] ews_priority;

	public int[] ews_Seoul_code_index;
	public String[] ews_Seoul_code;

	public int[] ews_Busan_code_index;
	public String[] ews_Busan_code;

	public int[] ews_Daegu_code_index;
	public String[] ews_Daegu_code;

	public int[] ews_Incheon_code_index;
	public String[] ews_Incheon_code;

	public int[] ews_Gwangju_code_index;
	public String[] ews_Gwangju_code;

	public int[] ews_Daejeon_code_index;
	public String[] ews_Daejeon_code;

	public int[] ews_Ulsan_code_index;
	public String[] ews_Ulsan_code;

	public int[] ews_Gyeonggi_code_index;
	public String[] ews_Gyeonggi_code;

	public int[] ews_Gangwon_code_index;
	public String[] ews_Gangwon_code;

	public int[] ews_Chungbuk_code_index;
	public String[] ews_Chungbuk_code;

	public int[] ews_Chungnam_code_index;
	public String[] ews_Chungnam_code;

	public int[] ews_Jeonbuk_code_index;
	public String[] ews_Jeonbuk_code;

	public int[] ews_Jeonnam_code_index;
	public String[] ews_Jeonnam_code;

	public int[] ews_Gyeongbuk_code_index;
	public String[] ews_Gyeongbuk_code;

	public int[] ews_Gyeongnam_code_index;
	public String[] ews_Gyeongnam_code;

	public int[] ews_Jeju_code_index;
	public String[] ews_Jeju_code;

	public int[] ews_Local_code_index;
	public String[] ews_Local_code;

	public int[] ews_local_code_sub_index;

    public int id;
    public byte[] type;                 // 재난 종류
    public char priority;               // 경보 우선 순위
    public int time;                    // 재난 발령 시간
    public char local_type;             // 재난 지역 형식
    public char local_count;            // 재난 지역 수
    public int[] local_code_main;       // 재난 지역(도,광역 단위)
    public int[] local_code_sub;        // 재난 지역(시,군 단위)
	public byte[] message;              // 단문

	public String GetLocalSubCode(int index, int subCode)
	{
		int i;
		switch(index)
		{
			case 0:
				break;

			case 1:
				for(i=0; i<ews_Seoul_code.length; i++)
				{
					if(subCode == ews_Seoul_code_index[i])
					{
						return ews_Seoul_code[i];
					}
				}
				break;

			case 2:
				for(i=0; i<ews_Busan_code.length; i++)
				{
					if(subCode == ews_Busan_code_index[i])
					{
						return ews_Busan_code[i];
					}
				}
				break;

			case 3:
				for(i=0; i<ews_Daegu_code.length; i++)
				{
					if(subCode == ews_Daegu_code_index[i])
					{
						return ews_Daegu_code[i];
					}
				}
				break;

			case 4:
				for(i=0; i<ews_Incheon_code.length; i++)
				{
					if(subCode == ews_Incheon_code_index[i])
					{
						return ews_Incheon_code[i];
					}
				}
				break;

			case 5:
				for(i=0; i<ews_Gwangju_code.length; i++)
				{
					if(subCode == ews_Gwangju_code_index[i])
					{
						return ews_Gwangju_code[i];
					}
				}
				break;

			case 6:
				for(i=0; i<ews_Daejeon_code.length; i++)
				{
					if(subCode == ews_Daejeon_code_index[i])
					{
						return ews_Daejeon_code[i];
					}
				}
				break;

			case 7:
				for(i=0; i<ews_Ulsan_code.length; i++)
				{
					if(subCode == ews_Ulsan_code_index[i])
					{
						return ews_Ulsan_code[i];
					}
				}
				break;
				
			case 8:
				for(i=0; i<ews_Gyeonggi_code.length; i++)
				{
					if(subCode == ews_Gyeonggi_code_index[i])
					{
						return ews_Gyeonggi_code[i];
					}
				}
				break;

			case 9:
				for(i=0; i<ews_Gangwon_code.length; i++)
				{
					if(subCode == ews_Gangwon_code_index[i])
					{
						return ews_Gangwon_code[i];
					}
				}
				break;

			case 10:
				for(i=0; i<ews_Chungbuk_code.length; i++)
				{
					if(subCode == ews_Chungbuk_code_index[i])
					{
						return ews_Chungbuk_code[i];
					}
				}
				break;

			case 11:
				for(i=0; i<ews_Chungnam_code.length; i++)
				{
					if(subCode == ews_Chungnam_code_index[i])
					{
						return ews_Chungnam_code[i];
					}
				}
				break;

			case 12:
				for(i=0; i<ews_Jeonbuk_code.length; i++)
				{
					if(subCode == ews_Jeonbuk_code_index[i])
					{
						return ews_Jeonbuk_code[i];
					}
				}
				break;

			case 13:
				for(i=0; i<ews_Jeonnam_code.length; i++)
				{
					if(subCode == ews_Jeonnam_code_index[i])
					{
						return ews_Jeonnam_code[i];
					}
				}
				break;

			case 14:
				for(i=0; i<ews_Gyeongbuk_code.length; i++)
				{
					if(subCode == ews_Gyeongbuk_code_index[i])
					{
						return ews_Gyeongbuk_code[i];
					}
				}
				break;

			case 15:
				for(i=0; i<ews_Gyeongnam_code.length; i++)
				{
					if(subCode == ews_Gyeongnam_code_index[i])
					{
						return ews_Gyeongnam_code[i];
					}
				}
				break;

			case 16:
				for(i=0; i<ews_Jeju_code.length; i++)
				{
					if(subCode == ews_Jeju_code_index[i])
					{
						return ews_Jeju_code[i];
					}
				}
				break;
		}
		return null;
	}

	public int GetLocalSubCodeCount(int index)
	{
		int ret = 0;
		switch(index)
		{
			case 0:
				break;

			case 1:
				ret = ews_Seoul_code.length;
				break;

			case 2:
				ret = ews_Busan_code.length;
				break;

			case 3:
				ret = ews_Daegu_code.length;
				break;

			case 4:
				ret = ews_Incheon_code.length;
				break;

			case 5:
				ret = ews_Gwangju_code.length;
				break;

			case 6:
				ret = ews_Daejeon_code.length;
				break;

			case 7:
				ret = ews_Ulsan_code.length;
				break;
				
			case 8:
				ret = ews_Gyeonggi_code.length;
				break;

			case 9:
				ret = ews_Gangwon_code.length;
				break;

			case 10:
				ret = ews_Chungbuk_code.length;
				break;

			case 11:
				ret = ews_Chungnam_code.length;
				break;

			case 12:
				ret = ews_Jeonbuk_code.length;
				break;

			case 13:
				ret = ews_Jeonnam_code.length;
				break;

			case 14:
				ret = ews_Gyeongbuk_code.length;
				break;

			case 15:
				ret = ews_Gyeongnam_code.length;
				break;

			case 16:
				ret = ews_Jeju_code.length;
				break;
		}
		return ret;
	}
}

