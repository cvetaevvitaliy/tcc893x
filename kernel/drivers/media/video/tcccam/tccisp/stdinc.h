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


#ifndef _STDINC_H
#define _STDINC_H

//#define SL_FULL_SUPPORT
#define SL_SUPPORT_PARTLY

#include "debug_filter.h"
#include "project_settings.h"
//#include <stdarg.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <time.h>
//#include <math.h>
//#include <float.h>
#include "def.h"
//#include "mem.h"
//#include "mem_usage.h"
//#include "core.h"
//#include "csc.h"
//#include "gpio.h"
//#include "i2c.h"
#include "isi.h"
//#include "keyled.h"
//#include "motodrive.h"
//#include "mdi_motor.h"
//#include "lcd.h"
//#include "lcd_sls.h"
#include "mrv.h"
//#include "algorithms.h"
//#include "message_system.h"
//#include "fontprint_interface.h"
//#include "uisupport_interface.h"
//#include "debug.h"
#include "mrv_isp_cac.h"
#include "mrv_mipi.h"
//#include "mrv_smia.h"
//#include "mrv_smia_mipi.h"
#include "mrv_isp_dpcc.h"
#include "mrv_isp_dpf.h"
#include "mrv_isp_flash.h"
#include "mrv_isp_shutter.h"
#include "mrv_isp_hist_calc.h"
#include "mrv_isp_exposure.h"
//#include "cem_rc.h"
//#include "xchg_global.h"
#include "mrv_sls.h"
#include "utl_fixfloat.h"

#include "isp_interface.h"

//#include <reg_physical.h>

// extra definition
// ======================================================================
//#define MEM_MRV_REG_BASE                                  (volatile unsigned long*)0xB0210000
//#define IO_OFFSET		0x00000000	/* Virtual IO = 0xB0000000 */
#define MEM_MRV_REG_BASE                                  (volatile unsigned long*)(RegISP_BASE+IO_OFFSET)//(volatile unsigned long*)(0xB0210000+IO_OFFSET)
/* Physical value to Virtual Address */
//#define tcc_p2v(pa)	((unsigned int)(pa) + IO_OFFSET)



#define ASSERT(...) 
#define DBG_OUT( x )    

extern void* memset(void *dest, int c, int count );

extern UINT32 MrvSls_CalcScaleFactors( const tsMrvDatapathDesc *ptSource,
                                     const tsMrvDatapathDesc *ptPath,
                                     tsMrvScale* ptScale,
                                     int iImplementation )   ;
extern const tsMrvRszLut* MrvSls_GetRszLut( UINT32 ulFlags );
extern RESULT MrvSls_CalcMainPathSettings( const tsMrvDatapathDesc *ptSource,
                                           const tsMrvDatapathDesc *ptMain,
                                           tsMrvScale* ptScaleM,
                                           tsMrvMiCtrl* ptMrvMiCtrl);
extern RESULT MrvSls_CalcSelfPathSettings( const tsMrvDatapathDesc *ptSource,
                                           const tsMrvDatapathDesc *ptSelf,
                                           tsMrvScale* ptScaleS,
                                           tsMrvMiCtrl* ptMrvMiCtrl);
extern void MiscSetupExtMemory(int iWidth, int iHeight, teBufConfig eBufType, tsMrvMiPathConf* ptMrvMiPathConf, unsigned addr );

#endif //_STDINC_H
