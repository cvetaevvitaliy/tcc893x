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


#ifndef _PROJECT_SETTTINGS_H
#define _PROJECT_SETTTINGS_H

#if 0
	/*****************************************************************************/
	/*!
	 * \name global usage of semihosting routines
	 */
	/*****************************************************************************/
	//@{
	//#define NO_SEMIHOSTING
	//@}


	/*****************************************************************************/
	/*!
	 * \name MEMORY ADDON
	 * - select what sort of memory add-on is used. (mainly interesting
	 *   for SM3-Boards, as SM1 and SM2 do not support memory add-ons
	 */
	/*****************************************************************************/
	//@{
	#define MEM_ADDON_NONE  0
	#define MEM_ADDON_SDRAM 1
	#define MEM_ADDON_SRAM  2

	#define MEMORY_ADDON  MEM_ADDON_SDRAM
	//@}


	/*****************************************************************************/
	/*!
	 * \name USED LCD
	 * - select which LCD is connected to the board.
	 */
	/*****************************************************************************/
	//@{
	#define LCD_EPSON_MD22          1
	#define LCD_TOSHIBA_LTM04C380K  2

	#define USED_LCD  LCD_TOSHIBA_LTM04C380K
	//@}


	/*****************************************************************************/
	/*!
	 * \name LCD SLOWDOWN MODE
	 * - select if the LCD should be slowed-down or stopped.
	 */
	/*****************************************************************************/
	//@{
	#define LCD_STOP                1
	#define LCD_SLOW_DOWN           2

	#define LCD_SLOWDOWN_MODE       LCD_STOP
	//@}


	/*****************************************************************************/
	/*!
	 * \name file access: fileblock or semihosting
	 */
	/*****************************************************************************/
	//@{
	#define NOSE    0  //!< no semihosting, use homebrewed functions for file access / function prefix is: nose_
	#define SEMI    1  //!< semihosting, use semihosted file access with c-lib functions

	#ifdef NO_SEMIHOSTING
	    #define FILEACC NOSE
	#else
	    #define FILEACC SEMI
	#endif
	//@}


	/*****************************************************************************/
	/*!
	 * \name I/O channel to use for user interface
	 */
	/*****************************************************************************/
	//@{
	#define UICH_SEMIHOSTED             1  //!< use semihosted console from AXD for user interactions
	#define UICH_LCD_KEYS               2  //!< use LCD and keys of the target board for user interactions

	#ifdef NO_SEMIHOSTING
	    #define UI_CHANNEL UICH_LCD_KEYS
	#else
	    #define UI_CHANNEL UICH_SEMIHOSTED
	#endif
	//@}


	/*****************************************************************************/
	/*!
	 * \name Output channel to use for debug messages, warnings, errors,
	 *       asserts and pre/post condition failures
	 */
	/*****************************************************************************/
	//@{
	#define DBGCH_SEMIHOSTED             1  //!< use semihosted console from AXD for debugging output
	#define DBGCH_LCD                    2  //!< use LCD of the target board for debugging output

	#ifdef NO_SEMIHOSTING
	    #define DBG_CHANNEL DBGCH_LCD
	#else
	    #define DBG_CHANNEL DBGCH_SEMIHOSTED
	#endif
	//@}


	/*****************************************************************************/
	/*!
	 * \name MotoDrive HW
	 * - select which motodrive hardware is used.
	 */
	/*****************************************************************************/
	//@{
	#define MOTODRIVE_NO                      1
	#define MOTODRIVE_NIKON_CP_4100           2
	#define MOTODRIVE_DUMMY                   3  //!< virtual drive unit for debugging
	#define MOTODRIVE_1LTD_HD80C              4
	#define MOTODRIVE_OV2630_NIDEC_COPAL_LPD4 5
	#define MOTODRIVE_6850                    6
	#define MOTODRIVE_MT9T012                 7
	#define MOTODRIVE_OV5630                  8
	#define MOTODRIVE_IMX034PQH5              9

	#define MOTODRIVE_HW MOTODRIVE_OV5630
	//@}

	//@}

	/*****************************************************************************/
	/*!
	 * \name startup behaviour of the FrameFun application
	 */
	/*****************************************************************************/
	//@{
	#define FF_STARTUP_MAINMENU       0  //!< just show the main menu (traditional behaviour)
	#define FF_STARTUP_DEMO           1  //!< immediately start the Demo-Mode
	#define FF_STARTUP_DEFAULTCFG     2  //!< load the default configuration file and enter marvin loop

	#define FRAMEFUN_STARTUP  FF_STARTUP_MAINMENU
	//#define FRAMEFUN_STARTUP  FF_STARTUP_DEMO
	//@}

	/*****************************************************************************/
	/*!
	 * \name usage of the camera emulator remote control feature.
	 */
	/*****************************************************************************/
	//@{
	#define USE_CEM_REMOTE_CONTROL
	//@}

	/*****************************************************************************/
	/*!
	 * \name CRC32_CHECK_MODE
	 * Use a CRC32 checksum to validate the picture output
	 */
	/*****************************************************************************/
	//@{
	#define CRC32_CHECK_MODE 2 //0 = just before, without crc32 checksum calculation
	                           //1 = store image and additional crc32 check and printout crc32 value
	                           //2 = only check crc32 and store picture on error
	//@}

#endif 


// build-time configuration for marvin and image sensor interface
// has been moved to specific files

#include "project_settings_mrv.h"
//#include "project_settings_isi.h"



#endif//_PROJECT_SETTTINGS_H
