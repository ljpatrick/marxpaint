/*
  sharpen.c

  Sharpen, Trace Contour and Silhouette Magic Tool Plugin
  Tux Paint - A simple drawing program for children.

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

#ifndef gettext_noop
#define gettext_noop(String) String
#endif

/* Our globals: */

enum {
	TOOL_TRACE,
	TOOL_SHARPEN,
	TOOL_SILHOUETTE,
	sharpen_NUM_TOOLS
};

//Holder for the unnormalised edge values
double* sharpen_temp;

const int THRESHOLD = 50;

const double SHARPEN = 0.5;

static Mix_Chunk * sharpen_snd_effect[sharpen_NUM_TOOLS];

const char * sharpen_snd_filenames[sharpen_NUM_TOOLS] = {
  "flip.wav",
  "flip.wav",
  "flip.wav"
};
const char * sharpen_icon_filenames[sharpen_NUM_TOOLS] = {
  "flip.png",
  "flip.png",
  "flip.png"
};
const char * sharpen_names[sharpen_NUM_TOOLS] = {
  gettext_noop("Trace Contour"),
  gettext_noop("Sharpen"),
  gettext_noop("Silhouette")
};
const char * sharpen_descs[sharpen_NUM_TOOLS] = {
  gettext_noop("Click to trace the edges of objects in the image."),
  gettext_noop("Click to sharpen the image."),
  gettext_noop("Click to create a black and white silhouette of the image.")
};

Uint32 sharpen_api_version(void) { return(TP_MAGIC_API_VERSION); }


// No setup required:
int sharpen_init(magic_api * api){
  
  int i;
  char fname[1024];

  for (i = 0; i < sharpen_NUM_TOOLS; i++){
    snprintf(fname, sizeof(fname), "%s/sounds/magic/%s", api->data_directory, sharpen_snd_filenames[i]);
    sharpen_snd_effect[i] = Mix_LoadWAV(fname);
  }
  return(1);
}

// We have multiple tools:
int sharpen_get_tool_count(magic_api * api)
{
  return(sharpen_NUM_TOOLS);
}

// Load our icons:
SDL_Surface * sharpen_get_icon(magic_api * api, int which){
  char fname[1024];
  snprintf(fname, sizeof(fname), "%simages/magic/%s", api->data_directory, sharpen_icon_filenames[which]);
  return(IMG_Load(fname));
}

// Return our names, localized:
char * sharpen_get_name(magic_api * api, int which){
    return(strdup(gettext(sharpen_names[which])));
}

// Return our descriptions, localized:
char * sharpen_get_description(magic_api * api, int which){
  return(strdup(gettext(sharpen_descs[which])));
}

//Calculates the grey scale value for a rgb pixel
static int sharpen_grey(Uint8 r1,Uint8 g1,Uint8 b1){
	return 0.3*r1+.59*g1+0.11*b1;
}

// Do the effect:
static void do_sharpen(void * ptr,SDL_Surface * canvas, SDL_Surface * last, int which)
{
	magic_api * api = (magic_api *) ptr;
  sharpen_temp = (double*)malloc(api->canvas_w*api->canvas_h*sizeof(double));
	int x, y;
	int grey;
	Uint8 r1, g1, b1;

	//For sobel calculation
	int i,j;
	int sobel_1,sobel_2;

	//For normalisation
	double min=INT_MAX;
	double max=0;


	//Sobel weighting masks
	const int sobel_weights_1[3][3] = {	{1,2,1},
                                      {0,0,0},
                                      {-1,-2,-1}};
	const int sobel_weights_2[3][3] = {	{-1,0,1},
                                      {-2,0,2},
                                      {-1,0,1}};

	for (y = 0; y < canvas->h; y++){
		for (x=0; x < canvas->w; x++){
			//Calculate Sobel edge values

			sobel_1=0;
			sobel_2=0;
			for (i=-1;i<2;i++){
				for(j=-1; j<2; j++){
					//No need to check if inside canvas, getpixel does it for us.
					SDL_GetRGB(api->getpixel(last, x+i, y+j), last->format, &r1, &g1, &b1);
					grey = sharpen_grey(r1,g1,b1);
					sobel_1 += grey * sobel_weights_1[i+1][j+1];
					sobel_2 += grey * sobel_weights_2[i+1][j+1];
				}
			}

			//And store in temp variable
			//Cant just write to surface as they may not be 0-255 and surface will clamp them and lose data
			sharpen_temp[x*(canvas->h-1) + y] = sqrt(sobel_1*sobel_1 + sobel_2*sobel_2);

			//Calculate normalisation
			if (sharpen_temp[x*(canvas->h-1) + y]<min){
				min=sharpen_temp[x*(canvas->h-1) + y];
			}
			if(sharpen_temp[x*(canvas->h-1) + y]>max){
				max=sharpen_temp[x*(canvas->h-1) + y];
			}
		}
	}

	for (y = 0; y < canvas->h; y++){
		for (x=0; x < canvas->w; x++){

			//apply normalisation
			sharpen_temp[x*(canvas->h-1) + y]= ((sharpen_temp[x*(canvas->h-1) + y]-min)/(max-min))*255.0;

			// set image to white where edge value is below THRESHOLD
			if (which == TOOL_TRACE){
				if (sharpen_temp[x*(canvas->h-1) + y]<THRESHOLD)
				{
					api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format, 255, 255, 255));
				}
		
			}
			//Simply display the edge values - provides a nice black and white silhouette image
			else if (which == TOOL_SILHOUETTE){
				api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format, 	sharpen_temp[x*(canvas->h-1) + y], 
																sharpen_temp[x*(canvas->h-1) + y], 
																sharpen_temp[x*(canvas->h-1) + y]));
			}
			//Add the edge values to the original image, creating a more distinct jump in contrast at edges
			else if(which == TOOL_SHARPEN){
				SDL_GetRGB(api->getpixel(last, x, y), last->format, &r1, &g1, &b1);
				api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format, clamp(0.0, r1 + SHARPEN * sharpen_temp[x*(canvas->h-1) + y], 255.0), 
															clamp(0.0, g1 + SHARPEN * sharpen_temp[x*(canvas->h-1) + y], 255.0), 
															clamp(0.0, b1 + SHARPEN * sharpen_temp[x*(canvas->h-1) + y], 255.0)));
			}
		}
	}
  //Clean up temp variable
	if (sharpen_temp != NULL){
		free(sharpen_temp);
	}
}

// Affect the canvas on drag:
void sharpen_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{
   // No-op
}

// Affect the canvas on click:
void sharpen_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
  update_rect->x = 0;
  update_rect->y = 0;
  update_rect->w = canvas->w;
  update_rect->h = canvas->h;
  do_sharpen(api, canvas,  last, which);
  //play sound
	api->playsound(sharpen_snd_effect[which], 128, 255);
}

// Affect the canvas on release:
void sharpen_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
}

// No setup happened:
void sharpen_shutdown(magic_api * api)
{
	//Clean up sounds
	int i;
	for(i=0; i<sharpen_NUM_TOOLS; i++){
		if(sharpen_snd_effect[i] != NULL){
			Mix_FreeChunk(sharpen_snd_effect[i]);
		}
	}
}

// Record the color from Tux Paint:
void sharpen_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int sharpen_requires_colors(magic_api * api, int which)
{
  return 0;
}

