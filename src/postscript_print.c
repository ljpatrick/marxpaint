
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <paper.h>
#include "postscript_print.h"
#include "pixels.h"

///////////////////////////////////// PostScript printing ///////////
#ifdef PRINTMETHOD_PS

/* Actually save the PostScript data to the file stream: */
int do_ps_save(FILE * fi,
		      // const char *restrict const fname,
		      const char * fname,
		      SDL_Surface * surf)
{
  unsigned char * /* restrict const */ ps_row = malloc(surf->w * 3);
  int x, y;
  char buf[256];
  Uint32(*getpixel) (SDL_Surface *, int, int) =
    getpixels[surf->format->BytesPerPixel];
  time_t t = time(NULL);

  fprintf(fi, "%%!PS-Adobe-3.0 EPSF-3.0\n");	// probably broken, but close enough maybe
  fprintf(fi, "%%%%Title: (%s)\n", fname);
  strftime(buf, sizeof buf - 1, "%a %b %e %H:%M:%S %Y", localtime(&t));
  fprintf(fi, "%%%%CreationDate: (%s)\n", buf);
  fprintf(fi, "%%%%Creator: (Tux Paint " VER_VERSION ", " VER_DATE ")\n");
  fprintf(fi, "%%%%LanguageLevel: 2\n");
//  fprintf(fi, "%%%%BoundingBox: 72 214 540 578\n");  // doubt we have the needed info
  fprintf(fi, "%%%%DocumentData: Binary\n");
  fprintf(fi, "%%%%EndComments\n");
  fprintf(fi, "\n");
  fprintf(fi, "gsave\n");
  fprintf(fi, "\n");

  if (surf->w > surf->h)
    fprintf(fi, "90 rotate\n");  // landscape mode

  fprintf(fi, "%% First, grab the page size.\n");
  fprintf(fi, "gsave\n");
  fprintf(fi, "  clippath\n");
  fprintf(fi, "  pathbbox\n");
  fprintf(fi, "grestore\n");
  fprintf(fi, "/ury exch def\n");
  fprintf(fi, "/urx exch def\n");
  fprintf(fi, "/lly exch def\n");
  fprintf(fi, "/llx exch def\n");
  fprintf(fi, "\n");
  fprintf(fi, "llx lly translate\n");
  fprintf(fi, "\n");
  fprintf(fi, "/width %u def\n", surf->w);
  fprintf(fi, "/height %u def\n", surf->h);
  fprintf(fi, "width height scale\n");
  fprintf(fi, "\n");
  fprintf(fi, "urx llx sub width div\n");
  fprintf(fi, "ury lly sub height div\n");
  fprintf(fi, "%% now do a 'min' operation\n");
  fprintf(fi, "2 copy gt { exch } if pop\n");
  fprintf(fi, "\n");
  fprintf(fi, "dup scale\n");
  fprintf(fi, "/DeviceRGB setcolorspace\n");
  fprintf(fi, "<<\n");
  fprintf(fi, "  /ImageType 1\n");
  fprintf(fi, "  /Width width /Height height\n");
  fprintf(fi, "  /BitsPerComponent 8\n");
  fprintf(fi, "  /ImageMatrix [width 0 0 height neg 0 height]\n");
  fprintf(fi, "  /Decode [0 1 0 1 0 1]\n");
  fprintf(fi, "  /DataSource currentfile\n");
  fprintf(fi, ">>\n");
  fprintf(fi, "%%%%BeginData: %u Binary Bytes\n", surf->w * surf->h * 3u);
  fprintf(fi, "image\n");

  /* Save the picture: */
  for (y = 0; y < surf->h; y++)
  {
    for (x = 0; x < surf->w; x++)
    {
      Uint8 r, g, b;
      SDL_GetRGB(getpixel(surf, x, y), surf->format, &r, &g, &b);
      ps_row[x * 3 + 0] = r;
      ps_row[x * 3 + 1] = g;
      ps_row[x * 3 + 2] = b;
    }
    fwrite(ps_row, surf->w, 3, fi);
  }
  free(ps_row);

  fprintf(fi, "\n");
  fprintf(fi, "%%%%EndData\n");
  fprintf(fi, "grestore\n");
  fprintf(fi, "showpage\n");
  fprintf(fi, "%%%%EOF\n");

  fclose(fi);
  return 1;
}

#endif
/////////////////////////////////////////////////////////////////

