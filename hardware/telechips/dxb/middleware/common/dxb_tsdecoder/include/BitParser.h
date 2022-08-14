/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

#ifndef	_BITPARSER_H_
#define	_BITPARSER_H_


#define BITPARSER_DRV_NO_ERRROR 						0
#define BITPARSER_DRV_ERR_INVALID_HANDLE				-1
#define BITPARSER_DRV_ERR_INVALID_PARAMETER			-2
#define BITPARSER_DRV_ERR_ALLOC_MEM					-3
#define BITPARSER_DRV_ERR_INVALID_LENGTH				-4


typedef  struct s_BITPARSER_DRIVER_INFO_
{

	unsigned char  	fCurrent;       		/* data as unsigned char, 1 bit      */
	unsigned char     	ucCurrent;      		/* data as unsigned char,   1-8 bits   */
	unsigned short    	uiCurrent;     		 /* data as unsigned short,  9-16 bits  */
	
	unsigned int    	ulCurrent;      		/* data as unsigned int, 17-32 bits  */
	unsigned char    	*pucRawData;    		/* global data pointer      */
	unsigned char    	*pucSaveRawData; 	/* save global data pointer */

	unsigned char     	ucBitIndex;     		/* bit index                */
	unsigned char   	fDescrActive;   		/* used for descriptor length checking       */
	unsigned char		fLengthError;
	unsigned char		dummy;

	short    			iDescrLength;   		/* used for descriptor length error checking */

	//by see21
	//2007-4-9 16:53:38
	//original vesion is "short iGlobalLength"
	//but the length is 16bit width, signed is not enough.
	unsigned short    			iGlobalLength;  		/* used for parsing length error checking    */

}t_BITPARSER_DRIVER_INFO_;


/*****************************************************************************
*							
*     ROUTINE : BIT_OpenBitParser 
*                           
* DESCRIPTION : 
*
*       INPUT : none.  (Uses module wide global data)   
*                                                                            
*      OUTPUT : none                                                         
*                                                                            
*     RETURNS : none                                                         
*                                                                            
*****************************************************************************/
extern int BITPARS_OpenBitParser(void **handle );

/*****************************************************************************
*							
*     ROUTINE : BIT_CloseBitParser 
*                           
* DESCRIPTION : 
*
*       INPUT : none.  (Uses module wide global data)   
*                                                                            
*      OUTPUT : none                                                         
*                                                                            
*     RETURNS : none                                                         
*                                                                            
*****************************************************************************/
extern int BITPARS_CloseBitParser(void *handle );


/*****************************************************************************
*							
*     ROUTINE : BIT_CloseBitParser 
*                           
* DESCRIPTION : 
*
*       INPUT : none.  (Uses module wide global data)   
*                                                                            
*      OUTPUT : none                                                         
*                                                                            
*     RETURNS : none                                                         
*                                                                            
*****************************************************************************/
extern int BITPARS_InitBitParser(void *handle, void *pRawData, int iDefaultLength  );

/*****************************************************************************
*                                                                            
*     ROUTINE : DecrementGlobalLength                                        
*                                                                            
* DESCRIPTION : Decrements the Section's global length counter for each      
*               byte accessed, if the value goes below zero, there has been  
*               an error, parsing is stopped.                                
*               If a descriptor parsing is active, a check is                
*               also made to see that its length is NOT exceeded.            
*                                                                            
*               This routine also increments the "global" data pointer.      
*                                                                            
*       INPUT : none.  (Uses module wide global data)                        
*                                                                            
*      OUTPUT : none                                                         
*                                                                            
*     RETURNS : none                                                         
*                                                                            
*****************************************************************************/
extern int BITPARS_DecrementGlobalLength (void *handle);

/*****************************************************************************
*                                                                            
*     ROUTINE : IncrementGlobalLength                                        
*                                                                            
* DESCRIPTION : Decrements the Section's global length counter for each      
*               byte accessed, if the value goes below zero, there has been  
*               an error, parsing is stopped.                                
*               If a descriptor parsing is active, a check is                
*               also made to see that its length is NOT exceeded.            
*                                                                            
*               This routine also increments the "global" data pointer.      
*                                                                            
*       INPUT : none.  (Uses module wide global data)                        
*                                                                            
*      OUTPUT : none                                                         
*                                                                            
*     RETURNS : none                                                         
*                                                                            
*****************************************************************************/
extern int BITPARS_IncrementGlobalLength (void *handle);

/*****************************************************************************
*                                                                            
*     ROUTINE : GetBit                                                      
*                                                                            
* DESCRIPTION : This routine fetches a single bit from the MPEG "raw" table. 
*               This fecthing is done in such a way that it is Endian        
*               independant.                                                 
*                                                                            
*               This routine should ONLY be called by GetBits.               
*                                                                            
*       INPUT : none.  (Uses module wide global data)                        
*                                                                            
*      OUTPUT : none                                                         
*                                                                            
*     RETURNS : a bit stored in a unsigned char.                                        
*                                                                            
*****************************************************************************/
extern unsigned char BITPARS_GetBit (void *handle);

/*****************************************************************************
*                                                                            
*     ROUTINE : GetBits                                                      
*                                                                            
* DESCRIPTION : Returns the requested number of bits from the buffer         
*               containing the current table section that is being           
*               decoded. The coding could have simply called GetBit for      
*               the requested number of bits, but by adding some extra       
*               code, the routine is speeded up for unsigned char aligned data          
*               fetches. DecrementGlobalLength is used to increment the      
*               global data pointer.                                         
*                                                                            
*                                                                           
*        NOTE : The value fetched is returned in a module wide set of        
*               variables, the actual one used is dependant on the size      
*               of the requested data.                                       
*                      Size(bits)   Variable                                 
*                      -------------------------------------                 
*                          1        fCurrent and ucCurrent                   
*                          2-8      ucCurrent                                
*                          9-16     uiCurrent                                
*                         17-32     ulCurrent                                
*                                                                            
*                                                                            
*       INPUT : iLength - Number of bits of data to fetched from the MPEG    
*                         PSI buffer.                                        
*                                                                           
*      OUTPUT : none                                                         
*                                                                            
*     RETURNS : none                                                         
*                                                                            
*****************************************************************************/
extern int BITPARS_GetBits (void *handle, short iLength);
#endif //_BITPARSER_H_

