#ifndef MAGIC_HELPERS_H
#define MAGIC_HELPERS_H

#include "SDL.h"
#include <libintl.h>

int MAGIC_in_circle(int x, int y, int radius);

void MAGIC_putpixel8(SDL_Surface * surface, int x, int y, Uint32 pixel);
void MAGIC_putpixel16(SDL_Surface * surface, int x, int y, Uint32 pixel);
void MAGIC_putpixel24(SDL_Surface * surface, int x, int y, Uint32 pixel);
void MAGIC_putpixel32(SDL_Surface * surface, int x, int y, Uint32 pixel);

extern void (*MAGIC_putpixels[]) (SDL_Surface *, int, int, Uint32);

Uint32 MAGIC_getpixel8(SDL_Surface * surface, int x, int y);
Uint32 MAGIC_getpixel16(SDL_Surface * surface, int x, int y);
Uint32 MAGIC_getpixel24(SDL_Surface * surface, int x, int y);
Uint32 MAGIC_getpixel32(SDL_Surface * surface, int x, int y);

extern Uint32(*MAGIC_getpixels[]) (SDL_Surface *, int, int);

#endif
