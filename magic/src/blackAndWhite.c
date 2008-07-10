/*
  blackAndWhite.c

  blackAndWhite, Convert the image to greyscale or threshold it into pure black and pure white
  Tux Paint - A simple drawing program for children.

  FIXME: Credits

  Copyright (c) 2002-2008 by Bill Kendrick and others; see AUTHORS.txt
  bill@newbreedsoftware.com
  http://www.tuxpaint.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  (See COPYING.txt)

  Last updated: July 8, 2008
  $Id$
*/

#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include <math.h>
#include <limits.h>

#ifndef gettext_noop
#define gettext_noop(String) String
#endif

enum {
	TOOL_BANDW,
	TOOL_THRESHOLD,
	bandw_NUM_TOOLS
};

static Mix_Chunk * bandw_snd_effect[bandw_NUM_TOOLS];

const char * bandw_snd_filenames[bandw_NUM_TOOLS] = {
  "flip.wav",
  "flip.wav"
};
const char * bandw_icon_filenames[bandw_NUM_TOOLS] = {
  "flip.png",
  "flip.png"
};
const char * bandw_names[bandw_NUM_TOOLS] = {
  gettext_noop("Black & White"),
  gettext_noop("Threshold")
};
const char * bandw_descs[bandw_NUM_TOOLS] = {
  gettext_noop("Click to convert the image to greyscale."),
  gettext_noop("Click to threshold the image into black and white regions.")
};

Uint32 blackAndWhite_api_version(void) { return(TP_MAGIC_API_VERSION); }

//Load sounds
int blackAndWhite_init(magic_api * api){
  int i;
  char fname[1024];

  for (i = 0; i < bandw_NUM_TOOLS; i++){
    snprintf(fname, sizeof(fname), "%s/sounds/magic/%s", api->data_directory, bandw_snd_filenames[i]);
    bandw_snd_effect[i] = Mix_LoadWAV(fname);
  }
  return(1);
}

int blackAndWhite_get_tool_count(magic_api * api){
  return(bandw_NUM_TOOLS);
}

// Load our icons:
SDL_Surface * blackAndWhite_get_icon(magic_api * api, int which){
  char fname[1024];
  snprintf(fname, sizeof(fname), "%simages/magic/%s", api->data_directory, bandw_icon_filenames[which]);
  return(IMG_Load(fname));
}

// Return our names, localized:
char * blackAndWhite_get_name(magic_api * api, int which){
    return(strdup(gettext(bandw_names[which])));
}

// Return our descriptions, localized:
char * blackAndWhite_get_description(magic_api * api, int which, int mode){
  return(strdup(gettext(bandw_descs[which])));
}

//Calculates the grey scale value for a rgb pixel
static int blackAndWhite_grey(Uint8 r1,Uint8 g1,Uint8 b1){
	return 0.3*r1+.59*g1+0.11*b1;
}

// Do the effect:
static void do_blackAndWhite(void * ptr,SDL_Surface * canvas, SDL_Surface * last, int which){

	magic_api * api = (magic_api *) ptr;

	int x,y;
	Uint8 r1,g1,b1;
	int min = INT_MAX;
	int max = 0;
	int thresholdValue;

	for (y = 0; y < last->h; y++)
	{
		for (x=0; x < last->w; x++)
		{
			SDL_GetRGB(api->getpixel(last, x, y), last->format, &r1, &g1, &b1);
			int greyValue = blackAndWhite_grey(r1,g1,b1);
			if (which == TOOL_BANDW)
			{
				api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format, greyValue, greyValue, greyValue));
			}else if(which == TOOL_THRESHOLD)
			{
				if (greyValue<min)
				{
					min=greyValue;
				}
				if (greyValue>max)
				{
					max=greyValue;
				}
			}			
		}
	}
	thresholdValue = (max-min)/2;
	if (which == TOOL_THRESHOLD){
		for (y = 0; y < last->h; y++){
			for (x=0; x < last->w; x++){
				SDL_GetRGB(api->getpixel(last, x, y), last->format, &r1, &g1, &b1);
				int greyValue = blackAndWhite_grey(r1,g1,b1);
				if (greyValue < thresholdValue){
					api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format, 0, 0, 0));
				}else{
					api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format, 255, 255, 255));
				}
			}
		}
	}
}

// Affect the canvas on drag:
void blackAndWhite_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect){
   // No-op
}

// Affect the canvas on click:
void blackAndWhite_click(magic_api * api, int which, int mode,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect){
  update_rect->x = 0;
  update_rect->y = 0;
  update_rect->w = canvas->w;
  update_rect->h = canvas->h;
  do_blackAndWhite(api, canvas,  last, which);
  api->playsound(bandw_snd_effect[which], 128, 255);
}

// Affect the canvas on release:
void blackAndWhite_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
}

// No setup happened:
void blackAndWhite_shutdown(magic_api * api)
{
	//Clean up sounds
	int i;
	for(i=0; i<bandw_NUM_TOOLS; i++){
		if(bandw_snd_effect[i] != NULL){
			Mix_FreeChunk(bandw_snd_effect[i]);
		}
	}
}

// Record the color from Tux Paint:
void blackAndWhite_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int blackAndWhite_requires_colors(magic_api * api, int which)
{
  return 0;
}

void blackAndWhite_switchin(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
}

void blackAndWhite_switchout(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
}

int blackAndWhite_modes(magic_api * api, int which)
{
  return(MODE_FULLSCREEN); /* FIXME - Can also be turned into a painted effect */
}
