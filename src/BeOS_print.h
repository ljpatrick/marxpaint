/* BeOS_print.h */

/* printing support for Tux Paint */
/* Marcin 'Shard' Konicki <shard@beosjournal.org> */

/* Jan. 17, 2003 */


#ifndef __BEOS_PRINT_H__
#define __BEOS_PRINT_H__

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int SurfacePrint( SDL_Surface *surf );
extern int IsPrinterAvailable();

#ifdef __cplusplus
}
#endif

#endif
