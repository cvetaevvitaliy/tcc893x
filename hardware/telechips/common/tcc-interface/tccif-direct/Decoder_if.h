/**
 * Copyright (C) 2012, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef DECODER_IF_H_
#define DECODER_IF_H_

extern "C"
{
#include "vdec.h"
}
#include "common_if.h"
#include <mach/tcc_video_private.h>

//using namespace android;

#define RESTORE_DECODE_ERR
#define CHECK_SEQHEADER_WITH_SYNCFRAME

#define TS_TIMESTAMP_CORRECTION
#define EXT_V_DECODER_TR_TEST

#define RESOLUTION_CHANGE_FUNC

#define NEW_BUFFER_MANAGEMENT
#ifdef NEW_BUFFER_MANAGEMENT
#define TMEM_DEVICE	"/dev/tmem"
#include <mach/tcc_mem_ioctl.h>
#endif

/***********************************************************/
//COMMON PARAMETERS
typedef enum CONTAINER_TYPE {
	CONTAINER_NONE = 0,
	CONTAINER_MKV,
	CONTAINER_MP4,
	CONTAINER_AVI,
	CONTAINER_MPG,
	CONTAINER_TS,
	CONTAINER_ASF,
	CONTAINER_RMFF,
	CONTAINER_FLV = 10
} tCONTAINER_TYPE;


/***********************************************************/
//INTERNAL PARAMETERS
#define MIN_NAL_STARTCODE_LEN 	    3
#define MAX_NAL_STARTCODE_LEN 	    4

#define MAX_CHECK_COUNT_FOR_CLEAR 100

typedef struct dec_disp_info_ctrl_t {
	int		m_iTimeStampType;	//! TS(Timestamp) type (0: Presentation TS(default), 1:Decode TS)
	int		m_iStdType;			//! STD type
	int		m_iFmtType;			//! Formater Type

	int		m_iUsedIdxPTS;		//! total number of decoded index for PTS
	int		m_iRegIdxPTS[32];	//! decoded index for PTS
	void	*m_pRegInfoPTS[32];	//! side information of the decoded index for PTS

	int		m_iDecodeIdxDTS;	//! stored DTS index of decoded frame
	int		m_iDispIdxDTS;		//! display DTS index of DTS array
	int		m_iDTS[32];			//! Decode Timestamp (decoding order)
	
	int		m_Reserved;
} dec_disp_info_ctrl_t;

typedef struct dec_disp_info_t {
	int m_iFrameType;			//! Frame Type

	int m_iTimeStamp;			//! Time Stamp
	int m_iextTimeStamp;			//! TR(RV)

	int m_iPicStructure;		//! PictureStructure
	int m_iM2vFieldSequence;	//! Field sequence(MPEG2) 
	int m_iFrameDuration;		//! MPEG2 Frame Duration
	
	int m_iFrameSize;			//! Frame size
} dec_disp_info_t;

typedef struct dec_disp_info_input_t {
	int m_iFrameIdx;			//! Display frame buffer index for CVDEC_DISP_INFO_UPDATE command
								//! Decoded frame buffer index for CVDEC_DISP_INFO_GET command
	int m_iStdType;				//! STD type for CVDEC_DISP_INFO_INIT
	int m_iTimeStampType;		//! TS(Timestamp) type (0: Presentation TS(default), 1:Decode TS) for CVDEC_DISP_INFO_INIT
	int m_iFmtType;				//! Formater Type specification
	int m_iFrameRate;
} dec_disp_info_input_t;

typedef struct mpeg2_pts_ctrl{
	int m_iLatestPTS;
	int m_iPTSInterval;
	int m_iRamainingDuration;
} mpeg2_pts_ctrl;

#ifdef TS_TIMESTAMP_CORRECTION
typedef struct ts_pts_ctrl{
	int m_iLatestPTS;
	int m_iPTSInterval;
	int m_iRamainingDuration;
} ts_pts_ctrl;
#endif

#ifdef EXT_V_DECODER_TR_TEST
typedef struct EXT_F_frame_t{
	int Current_TR;
	int Previous_TR;
	int Current_time_stamp;
	int Previous_time_stamp;
} EXT_F_frame_t;

typedef struct EXT_F_frame_time_t {
	EXT_F_frame_t  ref_frame;		
	EXT_F_frame_t  frame_P1;
	EXT_F_frame_t  frame_P2;
} EXT_F_frame_time_t;
#endif



//EXTERNAL PARAMETERS
typedef struct dec_init_params {
	tCODEC_FORMAT 		codecFormat;   	/* Decoder's input(compressed) format */
	tFRAME_BUF_FORMAT	frameFormat;    /* Decoder's output(de-compressed) format, only can support FRAME_BUF_FORMAT_YUV420P and FRAME_BUF_FORMAT_YUV420I.*/
	int 				picWidth;		/* Decoder's picture width, by pixels */
	int					picHeight;		/* Decoder's picture height, by pixels */
} tDEC_INIT_PARAMS;

typedef struct dec_frame_input {
	unsigned char	*inputStreamAddr;	/* Base address of input bitstream (virtual address) */
	int				inputStreamSize;	/* length of input bitstream, by Bytes */
	unsigned char	seek;				/* in case of seek */
	int				nTimeStamp;			/* TimeStamp of input bitstream, by ms */
} tDEC_FRAME_INPUT;


typedef struct dec_frame_output {
	unsigned int		bufPhyAddr[3];		/* (Y,U,V or Y, UV). Physical address of output frame. */
	unsigned int		bufVirtAddr[3];		/* (Y,U,V or Y, UV). Virtual address of output frame. */
	int					nTimeStamp;			/* TimeStamp of output bitstream, by ms */
	int					picWidth;			/* Picture width, by pixels */
	int					picHeight;			/* Picture height, by pixels */
	int					stride;
} tDEC_FRAME_OUTPUT;

typedef struct dec_result {
	int 	need_input_retry;	/* If this value is set, same input stream has to put again because decoder is failed with the reason related with internal error. */
	int 	no_frame_output;	/* If this value is set, ther is no frame to display because decoder fail or need more frame. */
	TCC_PLATFORM_PRIVATE_PMEM_INFO plat_priv;
} tDEC_RESULT;

#ifdef NEW_BUFFER_MANAGEMENT
typedef struct VBUFFER_MANAGEMENT{
//  unsigned int Display_out_addr; // a address for output buffer in component;
  bool Display_cleared;
  int Display_Buff_ID;
  int Display_index;
  int Keep_index; // in case of not-vsync and on-the-fly mode, it means current index displaying.
} VBUFFER_MANAGEMENT;
#endif

typedef struct _VDEC_INSTANCE_ {
	int avcodecReady;
	unsigned int video_coding_type;
	unsigned char	container_type;
	unsigned int  bitrate_mbps;
	vdec_input_t gsVDecInput;
	vdec_output_t gsVDecOutput;
	vdec_init_t gsVDecInit;
	vdec_user_info_t gsVDecUserInfo;
	int  isVPUClosed;
	unsigned int video_dec_idx;
	cdmx_info_t cdmx_info;
	dec_disp_info_ctrl_t dec_disp_info_ctrl;
	dec_disp_info_t dec_disp_info[32];
	dec_disp_info_input_t dec_disp_info_input;
	void* pVdec_Instance;
	ts_pts_ctrl gsTSPtsInfo;
	unsigned int 	restored_count;

	bool		 		bSequenceHeaderDone;	
	unsigned int 		mFrameSearchOrSkip_flag;
	bool		 		bFirst_Frame;
	signed char			mSkip_scheme_level;
	signed char  		mSkip_count;
	signed char  		mSkip_interval;
	bool		 		bUseFrameDefragmentation;
	unsigned char		mFps;

	mpeg2_pts_ctrl 		mMPEG2PtsInfo;
	unsigned int 		mOut_index;
	unsigned int 		mIn_index;
	unsigned int 		mFrm_clear;
#ifdef NEW_BUFFER_MANAGEMENT
	int					mProxy_fd;
	unsigned int 		mUsed_fifo_count;
	unsigned int		mBuffer_unique_id;
#endif
	VBUFFER_MANAGEMENT  mBuffVpu[VPU_BUFF_COUNT*2];
	unsigned int 		mMax_fifo_cnt;
	unsigned int 		mCodecStart_ms;

	signed char 		mSeq_header_init_error_count;
	unsigned int 		mConsecutiveVdecFailCnt;
	signed int			mConsecutiveBufferFullCnt;
	
#ifdef RESTORE_DECODE_ERR
	unsigned char* 		pSeqHeader_backup;
	unsigned int 		mSeqHeader_len;
	unsigned char 		mDecErrCount;
#endif

	int 		mCurrWidth;
	int		mCurrHeight;

#ifdef RESOLUTION_CHANGE_FUNC
	bool 				bDetected_res_changed;
#endif

#ifdef CHECK_SEQHEADER_WITH_SYNCFRAME
	unsigned char*		pSequence_header_only;
	unsigned int 		mSequence_header_size;
	bool		 		bNeed_sequence_header_attachment;
#endif

#ifdef TIMESTAMP_CORRECTION
	pts_ctrl gsPtsInfo;
#endif

#ifdef EXT_V_DECODER_TR_TEST
	int gsextTRDelta;
	int gsextP_frame_cnt;
	int gsextReference_Flag;
	EXT_F_frame_time_t gsEXT_F_frame_time;
#endif
} _VDEC_INSTANCE_;

class DecoderIf {
public:
/*!
 *************************************************************************************************
 * \brief
 *		DecoderIf/~DecoderIf	: Constructor / Destructor of Decoder Interface class
 *************************************************************************************************
 */
	DecoderIf();
    ~DecoderIf();

/*!
 *************************************************************************************************
 * \brief
 *		Init	: initial function of decoder
 * \param 0
 *		[in] pInit		: pointer of decoder initial parameters 
 * \return
 *		If successful, Init() returns 0 or plus. Otherwise, it returns a minus value.
 *************************************************************************************************
 */
	int Init(tDEC_INIT_PARAMS *pInit);

/*!
 *************************************************************************************************
 * \brief
 *		Decode	: decoder function of decoder
 * \param 0
 *		[in] pInput		: pointer of input parameters
 * \param 1
 *		[out] pOutput 	: pointer of output parameters
 * \param 2
 *		[out] pResult 	: pointer of result patameters
 * \return
 *		If successful, Decode() returns 0 or plus. Otherwise, it returns a minus value.
 *		If successful, check *pResult.
 *
 * \Caution!!
 *		You should put the completed one frame.
 *************************************************************************************************
 */
	int Decode(tDEC_FRAME_INPUT *pInput, tDEC_FRAME_OUTPUT *pOutput, tDEC_RESULT *pResult);

/*!
 *************************************************************************************************
 * \brief
 *		Close	: close function of decoder
 *************************************************************************************************
 */
	int Close(void);

private:
	void _disp_pic_info (int Opcode, void* pParam1, void *pParam2, void *pParam3, unsigned int fps);
	int _get_frame_type_for_frame_skipping(int iStdType, int iPicType, int iPicStructure);
	void _videoDecErrorProcess(int ret);
	int _extract_seqheader(const unsigned char *pbyStreamData, unsigned int lStreamDataSize,unsigned char **ppbySeqHeaderData, unsigned int *plSeqHeaderSize);
	char* _print_pic_type( int iVideoType, int iPicType, int iPictureStructure );

    int _init_buffer_id_for_kernel(void);
    int _init_stBuffer_Management(void);
    int _clear_all_displayedIndex(void);

#ifdef NEW_BUFFER_MANAGEMENT
	int _clear_vpu_buffer(int buffer_id);
	int _pull_displayedIndex(void);
    int _push_displayedIndex(int dispOutIdx );
#endif
	
/* Info */
	_VDEC_INSTANCE_ 	*pVDecInstance;

};

#endif

