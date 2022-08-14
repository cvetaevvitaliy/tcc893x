
//
// Filename     : bip.c
// Description  : 
// Author       : shyfool
// Created On   : Fri Aug 21 15:43:01 2009
// $ID$
//
// 
#include <stdio.h>

#include "tcc_resizer.h"
#include "utils/Log.h"
extern pixel_t bilinear_ip (frame_op_t *src, int ci, int cj);

pixel_t
bilinear_ip (frame_op_t *src, 
			 int ci,
			 int cj)
{
  unsigned int xi, xj;
  unsigned int p0, p1, p2, p3, p;
  unsigned int ph0, ph1;
  unsigned int i_frac, j_frac;
  pixel_t ip;


  j_frac = cj & 0xff;
  i_frac = ci & 0xff;
  xj = cj >> 8;
  xi = ci >> 8;

  p0 = fop_get_pixel (src,
					  xj,
					  xi);
  p1 = fop_get_pixel (src,
					  xj+1,
					  xi);
  p2 = fop_get_pixel (src,
					  xj, 
					  xi+1);
  p3 = fop_get_pixel (src,
					  xj+1,
					  xi+1);
  ph0 = (p0*(256-j_frac) + p1 * j_frac);
  ph1 = (p2*(256-j_frac) + p3 * j_frac);
  p = ((ph0 * (256-i_frac) + ph1*i_frac) + 65535) >> 16;

  ip = (p  >= 256) ? 255: p;

  return ip;

}

unsigned int
bilinear_ip_bmp (frame_op_t *src, frame_op_t *dst,
			 int ci,
			 int cj)
{
  unsigned int xi, xj;
  unsigned int p0, p1, p2, p3, p;
  unsigned int ph0, ph1;
  unsigned int i_frac, j_frac;
  unsigned int ip;
  unsigned int rgb_p0, rgb_p1, rgb_p2, rgb_p3;
  unsigned int rgb_ip;
  unsigned int i;

  j_frac = cj & 0xff;
  i_frac = ci & 0xff;
  xj = cj >> 8;
  xi = ci >> 8;

  rgb_p0 = fop_get_pixel_bmp (src, TCC_MAX_XY(xj+0,src->window_w-1), TCC_MAX_XY(xi+0,src->window_h-1));
  rgb_p1 = fop_get_pixel_bmp (src, TCC_MAX_XY(xj+1,src->window_w-1), TCC_MAX_XY(xi+0,src->window_h-1));
  rgb_p2 = fop_get_pixel_bmp (src, TCC_MAX_XY(xj+0,src->window_w-1), TCC_MAX_XY(xi+1,src->window_h-1));
  rgb_p3 = fop_get_pixel_bmp (src, TCC_MAX_XY(xj+1,src->window_w-1), TCC_MAX_XY(xi+1,src->window_h-1));
  rgb_ip = 0;

  for ( i = 0 ; i < 4 ; i++ ) {			// 0 for B, 1 for G, 2 for R
    p0 = ((rgb_p0>>(src->shift[i])<<src->shift1[i]) & src->mask[i]);	//fop_get_pixel_bmp (src, xj, xi, idx);
    p1 = ((rgb_p1>>(src->shift[i])<<src->shift1[i]) & src->mask[i]);	//fop_get_pixel_bmp (src, xj+1, xi, idx);
    p2 = ((rgb_p2>>(src->shift[i])<<src->shift1[i]) & src->mask[i]);	//fop_get_pixel_bmp (src, xj, xi+1, idx);
    p3 = ((rgb_p3>>(src->shift[i])<<src->shift1[i]) & src->mask[i]);	//fop_get_pixel_bmp (src, xj+1, xi+1, idx);
    ph0 = (p0*(256-j_frac) + p1 * j_frac);
    ph1 = (p2*(256-j_frac) + p3 * j_frac);
    p = ((ph0 * (256-i_frac) + ph1*i_frac) + 65535) >> 16;
    
    ip = (p  >= 256) ? 255: p;
	//rgb_ip |= (ip<<(i*shift[i]));
	rgb_ip |= (((ip&dst->mask[i])>>dst->shift1[i])<<(dst->shift[i]));
  }

  return rgb_ip;
}


static int 
run_interpolation (resize_op_t *sop,
				   frame_op_t *src,
				   frame_op_t *dst)
{
  unsigned int i, j;
  unsigned int yi, yj;

  pixel_t p;

  for (yi = 0, i = 0; i < dst->window_h; i++)
	{
	  for (yj = 0, j = 0; j < dst->window_w; j++)
		{
		  p = bilinear_ip (src, yi, yj);
		  fop_set_pixel (dst, j, i, p);
		  yj = yj + sop->winw;
		}
	  yi = yi + sop->winh;
	}
  return 0;
}

int
bilinear (frame_op_t src[3],
		  frame_op_t dst[3])
{
  int i;
  resize_op_t sop;
  unsigned int sw, sh, dw, dh;

  for (i = 0; i < 3; i++)
	{
	  fop_get_window_size (&src[i], &sw, &sh);
	  fop_get_window_size (&dst[i], &dw, &dh);

	  sop.hratio = (dw*256)/sw;
	  sop.vratio = (dh*256)/sh;
	  sop.ratio = (sop.hratio * sop.vratio);
	  sop.winw = sw*256/dw;
	  sop.winh = sh*256/dh;
	  sop.sr = sop.ratio;

	  run_interpolation (&sop, &src[i], &dst[i]);
	}
  return 0;
}

static int 
TCC_ResizerInterpolator (resize_op_t *sop,
				   frame_op_t *src,
				   frame_op_t *dst)
{
  unsigned int i, j;
  unsigned int yi, yj;

  unsigned int p;
  for (yi = 0, i = 0; i < dst->window_h; i++)
	{
	  for (yj = 0, j = 0; j < dst->window_w; j++)
		{
		  p = bilinear_ip_bmp (src, dst, yi, yj);
		  fop_set_pixel_bmp (dst, j, i, p);
		  fop_set_pixel_bmp (dst, j, i, p);
		  yj = yj + sop->winw;
		}
	  yi = yi + sop->winh;
	}
  return 0;
}

void set_resizer_src(frame_op_t *src,unsigned char*src_buf,unsigned int fw, unsigned int fh,unsigned int ww,unsigned int wh,unsigned int srcfmt)
{
	src->img= src_buf;
	src->frame_w= fw;
	src->frame_h= fh;
	src->window_w= ww;	
	src->window_h= wh;	

	switch(srcfmt)
	{
		case IMG_RGB888:
			src->bpp = 3;
			src->fmt = IMG_RGB888;
			src->shift[0] = 0;
			src->shift[1] = 8;
			src->shift[2] = 16;
			src->shift[3] = 24;
			src->mask[0] = 0xff;
			src->mask[1] = 0xff;
			src->mask[2] = 0xff;
			src->mask[3] = 0x00;
			src->shift1[0] = 0;
			src->shift1[1] = 0;
			src->shift1[2] = 0;
			src->shift1[3] = 0;
		break;
		
		case IMG_ARGB8888:  
		{
			src->bpp = 4;
			src->fmt = IMG_ARGB8888;
			src->shift[0] = 0;
			src->shift[1] = 8;
			src->shift[2] = 16;
			src->shift[3] = 24;
			src->mask[0] = 0xff;
			src->mask[1] = 0xff;
			src->mask[2] = 0xff;
			src->mask[3] = 0xff;
			src->shift1[0] = 0;
			src->shift1[1] = 0;
			src->shift1[2] = 0;
			src->shift1[3] = 0;
		}

		case IMG_DRGB888:  
			src->bpp = 4;
			src->fmt = IMG_DRGB888;
			src->shift[0] = 0;
			src->shift[1] = 8;
			src->shift[2] = 16;
			src->shift[3] = 24;
			src->mask[0] = 0xff;
			src->mask[1] = 0xff;
			src->mask[2] = 0xff;
			src->mask[3] = 0x00;
			src->shift1[0] = 0;
			src->shift1[1] = 0;
			src->shift1[2] = 0;
			src->shift1[3] = 0;
		break;
		
		case IMG_RGB565:  
			src->bpp = 2;
			src->fmt = IMG_RGB565;
			src->shift[0] = 0;
			src->shift[1] = 5;
			src->shift[2] = 11;
			src->shift[3] = 16;
			src->mask[0] = 0xf8;
			src->mask[1] = 0xfc;
			src->mask[2] = 0xf8;
			src->mask[3] = 0x00;
			src->shift1[0] = 3;
			src->shift1[1] = 2;
			src->shift1[2] = 3;
			src->shift1[3] = 0;
		break;
		
		case IMG_ARGB1555:
			src->bpp = 2;
			src->fmt = IMG_ARGB1555;
			src->shift[0] = 0;
			src->shift[1] = 5;
			src->shift[2] = 10;
			src->shift[3] = 15;
			src->mask[0] = 0xf8;
			src->mask[1] = 0xf8;
			src->mask[2] = 0xf8;
			src->mask[3] = 0x80;
			src->shift1[0] = 3;
			src->shift1[1] = 3;
			src->shift1[2] = 3;
			src->shift1[3] = 7;
		break;

		case IMG_ARGB4444: 
			src->bpp = 2;
			src->fmt = IMG_ARGB4444;
			src->shift[0] = 0;
			src->shift[1] = 4;
			src->shift[2] = 8;
			src->shift[3] = 12;
			src->mask[0] = 0xf0;
			src->mask[1] = 0xf0;
			src->mask[2] = 0xf0;
			src->mask[3] = 0xf0;
			src->shift1[0] = 4;
			src->shift1[1] = 4;
			src->shift1[2] = 4;
			src->shift1[3] = 4;
		break;
	}
}

void set_resizer_dst(frame_op_t *dst,unsigned char*dst_buf,unsigned int fw, unsigned int fh, unsigned int ww,unsigned int wh,unsigned int dstfmt)
{

	dst->img= dst_buf;
	dst->frame_w= fw;
	dst->frame_h= fh;
	dst->window_w= ww;	
	dst->window_h= wh;	

	switch(dstfmt)
	{
		case IMG_RGB888:
			dst->bpp = 3;
			dst->fmt = IMG_RGB888;
			dst->shift[0] = 0;
			dst->shift[1] = 8;
			dst->shift[2] = 16;
			dst->shift[3] = 24;
			dst->mask[0] = 0xff;
			dst->mask[1] = 0xff;
			dst->mask[2] = 0xff;
			dst->mask[3] = 0x00;
			dst->shift1[0] = 0;
			dst->shift1[1] = 0;
			dst->shift1[2] = 0;
			dst->shift1[3] = 0;
		break;
		
		case IMG_ARGB8888:  
		{
			dst->bpp = 4;
			dst->fmt = IMG_ARGB8888;
			dst->shift[0] = 0;
			dst->shift[1] = 8;
			dst->shift[2] = 16;
			dst->shift[3] = 24;
			dst->mask[0] = 0xff;
			dst->mask[1] = 0xff;
			dst->mask[2] = 0xff;
			dst->mask[3] = 0xff;
			dst->shift1[0] = 0;
			dst->shift1[1] = 0;
			dst->shift1[2] = 0;
			dst->shift1[3] = 0;
		}

		case IMG_DRGB888:  
			dst->bpp = 4;
			dst->fmt = IMG_DRGB888;
			dst->shift[0] = 0;
			dst->shift[1] = 8;
			dst->shift[2] = 16;
			dst->shift[3] = 24;
			dst->mask[0] = 0xff;
			dst->mask[1] = 0xff;
			dst->mask[2] = 0xff;
			dst->mask[3] = 0x00;
			dst->shift1[0] = 0;
			dst->shift1[1] = 0;
			dst->shift1[2] = 0;
			dst->shift1[3] = 0;
		break;
		
		case IMG_RGB565:  
			dst->bpp = 2;
			dst->fmt = IMG_RGB565;
			dst->shift[0] = 0;
			dst->shift[1] = 5;
			dst->shift[2] = 11;
			dst->shift[3] = 16;
			dst->mask[0] = 0xf8;
			dst->mask[1] = 0xfc;
			dst->mask[2] = 0xf8;
			dst->mask[3] = 0x00;
			dst->shift1[0] = 3;
			dst->shift1[1] = 2;
			dst->shift1[2] = 3;
			dst->shift1[3] = 0;
		break;
		
		case IMG_ARGB1555:
			dst->bpp = 2;
			dst->fmt = IMG_ARGB1555;
			dst->shift[0] = 0;
			dst->shift[1] = 5;
			dst->shift[2] = 10;
			dst->shift[3] = 15;
			dst->mask[0] = 0xf8;
			dst->mask[1] = 0xf8;
			dst->mask[2] = 0xf8;
			dst->mask[3] = 0x80;
			dst->shift1[0] = 3;
			dst->shift1[1] = 3;
			dst->shift1[2] = 3;
			dst->shift1[3] = 7;
		break;

		case IMG_ARGB4444: 
			dst->bpp = 2;
			dst->fmt = IMG_ARGB4444;
			dst->shift[0] = 0;
			dst->shift[1] = 4;
			dst->shift[2] = 8;
			dst->shift[3] = 12;
			dst->mask[0] = 0xf0;
			dst->mask[1] = 0xf0;
			dst->mask[2] = 0xf0;
			dst->mask[3] = 0xf0;
			dst->shift1[0] = 4;
			dst->shift1[1] = 4;
			dst->shift1[2] = 4;
			dst->shift1[3] = 4;
		break;
	}
}

int
TCC_Resizer (frame_op_t *src,
		      frame_op_t *dst)
{
  int i;
  resize_op_t sop;
  unsigned int sw, sh, dw, dh;

  fop_get_window_size (src, &sw, &sh);
  fop_get_window_size (dst, &dw, &dh);

  sop.hratio = (dw*256)/sw;
  sop.vratio = (dh*256)/sh;
  sop.ratio = (sop.hratio * sop.vratio);
  sop.winw = sw*256/dw;
  sop.winh = sh*256/dh;
  sop.sr = sop.ratio;

#if 0
  LOGE ("srcw    : %d\n",sw);
  LOGE ("srch    : %d\n",sh);
  LOGE ("dstw   : %d\n",dw);
  LOGE ("dsth   : %d\n",dh);
  LOGE ("hratio : %d\n",sop.hratio);
  LOGE ("vratio : %d\n",sop.vratio);
  LOGE ("ratio  : %d\n",sop.ratio);
  LOGE ("winw   : %d\n",sop.winw);
  LOGE ("winh   : %d\n",sop.winh);
  LOGE ("sr     : %d\n",sop.sr);
#endif

  TCC_ResizerInterpolator (&sop, src, dst);
  return 0;
}

// 
// $Log$
// 
