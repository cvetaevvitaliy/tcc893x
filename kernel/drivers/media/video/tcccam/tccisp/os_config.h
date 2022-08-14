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


#ifndef _OS_CONFIG_H
#define _OS_CONFIG_H

///////////////////////////////////////////////////////////////////////////////
///  \def OS_EVENT
///  \brief  Include event handling in OS Abstraction Layer library.
#define OS_EVENT

///////////////////////////////////////////////////////////////////////////////
///  \def    OS_MUTEX
///  \brief  Include mutex handling in OS Abstraction Layer library.
#define OS_MUTEX

///////////////////////////////////////////////////////////////////////////////
///  \def    OS_SEMAPHORE
///  \brief  Include semaphore handling in OS Abstraction Layer library.
#define OS_SEMAPHORE

///////////////////////////////////////////////////////////////////////////////
///  \def    OS_QUEUE
///  \brief  Include queue handling in OS Abstraction Layer library.
#define OS_QUEUE

///////////////////////////////////////////////////////////////////////////////
///  \def    OS_THREAD
///  \brief  Include thread handling in OS Abstraction Layer library.
#define OS_THREAD

///////////////////////////////////////////////////////////////////////////////
///  \def    OS_TIMER
///  \brief  Include timer support in OS Abstraction Layer library.
#define OS_TIMER
#undef OS_TIMER // we want this to be documented by doxygen but not compiled in for now

///////////////////////////////////////////////////////////////////////////////
///  \def    OS_FILE
///  \brief  Include file handling in OS Abstraction Layer library.
#define OS_FILE

///////////////////////////////////////////////////////////////////////////////
///  \def    OS_FILE_WANT_ARMSEMI
///  \brief  Include ARM's semihosting in OS Abstraction Layer library.
#define OS_FILE_WANT_ARMSEMI
////#undef OS_FILE_WANT_ARMSEMI // we want this to be documented by doxygen but not compiled in for now

///////////////////////////////////////////////////////////////////////////////
///  \def    OS_FILE_WANT_TRIXI
///  \brief  Include TRIXI remote I/O in OS Abstraction Layer library.
#define OS_FILE_WANT_TRIXI
#undef OS_FILE_WANT_TRIXI // we want this to be documented by doxygen but not compiled in for now

///////////////////////////////////////////////////////////////////////////////
///  \def    OS_MISC
///  \brief  Include misc functionality in OS Abstraction Layer library.
#define OS_MISC

///////////////////////////////////////////////////////////////////////////////
///  \def    OS_DEBUG
///  \brief  Enable debug functionality in OS Abstraction Layer library.
#define OS_DEBUG

///////////////////////////////////////////////////////////////////////////////
///  \def    OS_HWDEP
///  \brief  Include Trillian hardware dependent functionality in OS Abstraction Layer library.
#define OS_HWDEP


#endif // _OS_CONFIG_H
