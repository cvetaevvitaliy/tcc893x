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

#include "mrv_priv.h"


/*!********************************************************************
 *
 *  \FUNCTION    MrvColSetColorProcessing \n
 *  \RETURNVALUE none \n
 *  \PARAMETERS  pointer to Marvin color settings (NULL=disable stage) \n
 *
 *  \NOTES       if a NULL pointer is handed over the color registers
 *               are not changed but the color processing stage is
 *               disabled completely \n
 *
 *  \DESCRIPTION writes the color values for contrast, brightness,
 *               saturation and hue into the appropriate Marvin
 *               registers \n
 *
 *********************************************************************/
void MrvColSetColorProcessing( const tsMrvColorSettings *ptMrvCol )
{
   volatile tsMrvRegister *ptMrvReg = (tsMrvRegister*)MEM_MRV_REG_BASE;

   if( ptMrvCol == NULL )
   {
      ptMrvReg->c_proc_ctrl = 0; //disable color processing (bypass)
   }
   else
   {
      ptMrvReg->c_proc_contrast   = ptMrvCol->ucContrast;
      ptMrvReg->c_proc_brightness = ptMrvCol->ucBrightness;
      ptMrvReg->c_proc_saturation = ptMrvCol->ucSaturation;
      ptMrvReg->c_proc_hue        = ptMrvCol->ucHue;

#if( MARVIN_FEATURE_EXT_YCBCR_RANGE == MARVIN_FEATURE_EXIST )
      //modify color processing registers    
      if (ptMrvCol->ulFlags & MRV_CPROC_C_OUT_RANGE)
      {
        ptMrvReg->c_proc_ctrl = ptMrvReg->c_proc_ctrl | MRV_CPROC_C_OUT_RANGE;
      }
    
      if (ptMrvCol->ulFlags & MRV_CPROC_Y_IN_RANGE)
      {
        ptMrvReg->c_proc_ctrl = ptMrvReg->c_proc_ctrl | MRV_CPROC_Y_IN_RANGE;
      }
    
      if (ptMrvCol->ulFlags & MRV_CPROC_Y_OUT_RANGE)
      {
        ptMrvReg->c_proc_ctrl = ptMrvReg->c_proc_ctrl | MRV_CPROC_Y_OUT_RANGE;
      }  
    
      if (ptMrvCol->ulFlags & MRV_CPROC_ENABLE)
      {
        ptMrvReg->c_proc_ctrl = ptMrvReg->c_proc_ctrl | MRV_CPROC_ENABLE;             
      }
#else
      // switch color processing on
      ptMrvReg->c_proc_ctrl = ptMrvReg->c_proc_ctrl | MRV_CPROC_ENABLE;             
#endif //#if( MARVIN_FEATURE_EXT_YCBCR_RANGE == MARVIN_FEATURE_EXIST )
   }
}


/*!********************************************************************
 *
 *  \FUNCTION    MrvColGetColorProcessing \n
 *  \RETURNVALUE none \n
 *  \PARAMETERS  pointer to Marvin color settings \n
 *  \DESCRIPTION get the color values out of the appropriate Marvin
 *               registers \n
 *
 *********************************************************************/
void MrvColGetColorProcessing( tsMrvColorSettings *ptMrvCol )
{
   volatile tsMrvRegister *ptMrvReg = (tsMrvRegister*)MEM_MRV_REG_BASE;
   
   ptMrvCol->ucContrast   = (UINT8)ptMrvReg->c_proc_contrast;
   ptMrvCol->ucBrightness = (UINT8)ptMrvReg->c_proc_brightness;
   ptMrvCol->ucSaturation = (UINT8)ptMrvReg->c_proc_saturation;
   ptMrvCol->ucHue        = (UINT8)ptMrvReg->c_proc_hue;
   ptMrvCol->ulFlags      = (UINT32)ptMrvReg->c_proc_ctrl;
}
