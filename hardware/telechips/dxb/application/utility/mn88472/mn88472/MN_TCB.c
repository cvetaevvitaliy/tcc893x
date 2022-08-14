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

#include "MN_DMD_driver.h"
#include "MN_DMD_common.h"
#include "MN_DMD_device.h"
#include "MN_I2C.h"
#include "MN_TCB.h"
#include "MN88472.h"


/* **************************************************** */
/* Tuner BUS Controll */
/* **************************************************** */
/* **************************************************** */
/*! Write 1byte to Tuner via Demodulator */
/* **************************************************** */

DMD_ERROR_t DMD_TCB_Write(DMD_u8_t	slvadr , DMD_u8_t adr , DMD_u8_t data )
{
	return DMD_TCB_WriteRead( slvadr , adr , &data , 1 , 0, 0 );
}

/* **************************************************** */
/*! Read 1byte from Tuner via Demodulator */
/* **************************************************** */
DMD_ERROR_t DMD_TCB_Read(DMD_u8_t	slvadr , DMD_u8_t adr , DMD_u8_t *data )
{
	return DMD_TCB_WriteRead( slvadr , adr ,  0, 0 , data , 1 );
}

/* '11/08/05 : OKAMOTO Implement "Through Mode". */
DMD_ERROR_t DMD_TCB_WriteAnyLength(DMD_u8_t	slvadr , DMD_u8_t adr , DMD_u8_t* data , DMD_u32_t wlen)
{
	return DMD_TCB_WriteRead( slvadr , adr , data , wlen , 0, 0 );
}

/* '11/08/05 : OKAMOTO Implement "Through Mode". */
DMD_ERROR_t DMD_TCB_ReadAnyLength(DMD_u8_t	slvadr , DMD_u8_t *data  , DMD_u8_t rlen)
{
	return DMD_TCB_WriteRead( slvadr , 0 ,  0, 0 , data , rlen );
}

/* **************************************************** */
/*! Write&Read any length from/to Tuner via Demodulator */
/* **************************************************** */

DMD_ERROR_t DMD_TCB_WriteRead(DMD_u8_t	slvadr , DMD_u8_t adr , DMD_u8_t* wdata , DMD_u32_t wlen , DMD_u8_t* rdata , DMD_u32_t rlen)
{
	DMD_u8_t	d[DMD_TCB_DATA_MAX];
	DMD_u32_t i;
	DMD_ERROR_t	ret = DMD_E_ERROR;
	
/* Set TCB Through Mode */ //Troy.wangyx masked, 20120801, once is enough; already done after initialization.(fixed in register array)
//	ret  = DMD_I2C_MaskWrite( DMD_BANK_T2_ , DMD_TCBSET , 0x7f , 0x53 ); //should be 0xD3 : Don't trigger an automatic demod reset when performing a write to the tuner in bridge mode
//	ret |= DMD_I2C_Write( DMD_BANK_T2_ , DMD_TCBADR , 0x00 );

#ifdef PC_CONTROL_FE
     	if( (wlen == 0 && rlen == 0) ||  (wlen != 0) )
	{
		d[0] = slvadr;
		d[1] = adr;
		for(i=0;i<wlen;i++)	d[i+2] = wdata[i];
		/* Read/Write */
		ret |= DMD_I2C_WriteRead(DMD_BANK_T2_ , DMD_TCBCOM , d , wlen + 2 , rdata , rlen );
	}
	else
	{
		d[0] = slvadr | 1;
		ret |= DMD_I2C_WriteRead(DMD_BANK_T2_ , DMD_TCBCOM , d , 1 , rdata , rlen );
		}
#else
	if( (wlen == 0 && rlen == 0) ||  (wlen != 0) )
	{
		d[0] = slvadr;
		d[1] = adr;
		for(i=0;i<wlen;i++)	d[i+2] = wdata[i];
		/* Read/Write */
		ret = DMD_I2C_Write_Anylenth(DMD_BANK_T2_ , DMD_TCBCOM , d , wlen + 2);
	}
	else
	{
		d[0] = slvadr | 1;
		ret = DMD_TCBI2C_Read(DMD_BANK_T2_ , DMD_TCBCOM , d , 1 , rdata , rlen );	  //Tuner READ : first write, then read, no broke by other I2C action. 
	}
#endif

	return ret;
}	
