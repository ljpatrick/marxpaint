/*
  tools.h

  For Tux Paint
  List of available tools.

  Copyright (c) 2002-2006 by Bill Kendrick
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

  June 29, 2002 - February 17, 2006
  $Id$
*/


/* What tools are available: */

enum
{
  MAGIC_FILL,
  MAGIC_GRASS,

  MAGIC_LARGEBRICK,
  MAGIC_SMALLBRICK,

  MAGIC_RAINBOW,
  MAGIC_SPARKLES,

  MAGIC_BLUR,
  MAGIC_SMUDGE,

  MAGIC_CHALK,
  MAGIC_BLOCKS,

  MAGIC_TINT,

  MAGIC_DRIP,
  MAGIC_CARTOON,

  NUM_MAGICS
};


/* Need for the color selector: */

const int magic_colors[] = {
  COLORSEL_ENABLE,		// fill
  COLORSEL_ENABLE,		// grass

  COLORSEL_ENABLE,		// large bricks
  COLORSEL_ENABLE,		// small bricks

  COLORSEL_DISABLE,		// rainbow
  COLORSEL_ENABLE,		// sparkles

  COLORSEL_DISABLE,		// blur
  COLORSEL_DISABLE,		// smudge

  COLORSEL_DISABLE,		// chalk
  COLORSEL_DISABLE,		// blocks

  COLORSEL_ENABLE,		// tint

  COLORSEL_DISABLE,		// drip
  COLORSEL_DISABLE,		// cartoon
};

/* Magic tool names: */

const char *const magic_names[NUM_MAGICS] = {
  gettext_noop("Fill"),
  gettext_noop("Grass"),

  gettext_noop("Bricks"),
  gettext_noop("Bricks"),

  gettext_noop("Rainbow"),
  gettext_noop("Sparkles"),

  gettext_noop("Blur"),
  gettext_noop("Smudge"),

  gettext_noop("Chalk"),
  gettext_noop("Blocks"),

  gettext_noop("Tint"),

  gettext_noop("Drip"),
  gettext_noop("Cartoon"),
};


/* Some text to write when each tool is selected: */

const char *const magic_tips[NUM_MAGICS] = {
  gettext_noop("Click in the picture to fill that area with color."),
  gettext_noop("Click and move to draw grass. Don’t forget the dirt!"),

  gettext_noop("Click and move to draw large bricks."),
  gettext_noop("Click and move to draw small bricks."),

  gettext_noop("You can draw in rainbow colors!"),
  gettext_noop("Click and move to draw sparkles."),

  gettext_noop("Click and move the mouse around to blur the picture."),
  gettext_noop("Click and move the mouse around to smudge the picture."),

  gettext_noop
    ("Click and move the mouse around to turn the picture into a chalk drawing."),
  gettext_noop("Click and move the mouse around to make the picture blocky."),

  gettext_noop
    ("Click and move the mouse around to change the picture’s color."),

  gettext_noop("Click and move the mouse around to make the picture drip."),
  gettext_noop
    ("Click and move the mouse around to turn the picture into a cartoon."),
};


/* Tool icon filenames: */

const char *const magic_img_fnames[NUM_MAGICS] = {
  DATA_PREFIX "images/magic/fill.png",
  DATA_PREFIX "images/magic/grass.png",

  DATA_PREFIX "images/magic/largebrick.png",
  DATA_PREFIX "images/magic/smallbrick.png",

  DATA_PREFIX "images/magic/rainbow.png",
  DATA_PREFIX "images/magic/sparkles.png",

  DATA_PREFIX "images/magic/blur.png",
  DATA_PREFIX "images/magic/smudge.png",

  DATA_PREFIX "images/magic/chalk.png",
  DATA_PREFIX "images/magic/blocks.png",

  DATA_PREFIX "images/magic/tint.png",

  DATA_PREFIX "images/magic/drip.png",
  DATA_PREFIX "images/magic/cartoon.png",
};


/* FIXME: Should we show different Tux icons depending on magic,
   like tools? */


/* Rainbow color values: */

#define NUM_RAINBOW_COLORS 23

const int rainbow_hexes[NUM_RAINBOW_COLORS][3] = {
  {255, 0, 0},
  {255, 64, 0},
  {255, 128, 0},
  {255, 192, 0},
  {255, 255, 0},
  {192, 255, 0},
  {128, 255, 0},
  {64, 255, 0},
  {0, 255, 0},
  {0, 255, 64},
  {0, 255, 128},
  {0, 255, 192},
  {0, 255, 255},
  {0, 192, 255},
  {0, 128, 255},
  {0, 64, 255},
  {64, 0, 255},
  {128, 0, 255},
  {192, 0, 255},
  {255, 0, 255},
  {255, 0, 192},
  {255, 0, 128},
  {255, 0, 64}
};
