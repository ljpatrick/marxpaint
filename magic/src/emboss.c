#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

/* Our globals: */

Mix_Chunk * emboss_snd;


Uint32 emboss_api_version(void) { return(TP_MAGIC_API_VERSION); }


// No setup required:
int emboss_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/emboss.wav",
	    api->data_directory);
  emboss_snd = Mix_LoadWAV(fname);

  return(1);
}

// We have multiple tools:
int emboss_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icons:
SDL_Surface * emboss_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/emboss.png",
	   api->data_directory);

  return(IMG_Load(fname));
}

// Return our names, localized:
char * emboss_get_name(magic_api * api, int which)
{
  return(strdup(gettext("Emboss")));
}

// Return our descriptions, localized:
char * emboss_get_description(magic_api * api, int which)
{
  return(strdup(gettext("Click and drag the mouse to emboss the picture.")));
}

// Do the effect:

void do_emboss(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
                int x, int y)
{
  magic_api * api = (magic_api *) ptr;
  int xx, yy;
  Uint8 r1, g1, b1,
        r2, g2, b2;
  int r, g, b;
  float h, s, v;
  int avg1, avg2;

  for (yy = -16; yy < 16; yy++)
  {
    for (xx = -16; xx < 16; xx++)
    {
      if (api->in_circle(xx, yy, 16))
      {
        if (!api->touched(x + xx, y + yy))
        {
          SDL_GetRGB(api->getpixel(last, x + xx, y + yy), last->format, &r1, &g1, &b1);
          SDL_GetRGB(api->getpixel(last, x + xx + 2, y + yy + 2), last->format, &r2, &g2, &b2);

          avg1 = (r1 + g1 + b1) / 3;
          avg2 = (r2 + g2 + b2) / 3;

	  api->rgbtohsv(r1, g1, b1, &h, &s, &v);

          r = 128 + (((avg1 - avg2) * 3) / 2);
          if (r < 0) r = 0;
          if (r > 255) r = 255;
          g = b = r;

	  v = (r / 255.0);

	  api->hsvtorgb(h, s, v, &r1, &g1, &b1);

          api->putpixel(canvas, x + xx, y + yy, SDL_MapRGB(canvas->format, r1, g1, b1));
        }
      }
    }
  }
}

// Affect the canvas on drag:
void emboss_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{
  api->line(which, canvas, last, ox, oy, x, y, 1, do_emboss);

  if (ox > x) { int tmp = ox; ox = x; x = tmp; }
  if (oy > y) { int tmp = oy; oy = y; y = tmp; }

  update_rect->x = ox - 16;
  update_rect->y = oy - 16;
  update_rect->w = (x + 16) - update_rect->x;
  update_rect->h = (y + 16) - update_rect->h;

  api->playsound(emboss_snd,
                 (x * 255) / canvas->w, (y * 255) / canvas->h);
}

// Affect the canvas on click:
void emboss_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
  emboss_drag(api, which, canvas, last, x, y, x, y, update_rect);
}

// Affect the canvas on release:
void emboss_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
}

// No setup happened:
void emboss_shutdown(magic_api * api)
{
  if (emboss_snd != NULL)
    Mix_FreeChunk(emboss_snd);
}

// Record the color from Tux Paint:
void emboss_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int emboss_requires_colors(magic_api * api, int which)
{
  return 0;
}

