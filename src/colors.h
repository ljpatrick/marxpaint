/*
  colors.h

  For Tux Paint
  List of colors

  Copyright (c) 2002 by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  June 14, 2002 - January 19, 2003
*/



/* What colors are available: */

enum {
  COLOR_BLACK,
  COLOR_WHITE,
  COLOR_RED,
  COLOR_PINK,
  COLOR_ORANGE,
  COLOR_YELLOW,
  COLOR_LIME,
  COLOR_GREEN,
  COLOR_CYAN,
  COLOR_BLUE,
  COLOR_PURPLE,
  COLOR_FUCHSIA, /* ... */
  COLOR_BROWN,
  COLOR_GREY,
  COLOR_SILVER,  /* ... */
  NUM_COLORS
};


/* Hex codes: */

int color_hexes[NUM_COLORS][3] = {
  {0, 0, 0},       /* Black */
  {255, 255, 255}, /* White */
  {255,   0,   0}, /* Red */
  {255, 128, 160}, /* Pink */
  {255, 128,   0}, /* Orange */
  {255, 255,   0}, /* Yellow */
  {  0, 255,   0}, /* Lime */
  {  0, 128,   0}, /* Green */
  {  0, 255, 255}, /* Cyan */
  {  0,   0, 255}, /* Blue */
  {128,   0, 128}, /* Purple */
  {255,   0, 255}, /* Fuchsia */
  {128,  96,   0}, /* Brown */
  {128, 128, 128}, /* Grey */
  {192, 192, 192}  /* Silver */
};


/* Color names: */

char * color_names[NUM_COLORS] = {
  gettext_noop("Black"),
  gettext_noop("White"),
  gettext_noop("Red"),
  gettext_noop("Pink"),
  gettext_noop("Orange"),
  gettext_noop("Yellow"),
  gettext_noop("Lime"),
  gettext_noop("Green"),
  gettext_noop("Cyan"),
  gettext_noop("Blue"),
  gettext_noop("Purple"),
  gettext_noop("Fuchsia"),
  gettext_noop("Brown"),
  gettext_noop("Grey"),
  gettext_noop("Silver")
};
