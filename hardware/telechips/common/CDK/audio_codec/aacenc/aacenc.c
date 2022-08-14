
#include "aenc.h"

int Audio_Proc( int iOpCode, int* pHandle, void* pParam1, void* pParam2 )
{
	return TCC_AAC_ENC(iOpCode, pHandle, pParam1, pParam2);
}
