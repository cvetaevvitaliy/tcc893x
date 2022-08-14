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

#if MRV_SUPPORT_STATE2STRING

static char s_szText[64];


/*!*********************************************************************
 *  \if EMPTY_HEADER  <!-- Delete complete line, if Function Header is used -->
 *
 *  \FUNCTION     (empty) \n
 *
 *  \RETURNVALUE  Return Value 1  [Description] \n
 *                Return Value 2  [Description] \n
 *                Return Value n  [Description] \n
 *                
 *  \PARAMETERS   Parameter 1     [Description] \n
 *                Parameter 2     [Description] \n
 *                Parameter n     [Description] \n
 *
 *  \NOTES        (empty) \n
 *                (empty new line) \n
 *  
 *  \DESCRIPTION  (empty) \n
 *                (empty new line) \n
 *
 *  \endif  <!-- Delete complete line, if Function Header is used -->
 *********************************************************************/
const char *Mrv2Str_AwbMode(teMrvAwbMode eMode)
{
   switch(eMode)
   {
   case eMrvAwbCompletelyOff:   return "AWB block completely off";
   case eMrvAwbAuto:            return "HW automatic";
   case eMrvAwbManMeas:         return "manual, measure YCbCr means";
   case eMrvAwbManNoMeas:       return "measurements off";
   default:                     return "ERROR: Unknown AWB mode";
   }
}


/*!*********************************************************************
 *  \if EMPTY_HEADER  <!-- Delete complete line, if Function Header is used -->
 *
 *  \FUNCTION     (empty) \n
 *
 *  \RETURNVALUE  Return Value 1  [Description] \n
 *                Return Value 2  [Description] \n
 *                Return Value n  [Description] \n
 *                
 *  \PARAMETERS   Parameter 1     [Description] \n
 *                Parameter 2     [Description] \n
 *                Parameter n     [Description] \n
 *
 *  \NOTES        (empty) \n
 *                (empty new line) \n
 *  
 *  \DESCRIPTION  (empty) \n
 *                (empty new line) \n
 *
 *  \endif  <!-- Delete complete line, if Function Header is used -->
 *********************************************************************/
const char *Mrv2Str_DemosaicMode(teMrvDemosaicMode eMode)
{
   switch(eMode)
   {
   case eMrvDemosaicStandard: return "standard";
   case eMrvDemosaicEnhanced: return "enhanced";
   default:                   return "ERROR: Unknown Demosaic mode";
   }
}


/*!*********************************************************************
 *  \if EMPTY_HEADER  <!-- Delete complete line, if Function Header is used -->
 *
 *  \FUNCTION     (empty) \n
 *
 *  \RETURNVALUE  Return Value 1  [Description] \n
 *                Return Value 2  [Description] \n
 *                Return Value n  [Description] \n
 *                
 *  \PARAMETERS   Parameter 1     [Description] \n
 *                Parameter 2     [Description] \n
 *                Parameter n     [Description] \n
 *
 *  \NOTES        (empty) \n
 *                (empty new line) \n
 *  
 *  \DESCRIPTION  (empty) \n
 *                (empty new line) \n
 *
 *  \endif  <!-- Delete complete line, if Function Header is used -->
 *********************************************************************/
const char *Mrv2Str_Window(tsMrvWindow tWnd)
{
   sprintf(s_szText, "(x:%d, y:%d, w:%d, h:%d)", 
      (int)tWnd.usHOffs, (int)tWnd.usVOffs, (int)tWnd.usHSize, (int)tWnd.usVSize);
   return s_szText;
}


/*!*********************************************************************
 *  \if EMPTY_HEADER  <!-- Delete complete line, if Function Header is used -->
 *
 *  \FUNCTION     (empty) \n
 *
 *  \RETURNVALUE  Return Value 1  [Description] \n
 *                Return Value 2  [Description] \n
 *                Return Value n  [Description] \n
 *                
 *  \PARAMETERS   Parameter 1     [Description] \n
 *                Parameter 2     [Description] \n
 *                Parameter n     [Description] \n
 *
 *  \NOTES        (empty) \n
 *                (empty new line) \n
 *  
 *  \DESCRIPTION  (empty) \n
 *                (empty new line) \n
 *
 *  \endif  <!-- Delete complete line, if Function Header is used -->
 *********************************************************************/
const char *Mrv2Str_SlsAecMode(teMrvAecMode eMode)
{
   switch(eMode)
   {
   case eMrvAec_off:      return "off";
   case eMrvAec_integral: return "integral";
   case eMrvAec_spot:     return "spot";
   case eMrvAec_mfield5:  return "multi-field(5)";
   case eMrvAec_mfield9:  return "multi-field(9)";
   default:               return "ERROR: Unknown AEC mode";
   }
}

/*!*********************************************************************
 *  \if EMPTY_HEADER  <!-- Delete complete line, if Function Header is used -->
 *
 *  \FUNCTION     (empty) \n
 *
 *  \RETURNVALUE  Return Value 1  [Description] \n
 *                Return Value 2  [Description] \n
 *                Return Value n  [Description] \n
 *                
 *  \PARAMETERS   Parameter 1     [Description] \n
 *                Parameter 2     [Description] \n
 *                Parameter n     [Description] \n
 *
 *  \NOTES        (empty) \n
 *                (empty new line) \n
 *  
 *  \DESCRIPTION  (empty) \n
 *                (empty new line) \n
 *
 *  \endif  <!-- Delete complete line, if Function Header is used -->
 *********************************************************************/
const char *Mrv2Str_JpgCompression(int iCompression)
{
   switch (iCompression)
   {
      case JPEG_HIGH_COMPRESSION: return "JPEG_HIGH_COMPRESSION";
      case JPEG_LOW_COMPRESSION: return "JPEG_LOW_COMPRESSION";
      case JPEG_01_PERCENT: return "JPEG_01_PERCENT";
      case JPEG_20_PERCENT: return "JPEG_20_PERCENT";
      case JPEG_30_PERCENT: return "JPEG_30_PERCENT";
      case JPEG_40_PERCENT: return "JPEG_40_PERCENT";
      case JPEG_50_PERCENT: return "JPEG_50_PERCENT";
      case JPEG_60_PERCENT: return "JPEG_60_PERCENT";
      case JPEG_70_PERCENT: return "JPEG_70_PERCENT";
      case JPEG_80_PERCENT: return "JPEG_80_PERCENT";
      case JPEG_90_PERCENT: return "JPEG_90_PERCENT";
      case JPEG_99_PERCENT: return "JPEG_99_PERCENT";
      default:              return "ERROR: Unknown Compression ratio";
   }
}


/*!*********************************************************************
 *  \if EMPTY_HEADER  <!-- Delete complete line, if Function Header is used -->
 *
 *  \FUNCTION     (empty) \n
 *
 *  \RETURNVALUE  Return Value 1  [Description] \n
 *                Return Value 2  [Description] \n
 *                Return Value n  [Description] \n
 *                
 *  \PARAMETERS   Parameter 1     [Description] \n
 *                Parameter 2     [Description] \n
 *                Parameter n     [Description] \n
 *
 *  \NOTES        (empty) \n
 *                (empty new line) \n
 *  
 *  \DESCRIPTION  (empty) \n
 *                (empty new line) \n
 *
 *  \endif  <!-- Delete complete line, if Function Header is used -->
 *********************************************************************/
const char *Mrv2Str_Path(teMrvPath ePath)
{
   switch (ePath)
   {
      case eMrvPathDpcc:  return "dpcc BPT";
      case eMrvPathRaw16: return "raw16";
      case eMrvPathRaw:   return "raw";
      case eMrvPathJpe:   return "Jpeg encode";
      case eMrvPathOff:   return "off";
      case eMrvPathOn:    return "on";
      default:            return "ERROR: Unknown Path";
   }
}


/*!*********************************************************************
 *  \if EMPTY_HEADER  <!-- Delete complete line, if Function Header is used -->
 *
 *  \FUNCTION     (empty) \n
 *
 *  \RETURNVALUE  Return Value 1  [Description] \n
 *                Return Value 2  [Description] \n
 *                Return Value n  [Description] \n
 *                
 *  \PARAMETERS   Parameter 1     [Description] \n
 *                Parameter 2     [Description] \n
 *                Parameter n     [Description] \n
 *
 *  \NOTES        (empty) \n
 *                (empty new line) \n
 *  
 *  \DESCRIPTION  (empty) \n
 *                (empty new line) \n
 *
 *  \endif  <!-- Delete complete line, if Function Header is used -->
 *********************************************************************/
const char *Mrv2Str_YcsChnMode(teMrvYcsChnMode eMode)
{
   switch (eMode)
   {
      case eMrvYcsY:    return "Y only output";
      case eMrvYcsMvSp: return "separated Y/C main and self video";
      case eMrvYcsMv:   return "separated Y/C main video";
      default:          return "ERROR: Unknown channel mode";
   }
}


/*!*********************************************************************
 *  \if EMPTY_HEADER  <!-- Delete complete line, if Function Header is used -->
 *
 *  \FUNCTION     (empty) \n
 *
 *  \RETURNVALUE  Return Value 1  [Description] \n
 *                Return Value 2  [Description] \n
 *                Return Value n  [Description] \n
 *                
 *  \PARAMETERS   Parameter 1     [Description] \n
 *                Parameter 2     [Description] \n
 *                Parameter n     [Description] \n
 *
 *  \NOTES        (empty) \n
 *                (empty new line) \n
 *  
 *  \DESCRIPTION  (empty) \n
 *                (empty new line) \n
 *
 *  \endif  <!-- Delete complete line, if Function Header is used -->
 *********************************************************************/
const char *Mrv2Str_DpSwitch(teMrvDpSwitch eDataPath)
{
   switch (eDataPath)
   {
      case eMrvDpRaw:  return "raw data path";
      case eMrvDpJpeg: return "JPEG encoding path";
      case eMrvDpMv:   return "main path only";
      default:         return "ERROR: Unknown data path";
   }
}


/*!*********************************************************************
 *  \if EMPTY_HEADER  <!-- Delete complete line, if Function Header is used -->
 *
 *  \FUNCTION     (empty) \n
 *
 *  \RETURNVALUE  Return Value 1  [Description] \n
 *                Return Value 2  [Description] \n
 *                Return Value n  [Description] \n
 *                
 *  \PARAMETERS   Parameter 1     [Description] \n
 *                Parameter 2     [Description] \n
 *                Parameter n     [Description] \n
 *
 *  \NOTES        (empty) \n
 *                (empty new line) \n
 *  
 *  \DESCRIPTION  (empty) \n
 *                (empty new line) \n
 *
 *  \endif  <!-- Delete complete line, if Function Header is used -->
 *********************************************************************/
const char *Mrv2Str_AfmMode(teMrvAfmMode eMode)
{
   switch (eMode)
   {
      case eMrvAfm_HW:               return "hardware";
      case eMrvAfm_SwTenengrad:      return "Tennengrad function (SW)";
      case eMrvAfm_SwTreshSqrtGrad:  return "Threshold Squared Gradient (SW)";
      case eMrvAfm_SwFSWMedian:      return "Frequency Selective Weighted Median (SW)";
      case eMrvAfm_HW_norm:          return "hardware, luma normalized";
      case eMrvAfm_SwTenengrad_norm: return "Tennengrad function (SW), luma normalized";
      case eMrvAfm_SwFSWMedian_norm: return "Frequency Selective Weighted Median (SW), luma normalized";
      default:                       return "ERROR: Unknown AF measurement mode";
   }
}


/*!*********************************************************************
 *  \if EMPTY_HEADER  <!-- Delete complete line, if Function Header is used -->
 *
 *  \FUNCTION     (empty) \n
 *
 *  \RETURNVALUE  Return Value 1  [Description] \n
 *                Return Value 2  [Description] \n
 *                Return Value n  [Description] \n
 *                
 *  \PARAMETERS   Parameter 1     [Description] \n
 *                Parameter 2     [Description] \n
 *                Parameter n     [Description] \n
 *
 *  \NOTES        (empty) \n
 *                (empty new line) \n
 *  
 *  \DESCRIPTION  (empty) \n
 *                (empty new line) \n
 *
 *  \endif  <!-- Delete complete line, if Function Header is used -->
 *********************************************************************/
const char *Mrv2Str_AfssMode(teMrvAfssMode eMode)
{
   switch (eMode)
   {
      case eMrvAfss_Off:           return "no search";
      case eMrvAfss_FullRange:     return "full range search";
      case eMrvAfss_Hillclimbing:  return "hill climbing";
      case eMrvAfss_AdaptiveRange: return "adaptive range search";
      case eMrvAfss_HelimorphOpt:  return "helimorph optimized";
      case eMrvAfss_OV2630LPD4Opt: return "OV2630/LPD4 optimized";
      default:                     return "ERROR: Unknown AF search mode";
   }
}


/*!*********************************************************************
 *  \if EMPTY_HEADER  <!-- Delete complete line, if Function Header is used -->
 *
 *  \FUNCTION     (empty) \n
 *
 *  \RETURNVALUE  Return Value 1  [Description] \n
 *                Return Value 2  [Description] \n
 *                Return Value n  [Description] \n
 *                
 *  \PARAMETERS   Parameter 1     [Description] \n
 *                Parameter 2     [Description] \n
 *                Parameter n     [Description] \n
 *
 *  \NOTES        (empty) \n
 *                (empty new line) \n
 *  
 *  \DESCRIPTION  (empty) \n
 *                (empty new line) \n
 *
 *  \endif  <!-- Delete complete line, if Function Header is used -->
 *********************************************************************/
const char *Mrv2Str_IeMode(teMrvIeMode eMode)
{
   switch (eMode)
   {
      case eMrvIeMode_Off:       return "Image Effects OFF";
      case eMrvIeMode_Grayscale: return "grayscale";
      case eMrvIeMode_Negative:  return "negative";
      case eMrvIeMode_Sepia:     return "sepia";
      case eMrvIeMode_ColorSel:  return "color selection";
      case eMrvIeMode_Emboss:    return "emboss";
      case eMrvIeMode_Sketch:    return "sketch";
      case eMrvIeMode_Sharpen:   return "sharpen";
      default:                   return "ERROR: Unknown image effect";
   }
}

#else // MRV_SUPPORT_STATE2STRING

// This is just to avoid the boring compiler warning "C3040E: no external declaration
// in translation unit" at minimum costs. Sadly, we can not temporarily disable warnings in
// ADS1.2, but we want to keep our code warning-free.
// Chances are good that this will be optimized away by the linker...
//static const char s_cDummy = 0; 

#endif // MRV_SUPPORT_STATE2STRING

