
typedef enum
{	
	// LATM/LOAS (Low Overhead Audio Stream): LATM with sync information
	TF_AAC_LOAS			= 0,	// default value

	// LATM (Low-overhead MPEG-4 Audio Transport Multiplex), without LOAS Sync-Layer, No random access is possible
	TF_AAC_LATM_MCP1	= 1,	// LATM wiht muxConfigPresent = 1
	TF_AAC_LATM_MCP0	= 2,	// LATM wiht muxConfigPresent = 0

	// ADTS (Audio Data Transport Stream)
	TF_AAC_ADTS			= 3,	

	// ADIF (Audio Data Interchange Format)
	TF_AAC_ADIF			= 4,	// not supported

	TF_UNKNOWN			= 0x7FFFFFFF	// Unknown format
}TransportFormat;

void* latm_parser_init(unsigned char *pucPacketDataPtr, 
  							   unsigned int iDataLength, 
  							   int *piSamplingFrequency, 
  							   int *piChannelNumber, 
  							   void *pCallback,
  							   TransportFormat eFormat);

int latm_parser_get_frame(void *pLatmDmxHandle,
									unsigned char *pucPacketDataPtr, int iStreamLength,
									unsigned char **pucAACRawDataPtr, int *piAACRawDataLength,
									unsigned int uiInitFlag);

int latm_parser_close(void *pLatmDmxHandle);

int latm_parser_get_header_type(void *pLatmDmxHandle);


//
void* Latm_Init(unsigned char *pucPacketDataPtr,
					unsigned int piDataLength, 
					int *piSamplingFrequency, 
					int *piChannelNumber, 
					void *psCallback, 
					TransportFormat eFormat)
{
	return latm_parser_init(pucPacketDataPtr, piDataLength, piSamplingFrequency, piChannelNumber, psCallback, eFormat);
}

int Latm_Close(void *pLatm)
{
	return latm_parser_close(pLatm);
}

int Latm_GetFrame(void *pLatmHandle, 
						unsigned char *pucPacketDataPtr,
						int iStreamLength,
						unsigned char **pucAACRawDataPtr,
						int *piAACRawDataLength,
						unsigned int uiInitFlag)
{
	return latm_parser_get_frame(pLatmHandle, pucPacketDataPtr, iStreamLength, pucAACRawDataPtr, piAACRawDataLength, uiInitFlag);
}

int Latm_GetHeaderType(void *pLatmHandle)
{
	return latm_parser_get_header_type( pLatmHandle );
}

