#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

/* Our globals: */

Mix_Chunk * kalidescope_snd;
Uint8 kalidescope_r, kalidescope_g, kalidescope_b;


Uint32 kalidescope_api_version(void) { return(TP_MAGIC_API_VERSION); }


// No setup required:
int kalidescope_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/kalidescope.wav",
	    api->data_directory);
  kalidescope_snd = Mix_LoadWAV(fname);

  return(1);
}

// We have multiple tools:
int kalidescope_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icons:
SDL_Surface * kalidescope_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/kalidescope.png",
	   api->data_directory);

  return(IMG_Load(fname));
}

// Return our names, localized:
char * kalidescope_get_name(magic_api * api, int which)
{
  return(strdup(gettext("Kaleidoscope")));
}

// Return our descriptions, localized:
char * kalidescope_get_description(magic_api * api, int which)
{
  return(strdup(gettext("Click and drag the mouse to draw with symmetric brushes (a kaleidoscope).")));
}

// Do the effect:

void do_kalidescope(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
                int x, int y)
{
  magic_api * api = (magic_api *) ptr;
  int xx, yy;
  Uint32 colr;

  colr = SDL_MapRGB(canvas->format,
                    kalidescope_r,
                    kalidescope_g,
                    kalidescope_b);

  for (yy = -8; yy < 8; yy++)
  {
    for (xx = -8; xx < 8; xx++)
    {
      if (api->in_circle(xx, yy, 8))
      {
        api->putpixel(canvas, x + xx, y + yy, colr);
        api->putpixel(canvas, canvas->w - 1 - x + xx, y + yy, colr);
        api->putpixel(canvas, x + xx, canvas->h - 1 - y + yy, colr);
        api->putpixel(canvas, canvas->w - 1 - x + xx, canvas->h - 1 - y + yy, colr);
      }
    }
  }
}

// Affect the canvas on drag:
void kalidescope_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{
  api->line(which, canvas, last, ox, oy, x, y, 1, do_kalidescope);

  update_rect->x = 0;
  update_rect->y = 0;
  update_rect->w = canvas->w;
  update_rect->h = canvas->h;

  api->playsound(kalidescope_snd, 255, 255);
}

// Affect the canvas on click:
void kalidescope_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
  kalidescope_drag(api, which, canvas, last, x, y, x, y, update_rect);
}

// Affect the canvas on release:
void kalidescope_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
}

// No setup happened:
void kalidescope_shutdown(magic_api * api)
{
  if (kalidescope_snd != NULL)
    Mix_FreeChunk(kalidescope_snd);
}

// Record the color from Tux Paint:
void kalidescope_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
  kalidescope_r = r;
  kalidescope_g = g;
  kalidescope_b = b;
}

// Use colors:
int kalidescope_requires_colors(magic_api * api, int which)
{
  return 1;
}

