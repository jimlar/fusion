/**
 * vga.c
 *
 * Boot console driver for FUSION
 *
 * (c) Jimmy Larsson 1999
 *
 */


#include "vga.h"
#include "port_io.h"
#include "types.h"


#define ABS(a) (a < 0 ? -a : a)

/* VGA register data for mode 0x12 */

static char mode12_regs[62] = 
{0xE3, 0x00,                    /* Misc and Status (2)            */
                                /* VGA_MISC_PORT, VGA_STATUS_PORT */
 0x03, 0x01, 0x0F, 0x00, 0x06,  /* Sequencer       (5)            */
                                /* VGA_SEQ_PORT                   */
 0x5F, 0x4F, 0x50, 0x82, 0x54,  /* CRTC            (25)           */ 
 0x80, 0x0B, 0x3E, 0x00, 0x40,  /* VGA_CRTC_PORT                  */ 
 0x00, 0x00, 0x00, 0x00, 0x00, 
 0x59, 0xEA, 0x8C, 0xDF, 0x28, 
 0x00, 0xE7, 0x04, 0xE3, 0xFF, 
 
 0x00, 0x00, 0x00, 0x00, 0x00,  /* Graphics        (9)  */  
 0x00, 0x05, 0x0F, 0xFF,        /* VGA_GRACON_PORT      */
 
 0x00, 0x01, 0x02, 0x03, 0x04,  /* Attribute       (21) */ 
 0x05, 0x06, 0x07, 0x08, 0x09,  /* VGA_ATTRCON_PORT     */
 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 
 0x0F, 0x01, 0x00, 0x0F, 0x00, 
 0x00};


static char mode12_palette[] = {0x00, 0x00, 0x00,
				0x11, 0x11, 0x11,
				0x22, 0x22, 0x22, 
				0x33, 0x33, 0x33,
				0x44, 0x44, 0x44,
				0x55, 0x55, 0x55,
				0x66, 0x66, 0x66,
				0x77, 0x77, 0x77,
				0x88, 0x88, 0x88,
				0x99, 0x99, 0x99,
				0xaa, 0xaa, 0xaa,
				0xbb, 0xbb, 0xbb,
				0xcc, 0xcc, 0xcc,
				0xdd, 0xdd, 0xdd,
				0xee, 0xee, 0xee,
				0xff, 0xff, 0xff};


/* video memory pointer */
static char   *video_mem = (char *) VGA_MEM_ADDR;

/* font data pointer */
static char   *font_data;

/* font metrics */
static int     font_width;
static int     font_height;

/* font cursors */
static int   cursor_x = 0;
static int   cursor_y = 10;

/* minimum y value */
static int   min_cursor_y = 0;

/* maximum y value */
static int   max_cursor_y = 25;

/* maximum x value */
static int   max_cursor_x = 80;

/*
 * vga_set_wrapline
 *  set current y value to be the minimum one
 */

void vga_set_wrapline (void)
{
  min_cursor_y = cursor_y;
}

/*
 * vga_init
 * Initialize the vga subsystem
 *
 */

void vga_init (loader_args_t   *args)
{
  cursor_x = args->cursor_x;
  cursor_y = args->cursor_y;
  font_height = args->font_height;
  font_width = args->font_width;
  font_data = args->font_data;

  max_cursor_y = VGA_Y_RES / font_height;
  max_cursor_x = VGA_X_RES / font_width;
}

/**
 * vga_set_mode
 * Initialize vga mode
 *
 */

void vga_set_mode (int mode)
{
  int i = 0;
  int status;

  /* Set misc and status regs */
  outb (mode12_regs[0], VGA_MISC_PORT);
  outb (mode12_regs[1], VGA_STATUS_PORT);
  
  /* Set sequencer regs */
  for (i = 2; i < 7; i++) {
    outb (i - 2, VGA_SEQ_PORT);
    outb (mode12_regs[i], VGA_SEQ_PORT + 1);
  }

  /* Clear protection bits */
  outw (0x0e11, VGA_CRTC_PORT);

  /* Set CRTC regs */
  for (i = 7; i < 32; i++) {
    outb (i - 7, VGA_CRTC_PORT);
    outb (mode12_regs[i], VGA_CRTC_PORT + 1);
  }

  /* Set graphics regs */
  for (i = 32; i < 41; i++) {
    outb (i - 32, VGA_GRACON_PORT);
    outb (mode12_regs[i], VGA_GRACON_PORT + 1);
  }

  status = inb (VGA_STATUS_PORT);

  /* Set attribute controller regs */
  for (i = 41; i < 62; i++) {
    status = inb (VGA_ATTRCON_PORT);

    //outb (i - 41, VGA_ATTRCON_PORT);
    //outb (mode12_regs[i], VGA_ATTRCON_PORT + 1);
  }
  
  outb (0x20, VGA_ATTRCON_PORT);
  outb (0xff, VGA_PEL_MASK_PORT);

  vga_set_palette (mode12_palette, VGA_COLORS);

  cursor_y = 100 / font_width;
  if (100 % font_width)
    cursor_y++;
}

/**
 * vga_set_palette
 *
 * Initialize the palette
 * palette_data is an array with palette colors ordered by RGB
 *
 * num_colors is the number of colors in palette_data
 *
 */

void vga_set_palette (char palette_data[], int num_colors)
{
  int i;

  if (num_colors == 16) {

    for (i = 0; i < num_colors; i++) { 

      switch (i) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 7:
	outb (i, VGA_PEL_WRITE_INDEX_PORT);
	break;
      case 6:
	outb (0x14, VGA_PEL_WRITE_INDEX_PORT);
	break;

      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:	
      case 15:
	outb (i + 0x30, VGA_PEL_WRITE_INDEX_PORT);
	break;
      }

      outb (palette_data[i * 3], VGA_PEL_DATA_PORT);
      outb (palette_data[i * 3 + 1], VGA_PEL_DATA_PORT);
      outb (palette_data[i * 3 + 2], VGA_PEL_DATA_PORT);
    }
  }
}

/**
 * vga_put_pixel
 *
 * put a pixel at a location, a pixel is a color value
 * max value of pixeldata is 255
 *
 */
void vga_put_pixel (int x, int y, unsigned char pixeldata)
{
  int             byte_num;
  int             bit_num;
  unsigned char   tmp_data;
  unsigned char   tmp_pixel;

  byte_num = y * VGA_X_RES + x;

  bit_num = byte_num % PIXELS_PER_BYTE;
  bit_num = ABS(7 - bit_num); /* Make bitnum from pixel num (bit 7 = pix 0) */

  byte_num = byte_num / PIXELS_PER_BYTE;

  //Set bitmask register
  //outb (8, VGA_CRTC_PORT);
  //outb (1 << bit_num, VGA_CRTC_PORT + 1);

  //plane 0
  outb (2, VGA_SEQ_PORT);
  outb (1, VGA_SEQ_PORT + 1);    /* Write plane */
  outb (4, VGA_GRACON_PORT);
  outb (0, VGA_GRACON_PORT + 1); /* read plane */

  tmp_pixel = pixeldata & 1;
  if (tmp_pixel)
    tmp_pixel = 1;

  tmp_data = video_mem[byte_num] & ~(1 << bit_num);
  video_mem[byte_num] = (tmp_pixel << bit_num) | tmp_data;

  //plane 1
  outb (2, VGA_SEQ_PORT);
  outb (2, VGA_SEQ_PORT + 1);
  outb (4, VGA_GRACON_PORT);
  outb (1, VGA_GRACON_PORT + 1); /* read plane */
  tmp_pixel = pixeldata & 2;
  if (tmp_pixel)
    tmp_pixel = 1;

  tmp_data = video_mem[byte_num] & ~(1 << bit_num);
  video_mem[byte_num] = (tmp_pixel << bit_num) | tmp_data;

  //plane 2
  outb (2, VGA_SEQ_PORT);
  outb (4, VGA_SEQ_PORT + 1);
  outb (4, VGA_GRACON_PORT);
  outb (2, VGA_GRACON_PORT + 1); /* read plane */
  tmp_pixel = pixeldata & 4;
  if (tmp_pixel)
    tmp_pixel = 1;

  tmp_data = video_mem[byte_num] & ~(1 << bit_num);
  video_mem[byte_num] = (tmp_pixel << bit_num) | tmp_data;

  //plane 3
  outb (2, VGA_SEQ_PORT);
  outb (8, VGA_SEQ_PORT + 1);
  outb (4, VGA_GRACON_PORT);
  outb (3, VGA_GRACON_PORT + 1); /* read plane */
  tmp_pixel = pixeldata & 8;
  if (tmp_pixel)
    tmp_pixel = 1;

  tmp_data = video_mem[byte_num] & ~(1 << bit_num);
  video_mem[byte_num] = (tmp_pixel << bit_num) | tmp_data;

}

/**
 * vga_put_block
 *
 * put a square block of pixels at a location, 
 * data is the color values of the pixels
 *
 * width has to be divisible by 8 and x has to be aligned with 8
 *
 */

void vga_put_block (int x, int y, int width, int height, unsigned char *data)
{
  int             byte_num;
  int             plane_no;
  int             byte_column;
  int             byte_row;
  int             i;
  unsigned char   plane_byte;


  if (x % 8 || width % 8)
    return;

  byte_num = y * VGA_X_RES + x;
  byte_num = byte_num / PIXELS_PER_BYTE;
  
  for (plane_no = 0; plane_no < 4; plane_no++) {
  
    //Select plane
    outb (2, VGA_SEQ_PORT);
    outb (1 << plane_no, VGA_SEQ_PORT + 1);    /* Write plane */

    for (byte_row = 0; byte_row < height; byte_row++) {

      for (byte_column = 0; byte_column < width / 8; byte_column++) {
	
	plane_byte = 0;

	for (i = 0; i < 8; i++) {
	  if (data[byte_row * width + byte_column * 8 + i] & (1 << plane_no))
	    plane_byte |= (1 << (7 - i));
	}

	video_mem[byte_num + byte_row * VGA_BYTES_PER_ROW + byte_column] = plane_byte;
      }
    }
  }
}


/*
 * vga_putchar, routine to print a char to hardware
 *
 * (assumes font 8 pixels wide)
 */

void vga_putchar (char chr, char color)
{
  char            *char_start;
  int             i,j;
  unsigned char   this_row;
  unsigned char   this_char[font_width * font_height];

  if (chr == '\n') {
    cursor_x = 0;
    cursor_y++;
    if (cursor_y > max_cursor_y)
      cursor_y = min_cursor_y;

    return;
  }

  if (chr & 0x80)
    chr -= 0x80;
  else
    chr += 0x80;

  char_start = &font_data[font_height * chr];

  for (i = 0; i < font_height; i++) {

    this_row = char_start[i];

    for (j = 0; j < font_width; j++) {

      if ((this_row >> (font_width - j - 1)) & 1) 
	this_char[font_width * i + j] = color;
      else
	this_char[font_width * i + j] = 0;
    }
  }

  vga_put_block (cursor_x * font_width, cursor_y * font_height, font_width, font_height, this_char);

  cursor_x++;
  if (cursor_x >= max_cursor_x) {
    cursor_y++;
    cursor_x = 0;

    if (cursor_y > max_cursor_y)
      cursor_y = min_cursor_y;
  }
}


/* 
 * clear screen
 * 
 */

void vga_clear_screen (void)
{
  int x;

  /* select all planes */
  outb (2, VGA_SEQ_PORT);
  outb (0xf, VGA_SEQ_PORT + 1);

  for (x = 0; x < (VGA_X_RES * VGA_Y_RES) / PIXELS_PER_BYTE; x++)
  {
    video_mem[x] = 0;
  }
}

