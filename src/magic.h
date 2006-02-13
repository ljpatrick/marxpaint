/*
  tools.h

  For Tux Paint
  List of available tools.

  Copyright (c) 2002-2006 by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  June 29, 2002 - February 13, 2006
  $Id$
*/


/* What tools are available: */

enum {
  MAGIC_FILL,
  MAGIC_GRASS,
  
  MAGIC_LARGEBRICK,
  MAGIC_SMALLBRICK,
  
  MAGIC_RAINBOW,
  MAGIC_SPARKLES,
  
  MAGIC_BLUR,
  MAGIC_SMUDGE,
  
  MAGIC_FADE,
  MAGIC_DARKEN,
  
  MAGIC_CHALK,
  MAGIC_BLOCKS,
  
  MAGIC_NEGATIVE,
  MAGIC_TINT,

  MAGIC_DRIP,
  MAGIC_CARTOON,

  MAGIC_MIRROR,
  MAGIC_FLIP,

  NUM_MAGICS
};


/* Need for the color selector: */

const int magic_colors[] = {
  COLORSEL_ENABLE, // fill
  COLORSEL_ENABLE, // grass

  COLORSEL_ENABLE, // large bricks
  COLORSEL_ENABLE, // small bricks

  COLORSEL_DISABLE,
  COLORSEL_ENABLE, // sparkles

  COLORSEL_DISABLE,
  COLORSEL_DISABLE,

  COLORSEL_DISABLE,
  COLORSEL_DISABLE,

  COLORSEL_DISABLE,
  COLORSEL_DISABLE,

  COLORSEL_DISABLE,
  COLORSEL_ENABLE, // tint

  COLORSEL_DISABLE,
  COLORSEL_DISABLE,

  COLORSEL_DISABLE,
  COLORSEL_DISABLE,
};

/* Magic tool names: */

const char * const magic_names[NUM_MAGICS] = {
  gettext_noop("Fill"),
  gettext_noop("Grass"),

  gettext_noop("Bricks"),
  gettext_noop("Bricks"),

  gettext_noop("Rainbow"),
  gettext_noop("Sparkles"),

  gettext_noop("Blur"),
  gettext_noop("Smudge"),

  gettext_noop("Lighten"),
  gettext_noop("Darken"),

  gettext_noop("Chalk"),
  gettext_noop("Blocks"),

  gettext_noop("Negative"),
  gettext_noop("Tint"),

  gettext_noop("Drip"),
  gettext_noop("Cartoon"),

  gettext_noop("Mirror"),
  gettext_noop("Flip"),
};


/* Some text to write when each tool is selected: */

const char * const magic_tips[NUM_MAGICS] = {
  gettext_noop("Click in the picture to fill that area with color."),
  gettext_noop("Click and move to draw grass. Don’t forget the dirt!"),
  
  gettext_noop("Click and move to draw large bricks."),
  gettext_noop("Click and move to draw small bricks."),
  
  gettext_noop("You can draw in rainbow colors!"),
  gettext_noop("Click and move to draw sparkles."),
  
  gettext_noop("Click and move the mouse around to blur the picture."),
  gettext_noop("Click and move the mouse around to smudge the picture."),

  gettext_noop("Click and move to fade the colors."),
  gettext_noop("Click and move to darken the colors."),

  gettext_noop("Click and move the mouse around to turn the picture into a chalk drawing."),
  gettext_noop("Click and move the mouse around to make the picture blocky."),
  
  gettext_noop("Click and move the mouse around to draw a negative."),
  gettext_noop("Click and move the mouse around to change the picture’s color."),

  gettext_noop("Click and move the mouse around to make the picture drip."),
  gettext_noop("Click and move the mouse around to turn the picture into a cartoon."),

  gettext_noop("Click to make a mirror image."),
  gettext_noop("Click to flip the picture upside-down."),
};


/* Tool icon filenames: */

const char * const magic_img_fnames[NUM_MAGICS] = {
  DATA_PREFIX "images/magic/fill.png",
  DATA_PREFIX "images/magic/grass.png",
  
  DATA_PREFIX "images/magic/largebrick.png",
  DATA_PREFIX "images/magic/smallbrick.png",
  
  DATA_PREFIX "images/magic/rainbow.png",
  DATA_PREFIX "images/magic/sparkles.png",
  
  DATA_PREFIX "images/magic/blur.png",
  DATA_PREFIX "images/magic/smudge.png",

  DATA_PREFIX "images/magic/fade.png",
  DATA_PREFIX "images/magic/darken.png",

  DATA_PREFIX "images/magic/chalk.png",
  DATA_PREFIX "images/magic/blocks.png",
  
  DATA_PREFIX "images/magic/negative.png",
  DATA_PREFIX "images/magic/tint.png",
  
  DATA_PREFIX "images/magic/drip.png",
  DATA_PREFIX "images/magic/cartoon.png",

  DATA_PREFIX "images/magic/mirror.png",
  DATA_PREFIX "images/magic/flip.png",
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

