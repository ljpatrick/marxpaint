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
extern const  int   Ymask;
extern const  int   Umask;
extern const  int   Vmask;
extern const  int   trY;
extern const  int   trU;
extern const  int   trV;

void InitLUTs(int * LUT16to32, int * RGBtoYUV);
inline void Interp1(Uint8 * pc, int c1, int c2);
inline void Interp2(Uint8 * pc, int c1, int c2, int c3);
inline void Interp3(Uint8 * pc, int c1, int c2);
inline void Interp4(Uint8 * pc, int c1, int c2, int c3);
inline void Interp5(Uint8 * pc, int c1, int c2);
inline void Interp6(Uint8 * pc, int c1, int c2, int c3);
inline void Interp7(Uint8 * pc, int c1, int c2, int c3);
inline void Interp8(Uint8 * pc, int c1, int c2);
inline int Diff(unsigned int w1, unsigned int w2);

#endif

