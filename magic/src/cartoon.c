#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <math.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"


/* Our globals: */

Mix_Chunk * cartoon_snd;

#define OUTLINE_THRESH 48


// No setup required:
int cartoon_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/cartoon.wav",
	    api->data_directory);
  cartoon_snd = Mix_LoadWAV(fname);

  return(1);
}

// We have multiple tools:
int cartoon_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icons:
SDL_Surface * cartoon_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/cartoon.png",
	   api->data_directory);

  return(IMG_Load(fname));
}

// Return our names, localized:
char * cartoon_get_name(magic_api * api, int which)
{
  return(strdup(gettext("Cartoon")));
}

// Return our descriptions, localized:
char * cartoon_get_description(magic_api * api, int which)
{
  return(strdup(gettext(
"Click and move the mouse around to turn the picture into a cartoon.")));
}

// Do the effect:

void do_cartoon(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
                int x, int y)
{
  magic_api * api = (magic_api *) ptr;
  int xx, yy;
  Uint8 r1, g1, b1, r2, g2, b2;
  Uint8 r, g, b;
  float hue, sat, val;

  /* First, convert colors to more cartoony ones: */

  for (yy = y - 16; yy < y + 16; yy = yy + 1)
  {
    for (xx = x - 16; xx < x + 16; xx = xx + 1)
    {
      if (api->in_circle(xx - x, yy - y, 16))
      {
        /* Get original color: */

        SDL_GetRGB(api->getpixel(last, xx, yy), last->format, &r, &g, &b);

        api->rgbtohsv(r, g, b, &hue, &sat, &val);

        val = val - 0.5;
        val = val * 4;
        val = val + 0.5;

        if (val < 0)
          val = 0;
        else if (val > 1.0)
          val = 1.0;

        val = floor(val * 4) / 4;
        hue = floor(hue * 4) / 4;

        sat = floor(sat * 4) / 4;

        api->hsvtorgb(hue, sat, val, &r, &g, &b);

        api->putpixel(canvas, xx, yy, SDL_MapRGB(canvas->format, r, g, b));
      }
    }
  }

  /* Then, draw dark outlines where there's a large contrast change */

  for (yy = y - 16; yy < y + 16; yy++)
  {
    for (xx = x - 16; xx < x + 16; xx++)
    {
      if (api->in_circle(xx - x, yy - y, 16))
      {
        /* Get original color: */

        SDL_GetRGB(api->getpixel(last, xx, yy), last->format, &r, &g, &b);

        SDL_GetRGB(api->getpixel(last, xx + 1, yy),
                   last->format, &r1, &g1, &b1);

        SDL_GetRGB(api->getpixel(last, xx + 1, yy + 1),
                   last->format, &r2, &g2, &b2);

        if (abs(((r + g + b) / 3) - (r1 + g1 + b1) / 3) > OUTLINE_THRESH
            || abs(((r + g + b) / 3) - (r2 + g2 + b2) / 3) >
            OUTLINE_THRESH || abs(r - r1) > OUTLINE_THRESH
            || abs(g - g1) > OUTLINE_THRESH
            || abs(b - b1) > OUTLINE_THRESH
            || abs(r - r2) > OUTLINE_THRESH
            || abs(g - g2) > OUTLINE_THRESH
            || abs(b - b2) > OUTLINE_THRESH)
        {
          api->putpixel(canvas, xx - 1, yy,
                   SDL_MapRGB(canvas->format, 0, 0, 0));
          api->putpixel(canvas, xx, yy - 1,
                   SDL_MapRGB(canvas->format, 0, 0, 0));
          api->putpixel(canvas, xx - 1, yy - 1,
                   SDL_MapRGB(canvas->format, 0, 0, 0));
        }
      }
    }
  }
}

// Affect the canvas on drag:
void cartoon_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y)
{
  api->line(which, canvas, last, ox, oy, x, y, 1, do_cartoon);

  api->playsound(cartoon_snd, (x * 255) / canvas->w, 255);
}

// Affect the canvas on click:
void cartoon_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y)
{
  cartoon_drag(api, which, canvas, last, x, y, x, y);
}

// No setup happened:
void cartoon_shutdown(magic_api * api)
{
  if (cartoon_snd != NULL)
    Mix_FreeChunk(cartoon_snd);
}

// Record the color from Tux Paint:
void cartoon_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int cartoon_requires_colors(magic_api * api, int which)
{
  return 0;
}

