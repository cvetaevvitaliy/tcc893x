/****************************************************************************
FileName    : kernel/drivers/video/tcc/vioc/tcc_component_cs4954.h
Description : 

Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

#ifndef __TCC_COMPONENT_CS4954_H__
#define __TCC_COMPONENT_CS4954_H__


#define CS4954_I2C_ADDR			0x08
#define CS4954_I2C_DEVICE		(CS4954_I2C_ADDR>>1)

extern int cs4954_i2c_write(unsigned char reg, unsigned char val);
extern int cs4954_i2c_read(unsigned char reg, unsigned char *val);
extern void cs4954_reset(void);
extern int cs4954_set_ready(void);
extern void cs4954_set_end(void);
extern void cs4954_set_mode(int type);
extern void cs4954_enable(int type);

#endif //__TCC_COMPONENT_CS4954_H__

