/*
  blurAll.c

  blurAll, Blur the whole image
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
	TOOL_BLURALL,
	blurAll_NUM_TOOLS
};

static Mix_Chunk * blurAll_snd_effect[blurAll_NUM_TOOLS];

const char *blurAll_snd_filenames[blurAll_NUM_TOOLS] = {
  "flip.wav",
};
const char * blurAll_icon_filenames[blurAll_NUM_TOOLS] = {
  "flip.png",
};
const char * blurAll_names[blurAll_NUM_TOOLS] = {
  gettext_noop("Blur All"),
};
const char * blurAll_descs[blurAll_NUM_TOOLS] = {
  gettext_noop("Click to blur the whole image."),
};

Uint32 blurAll_api_version(void) { return(TP_MAGIC_API_VERSION); }

//Load sounds
int blurAll_init(magic_api * api){

  int i;
  char fname[1024];

  for (i = 0; i < blurAll_NUM_TOOLS; i++){
    snprintf(fname, sizeof(fname), "%s/sounds/magic/%s", api->data_directory, blurAll_snd_filenames[i]);
    blurAll_snd_effect[i] = Mix_LoadWAV(fname);
  }
  return(1);
}

int blurAll_get_tool_count(magic_api * api){
  return(blurAll_NUM_TOOLS);
}

// Load our icons:
SDL_Surface * blurAll_get_icon(magic_api * api, int which){
  char fname[1024];
  snprintf(fname, sizeof(fname), "%simages/magic/%s", api->data_directory, blurAll_icon_filenames[which]);
  return(IMG_Load(fname));
}

// Return our names, localized:
char * blurAll_get_name(magic_api * api, int which){
    return(strdup(gettext(blurAll_names[which])));
}

// Return our descriptions, localized:
char * blurAll_get_description(magic_api * api, int which, int mode){
  return(strdup(gettext(blurAll_descs[which])));
}

// Do the effect:
static void do_blurAll(void * ptr,SDL_Surface * canvas, SDL_Surface * last, int which){

	magic_api * api = (magic_api *) ptr;

	int x,y;
  int i,j;
	Uint8 temp[3];
  double blurValue[3];

  //5x5 gaussiann weighting window
  const int weight[5][5] = {  {1,4,7,4,1},
                              {4,16,26,16,4},
                              {7,26,41,26,7},
                              {4,16,26,16,4},
                              {1,4,7,4,1}};

	for (y = 0; y < last->h; y++){
		for (x=0; x < last->w; x++){
      int k;
      //zero the holder for the new value
      for (k =0;k<3;k++){
			  blurValue[k] =0;
      }
      for (i=-2;i<3;i++){
        for (j=-2;j<3;j++){
          //Add the pixels around the current one wieghted 
			    SDL_GetRGB(api->getpixel(last, x + i, y + j), last->format, &temp[0], &temp[1], &temp[2]);
          for (k =0;k<3;k++){
			      blurValue[k] += temp[k]* weight[i+2][j+2];
          }
        }
      }
      for (k =0;k<3;k++){
			  blurValue[k] /=273;
      }
		  api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format, blurValue[0], blurValue[1], blurValue[2]));
		}
	}
}

// Affect the canvas on drag:
void blurAll_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect){
   // No-op
}

// Affect the canvas on click:
void blurAll_click(magic_api * api, int which, int mode,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect){
  update_rect->x = 0;
  update_rect->y = 0;
  update_rect->w = canvas->w;
  update_rect->h = canvas->h;
  do_blurAll(api, canvas,  last, which);
  api->playsound(blurAll_snd_effect[which], 128, 255);
}

// Affect the canvas on release:
void blurAll_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
}

// No setup happened:
void blurAll_shutdown(magic_api * api)
{
	//Clean up sounds
	int i;
	for(i=0; i<blurAll_NUM_TOOLS; i++){
		if(blurAll_snd_effect[i] != NULL){
			Mix_FreeChunk(blurAll_snd_effect[i]);
		}
	}
}

// Record the color from Tux Paint:
void blurAll_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int blurAll_requires_colors(magic_api * api, int which)
{
  return 0;
}

void blurAll_switchin(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
}

void blurAll_switchout(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
}

int blurAll_modes(magic_api * api, int which)
{
  return(MODE_FULLSCREEN); /* FIXME - Can also be turned into a painted effect */ /* FIXME: Merge with blur */
}
