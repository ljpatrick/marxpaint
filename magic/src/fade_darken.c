#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"

enum {
  TOOL_FADE,
  TOOL_DARKEN,
  NUM_TOOLS
};

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

// No setup required:
int fade_darken_init(magic_api * api)
{
  return(1);
}

// Multiple tools:
int fade_darken_get_tool_count(magic_api * api)
{
  return(NUM_TOOLS);
}

// Load our icon:
SDL_Surface * fade_darken_get_icon(magic_api * api, int which)
{
  char fname[1024];

  if (which == TOOL_FADE)
  {
    snprintf(fname, sizeof(fname), "%s/images/magic/fade.png",
	     api->data_directory);
  }
  else if (which == TOOL_DARKEN)
  {
    snprintf(fname, sizeof(fname), "%s/images/magic/darken.png",
	     api->data_directory);
  }

  return(IMG_Load(fname));
}

// Return our name, localized:
char * fade_darken_get_name(magic_api * api, int which)
{
  if (which == TOOL_FADE)
    return(strdup(gettext("Lighten")));
  else if (which == TOOL_DARKEN)
    return(strdup(gettext("Darken")));

  return(NULL);
}

// Return our description, localized:
char * fade_darken_get_description(magic_api * api, int which)
{
  if (which == TOOL_FADE)
    return(strdup(
           gettext("Click and move to fade the colors.")));
  else if (which == TOOL_DARKEN)
    return(strdup(
           gettext("Click and move to darken the colors.")));

  return(NULL);
}

// Callback that does the fade_darken color effect on a circle centered around x,y
void do_fade_darken(void * ptr, int which,
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

        if (which == TOOL_FADE)
        {
          r = min(r + 48, 255);
          g = min(g + 48, 255);
          b = min(b + 48, 255);
        }
        else if (which == TOOL_DARKEN)
        {
          r = max(r - 48, 0);
          g = max(g - 48, 0);
          b = max(b - 48, 0);
        }

        api->putpixel(canvas, xx, yy, SDL_MapRGB(canvas->format, r, g, b));
      }
    }
  }
}

// Ask Tux Paint to call our 'do_fade_darken()' callback over a line
void fade_darken_drag(magic_api * api, int which, SDL_Surface * canvas,
	           SDL_Surface * last, int ox, int oy, int x, int y)
{
  SDL_LockSurface(last);
  SDL_LockSurface(canvas);

  api->line(which, canvas, last, ox, oy, x, y, 1, do_fade_darken);
  
  SDL_UnlockSurface(canvas);
  SDL_UnlockSurface(last);
}

// Ask Tux Paint to call our 'do_fade_darken()' callback at a single point
void fade_darken_click(magic_api * api, int which,
	            SDL_Surface * canvas, SDL_Surface * last,
	            int x, int y)
{
  fade_darken_drag(api, which, canvas, last, x, y, x, y);
}


// No setup happened:
void fade_darken_shutdown(magic_api * api)
{
}

// We don't use colors
void fade_darken_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// We don't use colors
int fade_darken_requires_colors(magic_api * api, int which)
{
  return 0;
}

