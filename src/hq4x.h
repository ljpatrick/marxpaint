#ifndef HQ4X_H
#define HQ4X_H

#include "SDL.h"

void hq4x_32(SDL_Surface * src, SDL_Surface * dest,
	     int LUT16to32[65536], int RGBtoYUV[65536]);

#endif
