#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

/* Our globals: */

Mix_Chunk * flower_snd;
Uint8 flower_r, flower_g, flower_b;
int flower_min_x, flower_max_x, flower_bottom_x, flower_bottom_y;

/* Local function prototypes: */

void flower_drawbase(magic_api * api, SDL_Surface * canvas, int x, int y);
void flower_drawstalk(magic_api * api, SDL_Surface * canvas,
		      int minx, int maxx, int endx, int starty, int endy);
void flower_drawflower(magic_api * api, SDL_Surface * canvas, int x, int y);


Uint32 flower_api_version(void) { return(TP_MAGIC_API_VERSION); }


// No setup required:
int flower_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/flower.wav",
	    api->data_directory);
  flower_snd = Mix_LoadWAV(fname);

  return(1);
}

// We have multiple tools:
int flower_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icons:
SDL_Surface * flower_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/flower.png",
	   api->data_directory);

  return(IMG_Load(fname));
}

// Return our names, localized:
char * flower_get_name(magic_api * api, int which)
{
  return(strdup(gettext("Flower")));
}

// Return our descriptions, localized:
char * flower_get_description(magic_api * api, int which)
{
  return(strdup(gettext("Click and drag to draw a flower stalk. Let go to finish the flower.")));
}

// Affect the canvas on drag:
void flower_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{
  if (x < flower_min_x)
    flower_min_x = x;
  if (ox < flower_min_x)
    flower_min_x = ox;
  if (x > flower_max_x)
    flower_max_x = x;
  if (ox > flower_max_x)
    flower_max_x = ox;

  if (y > flower_bottom_y)
    y = flower_bottom_y;
  if (oy > flower_bottom_y)
    y = flower_bottom_y;

  SDL_BlitSurface(last, NULL, canvas, NULL);

  flower_drawbase(api, canvas, flower_bottom_x, flower_bottom_y);

  flower_drawstalk(api, canvas, flower_min_x, flower_max_x, x,
		   y, flower_bottom_y);

  update_rect->x = 0;
  update_rect->y = 0;
  update_rect->w = canvas->w; 
  update_rect->h = canvas->h;

  api->playsound(flower_snd, (x * 255) / canvas->w, 255); /* FIXME: Distance? */
}

// Affect the canvas on click:
void flower_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
  flower_min_x = x;
  flower_max_x = x;
  flower_bottom_x = x;
  flower_bottom_y = y;

  flower_drag(api, which, canvas, last, x, y, x, y, update_rect);
}

// Affect the canvas on release:
void flower_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
  if (y >= flower_bottom_y - 32)
    y = flower_bottom_y - 32;

  flower_drag(api, which, canvas, last, x, y, x, y, update_rect);


  flower_drawflower(api, canvas, x, y);

/* FIXME */
  update_rect->x = 0;
  update_rect->y = 0;
  update_rect->w = canvas->w;
  update_rect->h = canvas->h;
}


void flower_drawflower(magic_api * api, SDL_Surface * canvas, int x, int y)
{
  SDL_Rect dest;

  /* FIXME: Draw the flower in the chosen color: */

  dest.x = x - 8;
  dest.y = y - 8;
  dest.w = 16;
  dest.h = 16;

  SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format,
					flower_r,
					flower_g,
					flower_b));
}

void flower_drawbase(magic_api * api, SDL_Surface * canvas, int x, int y)
{
  SDL_Rect dest;

  /* FIXME: Draw the flower stalk: */

  dest.x = x - 16;
  dest.y = y - 8;
  dest.w = 32;
  dest.h = 16;

  SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format, 0, 128, 0));
}

void flower_drawstalk(magic_api * api, SDL_Surface * canvas,
		      int minx, int maxx, int endx, int starty, int endy)
{
/* flower_min_x, flower_max_x, x, flower_bottom_y, y */

  int y, h;
  SDL_Rect dest;

  h = endy - starty;

  for (y = starty; y < starty + (h / 2); y++)
  {
    dest.x = minx;
    dest.y = y;
    dest.w = 2;
    dest.h = 2;

    SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format, 128, 255, 128));
  }

  for (y = starty + (h / 2); y < endy; y++)
  {
    dest.x = maxx;
    dest.y = y;
    dest.w = 2;
    dest.h = 2;

    SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format, 64, 192, 64));
  }
}

// No setup happened:
void flower_shutdown(magic_api * api)
{
  if (flower_snd != NULL)
    Mix_FreeChunk(flower_snd);
}

// Record the color from Tux Paint:
void flower_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
  flower_r = r;
  flower_g = g;
  flower_b = b;
}

// Use colors:
int flower_requires_colors(magic_api * api, int which)
{
  return 1;
}

