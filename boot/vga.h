/**
 * vga.h
 *
 * Boot console driver for FUSION
 *
 * (c) Jimmy Larsson 1999
 *
 */

#ifndef VGA_H
#define VGA_H

#define VGA_COLORS      16
#define VGA_X_RES       640
#define VGA_Y_RES       480

#define VGA_BYTES_PER_ROW  (VGA_X_RES/PIXELS_PER_BYTE)
#define PIXELS_PER_BYTE 8

#define VGA_MEM_ADDR 0xa0000

#define VGA_BACKGROUND 0x0f

/* VGA I/O Ports */
#define VGA_ATTRCON_PORT            0x3c0
#define VGA_MISC_PORT               0x3c2
#define VGA_ENABLE_PORT             0x3c3
#define VGA_SEQ_PORT                0x3c4
#define VGA_PEL_MASK_PORT           0x3c6
#define VGA_GRACON_PORT             0x3ce
#define VGA_CRTC_PORT               0x3d4
#define VGA_STATUS_PORT             0x3da
#define VGA_PEL_WRITE_INDEX_PORT    0x3c8
#define VGA_PEL_DATA_PORT           0x3c9 

/**
 * vga_set_mode
 * Initialize vga mode
 *
 */

void vga_set_mode (int mode);

/**
 * vga_set_palette
 *
 * Initialize the palette
 * palette_data is an array with palette colors ordered by RGB
 *
 * num_colors is the number of colors in palette_data
 *
 */

void vga_set_palette (char palette_data[], int num_colors);


/**
 * vga_put_pixel
 *
 * put a pixel at a location, a pixel is a color value
 * max value of pixeldata is 255
 *
 */

void vga_put_pixel (int x, int y, unsigned char pixeldata);

/**
 * vga_put_block
 *
 * put a square block of pixels at a location, 
 * data is the color values of the pixels
 *
 * width has to be divisible by 8 and x has to be aligned with 8
 *
 */

void vga_put_block (int x, int y, int width, int height, unsigned char *data);


/*
 * vga_putchar, routine to print a char to hardware
 *
 */

void vga_putchar (char chr, char color);

/* 
 * clear screen
 * 
 */

void vga_clear_screen (void);

/**
 * get state info 
 */
int get_cursor_x (void);
int get_cursor_y (void);
char *get_font_data (void); 
int get_font_height (void); 
int get_font_width (void); 


#endif
