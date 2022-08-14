#include "public_defs.h"            // uint8_t

#ifdef __cplusplus
extern "C" 
{
#endif

/*
*	For Framework in Android, These methods are C functions.
*/

/* hdcp2_Sink_Init
* 
*         It initializes Sink for HDCP. It has to be called once at the start, before any other functions. 
*        This function is blocking and uses the execution context of the caller to wait for network packets.
*        The callback will be invoked on a regular basis and can request (through a return value) the stack to stop running,
*        after which the blocking this function finally returns.
*        Other reasons for the function to return are communication errors.

* @ Param - addr -host address
* @ Param - port -unique port only for HDCP
* @ Return - int -The function returns 0 on success and a negative value(<0) when an error occurred.
*/
int hdcp2_Sink_Init(const char * addr, int port);

/* hdcp2_Sink_GetStatus
* 
*         This function shows current status of sink.
*
* @ Return - int -result of this method
*/
int hdcp2_Sink_GetStatus();

/* hdcp2_Sink_Process
* 
*         This function processes the specified amount of data, decrypting it. 
*        Note that a Protected Content Packet can be processed in multiple chunks.
*
* @ Param - inData -Pointer to the buffer that holds the data be decrypted including the header of protected content packet, in total dataSize bytes.
* @ Param - outData -.
* @ Param - dataSize - The number of bytes to process(decrypt)
* @ Param - streamCtr -.
* @ Param - inputCtr -.
* @ Return - int -The function returns 0 on success and a negative value(<0) when an error occurred.
*/
int hdcp2_Sink_Process(const uint8_t * inData, uint8_t * outData, uint32_t dataSize, uint32_t * streamCtr, uint64_t * inputCtr);

/* hdcp2_Sink_Close
* 
*         This function forces sink to stop it. It releases allocated resources.
*/
void hdcp2_Sink_Close();


#ifdef __cplusplus
}
#endif
