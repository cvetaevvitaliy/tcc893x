/* **************************************************** */
/*!
   @file	MN_DMD_device.c
   @brief	Device dependence functions for MN88472
   @author	R.Mori
   @date	2011/7/6
   @param
		(c)	Panasonic
   */
/* **************************************************** */

#include "../common/MN_DMD_driver.h"
#include "../common/MN_DMD_common.h"
#include "../common/MN_DMD_device.h"
#include "../common/MN_I2C.h"
#include "MN88472.h"

/* **************************************************** */
/*	Constant
/* **************************************************** */
#define	DMD_DVBT_LOCKED_STATUS	10
#define	DMD_DVBT2_LOCKED_STATUS	13
#define	DMD_DVBC_LOCKED_STATUS	8

#define DMD_L1_BANK_SIZE 11
#define DMD_L1_REG_SIZE   8

/* '11/08/01 : OKAMOTO	Correct Error. */
#define DMD_ERROR_STATUS 0xFFFFFFFF

/* **************************************************** */
/*	Local Functions */
/* **************************************************** */
DMD_u32_t	DMD_RegRev(void);
DMD_u32_t	DMD_PSEQRev(void);
DMD_u32_t	DMD_AGC(void);
DMD_u32_t	DMD_System(void);
DMD_u32_t	DMD_Status(DMD_SYSTEM_t sys);			//SSEQ Status
DMD_u32_t	DMD_BER( DMD_SYSTEM_t sys , DMD_u32_t *err , DMD_u32_t *sum);
DMD_u32_t	DMD_BER_dvbt2( DMD_u32_t* err , DMD_u32_t* sum  , DMD_u32_t common );
DMD_u32_t	DMD_PER( DMD_SYSTEM_t sys , DMD_u32_t *err , DMD_u32_t *sum);
DMD_u32_t	DMD_CNR( DMD_SYSTEM_t sys , DMD_u32_t *cnr_i,DMD_u32_t *cnr_d);

/*! Information */
DMD_ERROR_t DMD_information_dvbt( DMD_PARAMETER_t *param , DMD_u32_t id , DMD_u32_t allflag);
DMD_ERROR_t DMD_information_dvbt2( DMD_PARAMETER_t *param , DMD_u32_t id, DMD_u32_t allflag);
DMD_ERROR_t DMD_information_dvbc( DMD_PARAMETER_t *param , DMD_u32_t id);
DMD_u32_t	DMD_get_l1( DMD_u8_t l1info[11][8] , DMD_u32_t	bitw , DMD_u32_t bank , DMD_u32_t adr , DMD_u32_t pos , DMD_u32_t allflag);
DMD_ERROR_t DMD_get_l1all( DMD_u8_t l1info[11][8] );

/* Scan ( for Channel Search )*/
DMD_ERROR_t DMD_scan_dvbt2( DMD_PARAMETER_t *param );
DMD_ERROR_t DMD_scan_dvbt( DMD_PARAMETER_t *param );
DMD_ERROR_t DMD_scan_dvbc( DMD_PARAMETER_t *param );

/* **************************************************** */
/*	Functions
/* **************************************************** */
/* **************************************************** */
/*! System Support information */
/* **************************************************** */

DMD_ERROR_t	DMD_system_support( DMD_SYSTEM_t sys )
{
	DMD_ERROR_t	ret;

	switch( sys )
	{
	case DMD_E_DVBT:
	case DMD_E_DVBT2:
	case DMD_E_DVBC:
		ret = DMD_E_OK;
		break;
	case DMD_E_ISDBT:
	case DMD_E_ISDBS:
	case DMD_E_DVBC2:
	case DMD_E_ATSC:
	case DMD_E_QAMB_64QAM:
	case DMD_E_QAMB_256QAM:
	case DMD_E_QAMC_64QAM:
	case DMD_E_QAMC_256QAM:
	case DMD_E_NTSC_M_BTSC:
	case DMD_E_PAL_M_BTSC:
	case DMD_E_PAL_N_BTSC:
	case DMD_E_PAL_B_G_NICAM:
	case DMD_E_PAL_I_NiCAM:
	case DMD_E_PAL_D_NiCAM:
	case DMD_E_PAL_B_G_A2:
	case DMD_E_SECAM_L_NiCAM:
	case DMD_E_SECAM_D_K_A2:
	default:
		ret = DMD_E_ERROR;
		break;
	}

	return ret;
}

/* **************************************************** */
/*! Load Auto Control Sequence */
/* **************************************************** */
DMD_ERROR_t DMD_device_load_pseq( DMD_PARAMETER_t* param )
{
	DMD_ERROR_t ret;
	DMD_u8_t	data;
	/* Load PSEQ Program */
	ret  = DMD_I2C_Write    ( DMD_BANK_T_, DMD_PSEQCTRL , 0x20 );
	ret |= DMD_I2C_Write    ( DMD_BANK_T_, DMD_PSEQSET  , 0x03 );
	ret |= DMD_I2C_WriteRead( DMD_BANK_T_, DMD_PSEQPRG  , MN88472_REG_AUTOCTRL , MN88472_REG_AUTOCTRL_SIZE , 0, 0 );
	ret |= DMD_I2C_Write    ( DMD_BANK_T_, DMD_PSEQSET  , 0x00 );

	/* Check Parity bit */
	ret |= DMD_I2C_Read( DMD_BANK_T_ , DMD_PSEQFLG , &data);
	if( data & 0x10 ){
		DMD_DBG_TRACE( "ERROR: PSEQ Parity" );
		ret |= DMD_E_ERROR;

	}

	return ret;
}

/* **************************************************** */
/*! Device Open */
/* **************************************************** */
DMD_ERROR_t DMD_device_open( DMD_PARAMETER_t* param )
{
	 return DMD_E_OK;
}
/* **************************************************** */
/*! Device Open */
/* **************************************************** */
DMD_ERROR_t DMD_device_close( DMD_PARAMETER_t* param )
{
	 return DMD_E_OK;
}
/* **************************************************** */
/*! Device Open */
/* **************************************************** */
DMD_ERROR_t DMD_device_term( DMD_PARAMETER_t* param )
{
	 return DMD_E_OK;
}

/* **************************************************** */
/*!	Get Register Settings for Broadcast system */
/* **************************************************** */
DMD_ERROR_t DMD_device_init( DMD_PARAMETER_t* param )
{
	DMD_ERROR_t	ret;
	/* Tranfer common register setting */
	ret = DMD_trans_reg( MN88472_REG_COMMON );
	DMD_CHECK_ERROR( ret , "DMD_device_init" );
    return ret;
}
/* **************************************************** */
/*!	Set Register setting for each broadcast system & bandwidth */
/* **************************************************** */
DMD_ERROR_t	DMD_device_set_system( DMD_PARAMETER_t* param)
{
	DMD_ERROR_t	ret;
	DMD_u8_t	nosupport;
	ret = DMD_E_OK;
	nosupport = 0;
	switch( param->system ){
		case DMD_E_DVBT2:
			switch( param->bw ){
				case DMD_E_BW_8MHZ:
					ret = DMD_trans_reg( MN88472_REG_DVBT2_8MHZ );
					break;
				case DMD_E_BW_6MHZ:
					ret = DMD_trans_reg( MN88472_REG_DVBT2_6MHZ );
					break;
				case DMD_E_BW_5MHZ:
					ret = DMD_trans_reg( MN88472_REG_DVBT2_5MHZ );
					break;
				case DMD_E_BW_1_7MHZ:
					ret = DMD_trans_reg( MN88472_REG_DVBT2_1_7MHZ );
					break;
				case DMD_E_BW_7MHZ:
					ret = DMD_trans_reg( MN88472_REG_DVBT2_7MHZ );
					break;
				default:
					nosupport = 1;
					break;
			}
			break;
		case DMD_E_DVBT:
			switch( param->bw ){
				case DMD_E_BW_8MHZ:
					ret = DMD_trans_reg( MN88472_REG_DVBT_8MHZ );
					break;
				case DMD_E_BW_7MHZ:
					ret = DMD_trans_reg( MN88472_REG_DVBT_7MHZ );
					break;
				case DMD_E_BW_6MHZ:
					ret = DMD_trans_reg( MN88472_REG_DVBT_6MHZ );
					break;
				default:
					nosupport = 1;
					break;	
			}
			break;
		case DMD_E_DVBC:
				ret = DMD_trans_reg( MN88472_REG_DVBC );
			break;
		default:
				nosupport = 1;
	}

	if( nosupport )
	{
		DMD_DBG_TRACE( "ERROR : Not Supported System");
	}
	DMD_CHECK_ERROR( ret , "DMD_device_set_system" );
	return ret;
}
/* **************************************************** */
/*! Pretune Process */
/* **************************************************** */
DMD_ERROR_t DMD_device_pre_tune( DMD_PARAMETER_t* param )
{
	return DMD_E_OK;
}
/* **************************************************** */
/*! Posttune Process */
/* **************************************************** */
DMD_ERROR_t DMD_device_post_tune( DMD_PARAMETER_t* param )
{
	DMD_device_reset(param);
	return DMD_E_OK;
}
/* **************************************************** */
/*! Soft Reset */
/* **************************************************** */
DMD_ERROR_t DMD_device_reset( DMD_PARAMETER_t* param )
{
	DMD_I2C_Write(DMD_BANK_T2_,DMD_RSTSET1,0x9f);
	return DMD_E_OK;
}

/* **************************************************** */
/*! scan */
/* **************************************************** */
DMD_ERROR_t DMD_device_scan( DMD_PARAMETER_t* param )
{
	DMD_ERROR_t	ret;
	ret = DMD_E_ERROR;
	switch( param->system )
	{
	case DMD_E_DVBT2:
		ret = DMD_scan_dvbt2( param );
		break;
	case DMD_E_DVBT:
		ret = DMD_scan_dvbt( param );
		break;
	case DMD_E_DVBC:
		ret = DMD_scan_dvbc( param );
		break;
	default:
		break;
	}
	return ret;
}

/* **************************************************** */
/*! Set Information  */
/* **************************************************** */
DMD_ERROR_t DMD_device_set_info( DMD_PARAMETER_t* param , DMD_u32_t id , DMD_u32_t val )
{
	DMD_ERROR_t	ret =DMD_E_ERROR;
	DMD_u8_t	rd;
	switch( param->system )
	{
		case DMD_E_DVBT:
			switch( id )
			{
			case DMD_E_INFO_DVBT_HIERARCHY_SELECT:
				DMD_I2C_Read( DMD_BANK_T_ , DMD_RSDSET_T , &rd );
				if( val == 1 )
				{
					rd |= 0x10;
				}
				else
				{
					rd &= 0xef;
				}
				DMD_I2C_Write( DMD_BANK_T_ , DMD_RSDSET_T , rd );
				param->info[DMD_E_INFO_DVBT_HIERARCHY_SELECT]	= (rd >> 4) & 0x1;
				ret = DMD_E_OK;
				break;
			case DMD_E_INFO_DVBT_MODE:
				ret =DMD_E_OK;
				if( val == DMD_E_DVBT_MODE_NOT_DEFINED ){
					DMD_I2C_Write( DMD_BANK_T_ , DMD_MDSET_T  ,  0xba );
					DMD_I2C_Write( DMD_BANK_T_ , DMD_MDASET_T ,  0x13 );
				}
				else
				{
					DMD_I2C_MaskWrite( DMD_BANK_T_ , DMD_MDSET_T , 0xf0 , 0xf0 );
					DMD_I2C_Write( DMD_BANK_T_ , DMD_MDASET_T    , 0x0  );
					switch( val )
					{
					case DMD_E_DVBT_MODE_2K:
						DMD_I2C_MaskWrite( DMD_BANK_T_ , DMD_MDSET_T , 0x0c , 0x00 );
						break;
					case DMD_E_DVBT_MODE_8K:
						DMD_I2C_MaskWrite( DMD_BANK_T_ , DMD_MDSET_T , 0x0c , 0x08 );
						break;
					default:
						ret = DMD_E_ERROR;
						break;
					}

				}
				break;
			case DMD_E_INFO_DVBT_GI:
				ret =DMD_E_OK;
				if( val == DMD_E_DVBT_GI_NOT_DEFINED ){
					DMD_I2C_Write( DMD_BANK_T_ , DMD_MDSET_T  ,  0xba );
					DMD_I2C_Write( DMD_BANK_T_ , DMD_MDASET_T ,  0x13 );
				}
				else
				{
					DMD_I2C_MaskWrite( DMD_BANK_T_ , DMD_MDSET_T , 0xf0 , 0xf0 );
					DMD_I2C_Write( DMD_BANK_T_ , DMD_MDASET_T    , 0x0  );
					switch( val )
					{
					case DMD_E_DVBT_GI_1_32:
						DMD_I2C_MaskWrite( DMD_BANK_T_ , DMD_MDSET_T , 0x03 , 0x00 );
						break;
					case DMD_E_DVBT_GI_1_16:
						DMD_I2C_MaskWrite( DMD_BANK_T_ , DMD_MDSET_T , 0x03 , 0x01 );
						break;
					case DMD_E_DVBT_GI_1_8:
						DMD_I2C_MaskWrite( DMD_BANK_T_ , DMD_MDSET_T , 0x03 , 0x02 );
						break;
					case DMD_E_DVBT_GI_1_4:
						DMD_I2C_MaskWrite( DMD_BANK_T_ , DMD_MDSET_T , 0x03 , 0x03 );
						break;
					default:
						ret = DMD_E_ERROR;
						break;
					}

				}
     		} //case DVB-T end 
			break;	/* '12/02/22 : OKAMOTO	Correct error. */
		case DMD_E_DVBT2:
			switch( id )
			{
				case	DMD_E_INFO_DVBT2_SELECTED_PLP	:
					rd = (DMD_u8_t) val;
					DMD_I2C_Write( DMD_BANK_T2_ , DMD_PLPID , rd );
					ret = DMD_E_OK;
					break;

			}
			break;	
		
		default:
			ret =DMD_E_ERROR;
			break;
	}

	return ret;
}

/* **************************************************** */
/*! Get Information  */
/* **************************************************** */
DMD_ERROR_t DMD_device_get_info( DMD_PARAMETER_t* param , DMD_u32_t id )
{
	DMD_u32_t i;
	switch( param->system ){
		case DMD_E_DVBT:
			if( id == DMD_E_INFO_DVBT_ALL ){
				param->info[DMD_E_INFO_DVBT_ALL] = DMD_E_INFO_DVBT_END_OF_INFORMATION;
				for( i=1 ; i < param->info[DMD_E_INFO_DVBT_ALL] ; i++ )
					DMD_information_dvbt( param , i , 1 );
			}
			else
					DMD_information_dvbt( param , id , 0 );
			break;
		case DMD_E_DVBT2:
			if( id == DMD_E_INFO_DVBT2_ALL ){
				param->info[DMD_E_INFO_DVBT2_ALL] = DMD_E_INFO_DVBT2_END_OF_INFORMATION;
				for( i=1 ; i < param->info[DMD_E_INFO_DVBT2_ALL] ; i++ )
					DMD_information_dvbt2( param , i , 1);
			}
			else if( id == DMD_E_INFO_DVBT2_L1_ALL )
			{
				DMD_information_dvbt2( param , id , 0);
				for(i=DMD_E_INFO_DVBT2_L1_ALL+1;i<DMD_E_INFO_DVBT2_END_OF_INFORMATION;i++)
				{
					DMD_information_dvbt2( param , i , 1);
				}
			}
			else
					DMD_information_dvbt2( param , id , 0);
			break;
		case DMD_E_DVBC:
			if( id == DMD_E_INFO_DVBC_ALL ){
				param->info[DMD_E_INFO_DVBC_ALL] = DMD_E_INFO_DVBC_END_OF_INFORMATION;
				for( i=1 ; i < param->info[DMD_E_INFO_DVBC_ALL] ; i++ )
					DMD_information_dvbc( param , i );
			}
			else
					DMD_information_dvbc( param , id );
			break;
		default:
			DMD_DBG_TRACE( "Unsupported Broadcast system" );
			return DMD_E_ERROR;
	}
	return DMD_E_OK;
}
	
/* **************************************************** */
/* MN88472 Local functions */
/* **************************************************** */
DMD_ERROR_t DMD_information_dvbt( DMD_PARAMETER_t *param , DMD_u32_t id , DMD_u32_t allflag)
{
	DMD_u32_t	err,sum;
	DMD_u32_t	cnr_i,cnr_d;
	DMD_u8_t	rd;

	switch( id ){
		case	DMD_E_INFO_DVBT_ALL	:
			param->info[DMD_E_INFO_DVBT_ALL] = DMD_E_INFO_DVBT_END_OF_INFORMATION;
			break;
		case	DMD_E_INFO_DVBT_REGREV	:
			param->info[DMD_E_INFO_DVBT_REGREV] = DMD_RegRev();
			break;
		case	DMD_E_INFO_DVBT_PSEQRV	:
			param->info[DMD_E_INFO_DVBT_PSEQRV] = DMD_PSEQRev();
			break;
		case	DMD_E_INFO_DVBT_SYSTEM	:
			param->info[DMD_E_INFO_DVBT_SYSTEM] = DMD_System();
			break;
		case	DMD_E_INFO_DVBT_STATUS	:
			if( allflag == 1 ) break;
		case	DMD_E_INFO_DVBT_LOCK	:
			param->info[DMD_E_INFO_DVBT_STATUS] = DMD_Status(param->system);
			if( param->info[DMD_E_INFO_DVBT_STATUS] >= DMD_DVBT_LOCKED_STATUS ){
				param->info[DMD_E_INFO_DVBT_LOCK] = DMD_E_LOCKED;
			}
			else if( param->info[DMD_E_INFO_DVBT_LOCK] == DMD_E_LOCKED )
			{
				param->info[DMD_E_INFO_DVBT_LOCK] = DMD_E_ERROR;
			}
			break;
		case	DMD_E_INFO_DVBT_AGC	:
			param->info[DMD_E_INFO_DVBT_AGC] = DMD_AGC();
			break;
		case	DMD_E_INFO_DVBT_BERRNUM	:
			if( allflag == 1 ) break;
		case	DMD_E_INFO_DVBT_BITNUM	:
			DMD_BER(param->system,&err,&sum);
			param->info[DMD_E_INFO_DVBT_BERRNUM]  = err;
			param->info[DMD_E_INFO_DVBT_BITNUM]   = sum;
			break;
		case	DMD_E_INFO_DVBT_CNR_INT	:
			if( allflag == 1 ) break;
		case	DMD_E_INFO_DVBT_CNR_DEC	:
			DMD_CNR(param->system,&cnr_i,&cnr_d);
			param->info[DMD_E_INFO_DVBT_CNR_INT]  = cnr_i;
			param->info[DMD_E_INFO_DVBT_CNR_DEC]  = cnr_d;
			break;
		case	DMD_E_INFO_DVBT_PERRNUM	:
			if( allflag == 1 ) break;
		case	DMD_E_INFO_DVBT_PACKETNUM	:
			DMD_PER(param->system,&err,&sum);
			param->info[DMD_E_INFO_DVBT_PERRNUM]  = err;
			param->info[DMD_E_INFO_DVBT_PACKETNUM]   = sum;
			break;
		case	DMD_E_INFO_DVBT_ERRORFREE	:
			DMD_I2C_Read( DMD_BANK_T_ , DMD_ERRFLG_T , &rd );
			param->info[DMD_E_INFO_DVBT_ERRORFREE] = (rd & 0x1)?0:1;
			break;
		case	DMD_E_INFO_DVBT_SQI	:
			//Get CNR
			DMD_CNR(param->system,&cnr_i,&cnr_d);
			param->info[DMD_E_INFO_DVBT_CNR_INT]  = cnr_i;
			param->info[DMD_E_INFO_DVBT_CNR_DEC]  = cnr_d;
			//Get BER
			DMD_BER(param->system,&err,&sum);
			param->info[DMD_E_INFO_DVBT_BERRNUM]  = err;
			param->info[DMD_E_INFO_DVBT_BITNUM]   = sum;
			//Calc SQI
			param->info[DMD_E_INFO_DVBT_SQI] = DMD_calc_SQI( param );
			break;
		case	DMD_E_INFO_DVBT_HIERARCHY_SELECT	:
			DMD_I2C_Read( DMD_BANK_T_ , DMD_RSDSET_T , &rd );
			param->info[DMD_E_INFO_DVBT_HIERARCHY_SELECT]	= (rd >> 4) & 0x1;
			break;
		case	DMD_E_INFO_DVBT_MODE	:
		case	DMD_E_INFO_DVBT_GI	:
		case	DMD_E_INFO_DVBT_LENGTH_INDICATOR	:
		case	DMD_E_INFO_DVBT_CONSTELLATION	:
		case	DMD_E_INFO_DVBT_HIERARCHY	:
		case	DMD_E_INFO_DVBT_HP_CODERATE	:
		case	DMD_E_INFO_DVBT_LP_CODERATE	:
		case	DMD_E_INFO_DVBT_CELLID	:
			if( allflag == 1 ) break;
		case	DMD_E_INFO_DVBT_TPS_ALL	:
			//Get TPS Information
			if( DMD_Status(param->system) < DMD_DVBT_LOCKED_STATUS ){
				//Sync is not established , TPS information is not available
				param->info[DMD_E_INFO_DVBT_TPS_ALL]	=	0;
			}
			else
			{
				param->info[DMD_E_INFO_DVBT_TPS_ALL]	=	1;
				DMD_I2C_Read( DMD_BANK_T_ , DMD_TMCCD4_T , &rd );
				param->info[DMD_E_INFO_DVBT_MODE]	= rd & 0x3;
				param->info[DMD_E_INFO_DVBT_GI]	= (rd >> 2 ) & 0x3;
				DMD_I2C_Read( DMD_BANK_T_ , DMD_TMCCD1_T , &rd );
				param->info[DMD_E_INFO_DVBT_LENGTH_INDICATOR]	= rd & 0x3f;
				DMD_I2C_Read( DMD_BANK_T_ , DMD_TMCCD2_T , &rd );
				param->info[DMD_E_INFO_DVBT_CONSTELLATION]	= (rd >> 3 ) & 0x3; 
				param->info[DMD_E_INFO_DVBT_HIERARCHY]	= rd & 0x7;
				DMD_I2C_Read( DMD_BANK_T_ , DMD_TMCCD3_T , &rd );
				param->info[DMD_E_INFO_DVBT_HP_CODERATE]	= (rd >> 3 ) & 0x7;
				param->info[DMD_E_INFO_DVBT_LP_CODERATE]	= rd & 0x7;
				param->info[DMD_E_INFO_DVBT_HIERARCHY]	= rd & 0x7;
				DMD_I2C_Read( DMD_BANK_T_ , DMD_CELLIDU_T , &rd );
				param->info[DMD_E_INFO_DVBT_CELLID]	= rd * 0x100;
				DMD_I2C_Read( DMD_BANK_T_ , DMD_CELLIDL_T , &rd );
				param->info[DMD_E_INFO_DVBT_CELLID]	+= rd ;

			}
			break;

		default:
			return DMD_E_ERROR;
	}


	return DMD_E_OK;
}
/*! DVBT2 Information */
DMD_ERROR_t DMD_information_dvbt2( DMD_PARAMETER_t *param , DMD_u32_t id, DMD_u32_t allflag)
{
	DMD_u32_t	err,sum;
	DMD_u32_t	cnr_i,cnr_d;
	DMD_u8_t	rd;
	static	DMD_u8_t	l1info[11][8];			//L1 Information Register
	switch( id ){
		case	DMD_E_INFO_DVBT2_ALL	:
			param->info[DMD_E_INFO_DVBT2_ALL] = DMD_E_INFO_DVBT2_END_OF_INFORMATION;
			break;
		case	DMD_E_INFO_DVBT2_REGREV	:
			param->info[DMD_E_INFO_DVBT2_REGREV] = DMD_RegRev();
			break;
		case	DMD_E_INFO_DVBT2_PSEQRV	:
			param->info[DMD_E_INFO_DVBT2_PSEQRV] = DMD_PSEQRev();
			break;
		case	DMD_E_INFO_DVBT2_SYSTEM	:
			param->info[DMD_E_INFO_DVBT2_SYSTEM] = DMD_System();
			break;
		case	DMD_E_INFO_DVBT2_LOCK	:
			param->info[DMD_E_INFO_DVBT2_STATUS] = DMD_Status(param->system);
			if( param->info[DMD_E_INFO_DVBT2_STATUS] >= DMD_DVBT2_LOCKED_STATUS ){
				param->info[DMD_E_INFO_DVBT2_LOCK] = DMD_E_LOCKED;
			} else if( param->info[DMD_E_INFO_DVBT2_LOCK] == DMD_E_LOCKED )
			{
				param->info[DMD_E_INFO_DVBT2_LOCK] = DMD_E_LOCK_ERROR;
			}
			break;
		case	DMD_E_INFO_DVBT2_AGC	:
			param->info[DMD_E_INFO_DVBT2_AGC] = DMD_AGC();
			break;
		case	DMD_E_INFO_DVBT2_BERRNUM	:
		case	DMD_E_INFO_DVBT2_BITNUM	:
			DMD_BER(param->system,&err,&sum);
			param->info[DMD_E_INFO_DVBT2_BERRNUM]  = err;
			param->info[DMD_E_INFO_DVBT2_BITNUM]   = sum;
			break;
		case	DMD_E_INFO_DVBT2_CNR_INT	:
		case	DMD_E_INFO_DVBT2_CNR_DEC	:
			DMD_CNR(param->system,&cnr_i,&cnr_d);
			param->info[DMD_E_INFO_DVBT2_CNR_INT]  = cnr_i;
			param->info[DMD_E_INFO_DVBT2_CNR_DEC]  = cnr_d;
			break;
		case	DMD_E_INFO_DVBT2_PERRNUM	:
		case	DMD_E_INFO_DVBT2_PACKETNUM	:
			DMD_PER(param->system,&err,&sum);
			param->info[DMD_E_INFO_DVBT2_PERRNUM]  = err;
			param->info[DMD_E_INFO_DVBT2_PACKETNUM]   = sum;
			break;
		case	DMD_E_INFO_DVBT2_ERRORFREE	:
				DMD_I2C_Read( DMD_BANK_T2_ , DMD_ERRFLG , &rd );
				param->info[DMD_E_INFO_DVBT2_ERRORFREE] = (rd & 0x1)?0:1;
				break;
		case	DMD_E_INFO_DVBT2_SQI	:
			//Get CNR
			DMD_CNR(param->system,&cnr_i,&cnr_d);
			param->info[DMD_E_INFO_DVBT2_CNR_INT]  = cnr_i;
			param->info[DMD_E_INFO_DVBT2_CNR_DEC]  = cnr_d;
			//Get BER
			DMD_BER(param->system,&err,&sum);
			param->info[DMD_E_INFO_DVBT2_BERRNUM]  = err;
			param->info[DMD_E_INFO_DVBT2_BITNUM]   = sum;
			//Calc SQI
			param->info[DMD_E_INFO_DVBT2_SQI] = DMD_calc_SQI( param );
			break;
		case	DMD_E_INFO_DVBT2_MODE	:
				DMD_I2C_Read( DMD_BANK_T2_ , DMD_SSEQRD1 , &rd );
				param->info[DMD_E_INFO_DVBT2_MODE] = rd & 0x7;
				break;
		case	DMD_E_INFO_DVBT2_GI	:	
		  param->info[DMD_E_INFO_DVBT2_GI]= DMD_get_l1( l1info, 3 , 0 , 2 , 6 , 0); break;
			break;
		case	DMD_E_INFO_DVBT2_BERRNUM_C	:
		case	DMD_E_INFO_DVBT2_BITNUM_C	:
			DMD_BER_dvbt2(&err,&sum,1);
			param->info[DMD_E_INFO_DVBT2_BERRNUM_C]   = err;
			param->info[DMD_E_INFO_DVBT2_BITNUM_C]   = sum;
			break;
		case	DMD_E_INFO_DVBT2_SELECTED_PLP	:
				DMD_I2C_Read( DMD_BANK_T2_ , DMD_PLPID , &rd );
				param->info[DMD_E_INFO_DVBT2_SELECTED_PLP] = rd ;
				break;
		case DMD_E_INFO_DVBT2_L1_ALL:
			DMD_get_l1all( l1info );
			break;
		case DMD_E_INFO_DVBT2_TYPE :			  param->info[DMD_E_INFO_DVBT2_TYPE]= DMD_get_l1( l1info, 8 , 0 , 0 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_BW_EXT :			  param->info[DMD_E_INFO_DVBT2_BW_EXT]= DMD_get_l1( l1info, 1 , 0 , 1 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_S1 :			  param->info[DMD_E_INFO_DVBT2_S1]= DMD_get_l1( l1info, 3 , 0 , 1 , 6 , allflag); break;
		case DMD_E_INFO_DVBT2_S2 :			  param->info[DMD_E_INFO_DVBT2_S2]= DMD_get_l1( l1info, 4 , 0 , 1 , 3 , allflag); break;
		case DMD_E_INFO_DVBT2_PAPR :			  param->info[DMD_E_INFO_DVBT2_PAPR]= DMD_get_l1( l1info, 4 , 0 , 2 , 3 , allflag); break;
		case DMD_E_INFO_DVBT2_L1_MOD :			  param->info[DMD_E_INFO_DVBT2_L1_MOD]= DMD_get_l1( l1info, 4 , 0 , 3 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_L1_COD :			  param->info[DMD_E_INFO_DVBT2_L1_COD]= DMD_get_l1( l1info, 2 , 0 , 3 , 3 , allflag); break;
		case DMD_E_INFO_DVBT2_L1_FEC_TYPE :		  param->info[DMD_E_INFO_DVBT2_L1_FEC_TYPE]= DMD_get_l1( l1info, 2 , 0 , 3 , 1 , allflag); break;
		case DMD_E_INFO_DVBT2_L1_POST_SIZE :		  param->info[DMD_E_INFO_DVBT2_L1_POST_SIZE]= DMD_get_l1( l1info, 18 , 1 , 0 , 1 , allflag); break;
		case DMD_E_INFO_DVBT2_L1_POST_INFO_SIZE :	  param->info[DMD_E_INFO_DVBT2_L1_POST_INFO_SIZE]= DMD_get_l1( l1info, 18 , 1 , 3 , 1 , allflag); break;
		case DMD_E_INFO_DVBT2_PILOT_PATTERN :		  param->info[DMD_E_INFO_DVBT2_PILOT_PATTERN]= DMD_get_l1( l1info, 4 , 0 , 4 , 3 , allflag); break;
		case DMD_E_INFO_DVBT2_TX_ID_AVAILABILITY :	  param->info[DMD_E_INFO_DVBT2_TX_ID_AVAILABILITY]= DMD_get_l1( l1info, 8 , 10 , 0 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_CELL_ID :			  param->info[DMD_E_INFO_DVBT2_CELL_ID]= DMD_get_l1( l1info, 16 , 10 , 1 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_NETWORK_ID :		  param->info[DMD_E_INFO_DVBT2_NETWORK_ID]= DMD_get_l1( l1info, 16 , 10 , 3 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_T2_SYSTEM_ID :		  param->info[DMD_E_INFO_DVBT2_T2_SYSTEM_ID]= DMD_get_l1( l1info, 16 , 10 , 5 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_NUM_T2_FRAMES :		  param->info[DMD_E_INFO_DVBT2_NUM_T2_FRAMES]= DMD_get_l1( l1info, 8 , 0 , 5 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_NUM_DATA_SYMBOLS :	  param->info[DMD_E_INFO_DVBT2_NUM_DATA_SYMBOLS]= DMD_get_l1( l1info, 12 , 0 , 6 , 3 , allflag); break;
		case DMD_E_INFO_DVBT2_REGEN_FLAG :		  param->info[DMD_E_INFO_DVBT2_REGEN_FLAG]= DMD_get_l1( l1info, 3 , 0 , 4 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_L1_POST_EXTENSION :	  param->info[DMD_E_INFO_DVBT2_L1_POST_EXTENSION]= DMD_get_l1( l1info, 1 , 0 , 4 , 0 , allflag); break;
		case DMD_E_INFO_DVBT2_NUM_RF :			  param->info[DMD_E_INFO_DVBT2_NUM_RF]= DMD_get_l1( l1info, 3 , 10 , 7 , 5 , allflag); break;
		case DMD_E_INFO_DVBT2_CURRENT_RF_IDX :		  param->info[DMD_E_INFO_DVBT2_CURRENT_RF_IDX]= DMD_get_l1( l1info, 3 , 10 , 7 , 2 , allflag); break;
		case DMD_E_INFO_DVBT2_SUB_SLICES_PER_FRAME :	  param->info[DMD_E_INFO_DVBT2_SUB_SLICES_PER_FRAME]= DMD_get_l1( l1info, 15 , 2 , 1 , 6 , allflag); break;
		case DMD_E_INFO_DVBT2_SUB_SLICE_INTERVAL :	  param->info[DMD_E_INFO_DVBT2_SUB_SLICE_INTERVAL]= DMD_get_l1( l1info, 22 , 2 , 3 , 5 , allflag); break;
		case DMD_E_INFO_DVBT2_NUM_PLP :			  param->info[DMD_E_INFO_DVBT2_NUM_PLP]= DMD_get_l1( l1info, 8 , 2 , 0 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_NUM_AUX :			  param->info[DMD_E_INFO_DVBT2_NUM_AUX]= DMD_get_l1( l1info, 4 , 9 , 5 , 3 , allflag); break;
		case DMD_E_INFO_DVBT2_PLP_MODE :		  param->info[DMD_E_INFO_DVBT2_PLP_MODE]= DMD_get_l1( l1info, 2 , 3 , 2 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_FEF_TYPE :		  param->info[DMD_E_INFO_DVBT2_FEF_TYPE]= DMD_get_l1( l1info, 4 , 9 , 0 , 3 , allflag); break;
		case DMD_E_INFO_DVBT2_FEF_LENGTH :		  param->info[DMD_E_INFO_DVBT2_FEF_LENGTH]= DMD_get_l1( l1info, 22 , 9 , 1 , 5 , allflag); break;
		case DMD_E_INFO_DVBT2_FEF_INTERVAL :		  param->info[DMD_E_INFO_DVBT2_FEF_INTERVAL]= DMD_get_l1( l1info, 8 , 9 , 4 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_ID :		  param->info[DMD_E_INFO_DVBT2_DAT_PLP_ID]= DMD_get_l1( l1info, 8 , 7 , 0 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_TYPE :		  param->info[DMD_E_INFO_DVBT2_DAT_PLP_TYPE]= DMD_get_l1( l1info, 3 , 3 , 0 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_PAYLOAD_TYPE :	  param->info[DMD_E_INFO_DVBT2_DAT_PLP_PAYLOAD_TYPE]= DMD_get_l1( l1info, 5 , 3 , 0 , 4 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_GROUP_ID :	  param->info[DMD_E_INFO_DVBT2_DAT_PLP_GROUP_ID]= DMD_get_l1( l1info, 8 , 7 , 1 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_COD :		  param->info[DMD_E_INFO_DVBT2_DAT_PLP_COD]= DMD_get_l1( l1info, 3 , 3 , 1 , 4 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_MOD :		  param->info[DMD_E_INFO_DVBT2_DAT_PLP_MOD]= DMD_get_l1( l1info, 3 , 3 , 1 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_ROTATION :	  param->info[DMD_E_INFO_DVBT2_DAT_PLP_ROTATION]= DMD_get_l1( l1info, 1 , 3 , 2 , 1 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_FEC_TYPE :	  param->info[DMD_E_INFO_DVBT2_DAT_PLP_FEC_TYPE]= DMD_get_l1( l1info, 2 , 3 , 1 , 1 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_NUM_BLOCKS_MAX :	  param->info[DMD_E_INFO_DVBT2_DAT_PLP_NUM_BLOCKS_MAX]= DMD_get_l1( l1info, 10 , 3 , 4 , 1 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_FRAME_INTEVAL :	  param->info[DMD_E_INFO_DVBT2_DAT_PLP_FRAME_INTEVAL]= DMD_get_l1( l1info, 8 , 3 , 6 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_TIME_IL_LENGTH :	  param->info[DMD_E_INFO_DVBT2_DAT_PLP_TIME_IL_LENGTH]= DMD_get_l1( l1info, 8 , 3 , 3 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_TIME_IL_TYPE :	  param->info[DMD_E_INFO_DVBT2_DAT_PLP_TIME_IL_TYPE]= DMD_get_l1( l1info, 1 , 3 , 2 , 0 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_FF_FLAG :		  param->info[DMD_E_INFO_DVBT2_DAT_FF_FLAG]= DMD_get_l1( l1info, 1 , 7 , 2 , 3 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_ID :		  param->info[DMD_E_INFO_DVBT2_COM_PLP_ID]= DMD_get_l1( l1info, 8 , 7 , 4 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_TYPE :		  param->info[DMD_E_INFO_DVBT2_COM_PLP_TYPE]= DMD_get_l1( l1info, 3 , 5 , 0 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_PAYLOAD_TYPE :	  param->info[DMD_E_INFO_DVBT2_COM_PLP_PAYLOAD_TYPE]= DMD_get_l1( l1info, 5 , 5 , 0 , 4 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_GROUP_ID :	  param->info[DMD_E_INFO_DVBT2_COM_PLP_GROUP_ID]= DMD_get_l1( l1info, 8 , 7 , 5 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_COD :		  param->info[DMD_E_INFO_DVBT2_COM_PLP_COD]= DMD_get_l1( l1info, 3 , 5 , 1 , 4 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_MOD :		  param->info[DMD_E_INFO_DVBT2_COM_PLP_MOD]= DMD_get_l1( l1info, 3 , 5 , 1 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_ROTATION :	  param->info[DMD_E_INFO_DVBT2_COM_PLP_ROTATION]= DMD_get_l1( l1info, 1 , 5 , 2 , 1 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_FEC_TYPE :	  param->info[DMD_E_INFO_DVBT2_COM_PLP_FEC_TYPE]= DMD_get_l1( l1info, 2 , 5 , 1 , 1 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_NUM_BLOCKS_MAX :	  param->info[DMD_E_INFO_DVBT2_COM_PLP_NUM_BLOCKS_MAX]= DMD_get_l1( l1info, 10 , 5 , 4 , 1 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_FRAME_INTEVAL :	  param->info[DMD_E_INFO_DVBT2_COM_PLP_FRAME_INTEVAL]= DMD_get_l1( l1info, 8 , 5 , 6 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_TIME_IL_LENGTH :	  param->info[DMD_E_INFO_DVBT2_COM_PLP_TIME_IL_LENGTH]= DMD_get_l1( l1info, 8 , 5 , 3 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_TIME_IL_TYPE :	  param->info[DMD_E_INFO_DVBT2_COM_PLP_TIME_IL_TYPE]= DMD_get_l1( l1info, 1 , 5 , 2 , 0 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_FF_FLAG :		  param->info[DMD_E_INFO_DVBT2_COM_FF_FLAG]= DMD_get_l1( l1info, 1 , 7 , 6 , 3 , allflag); break;
		case DMD_E_INFO_DVBT2_FRAME_IDX :		  param->info[DMD_E_INFO_DVBT2_FRAME_IDX]= DMD_get_l1( l1info, 8 , 8 , 0 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_TYPE_2_START :		  param->info[DMD_E_INFO_DVBT2_TYPE_2_START]= DMD_get_l1( l1info, 22 , 8 , 2 , 5 , allflag); break;
		case DMD_E_INFO_DVBT2_L1_CHANGE_COUNTER :	  param->info[DMD_E_INFO_DVBT2_L1_CHANGE_COUNTER]= DMD_get_l1( l1info, 8 , 8 , 1 , 7 , allflag); break;
		case DMD_E_INFO_DVBT2_START_RF_IDX :		  param->info[DMD_E_INFO_DVBT2_START_RF_IDX]= DMD_get_l1( l1info, 8 , 3 , 4 , 3 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_FIRST_RF_IDX :	  param->info[DMD_E_INFO_DVBT2_DAT_FIRST_RF_IDX]= DMD_get_l1( l1info, 3 , 7 , 2 , 2 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_START :		  param->info[DMD_E_INFO_DVBT2_DAT_PLP_START]= DMD_get_l1( l1info, 22 , 4 , 2 , 5 , allflag); break;
		case DMD_E_INFO_DVBT2_DAT_PLP_NUM_BLOCKS :	  param->info[DMD_E_INFO_DVBT2_DAT_PLP_NUM_BLOCKS]= DMD_get_l1( l1info, 10 , 4 , 0 , 1 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_FIRST_RF_IDX :	  param->info[DMD_E_INFO_DVBT2_COM_FIRST_RF_IDX]= DMD_get_l1( l1info, 3 , 7 , 6 , 2 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_START :		  param->info[DMD_E_INFO_DVBT2_COM_PLP_START]= DMD_get_l1( l1info, 22 , 6 , 2 , 5 , allflag); break;
		case DMD_E_INFO_DVBT2_COM_PLP_NUM_BLOCKS :	  param->info[DMD_E_INFO_DVBT2_COM_PLP_NUM_BLOCKS]= DMD_get_l1( l1info, 10 , 6 , 0 , 1 , allflag); break;
		case DMD_E_INFO_DVBT2_STATIC_FLAG :		  param->info[DMD_E_INFO_DVBT2_STATIC_FLAG]= DMD_get_l1( l1info, 1 , 3 , 2 , 5 , allflag); break;
		case DMD_E_INFO_DVBT2_STATIC_PADDING_FLAG :	  param->info[DMD_E_INFO_DVBT2_STATIC_PADDING_FLAG]= DMD_get_l1( l1info, 1 , 3 , 2 , 4 , allflag); break;
		case DMD_E_INFO_DVBT2_IN_BAND_A_FLAG :		  param->info[DMD_E_INFO_DVBT2_IN_BAND_A_FLAG]= DMD_get_l1( l1info, 1 , 3 , 2 , 3 , allflag); break;
		case DMD_E_INFO_DVBT2_IN_BAND_B_FLAG :		  param->info[DMD_E_INFO_DVBT2_IN_BAND_B_FLAG]= DMD_get_l1( l1info, 1 , 3 , 2 , 2 , allflag); break;

	
	}
	return DMD_E_OK;
}
/*! DVBC Information */
DMD_ERROR_t DMD_information_dvbc( DMD_PARAMETER_t *param , DMD_u32_t id)
{
	DMD_u32_t	err,sum;
	DMD_u32_t	cnr_i,cnr_d;
	DMD_u8_t	rd;

	switch( id ){
		case	DMD_E_INFO_DVBC_ALL	:
			param->info[DMD_E_INFO_DVBC_ALL] = DMD_E_INFO_DVBC_END_OF_INFORMATION;
			break;
		case	DMD_E_INFO_DVBC_REGREV	:
			param->info[DMD_E_INFO_DVBC_REGREV] = DMD_RegRev();
			break;
		case	DMD_E_INFO_DVBC_PSEQRV	:
			param->info[DMD_E_INFO_DVBC_PSEQRV] = DMD_PSEQRev();
			break;
		case	DMD_E_INFO_DVBC_SYSTEM	:
			param->info[DMD_E_INFO_DVBC_SYSTEM] = DMD_System();
			break;
		case	DMD_E_INFO_DVBC_LOCK	:
			param->info[DMD_E_INFO_DVBC_STATUS] = DMD_Status(param->system);
			if( param->info[DMD_E_INFO_DVBC_STATUS] >= DMD_DVBC_LOCKED_STATUS ){
				param->info[DMD_E_INFO_DVBC_LOCK] = DMD_E_LOCKED;
			}
			break;
		case	DMD_E_INFO_DVBC_AGC	:
			param->info[DMD_E_INFO_DVBC_AGC] = DMD_AGC();
			break;
		case	DMD_E_INFO_DVBC_BERRNUM	:
		case	DMD_E_INFO_DVBC_BITNUM	:
			DMD_BER(param->system,&err,&sum);
			param->info[DMD_E_INFO_DVBC_BERRNUM]  = err;
			param->info[DMD_E_INFO_DVBC_BITNUM]   = sum;
			break;
		case	DMD_E_INFO_DVBC_CNR_INT	:
		case	DMD_E_INFO_DVBC_CNR_DEC	:
			DMD_CNR(param->system,&cnr_i,&cnr_d);
			param->info[DMD_E_INFO_DVBC_CNR_INT]  = cnr_i;
			param->info[DMD_E_INFO_DVBC_CNR_DEC]  = cnr_d;
			break;
		case	DMD_E_INFO_DVBC_PERRNUM	:
		case	DMD_E_INFO_DVBC_PACKETNUM	:
			DMD_PER(param->system,&err,&sum);
			param->info[DMD_E_INFO_DVBC_PERRNUM]  = err;
			param->info[DMD_E_INFO_DVBC_PACKETNUM]   = sum;
			break;
		case	DMD_E_INFO_DVBC_ERRORFREE	:
			DMD_I2C_Read( DMD_BANK_T_ , DMD_ERRFLG_T , &rd );
			param->info[DMD_E_INFO_DVBT_ERRORFREE] = (rd & 0x1)?0:1;
			break;
	}
	return DMD_E_OK;


}

/* **************************************************** */
/* Informations */
/* **************************************************** */
//! Infomation : Register set Version
DMD_u32_t	DMD_RegRev()
{
	return DMD_RegSet_Rev;
}
//! Infomation : Auto control Version
DMD_u32_t	DMD_PSEQRev()
{
	DMD_u8_t	rd;	
	DMD_I2C_Read( DMD_BANK_T_ , DMD_PSEQOP1_T , &rd );
	return (DMD_u32_t) rd;
}
//! Infomation : AGC Loop filter Level
DMD_u32_t	DMD_AGC(void)
{
	DMD_u8_t	rd;
	DMD_u32_t	ret;

	DMD_I2C_Read( DMD_BANK_T2_ , DMD_AGCRDU , &rd );
	ret = rd * 4;
	DMD_I2C_Read( DMD_BANK_T2_ , DMD_AGCRDL , &rd );
	ret += rd >> 6;

	return ret;

}

//Troy.wang added, 20120629, feedback IF center frequency bias for Tuner to retune, which is used for field test
//Unit : Hz
DMD_s32_t DMD_Carrier_Frequency_bias(DMD_PARAMETER_t* param)
{ 
	DMD_u8_t  CarrierErrU = 0, CarrierErrL = 0;
	DMD_u32_t CarrierErrRd = 0; //unsigned
	DMD_s32_t CarrierErrKHz = 0; //signed. 
	DMD_u8_t  notsupport = 0,fNegativeCarrierErr = 0;
 
	/* Get estimated carrier error */	
	switch( param->system )
	 {
		case DMD_E_DVBT2:
		     {
			   // AGC monitor AGCRD selection : Carrier frequency error
			      DMD_I2C_Write( DMD_BANK_T2_ , DMD_AGCRDSET, 0x06 );
			   //AGCRD read [15:0]
			 	DMD_I2C_Read( DMD_BANK_T2_ , DMD_AGCRDU , &CarrierErrU);
				DMD_I2C_Read( DMD_BANK_T2_ , DMD_AGCRDL , &CarrierErrL);

                           CarrierErrRd = ( ((DMD_u32_t)CarrierErrU << 8) & 0xFF00) + CarrierErrL;

				/* Signed Two's Compliment, so if the MSB is 1, set invert and add 1 */
				if (0x8000 & CarrierErrRd) 
				{
					CarrierErrRd = ((~CarrierErrRd) & 0x0000FFFF) + 0x1;
					fNegativeCarrierErr = 1;
				}
				/*
				Carrer frequency error can be calculated by the following equation:
				Carrier frequency error = (fS*10^6) / 2^17 * AGCRRD[15:0] [Hz] 
				                                   = AGCRRD[15:0] *(fS*10^3) / 2^17 [KHz]
				//1MHz = 10^6 Hz, 2^17 = 131072
				where
				fS : FFT sampling frequency
				8 MHz bandwidth : 64/7 MHz
				7 MHz bandwidth : 8 MHz
				6 MHz bandwidth : 48/7 MHz
				5 MHz bandwidth : 40/7 MHz
				1.7 MHz bandwidth : 131/71 MHz
				  */  				
				switch( param->bw ) // Integer ,For better accuracy; Uint : KHz
				{
					case DMD_E_BW_8MHZ:
						 CarrierErrKHz = 64*CarrierErrRd/7*1000/131072; 
						break;
				      	case DMD_E_BW_7MHZ:
						CarrierErrKHz = 8*CarrierErrRd*1000/131072; 
						break;
					case DMD_E_BW_6MHZ:
						 CarrierErrKHz = 48*CarrierErrRd/7*1000/131072; 
						break;
					case DMD_E_BW_5MHZ:
						CarrierErrKHz = 40*CarrierErrRd/7*1000/131072; 
						break;
					case DMD_E_BW_1_7MHZ:
						CarrierErrKHz = 131*CarrierErrRd/71*1000/131072;
						break;
					default:
						notsupport = 1; //Hz
						break;
					}

			 // Set to default -> AGC monitor AGCRD selection : AGC loop filter value (input signal level)
		      DMD_I2C_Write( DMD_BANK_T2_ , DMD_AGCRDSET, 0x00 );
			} //case DMD_E_DVBT2 ends. 
			break;			
			
		case DMD_E_DVBT:
				{		
				DMD_u8_t dvbt_fft_mode;
				DMD_u32_t dvbt_ffts;
			   // Synchronization monitor SYN2RD_T selection : Carrier frequency error
			      DMD_I2C_Write( DMD_BANK_T_ , DMD_SYN2RDSET_T, 0x00 );
			   //SYN2RDU read [10:0]
			 	DMD_I2C_Read( DMD_BANK_T_ , DMD_SYN2RDU_T, &CarrierErrU);
				DMD_I2C_Read( DMD_BANK_T_ , DMD_SYN2RDL_T, &CarrierErrL);

                           CarrierErrRd = ( ((DMD_u32_t)CarrierErrU << 8) & 0xFF00) + CarrierErrL;

				/* Signed Two's Compliment, so if the MSB is 1, set invert and add 1 */
				if (0x0400 & CarrierErrRd) //valid 11 bits.
				{
					CarrierErrRd = ((~CarrierErrRd) & 0x000007FF) + 0x1;
					fNegativeCarrierErr = 1;
				}
				/*
				Carrier frequency error = (fS*10^6) /ffts * SYN2RD[10:0] [Hz] 
				                                   = SYN2RD[10:0]*(fS*10^3) / ffts [Khz]
				where				
				ffts : FFT size (8K: 8192, 2K: 2048)
				fS : FFT sampling frequency
				8 MHz bandwidth : 64/7 MHz
				7 MHz bandwidth : 8 MHz
				6 MHz bandwidth : 48/7 MHz
				  */
				DMD_I2C_Read( DMD_BANK_T_ , DMD_MDRD_T, &dvbt_fft_mode );
				dvbt_fft_mode = dvbt_fft_mode&0xC0;
				if ( dvbt_fft_mode == 0 ) // 2K mode
					{
					    dvbt_ffts = 2048;
					}
				else if ( dvbt_fft_mode == 2 )//8K mode
					{
					    dvbt_ffts = 8192;
					} 
				else
					{
						notsupport = 1;
						break;
					}
					
				switch( param->bw ) //Integer , For better accuracy ; Unit : KHz 
				{
					case DMD_E_BW_8MHZ:
						 CarrierErrKHz =  CarrierErrRd*1000*64/7/dvbt_ffts; 
						break;
				      	case DMD_E_BW_7MHZ:
						CarrierErrKHz = CarrierErrRd*1000*8/dvbt_ffts;
						break;
					case DMD_E_BW_6MHZ:
						 CarrierErrKHz = CarrierErrRd*1000*48/7/dvbt_ffts;
						break;
					default:
						notsupport = 1;
						break;
				 }
			} //case DMD_E_DVBT: ends.
			break;
			
		case DMD_E_DVBC:
			// currently, not use, leave it empty. 
                     /*
                            Calculate carrier frequency fERR error from AFCMON_C and
				sampling frequency fADCK is described by the following equation:
				fERR = fADCK / 227 * AFCMON_C
			*/
			break;
		default:
			notsupport = 1;
			break;
	}

        if(notsupport == 1)
        	{
	       CarrierErrKHz = 0;
		 DMD_DBG_TRACE( "ERROR : Not Supported mode");
        	}
		else
		{                          				 	
			    if (1 == fNegativeCarrierErr) 
				CarrierErrKHz = CarrierErrKHz * (-1);

			   printf(" Carrier frequency bias is [%d Khz] /n", CarrierErrKHz);
		}
 
	return CarrierErrKHz;
}

//Troy.wang added, 20120711, feedback IF center frequency bias for Tuner to retune, which is used for field test
//unit Hz .
DMD_s32_t DMD_XTAL_Frequency_bias(DMD_PARAMETER_t* param)
{ 
	DMD_u8_t  ClockErrU = 0, ClockErrL = 0;
	DMD_u32_t ClockErr = 0;
	DMD_s32_t ClockErrPPM = 0;
	DMD_u8_t  fNegativeClockErr = 0;
      DMD_u8_t nosupport = 0;

	/* Get estimated clock error */	
	switch( param->system ){
		case DMD_E_DVBT2:
               //Synchronization monitor SYN1RD selection : Clock frequency error
                  DMD_I2C_Write( DMD_BANK_T2_ , DMD_SYN1RDSET, 0x01 );

              //Calculate clock frequency error from SYN1RD : read [15:0]
              	DMD_I2C_Read( DMD_BANK_T2_ , DMD_SYN1RDU , &ClockErrU );
	             DMD_I2C_Read( DMD_BANK_T2_ , DMD_SYN1RDL, &ClockErrL );				 
			break;			
		case DMD_E_DVBT:
		    //Synchronization monitor SYN1RD _T selection : Clock frequency error
                  DMD_I2C_Write( DMD_BANK_T_, DMD_SYN1RDSET_T, 0x01 );

              //Calculate clock frequency error from SYN1RD_T : read [15:0]
              	DMD_I2C_Read( DMD_BANK_T_ , DMD_SYN1RDU_T , &ClockErrU );
	             DMD_I2C_Read( DMD_BANK_T_ , DMD_SYN1RDL_T , &ClockErrL );
			break;
			
		//DVB-C demodulator monitor doesn't have this .
		case DMD_E_DVBC:
		default:
			nosupport = 1;
			break;
	}

	if( nosupport )
	{
	       ClockErrPPM = 0;
		DMD_DBG_TRACE( "ERROR : Not Supported System");
	}
	else
	  {
                          ClockErr = ( ((DMD_u32_t)ClockErrU << 8) & 0xFF00) + ClockErrL;
				
				/* Signed Two's Compliment, so if the MSB is 1, set invert and add 1 */
				if (0x8000 & ClockErr) 
				{
					ClockErr = ((~ClockErr) & 0x0000FFFF) + 0x1;
					fNegativeClockErr = 1;
				}

		switch( param->system ){
				case DMD_E_DVBT2:
			       // Equation : SYN1RD/2^24 * 10^6 [ppm]	, for better accuracy :	
				// ClockErrPPM = (ClockErr) / 2^24 * 2^20 - (ClockErr)/ 2^24 * 48576	
				ClockErrPPM = (ClockErr/16) - (ClockErr*48576/16777216);		 
					break;			
				case DMD_E_DVBT:
			       // Equation : SYN1RD/2^27 * 10^6 [ppm]	, for better accuracy :	
				// ClockErrPPM = (ClockErr) / 2^27 * 2^20 - (ClockErr)/ 2^27 * 48576	
				ClockErrPPM = (ClockErr/128) - (ClockErr*48576/134217728);
					break;
				default:
                                   break;
			}
		
				if (1 == fNegativeClockErr) 
					ClockErrPPM = ClockErrPPM * (-1);
	  }

	printf(" XTAL frequency bias is [%d ppm] /n", ClockErrPPM);
	return ClockErrPPM;
}


/*! Information : System detect (from register setting) */
DMD_u32_t	DMD_System(void)
{
	DMD_u8_t	rd;
	DMD_u32_t	ret;

	DMD_I2C_Read( DMD_BANK_T2_ , DMD_DTVSET , &rd );

	rd &= 0x7;
	switch( rd ){
		case 2:
			ret = DMD_E_DVBT;
			break;
		case 3:
			ret = DMD_E_DVBT2;
			break;
		case 4:
			ret = DMD_E_DVBC;
			break;
		default:
			ret =DMD_E_NOT_DEFINED;
			break;

	}
	return ret;
}
/*! Information : Reciever Status ( Detail )  */
DMD_u32_t	DMD_Status( DMD_SYSTEM_t sys ){
	DMD_u8_t	rd;
	DMD_u32_t	ret;

	ret = 0;
	switch( sys ){
		case DMD_E_DVBT:
			DMD_I2C_Read( DMD_BANK_T_ , DMD_SSEQRD_T , &rd );
			ret = (DMD_u32_t) (rd & 0xf);
			break;
		case DMD_E_DVBT2:
			DMD_I2C_Read( DMD_BANK_T2_ , DMD_SSEQFLG , &rd );
			ret = (DMD_u32_t) (rd & 0xf);
			break;
		case DMD_E_DVBC:
			DMD_I2C_Read( DMD_BANK_C_ , DMD_SSEQMON2_C , &rd );
			ret = (DMD_u32_t) (rd & 0xf);
			break;

		/* '11/08/01 : OKAMOTO	Correct Error. */
		default:
			ret = DMD_ERROR_STATUS;
			break;
	};

	return ret;
}
//! Information : ERROR Num
DMD_u32_t	DMD_BER( DMD_SYSTEM_t sys , DMD_u32_t* err , DMD_u32_t* sum ){
	DMD_u8_t	rd;

	switch( sys )
	{
		case DMD_E_DVBT:
		case DMD_E_DVBC:
			//SET BERSET1[5] = 0
			DMD_I2C_Read( DMD_BANK_T_ , DMD_BERSET1_T , &rd );
			rd &= 0xDF;	//1101_1111
			DMD_I2C_Write( DMD_BANK_T_ , DMD_BERSET1_T , rd );
			//SET BERRDSET[3:0] = 0101
			DMD_I2C_Read( DMD_BANK_T_ , DMD_BERRDSET_T, &rd );
			rd = (rd & 0xF0 ) | 0x5;
			DMD_I2C_Write( DMD_BANK_T_ , DMD_BERRDSET_T, rd );
			//Read ERROR
			DMD_I2C_Read( DMD_BANK_T_ , DMD_BERRDU_T , &rd );
			*err = rd * 0x10000;
			DMD_I2C_Read( DMD_BANK_T_ , DMD_BERRDM_T , &rd );
			*err += rd * 0x100;
			DMD_I2C_Read( DMD_BANK_T_ , DMD_BERRDL_T , &rd );
			*err += rd ;
			//Read BERLEN
			DMD_I2C_Read( DMD_BANK_T_ , DMD_BERLENRDU_T , &rd );
			*sum = rd * 0x100;
			DMD_I2C_Read( DMD_BANK_T_ , DMD_BERLENRDL_T , &rd );
			*sum += rd;
			*sum = *sum * 203 * 8;
			break;

		case DMD_E_DVBT2:
			DMD_BER_dvbt2( err , sum , 0 );	//Data PLP
			break;

		default:
			break;
	}
	return 0;
}




//! Information : ERROR Num
DMD_u32_t	DMD_BER_dvbt2( DMD_u32_t* err , DMD_u32_t* sum  , DMD_u32_t common )
{
		DMD_u8_t	rd;

		//DATA PLP 
		DMD_I2C_Read ( DMD_BANK_T2_ , DMD_BERSET , &rd );

		if( common == 0 )
		{
			rd |= 0x20;		//BERSET[5] = 1 (BER after LDPC)
			rd &= 0xef;		//BERSET[4] = 0 (Data PLP)
		}
		else
		{
			rd |= 0x20;		//BERSET[5] = 1 (BER after LDPC)
			rd |= 0x10;		//BERSET[4] = 1 (Common PLP)
		}
		DMD_I2C_Write( DMD_BANK_T2_ , DMD_BERSET , rd );
		//Read ERROR
		DMD_I2C_Read( DMD_BANK_T2_ , DMD_BERRDU , &rd );
		*err = rd * 0x10000;
		DMD_I2C_Read( DMD_BANK_T2_ , DMD_BERRDM , &rd );
		*err += rd * 0x100;
		DMD_I2C_Read( DMD_BANK_T2_ , DMD_BERRDL , &rd );
		*err += rd ;
		//Read BERLEN
		DMD_I2C_Read( DMD_BANK_T2_ , DMD_BERLEN , &rd );
		*sum = (1 << (rd & 0xf) );
		if( common == 0 )
		{
			DMD_I2C_Write( DMD_BANK_T2_ , DMD_TPDSET2 , 0x3 );
		}
		else
		{
			DMD_I2C_Read ( DMD_BANK_T2_ , DMD_TPD2    , &rd );
		}
		if( (rd & 0x1 ) == 1 )
		{
			//FEC TYPE = 1
			switch( (rd>>2) & 0x7 )
			{
			case 0:	*sum = (*sum) * 32400; break;
			case 1:	*sum = (*sum) * 38880; break;
			case 2:	*sum = (*sum) * 43200; break;
			case 3:	*sum = (*sum) * 48600; break;
			case 4:	*sum = (*sum) * 51840; break;
			case 5:	*sum = (*sum) * 54000; break;
			}
		}
		else
		{
			//FEC TYPE = 0
			switch( (rd>>2) & 0x7 )
			{
			case 0:	*sum = (*sum) * 7200 * 4; break;
			case 1:	*sum = (*sum) * 9720 * 4; break;
			case 2:	*sum = (*sum) * 10800* 4; break;
			case 3:	*sum = (*sum) * 11880* 4; break;
			case 4:	*sum = (*sum) * 12600* 4; break;
			case 5:	*sum = (*sum) * 13320* 4; break;
			}

		}


		return 0;
}
//! Information : CNR
DMD_u32_t	DMD_CNR( DMD_SYSTEM_t sys , DMD_u32_t* cnr_i , DMD_u32_t* cnr_d ){
	DMD_u8_t	rd;
	DMD_s32_t	cnr;
	DMD_s32_t	sig,noise;
	switch( sys )
	{
		case DMD_E_DVBT:
			DMD_I2C_Read( DMD_BANK_T_ , DMD_CNRDU_T , &rd );
			cnr = rd * 0x100;
			DMD_I2C_Read( DMD_BANK_T_ , DMD_CNRDL_T , &rd );
			cnr += rd;
			if( cnr != 0 )
			{
				cnr = 65536 / cnr;
				cnr = log10_easy( cnr ) + 200;
				if( cnr < 0 ) cnr = 0;
			}
			else
				cnr = 0;

			*cnr_i = (DMD_u32_t ) cnr / 100;
			*cnr_d = (DMD_u32_t ) cnr % 100;
			break;
		case DMD_E_DVBT2:
			DMD_I2C_Read( DMD_BANK_T2_ , DMD_CNRDU , &rd );
			cnr = rd * 0x100;
			DMD_I2C_Read( DMD_BANK_T2_ , DMD_CNRDL , &rd );
			cnr += rd;
			DMD_I2C_Read( DMD_BANK_T2_ , DMD_CNFLG , &rd );
			if( cnr != 0 )
			{
				if( rd & 0x4 )
				{
					//MISO
					cnr = 16384 / cnr;
					cnr = log10_easy( cnr ) - 600;
					if( cnr < 0 ) cnr = 0;
					*cnr_i = (DMD_u32_t ) cnr / 100;
					*cnr_d = (DMD_u32_t ) cnr % 100;
				}
				else
				{
					//SISO
					cnr = 65536 / cnr;
					cnr = log10_easy( cnr ) + 200;
					if( cnr < 0 ) cnr = 0;
					*cnr_i = (DMD_u32_t ) cnr / 100;
					*cnr_d = (DMD_u32_t ) cnr % 100;

				}
			}
			else
			{
				*cnr_i = 0;
				*cnr_d = 0;
			}
			break;
		case DMD_E_DVBC:
			DMD_I2C_Read( DMD_BANK_C_ , DMD_CNMON1_C , &rd );
			sig = rd * 0x100;
			DMD_I2C_Read( DMD_BANK_C_ , DMD_CNMON2_C , &rd );
			sig += rd;
			DMD_I2C_Read( DMD_BANK_C_ , DMD_CNMON3_C , &rd );
			noise = rd * 0x100;
			DMD_I2C_Read( DMD_BANK_C_ , DMD_CNMON4_C , &rd );
			noise += rd;

			if( noise != 0 )
				cnr = log10_easy(sig * 8  / noise);
			else
				cnr = 0;

			if( cnr < 0 ) cnr = 0;
			*cnr_i = (DMD_u32_t ) cnr / 100;
			*cnr_d = (DMD_u32_t ) cnr % 100;
			break;

   
      default:
			break;
	}
	return 0;
}
//! Information : Packet Error
DMD_u32_t	DMD_PER( DMD_SYSTEM_t sys , DMD_u32_t* err , DMD_u32_t* sum ){
	DMD_u8_t	rd;

	switch( sys )
	{
		case DMD_E_DVBT:
		case DMD_E_DVBC:
		case DMD_E_DVBT2:
			//Read ERROR
			DMD_I2C_Read( DMD_BANK_T_ , DMD_PERRDU , &rd );
			*err = rd * 0x100;
			DMD_I2C_Read( DMD_BANK_T_ , DMD_PERRDL , &rd );
			*err += rd ;
			//Read BERLEN
			DMD_I2C_Read( DMD_BANK_T_ , DMD_PERLENRDU , &rd );
			*sum = rd * 0x100;
			DMD_I2C_Read( DMD_BANK_T_ , DMD_PERLENRDL , &rd );
			*sum += rd;
			break;
		default:
			break;
	}
	return 0;
}

/* **************************************************** */
/* Scan (for Channel search) */
/* **************************************************** */
DMD_ERROR_t DMD_scan_dvbt2( DMD_PARAMETER_t *param )
{
	DMD_u32_t	st;
	DMD_u32_t	now;
	DMD_u32_t	timeout = 1800;
	DMD_u8_t	rd;
	DMD_ERROR_t	ret = DMD_E_ERROR;

	DMD_timer(&st);
	param->info[DMD_E_INFO_LOCK] = DMD_E_LOCK_ERROR;	
	do
	{
		//P1 Give up
		DMD_I2C_Read( DMD_BANK_T2_  ,DMD_SSEQFLG , &rd );
		if( rd & 0x40 )
		{
			param->info[DMD_E_INFO_LOCK] = DMD_E_LOCK_NOSIGNAL;	
			break;
		}
		//Search Error
		if( rd & 0x20 )
		{
			param->info[DMD_E_INFO_LOCK] = DMD_E_LOCK_NOSYNC;	
			break;
		}
		//Sync judgement
		if( (rd & 0xf) >= 13 )
		{
			ret = DMD_E_OK;
			param->info[DMD_E_INFO_LOCK] = DMD_E_LOCKED;	
			break;
		}
		DMD_wait(1);			//wait 1ms
		DMD_timer( &now );
	}
	while( now - st < timeout );	//timeout 1.8

	return ret;
}
DMD_ERROR_t DMD_scan_dvbt( DMD_PARAMETER_t *param )
{
	DMD_u32_t	st;
	DMD_u32_t	now;
	DMD_u32_t	timeout = 1200;
	DMD_u8_t	rd;
	DMD_ERROR_t	ret = DMD_E_ERROR;
	DMD_timer(&st);
	param->info[DMD_E_INFO_LOCK] = DMD_E_LOCK_ERROR;	

	//set mode&gi search
	DMD_set_info( param , DMD_E_INFO_DVBT_MODE , DMD_E_DVBT_MODE_NOT_DEFINED );
	DMD_set_info( param , DMD_E_INFO_DVBT_GI   , DMD_E_DVBT_GI_NOT_DEFINED );

	do
	{
		DMD_I2C_Read( DMD_BANK_T_  ,DMD_SSEQRD_T , &rd );

		//OFDM ERROR
		if( rd & 0x80 )
		{
			param->info[DMD_E_INFO_LOCK] = DMD_E_LOCK_NOSIGNAL;	
			break;
		}
		//Mode Search
		if( rd & 0x20 )
		{
			param->info[DMD_E_INFO_LOCK] = DMD_E_LOCK_NOSYNC;	
			break;
		}
		//Sync detection
		if( (rd & 0xf) >= 9 )
		{
			ret = DMD_E_OK;
			param->info[DMD_E_INFO_LOCK] = DMD_E_LOCKED;	
			break;
		}
		DMD_wait(1);			//wait 1ms
		DMD_timer( &now );
	}
	while( now - st < timeout );	//timeout
	return ret;
}
DMD_ERROR_t DMD_scan_dvbc( DMD_PARAMETER_t *param )
{
	DMD_u32_t	st;
	DMD_u32_t	now;
	DMD_u32_t	timeout = 1800;
	DMD_u8_t	rd;
	DMD_ERROR_t	ret = DMD_E_ERROR;

	DMD_timer(&st);
	param->info[DMD_E_INFO_LOCK] = DMD_E_LOCK_ERROR;	

	do
	{
		DMD_I2C_Read( DMD_BANK_C_  ,DMD_STSMON_C , &rd );

		//QAM ERR
		if( rd & 0x40 )
		{
			param->info[DMD_E_INFO_LOCK] = DMD_E_LOCK_NOSYNC;	
			break;
		}
		DMD_I2C_Read( DMD_BANK_C_  ,DMD_DMDSTSMON1_C , &rd );
		//VQLOCK
		if( rd & 0x1 )
		{
			ret = DMD_E_OK;
			param->info[DMD_E_INFO_LOCK] = DMD_E_LOCKED;	
			break;
		}
		DMD_wait(1);			//wait 1ms
		DMD_timer( &now );
	}
	while( now - st < timeout );	//timeout

	return ret;
}

DMD_u32_t	DMD_get_l1( DMD_u8_t l1info[11][8] , DMD_u32_t	bitw , DMD_u32_t bank , DMD_u32_t adr , DMD_u32_t pos , DMD_u32_t allflag)
{
	DMD_s32_t	flag = bitw;
	DMD_u8_t	data;
	DMD_u32_t	ret = 0;

	for(;;)
	{
		if( allflag == 1 )
		{
			data = l1info[bank][adr];

		}
		else
		{
			DMD_I2C_Write( DMD_BANK_T2_ , DMD_TPDSET1 , 0 );
			DMD_I2C_Write( DMD_BANK_T2_ , DMD_TPDSET2 , (DMD_u8_t) bank );
			DMD_I2C_Read( DMD_BANK_T2_ , (DMD_u8_t)(DMD_TPD1 + adr) , &data );
		}

		if( flag <= 8 )
		{
			data = ( data & ((1<<(pos+1))-1) );
			data = ( data >> (pos+1-flag ) ); 
			ret  += data;
		}
		else
		{
			ret += ( data & ((1<<(pos+1))-1) );
		}
		flag -= (pos + 1);
		if( flag <= 0 ) 
		{
			break;
		}
		else
		{
			ret <<= 8;

		}
		adr ++;
		pos = 7;
	}

	return ret;

}

DMD_ERROR_t	DMD_get_l1all( DMD_u8_t l1info[DMD_L1_BANK_SIZE][DMD_L1_REG_SIZE] )
{
	DMD_u8_t i,j;
	DMD_u8_t	data;

	for( i=0; i < DMD_L1_BANK_SIZE ; i++ )
	{
		DMD_I2C_Write( DMD_BANK_T2_ , DMD_TPDSET1 ,  0 );
		DMD_I2C_Write( DMD_BANK_T2_ , DMD_TPDSET2 ,  i );
		for( j=0; j< DMD_L1_REG_SIZE; j ++ )
		{
			DMD_I2C_Read( DMD_BANK_T2_ , DMD_TPD1 + j , &data );
			l1info[i][j] = data;
		}
	}

	return DMD_E_OK;
}
/* '11/08/29 : OKAMOTO	Select TS output. */
DMD_ERROR_t DMD_set_ts_output( DMD_TSOUT ts_out )
{
	switch(ts_out)
       {
	case DMD_E_TSOUT_PARALLEL_FIXED_CLOCK:
		//TS parallel (Fixed clock mode)		TSSET1:0x00	FIFOSET:0xE1
		if( DMD_I2C_Write( DMD_BANK_T2_ , DMD_TSSET1 , 0x00 ) == DMD_E_ERROR ){
			return DMD_E_ERROR;
		}
		if( DMD_I2C_Write( DMD_BANK_T_ , DMD_FIFOSET , 0xE1 ) == DMD_E_ERROR ){
			return DMD_E_ERROR;
		}
		break;
	case DMD_E_TSOUT_PARALLEL_VARIABLE_CLOCK:
		//TS parallel (Variable clock mode)		TSSET1:0x00	FIFOSET:0xE3
		if( DMD_I2C_Write( DMD_BANK_T2_ , DMD_TSSET1 , 0x00 ) == DMD_E_ERROR ){
			return DMD_E_ERROR;
		}
		if( DMD_I2C_Write( DMD_BANK_T_ , DMD_FIFOSET , 0xE3 ) == DMD_E_ERROR ){
			return DMD_E_ERROR;
		}
		break;
	case DMD_E_TSOUT_SERIAL_VARIABLE_CLOCK:
		//TS serial(Variable clock mode)		TSSET1:0x1D	FIFOSET:0xE3
		if( DMD_I2C_Write( DMD_BANK_T2_ , DMD_TSSET1 , 0x1D ) == DMD_E_ERROR ){
			return DMD_E_ERROR;
		}
		if( DMD_I2C_Write( DMD_BANK_T_ , DMD_FIFOSET , 0xE3 ) == DMD_E_ERROR ){
			return DMD_E_ERROR;
		}
		break;
	default:
		return DMD_E_ERROR;
		break;
	}
	return DMD_E_OK;
}
