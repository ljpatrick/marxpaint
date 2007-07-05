#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"

// No setup required:
int negative_init(magic_api * api)
{
  return(1);
}

// Only one tool:
int negative_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icon:
SDL_Surface * negative_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/negative.png",
	   api->data_directory);
  return(IMG_Load(fname));
}

// Return our name, localized:
char * negative_get_name(magic_api * api, int which)
{
  return(strdup(gettext("Negative")));
}

// Return our description, localized:
char * negative_get_description(magic_api * api, int which)
{
  return(strdup(
         gettext("Click and move the mouse around to draw a negative.")));
}

// Callback that does the negative color effect on a circle centered around x,y
void do_negative(void * ptr, int which,
	         SDL_Surface * canvas, SDL_Surface * last,
	         int x, int y)
{
  int xx, yy;
  Uint8 r, g, b;
  magic_api * api = (magic_api *) ptr;

  for (yy = y - 16; yy < y + 16; yy++)
  {
    for (xx = x - 16; xx < x + 16; xx++)
    {
      if (api->in_circle(xx - x, yy - y, 16))
      {
        SDL_GetRGB(api->getpixel(last, xx, yy), last->format, &r, &g, &b);

        r = 0xFF - r;
        g = 0xFF - g;
        b = 0xFF - b;

        api->putpixel(canvas, xx, yy, SDL_MapRGB(canvas->format, r, g, b));
      }
    }
  }
}

// Ask Tux Paint to call our 'do_negative()' callback over a line
void negative_drag(magic_api * api, int which, SDL_Surface * canvas,
	           SDL_Surface * last, int ox, int oy, int x, int y)
{
  SDL_LockSurface(last);
  SDL_LockSurface(canvas);

  api->line(which, canvas, last, ox, oy, x, y, 1, do_negative);
  
  SDL_UnlockSurface(canvas);
  SDL_UnlockSurface(last);
}

// Ask Tux Paint to call our 'do_negative()' callback at a single point
void negative_click(magic_api * api, int which,
	            SDL_Surface * canvas, SDL_Surface * last,
	            int x, int y)
{
  negative_drag(api, which, canvas, last, x, y, x, y);
}


// No setup happened:
void negative_shutdown(magic_api * api)
{
}

// We don't use colors
void negative_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// We don't use colors
int negative_requires_colors(magic_api * api, int which)
{
  return 0;
}

