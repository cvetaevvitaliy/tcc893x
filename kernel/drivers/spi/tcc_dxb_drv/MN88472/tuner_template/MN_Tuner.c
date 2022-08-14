
/* **************************************************** */
/*!
   @file	MN_DMD_Tuner_template.c
   @brief	Tuner API wrapper (sample)
   @author	R.Mori
   @date	2011/6/30
   @param
		(c)	Panasonic
   */
/* **************************************************** */
//---------- this is sample Tuner API  ----------------
#define LOG_TAG	"MN88472"
//#include <utils/Log.h>

//#include <stdio.h>
#include "../common/MN_DMD_driver.h"
#include "../common/MN_DMD_common.h"
#include "MxL603_Main.h"

#define PRINTF ALOGD

/* **************************************************** */
/* **************************************************** */
/*! Tuner Initialize Function*/
DMD_ERROR_t DMD_API DMD_Tuner_init()
{
	//TODO:	Please call tuner initialize API here
	//this function is called by DMD_init	
	//PRINTF("%s:LINE-%d\n",__func__, __LINE__);
	MxL603_OEM_Init();
	return DMD_E_OK;
}

/*! Tuner Tune Function */
DMD_ERROR_t DMD_API DMD_Tuner_tune(DMD_PARAMETER_t *param)
{
    int freqInKhz, bwInKhz;
	//TODO:	Please call tune  API here
	//this function is called by DMD_tune
	//PRINTF("%s:LINE-%d :%d-%d (%d)\n",__func__, __LINE__, param->freq, param->bw, param->funit);
    switch(param->funit)
    {
    case DMD_E_MHZ:
        freqInKhz = param->freq*1000;
        break;
    case DMD_E_KHZ:
        freqInKhz = param->freq;
        break;
    case DMD_E_HZ:
        freqInKhz = param->freq/1000;
        break;
    default:
        return DMD_E_ERROR;
    }

    switch(param->bw)
    {
    case DMD_E_BW_5MHZ:
        bwInKhz = 5000;
        break;
    case DMD_E_BW_6MHZ:
        bwInKhz = 6000;
        break;
    case DMD_E_BW_7MHZ:
        bwInKhz = 7000;
        break;
    case DMD_E_BW_8MHZ:
        bwInKhz = 8000;
        break;
    default:
        return DMD_E_ERROR;        
    }
    if(MxL603_OEM_Tune(freqInKhz,bwInKhz) != 0)
        return DMD_E_ERROR;        
	return DMD_E_OK;
}

/*! Tuner Termination Function */
DMD_ERROR_t DMD_API DMD_Tuner_term()
{
	//TODO:	Please call tune  API here
	//this function is called by DMD_term
	//PRINTF("%s:LINE-%d\n",__func__, __LINE__);
	MxL603_OEM_DeInit();
	return DMD_E_OK;
}

/*! Tuner Tune Function */
DMD_ERROR_t DMD_API DMD_Tuner_set_system(DMD_PARAMETER_t *param)
{
	//TODO:	Please call tune  API here
	//this function is called by DMD_tune
	//PRINTF("%s:LINE-%d\n",__func__, __LINE__);
	MxL603_OEM_SetSystem(param->system);
	return DMD_E_OK;
}
