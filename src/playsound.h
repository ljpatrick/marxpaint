#ifndef PLAYSOUND_H
#define PLAYSOUND_H

#include "SDL_mixer.h"
#include "sounds.h"

extern Mix_Chunk * sounds[NUM_SOUNDS];
extern int mute, use_sound;

void playsound(int chan, int s, int override);

#endif

