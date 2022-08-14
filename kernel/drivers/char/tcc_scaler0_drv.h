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


#define 	TRUE 		1
#define 	FALSE 		0

long tccxxx_scaler_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int tccxxx_scaler0_release(struct inode *inode, struct file *filp);
int tccxxx_scaler0_open(struct inode *inode, struct file *filp);
extern void test_scaler0_init(void);

#if defined(TEST_FEATURE)
extern void test_scaler0_init(void);
#endif


