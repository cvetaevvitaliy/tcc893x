/****************************************************************************
One line to give the program's name and a brief idea of what it does.
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


#ifndef _VENUS_I2C_CTL_H_
#define _VENUS_I2C_CTL_H_


#include "venus_project.h"


/*************************************************************************************************/
/* I2C Interface                                                                                 */
/*************************************************************************************************/
void transmit_byte_via_i2c(uint16 addr, uint8 data);
void transmit_word_via_i2c(uint16 addr, uint16 data);
void transmit_multibytes_via_i2c(uint8* ptr, uint16 addr, uint16 length);

uint8 receive_byte_via_i2c(uint16 addr);
uint16 receive_word_via_i2c(uint16 addr);
void receive_multibytes_via_i2c(uint8* ptr, uint16 addr, uint16 length);
uint8 receive_byte_via_i2c_ISP(uint16 addr); // This api will use for CIF camera interface.
void transmit_byte_via_i2c_ISP(uint16 addr, uint8 data); // This api will use for CIF camera interface.
uint16 receive_word_via_i2c_ISP(uint16 addr);


#endif // _VENUS_I2C_CTL_H_


