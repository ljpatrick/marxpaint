/*
  colors.h

  For Tux Paint
  List of colors

  Copyright (c) 2002 by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  June 14, 2002 - November 21, 2004
*/



/* What colors are available: */

enum {
  COLOR_BLACK,
  COLOR_GREY,
  COLOR_SILVER,
  COLOR_WHITE,
  COLOR_RED,
  COLOR_PINK,
  COLOR_ORANGE,
  COLOR_YELLOW,
  COLOR_NEON,
  COLOR_GREEN,
  COLOR_SKYBLUE,
  COLOR_BLUE,
  COLOR_PURPLE,
  COLOR_MAGENTA,
  COLOR_BROWN,
  COLOR_TAN,
  COLOR_BEIGE,
  NUM_COLORS
};


/* Hex codes: */

const int color_hexes[NUM_COLORS][3] = {
  {0, 0, 0},       /* Black */
  {128, 128, 128}, /* Gray */
  {192, 192, 192}, /* Silver */
  {255, 255, 255}, /* White */
  {255,   0,   0}, /* Red */
  {255, 165, 211}, /* Pink */
  {255, 128,   0}, /* Orange */
  {255, 255,   0}, /* Yellow */
  {140, 255,   0}, /* Neon */
  { 33, 148,  33}, /* Green */
  {138, 168, 205}, /* Sky blue */
  {  0,   0, 255}, /* Blue */
  {128,   0, 128}, /* Purple */
  {255,   0, 255}, /* Magenta */
  {128,  96,   0}, /* Brown */
  {226, 189, 166}, /* Tan */
  {247, 228, 219}  /* Beige */
};


/* Color names: */

const char * const color_names[NUM_COLORS] = {
  gettext_noop("Black!"),
  gettext_noop("Gray!"),
  gettext_noop("Silver!"),
  gettext_noop("White!"),
  gettext_noop("Red!"),
  gettext_noop("Pink!"),
  gettext_noop("Orange!"),
  gettext_noop("Yellow!"),
  gettext_noop("Neon green!"),
  gettext_noop("Green!"),
  gettext_noop("Sky blue!"),
  gettext_noop("Blue!"),
  gettext_noop("Purple!"),
  gettext_noop("Magenta!"),
  gettext_noop("Brown!"),
  gettext_noop("Tan!"),
  gettext_noop("Beige!")
};
