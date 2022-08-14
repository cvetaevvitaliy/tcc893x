/**

  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

#ifndef __TCC_OMX_INDEX_H__
#define __TCC_OMX_INDEX_H__

#define PV_OMX_COMPONENT_CAPABILITY_TYPE_INDEX 0xFF7A347

typedef enum{
	// telechips vendor index Audio add.
	OMX_IndexVendorParamFileOpen = 0x7F000001,
	OMX_IndexVendorParamMediaInfo,
	OMX_IndexVendorConfigPowerSpectrum,
	OMX_IndexVendorConfigEnergyVolume,
	OMX_IndexVendorParamSetAudioPause,			
	
	// for video
	OMX_IndexVendorFBSinkScreenMode, // fbsink screen mode	
	OMX_IndexVendorFBSinkSclaeroffsetX,
	OMX_IndexVendorFBSinkSclaeroffsetY,
	OMX_IndexVendorFBSinkSetYUVOutMode,
	OMX_IndexVendorSetSTCDelay,
	OMX_IndexVendorSetAudioDelay,
	OMX_IndexVendorChangeResolution, // get VPU resolution and set fbsink comp. resolution.
	OMX_IndexVendorQueueVpuDisplayIndex,
	OMX_IndexVendorParamDxBGetVideoInfo,
	OMX_IndexVendorParamSetVideoPause,			
	OMX_IndexVendorParamIframeSearchEnable,
	OMX_IndexVendorParamVideoSurfaceState,
	OMX_IndexVendorParamSetVpuReset,
	OMX_IndexVendorParamVideoSupportFieldDecoding,
	OMX_IndexVendorParamVideoSupportIFrameSearch,
	OMX_IndexVendorParamVideoSupportUsingErrorMB,
	OMX_IndexVendorParamVideoSupportDirectDisplay,
	OMX_IndexVendorParamVideoIsSupportCountry,

	// for audio
	OMX_IndexVendorNotifyStartTimeSyncWithVideo,
	OMX_IndexVendorParamSetEnableAudioStartSyncWithVideo,
	OMX_IndexVendorParamSetPatternToCheckPTSnSTC,
	OMX_IndexVendorParamAudioIsSupportCountry,
	
	//for DXB
	OMX_IndexVendorParamDxBGetSTCFunction,
	OMX_IndexVendorParamDxBGetPVRFunction,
	OMX_IndexVendorParamDxBVideoParser,
	OMX_IndexVendorParamSetSinkByPass, 	//ignore PTS Time
    OMX_IndexVendorParamStopRequest,    //Stop AudioTrack
    OMX_IndexVendorParamSetSubtitle,

	//for DXB Tuner
	OMX_IndexVendorParamTunerDeviceSet,	
	OMX_IndexVendorParamTunerChannelSearch,
	OMX_IndexVendorParamTunerAnalogChannelSearch,
	OMX_IndexVendorParamTunerChannelScan,
	OMX_IndexVendorParamTunerFreqScan,
	OMX_IndexVendorParamTunerChannelSet,
	OMX_IndexVendorParamTunerGetChannelIndexByFrequency, //unit Khz
	OMX_IndexVendorParamTunerAnalogChannelSet,
	OMX_IndexVendorParamTunerCountryCodeSet,	
	OMX_IndexVendorParamTunerModulationSet,	
	OMX_IndexVendorParamTunerChannelSearchStart,
	OMX_IndexVendorParamTunerChannelSearchStop,
	OMX_IndexVendorParamTunerOpen,
	OMX_IndexVendorParamTunerClose,
	OMX_IndexVendorParamTunerFrequencySet,
	OMX_IndexVendorParamTunerPlayStop,
	OMX_IndexVendorConfigGetSignalStrength,
	OMX_IndexVendorConfigGetSignalStrengthIndex,
	OMX_IndexVendorConfigGetSignalStrengthIndexTime,
	OMX_IndexVendorConfigSetAntennaPower,
	OMX_IndexVendorParamSetGetPacketDataEntry,
	OMX_IndexVendorParamRegisterPID,
	OMX_IndexVendorParamUnRegisterPID,
	OMX_IndexVendorParamTunerSetNumberOfBB,
	OMX_IndexVendorParamTunerGetDataPLPs,
	OMX_IndexVendorParamTunerSetDataPLP,

	// for ISDBT Tuner
	OMX_IndexVendorParamTunerGetEWSFlag,
	OMX_IndexVendorParamTunerSetCasOpen,
	OMX_IndexVendorParamTunerSetCasSetmulti2,
	OMX_IndexVendorParamTunerSetCasSetPid,
	OMX_IndexVendorParamTunerSetFreqBand,
	OMX_IndexVendorParamTunerGetChannelValidity,

	// for T-DMB Tuner
	OMX_IndexVendorParamTunerChanneIDSet,
	OMX_IndexVendorParamTunerServiceIDSet,
	OMX_IndexVendorParamTunerEnsembleFreqSet,	
	OMX_IndexVendorParamTunerEnsembleBitRateSet,
	OMX_IndexVendorParamTunerEnsembleRegSet,
	OMX_IndexVendorParamSetResync,
	OMX_IndexVendorParamNETBERSet,		

	OMX_IndexVendorParamDemuxSysInit,
	OMX_IndexVendorParamDemuxSysdeInit,
	OMX_IndexVendorParamDemuxStopRequest,
	OMX_IndexVendorConfigGetEpg,
	
	OMX_IndexVendorParamSetDemuxAudioVideoStreamType,
	OMX_IndexVendorParamSetDemuxAudioVideoPID,	
	OMX_IndexVendorParamAlsaSinkSetVolume,
	OMX_IndexVendorParamAlsaSinkMute,	
	OMX_IndexVendorParamAlsaSinkMuteConfig,
	OMX_IndexVendorParamAlsaSinkAudioStartNotify,
	
	OMX_IndexVendorParamFBCapture,		// DxB_capture

	OMX_IndexVendorParamRecStartRequest,
	OMX_IndexVendorParamRecStopRequest,
	
	// for PVR
	OMX_IndexVendorParamPlayStartRequest,
	OMX_IndexVendorParamPlayStopRequest,
	OMX_IndexVendorParamSeekToRequest,
	OMX_IndexVendorParamPlaySpeed,
	OMX_IndexVendorParamPlayPause,
	OMX_IndexVendorParamDuration,
	OMX_IndexVendorParamCurrentPosition,
	OMX_IndexVendorParamReadPosition,
	OMX_IndexVendorParamWritePosition,
	OMX_IndexVendorParamTotalSize,
	OMX_IndexVendorParamMaxSize,
	OMX_IndexVendorParamFreeSize,
	OMX_IndexVendorParamBlockSize,
	OMX_IndexVendorParamFSType,
	OMX_IndexVendorParamSetDulaMode,
	
	OMX_IndexVendorParamAudioAacDualMono,
	
	// for ATSC
	OMX_IndexVendorParamAtscSearch, 
	OMX_IndexVendorParamAtscPlay, 
	OMX_IndexVendorParamAtscStop, 
	OMX_IndexVendorParamSetClosedCaption,
	OMX_IndexVendorParamSetAudioTrack, 
	OMX_IndexVendorParamSetAspectRatio, 

    //DxB Demuxer
    OMX_IndexVendorParamDxBGetNumOfDemux,
	OMX_IndexVendorParamDxBGetCapability,
	OMX_IndexVendorParamDxBSetPath,
	OMX_IndexVendorParamDxBGetPath,
	OMX_IndexVendorParamDxBReleasePath,
	OMX_IndexVendorParamDxBStartPID,
	OMX_IndexVendorParamDxBStopPID,
	OMX_IndexVendorParamDxBRegisterPESCallback,
	OMX_IndexVendorParamDxBStartPESFilter,
	OMX_IndexVendorParamDxBStopPESFilter,
	OMX_IndexVendorParamDxBRegisterSectionCallback,
	OMX_IndexVendorParamDxBSetSectionFilter,
	OMX_IndexVendorParamDxBReleaseSectionFilter,
	OMX_IndexVendorParamDxBGetSTC,
	OMX_IndexVendorParamDxBSetTSIFReadSize,
	OMX_IndexVendorParamDxBResetVideo,
	OMX_IndexVendorParamDxBResetAudio,
	OMX_IndexVendorParamDxBStartTSCMonitor, //TSC(Transport Scrambling Control bit)
	OMX_IndexVendorParamDxBStopTSCMonitor,
	OMX_IndexVendorParamDxBGetTSCMonitor,
	OMX_IndexVendorParamDxBSetBBType,
	OMX_IndexVendorParamDxBSetBitrate,	
	OMX_IndexVendorParamDxBRegisterCasDecryptCallback,
	OMX_IndexVendorParamDxBEnableAudioDescription,

	// for IPTV
	OMX_IndexVendorParamDxBStartPlay,
	OMX_IndexVendorParamDxBStartDemuxing,
	OMX_IndexVendorParamDxBActiveModeSet,
	OMX_IndexVendorParamDxBSendData,
	OMX_IndexVendorParamDxBFirstDisplaySet,

	// for CAS
	OMX_IndexVendorParamDxBCASOpen,
	OMX_IndexVendorParamDxBCASClose,
	OMX_IndexVendorParamDxBCASSetKey,
	OMX_IndexVendorParamDxBCASOpenSWMulti2,
	OMX_IndexVendorParamDxBCASCloseSWMulti2,
	OMX_IndexVendorParamDxBCASSetKeySWMulti2,
	OMX_IndexVendorParamDxBStartSectionFilterEx,
	OMX_IndexVendorParamDxBStopSectionFilterEx,
	OMX_IndexVendorParamDxBGetSectionHandleEx,

	OMX_IndexVendorParamDxBSetParentLock,
	OMX_IndexVendorParamAudioOutputCh,
	OMX_IndexVendorParamVideoDisplayOutputCh,

	OMX_IndexVendorParamTunerInformation,
	OMX_IndexVendorParamStereoMode,
	// for DVBSx
	OMX_IndexVendorParamSetVoltage,
	OMX_IndexVendorParamSetTone,
	OMX_IndexVendorParamDiSEqCSendBurst,
	OMX_IndexVendorParamDiSEqCSendCMD,
	OMX_IndexVendorParamDxbGetAudioType,	//isdb-t
	OMX_IndexVendorParamBlindScanReset,
	OMX_IndexVendorParamBlindScanStart,
	OMX_IndexVendorParamBlindScanCancel,
	OMX_IndexVendorParamBlindScanState,
	OMX_IndexVendorParamBlindScanInfo,

	OMX_IndexVendorParamISDBTProprietaryData,
	OMX_IndexVendorMax = 0x7FFFFFFF
}TC_OMX_INDEXVENDORTYPE;

typedef enum
{
	OMX_ErrorVendorFileOpenFailed = 0x90000001,
	OMX_ErrorVendorFileCloseFailed,
	OMX_ErrorVendorFileReadFailed,  
	OMX_ErrorVendorMemAllocFailed ,
	OMX_ErrorVendorParserInitFailed,
	OMX_ErrorVendorParserDeinitFailed,
	OMX_ErrorVendorDecInitFailed,
	OMX_ErrorVendorEncInitFailed,
	OMX_ErrorVendorUnsupportedFormat,
	OMX_ErrorVendorNoVideoData,
	OMX_ErrorVendorNoAudioData,
	OMX_ErrorVendorNoImageData,	
	OMX_ErrorVendorBufferOverflow,
	OMX_ErrorVendorExtDataFailed,
	OMX_ErrorVendorAudioPortParamSettingFailed,
	OMX_ErrorVendorVideoPortParamSettingFailed,	
	OMX_ErrorVendorAppPrivMemAllocFailed ,
	OMX_ErrorVendorComponentLoadingFailed ,
	OMX_ErrorVendorSetupTunnelingFailed ,
	OMX_ErrorVendorComponentExecutingFailed ,
	OMX_ErrorVendorGetOneFrameFailed ,
	OMX_ErrorVendorGetExtIndexFailed ,
	OMX_ErrorVendorGetExtDataFailed ,
	OMX_ErrorVendorSetExtDataFailed ,
	OMX_ErrorVendorPortEnableFailed ,
	OMX_ErrorVendorPortDisableFailed ,
	OMX_ErrorVendorPortFlushFailed	,
	OMX_ErrorVendorTimeRequestFailed,
	OMX_ErrorVendorSemaTimerExpire,
	OMX_ErrorVendorCodecOpenFailed,
	OMX_ErrorVendorCodecCloseFailed,
	OMX_ErrorVendorInvalidInfo,
	OMX_ErrorVendorInvalidFile,
	// for rec
	OMX_ErrorVendorRecFileOpenFail,
	OMX_ErrorVendorRecFileCloseFail,

	//for image
	OMX_ErrorVendorImageGetOrgSizeFailed,	
	OMX_ErrorVendorImageSetOutWidth,	
	OMX_ErrorVendorImageSetOutHeight,
	OMX_ErrorVendorImageSetThumbOffset,
	OMX_ErrorVendorImageSetOutBufferPointer,
	OMX_ErrorVendorImageSetOutFormat,	
	
	OMX_ErrorVendorMax = 0x9000FFFF
}TC_OMX_ERRORVENDORTYPE;

typedef enum{
	STEREO_OUTPUT_MODE = 0,
	MULTI_OUTPUT_MODE
}AUDIO_OUTPUT_TYPE;

typedef enum{
	ALSA_OPEN_SRC = 0,
	ALSA_OPEN_PCM_SRC,
	ALSA_OPEN_NOTSRC,
}ALSA_OPENMODE_TYPE;

typedef enum OMX_VENDOR_EVENTDATA
{
	OMX_VENDOR_EVENTDATA_BUFFER_CONTROL = 1000,
	OMX_VENDOR_EVENTDATA_SET_AV_CODEC_TYPE,
	OMX_VENDOR_EVENTDATA_SET_PMT_PID,
	OMX_VENDOR_EVENTDATA_TUNER_RESYNC,
	OMX_VENDOR_EVENTDATA_TUNER,
	OMX_VENDOR_EVENTDATA_DEMUX,
	OMX_VENDOR_EVENTDATA_RECORD,
} OMX_VEDOR_EVENT_DATA;


typedef struct
{
	unsigned int uiAudioCodecType;
	unsigned int uiVideoCodecType;	
}TCC_DEMUX_ES_STREAM_TYPE;

typedef enum
{
	ACTIVEMODE_STANDBY=0,
	ACTIVEMODE_PLAY,
	ACTIVEMODE_TRICK_PLAY,
	ACTIVEMODE_PAUSE,
	ACTIVEMODE_SEEK
}TCC_IPTV_ACTIVEMODE;


// extention index_name
#define TCC_AUDIO_FILE_OPEN_STRING "OMX.tcc.index.param.dec.inputfilename"
//#define TCC_AUDIO_FILE_CLOSE_INDEX "OMX.tcc.index.param.fileclose"
#define TCC_AUDIO_MEDIA_INFO_STRING "OMX.tcc.index.config.dec.mediainfo"
#define TCC_AUDIO_POWERSPECTUM_STRING "OMX.tcc.index.config.dec.powerspectrum"
#define TCC_AUDIO_ENERGYVOLUME_STRING "OMX.tcc.index.config.dec.energyvolume"
#define TCC_AUDIO_PARSER_MEDIA_STRING "OMX.tcc.index.param.parser.mediainfo"

#define TCC_AUDIO_DEQ_ONOFF_STRING "OMX.tcc.index.config.deq.onoff"
#define TCC_AUDIO_DEQ_MODE_STRING "OMX.tcc.index.config.deq.mode"


#define TCC_ENC_SET_BITRATE_STRING "OMX.TCC.index.param.enc.setbitrate"
#define TCC_ENC_SET_FILE_FORMAT_STRING "OMX.TCC.index.param.enc.setfileformat"
#define TCC_ENC_SETFILE_PATH_STRING "OMX.TCC.index.param.enc.setfilepath"
#define TCC_ENC_WRITE_HEADER_DATA_STRING "OMX.TCC.index.param.enc.writeheader"
#define TCC_ENC_END_STRING "OMX.TCC.index.param.enc.end"

#define TCC_BROADCAST_PID_STRING "OMX.tcc.index.config.tuner.setpid"
#define TCC_BROADCAST_SET_MODE_STRING "OMX.tcc.index.param.dec.setmode"
#define TCC_BROADCAST_SET_PMT_ID_STRING "OMX.tcc.index.param.dec.setpmtid"

#endif /* __TCC_OMX_INDEX_H__ */
