/*
  hqNx filter look-up table init and helper functions

  Copyright (C) 2003 MaxSt <maxst@hiend3d.com>
  Library-ified by Bill Kendrick <bill@newbreedsoftware.com>
  Based on "hq3x_src_c.zip" dated August 5, 2003
  from: http://www.hiend3d.com/hq3x.html

  December 20, 2003 - December 23, 2003

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


#include <stdlib.h>
#include "SDL.h"
#include "hqxx.h"


Uint16 hqxx_getpixel(SDL_Surface * surface, int x, int y, Uint8 * alpha)
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
  
  if (alpha == NULL)
    SDL_GetRGB(pixel, surface->format, &r, &g, &b);
  else
    SDL_GetRGBA(pixel, surface->format, &r, &g, &b, alpha);

  
  /* (Perhaps reducing the number of colors by chopping off the bottoms of
     R, G and B will help?) */
  
  pixel16 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

  return pixel16;
}


void hqxx_putpixel(SDL_Surface * surface, int x, int y, Uint16 pixel, Uint8 alpha)
{
  int bpp;
  Uint8 * p;
  Uint8 r, g, b;
  Uint32 sdlpixel;
  
  
  /* Convert the 16bpp RGB-565 to the current surface's format: */
  
  r = (pixel & 0xF800) >> 8;
  g = ((pixel & 0x07E0) >> 5) << 2;
  b = (pixel & 0x001F) << 3;

  sdlpixel = SDL_MapRGBA(surface->format, r, g, b, alpha);
  
  
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
        *p = sdlpixel;
      else if (bpp == 2)
        *(Uint16 *)p = sdlpixel;
      else if (bpp == 3)
        {
          if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
              p[0] = (sdlpixel >> 16) & 0xff;
              p[1] = (sdlpixel >> 8) & 0xff;
              p[2] = sdlpixel & 0xff;
            }
          else
            {
              p[0] = sdlpixel & 0xff;
              p[1] = (sdlpixel >> 8) & 0xff;
              p[2] = (sdlpixel >> 16) & 0xff;
            }
        }
      else if (bpp == 4)
        {
          *(Uint32 *)p = sdlpixel;
        }

      /* SDL_UnlockSurface(surface); */
    }
}



void InitLUTs(Uint32 * RGBtoYUV)
{
  int i, j, k, r, g, b, Y, u, v;

  for (i = 0; i < 32; i++)
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
	      RGBtoYUV[(i << 11) + (j << 5) + k] = (((Y & 0xF8) << 8) |
						    ((u & 0xFE) << 3) |
						    (v >> 3));
	    }
	}
    }
}

inline void Interp0(SDL_Surface * dest, int x, int y, Uint16 c, Uint8 alpha)
{
  hqxx_putpixel(dest, x, y, c, alpha);
}

inline void Interp1(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint8 alpha)
{
  Uint32 c;
  
  // c = (c1 * 3 + c2) >> 2;

  c = ((((c1 & 0x07E0) * 3 + (c2 & 0x07E0)) & (0x07E0 << 2)) +
       (((c1 & 0xF81F) * 3 + (c2 & 0xF81F)) & (0xF81F << 2))) >> 2;
  
  hqxx_putpixel(dest, x, y, c, alpha);
}

inline void Interp2(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint16 c3, Uint8 alpha)
{
  Uint32 c;

  /* FIXME? */
  
  // c = (c1 * 2 + c2 + c3) >> 2;

  c = ((((c1 & 0x07E0) * 2 + (c2 & 0x07E0) + (c3 & 0x07E0)) & (0x07E0 << 2)) +
       (((c1 & 0xF81F) * 2 + (c2 & 0xF81F) + (c3 & 0xF81F)) & (0xF81F << 2)))
								      >> 2;


  hqxx_putpixel(dest, x, y, c, alpha);
}

inline void Interp3(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint8 alpha)
{
  Uint32 c;

  //c = (c1 * 7 + c2) / 8;

  //c = ((((c1 & 0x00FF00) * 7 + (c2 & 0x00FF00)) & 0x0007F800) +
  //     (((c1 & 0xFF00FF) * 7 + (c2 & 0xFF00FF)) & 0x07F807F8)) >> 3;
  
  c = ((((c1 & 0x07E0) * 7 + (c2 & 0x07E0)) & (0x07E0 << 3)) +
       (((c1 & 0xF81F) * 7 + (c2 & 0xF81F)) & (0xF81F << 3))) >> 3;

  hqxx_putpixel(dest, x, y, c, alpha);
}

inline void Interp4(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint16 c3, Uint8 alpha)
{
  Uint32 c;

  //c = (c1 * 2 + (c2 + c3) * 7) / 16;

  //c = ((((c1 & 0x00FF00) * 2 +
  // ((c2 & 0x00FF00) +
  //  (c3 & 0x00FF00)) * 7) & 0x000FF000) +
  //   (((c1 & 0xFF00FF) * 2 +
  // ((c2 & 0xFF00FF) +
  //  (c3 & 0xFF00FF)) * 7) & 0x0FF00FF0)) >> 4;
  
  c = ((((c1 & 0x07E0) * 2 +
	 ((c2 & 0x07E0) +
	  (c3 & 0x07E0)) * 7) & 0x7E00) +
       (((c1 & 0xF81F) * 2 +
	 ((c2 & 0xF81F) +
	  (c3 & 0xF81F)) * 7) & 0xF81F0)) >> 4;

  hqxx_putpixel(dest, x, y, c, alpha);
}

inline void Interp5(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint8 alpha)
{
  Uint32 c;

  // c = (c1 + c2) >> 1;

  c = ((((c1 & 0x07E0) + (c2 & 0x07E0)) & (0x07E0 << 1)) +
       (((c1 & 0xF81F) + (c2 & 0xF81F)) & (0xF81F << 1))) >> 1;

  hqxx_putpixel(dest, x, y, c, alpha);
}

inline void Interp6(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint16 c3, Uint8 alpha)
{
  Uint32 c;

  // c = (c1 * 5 + c2 * 2 + c3) / 8;

  //  c = ((((c1 & 0x00FF00) * 5 +
  // (c2 & 0x00FF00) * 2 + (c3 & 0x00FF00)) & 0x0007F800) +
  //   (((c1 & 0xFF00FF) * 5 +
  // (c2 & 0xFF00FF) * 2 + (c3 & 0xFF00FF)) & 0x07F807F8)) >> 3;

  c = ((((c1 & 0x07E0) * 5 +
	 (c2 & 0x07E0) * 2 + (c3 & 0x07E0)) & (0x07E0 << 3)) +
       (((c1 & 0xF81F) * 5 +
	 (c2 & 0xF81F) * 2 + (c3 & 0xF81F)) & (0xF81F << 3))) >> 3;

  hqxx_putpixel(dest, x, y, c, alpha);
}

inline void Interp7(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint16 c3, Uint8 alpha)
{
  Uint32 c;
  
  //c = (c1 * 6 + c2 + c3) / 8;

  //c = ((((c1 & 0x00FF00)*6 + (c2 & 0x00FF00) + (c3 & 0x00FF00)) & 0x0007F800) +
  //     (((c1 & 0xFF00FF)*6 + (c2 & 0xFF00FF) + (c3 & 0xFF00FF)) & 0x07F807F8))
  // >> 3;
  
  c = ((((c1 & 0x07E0)*6 + (c2 & 0x07E0) + (c3 & 0x07E0)) & (0x07E0 << 3)) +
       (((c1 & 0xF81F)*6 + (c2 & 0xF81F) + (c3 & 0xF81F)) & (0xF81F << 3)))
	 >> 3;

  hqxx_putpixel(dest, x, y, c, alpha);
}

inline void Interp8(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint8 alpha)
{
  Uint32 c;

  //c = (c1 * 5 + c2 * 3) / 8;
  
  //c = ((((c1 & 0x00FF00) * 5 + (c2 & 0x00FF00) * 3) & 0x0007F800) +
  //   (((c1 & 0xFF00FF) * 5 + (c2 & 0xFF00FF) * 3) & 0x07F807F8)) >> 3;

  c = ((((c1 & 0x07E0) * 5 + (c2 & 0x07E0) * 3) & (0x07E0 << 3)) +
       (((c1 & 0xF81F) * 5 + (c2 & 0xF81F) * 3) & (0xF81F << 3))) >> 3;

  hqxx_putpixel(dest, x, y, c, alpha);
}


inline int Diff(unsigned int w1, unsigned int w2)
{
  return ((abs((w1 & Ymask) - (w2 & Ymask)) > trY) ||
          (abs((w1 & Umask) - (w2 & Umask)) > trU) ||
          (abs((w1 & Vmask) - (w2 & Vmask)) > trV));
}
