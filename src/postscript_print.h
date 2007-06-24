/*
  postscript_print.h

*/

#ifndef POSTSCRIPT_PRINT_H
#define POSTSCRIPT_PRINT_H

#include <stdio.h>
#include "SDL.h"


/* Method for printing images: */

#define PRINTMETHOD_PS          /* Direct to PostScript */
//#define PRINTMETHOD_PNM_PS       /* Output PNM, assuming it gets printed */
//#define PRINTMETHOD_PNG_PNM_PS   /* Output PNG, assuming it gets printed */



/* Default print and alt-print command, depending on the print method: */

#define DEFAULT_PRINTCOMMAND "lpr"
#define DEFAULT_ALTPRINTCOMMAND "kprinter"

#ifdef PRINTMETHOD_PNG_PNM_PS
#define PRINTCOMMAND "pngtopnm | pnmtops | " DEFAULT_PRINTCOMMAND
#elif defined(PRINTMETHOD_PNM_PS)
#define PRINTCOMMAND "pnmtops | " DEFAULT_PRINTCOMMAND
#elif defined(PRINTMETHOD_PS)
#define PRINTCOMMAND DEFAULT_PRINTCOMMAND
#else
#error No print method defined!
#endif

#ifdef PRINTMETHOD_PNG_PNM_PS
#define ALTPRINTCOMMAND "pngtopnm | pnmtops | " DEFAULT_ALTPRINTCOMMAND
#elif defined(PRINTMETHOD_PNM_PS)
#define ALTPRINTCOMMAND "pnmtops | " DEFAULT_ALTPRINTCOMMAND
#elif defined(PRINTMETHOD_PS)
#define ALTPRINTCOMMAND DEFAULT_ALTPRINTCOMMAND
#else
#error No alt print method defined!
#endif


int do_ps_save(FILE * fi,
		      // const char *restrict const fname,
		      const char *fname,
                      SDL_Surface * surf);

#endif /* POSTSCRIPT_PRINT_H */

