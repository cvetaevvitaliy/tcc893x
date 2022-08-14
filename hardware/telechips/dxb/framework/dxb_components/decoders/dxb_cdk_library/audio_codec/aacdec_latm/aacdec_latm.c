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

void* latm_parser_init( unsigned char *pucPacketDataPtr, unsigned int piDataLength, int *piSamplingFrequency, int *piChannelNumber, void *psCallback, int eFormat );
int latm_parser_close( void *pLatm );
int latm_parser_get_frame( void *pLatmHandle, unsigned char *pucPacketDataPtr, int iStreamLength, unsigned char **pucAACRawDataPtr, int *piAACRawDataLength, unsigned int uiInitFlag );
int latm_parser_get_header_type( void *pLatmHandle );


void* Latm_Init( unsigned char *pucPacketDataPtr, unsigned int piDataLength, int *piSamplingFrequency, int *piChannelNumber, void *psCallback, int eFormat )
{
	return latm_parser_init( pucPacketDataPtr, piDataLength, piSamplingFrequency, piChannelNumber, psCallback, eFormat );
}

int Latm_Close( void *pLatm )
{
	return latm_parser_close( pLatm );
}

int Latm_GetFrame( void *pLatmHandle, unsigned char *pucPacketDataPtr, int iStreamLength, unsigned char **pucAACRawDataPtr, int *piAACRawDataLength, unsigned int uiInitFlag )
{
	return latm_parser_get_frame( pLatmHandle, pucPacketDataPtr, iStreamLength, pucAACRawDataPtr, piAACRawDataLength, uiInitFlag );
}

int Latm_GetHeaderType( void *pLatmHandle )
{
	return latm_parser_get_header_type( pLatmHandle );
}

