/*
  hqNx filter look-up table init and helper functions

  Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )
  Library-ified by Bill Kendrick <bill@newbreedsoftware.com>
  Based on "hq3x_src_c.zip" dated August 5, 2003
  from: http://www.hiend3d.com/hq3x.html

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


#ifndef HQINIT_H
#define HQINIT_H

#include "SDL.h"

static int   YUV1, YUV2;

#define Ymask 0x00FF0000
#define Umask 0x0000FF00
#define Vmask 0x000000FF

#define trY   0x00300000
#define trU   0x00000700
#define trV   0x00000006


Uint16 hqxx_getpixel(SDL_Surface * surface, int x, int y);
void InitLUTs(int * LUT16to32, int * RGBtoYUV);
inline void Interp0(SDL_Surface * dest, int x, int y, int c);
inline void Interp1(SDL_Surface * dest, int x, int y, int c1, int c2);
inline void Interp2(SDL_Surface * dest, int x, int y, int c1, int c2, int c3);
inline void Interp3(SDL_Surface * dest, int x, int y, int c1, int c2);
inline void Interp4(SDL_Surface * dest, int x, int y, int c1, int c2, int c3);
inline void Interp5(SDL_Surface * dest, int x, int y, int c1, int c2);
inline void Interp6(SDL_Surface * dest, int x, int y, int c1, int c2, int c3);
inline void Interp7(SDL_Surface * dest, int x, int y, int c1, int c2, int c3);
inline void Interp8(SDL_Surface * dest, int x, int y, int c1, int c2);
inline int Diff(unsigned int w1, unsigned int w2);

#endif

