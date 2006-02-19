/*
  floodfill.c

  For Tux Paint
  Flood fill functions

  Copyright (c) 2002-2006 by Bill Kendrick and others
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  (See COPYING.txt)

  June 14, 2002 - February 18, 2006
  $Id$
*/


#include "floodfill.h"
#include "pixels.h"
#include "rgblinear.h"
#include "sounds.h"
#include "playsound.h"
#include "progressbar.h"

#include "debug.h"


/* For flood fill... */

static int colors_close(SDL_Surface * canvas, Uint32 c1, Uint32 c2)
{
#ifdef LOW_QUALITY_FLOOD_FILL
  return (c1 == c2);
#else
  Uint8 r1, g1, b1,
    r2, g2, b2;

  if (c1 == c2)
    {
      /* Get it over with quick, if possible! */

      return 1;
    }
  else
    {
      double r, g, b;
      SDL_GetRGB(c1, canvas->format, &r1, &g1, &b1);
      SDL_GetRGB(c2, canvas->format, &r2, &g2, &b2);

      // use distance in linear RGB space
      r = sRGB_to_linear_table[r1] - sRGB_to_linear_table[r2];
      r *= r;
      g = sRGB_to_linear_table[g1] - sRGB_to_linear_table[g2];
      g *= g;
      b = sRGB_to_linear_table[b1] - sRGB_to_linear_table[b2];
      b *= b;

      // easy to confuse:
      //   dark grey, brown, purple
      //   light grey, tan
      //   red, orange
      return r+g+b < 0.04;
    }
#endif
}


/* Flood fill! */

void do_flood_fill(SDL_Surface * screen, SDL_Surface * canvas, int x, int y, Uint32 cur_colr, Uint32 old_colr)
{
  int fillL, fillR, i, in_line;
  static unsigned char prog_anim;
  Uint32 (*getpixel)(SDL_Surface *, int, int) = getpixels[canvas->format->BytesPerPixel];
  void (*putpixel)(SDL_Surface *, int, int, Uint32) = putpixels[canvas->format->BytesPerPixel];


  if (cur_colr == old_colr ||
      colors_close(canvas, cur_colr, old_colr))
    return;


  fillL = x;
  fillR = x;

  prog_anim++;
  if ((prog_anim % 4) == 0)
    {
      show_progress_bar(screen);
      playsound(0, SND_BUBBLE, 0);
    }


  /* Find left side, filling along the way */

  in_line = 1;

  while (in_line)
    {
      putpixel(canvas, fillL, y, cur_colr);
      fillL--;

      in_line = (fillL < 0) ? 0 : colors_close(canvas, getpixel(canvas, fillL, y),
                                               old_colr);
    }

  fillL++;


  /* Find right side, filling along the way */

  in_line = 1;
  while (in_line)
    {
      putpixel(canvas, fillR, y, cur_colr);
      fillR++;

      in_line = (fillR >= canvas->w) ? 0 : colors_close(canvas, getpixel(canvas,
                                                                 fillR, y),
                                                        old_colr);
    }

  fillR--;


  /* Search top and bottom */

  for (i = fillL; i <= fillR; i++)
    {
      if (y > 0 && colors_close(canvas, getpixel(canvas, i, y - 1), old_colr))
        do_flood_fill(screen, canvas, i, y - 1, cur_colr, old_colr);

      if (y < canvas->h && colors_close(canvas, getpixel(canvas, i, y + 1), old_colr))
        do_flood_fill(screen, canvas, i, y + 1, cur_colr, old_colr);
    }
}

