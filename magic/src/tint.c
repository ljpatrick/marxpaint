/*
  tint.c

  Tint Magic Tool Plugin
  Tux Paint - A simple drawing program for children.

  Copyright (c) 2002-2008 by Bill Kendrick and others; see AUTHORS.txt
  bill@newbreedsoftware.com
  http://www.tuxpaint.org/

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

  Last updated: July 8, 2008
  $Id$
*/

#include <stdio.h>
#include <string.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"


/* Our globals: */

static Mix_Chunk * tint_snd;
static Uint8 tint_r, tint_g, tint_b;


// No setup required:
int tint_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/tint.wav",
	    api->data_directory);
  tint_snd = Mix_LoadWAV(fname);

  return(1);
}

Uint32 tint_api_version(void) { return(TP_MAGIC_API_VERSION); }

// We have multiple tools:
int tint_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icons:
SDL_Surface * tint_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/tint.png",
	   api->data_directory);

  return(IMG_Load(fname));
}

// Return our names, localized:
char * tint_get_name(magic_api * api, int which)
{
  return(strdup(gettext_noop("Tint")));
}

// Return our descriptions, localized:
char * tint_get_description(magic_api * api, int which, int mode)
{
  return(strdup(gettext_noop(
"Click and move the mouse around to change the picture’s color.")));
}

// Do the effect:

static void do_tint(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
                int x, int y)
{
  magic_api * api = (magic_api *) ptr;
  double rd = api->sRGB_to_linear(tint_r);
  double gd = api->sRGB_to_linear(tint_g);
  double bd = api->sRGB_to_linear(tint_b);
  double old;
  int xx, yy;
  Uint8 r, g, b;

  for (yy = y - 16; yy < y + 16; yy++)
  {
    for (xx = x - 16; xx < x + 16; xx++)
    {
      if (api->in_circle(xx - x, yy - y, 16))
      {
        if (!api->touched(xx, yy))
        {
          /* Get original pixel: */

          SDL_GetRGB(api->getpixel(last, xx, yy), last->format, &r, &g, &b);

          old = api->sRGB_to_linear(r) * 0.2126 +
            api->sRGB_to_linear(g) * 0.7152 +
            api->sRGB_to_linear(b) * 0.0722;

          api->putpixel(canvas, xx, yy,
                 SDL_MapRGB(canvas->format,
                            api->linear_to_sRGB(rd * old),
                            api->linear_to_sRGB(gd * old),
                            api->linear_to_sRGB(bd * old)));
        }
      }
    }
  }
}

// Affect the canvas on drag:
void tint_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{
  api->line((void *) api, which, canvas, last, ox, oy, x, y, 1, do_tint);

  if (ox > x) { int tmp = ox; ox = x; x = tmp; }
  if (oy > y) { int tmp = oy; oy = y; y = tmp; }

  update_rect->x = ox - 16;
  update_rect->y = oy - 16;
  update_rect->w = (x + 16) - update_rect->x;
  update_rect->h = (y + 16) - update_rect->y;

  api->playsound(tint_snd, (x * 255) / canvas->w, 255);
}

// Affect the canvas on click:
void tint_click(magic_api * api, int which, int mode,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
  tint_drag(api, which, canvas, last, x, y, x, y, update_rect);
}


void tint_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
}

// No setup happened:
void tint_shutdown(magic_api * api)
{
  if (tint_snd != NULL)
    Mix_FreeChunk(tint_snd);
}

// Record the color from Tux Paint:
void tint_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
  tint_r = r;
  tint_g = g;
  tint_b = b;
}

// Use colors:
int tint_requires_colors(magic_api * api, int which)
{
  return 1;
}

void tint_switchin(magic_api * api, int which, SDL_Surface * canvas)
{
}

void tint_switchout(magic_api * api, int which, SDL_Surface * canvas)
{
}

int tint_modes(magic_api * api, int which)
{
  return(MODE_PAINT); /* FIXME - Can also be turned into a full-image effect */
}
