/**
  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

#ifndef __TCC_VPU_ENC_INTERFACE_H__
#define __TCC_VPU_ENC_INTERFACE_H__

extern int TCC_VPU_Enc_Init(int bitstream_format, unsigned int uiWidth, unsigned int uiHeight, unsigned char quality);
extern int TCC_VPU_Enc_Deinit(void);
extern unsigned int TCC_VPU_Enc_Get_CFGData(unsigned char  *pBuffer, unsigned int *size, unsigned char isSPS);
extern int TCC_VPU_Encode(unsigned char *p_input, unsigned int input_len, unsigned char *p_output, unsigned int *output_len);

extern unsigned int TCC_VPU_Enc_Get_VirtualBuffer(void);
extern unsigned int TCC_VPU_Enc_Get_PhysicalBuffer(void);
extern void TCC_VPU_Enc_Inc_BufferIdx(void);
#endif//__TCC_VPU_ENC_INTERFACE_H__
