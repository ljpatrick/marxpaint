#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

/* Our globals: */

#define NUM_RAINBOW_COLORS 23

const int rainbow_hexes[NUM_RAINBOW_COLORS][3] = {
  {255, 0, 0},
  {255, 64, 0},
  {255, 128, 0},
  {255, 192, 0},
  {255, 255, 0},
  {192, 255, 0},
  {128, 255, 0},
  {64, 255, 0},
  {0, 255, 0},
  {0, 255, 64},
  {0, 255, 128},
  {0, 255, 192},
  {0, 255, 255},
  {0, 192, 255},
  {0, 128, 255},
  {0, 64, 255},
  {64, 0, 255},
  {128, 0, 255},
  {192, 0, 255},
  {255, 0, 255},
  {255, 0, 192},
  {255, 0, 128},
  {255, 0, 64}
};

int rainbow_color;
Uint32 rainbow_rgb;
Mix_Chunk * rainbow_snd;

// Load our sfx:
int rainbow_init(magic_api * api)
{
  char fname[1024];


  rainbow_color = 0;

  snprintf(fname, sizeof(fname), "%s/sounds/magic/rainbow.wav",
	   api->data_directory);
  rainbow_snd = Mix_LoadWAV(fname);

  return(1);
}

Uint32 rainbow_api_version(void) { return(TP_MAGIC_API_VERSION); }

// We have multiple tools:
int rainbow_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icons:
SDL_Surface * rainbow_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/rainbow.png",
	   api->data_directory);

  return(IMG_Load(fname));
}

// Return our names, localized:
char * rainbow_get_name(magic_api * api, int which)
{
  return(strdup(gettext("Rainbow")));
}

// Return our descriptions, localized:
char * rainbow_get_description(magic_api * api, int which)
{
  return(strdup(
	   gettext("You can draw in rainbow colors!")));
}

// Do the effect:

void rainbow_linecb(void * ptr, int which,
		    SDL_Surface * canvas, SDL_Surface * last,
                    int x, int y)
{
  magic_api * api = (magic_api *) ptr;
  int xx, yy;

  for (yy = y - 16; yy < y + 16; yy++)
  {
    for (xx = x - 16; xx < x + 16; xx++)
    {
      if (api->in_circle(xx - x, yy - y, 16))
      {
        api->putpixel(canvas, xx, yy, rainbow_rgb);
      }
    }
  }
}

// Affect the canvas on drag:
void rainbow_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{
  rainbow_color = (rainbow_color + 1) % NUM_RAINBOW_COLORS;
  rainbow_rgb = SDL_MapRGB(canvas->format,
			   rainbow_hexes[rainbow_color][0],
			   rainbow_hexes[rainbow_color][1],
			   rainbow_hexes[rainbow_color][2]);

  api->line(which, canvas, last, ox, oy, x, y, 1, rainbow_linecb);

  if (ox > x) { int tmp = ox; ox = x; x = tmp; }
  if (oy > y) { int tmp = oy; oy = y; y = tmp; }

  update_rect->x = ox - 16;
  update_rect->y = oy - 16;
  update_rect->w = (x + 16) - update_rect->x;
  update_rect->h = (y + 16) - update_rect->y;

  api->playsound(rainbow_snd, (x * 255) / canvas->w, 255);
}

// Affect the canvas on click:
void rainbow_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y,
		   SDL_Rect * update_rect)
{
  rainbow_drag(api, which, canvas, last, x, y, x, y, update_rect);
}

void rainbow_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y,
		   SDL_Rect * update_rect)
{
}

// Clean up
void rainbow_shutdown(magic_api * api)
{
  if (rainbow_snd != NULL)
    Mix_FreeChunk(rainbow_snd);
}

// Record the color from Tux Paint:
void rainbow_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int rainbow_requires_colors(magic_api * api, int which)
{
  return 0;
}

