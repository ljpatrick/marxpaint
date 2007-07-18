#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"


/* Our globals: */

Mix_Chunk * sparkles_snd;
Uint8 sparkles_r, sparkles_g, sparkles_b;


Uint32 sparkles_api_version(void) { return(TP_MAGIC_API_VERSION); }


// No setup required:
int sparkles_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/sparkles.wav",
	    api->data_directory);
  sparkles_snd = Mix_LoadWAV(fname);

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
  int div;

  for (yy = -8; yy < 8; yy++)
  {
    for (xx = -8; xx < 8; xx++)
    {
      if (api->in_circle(xx, yy, 8))
      {
        pix = api->getpixel(canvas, x + xx, y + yy);

        SDL_GetRGB(pix, canvas->format, &r, &g, &b);

        div = abs(xx * yy);
        if (div == 0)
          div = 1;

	r = min(255, r + (sparkles_r / div));
	g = min(255, g + (sparkles_g / div));
	b = min(255, b + (sparkles_b / div));

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
  api->line(which, canvas, last, ox, oy, x, y, 1, do_sparkles);

  if (ox > x) { int tmp = ox; ox = x; x = tmp; }
  if (oy > y) { int tmp = oy; oy = y; y = tmp; }

  update_rect->x = ox - 8;
  update_rect->y = oy - 8;
  update_rect->w = (x + 8) - update_rect->x;
  update_rect->h = (y + 8) - update_rect->h;

  api->playsound(sparkles_snd, (x * 255) / canvas->w, 255);
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
  if (sparkles_snd != NULL)
    Mix_FreeChunk(sparkles_snd);
}

// Record the color from Tux Paint:
void sparkles_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
  sparkles_r = r + 10;
  sparkles_g = g + 10;
  sparkles_b = b + 10;
}

// Use colors:
int sparkles_requires_colors(magic_api * api, int which)
{
  return 1;
}

