/*
  ios.c

  Copyright (c) 2021
  http://www.tuxpaint.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  (See COPYING.txt)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ios.h"

#define IOS_FONTS_PATH              "../Library/Fonts"
#define IOS_PREFERENCES_PATH        "../Library/Application Support/TuxPaint"
#define IOS_PICTURES_PATH           "../Documents"


const char *apple_fontsPath(void)
{
    return IOS_FONTS_PATH;
}


const char *apple_preferencesPath(void)
{
    return IOS_PREFERENCES_PATH;
}


const char *apple_globalPreferencesPath(void)
{
    return IOS_PREFERENCES_PATH;
}


const char *apple_picturesPath(void)
{
    return IOS_PICTURES_PATH;
}
