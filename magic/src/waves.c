#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <math.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"


/* Our globals: */

Mix_Chunk * waves_snd;


Uint32 waves_api_version(void) { return(TP_MAGIC_API_VERSION); }


// No setup required:
int waves_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/waves.wav",
	    api->data_directory);
  waves_snd = Mix_LoadWAV(fname);

  return(1);
}

// We have multiple tools:
int waves_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icons:
SDL_Surface * waves_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/waves.png",
	   api->data_directory);

  return(IMG_Load(fname));
}

// Return our names, localized:
char * waves_get_name(magic_api * api, int which)
{
  return(strdup(gettext("Waves")));
}

// Return our descriptions, localized:
char * waves_get_description(magic_api * api, int which)
{
  return(strdup(gettext("Click to make the picture wavy. Click toward the top for shorter waves, the bottom for taller waves, the left for small waves, and the right for long waves.")));
}


void waves_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{
  int xx, yy;
  SDL_Rect src, dest;
  int width;
  int height;

  SDL_BlitSurface(last, NULL, canvas, NULL);

  width = ((x * 10) / canvas->w) + 10;
  height = ((canvas->h - y) / 10) + 1;
  
  for (yy = 0; yy < canvas->h; yy++)
  {
    xx = sin((yy * height) * M_PI / 180.0) * width;

    src.x = 0;
    src.y = yy;
    src.w = canvas->w;
    src.h = 1;

    dest.x = xx;
    dest.y = yy;

    SDL_BlitSurface(last, &src, canvas, &dest);
  }

  update_rect->x = 0;
  update_rect->y = 0;
  update_rect->w = canvas->w;
  update_rect->h = canvas->h;
}

// Affect the canvas on click:
void waves_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
  waves_drag(api, which, canvas, last, x, y, x, y, update_rect);
}

// Affect the canvas on release:
void waves_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
}

// No setup happened:
void waves_shutdown(magic_api * api)
{
  if (waves_snd != NULL)
    Mix_FreeChunk(waves_snd);
}

// Record the color from Tux Paint:
void waves_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int waves_requires_colors(magic_api * api, int which)
{
  return 0;
}
