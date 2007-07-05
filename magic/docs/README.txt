Stub docs for Tux Paint Magic Plugin Development

#include "tp_magic_api.h"


build plugins with:
-------------------
  Linux/Unix:
  -----------
    $(CC) -shared `tp-magic-config --cflags` plugin.c -o plugin.so

    Then install globally into:  /usr/[local/]lib/tuxpaint/.
    Or locally into:  ~/.tuxpaint/magic/  [[FIXME]]

  Windows:
  --------
    ??? [[FIXME]]

  Mac OS X:
  ---------
    ??? [[FIXME]]


magic plugins must provide:
---------------------------
  NOTE: Each function name should be preceded with the name of the
  shared object file, e.g. "negative.so" or "negative.dll" would have
  a function called "negative_init()".

  int get_tool_count(magic_api * api)
    return the number of Magic tools this plugin provides
    (used below as the 'which' values sent to each function)

  char * get_name(magic_api * api, int which)
    return the name of a/the magic tool (for 'Magic' tool buttons in UI)
    Tux Paint will free() the string;
    example:
      return (strdup(gettext("Fun")));

  SDL_Surface * get_icon(magic_api * api, int which)
    return the icon of a/the magic tool (for 'Magic' tool buttons in UI)
    Tux Paint will SDL_FreeSurface() the surface:
    example:
      sprintf(fname, "%s/images/magic/funtool.png", api->data_directory);
      return(IMG_Load(fname));

  char * get_description(magic_api * api, int which)
    return the description of a/the magic tool (for Tux help text in UI);
    Tux Paint will free() the string;
    example:
      return (strdup(gettext("A fun tool")));

  int requires_colors(magic_api * api, int which)
    return whether a/the magic tool accepts colors
    ('1' for true; activates color palette in UI;
     '0' for false; disables color palette in UI)

  void set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 g)
    accept the color palette choice from Tux Paint
    (only called if requires_colors() for the current tool returned true)

  int init(magic_api * api)
    initialization function; called once, during Tux Paint startup
    return 1 if success;
    return 0 if failure (tool(s) will be disabled in Magic tool);

  void shutdown(magic_api * api)
    cleanup function; should free any alloc'd memory, etc.;
    happens once, at Tux Paint shutdown

  void click(magic_api * api, int which,
             SDL_Surface * snapshot, SDL_Surface * canvas,
             int x, int y)
    should affect 'canvas' at (x,y) location; may use 'snapshot' to fetch
    pixels from most recent undo buffer;
    Tux Paint's undo buffer is updated prior to this call

  void drag(magic_api * api, int which,
            SDL_Surface * snapshot, SDL_Surface * canvas,
            int ox, int oy, int x, int y)
    should affect 'canvas' between (ox,oy) and (x,y) locations;
    may use 'snapshot' to fetch pixels from most recent undo buffer;
    Tux Paint's undo buffer is NOT updated prior to this call; the
    'snapshot' buffer will contain the same contents as when click() was
    last called


tp provides, via magic_api structure ("api" arg to magic tool functions):
-------------------------------------------------------------------------
  void putpixel(SDL_Surface * surf, int x, int y, Uint32 pixel)
    function that puts a pixel at an (x,y) position in a surface

  Uint32 getpixel(SDL_Surface * surf, int x, int y)
    function that returns a pixel value from an (x,y) position on a surface

  int in_circle(int x, int y, int radius)
    function that returns whether an x/y position (centered at (0,0)) is
    within a circle of 'radius'

  void show_progress_bar(void)
    draws the Tux Paint progress bar animation; use while you're busy

  void tuxpaint_version(int * major, int * minor, int * revision)
    returns the version of Tux Paint being used

  void line(int which, SDL_Surface * canvas, SDL_Surface * snapshot,
            int x1, int y1, int x2, int y2, int step, FUNC callback)
    function that calls calculates a line between (x1,y1) and (x2,y2)
    and calls 'callback' every 'step' iterations;
    sends to the callback: Tux Paint's 'magic_api' structure, along with the
    'which', 'canvas' and 'snapshot' values sent to line(), and the (x,y)
    coordinates


FIXME: Implement these:

  void playsound(Mix_Chunk * snd, int pan, int dist)
    function that plays a sound, panned left/right 'pan'
    and at distance 'dist'.  pan may be SNDPOS_LEFT, SNDPOS_CENTER or
    SNDPOS_RIGHT, and dist may be SNDDIST_NEAR.

  void special_notify(int flag)
    notifies tux paint of special events; SPECIAL_FLIP and SPECIAL_MIRROR
    flags may be sent

  float sRGB_to_linear_table[256]
    sRGB-to-linear look-up table

  unsigned char linear_to_sRGB(float linear)
    linear-to-sRGB look-up helper function

