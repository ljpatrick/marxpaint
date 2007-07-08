#ifndef TP_MAGIC_API_H
#define TP_MAGIC_API_H

#include "SDL.h"
#include "SDL_mixer.h"

#ifdef __GNUC__
// This version has strict type checking for safety.
// See the "unnecessary" pointer comparison. (from Linux)
#define min(x,y) ({ \
  typeof(x) _x = (x);     \
  typeof(y) _y = (y);     \
  (void) (&_x == &_y);            \
  _x < _y ? _x : _y; })
#define max(x,y) ({ \
  typeof(x) _x = (x);     \
  typeof(y) _y = (y);     \
  (void) (&_x == &_y);            \
  _x > _y ? _x : _y; })
#else
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define clamp(lo,value,hi)    (min(max(value,lo),hi))

#define SPECIAL_MIRROR	0x0001
#define SPECIAL_FLIP	0x0002

typedef struct magic_api_t {
  char * tp_version;
  char * data_directory;
  void (*update_progress_bar)(void);
  void (*special_notify)(int);
  float (*sRGB_to_linear)(Uint8);
  Uint8 (*linear_to_sRGB)(float);
  int (*in_circle)(int,int,int);
  Uint32 (*getpixel)(SDL_Surface *, int, int);
  void (*putpixel)(SDL_Surface *, int, int, Uint32);
  void (*playsound)(Mix_Chunk *, int, int);
  void (*line)(int, SDL_Surface *, SDL_Surface *, int, int, int, int, int, void (*)(void *, int, SDL_Surface *, SDL_Surface *, int, int));
  int (*button_down)(void);
  void (*rgbtohsv)(Uint8, Uint8, Uint8, float *, float *, float *);
  void (*hsvtorgb)(float, float, float, Uint8 *, Uint8 *, Uint8 *);
} magic_api;

#define TP_MAGIC_API_VERSION MAGICAPI_VERSION

#endif

