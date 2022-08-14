/******************************************************************************
*
*  (C)Copyright All Rights Reserved by Telechips Inc.
*
*  This material is confidential and shall remain as such.
*  Any unauthorized use, distribution, reproduction is strictly prohibited.
*
*******************************************************************************
*
*  File Name   : vioc_scaler.c
*
*  Description : VIOC Scaler Component Control Module
*
*******************************************************************************
*
*  yyyy/mm/dd     ver            descriptions                Author
*	---------	--------   ---------------       -----------------
*    2011/08/16     0.1            created                      hskim
*******************************************************************************/

#include "vioc_scaler.h"

void VIOC_SC_SetBypass(PVIOC_SC pSCALER, unsigned int nOnOff)
{
	//pSCALER->uCTRL.bREG.BP  = nOnOff;
	BITCSET(pSCALER->uCTRL.nREG, 0x1, nOnOff);
}

void VIOC_SC_SetUpdate(PVIOC_SC pSCALER)
{
	//pSCALER->uCTRL.bREG.UPD  = 1;
	BITCSET(pSCALER->uCTRL.nREG, (0x1<<16), (0x1<<16));
}

void VIOC_SC_SetSrcSize(PVIOC_SC pSCALER, unsigned int nWidth, unsigned int nHeight)
{
	//pSCALER->uSRCSIZE.bREG.WIDTH    = nWidth;
	//pSCALER->uSRCSIZE.bREG.HEIGHT   = nHeight;
	BITCSET(pSCALER->uSRCSIZE.nREG, (0x1FFF<<16), (nHeight<<16));
	BITCSET(pSCALER->uSRCSIZE.nREG, 0x1FFF, nWidth);
}

void VIOC_SC_SetDstSize(PVIOC_SC pSCALER, unsigned int nWidth, unsigned int nHeight)
{
	//pSCALER->uDSTSIZE.bREG.WIDTH    = nWidth;
	//pSCALER->uDSTSIZE.bREG.HEIGHT   = nHeight;
	BITCSET(pSCALER->uDSTSIZE.nREG, (0x1FFF<<16), (nHeight<<16));
	BITCSET(pSCALER->uDSTSIZE.nREG, 0x1FFF, nWidth);
}

void VIOC_SC_SetOutSize(PVIOC_SC pSCALER, unsigned int nWidth, unsigned int nHeight)
{
	//pSCALER->uOUTSIZE.bREG.WIDTH    = nWidth;
	//pSCALER->uOUTSIZE.bREG.HEIGHT   = nHeight;
	BITCSET(pSCALER->uOUTSIZE.nREG, (0x1FFF<<16), (nHeight<<16));
	BITCSET(pSCALER->uOUTSIZE.nREG, 0x1FFF, nWidth);
}

void VIOC_SC_SetOutPosition(PVIOC_SC pSCALER, unsigned int nXpos, unsigned int nYpos)
{
	//pSCALER->uOUTPOS.bREG.XPOS  = nXpos;
	//pSCALER->uOUTPOS.bREG.YPOS  = nYpos;
	BITCSET(pSCALER->uOUTPOS.nREG, (0x1FFF<<16), (nYpos<<16));
	BITCSET(pSCALER->uOUTPOS.nREG, 0x1FFF, nXpos);
}

