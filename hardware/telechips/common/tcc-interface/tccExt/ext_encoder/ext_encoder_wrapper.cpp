
#define LOG_TAG "TCCEncoder"
#include <utils/Log.h>

#include <Encoder_if.h>

EncoderIf *mEncoder = NULL;

void _display_Stream(unsigned char *p, int size)
{
	int i;
	unsigned char* ps = p;
	for(i=0; (i+10 <size) && (i+10 < 100); i += 10){
		ALOGE( "[VENC - Stream] 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", ps[i], ps[i+1], ps[i+2], ps[i+3], ps[i+4], ps[i+5], ps[i+6], ps[i+7], ps[i+8], ps[i+9] );
	}
}	

int TCC_VENC_Init_H264(unsigned int *pInitParam)
{
	int ret = 0;
	tENC_INIT_PARAMS InitParam;

    mEncoder = new EncoderIf();
    if( !mEncoder )
    {
        ALOGE("EncoderIf creation is failed.");
        return -1;
    }

	InitParam.codecFormat		= CODEC_FORMAT_H264;
	InitParam.picWidth			= pInitParam[0];
	InitParam.picHeight			= pInitParam[1];
	InitParam.frameRate			= pInitParam[3];
	InitParam.targetKbps 		= pInitParam[2] >> 10;		
	InitParam.keyFrameInterval	= pInitParam[4];		
	InitParam.sliceMode			= ENC_SLICE_MODE_SINGLE;		
	InitParam.sliceSizeMode		= ENC_SLICE_SIZE_MODE_BY_BYTE; 		
	InitParam.sliceSize			= 0; 	
	InitParam.use_NalStartCode	= 1;

    if( (ret = mEncoder->Init(&InitParam)) < 0)
	{
		ALOGE( "[VENC_INIT] [Err:%d] video encoder init", ret );
	}
    
	return ret;
}

int TCC_VENC_Enc(unsigned int *pInputStream, unsigned int *pOutstream)																				
{
	int ret = 0;
	tENC_FRAME_INPUT InputStream;
	tENC_FRAME_OUTPUT OutputStream;
	unsigned int *InputCurrBuffer;

    if( !mEncoder )
    {
        ALOGE("EncoderIf is not initialized.");
        return -1;
    }

	InputStream.frameFormat         = FRAME_BUF_FORMAT_YUV420P;
    InputCurrBuffer                 = (unsigned int*)pInputStream[0];	/* Memory pointer that has the physical address of input raw data (memory pointer received from camera callback) */
	InputStream.inputStreamAddr     = (unsigned char*)InputCurrBuffer[0];
    InputStream.noIncludePhyAddr    = pInputStream[2];                  /* Whether inputStreamAddr is virtual address including raw data by itself.*/
	                                                                    /* if this set into 1, raw data will be copied into physical memory region to encode using H/W block. It will be decreased performance.*/
	InputStream.inputStreamSize     = pInputStream[1];	                /* Bytes : length of input data */
	InputStream.nTimeStamp          = 0;			                    /* TimeStamp of input data, by ms */	
	InputStream.isForceIFrame       = pInputStream[4]; 
	InputStream.isSkipFrame         = 0;;		                        /* Whether skip (do not encode) current frame */
	InputStream.frameRate           = pInputStream[5];			        /* to change FrameRate during encoding, It is ignored if zero  */
	InputStream.targetKbps          = pInputStream[3];                  /* to change Bitrate for output stream during encoding, by Kbps, It is ignored if zero */	
	InputStream.Qp =0;

	ret = mEncoder->Encode(&InputStream,&OutputStream);

	//Physical addr : OutputStream.outputStreamPhyAddr
	pOutstream[0] = (unsigned int)OutputStream.outputStreamVirtAddr; 	/* Base address for output bitstream (virtual address)*/
	pOutstream[1] = (unsigned int)OutputStream.outputStreamPhyAddr; 	/* Base address for output bitstream (physical address)*/
	pOutstream[2] = OutputStream.picType;								/* Picture coding type */
	pOutstream[3] = OutputStream.nTimeStamp;							/* TimeStamp of output bitstream, by ms */	
	pOutstream[4] = OutputStream.headerLen;								/* Bytes of header */
	pOutstream[5] = OutputStream.frameLen;								/* Bytes of frame encoded */
	pOutstream[6] = OutputStream.m_iSliceCount;							/* total slice's count that one encoded frame have */
	pOutstream[7] = (unsigned int)OutputStream.m_pSliceInfo;

#if 0
	if(OutputStream.headerLen)
		_display_Stream(OutputStream.outputStreamAddr,OutputStream.headerLen);
	else
		_display_Stream(OutputStream.outputStreamAddr,OutputStream.frameLen);
#endif

	if(ret < 0)
	{
		ALOGE( "[VENC_ENC] [Err:%d] video encoder dec", ret );
		return -1;
	}
	
	return ret;

}

int TCC_VENC_Close(void)
{
    if( !mEncoder )
    {
        ALOGE("EncoderIf already was closed.");
        return 0;
    }

	mEncoder->Close();
    delete mEncoder;
    mEncoder = NULL;

    return 0;
}

