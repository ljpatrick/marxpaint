#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include "tp_magic_api.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

/* Our globals: */

enum { SIDE_LEFT, SIDE_RIGHT };
enum { LEAFSIDE_RIGHT_DOWN,
       LEAFSIDE_LEFT_DOWN,
       LEAFSIDE_RIGHT_UP,
       LEAFSIDE_LEFT_UP };

Mix_Chunk * flower_snd;
Uint8 flower_r, flower_g, flower_b;
int flower_min_x, flower_max_x, flower_bottom_x, flower_bottom_y;
int flower_side_first;
int flower_side_decided;
SDL_Surface * flower_base, * flower_leaf, * flower_petals,
  * flower_petals_colorized;

/* Local function prototypes: */

typedef struct
{
  float x, y;
} Point2D;

void flower_predrag(magic_api * api, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y);
void flower_drawbase(magic_api * api, SDL_Surface * canvas);
void flower_drawstalk(magic_api * api, SDL_Surface * canvas,
		      int top_x, int top_y, int minx, int maxx,
		      int bottom_x, int bottom_y, int final);
void flower_drawflower(magic_api * api, SDL_Surface * canvas, int x, int y);
Point2D flower_PointOnCubicBezier(Point2D* cp, float t);
void flower_ComputeBezier(Point2D* cp, int numberOfPoints, Point2D* curve);
void flower_colorize_petals(magic_api * api);



Uint32 flower_api_version(void) { return(TP_MAGIC_API_VERSION); }


// No setup required:
int flower_init(magic_api * api)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/sounds/magic/flower.wav",
	    api->data_directory);
  flower_snd = Mix_LoadWAV(fname);

  snprintf(fname, sizeof(fname), "%s/images/magic/flower_base.png",
	    api->data_directory);
  flower_base = IMG_Load(fname);

  snprintf(fname, sizeof(fname), "%s/images/magic/flower_leaf.png",
	    api->data_directory);
  flower_leaf = IMG_Load(fname);

  snprintf(fname, sizeof(fname), "%s/images/magic/flower_petals.png",
	    api->data_directory);
  flower_petals = IMG_Load(fname);

  return(1);
}

// We have multiple tools:
int flower_get_tool_count(magic_api * api)
{
  return(1);
}

// Load our icons:
SDL_Surface * flower_get_icon(magic_api * api, int which)
{
  char fname[1024];

  snprintf(fname, sizeof(fname), "%s/images/magic/flower.png",
	   api->data_directory);

  return(IMG_Load(fname));
}

// Return our names, localized:
char * flower_get_name(magic_api * api, int which)
{
  return(strdup(gettext("Flower")));
}

// Return our descriptions, localized:
char * flower_get_description(magic_api * api, int which)
{
  return(strdup(gettext("Click and drag to draw a flower stalk. Let go to finish the flower.")));
}

// Affect the canvas on drag:
void flower_predrag(magic_api * api, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y)
{
  if (x < flower_min_x)
    flower_min_x = x;
  if (ox < flower_min_x)
    flower_min_x = ox;
  if (x > flower_max_x)
    flower_max_x = x;
  if (ox > flower_max_x)
    flower_max_x = ox;

  if (y > flower_bottom_y)
    y = flower_bottom_y;
  if (oy > flower_bottom_y)
    y = flower_bottom_y;

  // Determine which way to bend first:
  //
  if (flower_side_decided == 0)
  {
    if (x < flower_bottom_x - 10)
    {
      flower_side_first = SIDE_LEFT;
      flower_side_decided = 1;
    }
    else if (x > flower_bottom_x + 10)
    {
      flower_side_first = SIDE_RIGHT;
      flower_side_decided = 1;
    }
  }
}

void flower_drag(magic_api * api, int which, SDL_Surface * canvas,
	          SDL_Surface * last, int ox, int oy, int x, int y,
		  SDL_Rect * update_rect)
{
  flower_predrag(api, canvas, last, ox, oy, x, y);


  /* Erase any old stuff; this is a live-edited effect: */

  SDL_BlitSurface(last, NULL, canvas, NULL);


  /* Draw the base and the stalk (low-quality) for now: */

  flower_drawstalk(api, canvas,
		   x, y, flower_min_x, flower_max_x,
		   flower_bottom_x, flower_bottom_y, !(api->button_down()));
  
  flower_drawbase(api, canvas);

  update_rect->x = 0;
  update_rect->y = 0;
  update_rect->w = canvas->w; 
  update_rect->h = canvas->h;
}

// Affect the canvas on click:
void flower_click(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
  flower_min_x = x;
  flower_max_x = x;
  flower_bottom_x = x;
  flower_bottom_y = y - flower_base->h;

  flower_side_decided = 0;
  flower_side_first = SIDE_LEFT;

  flower_drag(api, which, canvas, last, x, y, x, y, update_rect);
}

// Affect the canvas on release:
void flower_release(magic_api * api, int which,
	           SDL_Surface * canvas, SDL_Surface * last,
	           int x, int y, SDL_Rect * update_rect)
{
  /* Don't let flower be too low compared to base: */

  if (y >= flower_bottom_y - 32)
    y = flower_bottom_y - 32;


  /* Do final calcs and draw base: */

  flower_predrag(api, canvas, last, x, y, x, y);

  
  /* Erase any old stuff: */

  SDL_BlitSurface(last, NULL, canvas, NULL);


  /* Draw high-quality stalk, and flower: */

  flower_drawstalk(api, canvas,
		   x, y, flower_min_x, flower_max_x,
		   flower_bottom_x, flower_bottom_y, 1);

  flower_drawflower(api, canvas, x, y);

  flower_drawbase(api, canvas);

  
  update_rect->x = 0;
  update_rect->y = 0;
  update_rect->w = canvas->w;
  update_rect->h = canvas->h;

  api->playsound(flower_snd, (x * 255) / canvas->w, 255); /* FIXME: Distance? */
}


void flower_drawflower(magic_api * api, SDL_Surface * canvas, int x, int y)
{
  SDL_Rect dest;

  dest.x = x - (flower_petals->w / 2);
  dest.y = y - (flower_petals->h / 2);

  SDL_BlitSurface(flower_petals_colorized, NULL, canvas, &dest);
}

void flower_drawbase(magic_api * api, SDL_Surface * canvas)
{
  SDL_Rect dest;

  dest.x = flower_bottom_x - (flower_base->w / 2);
  dest.y = flower_bottom_y;

  SDL_BlitSurface(flower_base, NULL, canvas, &dest);
}

void flower_drawstalk(magic_api * api, SDL_Surface * canvas,
		      int top_x, int top_y, int minx, int maxx,
		      int bottom_x, int bottom_y, int final)
{
  Point2D control_points[4];
  Point2D * curve;
  int i, n_points;
  int left, right;
  SDL_Rect dest, src;
  int xx, yy, side;


  /* Compute a nice bezier curve for the stalk, based on the
     base (x,y), leftmost (x), rightmost (x), and top (x,y) */

  control_points[0].x = top_x;
  control_points[0].y = top_y;

  if (flower_side_first == SIDE_LEFT)
  {
    control_points[1].x = minx;
    control_points[2].x = maxx;
  }
  else
  {
    control_points[1].x = maxx;
    control_points[2].x = minx;
  }

  control_points[1].y = ((bottom_y - top_y) / 3) + top_y;
  control_points[2].y = (((bottom_y - top_y) / 3) * 2) + top_y;
 
  control_points[3].x = bottom_x;
  control_points[3].y = bottom_y;
 
  if (final == 0)
    n_points = 8;
  else
    n_points = bottom_y - top_y;

  curve = (Point2D *) malloc(sizeof(Point2D) * n_points);

  flower_ComputeBezier(control_points, n_points, curve);


  /* Draw the curve: */
 
  for (i = 0; i < n_points - 1; i++)
  {
    if (final == 0)
    {
      dest.x = curve[i].x;
      dest.y = curve[i].y;
      dest.w = 2;
      dest.h = 2;
    }
    else
    {
      left = min(curve[i].x, curve[i + 1].x);
      right = max(curve[i].x, curve[i + 1].x);

      dest.x = left;
      dest.y = curve[i].y;
      dest.w = right - left + 1;
      dest.h = 2;
    }

    SDL_FillRect(canvas, &dest, SDL_MapRGB(canvas->format, 0, 128, 0));


    /* When we're done (final render), we can add some random leaves: */

    if (final && i > 32 && i < n_points - 32 && (i % 16) == 0 &&
	(rand() % 5) > 0)
    {
      /* Check for hard left/right angles: */

      side = -1;

      if (curve[i - 2].x - curve[i + 2].x > 5)
      {
        /* Hard lower-left-to-upper-right (/),
           stick either a left-upward or right-downward facing leaf */

        if (rand() % 10 < 5)
          side = LEAFSIDE_LEFT_UP;
        else
          side = LEAFSIDE_RIGHT_DOWN;
      }
      else if (curve[i - 2].x - curve[i + 2].x < -5)
      {
        /* Hard lower-right-to-upper-left (\)
           stick either a right-upward or left-downward facing leaf */

        if (rand() % 10 < 5)
          side = LEAFSIDE_LEFT_DOWN;
        else
          side = LEAFSIDE_RIGHT_UP;
      }
      else if (abs(curve[i - 2].x - curve[i + 2].x) < 5)
      {
        /* Mostly up; stick left- or right-downward: */

        if (rand() % 10 < 5)
          side = LEAFSIDE_LEFT_DOWN;
        else
          side = LEAFSIDE_RIGHT_DOWN;
      }


      /* Draw the appropriately-oriented leaf, if any: */

      if (side == LEAFSIDE_RIGHT_DOWN)
      {
        dest.x = curve[i].x;
        dest.y = curve[i].y;

        SDL_BlitSurface(flower_leaf, NULL, canvas, &dest);
      }
      else if (side == LEAFSIDE_LEFT_DOWN)
      {
        for (xx = 0; xx < flower_leaf->w; xx++)
        {
          src.x = xx;
          src.y = 0;
          src.w = 1;
          src.h = flower_leaf->h;

          dest.x = curve[i].x - xx;
          dest.y = curve[i].y;

          SDL_BlitSurface(flower_leaf, &src, canvas, &dest);
        }
      }
      else if (side == LEAFSIDE_RIGHT_UP)
      {
        for (yy = 0; yy < flower_leaf->h; yy++)
        {
          src.x = 0;
          src.y = yy;
          src.w = flower_leaf->w;
          src.h = 1;

          dest.x = curve[i].x;
          dest.y = curve[i].y - yy;

          SDL_BlitSurface(flower_leaf, &src, canvas, &dest);
        }
      }
      else if (side == LEAFSIDE_LEFT_UP)
      {
        for (xx = 0; xx < flower_leaf->w; xx++)
        {
          for (yy = 0; yy < flower_leaf->h; yy++)
          {
            src.x = xx;
            src.y = yy;
            src.w = 1;
            src.h = 1;

            dest.x = curve[i].x - xx;
            dest.y = curve[i].y - yy;

            SDL_BlitSurface(flower_leaf, &src, canvas, &dest);
          }
        }
      }
    }
  } 

  free(curve);
}

void flower_shutdown(magic_api * api)
{
  if (flower_snd != NULL)
    Mix_FreeChunk(flower_snd);

  if (flower_base != NULL)
    SDL_FreeSurface(flower_base);
  if (flower_leaf != NULL)
    SDL_FreeSurface(flower_leaf);
  if (flower_petals != NULL)
    SDL_FreeSurface(flower_petals);
  if (flower_petals_colorized != NULL)
    SDL_FreeSurface(flower_petals_colorized);
}

// Record the color from Tux Paint:
void flower_set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 b)
{
  flower_r = r;
  flower_g = g;
  flower_b = b;

  flower_colorize_petals(api);
}

// Use colors:
int flower_requires_colors(magic_api * api, int which)
{
  return 1;
}



/*
Code to generate a cubic Bezier curve
*/

/*
cp is a 4 element array where:
cp[0] is the starting point, or P0 in the above diagram
cp[1] is the first control point, or P1 in the above diagram
cp[2] is the second control point, or P2 in the above diagram
cp[3] is the end point, or P3 in the above diagram
t is the parameter value, 0 <= t <= 1
*/

Point2D flower_PointOnCubicBezier( Point2D* cp, float t )
{
    float   ax, bx, cx;
    float   ay, by, cy;
    float   tSquared, tCubed;
    Point2D result;

    /* calculate the polynomial coefficients */

    cx = 3.0 * (cp[1].x - cp[0].x);
    bx = 3.0 * (cp[2].x - cp[1].x) - cx;
    ax = cp[3].x - cp[0].x - cx - bx;
        
    cy = 3.0 * (cp[1].y - cp[0].y);
    by = 3.0 * (cp[2].y - cp[1].y) - cy;
    ay = cp[3].y - cp[0].y - cy - by;
        
    /* calculate the curve point at parameter value t */
        
    tSquared = t * t;
    tCubed = tSquared * t;
        
    result.x = (ax * tCubed) + (bx * tSquared) + (cx * t) + cp[0].x;
    result.y = (ay * tCubed) + (by * tSquared) + (cy * t) + cp[0].y;
        
    return result;
}

/*
 ComputeBezier fills an array of Point2D structs with the curve   
 points generated from the control points cp. Caller must 
 allocate sufficient memory for the result, which is 
 <sizeof(Point2D) numberOfPoints>
*/

void flower_ComputeBezier( Point2D* cp, int numberOfPoints, Point2D* curve )
{
    float   dt;
    int   i;

    dt = 1.0 / ( numberOfPoints - 1 );

    for( i = 0; i < numberOfPoints; i++)
        curve[i] = flower_PointOnCubicBezier( cp, i*dt );
}


void flower_colorize_petals(magic_api * api)
{
  Uint32 amask;
  int x, y;
  Uint8 r, g, b, a;

  if (flower_petals_colorized != NULL)
    SDL_FreeSurface(flower_petals_colorized);

  /* Create a surface to render into: */

  amask = ~(flower_petals->format->Rmask |
            flower_petals->format->Gmask |
            flower_petals->format->Bmask);

  flower_petals_colorized =
    SDL_CreateRGBSurface(SDL_SWSURFACE,
                         flower_petals->w,
                         flower_petals->h,
                         flower_petals->format->BitsPerPixel,
                         flower_petals->format->Rmask,
                         flower_petals->format->Gmask,
                         flower_petals->format->Bmask, amask);

  /* Render the new petals: */

  SDL_LockSurface(flower_petals);
  SDL_LockSurface(flower_petals_colorized);

  for (y = 0; y < flower_petals->h; y++)
  {
    for (x = 0; x < flower_petals->w; x++)
    {
      SDL_GetRGBA(api->getpixel(flower_petals, x, y),
                  flower_petals->format, &r, &g, &b, &a);

      api->putpixel(flower_petals_colorized, x, y,
                    SDL_MapRGBA(flower_petals_colorized->format,
				flower_r, flower_g, flower_b, a));

      if (api->in_circle((x - flower_petals->w / 2),
			 (y - flower_petals->h / 2),
			 8))
      {
        api->putpixel(flower_petals_colorized, x, y,
                      SDL_MapRGBA(flower_petals_colorized->format,
				  0xFF, 0xFF, 0x00, a));
      }
    }
  }

  SDL_UnlockSurface(flower_petals_colorized);
  SDL_UnlockSurface(flower_petals);
}
