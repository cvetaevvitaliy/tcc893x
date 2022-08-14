#define	APETAGINFO_BYTE_SIZE			64
typedef enum _tAPEParserStatus
{
	cAPEParser_NoErr = 0,
	cAPEParser_Failed,
	cAPEParser_BadHeader,
	cAPEParser_BadVersionNumber,
	cAPEParser_OverSPEC,
	cAPEParser_BadArgument,
	cAPEParser_BrokenFrame,
	cAPEParser_NoMoreFrames,
	cAPEParser_BadSamplingRate,
	cAPEParser_BadNumberOfChannels,
	cAPEParser_EndStatus = 0x7FFFFFFF
} tAPEParserStatus;

typedef struct ape_taginfo_t
{
	char      m_cArtist[APETAGINFO_BYTE_SIZE+1];
	char      m_cAlbum[APETAGINFO_BYTE_SIZE+1];
	char      m_cTitle[APETAGINFO_BYTE_SIZE+1];
	char      m_cTrack[APETAGINFO_BYTE_SIZE+1];
	char      m_cGenre[APETAGINFO_BYTE_SIZE+1];
	char      m_cYear[APETAGINFO_BYTE_SIZE+1];
	char      m_cComment[APETAGINFO_BYTE_SIZE+1];
} ape_taginfo_t;

typedef struct ape_header_t
{
	unsigned int m_uiJunkbytes;
	unsigned int m_uiFirstframe;				//!< Location of first frame in ape file.
	unsigned int m_uiTotalsamples;
	char         m_cID[4];						//!< Should equal 'MAC '
	short        m_iVersion;					//!< Version number * 1000 (3.81 = 3810)
	short        m_iPadding;					//!< Used for align
	unsigned int m_uiDescriptorlength;			//!< The number of descriptor bytes (allows later expansion of this header)
	unsigned int m_uiHeaderlength;				//!< The number of header APE_HEADER bytes
	unsigned int m_uiSeekTableBytes;			//!< The number of bytes of the seek table
	unsigned int m_uiHeaderBytes;				//!< The number of header data bytes (from original file)
	unsigned int m_uiAudiodatalength;			//!< The number of bytes of APE frame data
	unsigned int m_uiAudiodatalength_high;		//!< The high order number of APE frame data bytes    
	unsigned int m_uiTerminatingBytes;			//!< The terminating data of the file (not including tag data)
	char      	 m_cFileMD5[16];				//!< The MD5 hash of the file (see notes for usage... it's a littly tricky)

	unsigned short m_uiCompressionType;			//!< The compression level (see defines I.E. COMPRESSION_LEVEL_FAST)
	unsigned short m_uiFormatFlags;				//!< Any format flags (for future use)
	unsigned int m_uiBlocksPerFrame;			//!< The number of audio blocks in one frame
	unsigned int m_uiFinalFrameBlocks;			//!< The number of audio blocks in the final frame
	unsigned int m_uiTotalFrames;				//!< The total number of frames
	unsigned short m_uiBitsPerSample;			//!< The bits per sample (typically 16)
	unsigned short m_uiChannels;				//!< The number of channels (1 or 2)
	unsigned int m_uiSamplerate;				//!< The sample rate (typically 44100)

	int      *m_pSeekTable;						//!< Pointer of seektable buffer 
	int       m_iMaxSeekPoints;					//!< Max seekpoints we can store (size of seektable buffer) 
	int       m_iNumSeekPoints;					//!< Number of seekpoints 
	int       m_iSeekTableFilePos;				//!< Location in .ape file of seektable

} ape_header_t;

typedef struct ape_dmx_exinfo_t
{
	short     m_iVersion;						//!< Version number * 1000 (3.81 = 3810)
	short     m_iPadding;						//!< Used for align
	unsigned short m_uiCompressionLevel;		//!< The compression level (see defines I.E. COMPRESSION_LEVEL_FAST)
	unsigned short m_uiFormatFlags;				//!< Any format flags (for future use)
	unsigned short m_uiBitsPerSample;			//!< The bits per sample (typically 16)
	unsigned short m_uiTemp_align;				//!< Not used
	unsigned int m_uiFinalFrameBlocks;			//!< The number of audio blocks in the final frame				
	unsigned int m_uiBlocksPerFrame;			//!< The number of audio blocks in one frame
	unsigned int m_uiTotalFrames;				//!< The total number of frames
} ape_dmx_exinfo_t;

typedef struct ape_dmx_exinput_t
{
	unsigned int		m_uiInputBufferByteOffset;			//!< Offset value in InputBuffer
	unsigned int		m_uiCurrentFrames;					//!< Indicates the current frame
} ape_dmx_exinput_t;

typedef struct ape_dmx_info_t
{
	void							*m_pInfile;
	audio_callback_func_t			*m_pCallbackFunc;		//!< System callback function pointer
	unsigned char					*m_pucStreamBuffer;		//!<
	unsigned int					m_uiBitrate;			//!< Bitrates	
	unsigned int					m_uiFileSize;			//!< Filesize
	unsigned int					m_uiTotalPlayTime;		//!< Total play time 
	unsigned int					m_uiSamplerate;			//!< Sampling rates
	unsigned int					m_uiChannels;			//!< Number of channels

	ape_dmx_exinfo_t				m_st_exinfo;			//!< Information for ape decoder decoding
	ape_dmx_exinput_t				m_st_exinput;			//!< After SEEK, filled this structure
	ape_taginfo_t					m_st_taginfo;			//!< Ape tag information

	unsigned int					m_ulSeekOffset;			//!< Time of one seek point 
	int								*m_pSeekTable;			//!< Seektable buffer
	int								m_iNumSeekPoints;		//!< Number of seekpoints 
	int								m_iSeekTableFilePos;	//!< Location in .ape file of seektable 
	unsigned int					m_uiSeekFlag;		    //!< 1 - after seek, 0 - nomal play
	unsigned int					m_uiFirstframe;         //!< Location of first frame in ape file.

	unsigned int					m_uiTimePos;		     //!< SampleCount
	unsigned int					m_uiTime;				 //!< Time (miliseconds)
}ape_dmx_info_t;

