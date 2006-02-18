#include "playsound.h"

#ifndef NOSOUND
Mix_Chunk * sounds[NUM_SOUNDS];
#endif

int mute, use_sound;

void playsound(int chan, int s, int override)
{
#ifndef NOSOUND
  if (!mute && use_sound && s != SND_NONE)
    {
      if (override || !Mix_Playing(chan))
        Mix_PlayChannel(chan, sounds[s], 0);
    }
#endif
}

