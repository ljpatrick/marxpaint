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

#define Ymask 0xF800
#define Umask 0x07E0
#define Vmask 0x001F

//trY in 32 = 0x00300000
//trU in 32 = 0x00000700
//trV in 32 = 0x00000006

#define trY   ((0x30 & 0xF8) << 8)
#define trU   ((0x07 & 0x7E) << 3)
#define trV   ((0x06 & 0x1F))

#ifdef WIN32
#define inline
#endif

Uint16 hqxx_getpixel(SDL_Surface * surface, int x, int y, Uint8 * alpha);
void InitLUTs(Uint32 * RGBtoYUV);
inline void Interp0(SDL_Surface * dest, int x, int y, Uint16 c, Uint8 alpha);
inline void Interp1(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint8 alpha);
inline void Interp2(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint16 c3, Uint8 alpha);
inline void Interp3(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint8 alpha);
inline void Interp4(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint16 c3, Uint8 alpha);
inline void Interp5(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint8 alpha);
inline void Interp6(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint16 c3, Uint8 alpha);
inline void Interp7(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint16 c3, Uint8 alpha);
inline void Interp8(SDL_Surface * dest, int x, int y, Uint16 c1, Uint16 c2, Uint8 alpha);
inline int Diff(unsigned int w1, unsigned int w2);

#ifdef WIN32
#undef inline
#endif

#endif

