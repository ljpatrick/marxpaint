#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"

/* What tools we contain: */

enum {
  TOOL_BLOCKS,
  TOOL_CHALK,
  TOOL_DRIP,
  NUM_TOOLS
};


/* Our globals: */

Mix_Chunk * snd_effect[NUM_TOOLS];


int blocks_chalk_drip_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/blocks.wav",
	    api->data_directory);
  snd_effect[0] = Mix_LoadWAV(fname);

  snprintf(fname, sizeof(fname), "%s/sounds/magic/chalk.wav",
	    api->data_directory);
  snd_effect[1] = Mix_LoadWAV(fname);

  snprintf(fname, sizeof(fname), "%s/sounds/magic/drip.wav",
	    api->data_directory);
  snd_effect[2] = Mix_LoadWAV(fname);

  return(1);
}

Uint32 blocks_chalk_drip_api_version(void) { return(TP_MAGIC_API_VERSION); }


// We have multiple tools:
int blocks_chalk_drip_get_tool_count(magic_api * api)
{
  return(NUM_TOOLS);
}

// Load our icons:
SDL_Surface * blocks_chalk_drip_get_icon(magic_api * api, int which)
{
  char fname[1024];

  if (which == TOOL_BLOCKS)
  {
    snprintf(fname, sizeof(fname), "%s/images/magic/blocks.png",
	     api->data_directory);
  }
  else if (which == TOOL_CHALK)
  {
    snprintf(fname, sizeof(fname), "%s/images/magic/chalk.png",
	     api->data_directory);
  }
  else if (which == TOOL_DRIP)
  {
    snprintf(fname, sizeof(fname), "%s/images/magic/drip.png",
	     api->data_directory);
  }

  return(IMG_Load(fname));
}

// Return our names, localized:
char * blocks_chalk_drip_get_name(magic_api * api, int which)
{
  if (which == TOOL_BLOCKS)
    return(strdup(gettext("Blocks")));
  else if (which == TOOL_CHALK)
    return(strdup(gettext("Chalk")));
  else if (which == TOOL_DRIP)
    return(strdup(gettext("Drip")));

  return(NULL);
}

// Return our descriptions, localized:
char * blocks_chalk_drip_get_description(magic_api * api, int which)
{
  if (which == TOOL_BLOCKS)
    return(strdup(gettext(
"Click and move the mouse around to turn the picture into a chalk drawing.")));
  else if (which == TOOL_CHALK)
    return(strdup(gettext(
"Click and move the mouse around to make the picture blocky.")));
  else if (which == TOOL_DRIP)
    return(strdup(gettext(
"Click and move the mouse around to make the picture drip.")));

  return(NULL);
}

// Do the effect:

void blocks_chalk_drip_linecb(void * ptr, int which,
			      SDL_Surface * canvas, SDL_Surface * last,
	                      int x, int y)
{
  magic_api * api = (magic_api *) ptr;
  int xx, yy;
  int h;
  SDL_Rect src, dest;
  Uint8 r, g, b;
  Uint32 colr;

  if (which == TOOL_BLOCKS)
  {
    /* Put x/y on exact grid points: */

    x = (x / 4) * 4;
    y = (y / 4) * 4;

    for (yy = y - 8; yy < y + 8; yy = yy + 4)
    {
      for (xx = x - 8; xx < x + 8; xx = xx + 4)
      {
        Uint32 pix[16];
        Uint32 p_or = 0;
        Uint32 p_and = ~0;
        unsigned i = 16;
        while (i--)
        {
          Uint32 p_tmp;
          p_tmp = api->getpixel(last, xx + (i >> 2), yy + (i & 3));
          p_or |= p_tmp;
          p_and &= p_tmp;
          pix[i] = p_tmp;
        }
        if (p_or == p_and)    // if all pixels the same already
        {
          SDL_GetRGB(p_or, last->format, &r, &g, &b);
        }
        else                  // nope, must average them
        {
          double r_sum = 0.0;
          double g_sum = 0.0;
          double b_sum = 0.0;
          i = 16;
          while (i--)
          {
            SDL_GetRGB(pix[i], last->format, &r, &g, &b);
            r_sum += api->sRGB_to_linear(r);
            g_sum += api->sRGB_to_linear(g);
            b_sum += api->sRGB_to_linear(b);
          }
          r = api->linear_to_sRGB(r_sum / 16.0);
          g = api->linear_to_sRGB(g_sum / 16.0);
          b = api->linear_to_sRGB(b_sum / 16.0);
        }

        /* Draw block: */

        dest.x = xx;
        dest.y = yy;
        dest.w = 4;
        dest.h = 4;

        SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format, r, g, b));
      }
    }
  }
  else if (which == TOOL_CHALK)
  {

    for (yy = y - 8; yy <= y + 8; yy = yy + 4)
    {
      for (xx = x - 8; xx <= x + 8; xx = xx + 4)
      {
        dest.x = xx + ((rand() % 5) - 2);
        dest.y = yy + ((rand() % 5) - 2);
        dest.w = (rand() % 4) + 2;
        dest.h = (rand() % 4) + 2;

        colr = api->getpixel(last, clamp(0, xx, canvas->w - 1),
                             clamp(0, yy, canvas->h - 1));
        SDL_FillRect(canvas, &dest, colr);
      }
    }
  }
  else if (which == TOOL_DRIP)
  {
    for (xx = x - 8; xx <= x + 8; xx++)
    {
      h = (rand() % 8) + 8;

      for (yy = y; yy <= y + h; yy++)
      {
        src.x = xx;
        src.y = y;
        src.w = 1;
        src.h = 16;

        dest.x = xx;
        dest.y = yy;

        SDL_BlitSurface(last, &src, canvas, &dest);
      }
    }
  }
}

// Affect the canvas on drag:
void blocks_chalk_drip_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y)
{
  api->line(which, canvas, last, ox, oy, x, y, 1, blocks_chalk_drip_linecb);

  api->playsound(snd_effect[which], (x * 255) / canvas->w, 255);
}

// Affect the canvas on click:
void blocks_chalk_drip_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y)
{
  blocks_chalk_drip_drag(api, which, canvas, last, x, y, x, y);
}

// No setup happened:
void blocks_chalk_drip_shutdown(magic_api * api)
{
  if (snd_effect[0] != NULL)
    Mix_FreeChunk(snd_effect[0]);

  if (snd_effect[1] != NULL)
    Mix_FreeChunk(snd_effect[1]);
}

// Record the color from Tux Paint:
void blocks_chalk_drip_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int blocks_chalk_drip_requires_colors(magic_api * api, int which)
{
  return 0;
}

