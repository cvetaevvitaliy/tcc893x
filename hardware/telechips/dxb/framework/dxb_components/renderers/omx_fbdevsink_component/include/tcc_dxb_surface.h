/**
  OpenMAX FBDEV sink component. This component is a video sink that copies
  data to a >inux framebuffer device.

  Originally developed by Peter Littlefield
  Copyright (C) 2007-2008  STMicroelectronics and Agere Systems
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

#ifndef _TCC_DXB_SURFACE_H_
#define _TCC_DXB_SURFACE_H_

#include <gui/Surface.h>

typedef struct
{
	int frame_addr[3]; // yuv
	int frame_width;
	int frame_height;
	int frame_rate;
	int field_info; // 0: top, 1: bottom
	int unique_id;
	int pts;
	int stc;
	int interlace; // 0: interlace, 1: non-interlace
	int vsync_enable; // 0: disable, 1:enable
	int unique_addr;
	int display_index;
	int vpu_index;
	int format;
} DISP_INFO, *PDISP_INFO;

class TCCDxBSurface
{
public:
	TCCDxBSurface(int flag);
	~TCCDxBSurface();

	int SetVideoSurface(ANativeWindow *native);
	int UseSurface(void);
	int ReleaseSurface(void);
	int SetSurfaceParm(int width, int height, int format, int flags, int outmode);
	int WriteFrameBuf(PDISP_INFO pDispInfo);
	int CaptureVideoFrame(char *strFilePath);

private:
	int SetNativeWindow();
	int SetSurfaceParm_l(int width, int height, int format, int flags, int outmode);
	int WriteFrameBuf();
	void *GetPrivateAddr(int fd_val, int width, int height);

	int mWidth;
	int mHeight;
	int mFormat;
	int mFlags;
	int mOutMode;
	DISP_INFO mDispInfo;

	gralloc_module_t const *m_grallocModule;
	int                     mTmem_fd;
	void*                   mTMapInfo;
	pmap_t                  mUmpReservedPmap;

	int            mbUseOnlyMali;
	int            mValidSurface;
	ANativeWindow* mNativeWindow;
	pthread_mutex_t m_mutex;
};

#endif//_TCC_DXB_SURFACE_H_
