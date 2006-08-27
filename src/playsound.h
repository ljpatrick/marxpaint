#ifndef PLAYSOUND_H
#define PLAYSOUND_H

#include "SDL.h"
#include "SDL_mixer.h"
#include "sounds.h"

#define SNDPOS_LEFT -997
#define SNDPOS_CENTER -998
#define SNDPOS_RIGHT -999

#define SNDDIST_NEAR -999

extern Mix_Chunk *sounds[NUM_SOUNDS];
extern int mute, use_sound;

void playsound(SDL_Surface * screen, int chan, int s, int override, int x,
	       int y);

#endif
