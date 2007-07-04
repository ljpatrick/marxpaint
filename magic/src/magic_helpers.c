#include "../../src/compiler.h"
#include "magic_helpers.h"

int MAGIC_in_circle(int x, int y, int radius)
{
  if ((x * x) + (y * y) - (radius * radius) < 0)
    return (1);
  else
    return (0);
}


/* Draw a single pixel into the surface: */

void MAGIC_putpixel8(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 *p;

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (likely
      (likely((unsigned) x < (unsigned) surface->w)
       && likely((unsigned) y < (unsigned) surface->h)))
  {
    // Set a pointer to the exact location in memory of the pixel
    p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start: beginning of RAM */
		   (y * surface->pitch) +	/* Go down Y lines */
		   x);		/* Go in X pixels */


    /* Set the (correctly-sized) piece of data in the surface's RAM
     *          to the pixel value sent in: */

    *p = pixel;
  }
}

/* Draw a single pixel into the surface: */
void MAGIC_putpixel16(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 *p;

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (likely
      (likely((unsigned) x < (unsigned) surface->w)
       && likely((unsigned) y < (unsigned) surface->h)))
  {
    // Set a pointer to the exact location in memory of the pixel
    p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start: beginning of RAM */
		   (y * surface->pitch) +	/* Go down Y lines */
		   (x * 2));	/* Go in X pixels */


    /* Set the (correctly-sized) piece of data in the surface's RAM
     *          to the pixel value sent in: */

    *(Uint16 *) p = pixel;
  }
}

/* Draw a single pixel into the surface: */
void MAGIC_putpixel24(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 *p;

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (likely
      (likely((unsigned) x < (unsigned) surface->w)
       && likely((unsigned) y < (unsigned) surface->h)))
  {
    // Set a pointer to the exact location in memory of the pixel
    p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start: beginning of RAM */
		   (y * surface->pitch) +	/* Go down Y lines */
		   (x * 3));	/* Go in X pixels */


    /* Set the (correctly-sized) piece of data in the surface's RAM
     *          to the pixel value sent in: */

    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
    {
      p[0] = (pixel >> 16) & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = pixel & 0xff;
    }
    else
    {
      p[0] = pixel & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = (pixel >> 16) & 0xff;
    }

  }
}

/* Draw a single pixel into the surface: */
void MAGIC_putpixel32(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 *p;

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (likely
      (likely((unsigned) x < (unsigned) surface->w)
       && likely((unsigned) y < (unsigned) surface->h)))
  {
    // Set a pointer to the exact location in memory of the pixel
    p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start: beginning of RAM */
		   (y * surface->pitch) +	/* Go down Y lines */
		   (x * 4));	/* Go in X pixels */


    /* Set the (correctly-sized) piece of data in the surface's RAM
     *          to the pixel value sent in: */

    *(Uint32 *) p = pixel;	// 32-bit display
  }
}

/* Get a pixel: */
Uint32 MAGIC_getpixel8(SDL_Surface * surface, int x, int y)
{
  Uint8 *p;

  /* get the X/Y values within the bounds of this surface */
  if (unlikely((unsigned) x > (unsigned) surface->w - 1u))
    x = (x < 0) ? 0 : surface->w - 1;
  if (unlikely((unsigned) y > (unsigned) surface->h - 1u))
    y = (y < 0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start at top of RAM */
		 (y * surface->pitch) +	/* Go down Y lines */
		 x);		/* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  return (*p);
}

/* Get a pixel: */
Uint32 MAGIC_getpixel16(SDL_Surface * surface, int x, int y)
{
  Uint8 *p;

  /* get the X/Y values within the bounds of this surface */
  if (unlikely((unsigned) x > (unsigned) surface->w - 1u))
    x = (x < 0) ? 0 : surface->w - 1;
  if (unlikely((unsigned) y > (unsigned) surface->h - 1u))
    y = (y < 0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start at top of RAM */
		 (y * surface->pitch) +	/* Go down Y lines */
		 (x * 2));	/* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  return (*(Uint16 *) p);
}

/* Get a pixel: */
Uint32 MAGIC_getpixel24(SDL_Surface * surface, int x, int y)
{
  Uint8 *p;
  Uint32 pixel;

  /* get the X/Y values within the bounds of this surface */
  if (unlikely((unsigned) x > (unsigned) surface->w - 1u))
    x = (x < 0) ? 0 : surface->w - 1;
  if (unlikely((unsigned) y > (unsigned) surface->h - 1u))
    y = (y < 0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start at top of RAM */
		 (y * surface->pitch) +	/* Go down Y lines */
		 (x * 3));	/* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  /* Depending on the byte-order, it could be stored RGB or BGR! */

  if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
    pixel = p[0] << 16 | p[1] << 8 | p[2];
  else
    pixel = p[0] | p[1] << 8 | p[2] << 16;

  return pixel;
}

/* Get a pixel: */
Uint32 MAGIC_getpixel32(SDL_Surface * surface, int x, int y)
{
  Uint8 *p;

  /* get the X/Y values within the bounds of this surface */
  if (unlikely((unsigned) x > (unsigned) surface->w - 1u))
    x = (x < 0) ? 0 : surface->w - 1;
  if (unlikely((unsigned) y > (unsigned) surface->h - 1u))
    y = (y < 0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start at top of RAM */
		 (y * surface->pitch) +	/* Go down Y lines */
		 (x * 4));	/* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  return *(Uint32 *) p;		// 32-bit display
}

void (*MAGIC_putpixels[]) (SDL_Surface *, int, int, Uint32) =
{
MAGIC_putpixel8, MAGIC_putpixel8, MAGIC_putpixel16, MAGIC_putpixel24, MAGIC_putpixel32};


Uint32(*MAGIC_getpixels[])(SDL_Surface *, int, int) =
{
MAGIC_getpixel8, MAGIC_getpixel8, MAGIC_getpixel16, MAGIC_getpixel24, MAGIC_getpixel32};

