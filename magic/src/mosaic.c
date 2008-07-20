/*
  mosaic.c

  mosaic, Add a mosaic effect to the image using a combination of other tools.
  Requires the mosaicAll sharpen and noise tools.
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
#include <time.h>
//#include "noise.c"
//#include "sharpen.c"
//#include "blur.c"


#ifndef gettext_noop
#define gettext_noop(String) String
#endif


const int mosaic_AMOUNT= 300;
const int mosaic_RADIUS = 16;
const double mosaic_SHARPEN = 1.0;

//Holder for the pre calulated pixel values
SDL_Surface * mosaic_temp;
SDL_Surface * mosaic_final;

enum {
	TOOL_MOSAIC,
	mosaic_NUM_TOOLS
};

static Mix_Chunk * mosaic_snd_effect[mosaic_NUM_TOOLS];

const char * mosaic_snd_filenames[mosaic_NUM_TOOLS] = {
  "flip.wav",
};
const char * mosaic_icon_filenames[mosaic_NUM_TOOLS] = {
  "flip.png",
};
const char * mosaic_names[mosaic_NUM_TOOLS] = {
  gettext_noop("Mosaic"),
};
const char * mosaic_descs[mosaic_NUM_TOOLS] = {
  gettext_noop("Click to add a mosaic effect to the image."),
};

Uint32 mosaic_api_version(void) { return(TP_MAGIC_API_VERSION); }

//Load sounds
int mosaic_init(magic_api * api){

  int i;
  char fname[1024];

  for (i = 0; i < mosaic_NUM_TOOLS; i++){
    snprintf(fname, sizeof(fname), "%s/sounds/magic/%s", api->data_directory, mosaic_snd_filenames[i]);
    mosaic_snd_effect[i] = Mix_LoadWAV(fname);
  }

  return(1);
}

int mosaic_get_tool_count(magic_api * api){
  return(mosaic_NUM_TOOLS);
}

// Load our icons:
SDL_Surface * mosaic_get_icon(magic_api * api, int which){
  char fname[1024];
  snprintf(fname, sizeof(fname), "%simages/magic/%s", api->data_directory, mosaic_icon_filenames[which]);
  return(IMG_Load(fname));
}

// Return our names, localized:
char * mosaic_get_name(magic_api * api, int which){
    return(strdup(gettext(mosaic_names[which])));
}

// Return our descriptions, localized:
char * mosaic_get_description(magic_api * api, int which){
  return(strdup(gettext(mosaic_descs[which])));
}

//Calculates the grey scale value for a rgb pixel
static int mosaic_grey(Uint8 r1,Uint8 g1,Uint8 b1){
	return 0.3*r1+.59*g1+0.11*b1;
}

//Do the effect for one pixel
static void mosaic_do_noise_pixel(magic_api * api, SDL_Surface * canvas, SDL_Surface * last, int x, int y){


}

//Do the effect for one pixel
static void do_mosaic_pixel(void * ptr, int which,
	         SDL_Surface * canvas, SDL_Surface * last,
	         int x, int y){
  magic_api * api = (magic_api *) ptr;

  Uint8 r1,g1,b1;
  SDL_GetRGB(api->getpixel(mosaic_final, x, y), mosaic_final->format, &r1, &g1, &b1);
  api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format, r1, g1, b1));
}

// Do the effect for the full image
static void do_mosaic_full(void * ptr, SDL_Surface * canvas, SDL_Surface * last, int which){

	magic_api * api = (magic_api *) ptr;

	int x,y;

	for (y = 0; y < canvas->h; y++){ 
		for (x=0; x < canvas->w; x++){
      do_mosaic_pixel(ptr, which, canvas, last, x, y);
	  }
  }
}

//do the effect for the brush
static void do_mosaic_brush(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last, int x, int y){
  int xx, yy;
  magic_api * api = (magic_api *) ptr;

  for (yy = y - mosaic_RADIUS; yy < y + mosaic_RADIUS; yy++)
  {
    for (xx = x - mosaic_RADIUS; xx < x + mosaic_RADIUS; xx++)
    {
      if (api->in_circle(xx - x, yy - y, mosaic_RADIUS) &&
	  !api->touched(xx, yy))
      {
        do_mosaic_pixel(api, which, canvas, last, xx, yy);
      }
    }
  }
}

// Affect the canvas on drag:
void mosaic_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect){

  api->line((void *) api, which, canvas, last, ox, oy, x, y, 1, do_mosaic_brush);

  api->playsound(mosaic_snd_effect[which], (x * 255) / canvas->w, 255);

  if (ox > x) { int tmp = ox; ox = x; x = tmp; }
  if (oy > y) { int tmp = oy; oy = y; y = tmp; }

  update_rect->x = ox - mosaic_RADIUS;
  update_rect->y = oy - mosaic_RADIUS;
  update_rect->w = (x + mosaic_RADIUS) - update_rect->x;
  update_rect->h = (y + mosaic_RADIUS) - update_rect->y;
}

// Affect the canvas on click:
void mosaic_click(magic_api * api, int which, int mode,
	            SDL_Surface * canvas, SDL_Surface * last,
	            int x, int y, SDL_Rect * update_rect){
  if (mode == MODE_PAINT){
    mosaic_drag(api, which, canvas, last, x, y, x, y, update_rect);
  }
  else{
    update_rect->x = 0;
    update_rect->y = 0;
    update_rect->w = canvas->w;
    update_rect->h = canvas->h;
    do_mosaic_full(api, canvas,  last, which);
    api->playsound(mosaic_snd_effect[which], 128, 255);
  }
}

// Affect the canvas on release:
void mosaic_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
}

// No setup happened:
void mosaic_shutdown(magic_api * api)
{
	//Clean up sounds
	int i;
	for(i=0; i<mosaic_NUM_TOOLS; i++){
		if(mosaic_snd_effect[i] != NULL){
			Mix_FreeChunk(mosaic_snd_effect[i]);
		}
	}
}

// Record the color from Tux Paint:
void mosaic_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

// Use colors:
int mosaic_requires_colors(magic_api * api, int which)
{
  return 0;
}

//Add noise to a pixel
static void mosaic_noise_pixel(void * ptr, SDL_Surface * canvas, int noise_AMOUNT, int x, int y){
	magic_api * api = (magic_api *) ptr;

  Uint8 temp[3];
  double temp2[3];

	SDL_GetRGB(api->getpixel(canvas,x, y), canvas->format, &temp[0], &temp[1], &temp[2]);
  int k;
  for (k =0;k<3;k++){
		temp2[k] = clamp(0.0, (int)temp[k] - (rand()%noise_AMOUNT) + noise_AMOUNT/2.0, 255.0);
  }
  api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format, temp2[0], temp2[1], temp2[2]));
}

//Blur a pixel
static void mosaic_blur_pixel(void * ptr, SDL_Surface * canvas, SDL_Surface * last, int x, int y){
  magic_api * api = (magic_api *) ptr;
  int i,j,k;
	Uint8 temp[3];
  double blurValue[3];

  for (k =0;k<3;k++){
		blurValue[k] = 0;
  }

  //5x5 gaussiann weighting window
  const int weight[5][5] = {  {1,4,7,4,1},
                              {4,16,26,16,4},
                              {7,26,41,26,7},
                              {4,16,26,16,4},
                              {1,4,7,4,1}};
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
    blurValue[k] /= 273;
  }
	api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format, blurValue[0], blurValue[1], blurValue[2]));
}

//Sharpen a pixel
static void mosaic_sharpen_pixel(void * ptr,
	         SDL_Surface * canvas, SDL_Surface * last,
	         int x, int y){

	magic_api * api = (magic_api *) ptr;

	Uint8 r1, g1, b1;
 	int grey;
	int i,j;
	double sobel_1=0,sobel_2=0;

  	//Sobel weighting masks
	const int sobel_weights_1[3][3] = {	{1,2,1},
                                      {0,0,0},
                                      {-1,-2,-1}};
	const int sobel_weights_2[3][3] = {	{-1,0,1},
                                      {-2,0,2},
                                      {-1,0,1}};

			sobel_1=0;
			sobel_2=0;
			for (i=-1;i<2;i++){
				for(j=-1; j<2; j++){
					//No need to check if inside canvas, getpixel does it for us.
					SDL_GetRGB(api->getpixel(last, x+i, y+j), last->format, &r1, &g1, &b1);
					grey = mosaic_grey(r1,g1,b1);
					sobel_1 += grey * sobel_weights_1[i+1][j+1];
					sobel_2 += grey * sobel_weights_2[i+1][j+1];
				}
			}
  
  double temp = sqrt(sobel_1*sobel_1 + sobel_2*sobel_2);
  temp = (temp/1443)*255.0;

  SDL_GetRGB(api->getpixel(last, x, y), last->format, &r1, &g1, &b1);
	api->putpixel(canvas, x, y, SDL_MapRGB(canvas->format, clamp(0.0, r1 + mosaic_SHARPEN * temp, 255.0), 
													clamp(0.0, g1 + mosaic_SHARPEN * temp, 255.0), 
													clamp(0.0, b1 + mosaic_SHARPEN * temp, 255.0)));
	
}

void mosaic_switchin(magic_api * api, int which, int mode, SDL_Surface * canvas)
{

  //Create two temp surfaces to store the calculated pixel values
  Uint32 amask = ~(canvas->format->Rmask |
            canvas->format->Gmask |
            canvas->format->Bmask);
  mosaic_temp =
    SDL_CreateRGBSurface(SDL_SWSURFACE,
                         canvas->w,
                         canvas->h,
                         canvas->format->BitsPerPixel,
                         canvas->format->Rmask,
                         canvas->format->Gmask,
                         canvas->format->Bmask, amask);
  mosaic_final =
    SDL_CreateRGBSurface(SDL_SWSURFACE,
                         canvas->w,
                         canvas->h,
                         canvas->format->BitsPerPixel,
                         canvas->format->Rmask,
                         canvas->format->Gmask,
                         canvas->format->Bmask, amask);

  //Copy the canvas to the temp surface
  SDL_BlitSurface(canvas, NULL, mosaic_final, NULL);

  int x,y;

  //Add noise to the temp surface
  for (y = 0; y < canvas->h; y++){ 
		for (x=0; x < canvas->w; x++){
      mosaic_noise_pixel(api, mosaic_final, mosaic_AMOUNT, x, y);
    }
  }

  api->update_progress_bar();

  //Blur the temp surface
  for (y = 0; y < canvas->h; y++){ 
		for (x=0; x < canvas->w; x++){
      //We dont want pixels to be affected by already blurred nearby pixels so we require two surfaces
      mosaic_blur_pixel(api, mosaic_temp, mosaic_final, x, y);
    }
  }
  
  api->update_progress_bar();
  
  //Sharpen the temp surface
  for (y = 0; y < canvas->h; y++){ 
		for (x=0; x < canvas->w; x++){
      //We dont want pixels to be affected by already blurred nearby pixels so we require two surfaces
      mosaic_sharpen_pixel(api, mosaic_final, mosaic_temp, x, y);
    }
  }

}

void mosaic_switchout(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
  SDL_FreeSurface(mosaic_temp);
  SDL_FreeSurface(mosaic_final);
}

int mosaic_modes(magic_api * api, int which)
{
  return(MODE_FULLSCREEN|MODE_PAINT);
}


