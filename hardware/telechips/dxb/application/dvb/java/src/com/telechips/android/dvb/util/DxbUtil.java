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

package com.telechips.android.dvb.util;

import java.util.Formatter;
import java.util.HashMap;
import java.util.Locale;

public class DxbUtil {

	private static HashMap<String, String> sLanguageCodeMap = new HashMap<String, String>();
	static {
		sLanguageCodeMap.put("abk", "Abkhazian");
		sLanguageCodeMap.put("ace", "Achinese");
		sLanguageCodeMap.put("ach", "Acoli");
		sLanguageCodeMap.put("ada", "Adangme");
		sLanguageCodeMap.put("aar", "Afar");
		sLanguageCodeMap.put("afh", "Afrihili");
		sLanguageCodeMap.put("afr", "Afrikaans");
		sLanguageCodeMap.put("afa", "Afro-Asiatic (Other)");
		sLanguageCodeMap.put("aka", "Akan");
		sLanguageCodeMap.put("akk", "Akkadian");
		sLanguageCodeMap.put("alb", "Albanian");
		sLanguageCodeMap.put("sqi", "Albanian");
		sLanguageCodeMap.put("ale", "Aleut");
		sLanguageCodeMap.put("alg", "Algonquian languages");
		sLanguageCodeMap.put("tut", "Altaic (Other)");
		sLanguageCodeMap.put("amh", "Amharic");
		sLanguageCodeMap.put("apa", "Apache languages");
		sLanguageCodeMap.put("ara", "Arabic");
		sLanguageCodeMap.put("arc", "Aramaic");
		sLanguageCodeMap.put("arp", "Arapaho");
		sLanguageCodeMap.put("arn", "Araucanian");
		sLanguageCodeMap.put("arw", "Arawak");
		sLanguageCodeMap.put("arm", "Armenian");
		sLanguageCodeMap.put("hye", "Armenian");
		sLanguageCodeMap.put("art", "Artificial (Other)");
		sLanguageCodeMap.put("asm", "Assamese");
		sLanguageCodeMap.put("ath", "Athapascan languages");
		sLanguageCodeMap.put("map", "Austronesian (Other)");
		sLanguageCodeMap.put("ava", "Avaric");
		sLanguageCodeMap.put("ave", "Avestan");
		sLanguageCodeMap.put("awa", "Awadhi");
		sLanguageCodeMap.put("aym", "Aymara");
		sLanguageCodeMap.put("aze", "Azerbaijani");
		sLanguageCodeMap.put("nah", "Aztec");
		sLanguageCodeMap.put("ban", "Balinese");
		sLanguageCodeMap.put("bat", "Baltic (Other)");
		sLanguageCodeMap.put("bal", "Baluchi");
		sLanguageCodeMap.put("bam", "Bambara");
		sLanguageCodeMap.put("bai", "Bamileke languages");
		sLanguageCodeMap.put("bad", "Banda");
		sLanguageCodeMap.put("bnt", "Bantu (Other)");
		sLanguageCodeMap.put("bas", "Basa");
		sLanguageCodeMap.put("bak", "Bashkir");
		sLanguageCodeMap.put("baq", "Basque");
		sLanguageCodeMap.put("eus", "Basque");
		sLanguageCodeMap.put("bej", "Beja");
		sLanguageCodeMap.put("bem", "Bemba");
		sLanguageCodeMap.put("ben", "Bengali");
		sLanguageCodeMap.put("ber", "Berber (Other)");
		sLanguageCodeMap.put("bho", "Bhojpuri");
		sLanguageCodeMap.put("bih", "Bihari");
		sLanguageCodeMap.put("bik", "Bikol");
		sLanguageCodeMap.put("bin", "Bini");
		sLanguageCodeMap.put("bis", "Bislama");
		sLanguageCodeMap.put("bra", "Braj");
		sLanguageCodeMap.put("bre", "Breton");
		sLanguageCodeMap.put("bug", "Buginese");
		sLanguageCodeMap.put("bul", "Bulgarian");
		sLanguageCodeMap.put("bua", "Buriat");
		sLanguageCodeMap.put("bur", "Burmese");
		sLanguageCodeMap.put("mya", "Burmese");
		sLanguageCodeMap.put("bel", "Byelorussian");
		sLanguageCodeMap.put("cad", "Caddo");
		sLanguageCodeMap.put("car", "Carib");
		sLanguageCodeMap.put("cat", "Catalan");
		sLanguageCodeMap.put("cau", "Caucasian (Other)");
		sLanguageCodeMap.put("ceb", "Cebuano");
		sLanguageCodeMap.put("cel", "Celtic (Other)");
		sLanguageCodeMap.put("cai", "Central American Indian (Other)");
		sLanguageCodeMap.put("chg", "Chagatai");
		sLanguageCodeMap.put("cha", "Chamorro");
		sLanguageCodeMap.put("che", "Chechen");
		sLanguageCodeMap.put("chr", "Cherokee");
		sLanguageCodeMap.put("chy", "Cheyenne");
		sLanguageCodeMap.put("chb", "Chibcha");
		sLanguageCodeMap.put("chi", "Chinese");
		sLanguageCodeMap.put("zho", "Chinese");
		sLanguageCodeMap.put("chn", "Chinook jargon");
		sLanguageCodeMap.put("cho", "Choctaw");
		sLanguageCodeMap.put("chu", "Church Slavic");
		sLanguageCodeMap.put("chv", "Chuvash");
		sLanguageCodeMap.put("cop", "Coptic");
		sLanguageCodeMap.put("cor", "Cornish");
		sLanguageCodeMap.put("cos", "Corsican");
		sLanguageCodeMap.put("cre", "Cree");
		sLanguageCodeMap.put("mus", "Creek");
		sLanguageCodeMap.put("crp", "Creoles and Pidgins (Other)");
		sLanguageCodeMap.put("cpe", "Creoles and Pidgins, English-based (Other)");
		sLanguageCodeMap.put("cpf", "Creoles and Pidgins, French-based (Other)");
		sLanguageCodeMap.put("cpp", "Creoles and Pidgins, Portuguese-based (Other)");
		sLanguageCodeMap.put("cus", "Cushitic (Other)");
		sLanguageCodeMap.put("ces", "Czech");
		sLanguageCodeMap.put("cze", "Czech");
		sLanguageCodeMap.put("dak", "Dakota");
		sLanguageCodeMap.put("dan", "Danish");
		sLanguageCodeMap.put("del", "Delaware");
		sLanguageCodeMap.put("din", "Dinka");
		sLanguageCodeMap.put("div", "Divehi");
		sLanguageCodeMap.put("doi", "Dogri");
		sLanguageCodeMap.put("dra", "Dravidian (Other)");
		sLanguageCodeMap.put("dua", "Duala");
		sLanguageCodeMap.put("dut", "Dutch");
		sLanguageCodeMap.put("nla", "Dutch");
		sLanguageCodeMap.put("dum", "Dutch, Middle (ca. 1050-1350)");
		sLanguageCodeMap.put("dyu", "Dyula");
		sLanguageCodeMap.put("dzo", "Dzongkha");
		sLanguageCodeMap.put("efi", "Efik");
		sLanguageCodeMap.put("egy", "Egyptian (Ancient)");
		sLanguageCodeMap.put("eka", "Ekajuk");
		sLanguageCodeMap.put("elx", "Elamite");
		sLanguageCodeMap.put("eng", "English");
		sLanguageCodeMap.put("enm", "English, Middle (ca. 1100-1500)");
		sLanguageCodeMap.put("ang", "English, Old (ca. 450-1100)");
		sLanguageCodeMap.put("esk", "Eskimo (Other)");
		sLanguageCodeMap.put("epo", "Esperanto");
		sLanguageCodeMap.put("est", "Estonian");
		sLanguageCodeMap.put("ewe", "Ewe");
		sLanguageCodeMap.put("ewo", "Ewondo");
		sLanguageCodeMap.put("fan", "Fang");
		sLanguageCodeMap.put("fat", "Fanti");
		sLanguageCodeMap.put("fao", "Faroese");
		sLanguageCodeMap.put("fij", "Fijian");
		sLanguageCodeMap.put("fin", "Finnish");
		sLanguageCodeMap.put("fiu", "Finno-Ugrian (Other)");
		sLanguageCodeMap.put("fon", "Fon");
		sLanguageCodeMap.put("fra", "French");
		sLanguageCodeMap.put("fre", "French");
		sLanguageCodeMap.put("frm", "French, Middle (ca. 1400-1600)");
		sLanguageCodeMap.put("fro", "French, Old (842- ca. 1400)");
		sLanguageCodeMap.put("fry", "Frisian");
		sLanguageCodeMap.put("ful", "Fulah");
		sLanguageCodeMap.put("gaa", "Ga");
		sLanguageCodeMap.put("gae", "Gaelic (Scots)");
		sLanguageCodeMap.put("gdh", "Gaelic (Scots)");
		sLanguageCodeMap.put("glg", "Gallegan");
		sLanguageCodeMap.put("lug", "Ganda");
		sLanguageCodeMap.put("gay", "Gayo");
		sLanguageCodeMap.put("gez", "Geez");
		sLanguageCodeMap.put("geo", "Georgian");
		sLanguageCodeMap.put("kat", "Georgian");
		sLanguageCodeMap.put("deu", "German");
		sLanguageCodeMap.put("ger", "German");
		sLanguageCodeMap.put("gmh", "German, Middle High (ca. 1050-1500)");
		sLanguageCodeMap.put("goh", "German, Old High (ca. 750-1050)");
		sLanguageCodeMap.put("gem", "Germanic (Other)");
		sLanguageCodeMap.put("gil", "Gilbertese");
		sLanguageCodeMap.put("gon", "Gondi");
		sLanguageCodeMap.put("got", "Gothic");
		sLanguageCodeMap.put("grb", "Grebo");
		sLanguageCodeMap.put("grc", "Greek, Ancient (to 1453)");
		sLanguageCodeMap.put("ell", "Greek, Modern (1453-)");
		sLanguageCodeMap.put("gre", "Greek, Modern (1453-)");
		sLanguageCodeMap.put("kal", "Greenlandic");
		sLanguageCodeMap.put("grn", "Guarani");
		sLanguageCodeMap.put("guj", "Gujarati");
		sLanguageCodeMap.put("hai", "Haida");
		sLanguageCodeMap.put("hau", "Hausa");
		sLanguageCodeMap.put("haw", "Hawaiian");
		sLanguageCodeMap.put("heb", "Hebrew");
		sLanguageCodeMap.put("her", "Herero");
		sLanguageCodeMap.put("hil", "Hiligaynon");
		sLanguageCodeMap.put("him", "Himachali");
		sLanguageCodeMap.put("hin", "Hindi");
		sLanguageCodeMap.put("hmo", "Hiri Motu");
		sLanguageCodeMap.put("hun", "Hungarian");
		sLanguageCodeMap.put("hup", "Hupa");
		sLanguageCodeMap.put("iba", "Iban");
		sLanguageCodeMap.put("ice", "Icelandic");
		sLanguageCodeMap.put("isl", "Icelandic");
		sLanguageCodeMap.put("ibo", "Igbo");
		sLanguageCodeMap.put("ijo", "Ijo");
		sLanguageCodeMap.put("ilo", "Iloko");
		sLanguageCodeMap.put("inc", "Indic (Other)");
		sLanguageCodeMap.put("ine", "Indo-European (Other)");
		sLanguageCodeMap.put("ind", "Indonesian");
		sLanguageCodeMap.put("ina", "Interlingua (International Auxiliary language Association)");
		sLanguageCodeMap.put("ine", "Interlingue");
		sLanguageCodeMap.put("iku", "Inuktitut");
		sLanguageCodeMap.put("ipk", "Inupiak");
		sLanguageCodeMap.put("ira", "Iranian (Other)");
		sLanguageCodeMap.put("gai", "Irish");
		sLanguageCodeMap.put("iri", "Irish");
		sLanguageCodeMap.put("sga", "Irish, Old (to 900)");
		sLanguageCodeMap.put("mga", "Irish, Middle (900 - 1200)");
		sLanguageCodeMap.put("iro", "Iroquoian languages");
		sLanguageCodeMap.put("ita", "Italian");
		sLanguageCodeMap.put("jpn", "Japanese");
		sLanguageCodeMap.put("jav", "Javanese");
		sLanguageCodeMap.put("jaw", "Javanese");
		sLanguageCodeMap.put("jrb", "Judeo-Arabic");
		sLanguageCodeMap.put("jpr", "Judeo-Persian");
		sLanguageCodeMap.put("kab", "Kabyle");
		sLanguageCodeMap.put("kac", "Kachin");
		sLanguageCodeMap.put("kam", "Kamba");
		sLanguageCodeMap.put("kan", "Kannada");
		sLanguageCodeMap.put("kau", "Kanuri");
		sLanguageCodeMap.put("kaa", "Kara-Kalpak");
		sLanguageCodeMap.put("kar", "Karen");
		sLanguageCodeMap.put("kas", "Kashmiri");
		sLanguageCodeMap.put("kaw", "Kawi");
		sLanguageCodeMap.put("kaz", "Kazakh");
		sLanguageCodeMap.put("kha", "Khasi");
		sLanguageCodeMap.put("khm", "Khmer");
		sLanguageCodeMap.put("khi", "Khoisan (Other)");
		sLanguageCodeMap.put("kho", "Khotanese");
		sLanguageCodeMap.put("kik", "Kikuyu");
		sLanguageCodeMap.put("kin", "Kinyarwanda");
		sLanguageCodeMap.put("kir", "Kirghiz");
		sLanguageCodeMap.put("kom", "Komi");
		sLanguageCodeMap.put("kon", "Kongo");
		sLanguageCodeMap.put("kok", "Konkani");
		sLanguageCodeMap.put("kor", "Korean");
		sLanguageCodeMap.put("kpe", "Kpelle");
		sLanguageCodeMap.put("kro", "Kru");
		sLanguageCodeMap.put("kua", "Kuanyama");
		sLanguageCodeMap.put("kum", "Kumyk");
		sLanguageCodeMap.put("kur", "Kurdish");
		sLanguageCodeMap.put("kru", "Kurukh");
		sLanguageCodeMap.put("kus", "Kusaie");
		sLanguageCodeMap.put("kut", "Kutenai");
		sLanguageCodeMap.put("lad", "Ladino");
		sLanguageCodeMap.put("lah", "Lahnda");
		sLanguageCodeMap.put("lam", "Lamba");
		sLanguageCodeMap.put("oci", "Langue d'Oc (post 1500)");
		sLanguageCodeMap.put("lao", "Lao");
		sLanguageCodeMap.put("lat", "Latin");
		sLanguageCodeMap.put("lav", "Latvian");
		sLanguageCodeMap.put("ltz", "Letzeburgesch");
		sLanguageCodeMap.put("lez", "Lezghian");
		sLanguageCodeMap.put("lin", "Lingala");
		sLanguageCodeMap.put("lit", "Lithuanian");
		sLanguageCodeMap.put("loz", "Lozi");
		sLanguageCodeMap.put("lub", "Luba-Katanga");
		sLanguageCodeMap.put("lui", "Luiseno");
		sLanguageCodeMap.put("lun", "Lunda");
		sLanguageCodeMap.put("luo", "Luo (Kenya and Tanzania)");
		sLanguageCodeMap.put("mac", "Macedonian");
		sLanguageCodeMap.put("mak", "Macedonian");
		sLanguageCodeMap.put("mad", "Madurese");
		sLanguageCodeMap.put("mag", "Magahi");
		sLanguageCodeMap.put("mai", "Maithili");
		sLanguageCodeMap.put("mak", "Makasar");
		sLanguageCodeMap.put("mlg", "Malagasy");
		sLanguageCodeMap.put("may", "Malay");
		sLanguageCodeMap.put("msa", "Malay");
		sLanguageCodeMap.put("mal", "Malayalam");
		sLanguageCodeMap.put("mlt", "Maltese");
		sLanguageCodeMap.put("man", "Mandingo");
		sLanguageCodeMap.put("mni", "Manipuri");
		sLanguageCodeMap.put("mno", "Manobo languages");
		sLanguageCodeMap.put("max", "Manx");
		sLanguageCodeMap.put("mao", "Maori");
		sLanguageCodeMap.put("mri", "Maori");
		sLanguageCodeMap.put("mar", "Marathi");
		sLanguageCodeMap.put("chm", "Mari");
		sLanguageCodeMap.put("mah", "Marshall");
		sLanguageCodeMap.put("mwr", "Marwari");
		sLanguageCodeMap.put("mas", "Masai");
		sLanguageCodeMap.put("myn", "Mayan languages");
		sLanguageCodeMap.put("men", "Mende");
		sLanguageCodeMap.put("mic", "Micmac");
		sLanguageCodeMap.put("min", "Minangkabau");
		sLanguageCodeMap.put("mis", "Miscellaneous (Other)");
		sLanguageCodeMap.put("moh", "Mohawk");
		sLanguageCodeMap.put("mol", "Moldavian");
		sLanguageCodeMap.put("mkh", "Mon-Kmer (Other)");
		sLanguageCodeMap.put("lol", "Mongo");
		sLanguageCodeMap.put("mon", "Mongolian");
		sLanguageCodeMap.put("mos", "Mossi");
		sLanguageCodeMap.put("mul", "Multiple languages");
		sLanguageCodeMap.put("mun", "Munda languages");
		sLanguageCodeMap.put("nau", "Nauru");
		sLanguageCodeMap.put("nav", "Navajo");
		sLanguageCodeMap.put("nde", "Ndebele, North");
		sLanguageCodeMap.put("nbl", "Ndebele, South");
		sLanguageCodeMap.put("ndo", "Ndongo");
		sLanguageCodeMap.put("nep", "Nepali");
		sLanguageCodeMap.put("new", "Newari");
		sLanguageCodeMap.put("nic", "Niger-Kordofanian (Other)");
		sLanguageCodeMap.put("ssa", "Nilo-Saharan (Other)");
		sLanguageCodeMap.put("niu", "Niuean");
		sLanguageCodeMap.put("non", "Norse, Old");
		sLanguageCodeMap.put("nai", "North American Indian (Other)");
		sLanguageCodeMap.put("nor", "Norwegian");
		sLanguageCodeMap.put("nno", "Norwegian (Nynorsk)");
		sLanguageCodeMap.put("nub", "Nubian languages");
		sLanguageCodeMap.put("nym", "Nyamwezi");
		sLanguageCodeMap.put("nya", "Nyanja");
		sLanguageCodeMap.put("nyn", "Nyankole");
		sLanguageCodeMap.put("nyo", "Nyoro");
		sLanguageCodeMap.put("nzi", "Nzima");
		sLanguageCodeMap.put("oji", "Ojibwa");
		sLanguageCodeMap.put("ori", "Oriya");
		sLanguageCodeMap.put("orm", "Oromo");
		sLanguageCodeMap.put("osa", "Osage");
		sLanguageCodeMap.put("oss", "Ossetic");
		sLanguageCodeMap.put("oto", "Otomian languages");
		sLanguageCodeMap.put("pal", "Pahlavi");
		sLanguageCodeMap.put("pau", "Palauan");
		sLanguageCodeMap.put("pli", "Pali");
		sLanguageCodeMap.put("pam", "Pampanga");
		sLanguageCodeMap.put("pag", "Pangasinan");
		sLanguageCodeMap.put("pan", "Panjabi");
		sLanguageCodeMap.put("pap", "Papiamento");
		sLanguageCodeMap.put("paa", "Papuan-Australian (Other)");
		sLanguageCodeMap.put("fas", "Persian");
		sLanguageCodeMap.put("per", "Persian");
		sLanguageCodeMap.put("peo", "Persian, Old (ca 600 - 400 B.C.)");
		sLanguageCodeMap.put("phn", "Phoenician");
		sLanguageCodeMap.put("pol", "Polish");
		sLanguageCodeMap.put("pon", "Ponape");
		sLanguageCodeMap.put("por", "Portuguese");
		sLanguageCodeMap.put("pra", "Prakrit languages");
		sLanguageCodeMap.put("pro", "Provencal, Old (to 1500)");
		sLanguageCodeMap.put("pus", "Pushto");
		sLanguageCodeMap.put("que", "Quechua");
		sLanguageCodeMap.put("roh", "Rhaeto-Romance");
		sLanguageCodeMap.put("raj", "Rajasthani");
		sLanguageCodeMap.put("rar", "Rarotongan");
		sLanguageCodeMap.put("roa", "Romance (Other)");
		sLanguageCodeMap.put("ron", "Romanian");
		sLanguageCodeMap.put("rum", "Romanian");
		sLanguageCodeMap.put("rom", "Romany");
		sLanguageCodeMap.put("run", "Rundi");
		sLanguageCodeMap.put("rus", "Russian");
		sLanguageCodeMap.put("sal", "Salishan languages");
		sLanguageCodeMap.put("sam", "Samaritan Aramaic");
		sLanguageCodeMap.put("smi", "Sami languages");
		sLanguageCodeMap.put("smo", "Samoan");
		sLanguageCodeMap.put("sad", "Sandawe");
		sLanguageCodeMap.put("sag", "Sango");
		sLanguageCodeMap.put("san", "Sanskrit");
		sLanguageCodeMap.put("srd", "Sardinian");
		sLanguageCodeMap.put("sco", "Scots");
		sLanguageCodeMap.put("sel", "Selkup");
		sLanguageCodeMap.put("sem", "Semitic (Other)");
		sLanguageCodeMap.put("scr", "Serbo-Croatian");
		sLanguageCodeMap.put("srr", "Serer");
		sLanguageCodeMap.put("shn", "Shan");
		sLanguageCodeMap.put("sna", "Shona");
		sLanguageCodeMap.put("sid", "Sidamo");
		sLanguageCodeMap.put("bla", "Siksika");
		sLanguageCodeMap.put("snd", "Sindhi");
		sLanguageCodeMap.put("sin", "Singhalese");
		sLanguageCodeMap.put("sit", "Sino-Tibetan (Other)");
		sLanguageCodeMap.put("sio", "Siouan languages");
		sLanguageCodeMap.put("sla", "Slavic (Other)");
		sLanguageCodeMap.put("ssw", "Siswant");
		sLanguageCodeMap.put("slk", "Slovak");
		sLanguageCodeMap.put("slo", "Slovak");
		sLanguageCodeMap.put("slv", "Slovenian");
		sLanguageCodeMap.put("sog", "Sogdian");
		sLanguageCodeMap.put("som", "Somali");
		sLanguageCodeMap.put("son", "Songhai");
		sLanguageCodeMap.put("wen", "Sorbian languages");
		sLanguageCodeMap.put("nso", "Sotho, Northern");
		sLanguageCodeMap.put("sot", "Sotho, Southern");
		sLanguageCodeMap.put("sai", "South American Indian (Other)");
		sLanguageCodeMap.put("esl", "Spanish");
		sLanguageCodeMap.put("spa", "Spanish");
		sLanguageCodeMap.put("suk", "Sukuma");
		sLanguageCodeMap.put("sux", "Sumerian");
		sLanguageCodeMap.put("sun", "Sudanese");
		sLanguageCodeMap.put("sus", "Susu");
		sLanguageCodeMap.put("swa", "Swahili");
		sLanguageCodeMap.put("ssw", "Swazi");
		sLanguageCodeMap.put("sve", "Swedish");
		sLanguageCodeMap.put("swe", "Swedish");
		sLanguageCodeMap.put("syr", "Syriac");
		sLanguageCodeMap.put("tgl", "Tagalog");
		sLanguageCodeMap.put("tah", "Tahitian");
		sLanguageCodeMap.put("tgk", "Tajik");
		sLanguageCodeMap.put("tmh", "Tamashek");
		sLanguageCodeMap.put("tam", "Tamil");
		sLanguageCodeMap.put("tat", "Tatar");
		sLanguageCodeMap.put("tel", "Telugu");
		sLanguageCodeMap.put("ter", "Tereno");
		sLanguageCodeMap.put("tha", "Thai");
		sLanguageCodeMap.put("bod", "Tibetan");
		sLanguageCodeMap.put("tib", "Tibetan");
		sLanguageCodeMap.put("tig", "Tigre");
		sLanguageCodeMap.put("tir", "Tigrinya");
		sLanguageCodeMap.put("tem", "Timne");
		sLanguageCodeMap.put("tiv", "Tivi");
		sLanguageCodeMap.put("tli", "Tlingit");
		sLanguageCodeMap.put("tog", "Tonga (Nyasa)");
		sLanguageCodeMap.put("ton", "Tonga (Tonga Islands)");
		sLanguageCodeMap.put("tru", "Truk");
		sLanguageCodeMap.put("tsi", "Tsimshian");
		sLanguageCodeMap.put("tso", "Tsonga");
		sLanguageCodeMap.put("tsn", "Tswana");
		sLanguageCodeMap.put("tum", "Tumbuka");
		sLanguageCodeMap.put("tur", "Turkish");
		sLanguageCodeMap.put("ota", "Turkish, Ottoman (1500 - 1928)");
		sLanguageCodeMap.put("tuk", "Turkmen");
		sLanguageCodeMap.put("tyv", "Tuvinian");
		sLanguageCodeMap.put("twi", "Twi");
		sLanguageCodeMap.put("uga", "Ugaritic");
		sLanguageCodeMap.put("uig", "Uighur");
		sLanguageCodeMap.put("ukr", "Ukrainian");
		sLanguageCodeMap.put("umb", "Umbundu");
		sLanguageCodeMap.put("und", "Undetermined");
		sLanguageCodeMap.put("urd", "Urdu");
		sLanguageCodeMap.put("uzb", "Uzbek");
		sLanguageCodeMap.put("vai", "Vai");
		sLanguageCodeMap.put("ven", "Venda");
		sLanguageCodeMap.put("vie", "Vietnamese");
		sLanguageCodeMap.put("vol", "Volapuk");
		sLanguageCodeMap.put("vot", "Votic");
		sLanguageCodeMap.put("wak", "Wakashan languages");
		sLanguageCodeMap.put("wal", "Walamo");
		sLanguageCodeMap.put("war", "Waray");
		sLanguageCodeMap.put("was", "Washo");
		sLanguageCodeMap.put("cym", "Welsh");
		sLanguageCodeMap.put("wel", "Welsh");
		sLanguageCodeMap.put("wol", "Wolof");
		sLanguageCodeMap.put("xho", "Xhosa");
		sLanguageCodeMap.put("sah", "Yakut");
		sLanguageCodeMap.put("yao", "Yao");
		sLanguageCodeMap.put("yap", "Yap");
		sLanguageCodeMap.put("yid", "Yiddish");
		sLanguageCodeMap.put("yor", "Yoruba");
		sLanguageCodeMap.put("zap", "Zapotec");
		sLanguageCodeMap.put("zen", "Zenaga");
		sLanguageCodeMap.put("zha", "Zhuang");
		sLanguageCodeMap.put("zul", "Zulu");
		sLanguageCodeMap.put("zun", "Zuni");
	}

	public static String getLanguageText(String code) {
		String langText = sLanguageCodeMap.get(code);
		if (langText == null) {
			return "Undefined";
		} else {
			return langText;
		}
	}


	private static StringBuilder mFormatBuilder = new StringBuilder();
	private static Formatter mFormatter = new Formatter(mFormatBuilder, Locale.getDefault());

	public static String stringForTime(int month, int date, int year, int hour, int minute) {
		mFormatBuilder.setLength(0);
		return mFormatter.format("%02d%02d%02d-%02d%02d", month, date, year, hour, minute).toString();
    }

    public static String stringForTime(int timeMs) {
        int totalSeconds = timeMs / 1000;

        int seconds = totalSeconds % 60;
        int minutes = (totalSeconds / 60) % 60;
        int hours   = totalSeconds / 3600;

        mFormatBuilder.setLength(0);
        if (hours > 0) {
            return mFormatter.format("%d:%02d:%02d", hours, minutes, seconds).toString();
        } else {
            return (timeMs >= 0) ? mFormatter.format("%02d:%02d", minutes, seconds).toString()
                                 : mFormatter.format("--:--").toString();
        }
    }

    public static String stringForTime(int iStart, int iDuration) {
    	int iStartHH = iStart / 60;
    	int iStartMM = iStart % 60;
		int	iEndHH		= ((iStart + iDuration) / 60) % 24;
		int	iEndMM		= (iStart + iDuration) % 60;

		String return_Time = String.format(	"%02d:%02d~%02d:%02d"
											, iStartHH
											, iStartMM
											, iEndHH
											, iEndMM);
		return	return_Time;
    }

    public static String formatFileSize(long number) {
		boolean shorter = false;

        float result = number;
        String suffix = "KB";
        //if (result > 900) {
        //    suffix = "KB";
        //    result = result / 1024;
        //}
        if (result > 900) {
            suffix = "MB";
            result = result / 1024;
        }
        if (result > 900) {
            suffix = "GB";
            result = result / 1024;
        }
        if (result > 900) {
            suffix = "TB";
            result = result / 1024;
        }
        if (result > 900) {
            suffix = "PB";
            result = result / 1024;
        }
        String value;
        if (result < 1) {
            value = String.format("%.2f", result);
        } else if (result < 10) {
            if (shorter) {
                value = String.format("%.1f", result);
            } else {
                value = String.format("%.2f", result);
            }
        } else if (result < 100) {
            if (shorter) {
                value = String.format("%.0f", result);
            } else {
                value = String.format("%.2f", result);
            }
        } else {
            value = String.format("%.0f", result);
        }
        return value + suffix;
    }

    public static String getMJD_Date(int MJD) {
		String	return_Date;
		
		int	y_dash, m_dash, k;
		int	day, month, year;
		
		y_dash = ( MJD * 100 - 1507820) / 36525;
		m_dash = ( MJD * 10000 - 149561000 - INT(y_dash * 3652500, 10000) ) / 306001;
		
		day	= (MJD * 10000 - 149560000 - INT(y_dash * 3652500, 10000) - INT(m_dash * 306001, 10000)) / 10000;
		if(m_dash == 14 || m_dash == 15)
			k = 1;
		else
			k = 0;

		year = y_dash + k + 1900;
		month = m_dash - 1 - k * 12;
		//weekday = ((MJD + 2) % 7) + 1;
		
		return_Date	= year + ". " + month + ". " + day + ".";

		return return_Date;
	}
	
	public static int getMJD_YY(int MJD) {
		int	y_dash, m_dash, k;
		int	year;
		
		y_dash = ( MJD * 100 - 1507820) / 36525;
		m_dash = ( MJD * 10000 - 149561000 - INT(y_dash * 3652500, 10000) ) / 306001;
		
		if(m_dash == 14 || m_dash == 15)
			k = 1;
		else
			k = 0;

		year = y_dash + k + 1900;
		
		return year;
	}
	
	public static int getMJD_MM(int MJD) {
		int	y_dash, m_dash, k;
		int	month;
		
		y_dash = ( MJD * 100 - 1507820) / 36525;
		m_dash = ( MJD * 10000 - 149561000 - INT(y_dash * 3652500, 10000) ) / 306001;
		
		if(m_dash == 14 || m_dash == 15)
			k = 1;
		else
			k = 0;

		month = m_dash - 1 - k * 12;
		
		return month;
	}
	
	public static int getMJD_DD(int MJD) {
		int	y_dash, m_dash;
		int	day;
		
		y_dash = ( MJD * 100 - 1507820) / 36525;
		m_dash = ( MJD * 10000 - 149561000 - INT(y_dash * 3652500, 10000) ) / 306001;
		
		day	= (MJD * 10000 - 149560000 - INT(y_dash * 3652500, 10000) - INT(m_dash * 306001, 10000)) / 10000;
		
		return day;
	}
	
	public static int INT(int x, int y) {
		return (x/y)*y;
	}
}
