
/* wave parser */

#ifndef _TCC_wav_DMX_H_
#define _TCC_wav_DMX_H_

#ifndef NULL
#define NULL 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif
#ifndef SEEK_END
#define SEEK_END    2
#endif
#ifndef SEEK_SET
#define SEEK_SET    0
#endif

//=============================================================================================

/***************************************************************************/
#define  TC_WAVE_FORMAT_UNKNOWN				0x0000
#define  TC_WAVE_FORMAT_PCM					0x0001
#define  TC_WAVE_FORMAT_EXTENSIBLE			0xFFFE
#define  TC_WAVE_FORMAT_MSADPCM				0x0002
#define  TC_WAVE_FORMAT_IEEE_FLOAT				0x0003
#define  TC_WAVE_FORMAT_IBM_CVSD				0x0005
#define  TC_WAVE_FORMAT_ALAW					0x0006
#define  TC_WAVE_FORMAT_MULAW					0x0007
#define  TC_WAVE_FORMAT_OKI_ADPCM				0x0010
#define  TC_WAVE_FORMAT_DVI_ADPCM				0x0011
#define  TC_WAVE_FORMAT_IMA_ADPCM  (TC_WAVE_FORMAT_DVI_ADPCM)
#define  TC_WAVE_FORMAT_MEDIASPACE_ADPCM		0x0012
#define  TC_WAVE_FORMAT_SIERRA_ADPCM			0x0013
#define  TC_WAVE_FORMAT_G723_ADPCM				0x0014
#define  TC_WAVE_FORMAT_DIGISTD				0x0015
#define  TC_WAVE_FORMAT_DIGIFIX				0x0016
#define  TC_WAVE_FORMAT_DIALOGIC_OKI_ADPCM		0x0017
#define  TC_WAVE_FORMAT_MEDIAVISION_ADPCM		0x0018
#define  TC_WAVE_FORMAT_YAMAHA_ADPCM			0x0020
#define  TC_WAVE_FORMAT_SONARC					0x0021
#define  TC_WAVE_FORMAT_DSPGROUP_TRUESPEECH	0x0022
#define  TC_WAVE_FORMAT_ECHOSC1				0x0023
#define  TC_WAVE_FORMAT_AUDIOFILE_AF36			0x0024
#define  TC_WAVE_FORMAT_APTX					0x0025
#define  TC_WAVE_FORMAT_AUDIOFILE_AF10			0x0026
#define  TC_WAVE_FORMAT_DOLBY_AC2				0x0030
#define  TC_WAVE_FORMAT_GSM610					0x0031
#define  TC_WAVE_FORMAT_MSNAUDIO				0x0032
#define  TC_WAVE_FORMAT_ANTEX_ADPCME			0x0033
#define  TC_WAVE_FORMAT_CONTROL_RES_VQLPC		0x0034
#define  TC_WAVE_FORMAT_DIGIREAL				0x0035
#define  TC_WAVE_FORMAT_DIGIADPCM				0x0036
#define  TC_WAVE_FORMAT_CONTROL_RES_CR10		0x0037
#define  TC_WAVE_FORMAT_NMS_VBXADPCM			0x0038
#define  TC_WAVE_FORMAT_CS_IMAADPCM			0x0039
#define  TC_WAVE_FORMAT_ECHOSC3				0x003A
#define  TC_WAVE_FORMAT_ROCKWELL_ADPCM			0x003B
#define  TC_WAVE_FORMAT_ROCKWELL_DIGITALK		0x003C
#define  TC_WAVE_FORMAT_XEBEC					0x003D
#define  TC_WAVE_FORMAT_G721_ADPCM				0x0040
#define  TC_WAVE_FORMAT_G728_CELP				0x0041
#define  TC_WAVE_FORMAT_MPEG					0x0050
#define  TC_WAVE_FORMAT_MPEGLAYER3				0x0055
#define  TC_WAVE_FORMAT_CIRRUS					0x0060
#define  TC_WAVE_FORMAT_ESPCM					0x0061
#define  TC_WAVE_FORMAT_VOXWARE				0x0062
#define  TC_WAVE_FORMAT_CANOPUS_ATRAC			0x0063
#define  TC_WAVE_FORMAT_G726_ADPCM				0x0064
#define  TC_WAVE_FORMAT_G722_ADPCM				0x0065
#define  TC_WAVE_FORMAT_DSAT					0x0066
#define  TC_WAVE_FORMAT_DSAT_DISPLAY			0x0067
#define  TC_WAVE_FORMAT_SOFTSOUND				0x0080
#define  TC_WAVE_FORMAT_RHETOREX_ADPCM			0x0100
#define  TC_WAVE_FORMAT_CREATIVE_ADPCM			0x0200
#define  TC_WAVE_FORMAT_CREATIVE_FASTSPEECH8   0x0202
#define  TC_WAVE_FORMAT_CREATIVE_FASTSPEECH10  0x0203
#define  TC_WAVE_FORMAT_QUARTERDECK			0x0220
#define  TC_WAVE_FORMAT_FM_TOWNS_SND			0x0300
#define  TC_WAVE_FORMAT_BTV_DIGITAL			0x0400
#define  TC_WAVE_FORMAT_OLIGSM					0x1000
#define  TC_WAVE_FORMAT_OLIADPCM				0x1001
#define  TC_WAVE_FORMAT_OLICELP				0x1002
#define  TC_WAVE_FORMAT_OLISBC					0x1003
#define  TC_WAVE_FORMAT_OLIOPR					0x1004
#define  TC_WAVE_FORMAT_LH_CODEC				0x1100
#define  TC_WAVE_FORMAT_NORRIS					0x1400


#define  WAVE_ERR_NONE				0
#define  WAVE_ERR_END_OF_FILE		-1
#define  WAVE_ERR_INVALID_DATA	-3
#define  WAVE_ERR_UNSUPPORTED       -4
#define  WAVE_ERR_BROKEN_CHUNK     -5



#ifndef ABS
#define ABS(a) (((a)>0)?(a):-(a))
#endif

typedef struct tWAVFOURCCID
{
    char	Data1;
    char	Data2;
    char	Data3;
    char	Data4;
} wav_fourccid_t; 

//****************************************************************************
//
// This structure contains the definition of the wave format structure as
// stored in the "fmt " chunk of the WAVE file.  
//
//****************************************************************************
#define BASICWAVEFORMATSIZE 14

typedef struct WAVEFORMATEX_tag
{
    unsigned short	wFormatTag;
    unsigned short	nChannels;
    unsigned long	nSamplesPerSec;
    unsigned long	nAvgBytesPerSec;
    unsigned short	nBlockAlign;
    unsigned short	wBitsPerSample;
    unsigned short	cbSize;
} waveformatex_t;


#define	WAVE_FORCC_INFO_SIZE			4
#define	WAVE_CHUNK_SIZE_BYTE			4
#define	WAVE_CHUNK_FORCC_SIZE_BYTE		8
#define	WAVE_FIRST_CHUNK_OFFSET			12

#define	INFO_BYTE_SIZE	32

#define WAVE_DEFINE_GUID(name, b1, b2, b3, b4) \
		const	wav_fourccid_t name; \
		const	wav_fourccid_t name = {b1,b2,b3,b4}


#define WAVE_EqualFOURCC(rguid1, rguid2) (!memcmp((void *)rguid1, (void *)rguid2, sizeof(wav_fourccid_t)))

/***************************************************************************/
WAVE_DEFINE_GUID(FOURCC_FMT, 0x66,0x6d,0x74,0x20);
WAVE_DEFINE_GUID(FOURCC_DATA, 0x64,0x61,0x74,0x61);
WAVE_DEFINE_GUID(FOURCC_LIST, 0x4c,0x49,0x53,0x54);
WAVE_DEFINE_GUID(FOURCC_INFO, 0x49,0x4e,0x46,0x4f);
WAVE_DEFINE_GUID(FOURCC_IART, 0x49,0x41,0x52,0x54);


WAVE_DEFINE_GUID(FOURCC_INAM, 0x49,0x4e,0x41,0x4d);
WAVE_DEFINE_GUID(FOURCC_IPRD, 0x49,0x50,0x52,0x44);
WAVE_DEFINE_GUID(FOURCC_IGNR, 0x49,0x47,0x4e,0x52);


typedef struct
{
    char Artist[INFO_BYTE_SIZE];
    char Name[INFO_BYTE_SIZE];
    char Product[INFO_BYTE_SIZE];
    char Genre[INFO_BYTE_SIZE];
} wav_metadata_t;


typedef struct wav_file_info_t
{
    unsigned long usOffset;		//!< The current offset into the encoded data buffer.
    unsigned long usValid;		//!< The number of valid bytes in the encoded data buffer.
    unsigned long ulDataStart;	//!< The offset into the file of the first byte of encoded data.
    unsigned long ulLength;	    //!< The byte-length of the encoded data.
} wav_file_info_t;



typedef struct wav_dmx_info_t
{
	void					*m_pInfile;
	audio_callback_func_t	*m_pCallbackFunc;	//!< System callback function
	unsigned char			*m_pucStreamBuffer;
	unsigned int			m_uiFileSize;
	unsigned int			m_uiTotalPlayTime;    
	unsigned long 			ulOffset;			//!< The current offset into the encoded data buffer.

	unsigned int			m_uiSamplerate;		//!< The sample rate of the decoded file.
	unsigned int			m_uiBitRate;		//!< The bit rate of audio in the file.
	unsigned int			m_uiChannels;		//!< The number of channels of audio in the file.
	unsigned int            m_uiFrameSize;		//!< The number of demuxer output length(= decoder input length)
	unsigned int            m_uiSamplePerFrame; 
	unsigned int			m_uiTimePos;
	unsigned int			m_uiTime;
	unsigned int			m_uiRemainStreamByte;   //!<The length of remain audio data.

	wav_metadata_t			m_sMetadata;
	wav_file_info_t			m_sFileInfo;
	waveformatex_t			sWaveFormat;	
	long					m_lExtraLength;		//!< The length of extra audio info.
	unsigned char			*m_pExtraData;		//!< The buffer pointer of extra audio info.
}wav_dmx_info_t;


#define READBUF_SIZE		(4096)	/* feel free to change this, but keep big enough for >= one frame at high bitrates */
#define MAX_ARM_FRAMES		100
#define ARMULATE_MUL_FACT	1
#define MAINBUF_SIZE	1940


int TCC_WAV_DMX( int Op, int* pHandle, void* pParam1, void* pParam2 );

#endif	/* _TCC_wav_DMX_H_ */
