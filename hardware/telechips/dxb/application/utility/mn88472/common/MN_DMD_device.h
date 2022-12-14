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
extern DMD_ERROR_t	DMD_device_open(  DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_term(  DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_close(  DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_init(  DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_load_pseq (  DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_pre_tune (  DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_post_tune (  DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_set_system(  DMD_PARAMETER_t *param ); 
extern DMD_ERROR_t	DMD_device_reset(  DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_scan(  DMD_PARAMETER_t *param );
extern DMD_ERROR_t	DMD_device_get_info(  DMD_PARAMETER_t *param , DMD_u32_t id);
extern DMD_ERROR_t	DMD_device_set_info(  DMD_PARAMETER_t *param , DMD_u32_t id ,DMD_u32_t val);
extern DMD_s32_t DMD_Carrier_Frequency_bias(  DMD_PARAMETER_t* param);//Troy.wang added, 20120629, feedback IF center frequency bias for Tuner to retune, which is used for field test
extern DMD_s32_t DMD_XTAL_Frequency_bias(  DMD_PARAMETER_t* param);//Troy.wang added, 20120629, feedback Clock frequency bias, , which is used for signal lock issue.

/* '11/08/29 : OKAMOTO	Select TS output. */
extern DMD_ERROR_t DMD_set_ts_output( DMD_TSOUT ts_out);
extern DMD_ERROR_t DMD_set_error_flag_output( DMD_u8_t bErrorFlagOutputEnable );
extern DMD_ERROR_t DMD_set_power_mode(  DMD_PWR_MODE_t running_mode );/* 2012130123 : Troy, power mode selection */
extern DMD_ERROR_t DMD_device_dvbt_64qam_2k_long_delay_echoes_improve(  DMD_PARAMETER_t *param );//20130220, troy, to improve DVB-T long echo delay performance of "Si2158+MN88472"
extern DMD_ERROR_t DMD_device_read_P1_flag(  DMD_u8_t* p1flg_rd ); //troy, 20130131, to judge signal including FEF or not 
extern DMD_ERROR_t DMD_device_dvbc_multipath_tolerance_improve(   );//20130220, troy, to improve DVB-C multipath tolerance performance of "Si2158+MN88472"
extern DMD_ERROR_t DMD_device_dvbt2_ISSY_validity_check(  DMD_PARAMETER_t *param );//20130306, Isshi san, added for checking ISSY field 
extern DMD_ERROR_t DMD_device_dvbt2_ignore_ISSY(void); //20130305, Simon san, added for ignoring ISSY, to check if ISSY field bring in abnormal action or not .
extern DMD_ERROR_t DMD_DVBC_GET_SYMBOL_RATE(  DMD_PARAMETER_t *param,DMD_u32_t *dvbc_get_symbolrate_Khz ); //20130318, troy added, for DVB-C symbol rate get. 
extern DMD_ERROR_t DMD_device_pseq_status( DMD_u8_t* pseq_status ); //20130412,troy.wang added, to check PSEQ running status. 
extern void DMD_BERLEN_adjust_dvbt2( DMD_PARAMETER_t* param );//20130318, troy added, to adjust BERLEN based on plp's real bitrate.

extern DMD_u32_t	DMD_RegSet_Rev;


#ifdef __cplusplus
}
#endif

#endif
