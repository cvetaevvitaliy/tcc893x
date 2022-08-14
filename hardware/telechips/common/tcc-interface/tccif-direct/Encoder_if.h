/**
 * Copyright (C) 2012, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef ENCODER_IF_H_
#define ENCODER_IF_H_

extern "C"
{
#include "venc.h"
}
#include <libpmap/pmap.h>
#include "common_if.h"
#include <EncoderMetadata.h>

//using namespace android;

/***********************************************************/
//INTERNAL PARAMETERS
typedef enum enc_slice_mode {
	ENC_SLICE_MODE_SINGLE = 0,
	ENC_SLICE_MODE_MULTY
} tENC_SLICE_MODE;

typedef enum enc_slice_size_mode {
	ENC_SLICE_SIZE_MODE_BY_BYTE = 0,		/* bytes : length per one slice */
	ENC_SLICE_SIZE_MODE_BY_MB				/* Macro block counts per one slice. one Macro block is 16x16.*/
} tENC_SLICE_SIZE_MODE;

/*
SliceMode :

In case sliceSizeModeSize is 0 (ENC_SLICE_SIZE_MODE_BY_BYTE).
	If this vaue is set 4KB, one slice length has about 4KB.
	But, slice count is changed per encoded frame.

 
In case sliceSizeModeSize is 0 (ENC_SLICE_SIZE_MODE_BY_MB).
	640x480 resolution has 1200 MBs. (640/16 * 480/16)
	If this value is set 400, one encoded frame has 3 slices. 
	But, Each slice has different length.
*/

typedef struct enc_init_params {
	tCODEC_FORMAT 			codecFormat;   		/* Encoder's output(compressed) format */
	int						picWidth;			/* Frame width, by pixels */
	int						picHeight;			/* Frame height, by pixels */
	int						frameRate;			/* FrameRate, It is used to calculate exact bitrate */
	int						targetKbps;			/* Bitrate for output stream, by Kbps, if zero, no bitrate control used */
	int						keyFrameInterval;	/* Key frame(I-Frame) interval */
	tENC_SLICE_MODE			sliceMode;			/* Slice mode, only can be used in case of AVC(H.264) */
	tENC_SLICE_SIZE_MODE	sliceSizeMode;		/* Slice size mode */
	int 					sliceSize;			/* Slice size */
	int 					use_NalStartCode;	/* Whether NAL Start Code (0x00000001) use or not, only can be used in case of AVC(H.264)  */
} tENC_INIT_PARAMS;


typedef struct enc_frame_input {
	tFRAME_BUF_FORMAT frameFormat;    	/* Encoder's input frame format, only can support FRAME_BUF_FORMAT_YUV420P and FRAME_BUF_FORMAT_YUV420I. */
	unsigned char	*inputStreamAddr;	/* The memory address that has input raw data. */
										/* Normally, This is physical address. for example, Refer memory pointer received from camera callback */
										/* Specially, This can be virtual address. but, for it noIncludePhyAddr variable has to be set. */
	unsigned int	noIncludePhyAddr;   /* Whether inputStreamAddr is virtual address including raw data by itself.*/
	                                    /* if this set into 1, raw data will be copied into physical memory region to encode using H/W block. It might be decreased performance.*/
	unsigned int	inputStreamSize;	/* Bytes : length of input data */
	int				nTimeStamp;			/* TimeStamp of input data, by ms */	
	int				isForceIFrame;		/* Whether force IDC frame is needed */
	int				isSkipFrame;		/* Whether skip (do not encode) current frame */
	int				frameRate;			/* to change FrameRate during encoding, It will be ignored if zero  */
	int				targetKbps;			/* to change Bitrate for output stream during encoding, by Kbps, It will be ignored if zero */	
	int				Qp;					/* TBD, Qp for current frame */
} tENC_FRAME_INPUT;


/*
 Generally in case of AVC(H.264), the structure of encoded frame is sa below:
	====================================================
	| SPS | PPS | I-Frame | P-Frame | P-Frame | P-Frame | ...
	====================================================

 But, If you use multi-slice mode, the structure will be as below:
    - AU(Access Unit) data is { 0x00,0x00,0x00,0x01,0x09,0x50,0x00,0x00 }. 
       You can confirm starting point of frame by AU.
 
 * Whole structure
	====================================================
	| AU | SPS | PPS | I-Frame | AU | P-Frame | AU | P-Frame | ...
	====================================================

 * I/P-Frame structure
	===========================================
	| AU | 1st slice | 2nd slice | 3rd slice ..... | n'th slice |
	===========================================
*/

typedef struct enc_frame_output {
	unsigned char	*outputStreamVirtAddr;	/* Base address for output bitstream (virtual address)*/
	unsigned char	*outputStreamPhyAddr;	/* Base address for output bitstream (physical address)*/
	tPIC_TYPE		picType;			/* Picture coding type : I-Frame(0), P-Frame(1)*/
	int				nTimeStamp;			/* TimeStamp of output bitstream, by ms */		
	int				headerLen;			/* Bytes of header */
	int				frameLen;			/* Bytes of frame encoded */
	int 			m_iSliceCount;      /* total slice's count that one encoded frame have */
	unsigned int* 	m_pSliceInfo;		/* bytes : This is the array pointer that has length of each slice. */
} tENC_FRAME_OUTPUT;


class EncoderIf {
public:
/*!
 *************************************************************************************************
 * \brief
 *		EncoderIf/~EncoderIf	: Constructor / Destructor of Encoder
 *************************************************************************************************
 */
	EncoderIf();
    ~EncoderIf();

/*!
 *************************************************************************************************
 * \brief
 *		Init	: initial function of encoder
 * \param 0
 *		[in] pInit		: pointer of encoder initial parameters 
 * \return
 *		If successful, Init() returns 0 or plus. Otherwise, it returns a minus value.
 *************************************************************************************************
 */
	int Init(tENC_INIT_PARAMS *pInit);

/*!
 ***********************************************************************
 * \brief
 *		Encode	: encode function of video encoder
 * \param
 *		[in] pInput			: pointer of encoder frame input parameters  
 * \param
 *		[out] pOutput		: pointer of encoder frame output parameters  
 * \return
 *		If successful, Encode() returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
	int Encode(tENC_FRAME_INPUT *pInput, tENC_FRAME_OUTPUT *pOutput);

/*!
 *************************************************************************************************
 * \brief
 *		Close	: close function of encoder
 *************************************************************************************************
 */
	int Close(void);

private:
	void _mem_prepare();
	void _mem_destory();
	int _mem_get_addr(unsigned int *out_virtaddr, unsigned int in_phyaddr);
#ifdef CHANGE_QP_FOR_IFRAME
	int _getInitQpND(int bitrate, int width, int height);
#endif
	int _enc_init();

//enc operation
	venc_init_t			mVEncInit;
	venc_seq_header_t	mVEncSeqHeader;
	venc_input_t		mVEncInput;
	venc_output_t		mVEncOutput;
	bool		  		bVPUClosed;

//info
	bool		 		bConfigDataFlag;
	unsigned char 		mVideo_coding_type;
	int					mCurr_frameRate;
	int					mCurr_targetKbps;	
	bool				bUse_NalStartCode;
	int					mQp_value;
	unsigned int 		mTotal_count;
	unsigned int 		mRestarted_count;
	
//error process
	bool				bEncError;
	pmap_t 				mVideomap;
	int 				mFd;
	void* 				mMapInfo;
};

#endif

