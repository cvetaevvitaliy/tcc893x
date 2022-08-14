/* **************************************************** */
/*!
   @file	MN_DMD_Device.h
   @brief	Panasonic Demodulator Driver
   @author	R.Mori
   @date	2011/6/30
   @param
		(c)	Panasonic
   */
/* **************************************************** */
/*!
   this file defines common interface for each demodulator device
   */

#include "MN_DMD_driver.h"

#ifndef MN_DMD_DEVICE_H
#define MN_DMD_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

//return DMD_E_OK , if device support the system & bandwidth
extern DMD_ERROR_t	DMD_system_support( DMD_SYSTEM_t sys );

/* **************************************************** */
/*  Demodulator dependence functions (not exported)*/
/* **************************************************** */
//these functions is defined by each device (device_name.c)
extern DMD_ERROR_t	DMD_device_open( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_term( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_close( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_init( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_load_pseq ( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_pre_tune ( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_post_tune ( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_set_system( DMD_PARAMETER_t *param ); 
extern DMD_ERROR_t	DMD_device_reset( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_scan( DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_get_info( DMD_PARAMETER_t *param , DMD_u32_t id);
extern DMD_ERROR_t	DMD_device_set_info( DMD_PARAMETER_t *param , DMD_u32_t id ,DMD_u32_t val);
extern DMD_s32_t DMD_Carrier_Frequency_bias(DMD_PARAMETER_t* param);//Troy.wang added, 20120629, feedback IF center frequency bias for Tuner to retune, which is used for field test
extern DMD_s32_t DMD_XTAL_Frequency_bias(DMD_PARAMETER_t* param);//Troy.wang added, 20120629, feedback Clock frequency bias, , which is used for signal lock issue.

/* '11/08/29 : OKAMOTO	Select TS output. */
extern DMD_ERROR_t DMD_set_ts_output(DMD_TSOUT ts_out);


extern DMD_u32_t	DMD_RegSet_Rev;


#ifdef __cplusplus
}
#endif

#endif
