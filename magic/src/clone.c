/*
  clone.c

  Clone tool paintbrush Magic Tools Plugin
  Marx Paint - A simple drawing program for communists.

  Copyright (c) 2021 by Bill Kendrick and others; see AUTHORS.txt
  bill@newbreedsoftware.com
  http://www.marxpaint.org/

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

  Last updated: February 20, 2021
  $Id$
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

/* What tools we contain: */

enum
{
  TOOL_CLONE,
  NUM_TOOLS
};


/* Tool states: */

enum
{
  CLONE_READY_TO_START,
  CLONE_STARTING,
  CLONE_CLONING
};


/* Our globals: */

static Mix_Chunk *clone_start_snd, *clone_snd;
int clone_state;
int clone_src_x, clone_src_y;
int clone_drag_start_x, clone_drag_start_y;
SDL_Surface * clone_last;
int clone_crosshair_visible;


/* Local function prototype: */

int clone_init(magic_api * api);
Uint32 clone_api_version(void);
int clone_get_tool_count(magic_api * api);
SDL_Surface *clone_get_icon(magic_api * api, int which);
char *clone_get_name(magic_api * api, int which);
char *clone_get_description(magic_api * api, int which, int mode);
void clone_drag(magic_api * api, int which, SDL_Surface * canvas,
                 SDL_Surface * last, int ox, int oy, int x, int y, SDL_Rect * update_rect);
void clone_doit(magic_api * api, int which, SDL_Surface * canvas,
                 SDL_Surface * last, int ox, int oy, int x, int y, SDL_Rect * update_rect,
                 int crosshairs);
void clone_click(magic_api * api, int which, int mode,
                  SDL_Surface * canvas, SDL_Surface * last, int x, int y, SDL_Rect * update_rect);
void clone_release(magic_api * api, int which, SDL_Surface * canvas, SDL_Surface * last, int x, int y, SDL_Rect * update_rect);
void clone_shutdown(magic_api * api);
void clone_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b);
int clone_requires_colors(magic_api * api, int which);
void clone_switchin(magic_api * api, int which, int mode, SDL_Surface * canvas);
void clone_switchout(magic_api * api, int which, int mode, SDL_Surface * canvas);
int clone_modes(magic_api * api, int which);
void clone_crosshairs(magic_api * api, SDL_Surface * canvas, int x, int y);
void done_cloning(magic_api * api, SDL_Surface * canvas, SDL_Rect * update_rect);

// No setup required:
int clone_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/clone_start.ogg", api->data_directory);
  clone_start_snd = Mix_LoadWAV(fname);

  snprintf(fname, sizeof(fname), "%s/sounds/magic/clone.ogg", api->data_directory);
  clone_snd = Mix_LoadWAV(fname);

  clone_state = CLONE_READY_TO_START;
  clone_crosshair_visible = 0;

  return (1);
}

Uint32 clone_api_version(void)
{
  return (TP_MAGIC_API_VERSION);
}

// We have multiple tools:
int clone_get_tool_count(magic_api * api ATTRIBUTE_UNUSED)
{
  return (NUM_TOOLS);
}

// Load our icons:
SDL_Surface *clone_get_icon(magic_api * api, int which ATTRIBUTE_UNUSED)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/clone.png", api->data_directory);

  return (IMG_Load(fname));
}

// Return our names, localized:
char *clone_get_name(magic_api * api ATTRIBUTE_UNUSED, int which ATTRIBUTE_UNUSED)
{
  return (strdup(gettext_noop("Clone")));
}

// Return our descriptions, localized:
char *clone_get_description(magic_api * api ATTRIBUTE_UNUSED, int which ATTRIBUTE_UNUSED, int mode ATTRIBUTE_UNUSED)
{
  return (strdup(gettext_noop("Click once to pick a spot to begin cloning. Click again and drag to clone that part of the picture.")));

  return (NULL);
}

// Do the effect:

static void do_clone(void *ptr, int which ATTRIBUTE_UNUSED, SDL_Surface * canvas, SDL_Surface * last, int x, int y)
{
  magic_api *api = (magic_api *) ptr;
  Uint8 r, g, b;
  int xx, yy;
  int srcx, srcy;
  Uint32 pixel;

  srcx = clone_src_x + (x - clone_drag_start_x);
  srcy = clone_src_y + (y - clone_drag_start_y);

  if (!api->touched(x, y))
    {
      for (yy = -16; yy < 16; yy++)
        {
          for (xx = -16; xx < 16; xx++)
            {
              if (api->in_circle(xx, yy, 16))
                {
                  SDL_GetRGB(api->getpixel(last, srcx + xx, srcy + yy), last->format, &r, &g, &b);
                  pixel = SDL_MapRGB(canvas->format, r, g, b);
                  api->putpixel(canvas, x + xx, y + yy, pixel);
                }
            }
        }
    }
}

// Affect the canvas on drag:
void clone_drag(magic_api * api, int which, SDL_Surface * canvas,
                 SDL_Surface * last ATTRIBUTE_UNUSED, int ox, int oy, int x, int y, SDL_Rect * update_rect)
{
  /* Step 3 - Actively cloning (moving the mouse) */

  /* Erase crosshairs at old source position */
  clone_crosshairs(api, canvas, clone_src_x, clone_src_y);
  clone_crosshair_visible = 0;

  /* Do the cloning (and draw crosshairs at new source position) */
  clone_doit(api, which, canvas, clone_last, ox, oy, x, y, update_rect, 1);
}

void clone_doit(magic_api * api, int which, SDL_Surface * canvas,
                 SDL_Surface * last, int ox, int oy, int x, int y, SDL_Rect * update_rect,
                 int crosshairs)
{
  if (clone_state != CLONE_CLONING)
    return;

  clone_drag_start_x = ox;
  clone_drag_start_y = oy;

  api->line((void *)api, which, canvas, last, ox, oy, x, y, 1, do_clone);

  /* Move source position relative to mouse motion */
  clone_src_x += (x - ox);
  clone_src_y += (y - oy);

  if (ox > x)
    {
      int tmp = ox;

      ox = x;
      x = tmp;
    }
  if (oy > y)
    {
      int tmp = oy;

      oy = y;
      y = tmp;
    }

  if (crosshairs) {
    clone_crosshairs(api, canvas, clone_src_x, clone_src_y);
    /* FIXME be more clever */
    update_rect->x = 0;
    update_rect->y = 0;
    update_rect->w = canvas->w;
    update_rect->h = canvas->h;
    clone_crosshair_visible = 1;
  } else {
    update_rect->x = x - 64;
    update_rect->y = y - 64;
    update_rect->w = (ox + 128) - update_rect->x;
    update_rect->h = (oy + 128) - update_rect->h;
  }

  api->playsound(clone_snd, (x * 255) / canvas->w, 255);
}

// Affect the canvas on click:
void clone_click(magic_api * api, int which, int mode ATTRIBUTE_UNUSED,
                  SDL_Surface * canvas, SDL_Surface * last, int x, int y, SDL_Rect * update_rect)
{
  if (clone_state == CLONE_READY_TO_START) {
    /* Step 1 - Picking a source for the clone */
    clone_src_x = x;
    clone_src_y = y;
    clone_state = CLONE_STARTING;
    api->playsound(clone_start_snd, (x * 255) / canvas->w, 255);

    SDL_BlitSurface(last, NULL, clone_last, NULL);

    /* Draw crosshairs at starting source position */
    clone_crosshairs(api, canvas, clone_src_x, clone_src_y);
    clone_crosshair_visible = 1;
    update_rect->x = x - 15;
    update_rect->y = y - 15;
    update_rect->w = 32;
    update_rect->h = 32;
  } else if (clone_state == CLONE_CLONING) {
    /* Step 2 - Starting a clone (hopefully holding mouse down here) */
    clone_doit(api, which, canvas, clone_last, x, y, x, y, update_rect, 0);
  }
}

void clone_release(magic_api * api, int which ATTRIBUTE_UNUSED,
                    SDL_Surface * canvas, SDL_Surface * last ATTRIBUTE_UNUSED,
                    int x ATTRIBUTE_UNUSED, int y ATTRIBUTE_UNUSED, SDL_Rect * update_rect)
{
  if (clone_state == CLONE_STARTING) {
    /* Release of the initial click (to pick initial source position);
       now ready for second click (to begin cloning) */
    clone_state = CLONE_CLONING;
  } else {
    done_cloning(api, canvas, update_rect);
  }
}

void done_cloning(magic_api * api, SDL_Surface * canvas, SDL_Rect * update_rect) {
  /* Done cloning! */

  /* Erase crosshairs from source position, now that we're all done */
  if (clone_crosshair_visible)
    {
      clone_crosshairs(api, canvas, clone_src_x, clone_src_y);
      update_rect->x = clone_src_x - 15;
      update_rect->y = clone_src_y - 15;
      update_rect->w = 32;
      update_rect->h = 32;
      clone_crosshair_visible = 0;
    }

  clone_state = CLONE_READY_TO_START;
  api->stopsound();
}

void clone_crosshairs(magic_api * api, SDL_Surface * canvas, int x, int y) {
  int i;

  for (i = -15; i < 16; i++) {
    api->xorpixel(canvas, x + i, y);
    api->xorpixel(canvas, x, y + i);
  }
}

void clone_shutdown(magic_api * api ATTRIBUTE_UNUSED)
{
  if (clone_snd != NULL)
    Mix_FreeChunk(clone_snd);
  if (clone_start_snd != NULL)
    Mix_FreeChunk(clone_start_snd);
}

void clone_set_color(magic_api * api ATTRIBUTE_UNUSED, Uint8 ATTRIBUTE_UNUSED r, Uint8 ATTRIBUTE_UNUSED g, Uint8 ATTRIBUTE_UNUSED b)
{
}

int clone_requires_colors(magic_api * api ATTRIBUTE_UNUSED, int which ATTRIBUTE_UNUSED)
{
  return 0;
}

void clone_switchin(magic_api * api ATTRIBUTE_UNUSED, int which ATTRIBUTE_UNUSED,
                     int mode ATTRIBUTE_UNUSED, SDL_Surface * canvas ATTRIBUTE_UNUSED)
{
  clone_last = SDL_CreateRGBSurface(SDL_ANYFORMAT, canvas->w, canvas->h, canvas->format->BitsPerPixel,
                                    canvas->format->Rmask, canvas->format->Gmask, canvas->format->Bmask,
                                    canvas->format->Amask);

  clone_state = CLONE_READY_TO_START;
}

void clone_switchout(magic_api * api, int which ATTRIBUTE_UNUSED,
                      int mode ATTRIBUTE_UNUSED, SDL_Surface * canvas)
{
  SDL_Rect update_rect; /* Needed to satisfy done_cloning() :-( */

  done_cloning(api, canvas, &update_rect);

  if (clone_last != NULL)
    SDL_FreeSurface(clone_last);
}

int clone_modes(magic_api * api ATTRIBUTE_UNUSED, int which ATTRIBUTE_UNUSED)
{
  return (MODE_PAINT);
}
