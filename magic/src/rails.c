#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include <stdbool.h>

#define SEG_NONE 0

#define SEG_LEFT 1
#define SEG_RIGHT 2
#define SEG_TOP 4
#define SEG_BOTTOM 8

#define SEG_LEFT_RIGHT (SEG_LEFT | SEG_RIGHT)
#define SEG_TOP_BOTTOM (SEG_TOP | SEG_BOTTOM)
#define SEG_RIGHT_TOP (SEG_RIGHT | SEG_TOP)
#define SEG_RIGHT_BOTTOM (SEG_RIGHT | SEG_BOTTOM)
#define SEG_LEFT_TOP (SEG_LEFT | SEG_TOP)
#define SEG_LEFT_BOTTOM (SEG_LEFT | SEG_BOTTOM)
#define SEG_LEFT_RIGHT_TOP (SEG_LEFT | SEG_RIGHT | SEG_TOP)
#define SEG_LEFT_RIGHT_BOTTOM (SEG_LEFT | SEG_RIGHT | SEG_BOTTOM)
#define SEG_LEFT_TOP_BOTTOM (SEG_LEFT | SEG_TOP | SEG_BOTTOM)
#define SEG_RIGHT_TOP_BOTTOM (SEG_RIGHT | SEG_TOP | SEG_BOTTOM)
#define SEG_LEFT_RIGHT_TOP_BOTTOM (SEG_LEFT | SEG_RIGHT | SEG_TOP | SEG_BOTTOM)

Mix_Chunk * rails_snd;
unsigned int img_w, img_h;
unsigned int rails_segments_x, rails_segments_y;	//how many segments do we have?
static int rails_math_ceil(int x, int y);	//ceil() in cstdlib returns float and is relative slow, so we'll use our one
static Uint8 * rails_status_of_segments;	//a place to store an info about bitmap used for selected segment
static char ** rails_images;	//the pathes to all the images needed
static unsigned int rails_segment_modified;		//which segment was modified this time?
static SDL_Rect modification_rect;
static SDL_Surface * canvas_backup;
//				Housekeeping functions

void rails_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * snapshot, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect);

SDL_Surface * rails_one, * rails_three, * rails_four, * rails_corner;

Uint32 rails_api_version(void)
{
  return(TP_MAGIC_API_VERSION);
}

int rails_modes(magic_api * api, int which)
{
  return(MODE_PAINT);
}

void rails_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
}

int rails_init(magic_api * api)
{
	char fname[1024];
	Uint8 i;		//is always < 3, so Uint8 seems to be a good idea
	
	rails_images=(char **)malloc(sizeof(char *)*4);
	
	for (i = 0; i < 4; i++)
		rails_images[i]=(char *)malloc(sizeof(char)*1024);
	
	snprintf(rails_images[0], 1024*sizeof(char), "%s/images/magic/rails_one.png", api->data_directory);
	snprintf(rails_images[1], 1024*sizeof(char), "%s/images/magic/rails_three.png", api->data_directory);
	snprintf(rails_images[2], 1024*sizeof(char), "%s/images/magic/rails_four.png", api->data_directory);
	snprintf(rails_images[3], 1024*sizeof(char), "%s/images/magic/rails_three.png", api->data_directory);

	rails_one=IMG_Load(rails_images[0]);
    rails_three=IMG_Load(rails_images[1]);
    rails_four=IMG_Load(rails_images[2]);
    rails_corner=IMG_Load(rails_images[3]);

        img_w = rails_one->w;
        img_h = rails_one->h;
    
	snprintf(fname, sizeof(fname), "%s/sounds/magic/rails.wav", api->data_directory);
    rails_snd = Mix_LoadWAV(fname);

  return(1);
}

int rails_get_tool_count(magic_api * api)
{
  return 1;
}

SDL_Surface * rails_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/rails.png",
	     api->data_directory);

  return(IMG_Load(fname));
}

char * rails_get_name(magic_api * api, int which) { return strdup(gettext_noop("Rails")); }

char * rails_get_description(magic_api * api, int which, int mode) { return strdup(gettext_noop("Click and drag to draw train track rails on your picture.")); }

int rails_requires_colors(magic_api * api, int which) { return 0;}

void rails_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * snapshot,
	           int x, int y, SDL_Rect * update_rect)
{
}

void rails_shutdown(magic_api * api)
{
#if 0

// FIXME -- commented out because a leak is better than a crash
//
// *** glibc detected *** /tmp/dd/usr/bin/tuxpaint: munmap_chunk(): invalid pointer
// : 0x108bd098 ***
// ======= Backtrace: =========
// /lib/libc.so.6[0xf9373a8]
// /usr/lib/tuxpaint/plugins/rails.so(rails_shutdown+0xac)[0xe7cf67c]
// /tmp/dd/usr/bin/tuxpaint[0x1000882c]
// /tmp/dd/usr/bin/tuxpaint[0x10021900]
// /lib/libc.so.6[0xf8d6b10]
// /lib/libc.so.6[0xf8d6cd0]

	Uint8 i;
	
	if (rails_snd!=NULL)
		Mix_FreeChunk(rails_snd);
	SDL_FreeSurface(rails_one);
	SDL_FreeSurface(rails_three);
	SDL_FreeSurface(rails_four);
	SDL_FreeSurface(rails_corner);
	SDL_FreeSurface(canvas_backup);
	
	for (i = 0; i < 4; i++)
		free(rails_images[i]);
	free(rails_images);
	free(rails_status_of_segments);
#endif
}

void rails_switchin(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
	//we've to compute the quantity of segments in each direction

	canvas_backup=SDL_CreateRGBSurface(SDL_ANYFORMAT, canvas->w, canvas->h, canvas->format->BitsPerPixel,
				canvas->format->Rmask, canvas->format->Gmask, canvas->format->Bmask, canvas->format->Amask);

	SDL_BlitSurface(canvas, NULL, canvas_backup, NULL);
	rails_segments_x=rails_math_ceil(canvas->w,img_w);
	rails_segments_y=rails_math_ceil(canvas->h,img_h);
	rails_status_of_segments=(Uint8 *)calloc(rails_segments_x*rails_segments_y,sizeof(Uint8));
}

void rails_switchout(magic_api * api, int which, int mode, SDL_Surface * canvas)
{
	free(rails_status_of_segments);
}

// Interactivity functions

static int rails_math_ceil(int x, int y)
{	
 int temp;
 temp=(int)x/y;

 if (x%y)
	return temp+1;
 else return temp;
}
	
inline unsigned int rails_get_segment(int x, int y)		
{
	int xx; 													//segments are numerated just like pixels
	int yy;														//in computer graphics: left upper (=1), ... ,right upper,
																//left bottom, ... , right bottom
	xx=rails_math_ceil(x, img_w);
	yy=rails_math_ceil(y, img_h);
	
	return (yy-1)*rails_segments_x+xx;
	
}

inline void rails_extract_coords_from_segment(unsigned int segment, Sint16 * x, Sint16 * y)
{																//extracts the coords of the beginning and the segment
	*x=((segment%rails_segments_x)-1)*img_w;					//useful to set update_rect as small as possible
	*y=(int)(segment/rails_segments_x)*img_h;
}

static void rails_flip(void * ptr, SDL_Surface * dest, SDL_Surface * src)
{
	magic_api * api = (magic_api *) ptr;

	Sint16 x, y;

	for (x=0; x<dest->w; x++)
		for (y=0; y<dest->h; y++)
			api->putpixel(dest, x, y, api->getpixel(src, x, src->h-y));
}

static void rails_rotate (void * ptr, SDL_Surface * dest, SDL_Surface * src, _Bool direction)
//src and dest must have same size
{
	magic_api * api = (magic_api *) ptr;
	Sint16 x,y;
	
	if (direction)	//rotate -90 degs
	{
		for (x = 0; x<dest->w; x++)
			for (y =0; y<dest->h; y++)
				api->putpixel(dest, x, y, api->getpixel(src, y, src->w-x));
	}
	else			//rotate +90 degs
	{
		for (x=0; x<dest->w; x++)
			for (y=0; y<dest->h; y++)
				api->putpixel(dest,x,y,api->getpixel(src,src->h-y,x));
	}
	
}

void rails_click(magic_api * api, int which, int mode,
           SDL_Surface * canvas, SDL_Surface * snapshot,
           int x, int y, SDL_Rect * update_rect)
{
	rails_drag(api, which, canvas, snapshot, x, y, x, y, update_rect);
}

static Uint8 rails_select_image(Uint8 segment)
{
	int take_up, take_down, take_left, take_right;
	int val_up, val_down, val_left, val_right;

	take_up=segment-rails_segments_x;	
	if (take_up<=0) val_up = SEG_NONE;
        else val_up = rails_status_of_segments[take_up];

	take_down=segment+rails_segments_x;
	if (take_down>rails_segments_x*rails_segments_y) val_down = SEG_NONE;
	else val_down = rails_status_of_segments[take_down];
	
	if ((segment%rails_segments_x)==1) val_left=SEG_NONE;
	else val_left = rails_status_of_segments[segment-1];
	
	if ((segment%rails_segments_x)==0) val_right=SEG_NONE;
	else val_right = rails_status_of_segments[segment+1];

	if (	(val_left & SEG_RIGHT) && (val_right & SEG_LEFT) &&
		(val_up & SEG_BOTTOM) && (val_down & SEG_TOP))
			return SEG_LEFT_RIGHT_TOP_BOTTOM;
		
	if (	(val_left & SEG_RIGHT) && (val_right & SEG_LEFT) &&
                 (val_up & SEG_BOTTOM))
			return SEG_LEFT_RIGHT_TOP;

	if (	(val_left & SEG_RIGHT) && (val_right & SEG_LEFT) &&
                 (val_down & SEG_TOP))
			return SEG_LEFT_RIGHT_BOTTOM;
		
	if (	(val_right & SEG_LEFT) &&
                 (val_down & SEG_TOP) && (val_up & SEG_BOTTOM) )
			return SEG_LEFT_TOP_BOTTOM;
		
	if (	(val_right & SEG_LEFT) &&
                 (val_down & SEG_TOP) && (val_up & SEG_BOTTOM) )
			return SEG_RIGHT_TOP_BOTTOM;

	if (	(val_right & SEG_LEFT) && (val_left & SEG_RIGHT))
			return SEG_LEFT_RIGHT;

        return SEG_TOP_BOTTOM;
}

static void rails_draw(void * ptr, int which, SDL_Surface * canvas, SDL_Surface * last,
                int x, int y)
{
	magic_api * api = (magic_api *) ptr;
	SDL_Surface * result, * temp;
	Uint8 image;
	_Bool use_temp;
	
	use_temp=0;
	rails_segment_modified=rails_get_segment(x,y);
	
	//modification_rect.x and modification_rect.y are set by function
	rails_extract_coords_from_segment(rails_segment_modified, &modification_rect.x, &modification_rect.y);
	modification_rect.h=img_w;
	modification_rect.w=img_h;
	
	image=rails_select_image(rails_segment_modified);		//select the image to display

        if (rails_status_of_segments[rails_segment_modified] == image)
          return;

	rails_status_of_segments[rails_segment_modified]=image;	//and write it to global table
	

	result=SDL_CreateRGBSurface(SDL_ANYFORMAT, img_w, img_h, rails_one->format->BitsPerPixel,
		rails_one->format->Rmask, rails_one->format->Gmask, rails_one->format->Bmask, rails_one->format->Amask);
	
	temp=SDL_CreateRGBSurface(SDL_ANYFORMAT, img_w, img_h, rails_one->format->BitsPerPixel,
		rails_one->format->Rmask, rails_one->format->Gmask, rails_one->format->Bmask, rails_one->format->Amask);

	SDL_BlitSurface(canvas_backup, &modification_rect, result, NULL);


	switch(image)
	{
		case 0:
		case SEG_TOP_BOTTOM:	
			SDL_BlitSurface(canvas_backup, &modification_rect, result, NULL);
			SDL_BlitSurface(rails_one, NULL, result, NULL);
		break;
		
		case SEG_LEFT_RIGHT:
			SDL_BlitSurface(canvas_backup, &modification_rect, result, NULL);
			rails_rotate(api, temp, rails_one, 1);
			use_temp=1;
		break;
		
		case SEG_LEFT_RIGHT_TOP_BOTTOM:
			SDL_BlitSurface(canvas_backup, &modification_rect, result, NULL);
			SDL_BlitSurface(rails_four, NULL, result, NULL);
		break;
		
		case SEG_LEFT_RIGHT_TOP:
			SDL_BlitSurface(rails_three, NULL, result, NULL);
		break;
		
		case SEG_LEFT_RIGHT_BOTTOM:
			rails_flip(api, temp, rails_three);
			use_temp=1;
		break;
		
		case SEG_LEFT_TOP_BOTTOM:
			rails_rotate(api, temp, rails_three, 0);
			use_temp=1;
		break;
		
		case SEG_RIGHT_TOP_BOTTOM:
			rails_rotate(api, temp, rails_three, 1);
			use_temp=1;
		break;

                case SEG_RIGHT_TOP:
			SDL_BlitSurface(rails_corner, NULL, result, NULL);
                break;

                case SEG_RIGHT_BOTTOM:
			rails_flip(api, temp, rails_corner);
			use_temp=1;
                break;

                case SEG_LEFT_TOP:
			rails_rotate(api, temp, rails_corner, 0);
			use_temp=1;
                break;

                case SEG_LEFT_BOTTOM:
			rails_rotate(api, temp, rails_corner, 0);
			use_temp=1;
                break;
	}
	
	if (use_temp) 
		SDL_BlitSurface(temp, NULL, result, NULL);
	
	SDL_FreeSurface(temp);
	SDL_BlitSurface(result, NULL, canvas, &modification_rect);
	SDL_FreeSurface(result);
	api->playsound(rails_snd, (x * 255) / canvas->w, 255);
}

void rails_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * snapshot, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{ 
	api->line((void *) api, which, canvas, snapshot, ox, oy, x, y, 1, rails_draw);

	update_rect->x=modification_rect.x;
	update_rect->y=modification_rect.y;
	update_rect->w=modification_rect.w;
	update_rect->h=modification_rect.h;
}
