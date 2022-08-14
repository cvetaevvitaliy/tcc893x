
#include "TCC_WMV78_DEC.h"

int Video_Proc( int iOpCode, int* pHandle, void* pParam1, void* pParam2 )
{
	return TCC_WMV78_DEC(iOpCode, pHandle, pParam1, pParam2);
}
