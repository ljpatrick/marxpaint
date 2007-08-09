/*
  sparkles.c

  Sparkles Magic Tool Plugin
  Tux Paint - A simple drawing program for children.

  Copyright (c) 2002-2007 by Bill Kendrick and others; see AUTHORS.txt
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

  Last updated: August 9, 2007
  $Id$
*/

#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

#include "math.h"

/* Our globals: */

Mix_Chunk * sparkles1_snd, * sparkles2_snd;
float sparkles_h, sparkles_s, sparkles_v;


Uint32 sparkles_api_version(void) { return(TP_MAGIC_API_VERSION); }


// No setup required:
int sparkles_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/sparkles1.wav",
	    api->data_directory);
  sparkles1_snd = Mix_LoadWAV(fname);

  snprintf(fname, sizeof(fname), "%s/sounds/magic/sparkles2.wav",
	    api->data_directory);
  sparkles2_snd = Mix_LoadWAV(fname);

  return(1);
}

// We have multiple tools:
int sparkles_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icons:
SDL_Surface * sparkles_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/sparkles.png",
	   api->data_directory);

  return(IMG_Load(fname));
}

// Return our names, localized:
char * sparkles_get_name(magic_api * api, int which)
{
  return(strdup(gettext("Sparkles")));
}

// Return our descriptions, localized:
char * sparkles_get_description(magic_api * api, int which)
{
  return(strdup(gettext("Click and drag to draw glowing sparkles on your picture.")));
}

// Do the effect:

void do_sparkles(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
                int x, int y)
{
  magic_api * api = (magic_api *) ptr;
  int xx, yy;
  Uint32 pix;
  Uint8 r, g, b;
  float h, s, v, new_h, new_s, new_v;
  float adj;

  for (yy = -8; yy < 8; yy++)
  {
    for (xx = -8; xx < 8; xx++)
    {
      if (api->in_circle(xx, yy, 8))
      {
        pix = api->getpixel(canvas, x + xx, y + yy);

        SDL_GetRGB(pix, canvas->format, &r, &g, &b);

        adj = (7.99 - sqrt(abs(xx * yy))) / 128.0;

        api->rgbtohsv(r, g, b, &h, &s, &v);

        v = min(1.0, v + adj);

        if (sparkles_h == -1 && h == -1)
        {
          new_h = -1;
          new_s = 0;
          new_v = v;
        }
        else if (sparkles_h == -1)
        {
          new_h = h;
          new_s = max(0.0, s - adj / 2.0);
          new_v = v;
        }
        else if (h == -1)
        {
          new_h = sparkles_h;
          new_s = max(0.0, sparkles_s - adj / 2.0);
          new_v = v;
        }
        else
        {
          new_h = (sparkles_h + h) / 2;
          new_s = max(0.0, s - adj / 2.0);
          new_v = v;
        }

        api->hsvtorgb(new_h, new_s, new_v, &r, &g, &b);

        api->putpixel(canvas, x + xx, y + yy,
                      SDL_MapRGB(canvas->format, r, g, b));
      }
    }
  }
}

// Affect the canvas on drag:
void sparkles_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{
  api->line((void *) api, which, canvas, last, ox, oy, x, y, 1, do_sparkles);

  if (ox > x) { int tmp = ox; ox = x; x = tmp; }
  if (oy > y) { int tmp = oy; oy = y; y = tmp; }

  update_rect->x = ox - 8;
  update_rect->y = oy - 8;
  update_rect->w = (x + 8) - update_rect->x;
  update_rect->h = (y + 8) - update_rect->h;

  if ((rand() % 10) == 0)
  {
    if ((rand() % 10) < 5)
      api->playsound(sparkles1_snd, (x * 255) / canvas->w, 255);
    else
      api->playsound(sparkles2_snd, (x * 255) / canvas->w, 255);
  }
}

// Affect the canvas on click:
void sparkles_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
  sparkles_drag(api, which, canvas, last, x, y, x, y, update_rect);
}

// Affect the canvas on release:
void sparkles_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
}

// No setup happened:
void sparkles_shutdown(magic_api * api)
{
  if (sparkles1_snd != NULL)
    Mix_FreeChunk(sparkles1_snd);
  if (sparkles2_snd != NULL)
    Mix_FreeChunk(sparkles2_snd);
}

// Record the color from Tux Paint:
void sparkles_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
  api->rgbtohsv(r, g, b, &sparkles_h, &sparkles_s, &sparkles_v);
}

// Use colors:
int sparkles_requires_colors(magic_api * api, int which)
{
  return 1;
}

