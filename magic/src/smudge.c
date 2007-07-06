#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

/* Our globals: */

Mix_Chunk * smudge_snd;



// No setup required:
int smudge_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/smudge.wav",
	    api->data_directory);
  smudge_snd = Mix_LoadWAV(fname);

  return(1);
}

// We have multiple tools:
int smudge_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icons:
SDL_Surface * smudge_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/smudge.png",
	   api->data_directory);

  return(IMG_Load(fname));
}

// Return our names, localized:
char * smudge_get_name(magic_api * api, int which)
{
  return(strdup(gettext("Smudge")));
}

// Return our descriptions, localized:
char * smudge_get_description(magic_api * api, int which)
{
  return(strdup(gettext(
"Click and move the mouse around to smudge the picture.")));
}

// Do the effect:

void do_smudge(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
                int x, int y)
{
  magic_api * api = (magic_api *) ptr;
  static double state[32][32][3];
  unsigned i = 32 * 32;
  double rate = api->button_down() ? 0.5 : 0.0;
  Uint8 r, g, b;

  while (i--)
  {
    int iy = i >> 5;
    int ix = i & 0x1f;
    // is it not on the circle of radius sqrt(120) at location 16,16?
    if ((ix - 16) * (ix - 16) + (iy - 16) * (iy - 16) > 120)
      continue;
    // it is on the circle, so grab it

    SDL_GetRGB(api->getpixel(canvas, x + ix - 16, y + iy - 16),
               last->format, &r, &g, &b);
    state[ix][iy][0] =
      rate * state[ix][iy][0] + (1.0 - rate) * api->sRGB_to_linear(r);
    state[ix][iy][1] =
      rate * state[ix][iy][1] + (1.0 - rate) * api->sRGB_to_linear(g);
    state[ix][iy][2] =
      rate * state[ix][iy][2] + (1.0 - rate) * api->sRGB_to_linear(b);

    // opacity 100% --> new data not blended w/ existing data
    api->putpixel(canvas, x + ix - 16, y + iy - 16,
             SDL_MapRGB(canvas->format, api->linear_to_sRGB(state[ix][iy][0]),
                        api->linear_to_sRGB(state[ix][iy][1]),
                        api->linear_to_sRGB(state[ix][iy][2])));
  }
}

// Affect the canvas on drag:
void smudge_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y)
{
  api->line(which, canvas, last, ox, oy, x, y, 1, do_smudge);

  api->playsound(smudge_snd, (x * 255) / canvas->w, 255);
}

// Affect the canvas on click:
void smudge_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y)
{
  smudge_drag(api, which, canvas, last, x, y, x, y);
}

// No setup happened:
void smudge_shutdown(magic_api * api)
{
  if (smudge_snd != NULL)
    Mix_FreeChunk(smudge_snd);
}

// Record the color from Tux Paint:
void smudge_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int smudge_requires_colors(magic_api * api, int which)
{
  return 0;
}

