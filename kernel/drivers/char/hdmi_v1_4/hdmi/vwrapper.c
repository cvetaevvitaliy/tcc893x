/****************************************************************************
 * FileName    : kernel/drivers/char/hdmi_v1_3/hdmi/vwrapper.c
 * Description : hdmi driver
 *
 * Copyright (C) 2013 Telechips Inc.
 *
 * This program is free software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
 * ****************************************************************************/

#include <asm/io.h>

#include "regs-vwrapper.h"
#include "vwrapper.h"

extern void __iomem *regs_vdwr;

/**
 * Enable/Disable video wapper
 * 
 * @param   enable   [in]  1 to enable, 0 to disable
 */
void video_wrapper_enable(unsigned char enable)
{
    if (enable)
        writeb(0x01,VIDEO_WRAPPER_SYS_EN);
    else
        writeb(0x00,VIDEO_WRAPPER_SYS_EN);
}

/**
 * Set video timing parameters
 *
 * @param   hdmi_3d_format  [in]  3D Structure
 * @param   param           [in]  Video timing parameters
 */
void video_wrapper_set_mode(enum HDMI3DVideoStructure hdmi_3d_format, 
					struct device_video_params param)
{
	// Horizontal params
	if (hdmi_3d_format == HDMI_3D_SSF_FORMAT)
	{
		// H is twice longer
		// HBlank
		writel(param.HBlank*2, VIDEO_WRAPPER_H_BLANK);
		// HTotal
		writel(param.HTotal*2, VIDEO_WRAPPER_HV_LINE_1);
		// HSYNC = [0:31] // [HSYNC_START:HSYNC_END]
		writel( param.HFront*2 - 2 , VIDEO_WRAPPER_HSYNC_GEN);
	}
	else
	{
		// HBlank
		writel(param.HBlank, VIDEO_WRAPPER_H_BLANK);
		// HTotal
		writel(param.HTotal, VIDEO_WRAPPER_HV_LINE_1);
		// HSYNC = [0:31] // [HSYNC_START:HSYNC_END]
		writel( param.HFront - 2 , VIDEO_WRAPPER_HSYNC_GEN);
	}

	// Vertical params
	if (hdmi_3d_format == HDMI_3D_FP_FORMAT ||
		hdmi_3d_format == HDMI_3D_FA_FORMAT ||
		hdmi_3d_format == HDMI_3D_LA_FORMAT ||
		hdmi_3d_format == HDMI_3D_LD_FORMAT )
	{
		// VTotal is twice longer
		writel(param.VTotal*2, VIDEO_WRAPPER_HV_LINE_0);
	}
	else if (hdmi_3d_format == HDMI_3D_LDGFX_FORMAT )
	{
		// VTotal is 4 times longer
		writel(param.VTotal*4, VIDEO_WRAPPER_HV_LINE_0);
	}
	else
	{
		// VTotal
		writel(param.VTotal, VIDEO_WRAPPER_HV_LINE_0);
	}

	if ( hdmi_3d_format == HDMI_3D_LA_FORMAT )
	{
		// V is twice longer
		// VSync
		writel(param.VFront*2, VIDEO_WRAPPER_VSYNC_GEN);
	}
	else
	{
		// VSync
		writel(param.VFront, VIDEO_WRAPPER_VSYNC_GEN);
	}

	// HSync Pol
	writel(param.HPol, VIDEO_WRAPPER_SYNC_MODE);

	writel(V_PATTERN_PASS_TO_CORE,VIDEO_WRAPPER_V_PATTERN);

	writel(VCLK_SEL_TO_EXTERNAL,VIDEO_WRAPPER_VCLK_SEL);
}

