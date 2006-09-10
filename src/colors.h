/*
  colors.h

  For Tux Paint
  List of colors

  Copyright (c) 2002-2006 by Bill Kendrick and others
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

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

  June 14, 2002 - September 9, 2006
  $Id$
*/



/* What colors are available: */

enum
{
  COLOR_BLACK,
  COLOR_DARKGREY,
  COLOR_LIGHTGREY,
  COLOR_WHITE,
  COLOR_RED,
  COLOR_ORANGE,
  COLOR_YELLOW,
  COLOR_LIGHTGREEN,
  COLOR_DARKGREEN,
  COLOR_SKYBLUE,
  COLOR_BLUE,
  COLOR_LAVENDER,
  COLOR_PURPLE,
  COLOR_PINK,
  COLOR_BROWN,
  COLOR_TAN,
  COLOR_BEIGE,
  NUM_DEFAULT_COLORS
};


/* Hex codes: */

const int default_color_hexes[NUM_DEFAULT_COLORS][3] = {
  {0, 0, 0},			/* Black */
  {128, 128, 128},		/* Dark grey */
  {192, 192, 192},		/* Light grey */
  {255, 255, 255},		/* White */
  {255, 0, 0},			/* Red */
  {255, 128, 0},		/* Orange */
  {255, 255, 0},		/* Yellow */
  {160, 228, 128},		/* Light green */
  {33, 148, 70},		/* Dark green */
  {138, 168, 205},		/* Sky blue */
  {50, 100, 255},		/* Blue */
  {186, 157, 255},		/* Lavender */
  {128, 0, 128},		/* Purple */
  {255, 165, 211},		/* Pink */
  {128, 80, 0},			/* Brown */
  {226, 189, 166},		/* Tan */
  {247, 228, 219}		/* Beige */
};


/* Color names: */

const char *const default_color_names[NUM_DEFAULT_COLORS] = {
  gettext_noop("Black!"),
  gettext_noop("Dark grey! Some people spell it “dark gray”."),
  gettext_noop("Light grey! Some people spell it “light gray”."),
  gettext_noop("White!"),
  gettext_noop("Red!"),
  gettext_noop("Orange!"),
  gettext_noop("Yellow!"),
  gettext_noop("Light green!"),
  gettext_noop("Dark green!"),
  gettext_noop("Sky blue!"),
  gettext_noop("Blue!"),
  gettext_noop("Lavender!"),
  gettext_noop("Purple!"),
  gettext_noop("Pink!"),
  gettext_noop("Brown!"),
  gettext_noop("Tan!"),
  gettext_noop("Beige!")
};
