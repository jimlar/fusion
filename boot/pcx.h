/*
 * pcx.h
 *
 * PCX image support for fusion boot loader
 * (c) 1999 Jimmy Larsson
 *
 */

#ifndef PCX_H
#define PCX_H

typedef struct
{
  unsigned char   manufacturer;
  unsigned char   version;
  unsigned char   encoding;
  unsigned char   bits_per_pixel;

  unsigned short   x_min;
  unsigned short   y_min;
  unsigned short   x_max;
  unsigned short   y_max;

  unsigned short  h_dpi;
  unsigned short  v_dpi;
  unsigned char   color_map[48];
  unsigned char   reserved;
  unsigned char   n_planes;
  unsigned short  bytes_per_line;
  unsigned short  palette_info;
  unsigned short  h_screen_size;
  unsigned short  v_screen_size;
  unsigned char   filler[54];
} pcx_header_t;


/*
 * decode a pcx image, put raw image into raw_dest
 *
 */

void pcx_draw_image (void *pcx_data, void *decode_buffer, int start_x, int start_y);


/*
 * convert palette to vgs format (RGB from 0-63)
 *
 * modifies and return "pcx_palette"
 */

char *convert_palette (char *pcx_palette, int num_cols);


#endif
