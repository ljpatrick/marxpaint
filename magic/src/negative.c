#include <stdio.h>
#include <string.h>
#include "SDL_image.h"
#include "magic_helpers.h"

void init()
{
  printf("negative plugin initializing\n");
}

int get_tool_count(void)
{
  printf("negative tool reporting tool count: 1\n");
  return(1);
}

SDL_Surface * get_icon(int which)
{
  printf("Loading icon: " DATA_PREFIX "/images/magic/negative.png\n");
  return(IMG_Load(DATA_PREFIX "/images/magic/negative.png"));
}

char * get_name(int which)
{
  /* Only 1 tool; ignoring 'which' */

  return(strdup(gettext("Negative")));
}

char * get_description(int which)
{
  /* Only 1 tool; ignoring 'which' */

  return(strdup(gettext("Click and move the mouse around to draw a negative.")));
}

void drag(int which, SDL_Surface * canvas, SDL_Surface * last, int ox, int oy, int x, int y);

void click(int which, SDL_Surface * canvas, SDL_Surface * last, int x, int y)
{
/*
  SDL_Rect src, dest;

  src.x = x - 12;
  src.y = y - 12;
  src.w = 16;
  src.h = 16;

  dest.x = x - 8;
  dest.y = y - 8;
  dest.w = 16;
  dest.h = 16;

  SDL_BlitSurface(last, &src, canvas, &dest);
*/

  drag(which, canvas, last, x, y, x, y);
}

void drag(int which, SDL_Surface * canvas, SDL_Surface * last, int ox, int oy, int x, int y)
{
  int xx, yy;
  Uint8 r, g, b;
  void (*putpixel) (SDL_Surface *, int, int, Uint32) =
    MAGIC_putpixels[canvas->format->BytesPerPixel];
  Uint32(*getpixel_last) (SDL_Surface *, int, int);

  getpixel_last = MAGIC_getpixels[last->format->BytesPerPixel];


  SDL_LockSurface(last);
  SDL_LockSurface(canvas);

  for (yy = y - 16; yy < y + 16; yy++)
  {
    for (xx = x - 16; xx < x + 16; xx++)
    {
      if (MAGIC_in_circle(xx - x, yy - y, 16))
      {
        SDL_GetRGB(getpixel_last(last, xx, yy), last->format, &r, &g, &b);

        r = 0xFF - r;
        g = 0xFF - g;
        b = 0xFF - b;

        putpixel(canvas, xx, yy, SDL_MapRGB(canvas->format, r, g, b));
      }
    }
  }
  
  SDL_UnlockSurface(canvas);
  SDL_UnlockSurface(last);
}

void shutdown()
{
  printf("negative plugin shutting down\n");
}

void set_color(Uint8 r, Uint8 g, Uint8 b)
{
  /* Doesn't use color; ignoring */
}

int requires_colors(int which)
{
  /* (Only 1 tool; ignoring 'which') */

  return 0;  // Don't need a color palette
}

