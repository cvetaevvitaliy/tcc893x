/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/

#ifndef		__FIG_H__
#define		__FIG_H__

#define SVN_REV        $Rev: 81 $
#define COMPILE_TIME   20131119_20:47:00
#define FIC_MAJOR_VER  2
#define FIC_MINOR_VER  0
#define FIC_EXTEND_VER 12
#define FIC_BUILD_VER  63
#define FIC_REV_VER    "2.0.12.63"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
	#define CALLSPEC	__cdecl
#else
	#define CALLSPEC
#endif

typedef	unsigned char	uchar;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned int uint;

#ifndef NULL
#define NULL (void *)0
#endif

#define FICERR_FIG1_5_NOTREADY_SERVICE			11050
#define FICERR_FIG1_4_NOTREADY_SRVCOMP			11041
#define FICERR_FIG1_4_NOTREADY_SRVCOMP1			11040
#define FICERR_FIG1_1_NOTREADY_SERVICE			11010
#define FICERR_FIG0_22_ALREADY_TIIDATA			10221
#define FICERR_FIG0_22_NOTREADY_MAINID			10220
#define FICERR_FIG0_16_ALREADY_PROGNUM			10161
#define FICERR_FIG0_16_NOTREADY_SERVICE			10160
#define FICERR_FIG0_13_ALREADY_USERAPPL			10130
#define FICERR_FIG0_7_BLOCK						10071
#define FICERR_FIG0_7_RETURN					10070
#define FICERR_FIG0_5_NOTREADY_SERVICE			10051
#define FICERR_FIG0_5_NOTREADY_SRVCOMP			10050
#define FICERR_FIG0_4_NOTREADY_SRVCOMP			10041
#define FICERR_FIG0_4_ALREADY_CA_FIELD			10040
#define FICERR_FIG0_3_NOTREADY_SRVCOMP			10032
#define FICERR_FIG0_3_NOTREADY_SRVCOMP1			10031
#define FICERR_FIG0_3_ALREADY_SRVCOMP			10030
#define FICERR_FIG_NODATA						2
#define FICERR_FIBD_ENDMARKER					1
#define FICERR_SUCCESS							0

#define FICERR_FIBD_FICSYNC_FAILURE				-1
#define FICERR_FIBD_CRC_FAILURE					-2
#define FICERR_FIBD_UNKNOWN_FIGTYPE				-3
#define FICERR_FIBD_INVALID_LENGTH				-4
#define FICERR_FIG0_NEXT_FIG					-1000
#define FICERR_FIG0_NEXT_FIG1					-1001
#define FICERR_FIG0_0_NO_ENSEMBLEARRAY			-10000
#define FICERR_FIG0_1_FULL_SUBCHARRAY			-10010
#define FICERR_FIG0_1_INVALID_LENGTH			-10011
#define FICERR_FIG0_1_NO_SUBCHARRAY				-10012
#define FICERR_FIG0_2_FULL_SERVICEARRAY			-10020
#define FICERR_FIG0_2_FULL_SRVCOMPARRAY			-10021
#define FICERR_FIG0_2_INVALID_LENGTH			-10022
#define FICERR_FIG0_2_NO_SRVARRAY				-10023
#define FICERR_FIG0_2_NO_SRVCOMPARRAY			-10024
#define FICERR_FIG0_3_INVALID_LENGTH			-10030
#define FICERR_FIG0_3_NO_SRVCOMPARRAY			-10031
#define FICERR_FIG0_4_INVALID_LENGTH			-10040
#define FICERR_FIG0_4_NO_SRVCOMPARRAY			-10041
#define FICERR_FIG0_5_INVALID_LENGTH			-10050
#define FICERR_FIG0_5_NO_SRVCOMPARRAY			-10051
#define FICERR_FIG0_6_NO_LINKARRAY				-10060
#define FICERR_FIG0_6_FULL_LINKARRAY			-10061
#define FICERR_FIG0_6_INVALID_LENGTH			-10062
#define FICERR_FIG0_8_INVALID_LENGTH			-10080
#define FICERR_FIG0_8_NO_SRVCOMPARRAY			-10081
#define FICERR_FIG0_13_FULL_USERAPPLARRAY		-10130
#define FICERR_FIG0_13_INVALID_LENGTH			-10131
#define FICERR_FIG0_13_NO_USERAPPLARRAY			-10132
#define FICERR_FIG0_16_NO_PROGNUMARRAY			-10160
#define FICERR_FIG0_16_OTHER_ENSEMBLE			-10161
#define FICERR_FIG0_16_FULL_PROGNUMARRAY		-10162
#define FICERR_FIG0_16_INVALID_LENGTH			-10063
#define FICERR_FIG0_17_OTHER_ENSEMBLE			-10170
#define FICERR_FIG0_17_NO_PROGTYPEARRAY			-10171
#define FICERR_FIG0_17_FULL_PROGTYPEARRAY		-10172
#define FICERR_FIG0_17_INVALID_LENGTH			-10073
#define FICERR_FIG0_18_NO_ANNARRAY				-10180
#define FICERR_FIG0_18_FULL_ANNARRAY			-10181
#define FICERR_FIG0_18_INVALID_LENGTH			-10182
#define FICERR_FIG0_19_NO_ANNARRAY				-10190
#define FICERR_FIG0_19_FULL_ANN_SUPPORT			-10191
#define FICERR_FIG0_19_INVALID_LENGTH			-10192
#define FICERR_FIG0_21_NO_FIARRAY				-10210
#define FICERR_FIG0_21_OTHER_ENSEMBLE			-10211
#define FICERR_FIG0_21_FULL_FIARRAY				-10212
#define FICERR_FIG0_21_FULL_FIARRAY2			-10213
#define FICERR_FIG0_21_INVALID_LENGTH			-10214
#define FICERR_FIG0_22_NO_TIIARRAY				-10220
#define FICERR_FIG0_22_INVALID_LENGTH			-10221
#define FICERR_FIG0_24_NO_OESRVARRAY			-10240
#define FICERR_FIG0_24_FULL_OESRV_ARRAY			-10241
#define FICERR_FIG0_24_FULL_OESRV_ARRAY1		-10242
#define FICERR_FIG0_24_INVALID_LENGTH			-10243
#define FICERR_FIG1_1_NO_SERVICEARRAY			-11011
#define FICERR_FIG1_2_NO_PROGTYPEDOWNINFO		-11020
#define FICERR_FIG1_4_NO_SRVCOMPARRAY			-11041
#define FICERR_FIG1_5_NO_SERVICEARRAY			-11052
#define FICERR_FIG1_6_NO_XPADLABELARRAY			-11061

#define		FIG0			0x0
#define		FIG1			0x1
#define		FIG5			0x5
#define		FIG6			0x6
#define		FIG7			0x7

#define		Ext00		0
#define		Ext01		1
#define		Ext02		2
#define		Ext03		3
#define		Ext04		4
#define		Ext05		5
#define		Ext06		6
#define		Ext07		7
#define		Ext08		8
#define		Ext09		9
#define		Ext10		10
#define		Ext11		11
#define		Ext12		12
#define		Ext13		13
#define		Ext14		14
#define		Ext15		15
#define		Ext16		16
#define		Ext17		17
#define		Ext18		18
#define		Ext19		19
#define		Ext20		20
#define		Ext21		21
#define		Ext22		22
#define		Ext23		23
#define		Ext24		24
#define		Ext25		25
#define		Ext26		26
#define		Ext27		27
#define		Ext28		28
#define		Ext29		29
#define		Ext30		30
#define		Ext31		31
#define		Ext01_R		40

#define		MAX_DAB_BB_NUM	4

#define		INITVAL_SCIDS	0xff
#define         INVALID_INT_CODE 0xff

#define		_NumOfSvc			64					/**< max num of ServiceInfo */
#define		_NumOfSubCh			(_NumOfSvc + 0)		/**< max num of SubChInfo */
#define		_NumOfSvcComp		_NumOfSubCh			/**< max num of SvcComponentInfo */

#define		_NumOfProgramNum	5					/**< max_num of ProgNumberInfo. */
#define		_NumOfProgType		_NumOfSvcComp		/**< max_num of ProgramTypeInfo. */
#define		_NumOfMainId		3					/**< max_num of TIIDatabaseInfo. */
#define		_NumOfAnn			5					/**< max_num of announcement. */
#define		_NumOfUserApp		_NumOfSvc			/**< max_num of UserAppInfo. */
#define		_NumOfFI			10					/**< max_num of DABFrequencyInfo. */
//#define		_NumServiceLink	10					/**< max_num of ServiceLinkInfo. */

#define		_NumServiceLink	32					/**< max_num of ServiceLinkInfo. */

#define		_NumOfOESvc		_NumOfSvc			/**< max_num of OEServiceInfo. */
#define		_NumOfXpadLabel 	_NumOfSvc			/**< max_num of XpadUserApplLabel. */

#define		ONLY_AUDIO_SERVICE	0

#define CRC16_POLYNOMIAL 		0x1021		//x^16 + x^12 + x^5 + 1;

#define USR_APP_TY_RESERVE		0					/**< refer to ETSI TS 101.756 Table 16. */
#define USR_APP_TY_DLS			1					/**< refer to ETSI TS 101.756 Table 16. */
#define USR_APP_TY_SLIDESHOW	2					/**< refer to ETSI TS 101.756 Table 16. */
#define USR_APP_TY_BWS			3					/**< refer to ETSI TS 101.756 Table 16. */
#define USR_APP_TY_TPEG			4					/**< refer to ETSI TS 101.756 Table 16. */
#define USR_APP_TY_DGPS			5					/**< refer to ETSI TS 101.756 Table 16. */
#define USR_APP_TY_TMC			6					/**< refer to ETSI TS 101.756 Table 16. */
#define USR_APP_TY_EPG			7					/**< refer to ETSI TS 101.756 Table 16. */
#define USR_APP_TY_PAD			8					/**< refer to ETSI TS 101.756 Table 16. */
#define USR_APP_TY_BIFS			9
#define USR_APP_TY_CAAGENT		10

#define USR_APP_TY_MAXNUM		10		//USR_APP_TY_RESERVE ~ USR_APP_TY_BIFS

/** not used */
typedef enum{
	SVCCOMP_LABEL_NONE=1,
	SVCCOMP_LABEL_SEC_NOT,
	SVCCOMP_LABEL_SEC_TOBE
}SVCCOMP_LABEL_INFO;

/**FIG 0/1 */
typedef struct{	
	uchar	SubChId;		/**< 6bits	Sub channel Id */
	uchar	TblIndex;		/**< 6bits	TableIndex */
	uchar	Form_Opt_Prot;  /**< [3] : FormFlag@n
								 [2] : Option@n
								 [1~0] : protection */
	ushort	StrtAdd;		/**< 10bits Start Address */
	ushort	SubChSize;		/**< 10bits Sub channel size */
	uchar   fec_scheme;
}SubChInfo;

/** FIG 0/2 and FIG1 */
typedef struct {
	uchar		Order;			/**< 4bits	0 : primary, 1: secondary */
	uchar		TMId;			/**< 2bits	Transport Mechanism Id */
	uchar		ASCTy_DSCTy;	/**< 6bits	Audio Service Component Type */
	uchar		SubCh_FIDCId;	/**< 6bits	SubChId or FIDCId in FIG 0/4 */
	uchar		CAflag;			/**< 1bit	CA Flag */
	uchar		DG_MFflag;		/**< 1bit	DG Flag or MF flag */
	uchar		Language;		/**< 8bit	language field of FIG 0/5 */
	uchar		SCIdS;			/**< 4bit		Service component Identifier within ther Service */
	uchar		CAOrgFlag;		/**< 1bit */
	uchar		charset;		/**< character set */
	uchar		Label[16];		/**< 16bytes	Service component Label */
	ushort		SCId;			/**< 12bits	Service Component Id */
	ushort		PackAdd;		/**< 10bits	Packet Address */
	ushort		CAOrg;			/**< 16bits	conditional access organization */
	ushort		CharacterFlag;	/**< refer to ETSI EN 300 401 5.2.2.1 */
	ulong		SId;			/**< 32bit */
}SvcComponentInfo;

/** FIG 0/10 */
typedef struct{
	uchar	LSI;			/**<1bit */
	uchar	ConfInd;		/**<1bit */
	uchar	UTCflag;		/**<1bit */
	ulong	UTC;			/**<11 or 27bit */
	ulong	MJD;			/**<17bit	Modified Julian Date */
}DateTimeInfo;

/** FIG 0/12 */
typedef struct{
	uchar	Lflag;			/**< 1bit */
	uchar	FF;				/**< 2bit */
	uchar	Language;		/**< 8bit */
//	uchar	FlagField[192];	/**< 32 ~ 192bit */
	uchar	FlagField[32];	/**< 32 ~ 192bit */
	ushort	EId;			/**< 16bit */
}ProgTypePrevInfo;

/** FIG 0/16 */
typedef struct {	
	uchar	ContFlag;		/**< 1bit */
	uchar	UpdateFlag;		/**< 1bit */
	ushort	SId;			/**< 16bit */
	ushort	PNum;			/**< 16bit */
	ushort	NewSId;			/**< 16bit */
	ushort	NewPNum;		/**< 16bit */
}ProgNumberInfo;

/** FIG 0/17 */
typedef struct{	
	uchar	SD;				/**< 1bit */
	uchar	PS;				/**< 1bit */
	uchar	NFC;			/**< 2bit */
	uchar	Language;		/**< 8Bit */
	uchar	IntCode;		/**< 5bit */
	uchar	CoarseCode;		/**< 6bit */
	uchar	FineCode;		/**< 8bit */
	ushort	SId;			/**< 16bit */
}ProgramTypeInfo;


/** FIG 1/2 */
typedef struct {	
	uchar country_flag;
	uchar coarse_code;
	uchar fine_code;
	uchar language;
	uchar charset;
	uchar character_field[16];
	ushort chr_flag_field;
	uchar ECC;
	uchar country_id;	
}ProgramTypedownInfo;


/** FIG 0/2	and FIG1/1 */
typedef struct{	
	ulong		SId;			/**< 32bits	CountryId + serviceReference // ECC + CountryId + ServiceReference */
	uchar		charset;		/**< character set */
	uchar		SvcLabel[16];	/**< 16bytes	Service Label(Program service and Data service) */
	ushort		CharacterFlag;	/**< refer to ETSI EN 300 401 5.2.2.1 */
	uchar		CAId;			/**< 3bit */
	uchar		NumSvcComp;		/**< 4bits	Number of Service Component */
} ServiceInfo;


/** FIG 0/0 */
typedef struct{	
	uchar	AlFlag;				/**< 1bit		Al flag */
	uchar   NumSubCh;			/**< a number of SubChInfo */
	uchar   NumSvc;				/**< a number of ServiceInfo */
	uchar	NumProgram;			/**< a number of ProgNumberInfo */
	uchar   NumSvcComp;			/**< a number of SvcComponentInfo */
	uchar	NumUserApp;			/**< a number of FIG0/13 */
	uchar	NumAnn;				/**< a number of FIG0/18 */
	uchar	NumProgType;		/**< a number of FIG0/17 */
	uchar	NumSvcLink;			/**< a number of FIG0/6 */
	uchar	NumOESvc;			/**< a number of FIG0/24 other ensemble*/
	uchar	NumFI;				/**< a number of FIG0/21 */
	uchar	NumOEFI;				/**< a number of FIG0/21 other ensemble*/
	uchar	charset;			/**< character set */
	uchar	EnsemLabel[16];		/**< 16bytes	Ensemble Label */
	ushort CharacterFlag;		/**< refer to ETSI EN 300 401 5.2.2.1 */
	ushort	EId;				/**< 16bits	country Id Ensemble reference */
} EnsembleInfo;

typedef struct{
    ushort AswFlag;
    uchar SubchId;
    ushort ClusterId;
    uchar New;
    uchar Reg;
    uchar RegIdRow;
    uchar Count;
} Fig_0_19;

typedef struct {
    ushort SId;
    ushort AsuFlag;
    uchar NumCluster;
    uchar ClusterIds[23];
} Fig_0_18;

typedef struct {
    Fig_0_18 Fig018[64];        /* max service number */
    Fig_0_19 Fig019[23];
    uchar NumSId;
}AnnouncementInfo;

/** FIG 0/22 */
typedef struct{
	uchar	SubId;				/**< 5bits SubId */
	ushort	TD;					/**< 11bits Time Delay */
	ushort	LatitudeOffset;		/**< 16bits Latitude offset */
	ushort	LongitudeOffset;	/**< 16bits Longitude offset */
}TIISubIdInfo;

/** FIG 0/22 */
typedef struct{
	short	CoaLatitude;		/**< 16bits	Coarse Latitude */
	short	CoaLongitude;		/**< 16bits	Coarse Longitude */
	uchar	MainId;				/**< 7bits	TII Main Id */
	uchar	FineLatitude;		/**< 4bits	Fine Latitude */
	uchar	FineLongitude;		/**< 4bits	Fine Longitude */
	uchar	NumSubId;			/**< 3bits 	Number of SubId fields */
	TIISubIdInfo	TIISubId[20]; /**< view TIISubIdInfo */
} TIIDatabaseInfo;

/** FIG 5 */
typedef struct {
	uchar Message[28];	/**< message */
	uchar NumMsg;		/**< length of message */
	uchar flag;			/**< 0=TMC user Message @n
							 1=TMC system message @n
							 2=EWS message @n
							 3=EWS control info @n */
}TMC_EWSInfo;

// steveyoon77_100429
/// the messages brought from DAB TMC
typedef struct {
	uchar Message[28]; ///< FIG type 5 extension 1 data field; 
	///< the message size was limitted by 1 FIB data field size(30bytes) and there is added 2 bytes for FIG header
	uchar MessageType; ///< 0: User message type, 1: System message type
	uchar TCId; ///< Type of Component ID, this may be used for identification of the message in the same extension channel
	uchar NumMessage; ///< Number of Message
	uchar MessageLength; ///<Length of Message
}DabTmcInfoT;

typedef struct {
	ushort	AppTy;		/**< User application Type */
	uchar	AppLen;		/**< User Application Type length */
	uchar	AppData[24]; /**< User Application Data */
}UserAppl;

/** FIG 0/13 */
typedef struct {
	ulong	SId;		/**< Service ID */
	uchar	SCIdS;		/**< scids */
	uchar	NumApp;		/**< appl */
	UserAppl App[6];
}UserAppInfo;

/** FIG 1/6 */
typedef struct {
	uchar	charset;	/**< character set */
	uchar	Label[16];	/**< label */
	ushort	AppTy;		/**< X-PAD application type */
	ulong	SId;		/**< Service ID */
	uchar	SCIdS;		/**< SCIdS */
       ushort     CharacterFlag;
}XpadUserApplLabel;

/** This is not used. */
typedef struct {
	int		PacketAddr;
	int		TId;
	int		SegNum;
	uchar	bCRC;
	uchar	bSegment;
	uchar	bAccess;
	uchar	DGTy;
	uchar	Cont_Idx;
	uchar	Rep_Idx;
	uchar	*DataField;
}PacketInfo;

/** FIG 0/6 */
typedef struct {
	uchar	IdLQ;			/**< Identifier List Qualifier */
	uchar	Shd;			/**< Shorthand indicator */
	uchar	ECC;
	ulong	SId;
}SLIdList;

typedef struct {
	uchar	OE;				/** other ensemble */
    uchar   CN;             /** current / next */       /* 20120901 */
	uchar	PD;				/** 16 or 32 bits SId */

	uchar	LA;				/**< Linkage Actuator */
	uchar	SH;				/**< Soft/Hard */
	uchar	ILS;			/**< International linkage set indicator */
//	uchar	IdLQ;			/**< Identifier List Qualifier */
//	uchar	Shd;			/**< Shorthand indicator */
	uchar	NumOfIds;
	uchar	Mode;			/**< 2, if P/D = 0, ILS = 0
								 3, if P/D = 0, ILS = 1
								 4, if P/D = 1, ILS = 0 or 1 */
	//uchar	IdList[24];
	SLIdList IdList[24];
	ushort	LSN;			/**< Linkage Set Number */
}ServiceLinkInfo;

/** FIG 0/21 */
typedef struct {
	ushort	Id;
	uchar	RM;
	uchar	ContF;			/**< continuity flag */
	uchar	NumFreqList;	/**< a length of freqlist */
	uchar	Mode;			/**< 1, if R&M = 0000 or 0001
								 2, if R&M = 1001, 1010, or 1100
								 3, if R&M = 1100
								 4, if R&M = 0110, 1110

							*/
//	uchar	freqlist[26];
	uchar	Ctrl_or_Id[8];
	uint	Freq[8];
}FIListInfo;

/** FIG 0/21 */
typedef struct {
	ushort		RId;		/**< Resion ID */
	uchar		CN;
	uchar		OE;			/**< Other Ensemble. */
	uchar		clen;
	uchar		NumFIList;	/**< Length of FI list */
	FIListInfo		filist[20]; /**< FI list */
}DABFrequencyInfo;

/** FIG 0/24 */
typedef struct {
	ulong		SId;
	uchar		CAId;
	uchar		NumEId;
	ushort		EId[12];
}OEServiceInfo;

/** FIG 6 */
typedef struct {
	uchar		CN;
	uchar		PD;
	uchar		OE;
	uchar		LEF;
	uchar		S_CASysId;
	uchar		CAIntCharLen;
	uchar		CAIntChar[24];
	ulong		SId;
	ushort		CASysId;
}FIG6Info;

/** This is not used. */
typedef struct{
	ushort	EId;			/* 16bits	country Id Ensemble reference */
	ushort	CharFlag;
	uchar   NumSvc;
	uchar   NumSvcComp;
	uchar   NumSubCh;
	uchar	Band;
	uchar	FreqIdx;
#ifdef  TCC77X
	SVCCOMP_LABEL_INFO svccomp_label_info;
#endif
#ifdef TCC76X
	uchar	pll_offset;
	uchar	svcidx[_MaxSvcPerEsb];
	uchar	Label[16];	// 16bytes	Ensemble Label
#else
	uchar	Pll_offset;
	uchar	EnsemLabel[16 + 1];	// 16bytes	Ensemble Label
	ServiceInfo			Service[_NumOfSvc];
	SubChInfo			SubCh[_NumOfSubCh];
	SvcComponentInfo	SvcComp[_NumOfSvcComp];
#endif
}EnsembleListInfo;

#ifdef TCC76X
/** This is not used. */
typedef struct {
	ulong		SId;
	ushort		DataRate;
	ushort		CharFlag;
	uchar		ASCTy_DSCTy;
	uchar		TMId;
	uchar		SubChId;
	uchar		Order;
	uchar		LabelInfo;
	uchar		Label[16 + 1];
	uchar		SubChInfo[7];
}ServiceListInfo;

/** This is not used. */
typedef struct{
	ulong		SId;
	ushort 		EId;
	uchar		SubChId;
	uchar		SvcLabel[16 + 1];
}FavoriteListInfo;
#endif

typedef void (*tFICEXTPARSER)(uchar *In, int iLen);

/* declare fig variable */
typedef struct
{
	EnsembleInfo 		*psEsb;					/* FIG 0/0 */
	SubChInfo 			*psSubCh;				/* FIG 0/1 */
	ServiceInfo 		*psService;				/* FIG 0/2 */
	SvcComponentInfo 	*psSvcComponent;
	DateTimeInfo 		*psDateTime;			/* FIG 0/10 */
	DateTimeInfo		*psBackUpTime;			/* FIG 0/10 */
	ProgramTypeInfo 	*psProgType;
	ProgramTypedownInfo	*psProgType_download;	/* FIG 1/2 */
	ProgNumberInfo 		*psProgNumber;			/* FIG 0/16 */
	AnnouncementInfo 	*psAnnouncement;		/* FIG 0/18 19 */
	UserAppInfo	 		*psUserApp;				/* FIG 0/13 */
	TIIDatabaseInfo 	*psTIIDatabase;
	TMC_EWSInfo 		*psTMC_EWS;				/* FIG 5/1 */
	DabTmcInfoT		*psDabTmcInfo;
	uchar 				*pFIG5_Paging;
	FIG6Info			*psFIG6;
	ServiceLinkInfo		*psSrvLinkInfo;
	DABFrequencyInfo	*psFilist;
	OEServiceInfo *psOEService;
	DABFrequencyInfo 	*psOEFilist;
}FIC_INFO_T;

typedef struct
{
	uchar gucFig_RECONF;			/**< reconfiguration is occured. */

	uchar   gucAppearAnnouncement;	/**< fig 0/19 is occured. */
	uchar gucAswCnt;

	uchar LTO_unique;				/**< a parameter of FIG 0/9 */
	char Ensemble_LTO;				/**< a parameter of FIG 0/9 */
	uchar Ensemble_ECC;				/**< a parameter of FIG 0/9 */
	uchar Inter_Tb_Id;				/**< a parameter of FIG 0/9 */
	
	tFICEXTPARSER ExtenalParsingFig5;
	uchar nEnsembleLabel;	
	int label_update;
	int ptype_update;

	uchar NumTiiMainId;
	uchar NumTotalTii;
}FIC_USER_INFO_T;

/*extern EnsembleListInfo 	*psEnsembleList;*/		/*declare this variable out of fic parser.*/
extern FIC_USER_INFO_T gFicParserInfo[MAX_DAB_BB_NUM];

/*============================================================================*/
/** @brief software crc routine.                                              */
/** @param data : pointer of fib data                                         */
/** @param init : initial value of crc. In case of fic, it is -1              */
/** @param len : length of fib data except crc field. In case of fic, it is 30*/
/** @param flag : direction. In case of fic, it is 1.                         */
/** @return 1 : CRC check ok @n                                               */
/**			0 : CRC check fail.                                               */
/*============================================================================*/
extern uchar CALLSPEC FIC_CRC16(uchar *data, ushort init, ushort len, uchar flag);

/** @defgroup group1 XXX_Mem_Align
 * These functions allocate memory to each parameters.
	@see DMB_BufferAlign
 */
/** @{ */
extern void CALLSPEC Service_Mem_Align(int handle, ServiceInfo *p, uchar max);
extern void CALLSPEC SvcComponent_Mem_Align(int handle, SvcComponentInfo *p, uchar max);
extern void CALLSPEC SubCh_Mem_Align(int handle, SubChInfo *p, uchar max);
extern void CALLSPEC UserApp_Mem_Align(int handle, UserAppInfo *p, uchar max);
extern void CALLSPEC ProgNumber_Mem_Align(int handle, ProgNumberInfo *p, uchar max);
extern void CALLSPEC TII_Mem_Align(int handle, TIIDatabaseInfo *p, uchar max);
extern void CALLSPEC Announcement_Mem_Align(int handle, AnnouncementInfo *p, uchar max);  
extern void CALLSPEC ProgramType_Mem_Align(int handle, ProgramTypeInfo *p, uchar max);
extern void CALLSPEC Ensemble_Mem_Align(int handle, EnsembleInfo *p);
extern void CALLSPEC ProgramType_Mem_Align(int handle, ProgramTypeInfo *p, uchar max);
extern void CALLSPEC DateTime_Mem_Align(int handle, DateTimeInfo *p);
extern void CALLSPEC Paging_Mem_Align(int handle, uchar *p);
extern void CALLSPEC TMC_EWS_Mem_Align(int handle, TMC_EWSInfo *p);
extern void CALLSPEC DabTmc_Mem_Align(int handle, DabTmcInfoT * p);
extern void CALLSPEC ServiceLinkInfo_Mem_Align(int handle, ServiceLinkInfo*p, uchar max);
extern void CALLSPEC FrequencyListInfo_Mem_Align(int handle, DABFrequencyInfo*p, uchar max);
extern void CALLSPEC OEFrequencyListInfo_Mem_Align(int handle, DABFrequencyInfo*p, uchar max);
extern void CALLSPEC OEServiceInfo_Mem_Align(int handle, OEServiceInfo * p, uchar max);
extern void CALLSPEC FIG6Info_Mem_Align(int handle, FIG6Info*p);
extern void CALLSPEC ProgramType_Download_Mem_Align(int handle, ProgramTypedownInfo * p);
extern void CALLSPEC XPadLabel_Mem_Align(int handle, XpadUserApplLabel * p, uchar max);
/** @} */
extern void CALLSPEC Set_SubChId_To_Parser(int handle, uchar subch_id);

extern uchar CALLSPEC FIC_GetSCIdS(int handle, ulong SId, ushort SCId);
/*========================================================================*/
/** @brief Fib parser													  */
/** @param In : a pointer of fib data									  */
/** @return none														  */
/*========================================================================*/
/* crcFlag = 0 : crc success, crcFlag = 1: crc failure */
extern int FIBDecoding(int handle, uchar *In, int crcFlag);

void Parsing_Buffer_Init(int handle, uchar isNewFreq);
void FIC_TrackingFig0_Enable(int enable);
int FIB_GetMaxErrorMessage(int handle);
int FIB_GetErrorMessages(int handle, int * pMessage, unsigned int max_msg_cnt);
#undef CALLSPEC

#ifdef __cplusplus
}
#endif

#endif	/* _FIG_H_ */

/* end of file */

