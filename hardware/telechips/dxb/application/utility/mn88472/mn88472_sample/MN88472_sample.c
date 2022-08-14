/* **************************************************** */
/*!
   @file	MN88472_sample.c
   @brief	Sample Application for MN88472(Panasonic DVB-T2/T/C Demodulator)
   @author	R.Mori
   @date	2011/7/06
   @param
		(c)	Panasonic
   */
/* **************************************************** */
//#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
//#include <windows.h>
#include "MN_DMD_driver.h"
#include "MN_DMD_common.h"
#include "MN_DMD_console.h"
#include "MN88472_UserFunction.h"
#include "MN_I2C.h"

#if (TUNER_TYPE == MXL603)    
#include "MxL603_TunerApi.h"
extern MXL603_TUNER_MODE_CFG_T tunerModeCfg;
extern MXL603_CHAN_TUNE_CFG_T chanTuneCfg;
#endif

#ifdef PC_CONTROL_FE
#define INPUT_ERROR(x)  fprintf( stderr , x )
#define PARAM_STRING(i) DMD_info_value( param->system , i , param->info[i] )
int select_menu( DMD_PARAMETER_t *param );
int select_system( DMD_PARAMETER_t *param );
int iic_test( );
#endif

int main(int argc , char *argv[])
{
	DMD_PARAMETER_t	param;			//Demodulator Parameter

	//Open API
	printf("Loading USB I2C Driver(API Open) ...");
	if( DMD_open(&param) == DMD_E_OK )
		printf("OK\n");
	else
		printf("NG\n");

	//Initialize LSI
	printf("Initializing LSI .. ");
	if( DMD_init(&param) == DMD_E_OK )
		printf("DMD init OK\n");
	else
		printf("DMD init NG\n");
	
#ifdef PC_CONTROL_FE 
	select_menu( &param ) ;
#else
    //pls. assign dev_id and do signal scan, refer to function:
    Scan_one_RF_sample(  param.bw,param.freq,DMD_E_DVBT2,0,DMD_E_DVBT_HIER_SEL_LP,DMD_E_SCAN_GET_CHANNEL_INFO);
#endif    
    return 0;
}

/* **************************************************** */
/*!	User interface  */
/*
Description :
purpose : 1. To find all info. in one RF channel; 2. to jump to one service based on stored info. 
input  : //you can also add IF_out_freq, or AGC control, if you want.Normally, not necessary
  user_scan_order ->  
          DMD_E_SCAN_GET_CHANNEL_INFO    : for user to get RF changel information 
          DMD_E_SCAN_SET_CHANNEL_PLP_ID  : for user to jump to one Service(like CCTV5), based on store info. 
                                           1. RF channel(freq.)
                                           2. Bandwidth;                                           
                                           3. PLP id;
                                           4. DTV system( if you don't want input this, do auto judge as DMD_E_SCAN_GET_CHANNEL_INFO did)
  dmd_bw   :  user-set bandwidth 
  freq_khz :  user-set RF freq. 

ONLY used for "DMD_E_SCAN_SET_CHANNEL_PLP_ID"
  set_dmd_sytem_mode : DTV system, DVB-T or DVB-T2
  t2_plp_no       : the PLP id towards the service user selected, for DVB-T2 only
  dvbt_hp_lp_sel  : Hierarchy mode or Low priority mode, for DVB-T only. 

return : ret -> 1 : signal unlocked, or parameter set error; 0. signal locked
Author :  Troy.wangyx, 20120801 
*/
/* **************************************************** */

DMD_ERROR_t Scan_one_RF_sample(  DMD_BANDWIDTH_t dmd_bw,DMD_u32_t freq_khz,
DMD_SYSTEM_t set_dmd_sytem_mode,DMD_u32_t t2_plp_no, DMD_DVBT_HIER_SEL_t dvbt_hp_lp_sel,DMD_SCAN_t user_scan_order)
{	 
    DMD_ERROR_t ret = DMD_E_ERROR;
#if (TUNER_TYPE == MXL603)
    MXL603_BW_E tuner_bw;	
     MXL603_SIGNAL_MODE_E signalMode;
#endif

//For DVB-T2
 //DMD_u8_t* dataPLParray = (void*)0;
	DMD_u8_t dataPLParray[256]; //Patch is here. 
	DMD_u8_t dataPLPsum = 0;
//For DVB-T 
	DMD_DVBT_HIERARCHY_t   hierarchy_exist;
	//DMD_DVBT_HIER_SEL_t hierarchy_selection;
//DMD param receive
	DMD_PARAMETER_t param; //Demodulator Parameter    
	param.bw = dmd_bw;
	param.freq= freq_khz*1000;
	param.system = set_dmd_sytem_mode;

//Set Tuner application mode --- BEGIN ---
#if (TUNER_TYPE == MXL603)
	switch(param.system)
	{
	case DMD_E_DVBT:           
	case DMD_E_DVBT2:        
			signalMode= MXL603_DIG_DVB_T_DTMB;
		 	switch(dmd_bw)
		    {
			case DMD_E_BW_6MHZ:           
					tuner_bw = MXL603_TERR_BW_6MHz;
			        break;
			case DMD_E_BW_7MHZ:          
					tuner_bw = MXL603_TERR_BW_7MHz;
			        break;      
			case DMD_E_BW_8MHZ:            
					tuner_bw = MXL603_TERR_BW_8MHz;
			        break;
			case DMD_E_BW_5MHZ:
			case DMD_E_BW_1_7MHZ:
			default:  
				       printf("--- UNSUPPORT BANDWIDTH!!---\n");
					  return DMD_E_ERROR;
		    }
	    break;      
	case DMD_E_DVBC:           
	case DMD_E_DVBC2:        
			signalMode= MXL603_DIG_DVB_C;
			switch(dmd_bw)
		    {         
			case DMD_E_BW_8MHZ:            
					tuner_bw = MXL603_CABLE_BW_8MHz;
	        break;
			case DMD_E_BW_6MHZ:
			case DMD_E_BW_7MHZ: 
			case DMD_E_BW_5MHZ:
			case DMD_E_BW_1_7MHZ:
			default:  
		       printf("--- UNSUPPORT BANDWIDTH!!---\n");
			   return DMD_E_ERROR;
		    }
			
	    break;  	
	default:  
		       printf("--- UNSUPPORT SYSTEM!!---\n");
			  return DMD_E_ERROR;
	}
		tunerModeCfg.signalMode = signalMode;
		chanTuneCfg.signalMode = signalMode;
		chanTuneCfg.freqInHz  = freq_khz*1000;
		chanTuneCfg.bandWidth = tuner_bw;
#endif
//Set Tuner application mode --- END --- 


   if ( DMD_E_SCAN_GET_CHANNEL_INFO == user_scan_order)  
  	{	  
      if ( param.system == DMD_E_DVBC )
      	{
			DMD_set_system(  &param);
			DMD_scan(  &param);	 		   
			if (param.info[DMD_E_INFO_LOCK] == DMD_E_LOCKED)
		   	{
		   	    // you find DVB-C frames in this channel 
	           ret = DMD_E_OK;			 
	            printf("--- DVB-C signal found !!---\n");

				;//you may add interface to get programe stream info.(eg. EPG, pid)
			     return ret;
		   	}
      	}
	  else
	  	{
		    // Auto recognizing of DVB-T and DVB-T2
	       /*----------------------START : Try to lock DVB-T  ---------------------*/
				//Tuner and DMD call relative register array, toward different System mode and Bandwidth.
	         param.system = DMD_E_DVBT;
		   DMD_set_system(  &param);
		
	        //Tuner tuning -> DMD scan
	       DMD_scan(  &param);	  
		   
		   if (param.info[DMD_E_INFO_LOCK] == DMD_E_LOCKED)
		   	{
		   	    // you find DVB-T frames in this channel 
	           ret = DMD_E_OK;			 
	            printf("--- DVB-T system found !!---\n");

				;//you may add interface to get programe stream info.(eg. EPG, pid)
			     return ret;
		   	}
	       /*----------------------END : Try to lock DVB-T  ---------------------*/



			 /*----------------------START : Try to lock DVB-T2  ---------------------*/
			//T2 signal, set PLP number,  for SPLP(signal PLP), pls. set PLP number to ZERO. 
			DMD_set_info(   &param , DMD_E_INFO_DVBT2_SELECTED_PLP , 0);	

			//Tuner and DMD call relative register array, toward different System mode and Bandwidth.
			param.system = DMD_E_DVBT2;
			DMD_set_system(  &param);

			//Tuner tuning -> DMD scan
			//if you don't change system mode, bandwidth,  you can just use DMD_scan to do Auto search.
			DMD_scan(  &param);	 

		   if (param.info[DMD_E_INFO_LOCK] == DMD_E_LOCKED)
		   	{
				char temp_i;
		   	    // you find DVB-T2 frames in this channel 
		   	    ret = DMD_E_OK; 
		   	/*
		   	You can use this function to get all PLP and store Data plp info.( one data plp may contain more than one program stream)
		   	DMD will analyze Data plp and its relative common plp automaticly, so no need to store common plp info.
		   	How to match RF freq, data plp and program stream ? Store their info. and link each other. 
		   	*/ 
				 //get data plp ID array and total data plp number
			   	 DMD_get_dataPLPs(  dataPLParray, &dataPLPsum , &param );
			            
		             	for(temp_i=0;temp_i<dataPLPsum;temp_i++)
						{ 		     
							 printf("Data PLP No.%d ",temp_i);
							 printf("PLP No. : 04x \n",dataPLParray[temp_i]);
						}
					
		              if(dataPLPsum == 0) //no data plp;  SPLP, dataPLPsum = 1
		          	   {  
		          	      printf("--- DVB-T2, no valid PLP !!---\n");
		                  return DMD_E_ERROR;
		          	}
				  
	              if (dataPLPsum >= 1) 
	              	{
							DMD_u8_t i;
							for(i=0;i<dataPLPsum;i++) //SPLP & Multi-PLP search;
							{
								// only selelt DATA PLP
								DMD_set_info(   &param, DMD_E_INFO_DVBT2_SELECTED_PLP, dataPLParray[i]);	

								/*
								| When performing a PLP switch reset the demod. Helps prevent any possible latchup
								| or performance issue that may happen when previously tuned to a "bad" PLP
								*/
								ret = DMD_device_reset(   &param ); 

								/* Call Lock/SYNC Status Judgement , analyse this PLP */
								ret = DMD_device_scan(   &param );

								if (param.info[DMD_E_INFO_LOCK] == DMD_E_LOCKED)
								{
									//you may add interface to get programe stream info. of each data PLP(eg. EPG, pid )
									;
								}
							}
	              	}
				  
		   	}
		   else //no DVB-T nor DVB-T2 signal 
		   	{
		   	       printf("--- NO VALID SIGNAL FOUND !!---\n");
	            ret = DMD_E_ERROR;
		   	}
	  	}
       /*----------------------END : Try to lock DVB-T2---------------------*/
   	}
   else if (DMD_E_SCAN_SET_CHANNEL_PLP_ID == user_scan_order)
   	{
		   	DMD_set_system(  &param);
			
        //Tuner tuning -> DMD scan
			/* 
			if you don't change system mode, bandwidth, you can just use DMD_scan to do Auto search.
			*/
           DMD_scan(  &param);
	
	   if (param.info[DMD_E_INFO_LOCK] == DMD_E_LOCKED)
	   	{
	   	  ret = DMD_E_OK;

           //actually, I want to set HP/LP, PLP before scan, it will save time. 
              //to set it later to prevent wrong parameter be set
	  	   	switch(param.system) 
			    {
			      case DMD_E_DVBT:     
			  	        // Check HP stream exist or not 	 
		                DMD_get_info( &param, DMD_E_INFO_DVBT_HIERARCHY);			
	                      hierarchy_exist = param.info[DMD_E_INFO_DVBT_HIERARCHY];

					     if ( hierarchy_exist > 0) //if no Hierarchy mode transferred, not allowed selected
					     	{
							  DMD_set_info(  &param, DMD_E_INFO_DVBT_HIERARCHY_SELECT, dvbt_hp_lp_sel);
					     	}
						  else //default : LP 
						  	{
						  	  DMD_set_info(  &param, DMD_E_INFO_DVBT_HIERARCHY_SELECT, DMD_E_DVBT_HIER_SEL_LP);
						  	}
				        break;
			      case DMD_E_DVBT2: 
			                //Get PLP Number
								DMD_get_info(  &param , DMD_E_INFO_DVBT2_NUM_PLP );
							
							if (param.info[DMD_E_INFO_DVBT2_NUM_PLP] > 1) //MPLP
								{  
								   if ( t2_plp_no+1 <= param.info[DMD_E_INFO_DVBT2_NUM_PLP]) //t2_plp_no starts from '0'.
								   	{
								      //T2 signal, set PLP number,  for SPLP(signal PLP), pls. set PLP number to ZERO. 
						            DMD_set_info(   &param , DMD_E_INFO_DVBT2_SELECTED_PLP , t2_plp_no);	
								   	}
									else
							    	{
										ret = DMD_E_ERROR;
										printf("--- INVALID PLP SETTING ! MORE THAN PLPNUM---\n");
							    	}
								}
							else //SPLP, default setting
								{
								  DMD_set_info(   &param , DMD_E_INFO_DVBT2_SELECTED_PLP , 0);	
								}
								/*
								 | When performing a PLP switch reset the demod. Helps prevent any possible latchup
								 | or performance issue that may happen when previously tuned to a "bad" PLP
								*/
								ret = DMD_device_reset(   &param ); 
					        break;      

                 case DMD_E_DVBC: 
				 	       ; //nothing to do..
				 	       break;
			        default:          
							  ret = DMD_E_ERROR;
			    }

			  if ( ret == DMD_E_OK)
			  	{  
                  /* Call Lock/SYNC Status Judgement */
                  ret = DMD_device_scan(   &param ); // re-analyse 
			  	}
	   	}
	   else 
	   	{
			    printf("--- USER SET CHANNEL INFO, SIGNAL UNLOCKED !!---\n");

				 ret = DMD_E_ERROR;
	   	}

   	}
    else
    	{  
    	     printf("--- USER ACTION INVALID !!---\n");
  
           ret = DMD_E_ERROR;
    	}
	
	return ret;
}

//MPLP Information sample
DMD_ERROR_t	dvbt2_mplp_information_sample( DMD_PARAMETER_t*	param )
{
	DMD_u32_t i;
	DMD_u8_t	pPLPIds[256];
	DMD_u8_t	pNumPLPs;


	DMD_get_dataPLPs(  pPLPIds, &pNumPLPs , param );
	for(i=0;i<pNumPLPs;i++)
	{
		 printf("No.%d ",i);
		 printf("ID:%04x ",pPLPIds[i]);
		 printf("\n");
	}
	return DMD_E_OK;
}


/*
The functions below are using PC communication protocal, 
Normally, the code below is just for refercence of system flow: Set system-> tuning -> scan ->  lock signal 

After you get the source code, if you want to compile and connect PC with FE ,
you can open the define as below. 
After compile, it will generate CMD box, try to communicate with FE. Don't forget to run with folder "i2c_fx2i2c_for_PC"
*/
#ifdef PC_CONTROL_FE 


/*! Main Function */
#define BUF_MAX	256
int tune( DMD_PARAMETER_t *param )
{
	char	buf[BUF_MAX];

	printf(" Frequency [MHz]:" );
	fgets( buf , BUF_MAX , stdin );

	param->freq = atoi( buf );
	param->funit = DMD_E_MHZ;

	DMD_tune( param );

	return 0;
}

int iic_test( ){
	char	buf[BUF_MAX];
	DMD_u8_t	slv,adr,dat;
	printf("IIC Test [r..read , w..write , q..quit]>");
	fgets( buf , BUF_MAX , stdin );


	switch( buf[0] ){
		case 'r':
			printf( "slave>" );
			fgets( buf , BUF_MAX , stdin );
			slv = (DMD_u8_t)strtol( buf , NULL , 16 );
			printf( "adr  >" );
			fgets( buf , BUF_MAX , stdin );
			adr = (DMD_u8_t)strtol( buf , NULL , 16 );
			DMD_I2C_Read( slv ,adr , &dat) ;
			printf("read >%x\n",dat);
			break;

		case 'w':
			printf( "slave>" );
			fgets( buf , BUF_MAX , stdin );
			slv = (DMD_u8_t)strtol( buf , NULL , 16 );
			printf( "adr  >" );
			fgets( buf , BUF_MAX , stdin );
			adr = (DMD_u8_t)strtol( buf , NULL , 16 );
			printf( "data >" );
			fgets( buf , BUF_MAX , stdin );
			dat = (DMD_u8_t)strtol( buf , NULL , 16 );
			DMD_I2C_Write( slv ,adr , dat) ;
			break;
		case 'q':			return 0;
	}

	return 1;
}

//Set PLP No sample
DMD_ERROR_t dvbt2_set_plp_sample(  DMD_PARAMETER_t *param )
{
	char	buf[BUF_MAX];
	printf("Select PLP No.(default=0)>" );
	fgets( buf , BUF_MAX , stdin );
	//select PLP No
	return DMD_set_info(  param , DMD_E_INFO_DVBT2_SELECTED_PLP , atoi( buf ) );
}

//Tranmission mode preset sample
DMD_ERROR_t dvbt_mode_preset_sample( DMD_PARAMETER_t *param )
{
	char	buf[BUF_MAX];
	printf("Use Mode Preset?(y/n)" );
	fgets( buf , BUF_MAX , stdin );
	if( buf[0] == 'y' )
	{
		printf("Select Mode(0..2k, 1..8k)>");
		fgets( buf , BUF_MAX , stdin );
		switch( buf[0] )
		{ 
			//set Mode preset
		case '0':
			DMD_set_info( param , DMD_E_INFO_DVBT_MODE , DMD_E_DVBT_MODE_2K );
			break;
		case '1':
			DMD_set_info(  param , DMD_E_INFO_DVBT_MODE , DMD_E_DVBT_MODE_8K );
			break;
		default:
			//To run mode&gi auto search , set both Mode & GI to NOT DEFINED .  
			DMD_set_info(  param , DMD_E_INFO_DVBT_MODE , DMD_E_DVBT_MODE_NOT_DEFINED );
			DMD_set_info(  param , DMD_E_INFO_DVBT_GI   , DMD_E_DVBT_GI_NOT_DEFINED );
			printf("Mode Error\n");
			return DMD_E_ERROR;
		}
		printf("Select GI(0..1/32, 1..1/16 , 2..1/8 , 3..1/4)>");
		fgets( buf , BUF_MAX , stdin );
		switch( buf[0] )
		{
			//set GI preset
		case '0':
			DMD_set_info(  param , DMD_E_INFO_DVBT_MODE , DMD_E_DVBT_GI_1_32 );
			break;
		case '1':
			DMD_set_info(  param , DMD_E_INFO_DVBT_MODE , DMD_E_DVBT_GI_1_16 );
			break;
		case '2':
			DMD_set_info(  param , DMD_E_INFO_DVBT_MODE , DMD_E_DVBT_GI_1_8 );
			break;
		case '3':
			DMD_set_info(  param , DMD_E_INFO_DVBT_MODE , DMD_E_DVBT_GI_1_4 );
			break;
		default:
			DMD_set_info(  param , DMD_E_INFO_DVBT_MODE , DMD_E_DVBT_MODE_NOT_DEFINED );
			DMD_set_info(  param , DMD_E_INFO_DVBT_GI   , DMD_E_DVBT_GI_NOT_DEFINED );
			printf("GI Error\n");
			return DMD_E_ERROR;
		}

	}
	else
	{
		DMD_set_info(  param , DMD_E_INFO_DVBT_MODE , DMD_E_DVBT_MODE_NOT_DEFINED );
		DMD_set_info(  param , DMD_E_INFO_DVBT_GI   , DMD_E_DVBT_GI_NOT_DEFINED );
	}

	return DMD_E_OK;

}
//Menu
int select_menu( DMD_PARAMETER_t *param )
{
	char	buf[BUF_MAX];
	DMD_u32_t	i,st,ed,s;
	DMD_u32_t	exit_flag=0;	

	while( exit_flag == 0 )
	{
		printf( "<MN88472 sample Menu>\n");
		printf( "0.. select broadcast system( DVBT2/T/C )\n");
		printf( "1.. tune\n");
 		printf( "2.. scan \n");
		printf( "3.. show the demodulator status\n");		
		printf( "4.. MPLP information sample \n");
		printf( "5.. PLP - select data PLP \n");
		printf( "m.. status monitor \n");
		printf( "z.. iic test\n");
		printf( "q.. quit application\n");
		printf( "select no.>");
		fgets( buf , BUF_MAX , stdin );

		switch( buf[0] ){
			case '0':	
				DMD_con_select_system(param );	
				DMD_set_system( param );
				break;
			case '1':	

				if( param->system == DMD_E_DVBT )
					//DVBT mode preset
					dvbt_mode_preset_sample( param);
				else if( param->system == DMD_E_DVBT2 )
					//DVBT2 plp no.
					dvbt2_set_plp_sample( param );

				DMD_con_input_freq(param );
				DMD_tune( param );
				break;

			case '2':
				printf( "start freq>");
				fgets( buf , BUF_MAX , stdin );
				st = atoi( buf );	
				printf( "end   freq>");
				fgets( buf , BUF_MAX , stdin );
				ed = atoi( buf );	
				printf( "step  freq>");
				fgets( buf , BUF_MAX , stdin );
				s = atoi( buf );	
				for( i=st;i<=ed;i+=s )
				{
					param->freq = i;
					param->funit = DMD_E_KHZ;
					printf("%4d : ",i );
					if( DMD_scan( param ) == DMD_E_OK )
					{
						printf("Locked");
					}
					else
					{
						printf("NG    ");
					}
					printf(" : %d" , param->info[DMD_E_INFO_LOCK]  );
					printf("\n");
				}

				break;

			case '3':	
				DMD_get_info( param , DMD_E_INFO_ALL );	
				DMD_con_show_status(param);
				break;

			case '4':
				dvbt2_mplp_information_sample( param );
				break;
				
		    case '5':
				printf( "after locked. Pls run case '4' to know the valid PLP you have.\n ");
				printf( "MPLP : pls. set the PLP ID> ");
				fgets( buf , BUF_MAX , stdin );
				i = atoi( buf );	
				  //select data PLP ID
		         DMD_set_info(  param , DMD_E_INFO_DVBT2_SELECTED_PLP , i );		 
		        DMD_wait(  300); 
				//wait demod to analyze this PLP,
				//if no such PLP(empty PLP), PER will be > 0. then you cannot see video output
				break;

			case 'm':
				printf("Press key to  exit\n");
				//while( !_kbhit() )
				for(i=0;i<10;i++)
				{
					DMD_get_info( param,0);
					printf("SQI: %3d " , param->info[DMD_E_INFO_DVBT_SQI]);
					DMD_con_show_monitor(param);					
					DMD_wait(  1000);

				}
				break;
			case 'z':	while(iic_test( ));	break;
#ifdef DMD_I2C_DEBUG
			case 'x':
				dmd_i2c_debug_flag ^= 1;
				break;
#endif
			case 'q':	exit_flag=1; break;
			default:	
				INPUT_ERROR("Not Exists\n");
		}
	}
	return 1;
}
#endif //#ifdef PC_CONTROL_FE


