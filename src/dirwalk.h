/*
  dirwalk.h
*/

#ifndef DIRWALK_H
#define DIRWALK_H

#include "SDL.h"

#include "compiler.h"

/////////////////////////////// directory tree walking /////////////////////

#define TP_FTW_UNKNOWN 1
#define TP_FTW_DIRECTORY 2
#define TP_FTW_NORMAL 0

#define TP_FTW_PATHSIZE 400

typedef struct tp_ftw_str
{
  char *str;
  unsigned char len;
//  unsigned char is_rsrc;
} tp_ftw_str;


void loadfont_callback(SDL_Surface * screen, const char *restrict const dir,
		       unsigned dirlen, tp_ftw_str * files, unsigned i);
int compare_ftw_str(const void *v1, const void *v2);
void tp_ftw(SDL_Surface * screen, char *restrict const dir, unsigned dirlen,
	    int rsrc, void (*fn) (SDL_Surface * screen,
				  const char *restrict const dir,
				  unsigned dirlen, tp_ftw_str * files,
				  unsigned count));

#endif
