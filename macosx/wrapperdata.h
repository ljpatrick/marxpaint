/*
 *  wrapperdata.h
 *  TuxPaint
 *
 *  Created by Martin Fuhrer on Wed May 12 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef WRAPPER_DATA
#define WRAPPER_DATA

struct WrapperDataStruct
{
    char dataPath[2048];  // path to data folder inside application bundle
    char preferencesPath[2048];  // path to the user's preferences folder
	char fontsPath[2048]; // path to the user's fonts folder
    int foundSDL;         // was SDL.framework found?
    int foundSDL_image;   // was SDL_image.framework found?
    int foundSDL_mixer;   // was SDL_mixer.framework found?
};

typedef struct WrapperDataStruct WrapperData;

#endif