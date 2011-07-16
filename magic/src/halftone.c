/* halftone.c

   Last modified: 2011.07.15
*/


/* Inclusion of header files: */
/* -------------------------- */

#include <stdio.h>
#include <string.h>
#include <libintl.h>

#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

enum {
  TOOL_HALFTONE,
  NUM_TOOLS
};


const char * snd_filenames[NUM_TOOLS] = {
  "halftone.wav",
};

const char * icon_filenames[NUM_TOOLS] = {
  "halftone.png",
};

const char * names[NUM_TOOLS] = {
  gettext_noop("Halftone"),
};

const char * descs[NUM_TOOLS] = {
  gettext_noop("Click and drag to turn your drawing into a newspaper."),
};

Mix_Chunk * snd_effect[NUM_TOOLS];

void halftone_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * snapshot, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect);
void halftone_line_callback(void * ptr, int which,
                           SDL_Surface * canvas, SDL_Surface * snapshot,
                           int x, int y);


Uint32 halftone_api_version(void)
{
  return(TP_MAGIC_API_VERSION);
}

int halftone_init(magic_api * api)
{
  int i;
  char fname[1024];

  for (i = 0; i < NUM_TOOLS; i++)
  {
    snprintf(fname, sizeof(fname),
             "%s/sounds/magic/%s",
	     api->data_directory, snd_filenames[i]);

/* FIXME    snd_effect[i] = Mix_LoadWAV(fname); */
  }

  return(1);
}

int halftone_get_tool_count(magic_api * api)
{
  return(NUM_TOOLS);
}

SDL_Surface * halftone_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/%s.png",
	     api->data_directory, icon_filenames[which]);

  return(IMG_Load(fname));
}

char * halftone_get_name(magic_api * api, int which)
{
  const char * our_name_english;
  const char * our_name_localized;

  our_name_english = names[which];
  our_name_localized = gettext(our_name_english);

  return(strdup(our_name_localized));
}

char * halftone_get_description(magic_api * api, int which, int mode)
{
  const char * our_desc_english;
  const char * our_desc_localized;

  our_desc_english = descs[which];
  our_desc_localized = gettext(our_desc_english);

  return(strdup(our_desc_localized));
}

int halftone_requires_colors(magic_api * api, int which)
{
  return 0;
}

int halftone_modes(magic_api * api, int which)
{
  return MODE_PAINT;
}

void halftone_shutdown(magic_api * api)
{
  int i;

  for (i = 0; i < NUM_TOOLS; i++)
    Mix_FreeChunk(snd_effect[i]);
}

void halftone_click(magic_api * api, int which, int mode,
	           SDL_Surface * canvas, SDL_Surface * snapshot,
	           int x, int y, SDL_Rect * update_rect)
{
  halftone_drag(api, which, canvas, snapshot, x, y, x, y, update_rect);
}

void halftone_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * snapshot, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{
  api->line((void *) api, which, canvas, snapshot,
            ox, oy, x, y, 1, halftone_line_callback);

  if (ox > x) { int tmp = ox; ox = x; x = tmp; }
  if (oy > y) { int tmp = oy; oy = y; y = tmp; }

  update_rect->x = ox - 16;
  update_rect->y = oy - 16;
  update_rect->w = (x + 16) - update_rect->x;
  update_rect->h = (y + 16) - update_rect->h;

/* FIXME
  api->playsound(snd_effect[which],
                 (x * 255) / canvas->w, // pan
	         255); // distance
*/
}

enum {
  CHAN_CYAN,
  CHAN_MAGENTA,
  CHAN_YELLOW,
  CHAN_BLACK,
  NUM_CHANS
};

Uint8 chan_colors[NUM_CHANS][3] = {
  {   0, 255, 255 },
  { 255,   0, 255 },
  { 255, 255,   0 },
  {   0,   0,   0 }
};

int chan_angles[NUM_CHANS] = {
  100,
  15,
  0,
  45
};

void halftone_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * snapshot,
	           int x, int y, SDL_Rect * update_rect)
{
}

void halftone_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

void halftone_line_callback(void * ptr, int which,
                           SDL_Surface * canvas, SDL_Surface * snapshot,
                           int x, int y)
{
  Uint8 r, g, b;
  Uint32 total_r, total_g, total_b;
  Uint32 pixel;
  int xx, yy, xxx, yyy, channel;
  magic_api * api = (magic_api *) ptr;

  for (xx = x - 16; xx < x + 16; xx = xx + 4) {
    for (yy = y - 16; yy < y + 16; yy = yy + 4) {
      total_r = total_g = total_b = 0;
      for (xxx = xx; xxx < xx + 4; xxx++) {
        for (yyy = xx; yyy < xx + 4; yyy++) {
          SDL_GetRGB(api->getpixel(canvas, x, y), canvas->format, &r, &g, &b);
          total_r += r;
          total_g += g;
          total_b += b;
        }
      }
      total_r /= 16;
      total_g /= 16;
      total_b /= 16;

      for (channel = 0; channel < NUM_CHANS; channel++) {
        
      }

      pixel = SDL_MapRGB(canvas->format, total_r, total_g, total_b);
      for (xxx = xx; xxx < xx + 4; xxx++) {
        for (yyy = yy; yyy < yy + 4; yyy++) {
          api->putpixel(canvas, xxx, yyy, pixel);
        }
      }
    }
  }
}

void halftone_switchin(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
}

void halftone_switchout(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
}
