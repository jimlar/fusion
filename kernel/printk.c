/*
 * Printing utilities for Fusion
 * (c) Jimmy Larsson 1998
 * 
 */

#include "printk.h"
#include "port_io.h"
#include "vga.h"
#include <stdarg.h>

#define CHAR_ATTRIBUTE 0x1f
#define MAX_X 80
#define MAX_Y 25

/* Video Card I/O ports */
#define CARD_TEXT_INDEX      0x03D4
#define CARD_TEXT_DATA       0x03D5

/* Video Card Index Registers */
#define CARD_TEXT_CURSOR_LO  0x0F
#define CARD_TEXT_CURSOR_HI  0x0E


/* Hexadecimal mappings  */
static char hexmap[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' 
};


/* 
 * va_snprintf, main function for sprintf, snprintf, printf and nprintf 
 * (s_flag==1 means sprintf, s_flag==0 means printf)                 
 */

void va_snprintf(char *b, int l, char *fmt, va_list pvar, int s_flag) 
{
  int n, i;
  unsigned u;    
  char *t, d[10], uint_str[10];    
  
  /* bail if args == null */
  if (!fmt || (!b && s_flag) || (l < 1)) 
    return; 
    
  while (l && *fmt) 
  {
    if (*fmt == '\n')
    {
      vga_putchar (*fmt, 15);

    } else if (*fmt == '%')
    {
      fmt++;
      if (!(--l)) 
	break;
      
      switch (*fmt) 
      {
        case 's': /* string */
	  t = va_arg (pvar, char *);
	  while (l && *t)
	  {
	    if (s_flag) 
	      *b++ = *t++;
	    else
	      vga_putchar(*t++, 15);
	    
	    l--;
	  }                
	  break;
			
        case 'c': /* single character */
	  if (s_flag)
	    *b++ = va_arg (pvar, char);
	  else
	    vga_putchar (va_arg (pvar, char), 15);
	  l--;                
	  break;
	  
        case 'x': /* 8 digit, unsigned 32bit hex integer */
	  if (l < 8) 
	  { 
	    l = 0; 
	    break; 
	  }
	  u = va_arg (pvar, unsigned int);
	  for(i = 7; i >= 0; i--)
	  {
	    uint_str[i] = hexmap[u & 0x0F];
	    u >>= 4;
	  }
	  for (i = 0; i <= 7; i++)
	  {
	    if (s_flag)
	      *b++ = uint_str[i];
	    else
	      vga_putchar(uint_str[i], 15);
	  }
	  l -= 8;                
	  break;

        case 'd': /* signed integer */
	  n = va_arg (pvar, int);
	  if (n < 0) 
	  {
	    u = -n;
	    
	    if (s_flag)
	      *b++ = '-';
	    else
	      vga_putchar ('-', 15);
	    
	    if(!(--l)) 
	      break;                    
	  } else 
	  {
	    u = n;
	  }
	  goto u2;                
	  
        case 'u': /* unsigned integer */
	  u = va_arg (pvar, unsigned int);               
      u2:
	  i = 9;
	  do {
	    d[i] = (u % 10) + '0';
	    u /= 10;
	    i--;
	  } while (u && i >= 0);
	  
	  while (++i < 10)
	  {
	    if (s_flag)
	      *b++ = d[i];
	    else
	      vga_putchar (d[i], 15);
                    		
	    if(!(--l)) 
	      break;                    
	  }
	  break;                                    
	  
        case 'X': /* 2 digit, unsigned 8bit hex int */
	  if (l < 2) 
	  { 
	    l = 0; 
	    break; 
	  }
	  n = va_arg (pvar, int);
	  if (s_flag)
	  {
	    *b++ = hexmap[(n & 0xF0) >> 4];
	    *b++ = hexmap[n & 0x0F];
	  } else
	  {
	    vga_putchar (hexmap[(n & 0xF0) >> 4], 15);
	    vga_putchar (hexmap[n & 0x0F], 15);
	  }
	  l -= 2;                
	  break;
	  
        default:
	  if (s_flag)
	    *b++ = *fmt;
	  else
	    vga_putchar (*fmt, 15);                
      }
    } else 
    {
      if (s_flag)
	*b++ = *fmt;
      else
	vga_putchar (*fmt, 15);
      
      l--;            
    }
    fmt++;            
  }
  if (s_flag) /* zero terminate string */
    *b = 0;
}


/*
 * snprintf
 *
 */
 
void snprintf(char *str, int maxlen, char *fmt, ...)
{
    va_list pvar;    
    va_start (pvar,fmt);
    va_snprintf (str, maxlen, fmt, pvar, 1);
    va_end (pvar);    
}

/*
 * sprintf
 *
 */
 
void sprintf(char *str, char *fmt, ...)
{
    va_list pvar;    
    va_start (pvar,fmt);
    
    /* This should be MAX_INT or something but what the heck */
    va_snprintf (str, 0x7fffffff, fmt, pvar, 1);
    va_end (pvar);    
}


/*
 * nprintf
 *
 */
 
void nprintf(int maxlen, char *fmt, ...)
{
    va_list pvar;    
    va_start (pvar,fmt);
    va_snprintf (0L, maxlen, fmt, pvar, 0);
    va_end (pvar);    
}


/*
 * printf
 *
 */
 
void printf(char *fmt, ...)
{
    va_list pvar;    
    va_start (pvar,fmt);
    va_snprintf (0L, 0x7fffffff, fmt, pvar, 0);
    va_end (pvar);    
}

