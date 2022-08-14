
//
// Filename     : image.h
// Description  : 
// Author       : shyfool
// Created On   : Thu Jul 09 17:24:12 2009
// $ID$
//
#ifndef _TCC_RESIZER_H_
#define _TCC_RESIZER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define	TCC_MAX_XY(x,y) (((x)>(y))?(y):(x))

typedef unsigned char pixel_t;

typedef struct frame_op_t frame_op_t;

typedef enum {
  IMG_ARGB4444,
  IMG_ARGB1555,
  IMG_RGB565,
  IMG_ARGB8888,
  IMG_DRGB888,		// 32bit RGB : the bits [31:24] are ignored
  IMG_RGB888,
  IMG_MAX_FORMAT,
} imgfmt_t;

struct frame_op_t {
  pixel_t 	*img;
  int		fmt;					/* FORMAT */
  int		bpp;					/* byte(s) per pixel : 1,2,3,4 */
  int		frame_w;				/* pixel count toward horizontal axis */
  int		frame_h;				/* pixel count toward vertical axis */
  int		window_w;				/* pixel count toward horizontal axis */
  int		window_h;				/* pixel count toward vertical axis */
  int		start_x;				/* horizontal start point */
  int		start_y;				/* vertical start point */
  int		frame_size;				/* total byte count */
  int		shift[4];				/* variables for color format conversion */
  int		mask[4];				/* variables for color format conversion */
  int		shift1[4];				/* variables for color format conversion */
};

typedef struct {
  unsigned int hratio;			/* 24.8 fixed point */
  unsigned int vratio;			/* 24.8 fixed point */
  unsigned int ratio;			/* 24.8 fixed point */

  unsigned int winw;
  unsigned int winh;
  unsigned int sr;

} resize_op_t;

inline  void fop_create (frame_op_t *fop,
						 unsigned int width,
						 unsigned int height,
						 unsigned int real_width,
						 unsigned int real_height);
inline void fop_delete (frame_op_t *fop);
inline void fop_get_size (frame_op_t *fop, int *width, int *height);
inline pixel_t fop_get_pixel (frame_op_t *fop, int x, int y);
inline void fop_set_pixel (frame_op_t *fop, int x, int y, pixel_t v);

extern void set_resizer_src(frame_op_t *src,unsigned char*src_buf,unsigned int fw, 
	                     unsigned int fh,unsigned int ww,unsigned int wh,unsigned int srcfmt);

extern void set_resizer_dst(frame_op_t *dst,unsigned char*dst_buf,unsigned int fw, 
	                     unsigned int fh, unsigned int ww,unsigned int wh,unsigned int dstfmt);

extern int TCC_Resizer (frame_op_t *src, frame_op_t *dst);

#ifdef __cplusplus
}
#endif
#endif /* _IMAGE_H_ */
