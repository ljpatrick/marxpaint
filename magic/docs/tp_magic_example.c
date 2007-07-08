#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

/* What tools we contain: */

enum {
  TOOL_ONE,
  TOOL_TWO,
  NUM_TOOLS
};


/* Our globals: */

Mix_Chunk * snd_effect[NUM_TOOLS];
Uint8 example_r, example_g, example_b;


Uint32 example_api_version(void) { return(TP_MAGIC_API_VERSION); }


// No setup required:
int example_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/one.wav",
	    api->data_directory);
  snd_effect[0] = Mix_LoadWAV(fname);

  snprintf(fname, sizeof(fname), "%s/sounds/magic/two.wav",
	    api->data_directory);
  snd_effect[1] = Mix_LoadWAV(fname);

  return(1);
}

// We have multiple tools:
int example_get_tool_count(magic_api * api)
{
  return(NUM_TOOLS);
}

// Load our icons:
SDL_Surface * example_get_icon(magic_api * api, int which)
{
  char fname[1024];

  if (which == TOOL_ONE)
  {
    snprintf(fname, sizeof(fname), "%s/images/magic/one.png",
	     api->data_directory);
  }
  else if (which == TOOL_TWO)
  {
    snprintf(fname, sizeof(fname), "%s/images/magic/two.png",
	     api->data_directory);
  }

  return(IMG_Load(fname));
}

// Return our names, localized:
char * example_get_name(magic_api * api, int which)
{
  if (which == TOOL_ONE)
    return(strdup(gettext("One")));
  else if (which == TOOL_TWO)
    return(strdup(gettext("Two")));

  return(NULL);
}

// Return our descriptions, localized:
char * example_get_description(magic_api * api, int which)
{
  if (which == TOOL_ONE)
    return(strdup(gettext("Tool one.")));
  else
    return(strdup(gettext("Tool two.")));

  return(NULL);
}

// Do the effect:

void do_example(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
                int x, int y)
{
  magic_api * api = (magic_api *) ptr;
  int xx, yy;

  if (which == TOOL_ONE)
  {
    api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format,
                                           example_r,
                                           example_g,
                                           example_b));
  }
  else if (which == TOOL_TWO)
  {
    for (yy = -4; yy < 4; yy++)
    {
      for (xx = -4; xx < 4; xx++)
      {
        api->putpixel(canvas, x + xx, y + yy,
		      api->getpixel(last,
				    canvas->w - x - xx,
		      		    canvas->h - y - yy));
      }
    }
  }
}

// Affect the canvas on drag:
void example_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y)
{
  api->line(which, canvas, last, ox, oy, x, y, 1, do_example);

  api->playsound(snd_effect[which],
                 (x * 255) / canvas->w, (y * 255) / canvas->h);
}

// Affect the canvas on click:
void example_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y)
{
  example_drag(api, which, canvas, last, x, y, x, y);
}

// No setup happened:
void example_shutdown(magic_api * api)
{
  if (snd_effect[0] != NULL)
    Mix_FreeChunk(snd_effect[0]);

  if (snd_effect[1] != NULL)
    Mix_FreeChunk(snd_effect[1]);
}

// Record the color from Tux Paint:
void example_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
  example_r = r;
  example_g = g;
  example_b = b;
}

// Use colors:
int example_requires_colors(magic_api * api, int which)
{
  return 1;
}

