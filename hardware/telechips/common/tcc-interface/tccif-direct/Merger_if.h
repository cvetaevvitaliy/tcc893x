/**
 * Copyright (C) 2012, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef MERGER_IF_H_
#define MERGER_IF_H_

#include <libpmap/pmap.h>
#include "common_if.h"
#include <mach/tcc_scaler_ioctrl.h>

//using namespace android;

#define MAX_BUFFER_COUNT 5

typedef struct buffer_info_t {
	unsigned int phy_addr;		
	unsigned int virt_addr;		
} buffer_info_t;

class MergerIf {
public:
/*!
 *************************************************************************************************
 * \brief
 *		MergerIf/~MergerIf	: Constructor / Destructor of Merger Interface class
 *************************************************************************************************
 */
	MergerIf();
    ~MergerIf();

/*!
 *************************************************************************************************
 * \brief
 *		Alloc_TargetBuffer	: initial function of video Merger
 * \param 0
 *		-	These are the information to allocate final target buffer that input images are merged in.
 *		-	Based on these parameters, target buffer can be allocated.
 *
 *		[in] width		: width of target buffer.
 *		[in] height		: height of target buffer.
 *		[in] format		: format of target buffer. 
 * \return
 *		If successful, Alloc_TargetBuffer() returns 0 or plus. Otherwise, it returns a minus value.
 *************************************************************************************************
 */
	int Alloc_TargetBuffer(int width, int height, tFRAME_BUF_FORMAT format);

/*!
 ***********************************************************************
 * \brief
 *		Merge	: Merge function of video Merger
 * \param
 *  => These are the information related to input frames from Camera and Decoders. 
 *		[in] in_width			: width of input frame
 *		[in] in_height			: height of input frame
 *		[in] in_format			: format of input frame
 *		[in] in_address			: physical address of input frame having raw data from Camera and Decoders.
 *
 *  => These are the information related to region in target buffer where input frames are merged in.
 *		[in] out_sx				: start x-position.  
 *		[in] out_sy				: start y-position. 
 *		[in] out_width			: width
 *		[in] out_height			: height
 *
 *		[in] new_buffer			: to replace the target buffer into new one.  
 * \return
 *		If successful, Merge() returns 0 or plus value. Otherwise, it returns a minus value.
 ***********************************************************************
 */

/* Ex> In case target resolution is 640x480.
       The value in brakets are start position of each region.
|***********************|
|(0,0)		|(320,0)	|
|			|			|
|	1st	    |	2nd		|
|***********************|
|(0,240)	|(320,240)	|
|			|			|
|	3rd		|	4th		|
*************************

 In above picture, each region of target is QVGA.
 
 1. if the QVGA frame with YUV420P want to be displayed in 2nd region, set like below
    Merge( 320, 240, FRAME_BUF_FORMAT_YUV420P, address, 320, 0, 320, 240, 0);
    
 2. if the VGA frame with YUV420I want to be displayed in 3rd region, set like below
 	Merge( 640, 480, FRAME_BUF_FORMAT_YUV420I, address, 0, 240, 320, 240, 0);
 	In case of this, input VGA frame will be scaled down into QVGA.
 	
 2. if the QCIF frame with YVU420P want to be displayed in 4th region, set like below
 	Merge( 176, 144, FRAME_BUF_FORMAT_YVU420P, address, 320, 240, 320, 240, 0);
 	In case of this, input QCIF frame will be scaled up into QVGA.
*/
	int Merge(int in_width, int in_height, tFRAME_BUF_FORMAT in_format, unsigned int in_address, 
                    int out_sx, int out_sy, int out_width, int out_height, int new_buffer);

/*!
 *************************************************************************************************
 * \brief
 *		Close	: close function of Merger
 *************************************************************************************************
 */
	int Close(void);

/*!
 *************************************************************************************************
 * \brief
 *		Get_BufferAddress	: to get buffer address (virtual or physical address)
 * \param
 *		[in] isPhyAddr			: Set this value to get physical address, otherwise virtual address will be returned.
 *		[in] previous_buffer	: Set this value to get previous buffer's address, otherwise current buffer's address will be returned.
 * \return
 *		memory address (virtual or physical address)
 *************************************************************************************************
Ex> In order to get physical address of current buffer.   => Get_BufferAddress(1, 0).
    In order to get virtual address of current buffer.    => Get_BufferAddress(0, 0).
	In order to get physical address of previous buffer.  => Get_BufferAddress(1, 1).
    In order to get virtual address of previous buffer.   => Get_BufferAddress(0, 1).
 */
	unsigned int Get_BufferAddress(bool isPhyAddr, bool previous_buffer);

private:
	void _mem_prepare();
	void _mem_destory();
	int _merge_frame(int in_width, int in_height, tFRAME_BUF_FORMAT in_format, unsigned int in_address, 
                        int out_sx, int out_sy, int out_width, int out_height);

//	mutable Mutex mLock;
	
//info	
	pmap_t 				mMergerMap;
	int 				mFd;
	void* 				mMap_addr;

	unsigned int 		mBuffer_width;
	unsigned int		mBuffer_height;
	tFRAME_BUF_FORMAT	mBuffer_format;
	buffer_info_t		mBufferInfo[MAX_BUFFER_COUNT];
	unsigned char		mBuffer_curr_index;
	unsigned char		mBuffer_total_count;

	bool				bAllocated;
	int 				mFd_Merge;
	char				*pDev_Merge;
	SCALER_TYPE 		mScaleInfo;

};

#endif

