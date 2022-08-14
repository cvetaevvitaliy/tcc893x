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


#ifndef _DBG_FILTER_H
#define _DBG_FILTER_H


/*********************************************************************/
/*!
 * \name DBG_CAT_FILTER, 
 *       debug output categories, values must be unique and power
 *       of 2. Upper 3 bytes must be 0. Debug output is only shown for
 *       the categories specified in DBG_CAT_FILTER
 */
/*********************************************************************/
//@{
#define DBG_INFO        0x00000001      //!< information mask
#define DBG_WARN        0x00000002      //!< warning mask
#define DBG_ERR         0x00000004      //!< error mask
#define DBG_ASSERT      0x00000008      //!< assert mask
#define DBG_CAT__ALL    0x000000FF      //!< mask to get all categories

#define DBG_CAT_FILTER  (DBG_CAT__ALL)  //!< currently used category mask
//@}


/*********************************************************************/
/*!
 * \name DBG_CAT_FILTER, 
 *       debug output modules, values must be unique and power
 *       of 2. Lower byte must be 0. Debug output is only shown for
 *       the modules specified in DBG_MODULE_FILTER
 */
/*********************************************************************/
//@{
#define DBG_I2C             0x00000100          //!< I2C driver mask
#define DBG_IRQ             0x00000200          //!< interrupt driver mask
#define DBG_ISI             0x00000400          //!< ISI driver mask
#define DBG_LCD             0x00000800          //!< LCD driver mask
#define DBG_MRV             0x00001000          //!< MARVIN driver mask
#define DBG_APP             0x00002000          //!< application mask
#define DBG_KLED            0x00004000          //!< key and LED driver mask
#define DBG_MOTO            0x00008000          //!< motor driver mask
#define DBG_UI              0x00010000          //!< user interface mask
#define DBG_NOSE            0x00020000          //!< non semi-hosting file access mask
#define DBG_MIPI            0x00040000          //!< mipi driver mask
#define DBG_REG             0x10000000          //!< register access functions/macros (very often!)
#define DBG_MODULE__ALL     0x0FFFFF00          //!< mask to get all modules

#define DBG_MODULE_FILTER   (DBG_MODULE__ALL)   //!< currently used module mask
//@}



#endif //_DBG_FILTER_H
