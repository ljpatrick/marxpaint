#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"


/* Our globals: */

Mix_Chunk * blur_snd;


int blur_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/blur.wav",
	    api->data_directory);
  blur_snd = Mix_LoadWAV(fname);

  return(1);
}

Uint32 blur_api_version(void) { return(TP_MAGIC_API_VERSION); }

int blur_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icons:
SDL_Surface * blur_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/blur.png",
	   api->data_directory);

  return(IMG_Load(fname));
}

// Return our names, localized:
char * blur_get_name(magic_api * api, int which)
{
  return(strdup(gettext("Blur")));
}

// Return our descriptions, localized:
char * blur_get_description(magic_api * api, int which)
{
  return(strdup(gettext(
"Click and move the mouse around to blur the picture.")));
}

// Do the effect:

void do_blur(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
             int x, int y)
{
  magic_api * api = (magic_api *) ptr;
  double state[32][32][3];
  unsigned i;
  Uint8 r, g, b;

  i = 32 * 32;

  while (i--)
  {
    int iy = i >> 5;
    int ix = i & 0x1f;
    // is it not on the circle of radius sqrt(220) at location 16,16?
    if ((ix - 16) * (ix - 16) + (iy - 16) * (iy - 16) > 220)
      continue;
    // it is on the circle, so grab it

    SDL_GetRGB(api->getpixel(canvas, x + ix - 16, y + iy - 16),
               last->format, &r, &g, &b);
    state[ix][iy][0] = api->sRGB_to_linear(r);
    state[ix][iy][1] = api->sRGB_to_linear(g);
    state[ix][iy][2] = api->sRGB_to_linear(b);
  }

  i = 32 * 32;
  while (i--)
  {
    double lr, lg, lb;      // linear red,green,blue
    double weight;
    int iy = i >> 5;
    int ix = i & 0x1f;
    int r2 = (ix - 16) * (ix - 16) + (iy - 16) * (iy - 16); // radius squared

    // is it not on the circle of radius sqrt(140) at location 16,16?
    if (r2 > 140)
      continue;

    // It is on the circle, but how strongly will it be affected?
    // This is lame; we should use something like a gaussian or cosine
    // via a lookup table. (inverted, because this is the center weight)
    weight = r2 / 16.0 + 3.0;

    // Sampling more points would be good too, though it'd be slower.

    lr = state[ix][iy - 1][0]
      + state[ix - 1][iy][0] + state[ix][iy][0] * weight + state[ix +
                                                                 1][iy][0]
      + state[ix][iy + 1][0];

    lg = state[ix][iy - 1][1]
      + state[ix - 1][iy][1] + state[ix][iy][1] * weight + state[ix +
                                                                 1][iy][1]
      + state[ix][iy + 1][1];

    lb = state[ix][iy - 1][2]
      + state[ix - 1][iy][2] + state[ix][iy][2] * weight + state[ix +
                                                                 1][iy][2]
      + state[ix][iy + 1][2];

    lr /= weight + 4.0;
    lg /= weight + 4.0;
    lb /= weight + 4.0;
    api->putpixel(canvas, x + ix - 16, y + iy - 16,
             SDL_MapRGB(canvas->format, api->linear_to_sRGB(lr),
                        api->linear_to_sRGB(lg), api->linear_to_sRGB(lb)));
  }
}

// Affect the canvas on drag:
void blur_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y)
{
  api->line(which, canvas, last, ox, oy, x, y, 1, do_blur);

  api->playsound(blur_snd, (x * 255) / canvas->w, 255);
}

// Affect the canvas on click:
void blur_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y)
{
  blur_drag(api, which, canvas, last, x, y, x, y);
}

void blur_shutdown(magic_api * api)
{
  if (blur_snd != NULL)
    Mix_FreeChunk(blur_snd);
}

// Record the color from Tux Paint:
void blur_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int blur_requires_colors(magic_api * api, int which)
{
  return 0;
}

