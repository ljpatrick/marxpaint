//optimized version soon :)
//when "folding" same corner many times it gives strange results. Now it's allowed. Let me know
//if you think it shouldn't be.

#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

#define FOLD_LEN 80

Mix_Chunk * fold_snd;
Uint8 fold_r, fold_g, fold_b;
Uint32 fold_color;
SDL_Surface * fold_surface_src, * fold_surface_dst;

//				Housekeeping functions

void fold_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * snapshot, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect);

Uint32 fold_api_version(void)

{
  return(TP_MAGIC_API_VERSION);
}

void fold_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)	//get the colors from API and store it in structure
{
	fold_r=r;
	fold_g=g;
	fold_b=b;
}

int fold_init(magic_api * api)
{
  char fname[1024];
	
    snprintf(fname, sizeof(fname), "%s/sounds/magic/fold.wav", api->data_directory);
    fold_snd = Mix_LoadWAV(fname);
	
  return(1);
}

int fold_get_tool_count(magic_api * api)
{
  return 1;
}

SDL_Surface * fold_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/fold.png",
	     api->data_directory);

  return(IMG_Load(fname));
}

char * fold_get_name(magic_api * api, int which) { return(strdup("Fold")); }

char * fold_get_description(magic_api * api, int which, int mode) { return strdup(gettext_noop("Choose a background color and click to turn the corner of the page over")); }

int fold_requires_colors(magic_api * api, int which) { return 1; }		//selected color will be a "backpage" color

void fold_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * snapshot,
	           int x, int y, SDL_Rect * update_rect)
{
  
}

void fold_shutdown(magic_api * api)
	{ 
		Mix_FreeChunk(fold_snd);
		SDL_FreeSurface(fold_surface_dst);
		SDL_FreeSurface(fold_surface_src);
	}

// Interactivity functions

inline Uint8 fold_what_corner(int x, int y, SDL_Surface * canvas)
{
	if (x>=canvas->w/2)
	{
		if (y>=canvas->h/2) return 4;
		else return 1;
	}
	else 
	{
		if (y>=canvas->h/2) return 3;
		else return 2;
	}
}

void static fold_rotate(void * ptr, SDL_Surface * canvas, SDL_Surface * snapshot, Uint8 corner)
{
	magic_api * api = (magic_api *) ptr;
	
	unsigned int x,y;
	
	if ((corner==2) || (corner==4))	//rotate +90 degs
	{
		for (x = 0; x<FOLD_LEN; x++)
			for (y =0; y<FOLD_LEN; y++)
				api->putpixel(canvas, x, y, api->getpixel(snapshot, y, FOLD_LEN-x));
	}
	else							//rotate -90 degs
	{
		for (x=0; x<FOLD_LEN; x++)
			for (y=0; y<FOLD_LEN; y++)
				api->putpixel(canvas,x,y,api->getpixel(snapshot,FOLD_LEN-y,x));
	}
}

void static fold_flip(void * ptr, SDL_Surface * canvas, SDL_Surface * snapshot, Uint8 corner)	//flip page
{
	magic_api * api = (magic_api *) ptr;

	unsigned int x, y;

	for (x=0; x<snapshot->w; x++)
		for (y=0; y<snapshot->h; y++)
			api->putpixel(canvas, x, y, api->getpixel(snapshot, snapshot->w-x, y));	//flip horizontal
}

static void fold_print_line(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
                int x, int y)
{
	magic_api * api = (magic_api *) ptr;
	api->putpixel(canvas, x, y, SDL_MapRGB(last->format, 222, 222, 222));		//Middle gray. Color have been set arbitrary. 
}

static void fold_erase(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
                int x, int y)
{
	magic_api * api = (magic_api *) ptr;
	api->putpixel(canvas, x, y, fold_color);
}

static void fold_draw_triangle (magic_api * ptr, int which, SDL_Surface * canvas, SDL_Surface * snapshot, Uint8 corner)
{
	magic_api * api = (magic_api *) ptr;
	unsigned int i;
	//this function could be optimized but it would make this code less readable
	
	switch(corner)		//corners in mathematical order: 1: right upper, 2: left upper, ...
	{
		case 1:
			for (i = 0; i < FOLD_LEN; i++)
				api->line((void *)api, which, canvas, snapshot, canvas->w-FOLD_LEN+i, 0, canvas->w, FOLD_LEN-i, 1, fold_erase);
				//clear the triangle by the corner

			api->line((void *)api, which, canvas, snapshot,canvas->w-FOLD_LEN, 0, canvas->w-FOLD_LEN, FOLD_LEN, 1, fold_print_line);
				//draw vertical line
			api->line((void *)api, which, canvas, snapshot,canvas->w-FOLD_LEN, FOLD_LEN, canvas->w, FOLD_LEN, 1, fold_print_line);
				//draw horizontal line
			api->line((void *)api, which, canvas, snapshot,canvas->w-FOLD_LEN, 0, canvas->w, FOLD_LEN, 1, fold_print_line);
				//and another: oblique
			break;

		case 2:
			for (i = 0; i < FOLD_LEN; i++)
				api->line((void *)api, which, canvas, snapshot, 0, FOLD_LEN-i, FOLD_LEN-i, 0, 1, fold_erase);
				//sequence just like above
			api->line((void *)api, which, canvas, snapshot,FOLD_LEN, 0, FOLD_LEN, FOLD_LEN, 1, fold_print_line);
			api->line((void *)api, which, canvas, snapshot,0, FOLD_LEN, FOLD_LEN, FOLD_LEN, 1, fold_print_line);
			api->line((void *)api, which, canvas, snapshot,0, FOLD_LEN, FOLD_LEN, 0, 1, fold_print_line);
			break;

		case 3:
			for (i = 0; i < FOLD_LEN; i++)
				api->line((void *)api, which, canvas, snapshot, 0, canvas->h-FOLD_LEN+i, FOLD_LEN-i, canvas->h, 1, fold_erase);

			api->line((void *)api, which, canvas, snapshot,FOLD_LEN, canvas->h-FOLD_LEN, FOLD_LEN, canvas->h, 1, fold_print_line);
			api->line((void *)api, which, canvas, snapshot,0, canvas->h-FOLD_LEN, FOLD_LEN, canvas->h-FOLD_LEN, 1, fold_print_line);
			api->line((void *)api, which, canvas, snapshot,0, canvas->h-FOLD_LEN, FOLD_LEN, canvas->h, 1, fold_print_line);
			break;	

		case 4:
			for (i = 0; i < FOLD_LEN; i++)
				api->line((void *)api, which, canvas, snapshot, canvas->w-FOLD_LEN+i, canvas->h, canvas->w,
					canvas->h-FOLD_LEN+i, 1, fold_erase);

			api->line((void *)api, which, canvas, snapshot,canvas->w-FOLD_LEN, canvas->h-FOLD_LEN, canvas->w-FOLD_LEN,
				canvas->h, 1, fold_print_line);
			api->line((void *)api, which, canvas, snapshot,canvas->w-FOLD_LEN, canvas->h-FOLD_LEN, canvas->w,
				canvas->h-FOLD_LEN, 1, fold_print_line);
			api->line((void *)api, which, canvas, snapshot,canvas->w-FOLD_LEN, canvas->h, canvas->w, canvas->h-FOLD_LEN, 1, fold_print_line);
			break;	
	}
}

void fold_click(magic_api * ptr, int which, int mode,
           SDL_Surface * canvas, SDL_Surface * snapshot,
           int x, int y, SDL_Rect * update_rect)
{
	Uint8 corner;

	magic_api * api = (magic_api *) ptr;
	corner=fold_what_corner(x, y, snapshot);
	
	fold_color=SDL_MapRGB(snapshot->format, fold_r, fold_g, fold_b);

	fold_surface_src= SDL_CreateRGBSurface(SDL_ANYFORMAT, FOLD_LEN, FOLD_LEN, snapshot->format->BitsPerPixel,
				snapshot->format->Rmask, snapshot->format->Gmask, snapshot->format->Bmask, snapshot->format->Amask);

	fold_surface_dst=SDL_CreateRGBSurface(SDL_ANYFORMAT, FOLD_LEN, FOLD_LEN, snapshot->format->BitsPerPixel,
				snapshot->format->Rmask, snapshot->format->Gmask, snapshot->format->Bmask, snapshot->format->Amask);
	
	
	switch(corner)
	{
		case 1:					//Right Upper			
			update_rect->x=canvas->w-FOLD_LEN;
			update_rect->y=0;
			break;

		case 2:					//LU
			update_rect->x=update_rect->y=0;
			break;
		
		case 3:					//LL
			update_rect->x=0;
			update_rect->y=canvas->h-FOLD_LEN;
			break;

		case 4:					//RL
			update_rect->x=canvas->w-FOLD_LEN;
			update_rect->y=canvas->h-FOLD_LEN;
			break;
	}
	
	update_rect->w=FOLD_LEN;
	update_rect->h=FOLD_LEN;
	
	SDL_BlitSurface(snapshot, update_rect, fold_surface_src, NULL);
	fold_flip(api, fold_surface_dst, fold_surface_src, corner);
	SDL_BlitSurface(fold_surface_dst, NULL, canvas, update_rect);

	SDL_BlitSurface(canvas, update_rect, fold_surface_src, NULL);
	fold_rotate(api, fold_surface_dst, fold_surface_src, corner);
	SDL_BlitSurface(fold_surface_dst, NULL, canvas, update_rect);
	
	fold_draw_triangle (api, which, canvas, snapshot, corner);
}

void fold_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * snapshot, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{

}

void fold_switchin(magic_api * api, int which, SDL_Surface * canvas)
{
	
}

void fold_switchout(magic_api * api, int which, SDL_Surface * canvas)
{
	
}

int fold_modes(magic_api * api, int which)
{
  return(MODE_PAINT);
}
