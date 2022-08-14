
#include "adec.h"

int Audio_Proc( int iOpCode, int* pHandle, void* pParam1, void* pParam2 )
{
	return TCC_DDP_DEC(iOpCode, pHandle, pParam1, pParam2);
}
