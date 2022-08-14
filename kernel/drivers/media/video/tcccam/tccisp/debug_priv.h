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


#ifndef _DBG_PRIV_H
#define _DBG_PRIV_H

// filter settings, change this to turn on output (1 = turn on)
#define DBG_FILTER_LEVEL  ( DBG_INFO | DBG_WARN | DBG_ERR )
#define DBG_FILTER_MODULE 0xffffff00


#endif //_DBG_PRIV_H
