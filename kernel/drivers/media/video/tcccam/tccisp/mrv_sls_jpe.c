/****************************************************************************
One line to give the program's name and a brief idea of what it does.
Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/


#include "stdinc.h"

// #if MRV_SUPPORT_SL
#ifdef SL_SUPPORT_PARTLY

#include "mrv_priv.h"


/*****************************************************************************
 * DEFINES
 *****************************************************************************/
// abbreviations for local debug control ( level | module )
#define DERR  ( DBG_ERR  | DBG_MRV )    //!< errors
#define DWARN ( DBG_WARN | DBG_MRV )    //!< warnings
#define DINFO ( DBG_INFO | DBG_MRV )    //!< information



/*****************************************************************************/
/*!
 *  \FUNCTION    MrvSls_JpeCapture \n
 *  \RETURNVALUE amount of data written to jpeg buffer \n
 *  \PARAMETERS  enum for immediate: Marvin starts right away with the new 
 *                            configuration. This is useful if the image sensor
 *                            delivers only a limited number of frames, and
 *                            ons can't rely on the config update at the last
 *                            pixel of the previous frame. \n
 *               frame synchronous: 'traditional' behaviour, the changed
 *                            configuration becomes valid at the last pixel of
 *                            the just processed frame. Thus, the next complete
 *                            frame will be the first to be captured. \n
 *               or later (external) update \n
 *  \NOTES       Application must setup sensor and complete path before
 *               calling MrvJpeCapture. It is assumed that Marvin is in
 *               idle state. \n
 *
 *  \DESCRIPTION capture one frame \n
 */
/*****************************************************************************/
UINT32 MrvSls_JpeCapture(teMrvConfUpdateTime eUpdateTime)
{
    RESULT RetVal = RET_SUCCESS;
    
/*
    DBG_OUT( (DINFO, "MrvSlsJpeCapture:              Register ISP_RIS        = 0x%08X\n", ptMrvReg->isp_ris) );
    DBG_OUT( (DINFO, "MrvSlsJpeCapture:              Register MI_RIS         = 0x%08X\n", ptMrvReg->mi_ris) );
    DBG_OUT( (DINFO, "MrvSlsJpeCapture:              Register JPE_DEBUG      = 0x%08X\n", ptMrvReg->jpe_debug) );
    DBG_OUT( (DINFO, "MrvSlsJpeCapture:              Register JPE_ERROR_RIS  = 0x%08X\n", ptMrvReg->jpe_error_ris) );
    DBG_OUT( (DINFO, "MrvSlsJpeCapture:              Register JPE_STATUS_RIS = 0x%08X\n", ptMrvReg->jpe_status_ris) );
*/
    // generate header
    RetVal = MrvJpeGenerateHeader(MRV_JPE_HEADER_MODE_JFIF);
    if (RetVal != RET_SUCCESS)
    {
        return 0;
    }

    // now encode JPEG
    RetVal = MrvSls_JpeEncode(eUpdateTime, eMrvJpe_SingleShot);
    if (RetVal != RET_SUCCESS)
    {
        return 0;
    }

    return MrvMifGetByteCnt();
}

/*****************************************************************************/
/*!
 *  \FUNCTION    MrvSls_JpeEncode \n
 *  \RETURNVALUE RET_SUCCESS - everything ok \n
 *               RET_FAILURE - timeout during wait for encode done \n
 *  \PARAMETERS  enum for immediate: Marvin starts right away with the new 
 *                            configuration. This is useful if the image sensor
 *                            delivers only a limited number of frames, and
 *                            ons can't rely on the config update at the last
 *                            pixel of the previous frame. \n
 *               frame synchronous: 'traditional' behaviour, the changed
 *                            configuration becomes valid at the last pixel of
 *                            the just processed frame. Thus, the next complete
 *                            frame will be the first to be captured. \n
 *               or later (external) update \n
 *  \DESCRIPTION starts the JPEG encoder for 1 frame and waits until
 *               encoding is done \n
 */
/*****************************************************************************/
RESULT MrvSls_JpeEncode(teMrvConfUpdateTime eUpdateTime, teMrvJpeEncMode tMrvJpeEncMode)
{
    MrvJpePrepEnc(tMrvJpeEncMode);
    MrvStart(1, eUpdateTime);           //start Marvin for 1 frame to capture
    
    return MrvJpeWaitForEncodeDone();
}

/*****************************************************************************/
/*!
 *  \FUNCTION    MrvSls_JpeEncMotion \n
 *  \RETURNVALUE RET_SUCCESS - everything ok \n
 *               RET_FAILURE - timeout during wait for encode done \n
 *  \PARAMETERS  teMrvJpeEncMode - selects the kind of motion JPEG or not \n
 *               usFramesNum     - Number of encoding frames \n
 *               pulByteCount    - Pointer to Table for the filelength of 
 *                                 each encoded frame \n
 *  \DESCRIPTION starts the JPEG encoder for usFramesNum frames and waits until
 *               all encoding is done \n
 */
/*****************************************************************************/
RESULT MrvSls_JpeEncMotion(teMrvJpeEncMode tMrvJpeEncMode, UINT16 usFramesNum, UINT32* pulByteCount)
{
    RESULT RetVal = RET_SUCCESS;
    UINT32 uli = 0;

    MrvJpePrepEnc(tMrvJpeEncMode);
    MrvStart(usFramesNum, eMrvCfgUpdateFrameSync);           //start Marvin for ulFramesNum frames

    for (uli = 0; uli < usFramesNum; uli++)
    {  
       // JPEG encoding for ulFamesNum 
       RetVal = MrvJpeWaitForEncodeDone();
       if (RetVal != RET_SUCCESS)
       {
           return RetVal;
       }

       *pulByteCount = MrvMifGetByteCnt();          // [hf], 11.6.07: What's that? I want total number of bytes, not bytes of last frame + 1
       if (*pulByteCount == 0)                      //                dto.
       {
          return RET_FAILURE;
       }

       pulByteCount++;                              //                dto.
    }

    return RET_SUCCESS;
}


#endif    // #ifdef SL_SUPPORT_PARTLY
//#endif //if MRV_SUPPORT_SL

