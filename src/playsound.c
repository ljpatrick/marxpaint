#include "playsound.h"
#include "debug.h"

#ifndef NOSOUND
Mix_Chunk *sounds[NUM_SOUNDS];
#endif

int mute, use_sound;
int old_sound[4] = { -1, -1, -1, -1 };

void playsound(SDL_Surface * screen, int chan, int s, int override, int x,
	       int y)
{
#ifndef NOSOUND
  int left, dist;

  if (!mute && use_sound && s != SND_NONE)
  {
    if (override || !Mix_Playing(chan))
    {
      Mix_PlayChannel(chan, sounds[s], 0);

      old_sound[chan] = s;
    }

    if (old_sound[chan] == s)
    {
      if (y == SNDDIST_NEAR)
	dist = 0;
      else
      {
	if (y < 0)
	  y = 0;
	else if (y >= screen->h - 1)
	  y = screen->h - 1;

	dist = (255 * ((screen->h - 1) - y)) / (screen->h - 1);
      }

      if (x == SNDPOS_LEFT)
	left = 255 - dist;
      else if (x == SNDPOS_CENTER)
	left = (255 - dist) / 2;
      else if (x == SNDPOS_RIGHT)
	left = 0;
      else
      {
	if (x < 0)
	  x = 0;
	else if (x >= screen->w)
	  x = screen->w - 1;

	left = ((255 - dist) * ((screen->w - 1) - x)) / (screen->w - 1);
      }



      Mix_SetPanning(chan, left, (255 - dist) - left);
    }
  }
#endif
}
