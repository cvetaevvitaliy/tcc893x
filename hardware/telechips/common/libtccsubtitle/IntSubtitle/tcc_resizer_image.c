
//
// Filename     : image.c
// Description  : 
// Author       : shyfool
// Created On   : Thu Jul 09 22:15:47 2009
// $ID$
//
// 
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "tcc_resizer.h"
#include "utils/Log.h"
/*
void
fop_create (frame_op_t *fop,
			unsigned int width,
			unsigned int height,
			unsigned int real_width,
			unsigned int real_height)
{
  assert (fop != 0);

  fop->img = TCC_malloc (sizeof(pixel_t)*real_width*real_height);
  fop->window_w = width;
  fop->window_h = height;
  fop->real_width = real_width;
  fop->real_height = real_height;
}
*/

void
fop_create_new (frame_op_t *fop)
{
  assert (fop != 0);

  switch (fop->fmt) {
	  case (IMG_RGB888) :
        fop->bpp = 3;
        fop->frame_size = fop->frame_w * fop->bpp * fop->frame_h;
		break;
	  case (IMG_ARGB8888) :
        fop->bpp = 4;
        fop->frame_size = fop->frame_w * fop->bpp * fop->frame_h;
		break;
	  case (IMG_DRGB888) :
        fop->bpp = 4;
        fop->frame_size = fop->frame_w * fop->bpp * fop->frame_h;
		break;
	  case (IMG_RGB565) :
	  case (IMG_ARGB1555) :
      case (IMG_ARGB4444) :
        fop->bpp = 2;
        fop->frame_size = fop->frame_w * fop->bpp * fop->frame_h;
		break;
	  default :
		fprintf (stderr, "error  : not supported format ..\n");
        break;
  }
  fop->img = calloc (1,sizeof(pixel_t)*fop->frame_size);
}

void
fop_delete (frame_op_t *fop)
{
  assert (fop != 0);
  free (fop->img);
}

void
fop_get_window_size (frame_op_t *fop,
			  int *width,
			  int *height)
{
  *width = fop->window_w;
  *height = fop->window_h;
}

void
fop_get_frame_size (frame_op_t *fop,
			  int *width,
			  int *height)
{
  *width = fop->frame_w;
  *height = fop->frame_h;
}

void
fop_set_window_size (frame_op_t *fop,
			  int width,
			  int height)
{
  fop->window_w = width;
  fop->window_h = height;
}

void
fop_set_start_pos (frame_op_t *fop,
			  int x,
			  int y)
{
  fop->start_x = x;
  fop->start_y = y;
}

void
fop_set_frame_size (frame_op_t *fop,
			  int width,
			  int height)
{
  fop->frame_w = width;
  fop->frame_h = height;
}

void
fop_set_format (frame_op_t *fop,
			  int fmt)
{
  fop->fmt = fmt;
}

pixel_t
fop_get_pixel (frame_op_t *fop, 
			   int x,
			   int y)
{
  int xx, yy;
  assert (fop);

  if (x < 0) xx = 0;
  else if (x >= fop->window_w) xx = fop->window_w -1;
  else xx = x;

  if (y < 0) yy = 0;
  else if (y >= fop->window_h) yy = fop->window_h -1;
  else yy = y;

  return *((fop->img) + (fop->window_w*yy) + xx);
}

unsigned int
fop_get_pixel_bmp (frame_op_t *fop, 
			   int x,
			   int y)
{
  int xx, yy;
  int	i;
  unsigned int ret;
  assert (fop);

  /*
  if (x < 0) {
	  //fprintf (stderr,"x is less than 0 ...\n");
	  xx = 0;
  }
  else 
  if (x >= fop->window_w) {
	  //fprintf (stderr,"x is greater equal max ...\n");
	  xx = fop->window_w -1;
  }
  else xx = x;

  if (y < 0) {
	  //fprintf (stderr,"y is less than 0 ...\n");
	  yy = 0;
  }
  else if (y >= fop->window_h) {
	  //fprintf (stderr,"y is greater equal max ...\n");
	  yy = fop->window_h -1;
  }
  else yy = y;
  */
  xx = x; yy = y;
  ret = 0;
  for ( i = 0 ; i < fop->bpp ; i++ ) {
    ret |= *(fop->img + (fop->frame_w*fop->bpp*(yy+fop->start_y)) + ((xx+fop->start_x)*fop->bpp) + i )<<(i*8);
  }
  return ret;
}

unsigned int
fop_get_pixel_bmp_frame (frame_op_t *fop, 
			   int x,
			   int y)
{
  int xx, yy;
  int	i;
  unsigned int ret;
  assert (fop);

  xx = x;
  yy = y;

  ret = 0;
  for ( i = 0 ; i < fop->bpp ; i++ ) {
    ret |= *(fop->img + (fop->frame_w*fop->bpp*(yy)) + ((xx)*fop->bpp) + i )<<(i*8);
  }
  return ret;
}

void
fop_set_pixel (frame_op_t *fop, 
			   int x,
			   int y,
			   pixel_t v)
{
  assert (fop);
  assert ((x < fop->window_w) && (y < fop->window_h));

  *((fop->img) + (fop->window_w*y) + x) = v;
}

void
fop_set_pixel_bmp (frame_op_t *fop, 
			   int x,
			   int y,
			   unsigned int v)
{
  int i;
  assert (fop);
  assert ((x < fop->window_w) && (y < fop->window_h));

  for ( i = 0 ; i < fop->bpp ; i++ ) {
    *((fop->img) + (fop->frame_w*fop->bpp*(y+fop->start_y)) + ((x+fop->start_x)*fop->bpp) + i) = v>>(i*8);
  }
}

void
fop_display (frame_op_t *fop)
{
  fprintf (stdout, "=================================\n");
  fprintf (stdout, "fmt          : %d\n",fop->fmt);
  fprintf (stdout, "bpp          : %d\n",fop->bpp);
  fprintf (stdout, "frame_w      : %d\n",fop->frame_w);
  fprintf (stdout, "frame_h      : %d\n",fop->frame_h);
  fprintf (stdout, "window_w     : %d\n",fop->window_w);
  fprintf (stdout, "window_h     : %d\n",fop->window_h);
  fprintf (stdout, "start_x      : %d\n",fop->start_x);
  fprintf (stdout, "start_y      : %d\n",fop->start_y);
  fprintf (stdout, "frame_size   : %d\n",fop->frame_size);
  fprintf (stdout, "shift        : (0x%x,0x%x,0x%x,0x%x)\n", fop->shift[0], fop->shift[1], fop->shift[2], fop->shift[3]);
  fprintf (stdout, "mask         : (0x%x,0x%x,0x%x,0x%x)\n", fop->mask[0], fop->mask[1], fop->mask[2], fop->mask[3]);
  fprintf (stdout, "shift1       : (0x%x,0x%x,0x%x,0x%x)\n", fop->shift1[0], fop->shift1[1], fop->shift1[2], fop->shift1[3]);
  fprintf (stdout, "=================================\n");
}


// 
// $Log$
// 
