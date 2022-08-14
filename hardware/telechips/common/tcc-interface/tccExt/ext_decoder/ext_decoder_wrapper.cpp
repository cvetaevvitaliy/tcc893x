
#define LOG_TAG "TCCDecoder"
#include <utils/Log.h>

#include <Decoder_if.h>

DecoderIf *mDecoder = NULL;

static void _display_Stream(unsigned char *p, int size)
{
	int i;
	unsigned char* ps = p;
	for(i=0; (i+10 <size) && (i+10 < 100); i += 10){
		ALOGE( "[VDEC - InPut Stream] 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", ps[i], ps[i+1], ps[i+2], ps[i+3], ps[i+4], ps[i+5], ps[i+6], ps[i+7], ps[i+8], ps[i+9] );
	}
}	

int TCC_VDEC_Init_H264(int Width, int Height)
{
	int ret = 0;
	tDEC_INIT_PARAMS pInit;

    mDecoder = new DecoderIf();
    if( !mDecoder )
    {
        ALOGE("DecoderIf creation is failed.");
        return -1;
    }
    
	pInit.codecFormat = CODEC_FORMAT_H264;
	pInit.frameFormat = FRAME_BUF_FORMAT_YUV420I;
	pInit.picWidth = Width;
	pInit.picHeight = Height;
	ALOGE("%s %d!!",__func__,__LINE__);

	if( (ret = mDecoder->Init(&pInit)) < 0)
	{
		ALOGE( "[VDEC_INIT] [Err:%d] video decoder init", ret );
	}
    
	return ret;
}

int TCC_VDEC_Dec(unsigned int *pInputStream, unsigned int *pOutstream)																				
{
	int ret = 0;
	int i = 0;
	
	tDEC_FRAME_INPUT Input;
	tDEC_FRAME_OUTPUT Output;
	tDEC_RESULT Result;

	Input.inputStreamAddr = (unsigned char*)pInputStream[0];
	Input.inputStreamSize = pInputStream[1];
	Input.nTimeStamp = 0;	
	Input.seek = 0;

	//_display_Stream(Input.inputStreamAddr,Input.inputStreamSize);
    if( !mDecoder )
    {
        ALOGE("DecoderIf is not initialized.");
        return -1;
    }
	
	ret = mDecoder->Decode(&Input, &Output, &Result);
	if(ret < 0)
	{
		ALOGE( "[VDEC_DEC] [Err:%d] video decoder dec", ret );
		return ret;
	}
    
	if(Result.no_frame_output)
	{
		ALOGE( "[VDEC_DEC]  No_frame_output");
		return -1;
	}
	else
	{
		pOutstream[0] = FRAME_BUF_FORMAT_YUV420I;
		pOutstream[1] = Output.bufPhyAddr[0];
		pOutstream[2] = Output.bufPhyAddr[1];
		pOutstream[3] = Output.bufPhyAddr[2];
		pOutstream[4] = Output.bufVirtAddr[0];
		pOutstream[5] = Output.bufVirtAddr[1];
		pOutstream[6] = Output.bufVirtAddr[2];
		pOutstream[7] = Output.nTimeStamp; /* TimeStamp of output bitstream, by ms */
		pOutstream[8] = Output.picWidth; /* Picture width, by pixels */
		pOutstream[9] = Output.picHeight; /* Picture height, by pixels */
		pOutstream[10] = Output.stride;
	}

	return ret;
}

void TCC_VDEC_Close(void)
{
    if( !mDecoder )
    {
        ALOGE("DecoderIf already was closed.");
        return;
    }

	mDecoder->Close();
    delete mDecoder;
    mDecoder = NULL;
}

