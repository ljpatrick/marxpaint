/*
  hqNx filter look-up table init and helper functions

  Copyright (C) 2003 MaxSt <maxst@hiend3d.com>
  Library-ified by Bill Kendrick <bill@newbreedsoftware.com>
  Based on "hq3x_src_c.zip" dated August 5, 2003
  from: http://www.hiend3d.com/hq3x.html

  December 20, 2003 - December 20, 2003

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


#include "SDL.h"
#include "hqxx.h"


Uint16 hqxx_getpixel(SDL_Surface * surface, int x, int y)
{
  int bpp;
  Uint8 * p;
  Uint32 pixel;
  Uint8 r, g, b;
  Uint16 pixel16;
  
  pixel = 0;


  /* Assuming the X/Y values are within the bounds of this surface... */

  if (x >= 0 && y >= 0 && x < surface -> w && y < surface -> h)
    {
      /* SDL_LockSurface(surface); */


      /* Determine bytes-per-pixel for the surface in question: */

      bpp = surface->format->BytesPerPixel;


      /* Set a pointer to the exact location in memory of the pixel
         in question: */

      p = (Uint8 *) (((Uint8 *)surface->pixels) +  /* Start at top of RAM */
                      (y * surface->pitch) +  /* Go down Y lines */
                      (x * bpp));             /* Go in X pixels */


      /* Return the correctly-sized piece of data containing the
       * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
       * RGB value) */

      if (bpp == 1)         /* 8-bit display */
        pixel = *p;
      else if (bpp == 2)    /* 16-bit display */
        pixel = *(Uint16 *)p;
      else if (bpp == 3)    /* 24-bit display */
        {
          /* Depending on the byte-order, it could be stored RGB or BGR! */

          if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            pixel = p[0] << 16 | p[1] << 8 | p[2];
          else
            pixel = p[0] | p[1] << 8 | p[2] << 16;
        }

      else if (bpp == 4)    /* 32-bit display */
        pixel = *(Uint32 *)p;

      /* SDL_UnlockSurface(surface); */
    }
  
  SDL_GetRGB(pixel, surface->format, &r, &g, &b);
  
  pixel16 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
  
  return pixel16;
}


void hqxx_putpixel(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  int bpp;
  Uint8 * p;


  /* Assuming the X/Y values are within the bounds of this surface... */

  if (x >= 0 && y >= 0 && x < surface->w && y < surface->h)
    {
      /* SDL_LockSurface(surface); */


      /* Determine bytes-per-pixel for the surface in question: */

      bpp = surface->format->BytesPerPixel;


      /* Set a pointer to the exact location in memory of the pixel
       *          in question: */

      p = (Uint8 *) (((Uint8 *)surface->pixels) + /* Start: beginning of RAM */
                     (y * surface->pitch) +  /* Go down Y lines */
                     (x * bpp));             /* Go in X pixels */


      /* Set the (correctly-sized) piece of data in the surface's RAM
       *          to the pixel value sent in: */

      if (bpp == 1)
        *p = pixel;
      else if (bpp == 2)
        *(Uint16 *)p = pixel;
      else if (bpp == 3)
        {
          if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
              p[0] = (pixel >> 16) & 0xff;
              p[1] = (pixel >> 8) & 0xff;
              p[2] = pixel & 0xff;
            }
          else
            {
              p[0] = pixel & 0xff;
              p[1] = (pixel >> 8) & 0xff;
              p[2] = (pixel >> 16) & 0xff;
            }
        }
      else if (bpp == 4)
        {
          *(Uint32 *)p = pixel;
        }

      /* SDL_UnlockSurface(surface); */
    }
}



void InitLUTs(int * LUT16to32, int * RGBtoYUV)
{
  int i, j, k, r, g, b, Y, u, v;

  for (i = 0; i < 65536; i++)
    {
      LUT16to32[i] = (((i & 0xF800) << 8) +
		      ((i & 0x07E0) << 5) +
		      ((i & 0x001F) << 3));
    }

  for (i=0; i<32; i++)
    {
      for (j = 0; j < 64; j++)
	{
	  for (k = 0; k < 32; k++)
	    {
	      r = i << 3;
	      g = j << 2;
	      b = k << 3;
	      Y = (r + g + b) >> 2;
	      u = 128 + ((r - b) >> 2);
	      v = 128 + ((-r + 2 * g - b) >> 3);
	      RGBtoYUV[(i << 11) + (j << 5) + k] = (Y<<16) + (u<<8) + v;
	    }
	}
    }
}

inline void Interp0(SDL_Surface * dest, int x, int y, int c)
{
  Uint8 r, g, b;
  
  r = (c >> 11) & 0xFF;
  g = (c >> 5) & 0xFF;
  b = (c >> 0)  & 0xFF;
  
  hqxx_putpixel(dest, x, y, SDL_MapRGB(dest->format, r, g, b));
}

inline void Interp1(SDL_Surface * dest, int x, int y, int c1, int c2)
{
  Uint8 r, g, b;
  Uint16 c;
  
  c = (c1 * 3 + c2) >> 2;
  
  r = (c >> 11) & 0xFF;
  g = (c >> 5) & 0xFF;
  b = (c >> 0)  & 0xFF;
  
  hqxx_putpixel(dest, x, y, SDL_MapRGB(dest->format, r, g, b));
}

inline void Interp2(SDL_Surface * dest, int x, int y, int c1, int c2, int c3)
{
  Uint8 r, g, b;
  Uint16 c;

  c = (c1 * 2 + c2 + c3) >> 2;

  r = (c >> 11) & 0xFF;
  g = (c >> 5) & 0xFF;
  b = (c >> 0)  & 0xFF;
  
  hqxx_putpixel(dest, x, y, SDL_MapRGB(dest->format, r, g, b));
}

inline void Interp3(SDL_Surface * dest, int x, int y, int c1, int c2)
{
  Uint8 r, g, b;
  Uint16 c;

  //c = (c1*7+c2)/8;

  c = ((((c1 & 0x00FF00) * 7 + (c2 & 0x00FF00)) & 0x0007F800) +
       (((c1 & 0xFF00FF) * 7 + (c2 & 0xFF00FF)) & 0x07F807F8)) >> 3;

  r = (c >> 11) & 0xFF;
  g = (c >> 5) & 0xFF;
  b = (c >> 0)  & 0xFF;
  
  hqxx_putpixel(dest, x, y, SDL_MapRGB(dest->format, r, g, b));
}

inline void Interp4(SDL_Surface * dest, int x, int y, int c1, int c2, int c3)
{
  Uint8 r, g, b;
  Uint16 c;

  //c = (c1*2+(c2+c3)*7)/16;

  c = ((((c1 & 0x00FF00) * 2 +
	 ((c2 & 0x00FF00) +
	  (c3 & 0x00FF00)) * 7) & 0x000FF000) +
       (((c1 & 0xFF00FF) * 2 +
	 ((c2 & 0xFF00FF) +
	  (c3 & 0xFF00FF)) * 7) & 0x0FF00FF0)) >> 4;

  r = (c >> 11) & 0xFF;
  g = (c >> 5) & 0xFF;
  b = (c >> 0)  & 0xFF;
  
  hqxx_putpixel(dest, x, y, SDL_MapRGB(dest->format, r, g, b));
}

inline void Interp5(SDL_Surface * dest, int x, int y, int c1, int c2)
{
  Uint8 r, g, b;
  Uint16 c;

  c = (c1+c2) >> 1;

  r = (c >> 11) & 0xFF;
  g = (c >> 5) & 0xFF;
  b = (c >> 0)  & 0xFF;
  
  hqxx_putpixel(dest, x, y, SDL_MapRGB(dest->format, r, g, b));
}

inline void Interp6(SDL_Surface * dest, int x, int y, int c1, int c2, int c3)
{
  Uint8 r, g, b;
  Uint16 c;

  //c = (c1*5+c2*2+c3)/8;

  c = ((((c1 & 0x00FF00) * 5 +
	 (c2 & 0x00FF00) * 2 + (c3 & 0x00FF00)) & 0x0007F800) +
       (((c1 & 0xFF00FF) * 5 +
	 (c2 & 0xFF00FF) * 2 + (c3 & 0xFF00FF)) & 0x07F807F8)) >> 3;

  r = (c >> 11) & 0xFF;
  g = (c >> 5) & 0xFF;
  b = (c >> 0)  & 0xFF;
  
  hqxx_putpixel(dest, x, y, SDL_MapRGB(dest->format, r, g, b));
}

inline void Interp7(SDL_Surface * dest, int x, int y, int c1, int c2, int c3)
{
  Uint8 r, g, b;
  Uint16 c;

  //c = (c1*6+c2+c3)/8;

  c = ((((c1 & 0x00FF00)*6 + (c2 & 0x00FF00) + (c3 & 0x00FF00)) & 0x0007F800) +
       (((c1 & 0xFF00FF)*6 + (c2 & 0xFF00FF) + (c3 & 0xFF00FF)) & 0x07F807F8))
	 >> 3;

  r = (c >> 11) & 0xFF;
  g = (c >> 5) & 0xFF;
  b = (c >> 0)  & 0xFF;
  
  hqxx_putpixel(dest, x, y, SDL_MapRGB(dest->format, r, g, b));
}

inline void Interp8(SDL_Surface * dest, int x, int y, int c1, int c2)
{
  Uint8 r, g, b;
  Uint16 c;

  //c = (c1*5+c2*3)/8;

  c = ((((c1 & 0x00FF00) * 5 + (c2 & 0x00FF00) * 3) & 0x0007F800) +
       (((c1 & 0xFF00FF) * 5 + (c2 & 0xFF00FF) * 3) & 0x07F807F8)) >> 3;

  r = (c >> 11) & 0xFF;
  g = (c >> 5) & 0xFF;
  b = (c >> 0)  & 0xFF;
  
  hqxx_putpixel(dest, x, y, SDL_MapRGB(dest->format, r, g, b));
}


inline int Diff(unsigned int w1, unsigned int w2)
{
  YUV1 = w1; /* RGBtoYUV[w1]; */
  YUV2 = w2; /* RGBtoYUV[w2]; */
  return ((abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY) ||
          (abs((YUV1 & Umask) - (YUV2 & Umask)) > trU) ||
          (abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV));
}
