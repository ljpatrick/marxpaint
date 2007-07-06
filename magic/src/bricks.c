#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

/* What tools we contain: */

enum {
  TOOL_LARGEBRICKS,
  TOOL_SMALLBRICKS,
  NUM_TOOLS
};


/* Our globals: */

Mix_Chunk * brick_snd;
Uint8 bricks_r, bricks_g, bricks_b;


/* Local function prototype: */

void do_brick(magic_api * api, SDL_Surface * canvas,
              int x, int y, int w, int h);


// No setup required:
int bricks_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/brick.wav",
	    api->data_directory);
  brick_snd = Mix_LoadWAV(fname);

  return(1);
}

// We have multiple tools:
int bricks_get_tool_count(magic_api * api)
{
  return(NUM_TOOLS);
}

// Load our icons:
SDL_Surface * bricks_get_icon(magic_api * api, int which)
{
  char fname[1024];

  if (which == TOOL_LARGEBRICKS)
  {
    snprintf(fname, sizeof(fname), "%s/images/magic/largebrick.png",
	     api->data_directory);
  }
  else if (which == TOOL_SMALLBRICKS)
  {
    snprintf(fname, sizeof(fname), "%s/images/magic/smallbrick.png",
	     api->data_directory);
  }

  return(IMG_Load(fname));
}

// Return our names, localized:
char * bricks_get_name(magic_api * api, int which)
{
  /* Both are named "Bricks", at the moment: */

  return(strdup(gettext("Bricks")));
}

// Return our descriptions, localized:
char * bricks_get_description(magic_api * api, int which)
{
  if (which == TOOL_LARGEBRICKS)
    return(strdup(gettext("Click and move to draw large bricks.")));
  else if (which == TOOL_SMALLBRICKS)
    return(strdup(gettext("Click and move to draw small bricks.")));

  return(NULL);
}

// Do the effect:

void do_bricks(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
                int x, int y)
{
  magic_api * api = (magic_api *) ptr;

  // "specified" means the brick itself, w/o morter
  // "nominal" means brick-to-brick (includes morter)
  int specified_width, specified_height, specified_length;
  int nominal_length;
  int brick_x, brick_y;

  int vertical_joint = 2;   // between a brick and the one above/below
  int horizontal_joint = 2; // between a brick and the one to the side
  int nominal_width = 18;
  int nominal_height = 12;  // 11 to 14, for joints of 2
  static unsigned char *map;
  static int x_count;
  static int y_count;
  unsigned char *mybrick;

  if (which == TOOL_LARGEBRICKS)
  {
    vertical_joint = 4;     // between a brick and the one above/below
    horizontal_joint = 4;   // between a brick and the one to the side
    nominal_width = 36;
    nominal_height = 24;    // 11 to 14, for joints of 2
  }

  nominal_length = 2 * nominal_width;
  specified_width = nominal_width - horizontal_joint;
  specified_height = nominal_height - vertical_joint;
  specified_length = nominal_length - horizontal_joint;

  if (!api->button_down())
  {
    if (map)
      free(map);
    // the "+ 3" allows for both ends and misalignment
    x_count = (canvas->w + nominal_width - 1) / nominal_width + 3;
    y_count = (canvas->h + nominal_height - 1) / nominal_height + 3;
    map = calloc(x_count, y_count);
  }

  brick_x = x / nominal_width;
  brick_y = y / nominal_height;

  mybrick = map + brick_x + 1 + (brick_y + 1) * x_count;

  if ((unsigned) x < (unsigned) canvas->w
      && (unsigned) y < (unsigned) canvas->h && !*mybrick)
  {
    int my_x = brick_x * nominal_width;
    int my_w = specified_width;
    *mybrick = 1;


    // FIXME:
    //SDL_LockSurface(canvas);

    if ((brick_y ^ brick_x) & 1)
    {
      if (mybrick[1])
        my_w = specified_length;
    }
    else if (mybrick[-1])
    {
      my_x -= nominal_width;
      my_w = specified_length;
    }
    do_brick(api, canvas, my_x, brick_y * nominal_height,
	     my_w, specified_height);
    

    // FIXME:
    // SDL_UnlockSurface(canvas);

    //            upper left corner        and     lower right corner

    // FIXME
    /*
    update_canvas(brick_x * nominal_width - nominal_width,
                  brick_y * nominal_height - vertical_joint,
                  brick_x * nominal_width + specified_length,
                  (brick_y + 1) * nominal_height);
    */
  }
}

// Affect the canvas on drag:
void bricks_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y)
{
  api->line(which, canvas, last, ox, oy, x, y, 1, do_bricks);

  api->playsound(brick_snd, (x * 255) / canvas->w, 255);
}

// Affect the canvas on click:
void bricks_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y)
{
  bricks_drag(api, which, canvas, last, x, y, x, y);
}

// No setup happened:
void bricks_shutdown(magic_api * api)
{
  if (brick_snd != NULL)
    Mix_FreeChunk(brick_snd);
}

// Record the color from Tux Paint:
void bricks_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
  bricks_r = r;
  bricks_g = g;
  bricks_b = b;
}

// Use colors:
int bricks_requires_colors(magic_api * api, int which)
{
  return 1;
}

void do_brick(magic_api * api, SDL_Surface * canvas,
              int x, int y, int w, int h)
{
  SDL_Rect dest;

  // brick color: 127,76,73
  double ran_r = rand() / (double) RAND_MAX;
  double ran_g = rand() / (double) RAND_MAX;
  double base_r =
    api->sRGB_to_linear(bricks_r) * 1.5 +
    api->sRGB_to_linear(127) * 5.0 + ran_r;
  double base_g =
    api->sRGB_to_linear(bricks_g) * 1.5 +
    api->sRGB_to_linear(76) * 5.0 + ran_g;
  double base_b =
    api->sRGB_to_linear(bricks_b) * 1.5 +
    api->sRGB_to_linear(73) * 5.0 + (ran_r + ran_g * 2.0) / 3.0;

  Uint8 r = api->linear_to_sRGB(base_r / 7.5);
  Uint8 g = api->linear_to_sRGB(base_g / 7.5);
  Uint8 b = api->linear_to_sRGB(base_b / 7.5);

  dest.x = x;
  dest.y = y;
  dest.w = w;
  dest.h = h;


  SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format, r, g, b));


  /* Note: We only play the brick sound when we actually DRAW a brick: */

  api->playsound(brick_snd, (x * 255) / canvas->w, 255);
}
