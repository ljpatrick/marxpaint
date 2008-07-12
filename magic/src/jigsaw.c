/*
  jigsaw.c

  jigsaw, Add a jigsaw outline to the whole image.
  Tux Paint - A simple drawing program for children.

  Credits: Andrew Corcoran <akanewbie@gmail.com>

  Copyright (c) 2002-2007 by Bill Kendrick and others; see AUTHORS.txt
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

  Last updated: June 6, 2008
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
#include <time.h>

#ifndef gettext_noop
#define gettext_noop(String) String
#endif

static SDL_Surface* jigsaw;

enum {
  TOOL_JIGSAW,
	jigsaw_NUM_TOOLS
};

static Mix_Chunk * jigsaw_snd_effect[jigsaw_NUM_TOOLS];

const char * jigsaw_snd_filenames[jigsaw_NUM_TOOLS] = {
  "flip.wav",
};
const char * jigsaw_icon_filenames[jigsaw_NUM_TOOLS] = {
  "flip.png",
};
const char * jigsaw_names[jigsaw_NUM_TOOLS] = {
  gettext_noop("Jigsaw"),
};
const char * jigsaw_descs[jigsaw_NUM_TOOLS] = {
  gettext_noop("Click to make your picture into a jigsaw which you can print and cut out, make sure to ask an adult for help."),
};

Uint32 jigsaw_api_version(void) { return(TP_MAGIC_API_VERSION); }

//Load sounds
int jigsaw_init(magic_api * api){

  int i;
  char fname[1024];
  srand(time(0));


  snprintf(fname, sizeof(fname), "%s/images/magic/jigsaw2.png", api->data_directory);
  jigsaw = IMG_Load(fname);


  for (i = 0; i < jigsaw_NUM_TOOLS; i++){
    snprintf(fname, sizeof(fname), "%s/sounds/magic/%s", api->data_directory, jigsaw_snd_filenames[i]);
    jigsaw_snd_effect[i] = Mix_LoadWAV(fname);
  }
  return(1);
}

int jigsaw_get_tool_count(magic_api * api){
  return(jigsaw_NUM_TOOLS);
}

// Load our icons:
SDL_Surface * jigsaw_get_icon(magic_api * api, int which){
  char fname[1024];
  snprintf(fname, sizeof(fname), "%simages/magic/%s", api->data_directory, jigsaw_icon_filenames[which]);
  return(IMG_Load(fname));
}

// Return our names, localized:
char * jigsaw_get_name(magic_api * api, int which){
    return(strdup(gettext(jigsaw_names[which])));
}

// Return our descriptions, localized:
char * jigsaw_get_description(magic_api * api, int which){
  return(strdup(gettext(jigsaw_descs[which])));
}


// Affect the canvas on drag:
void jigsaw_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect){
   // No-op
}

// Affect the canvas on click:
void jigsaw_click(magic_api * api, int which, int mode,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect){
  update_rect->x = 0;
  update_rect->y = 0;
  update_rect->w = canvas->w;
  update_rect->h = canvas->h;
  SDL_BlitSurface(jigsaw, NULL, canvas, NULL);
  api->playsound(jigsaw_snd_effect[which], 128, 255);
}

// Affect the canvas on release:
void jigsaw_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
}

// No setup happened:
void jigsaw_shutdown(magic_api * api)
{
	//Clean up sounds
	int i;
	for(i=0; i<jigsaw_NUM_TOOLS; i++){
		if(jigsaw_snd_effect[i] != NULL){
			Mix_FreeChunk(jigsaw_snd_effect[i]);
		}
	}
  if (jigsaw != NULL){
    SDL_FreeSurface(jigsaw);
  }
}

// Record the color from Tux Paint:
void jigsaw_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int jigsaw_requires_colors(magic_api * api, int which)
{
  return 0;
}

void jigsaw_switchin(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
}

void jigsaw_switchout(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
}

int jigsaw_modes(magic_api * api, int which)
{
  return(MODE_FULLSCREEN);
}

