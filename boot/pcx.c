/*
 * pcx.c
 *
 * PCX image support for fusion boot loader
 * (c) 1999 Jimmy Larsson
 *
 */

#include "vga.h"
#include "pcx.h"
#include "printk.h"

/*
 * decode a pcx header
 *
 */

static void pcx_header_decode (void *pcx_data, pcx_header_t *hdr)
{
  char   *data = (char *) pcx_data;
  int     i;

  hdr->manufacturer = data[0];
  hdr->version = data[1];
  hdr->encoding = data[2];
  hdr->bits_per_pixel = data[3];

  hdr->x_min = data[4] + 256 * data[5];
  hdr->y_min = data[6] + 256 * data[7];
  hdr->x_max = data[8] + 256 * data[9];
  hdr->y_max = data[10] + 256 * data[11];

  hdr->h_dpi = data[12] + 256 * data[13];
  hdr->v_dpi = data[14] + 256 * data[15];
  
  for (i = 0; i < 48; i++)
    hdr->color_map[i] = data[16+i];

  hdr->reserved = data[64];
  hdr->n_planes = data[65];
  hdr->bytes_per_line = data[66] + 256 * data[67];
  hdr->palette_info = data[68] + 256 * data[69];
  hdr->h_screen_size = data[70] + 256 * data[71];
  hdr->v_screen_size = data[72] + 256 * data[73];
}

/*
 * convert palette to VGA format (RGB from 0-63)
 *
 * modifies and return "pcx_palette"
 */

char *convert_palette (char *pcx_palette, int num_cols)
{
  int i;

  for (i = 0; i < num_cols * 3; i++) {
    pcx_palette[i] = pcx_palette[i] >> 2;
  }

  return pcx_palette;
}

/*
 * decode a pcx image, put raw image into raw_dest
 *
 */

void pcx_draw_image (void *pcx_data, void *decode_buffer, int start_x, int start_y)
{
  pcx_header_t   pcx_hdr;
  int            width, height, line_bytes;
  int            decoded_size, i, j;
  char          *src_ptr;
  char          *dst_ptr = (char *) decode_buffer;

  pcx_header_decode (pcx_data, &pcx_hdr);

  /* Get image size */
  width = pcx_hdr.x_max - pcx_hdr.x_min + 1;
  height = pcx_hdr.y_max - pcx_hdr.y_min + 1;

  /* get total bytes for one scanline */
  line_bytes = pcx_hdr.n_planes * pcx_hdr.bytes_per_line;

  src_ptr = (char *) pcx_data + sizeof (pcx_header_t);
  decoded_size = width * height; // WAS: decoded_size = line_bytes * height;

  for (i = 0; i < decoded_size;) {
    
    /* is it a count byte? */
    if (*src_ptr & 0xc0) {
      int count = *src_ptr++ & 0x3f;

      for (j = 0; j < count; j++) {
	*dst_ptr++ = *src_ptr;
	i++;
      }
      src_ptr++;

    } else {
      *dst_ptr++ = *src_ptr++;
      i++;
    }
  }

  //Is the palette located after data?
  if (pcx_hdr.version >= 5 && *src_ptr++ == 12) {

    for (i = 0; i < 16; i++) {

      pcx_hdr.color_map[i * 3] = *src_ptr++;
      pcx_hdr.color_map[i * 3 + 1] = *src_ptr++;
      pcx_hdr.color_map[i * 3 + 2] = *src_ptr++;

      //
      //printf ("PCX palette %u: %u, %u, %u\n",
      //      i,
      //      pcx_hdr.color_map[i * 3],
      //      pcx_hdr.color_map[i * 3 + 1],
      //      pcx_hdr.color_map[i * 3 + 2]);
      //
    }

  }

  vga_set_palette (convert_palette (pcx_hdr.color_map, 16), 16);

  //Paint the picture
  vga_put_block (start_x, start_y, width, height, decode_buffer);
}

