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

#if (MARVIN_FEATURE_SMALL_OUTUNIT  == MARVIN_FEATURE_EXIST)
    #include "reg_access.h"
    #include "mrv_priv.h"
#endif // #if (MARVIN_FEATURE_SMALL_OUTUNIT  == MARVIN_FEATURE_EXIST)


/*****************************************************************************
 * DEFINES
 *****************************************************************************/
// abbreviations for local debug control ( level | module )
#define DERR  ( DBG_ERR  | DBG_MRV )    //!< errors
#define DWARN ( DBG_WARN | DBG_MRV )    //!< warnings
#define DINFO ( DBG_INFO | DBG_MRV )    //!< information


/*****************************************************************************/
/*!
 *  \FUNCTION    MrvSmlOutSetPath \n
 *  \RETURNVALUE none \n
 *  \PARAMETERS  discrete values to control the dat path \n
 *               tMainPath: eMrvPathOn  turn picture path on \n 
 *                          eMrvPathJpe choose JPEG encoder output \n
 *                          eMrvPathOff turn main picture path off \n
 *  \DESCRIPTION sets the Marvin small output interface path \n
 */
/*****************************************************************************/
void MrvSmlOutSetPath(teMrvPath tMainPath)
{
#if (MARVIN_FEATURE_SMALL_OUTUNIT  == MARVIN_FEATURE_EXIST)
    volatile tsMrvRegister *ptMrvReg = (tsMrvRegister*)MEM_MRV_REG_BASE;

    // switch main picture path
    switch (tMainPath)
    {
    case eMrvPathOn:
        REG_SET_SLICE(ptMrvReg->emp_out_ctrl, MRV_EMPOUT_JPEG_ENABLE, DISABLE);
        REG_SET_SLICE(ptMrvReg->emp_out_ctrl, MRV_EMPOUT_MP_ENABLE,    ENABLE);
        break;
    case eMrvPathJpe:
        REG_SET_SLICE(ptMrvReg->emp_out_ctrl, MRV_EMPOUT_JPEG_ENABLE,  ENABLE);
        REG_SET_SLICE(ptMrvReg->emp_out_ctrl, MRV_EMPOUT_MP_ENABLE,   DISABLE);
        break;
    case eMrvPathOff:
        REG_SET_SLICE(ptMrvReg->emp_out_ctrl, MRV_EMPOUT_JPEG_ENABLE, DISABLE);
        REG_SET_SLICE(ptMrvReg->emp_out_ctrl, MRV_EMPOUT_MP_ENABLE,   DISABLE);
        break;
    default:
        ASSERT("MrvSmlOutSetPath()" == "Wrong main picture path!");
        break;
    }
#else // #if (MARVIN_FEATURE_SMALL_OUTUNIT  == MARVIN_FEATURE_EXIST)
    UNUSED_PARAM(tMainPath);
#endif // #if (MARVIN_FEATURE_SMALL_OUTUNIT  == MARVIN_FEATURE_EXIST)
}

