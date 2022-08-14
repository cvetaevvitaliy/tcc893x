/****************************************************************************
 *   FileName    : tcc317x_stream_process.c
 *   Description : Sample source for stream process
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall 
constitute any express or implied warranty of any kind, including without limitation, any warranty 
of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright 
or other third party intellectual property right. No warranty is made, express or implied, 
regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of 
or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************/

#define _PleaseInsertYourCode_

#include "tcc317x_stream_process.h"
#include "tcc317x_monitoring.h"
#include "tcpal_os.h"

#define TCC317X_STREAM_MAXSIZE  4096
#define MAX_USERINFO_NUM_PER_CHANNEL 6

#if defined (USE_TELECHIPS_TCC79_MUSE_NUCLEUS_ADAPT)
extern void Tcc317xMuseStreamResult (I32S _moduleIndex, I08U * _data, I32S _size, I32S _types, I32U _reservedParams1,
                                     I32U _reservedParams2);
#endif
extern I08U ServiceTypes[TCC317X_MAX][0x40];

Tcc317xStreamCtrl_t Tcc317xStreamCtrl[2];
I32S      EpHeaderLostCount[2];
I08U      Tcc317xFICMSCData[2][MAX_USERINFO_NUM_PER_CHANNEL][TCC317X_STREAM_MAXSIZE];

static I08U TDMBEP_Header_check (I08U * in);
static I32S Parse_TDMB_EPHEADER (I32S _moduleIndex, I08U * p);
static I32S Parse_TDMB_FIC_HEADER (I32S _moduleIndex, I08U * pFIC, I32S FICFieldSize);

static I32S TDMBEP_Process (I32S _moduleIndex, I08U * _data, I32S point, I32U * p_idx);
static I32U Tcc317xStreamParsingSub (I32S _moduleIndex, I08U * _data, I32S _size);

void Tcc317xStreamParserInit (I32S _moduleIndex)
{
    TcpalMemset (&Tcc317xStreamCtrl[_moduleIndex], 0x00, sizeof (Tcc317xStreamCtrl_t));
    EpHeaderLostCount[_moduleIndex] = 0;
}

void Tcc317xStreamParsing (I32S _moduleIndex, I08U * _data, I32S _size)
{
	I32S count = _size;
	I32U temp = 0;
	I32U index = 0;

	while( count> 0 ) {
		if(Tcc317xStreamCtrl[0].parserState==ENUM_FIND_OTHER_CHIP) {
			if(count>=(I32S)(Tcc317xStreamCtrl[_moduleIndex].OtherChipModuleSize)) {
				temp = Tcc317xStreamParsingSub (1, &_data[index], Tcc317xStreamCtrl[_moduleIndex].OtherChipModuleSize);
				count -= temp;
				index += temp;
				Tcc317xStreamCtrl[_moduleIndex].OtherChipModuleSize -= temp;
				if(Tcc317xStreamCtrl[_moduleIndex].OtherChipModuleSize == 0) {
					Tcc317xStreamCtrl[0].parserState = ENUM_FIND_EP_SYNC;
				}
			} else {
				temp = Tcc317xStreamParsingSub (1, &_data[index], count);
				count -= temp;
				index += temp;
				Tcc317xStreamCtrl[_moduleIndex].OtherChipModuleSize -= temp;
			}
		} else {
			temp = Tcc317xStreamParsingSub (_moduleIndex, &_data[index], count);
			count -= temp;
			index += temp;
		}
	}
}

I32U Tcc317xStreamParsingSub (I32S _moduleIndex, I08U * _data, I32S _size)
{
    /* all streams - 4byte align */
    I32U      i;
    I32U      point;
    I32U      size4;
    I32S	  tempSize;
    size4 = (_size >> 2);
    point = 0;

    for (i = 0; i < size4; i++)
    {
        point = (i << 2);

        switch (Tcc317xStreamCtrl[_moduleIndex].parserState)
        {
        case ENUM_FIND_EP_SYNC:
            TDMBEP_Process (_moduleIndex, _data, point, &i);
            break;

        case ENUM_FIND_OTHER_CHIP:            
            return point;
            break;

        case ENUM_STATUS_GET:
			tempSize = sizeof(Tcc317xStreamCtrl[_moduleIndex].statusData)-4;
			if(Tcc317xStreamCtrl[_moduleIndex].currDataSize<=tempSize) {
					   TcpalMemcpy (&Tcc317xStreamCtrl[_moduleIndex].statusData[Tcc317xStreamCtrl[_moduleIndex].currDataSize],
										&_data[point], 4);
			}
			Tcc317xStreamCtrl[_moduleIndex].currDataSize += 4;
			if (Tcc317xStreamCtrl[_moduleIndex].currDataSize >= Tcc317xStreamCtrl[_moduleIndex].dataSize)
			{
				/* update monitoring status */
				Tcc317xParseStatusData (_moduleIndex,
										&Tcc317xStreamCtrl[_moduleIndex].statusData[0],
										Tcc317xStreamCtrl[_moduleIndex].dataSize);
				Tcc317xStreamCtrl[_moduleIndex].currDataSize = 0;
				Tcc317xStreamCtrl[_moduleIndex].dataSize = 0;
				Tcc317xStreamCtrl[_moduleIndex].parserState = ENUM_FIND_EP_SYNC;
			}
			break;

        case ENUM_DATA_GET_S:
            {
                I32S      AvailSize;
                I32S      realsize;

                AvailSize = ((size4 - i) << 2);

                if (AvailSize >=
                    (Tcc317xStreamCtrl[_moduleIndex].dataSize - Tcc317xStreamCtrl[_moduleIndex].currDataSize))
                {
                    realsize =
                        (Tcc317xStreamCtrl[_moduleIndex].dataSize - Tcc317xStreamCtrl[_moduleIndex].currDataSize);
                    TcpalMemcpy (&Tcc317xFICMSCData[_moduleIndex][Tcc317xStreamCtrl[_moduleIndex].streamHeader.identifier][Tcc317xStreamCtrl[_moduleIndex].currDataSize],
                                     &_data[point], realsize);

                    if (Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType == 0x40)
                    {
                        /* FIC Data */
                        I32U      CRCResult;
                        Parse_TDMB_FIC_HEADER (_moduleIndex, &Tcc317xFICMSCData[_moduleIndex][Tcc317xStreamCtrl[_moduleIndex].streamHeader.identifier][0],
                                               Tcc317xStreamCtrl[_moduleIndex].dataSize);
                        CRCResult = Tcc317xStreamCtrl[_moduleIndex].ficCrcResult;
                        if (CRCResult)
                        {
                            /* It has CRC Error */
                            TcpalPrintLog ((I08S*)"#[%d]FIC Error[0x%x]\n", _moduleIndex, CRCResult);
                            Tcc317xStreamResult (_moduleIndex, &Tcc317xFICMSCData[_moduleIndex][Tcc317xStreamCtrl[_moduleIndex].streamHeader.identifier][0],
                                                 Tcc317xStreamCtrl[_moduleIndex].dataSize - _SIZE_FICHEADER_,
                                                 SRVTYPE_FIC_WITH_ERR, CRCResult, 0);
                        }
                        else
                        {
                            /* CRC Error Free */
                            Tcc317xStreamResult (_moduleIndex, &Tcc317xFICMSCData[_moduleIndex][Tcc317xStreamCtrl[_moduleIndex].streamHeader.identifier][0],
                                                 Tcc317xStreamCtrl[_moduleIndex].dataSize - _SIZE_FICHEADER_,
                                                 SRVTYPE_FIC, CRCResult, 0);
                        }
                    }
                    else if (Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType == 0x44)
                    {
                        /* dummy */
                    }
                    else if (Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType < 0x40)
                    {
                        /* MSC Data : DMB / DAB / DAB+ / DATA */
                        Tcc317xStreamResult (_moduleIndex, &Tcc317xFICMSCData[_moduleIndex][Tcc317xStreamCtrl[_moduleIndex].streamHeader.identifier][0],
                                             Tcc317xStreamCtrl[_moduleIndex].dataSize,
                                             ServiceTypes[_moduleIndex][Tcc317xStreamCtrl[_moduleIndex].streamHeader.identifier],
                                             Tcc317xStreamCtrl[_moduleIndex].streamHeader.identifier, 0);
                    }
                    else
                    {
                        /* unknown */
                        TcpalPrintErr ((I08S*)"#[%d] Unknown type data\n", _moduleIndex);
                    }

                    i += (realsize >> 2) - 1;
                    Tcc317xStreamCtrl[_moduleIndex].currDataSize = 0;
                    Tcc317xStreamCtrl[_moduleIndex].dataSize = 0;
                    Tcc317xStreamCtrl[_moduleIndex].parserState = ENUM_FIND_EP_SYNC;
                }
                else
                {
                    realsize = AvailSize;
                    if (Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType != 0x44)
                    {
                        TcpalMemcpy (&Tcc317xFICMSCData[_moduleIndex][Tcc317xStreamCtrl[_moduleIndex].streamHeader.identifier][Tcc317xStreamCtrl[_moduleIndex].currDataSize],
                                         &_data[point], realsize);
                    }
                    i += (realsize >> 2) - 1;
                    Tcc317xStreamCtrl[_moduleIndex].currDataSize += realsize;
                }
            }
            break;

        }
    }
    return _size;
}

static I08U TDMBEP_Header_check (I08U * in)
{
    I32U      i, k;
    I08U      p = 0;

    for (i = 0; i < 4; i++)
    {
        for (k = 0; k < 8; k++)
        {
            p = (p + ((in[i] >> k) & 1)) & 1;
        }
    }

    if (p == 0) {
        TcpalPrintErr ((I08S*)"# Odd parity error\n");
    }

    return p;
}

static I32S Parse_TDMB_EPHEADER (I32S _moduleIndex, I08U * p)
{
    Tcc317xStreamCtrl[_moduleIndex].streamHeader.epSyncByte = 0x5C;

    Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType = p[2];

    if (Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType < 0x40)
        Tcc317xStreamCtrl[_moduleIndex].streamHeader.identifier = p[2] & 0x3F;
    else
        Tcc317xStreamCtrl[_moduleIndex].streamHeader.identifier = 0x00;

    Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamSize = ((((p[1] << 8) | p[0]) >> 1) << 2);
    Tcc317xStreamCtrl[_moduleIndex].streamHeader.parity = (p[0] & 0x01);

    if (Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamSize >= 1024 * 4)
    {
        TcpalPrintErr ((I08S*)"# [E]EPDataSizeOver4k 0x%02x%02x%02x%02x\n", p[0], p[1], p[2], p[3]);
        return TCC317X_RETURN_FAIL;
    }
    return TCC317X_RETURN_SUCCESS;
}

static I32S Parse_TDMB_FIC_HEADER (I32S _moduleIndex, I08U * pFIC, I32S FICFieldSize)
{
    Tcc317xStreamCtrl[_moduleIndex].flagFig5_2 = ((pFIC[FICFieldSize - 2] >> 4) & 0x01);
    Tcc317xStreamCtrl[_moduleIndex].flagFig0_19 = ((pFIC[FICFieldSize - 2] >> 3) & 0x01);
    Tcc317xStreamCtrl[_moduleIndex].flagFig0_0 = ((pFIC[FICFieldSize - 2] >> 2) & 0x01);
    Tcc317xStreamCtrl[_moduleIndex].flagFig0_0Change = ((pFIC[FICFieldSize - 2] >> 1) & 0x01);
    Tcc317xStreamCtrl[_moduleIndex].flagFig0_0Alarm = (pFIC[FICFieldSize - 2] & 0x01);
    Tcc317xStreamCtrl[_moduleIndex].ficCrcResult =
        ((((pFIC[FICFieldSize - 3] << 8) | pFIC[FICFieldSize - 4]) >> 4) & 0xFFF);
    return TCC317X_RETURN_SUCCESS;
}

static I32S TDMBEP_Process (I32S _moduleIndex, I08U * _data, I32S point, I32U * p_idx)
{
    I32U      i = p_idx[0];

    if (_data[point + 3] == 0x5c && TDMBEP_Header_check (&_data[point]) == 1)
    {
        EpHeaderLostCount[_moduleIndex] = 0;

        Tcc317xStreamCtrl[_moduleIndex].streamHeader.epHeaderData[0] = _data[point];
        Tcc317xStreamCtrl[_moduleIndex].streamHeader.epHeaderData[1] = _data[point + 1];
        Tcc317xStreamCtrl[_moduleIndex].streamHeader.epHeaderData[2] = _data[point + 2];
        Tcc317xStreamCtrl[_moduleIndex].streamHeader.epHeaderData[3] = _data[point + 3];

        if (Parse_TDMB_EPHEADER (_moduleIndex, &Tcc317xStreamCtrl[_moduleIndex].streamHeader.epHeaderData[0])==TCC317X_RETURN_SUCCESS)
        {
            if (Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType == 0xFD)
            {
                /* Other Chip */
                Tcc317xStreamCtrl[_moduleIndex].parserState = ENUM_FIND_OTHER_CHIP;
                Tcc317xStreamCtrl[_moduleIndex].OtherChipModuleSize = Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamSize;
                /*
                TcpalPrintLog ((I08S*)"# Other Chip[%d]\n", Tcc317xStreamCtrl[_moduleIndex].dataSize);
                */
            }
            else if (Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType == 0xBF)
            {
                /* Status */
                Tcc317xStreamCtrl[_moduleIndex].parserState = ENUM_STATUS_GET;
                Tcc317xStreamCtrl[_moduleIndex].dataSize = Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamSize;
                Tcc317xStreamCtrl[_moduleIndex].currDataSize = 0x00;
            }
            else if (Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType == 0x40)
            {
                /* FIC */
                Tcc317xStreamCtrl[_moduleIndex].parserState = ENUM_DATA_GET_S;
                Tcc317xStreamCtrl[_moduleIndex].dataSize = Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamSize;
                Tcc317xStreamCtrl[_moduleIndex].currDataSize = 0x00;
            }
            else if (Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType == 0x44)
            {
                /* DummyData */
                Tcc317xStreamCtrl[_moduleIndex].parserState = ENUM_DATA_GET_S;
                Tcc317xStreamCtrl[_moduleIndex].dataSize = Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamSize;
                Tcc317xStreamCtrl[_moduleIndex].currDataSize = 0x00;
            }
            else if (Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType < 0x40)
            {
                /* MSC */
                Tcc317xStreamCtrl[_moduleIndex].parserState = ENUM_DATA_GET_S;
                Tcc317xStreamCtrl[_moduleIndex].dataSize = Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamSize;
                Tcc317xStreamCtrl[_moduleIndex].currDataSize = 0x00;
            }
            else
            {
                /* Error */
                TcpalPrintLog ((I08S*)"# [%d]EP Stream Type Unknown [0x%x]\n", _moduleIndex,
                               Tcc317xStreamCtrl[_moduleIndex].streamHeader.streamType);
            }
        }
        else
        {
            /* no find sync? && no syntax? */
            Tcc317xStreamCtrl[_moduleIndex].parserState = ENUM_FIND_EP_SYNC;    /* find first byte.. 0x15 (0xa80x) */
            TcpalPrintLog ((I08S*)"# EPHeaderError[0x%02x%02x%02x%02x]\n",
                           Tcc317xStreamCtrl[_moduleIndex].streamHeader.epHeaderData[0],
                           Tcc317xStreamCtrl[_moduleIndex].streamHeader.epHeaderData[1],
                           Tcc317xStreamCtrl[_moduleIndex].streamHeader.epHeaderData[2],
                           Tcc317xStreamCtrl[_moduleIndex].streamHeader.epHeaderData[3]);
        }
    }
    else if (_data[point] == 0 && _data[point + 1] == 0 && _data[point + 2] == 0 && _data[point + 3] == 0)
    {
        /* if STS mode. for align 188 */
    }
    else
    {
        if (EpHeaderLostCount[_moduleIndex] >= 48)
        {
            EpHeaderLostCount[_moduleIndex] = 0;
        }
        EpHeaderLostCount[_moduleIndex]++;

        if (EpHeaderLostCount[_moduleIndex] == 1) {
            TcpalPrintLog ((I08S*)"# [%d]EPHeader Lost [0x%02x%02x%02x%02x]\n",_moduleIndex,
                           _data[point + 0],_data[point + 1],_data[point + 2],_data[point + 3]);
        }
    }

    p_idx[0] = i;

    return TCC317X_RETURN_SUCCESS;
}

/* Final Stream Data */
void Tcc317xStreamResult (I32S _moduleIndex, I08U * _data, I32S _size, I32S _types, I32U _reservedParams1,
                          I32U _reservedParams2)
{
/* _reservedParams1 : FIC [CRC Error Flag] / MSC [Sub channel ID] */
/* _reservedParams2 : none */

    Tcc317xAndroidStreamResult (_moduleIndex, _data, _size, _types, _reservedParams1, _reservedParams2);
}

void Tcc317xAndroidStreamResult (I32S _moduleIndex, I08U *_data, I32S _size, I32S _types, I32U _reservedParams1, I32U _reservedParams2)
{
	I32U identifier;
	identifier = _reservedParams1;

	switch (_types)
	{
		case SRVTYPE_FIC:
		case SRVTYPE_FIC_WITH_ERR:	
		case EWS_ANNOUNCE_FLAG:
		case RECONFIGURATION_FLAG:	
		case EWS_ANNOUNCE_RECONFIGURATION_FLAG:			
			TCC317XTDMB_FICProcess (_moduleIndex, _data, _size, _reservedParams1, _types);			
			break;

		case SRVTYPE_DMB:
			TCC317XTDMB_AVServiceProcess (_moduleIndex, _data, _size, _types, identifier);
			break;

		case SRVTYPE_DAB:
		case SRVTYPE_DABPLUS:
			TCC317XTDMB_AVServiceProcess (_moduleIndex, _data, _size, _types, identifier);
			break;

		case SRVTYPE_DATA:
			TCC317XTDMB_DataServiceProcess(_moduleIndex, _data, _size, _types, identifier);
			break;

		default:
			break;
	}	
}

