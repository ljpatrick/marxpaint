#ifndef TP_MAGIC_API_H
#define TP_MAGIC_API_H

#include "SDL.h"
#include "SDL_mixer.h"

#define SPECIAL_MIRROR	0x0001
#define SPECIAL_FLIP	0x0002

typedef struct magic_api_t {
  char * tp_version;
  char * data_directory;
  void (*update_progress_bar)(void);
  void (*special_notify)(int);
  float sRGB_to_linear_table[256];
  unsigned char (*linear_to_sRGB)(float);
  int (*in_circle)(int,int,int);
  Uint32 (*getpixel)(SDL_Surface *, int, int);
  void (*putpixel)(SDL_Surface *, int, int, Uint32);
  void (*playsound)(Mix_Chunk *, int, int);
  void (*line)(int, SDL_Surface *, SDL_Surface *, int, int, int, int, int, void (*)(void *, int, SDL_Surface *, SDL_Surface *, int, int));
} magic_api;

#endif
