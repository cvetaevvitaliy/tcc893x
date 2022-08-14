/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/

/****************************************************************************

  Revision History

 ****************************************************************************

 ****************************************************************************/

#define LOG_NDEBUG 1
#define LOG_TAG "TUNER_INTERFACE"
#include <utils/Log.h>

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>
#include <OMX_Audio.h>
#include "OMX_Other.h"

#include <user_debug_levels.h>

#include "omx_tuner_interface.h"

#ifndef	NULL
#define	NULL	(0)
#endif

/*
  * 		ISDB-T
  *
  */
OMX_ERRORTYPE (*tcc_omx_isdbt_tuner_default (void))(OMX_COMPONENTTYPE *openaxStandComp)
{
	return	omx_tcc351x_CSPI_STS_tuner_component_Init;
}
OMX_ERRORTYPE  (*(pomx_isdbt_tuner_component_init[]))(OMX_COMPONENTTYPE *openmaxStandComp) =
{
	NULL,												// 0 - none
	omx_tcc351x_CSPI_STS_tuner_component_Init,		// 1 - TCC351X CSPI+STS
	NULL,					// 2 -Dib10096
	omx_tcc351x_I2C_STS_tuner_component_Init,			// 3 - TCC351X I2C+STS
	NULL,					// 4 - NMI326
	omx_tcc351x_I2C_SPIMS_tuner_component_Init,		// 5 - TCC351X I2C+SPIMS
	NULL,			// 6 - MTV818
	NULL,	// 7 - Toshiba TC90517
	omx_tcc353x_CSPI_STS_tuner_component_Init,  // 8 - TCC353X CSPI+STS
	omx_tcc353x_I2C_STS_tuner_component_Init,   // 9 - TCC353X CSPI+STS
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, 
};
 int	tcc_omx_isdbt_tuner_count (void)
{
	return 	(sizeof(pomx_isdbt_tuner_component_init)/sizeof(pomx_isdbt_tuner_component_init[0]));
}


/*
  *	DVB-T
  *
  */
OMX_ERRORTYPE (*tcc_omx_dvbt_tuner_default (void))(OMX_COMPONENTTYPE *openaxStandComp)
{
	return	omx_linuxtv_tuner_component_Init;
}
OMX_ERRORTYPE  (*(pomx_dvbt_tuner_component_init[]))(OMX_COMPONENTTYPE *openmaxStandComp) =
{
	NULL,												// 0 - none
	omx_linuxtv_tuner_component_Init, 					// 1 - LinuxTV Frontend
	omx_tcc351x_CSPI_STS_tuner_component_Init,		// 2 - TCC351X CSPI+STS
	omx_tcc351x_I2C_STS_tuner_component_Init,			// 3 - TCC351X I2C+STS
	omx_mxl101sf_tuner_component_Init,		// 4 - MxL101SF
	omx_mn88472_tuner_component_Init,  //5 - MN88472
	NULL, 
	NULL,
	NULL,
	NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL
};
int	tcc_omx_dvbt_tuner_count (void)
{
	return 	(sizeof(pomx_dvbt_tuner_component_init)/sizeof(pomx_dvbt_tuner_component_init[0]));
}

/*
  *	ATSC
  *
  */
OMX_ERRORTYPE (*tcc_omx_atsc_tuner_default (void))(OMX_COMPONENTTYPE *openaxStandComp)
{
	return	omx_atsc_tuner_component_Init;
}
OMX_ERRORTYPE  (*(pomx_atsc_tuner_component_init[]))(OMX_COMPONENTTYPE *openmaxStandComp) =
{
	NULL,												// 0 - none
	omx_atsc_tuner_component_Init,						// 1 - S5H1411
	NULL,
	NULL, 
	NULL,
	NULL,
	NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL
};
int	tcc_omx_atsc_tuner_count (void)
{
	return 	(sizeof(pomx_atsc_tuner_component_init)/sizeof(pomx_atsc_tuner_component_init[0]));
}  

/*
  * 		TDMB
  *
  */
OMX_ERRORTYPE (*tcc_omx_tdmb_tuner_default (void))(OMX_COMPONENTTYPE *openaxStandComp)
{
	return	omx_tcc317x_I2C_STS_tuner_component_Init;
}

OMX_ERRORTYPE  (*(pomx_tdmb_tuner_component_init[]))(OMX_COMPONENTTYPE *openmaxStandComp) =
{
	NULL,											// 0 - none
	omx_tcc351x_CSPI_SPIMS_tuner_component_Init,	// 1 - TCC351X CSPI+SPIMS
	omx_tcc351x_CSPI_STS_tuner_component_Init,		// 2 - TCC351X CSPI+STS
	omx_tcc351x_I2C_STS_tuner_component_Init,		// 3 - TCC351X I2C+STS
	omx_tcc317x_I2C_STS_tuner_component_Init,		// 4 - TCC317X I2C+STS
	NULL, 
	NULL,
	NULL,
	NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL
};

int	tcc_omx_tdmb_tuner_count (void)
{
	return 	(sizeof(pomx_tdmb_tuner_component_init)/sizeof(pomx_tdmb_tuner_component_init[0]));
}


